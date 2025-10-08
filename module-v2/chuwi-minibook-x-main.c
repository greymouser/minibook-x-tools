// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Chuwi Minibook X Platform Driver - Main Module
 *
 * Copyright (C) 2025 Your Name
 *
 * This driver provides tablet mode detection and screen orientation support
 * for the Chuwi Minibook X convertible laptop using dual accelerometers.
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/acpi.h>
#include <linux/input.h>
#include <linux/iio/consumer.h>
#include <linux/iio/iio.h>
#include <linux/delay.h>
#include <linux/workqueue.h>
#include <linux/sysfs.h>
#include <linux/mutex.h>
#include <linux/math64.h>
#include <linux/i2c.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#include "chuwi-minibook-x.h"

/* Module information */
MODULE_DESCRIPTION("Chuwi Minibook X tablet mode and orientation driver");
MODULE_AUTHOR("Your Name");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:" CHUWI_MINIBOOK_X_DRIVER_NAME);

/* Module dependencies - ensure required drivers are loaded */
MODULE_SOFTDEP("pre: mxc4005");
MODULE_SOFTDEP("pre: serial_multi_instantiate");

/* Module parameters */
static unsigned int poll_interval = 200;
module_param(poll_interval, uint, 0644);
MODULE_PARM_DESC(poll_interval, "Polling interval in milliseconds (default: 200)");

static unsigned int enter_threshold = 200;
module_param(enter_threshold, uint, 0644);
MODULE_PARM_DESC(enter_threshold, "Angle threshold to enter tablet mode in degrees (default: 200)");

static unsigned int exit_threshold = 170;
module_param(exit_threshold, uint, 0644);
MODULE_PARM_DESC(exit_threshold, "Angle threshold to exit tablet mode in degrees (default: 170)");

static unsigned int hysteresis = 10;
module_param(hysteresis, uint, 0644);
MODULE_PARM_DESC(hysteresis, "Hysteresis for tablet mode switching in degrees (default: 10)");

static bool signed_mode = true;
module_param(signed_mode, bool, 0644);
MODULE_PARM_DESC(signed_mode, "Use signed angle mode (0-360 degrees) instead of unsigned (0-180 degrees)");

static bool enable_orientation = true;
module_param(enable_orientation, bool, 0644);
MODULE_PARM_DESC(enable_orientation, "Enable automatic screen orientation detection (default: true)");

/* Global driver context - will be replaced with proper device matching */
static struct chuwi_minibook_x *g_chip;

/*
 * IIO channel names for accelerometer access
 * The Chuwi Minibook X has two MXC4005 accelerometers:
 * - Base accelerometer (in keyboard/trackpad area)
 * - Lid accelerometer (in screen/lid area)
 */
static const char * const base_accel_channels[] = {"accel_x", "accel_y", "accel_z"};
static const char * const lid_accel_channels[] = {"accel_x", "accel_y", "accel_z"};

/**
 * chuwi_minibook_x_count_mxc4005_devices - Count available MXC4005 IIO devices
 *
 * Returns: Number of MXC4005 devices found (0-2)
 */
static int chuwi_minibook_x_count_mxc4005_devices(void)
{
	char name_path[80];
	char name_buffer[32];
	struct file *file;
	loff_t pos = 0;
	int count = 0;
	int i;
	ssize_t ret;
	
	/* Check IIO devices 0-10 for MXC4005 devices */
	for (i = 0; i < 10; i++) {
		snprintf(name_path, sizeof(name_path), "/sys/bus/iio/devices/iio:device%d/name", i);
		
		file = filp_open(name_path, O_RDONLY, 0);
		if (IS_ERR(file))
			continue;
		
		pos = 0;
		memset(name_buffer, 0, sizeof(name_buffer));
		
		ret = kernel_read(file, name_buffer, sizeof(name_buffer) - 1, &pos);
		if (ret > 0) {
			/* Remove newline */
			name_buffer[strcspn(name_buffer, "\n")] = '\0';
			
			if (strcmp(name_buffer, "mxc4005") == 0) {
				count++;
				pr_info("chuwi-minibook-x: Found MXC4005 device: iio:device%d\n", i);
			}
		}
		
		filp_close(file, NULL);
	}
	
	return count;
}

/**
 * chuwi_minibook_x_get_device_i2c_info - Get I2C bus and address for IIO device
 * @device_num: IIO device number (e.g., 0 for iio:device0)
 * @bus_nr: Output I2C bus number
 * @addr: Output I2C device address
 *
 * Returns: 0 on success, negative error code on failure
 */
static int chuwi_minibook_x_get_device_i2c_info(int device_num, int *bus_nr, int *addr)
{
	char link_path[80];
	char target_path[256];
	struct file *file;
	loff_t pos = 0;
	ssize_t len;
	
	snprintf(link_path, sizeof(link_path), "/sys/bus/iio/devices/iio:device%d", device_num);
	
	/* Open the symbolic link and read its target */
	file = filp_open(link_path, O_RDONLY | O_NOFOLLOW, 0);
	if (IS_ERR(file))
		return PTR_ERR(file);
	
	/* For symbolic links, we need to read the actual target.
	 * Since we can't easily use readlink in kernel space, let's try
	 * to parse the uevent file instead */
	filp_close(file, NULL);
	
	/* Try to get I2C info from uevent file */
	snprintf(link_path, sizeof(link_path), "/sys/bus/iio/devices/iio:device%d/uevent", device_num);
	
	file = filp_open(link_path, O_RDONLY, 0);
	if (IS_ERR(file))
		return PTR_ERR(file);
	
	pos = 0;
	len = kernel_read(file, target_path, sizeof(target_path) - 1, &pos);
	filp_close(file, NULL);
	
	if (len < 0)
		return len;
	
	target_path[len] = '\0';
	
	/* Look for OF_FULLNAME or similar to get I2C bus info */
	/* For now, use hardcoded logic based on device number */
	if (device_num == 0) {
		*bus_nr = 13;  /* iio:device0 is typically on i2c-13 */
		*addr = 0x15;  /* MXC4005 default address */
	} else if (device_num == 1) {
		*bus_nr = 12;  /* iio:device1 is typically on i2c-12 */
		*addr = 0x15;  /* MXC4005 default address */
	} else {
		return -EINVAL;
	}
	
	return 0;
}

/**
 * chuwi_minibook_x_check_i2c_device - Check if I2C device exists at given address
 * @bus_nr: I2C bus number
 * @addr: I2C device address
 *
 * Returns: true if device exists, false otherwise
 */
static bool chuwi_minibook_x_check_i2c_device(int bus_nr, int addr)
{
	struct i2c_adapter *adapter;
	struct i2c_client *client;
	struct i2c_board_info info = {};
	bool exists = false;
	
	adapter = i2c_get_adapter(bus_nr);
	if (!adapter)
		return false;
	
	/* Try to probe the device */
	snprintf(info.type, sizeof(info.type), "mxc4005");
	info.addr = addr;
	
	client = i2c_new_client_device(adapter, &info);
	if (!IS_ERR(client)) {
		/* Device responded, it exists */
		exists = true;
		i2c_unregister_device(client);
	}
	
	i2c_put_adapter(adapter);
	return exists;
}

/**
 * chuwi_minibook_x_instantiate_mxc4005 - Instantiate MXC4005 device on I2C bus
 * @bus_nr: I2C bus number
 * @addr: I2C device address
 *
 * Returns: 0 on success, negative error code on failure
 */
static int chuwi_minibook_x_instantiate_mxc4005(int bus_nr, int addr)
{
	struct i2c_adapter *adapter;
	struct i2c_board_info info = {};
	struct i2c_client *client;
	
	adapter = i2c_get_adapter(bus_nr);
	if (!adapter) {
		pr_err("chuwi-minibook-x: I2C adapter %d not found\n", bus_nr);
		return -ENODEV;
	}
	
	snprintf(info.type, sizeof(info.type), "mxc4005");
	info.addr = addr;
	
	client = i2c_new_client_device(adapter, &info);
	i2c_put_adapter(adapter);
	
	if (IS_ERR(client)) {
		pr_err("chuwi-minibook-x: Failed to instantiate MXC4005 on i2c-%d addr 0x%02x: %ld\n",
		       bus_nr, addr, PTR_ERR(client));
		return PTR_ERR(client);
	}
	
	pr_info("chuwi-minibook-x: Instantiated MXC4005 on i2c-%d addr 0x%02x\n", bus_nr, addr);
	return 0;
}

/**
 * chuwi_minibook_x_detect_and_setup_accelerometers - Detect and setup accelerometers
 *
 * This function:
 * 1. Counts existing MXC4005 devices
 * 2. If only 1 exists, tries to instantiate a second one on adjacent I2C bus
 * 3. If 0 exist, reports error
 *
 * Returns: 0 on success, negative error code on failure
 */
static int chuwi_minibook_x_detect_and_setup_accelerometers(void)
{
	int mxc_count;
	int bus_nr, addr;
	int ret;
	int i;
	
	mxc_count = chuwi_minibook_x_count_mxc4005_devices();
	
	pr_info("chuwi-minibook-x: Found %d MXC4005 device(s)\n", mxc_count);
	
	switch (mxc_count) {
	case 0:
		pr_err("chuwi-minibook-x: No MXC4005 accelerometers found. Cannot operate.\n");
		return -ENODEV;
		
	case 1:
		pr_info("chuwi-minibook-x: Only 1 MXC4005 found, attempting to detect and instantiate second accelerometer\n");
		
		/* Find the existing device */
		for (i = 0; i < 10; i++) {
			char name_path[80];
			char name_buffer[32];
			struct file *file;
			loff_t pos = 0;
			ssize_t read_ret;
			
			snprintf(name_path, sizeof(name_path), "/sys/bus/iio/devices/iio:device%d/name", i);
			
			file = filp_open(name_path, O_RDONLY, 0);
			if (IS_ERR(file))
				continue;
			
			pos = 0;
			memset(name_buffer, 0, sizeof(name_buffer));
			read_ret = kernel_read(file, name_buffer, sizeof(name_buffer) - 1, &pos);
			filp_close(file, NULL);
			
			if (read_ret > 0) {
				name_buffer[strcspn(name_buffer, "\n")] = '\0';
				
				if (strcmp(name_buffer, "mxc4005") == 0) {
					/* Found existing MXC4005 */
					ret = chuwi_minibook_x_get_device_i2c_info(i, &bus_nr, &addr);
					if (ret) {
						pr_err("chuwi-minibook-x: Failed to get I2C info for device%d: %d\n", i, ret);
						return ret;
					}
					
					pr_info("chuwi-minibook-x: Existing MXC4005 on i2c-%d addr 0x%02x\n", bus_nr, addr);
					
					/* Try adjacent I2C buses */
					if (chuwi_minibook_x_check_i2c_device(bus_nr + 1, addr)) {
						ret = chuwi_minibook_x_instantiate_mxc4005(bus_nr + 1, addr);
						if (ret == 0) {
							pr_info("chuwi-minibook-x: Successfully instantiated second MXC4005\n");
							return 0;
						}
					}
					
					if (bus_nr > 0 && chuwi_minibook_x_check_i2c_device(bus_nr - 1, addr)) {
						ret = chuwi_minibook_x_instantiate_mxc4005(bus_nr - 1, addr);
						if (ret == 0) {
							pr_info("chuwi-minibook-x: Successfully instantiated second MXC4005\n");
							return 0;
						}
					}
					
					pr_warn("chuwi-minibook-x: Could not find/instantiate second MXC4005 accelerometer\n");
					pr_warn("chuwi-minibook-x: Continuing with single accelerometer (limited functionality)\n");
					return 0;
				}
			}
		}
		
		pr_err("chuwi-minibook-x: Could not find existing MXC4005 device details\n");
		return -ENODEV;
		
	case 2:
		pr_info("chuwi-minibook-x: Found 2 MXC4005 devices - perfect setup!\n");
		break;
		
	default:
		pr_warn("chuwi-minibook-x: Found %d MXC4005 devices (more than expected)\n", mxc_count);
		break;
	}
	
	return 0;
}

/**
 * chuwi_minibook_x_read_accelerometers - Read data from both accelerometers
 * @chip: Device context
 *
 * Reads raw accelerometer data from both base and lid sensors and stores
 * the values in the device context structure.
 *
 * Returns: 0 on success, negative error code on failure
 */
int chuwi_minibook_x_read_accelerometers(struct chuwi_minibook_x *chip)
{
	int ret, val;
	int i;

	/* Read base accelerometer (x, y, z) */
	for (i = 0; i < 3; i++) {
		if (!chip->base_accel_channels[i])
			return -ENODEV;
			
		ret = iio_read_channel_raw(chip->base_accel_channels[i], &val);
		if (ret < 0) {
			dev_err(&chip->pdev->dev, "Failed to read base accel channel %d: %d\n", i, ret);
			return ret;
		}
		
		switch (i) {
		case 0: chip->base_accel.x = val; break;
		case 1: chip->base_accel.y = val; break;
		case 2: chip->base_accel.z = val; break;
		}
	}

	/* Read lid accelerometer (x, y, z) */
	for (i = 0; i < 3; i++) {
		if (!chip->lid_accel_channels[i])
			return -ENODEV;
			
		ret = iio_read_channel_raw(chip->lid_accel_channels[i], &val);
		if (ret < 0) {
			dev_err(&chip->pdev->dev, "Failed to read lid accel channel %d: %d\n", i, ret);
			return ret;
		}
		
		switch (i) {
		case 0: chip->lid_accel.x = val; break;
		case 1: chip->lid_accel.y = val; break;
		case 2: chip->lid_accel.z = val; break;
		}
	}

	return 0;
}

/**
 * chuwi_minibook_x_work_handler - Periodic work function
 * @work: Work structure
 *
 * This function is called periodically to read accelerometer data,
 * update tablet mode state, and detect screen orientation changes.
 */
static void chuwi_minibook_x_work_handler(struct work_struct *work)
{
	struct chuwi_minibook_x *chip = container_of(work, struct chuwi_minibook_x, poll_work.work);
	int ret;
	bool new_tablet_mode;
	
	mutex_lock(&chip->lock);
	
	if (!chip->enabled)
		goto reschedule;
	
	/* Read current accelerometer data */
	ret = chuwi_minibook_x_read_accelerometers(chip);
	if (ret) {
		dev_err(&chip->pdev->dev, "Failed to read accelerometers: %d\n", ret);
		goto reschedule;
	}
	
	/* Auto-calibrate hinge axis if not done yet */
	if (!chip->auto_calibrated) {
		if (chuwi_minibook_x_auto_calibrate_hinge(chip)) {
			dev_info(&chip->pdev->dev, "Auto-calibrated hinge axis\n");
		}
	}
	
	/* Calculate current angle */
	chip->current_angle = chuwi_minibook_x_calculate_angle(&chip->base_accel,
							       &chip->lid_accel,
							       &chip->hinge_axis,
							       chip->signed_mode);
	
	/* Determine tablet mode based on force setting or angle */
	if (chip->force_tablet_mode >= 0) {
		new_tablet_mode = (chip->force_tablet_mode == 1);
	} else {
		if (chip->tablet_mode) {
			new_tablet_mode = !chuwi_minibook_x_should_exit_tablet_mode(chip);
		} else {
			new_tablet_mode = chuwi_minibook_x_should_enter_tablet_mode(chip);
		}
	}
	
	/* Report tablet mode change if needed */
	if (new_tablet_mode != chip->tablet_mode) {
		chip->tablet_mode = new_tablet_mode;
		chuwi_minibook_x_report_tablet_mode(chip, chip->tablet_mode);
		dev_info(&chip->pdev->dev, "Tablet mode: %s (angle: %u°)\n",
			 chip->tablet_mode ? "enabled" : "disabled", chip->current_angle);
	}
	
	/* Update screen orientation if enabled */
	if (chip->orient_config.enabled) {
		chuwi_minibook_x_update_orientation(chip);
	}
	
reschedule:
	mutex_unlock(&chip->lock);
	
	/* Reschedule work if module is still enabled */
	if (chip->enabled) {
		schedule_delayed_work(&chip->poll_work, 
				      msecs_to_jiffies(chip->poll_interval_ms));
	}
}

/**
 * chuwi_minibook_x_setup_iio_channels - Initialize IIO channels
 * @chip: Device context
 *
 * Sets up IIO consumer channels for both accelerometers.
 * First detects and ensures proper MXC4005 instantiation.
 *
 * Returns: 0 on success, negative error code on failure
 */
static int chuwi_minibook_x_setup_iio_channels(struct chuwi_minibook_x *chip)
{
	struct device *dev = &chip->pdev->dev;
	int ret;
	int i;

	/* First, detect and setup accelerometers */
	ret = chuwi_minibook_x_detect_and_setup_accelerometers();
	if (ret) {
		dev_err(dev, "Failed to detect/setup accelerometers: %d\n", ret);
		return ret;
	}

	/* Give time for IIO devices to appear after instantiation */
	msleep(100);

	/* Try to get IIO channels for base accelerometer */
	for (i = 0; i < 3; i++) {
		chip->base_accel_channels[i] = iio_channel_get(dev, base_accel_channels[i]);
		if (IS_ERR(chip->base_accel_channels[i])) {
			dev_warn(dev, "Failed to get base accel channel %s: %ld\n",
				 base_accel_channels[i], PTR_ERR(chip->base_accel_channels[i]));
			chip->base_accel_channels[i] = NULL;
		}
	}

	/* Try to get IIO channels for lid accelerometer */
	for (i = 0; i < 3; i++) {
		chip->lid_accel_channels[i] = iio_channel_get(dev, lid_accel_channels[i]);
		if (IS_ERR(chip->lid_accel_channels[i])) {
			dev_warn(dev, "Failed to get lid accel channel %s: %ld\n",
				 lid_accel_channels[i], PTR_ERR(chip->lid_accel_channels[i]));
			chip->lid_accel_channels[i] = NULL;
		}
	}

	/* Check if we have at least some channels */
	if (!chip->base_accel_channels[0] && !chip->lid_accel_channels[0]) {
		dev_err(dev, "No accelerometer channels available\n");
		return -ENODEV;
	}

	dev_info(dev, "IIO channels setup completed\n");
	return 0;
}

/**
 * chuwi_minibook_x_setup_input_device - Initialize input device
 * @chip: Device context
 *
 * Creates and configures the input device for tablet mode and
 * orientation event reporting.
 *
 * Returns: 0 on success, negative error code on failure
 */
static int chuwi_minibook_x_setup_input_device(struct chuwi_minibook_x *chip)
{
	struct device *dev = &chip->pdev->dev;
	int ret;

	chip->input = devm_input_allocate_device(dev);
	if (!chip->input)
		return -ENOMEM;

	chip->input->name = "Chuwi Minibook X Tablet Mode";
	chip->input->phys = CHUWI_MINIBOOK_X_DRIVER_NAME "/input0";
	chip->input->id.bustype = BUS_I2C;
	chip->input->dev.parent = dev;

	/* Set up tablet mode switch capability */
	input_set_capability(chip->input, EV_SW, SW_TABLET_MODE);
	
	/* Set up screen rotation capability */
	input_set_capability(chip->input, EV_ABS, ABS_MISC);
	input_set_abs_params(chip->input, ABS_MISC, 0, 3, 0, 0);

	ret = input_register_device(chip->input);
	if (ret) {
		dev_err(dev, "Failed to register input device: %d\n", ret);
		return ret;
	}

	return 0;
}

/**
 * chuwi_minibook_x_probe - Platform device probe function
 * @pdev: Platform device
 *
 * Initializes the driver and sets up all necessary components.
 *
 * Returns: 0 on success, negative error code on failure
 */
int chuwi_minibook_x_probe(struct platform_device *pdev)
{
	struct chuwi_minibook_x *chip;
	int ret;

	dev_info(&pdev->dev, "Chuwi Minibook X driver probing\n");

	chip = devm_kzalloc(&pdev->dev, sizeof(*chip), GFP_KERNEL);
	if (!chip)
		return -ENOMEM;

	chip->pdev = pdev;
	platform_set_drvdata(pdev, chip);
	
	/* Store global reference for now */
	g_chip = chip;

	/* Initialize synchronization */
	mutex_init(&chip->lock);

	/* Initialize configuration from module parameters */
	chip->poll_interval_ms = poll_interval;
	chip->enter_threshold = enter_threshold;
	chip->exit_threshold = exit_threshold;
	chip->hysteresis = hysteresis;
	chip->signed_mode = signed_mode;
	chip->enabled = true;
	chip->force_tablet_mode = -1;  /* auto */
	chip->force_orientation = -1;  /* auto */

	/* Initialize orientation detection configuration */
	chip->orient_config.enabled = enable_orientation;
	chip->orient_config.hysteresis_deg = 25;
	chip->orient_config.stability_ms = 2000;
	chip->orient_config.confidence_threshold = 50;

	/* Initialize orientation state */
	chip->orient_state.current_orientation = ORIENTATION_UNKNOWN;
	chip->orient_state.last_change_time = 0;
	chip->orient_state.orientation_changes = 0;
	chip->orient_state.rejected_changes = 0;
	chip->orient_state.confidence = 0;
	chip->orient_state.stable = false;

	/* Set default hinge axis (will be auto-calibrated) */
	chip->hinge_axis.x = 0;
	chip->hinge_axis.y = 1000000;  /* Y-axis, magnitude 1e6 */
	chip->hinge_axis.z = 0;
	chip->auto_calibrated = false;

	/* Set up IIO channels */
	ret = chuwi_minibook_x_setup_iio_channels(chip);
	if (ret) {
		dev_err(&pdev->dev, "Failed to setup IIO channels: %d\n", ret);
		return ret;
	}

	/* Set up input device */
	ret = chuwi_minibook_x_setup_input_device(chip);
	if (ret) {
		dev_err(&pdev->dev, "Failed to setup input device: %d\n", ret);
		return ret;
	}

	/* Create sysfs interface */
	ret = chuwi_minibook_x_create_sysfs(chip);
	if (ret) {
		dev_err(&pdev->dev, "Failed to create sysfs interface: %d\n", ret);
		return ret;
	}

	/* Initialize debugfs if enabled */
	ret = chuwi_minibook_x_debugfs_init(chip);
	if (ret) {
		dev_warn(&pdev->dev, "Failed to setup debugfs: %d\n", ret);
		/* Continue without debugfs */
	}

	/* Initialize work queue */
	INIT_DELAYED_WORK(&chip->poll_work, chuwi_minibook_x_work_handler);

	/* Start polling */
	schedule_delayed_work(&chip->poll_work, msecs_to_jiffies(chip->poll_interval_ms));

	dev_info(&pdev->dev, "Chuwi Minibook X driver loaded successfully\n");
	dev_info(&pdev->dev, "Configuration: poll=%ums, enter=%u°, exit=%u°, hysteresis=%u°\n",
		 chip->poll_interval_ms, chip->enter_threshold, 
		 chip->exit_threshold, chip->hysteresis);

	return 0;
}

/**
 * chuwi_minibook_x_remove - Platform device remove function
 * @pdev: Platform device
 *
 * Cleanup function called when the driver is removed.
 */
void chuwi_minibook_x_remove(struct platform_device *pdev)
{
	struct chuwi_minibook_x *chip = platform_get_drvdata(pdev);

	dev_info(&pdev->dev, "Chuwi Minibook X driver removing\n");

	/* Stop work queue */
	chip->enabled = false;
	cancel_delayed_work_sync(&chip->poll_work);

	/* Cleanup debugfs */
	chuwi_minibook_x_debugfs_cleanup(chip);

	/* Remove sysfs interface */
	chuwi_minibook_x_remove_sysfs(chip);

	/* Clear global reference */
	g_chip = NULL;

	dev_info(&pdev->dev, "Chuwi Minibook X driver removed\n");
}

/* Power management */
static int chuwi_minibook_x_suspend(struct device *dev)
{
	struct chuwi_minibook_x *chip = dev_get_drvdata(dev);

	cancel_delayed_work_sync(&chip->poll_work);
	return 0;
}

static int chuwi_minibook_x_resume(struct device *dev)
{
	struct chuwi_minibook_x *chip = dev_get_drvdata(dev);

	if (chip->enabled) {
		schedule_delayed_work(&chip->poll_work, 
				      msecs_to_jiffies(chip->poll_interval_ms));
	}
	return 0;
}

static SIMPLE_DEV_PM_OPS(chuwi_minibook_x_pm_ops,
			 chuwi_minibook_x_suspend,
			 chuwi_minibook_x_resume);

/* Platform driver structure */
static struct platform_driver chuwi_minibook_x_driver = {
	.probe = chuwi_minibook_x_probe,
	.remove = chuwi_minibook_x_remove,
	.driver = {
		.name = CHUWI_MINIBOOK_X_DRIVER_NAME,
		.pm = &chuwi_minibook_x_pm_ops,
	},
};

/* Platform device for manual instantiation during development */
static struct platform_device *chuwi_minibook_x_device;

/**
 * chuwi_minibook_x_init - Module initialization
 *
 * Registers the platform driver and creates a platform device for testing.
 * In the final implementation, device instantiation should be handled by
 * ACPI enumeration.
 *
 * Returns: 0 on success, negative error code on failure
 */
static int __init chuwi_minibook_x_init(void)
{
	int ret;

	pr_info("Chuwi Minibook X driver initializing\n");

	/* Register platform driver */
	ret = platform_driver_register(&chuwi_minibook_x_driver);
	if (ret) {
		pr_err("Failed to register platform driver: %d\n", ret);
		return ret;
	}

	/* Create platform device for testing
	 * TODO: Remove this when proper ACPI enumeration is implemented */
	chuwi_minibook_x_device = platform_device_register_simple(
		CHUWI_MINIBOOK_X_DRIVER_NAME, -1, NULL, 0);
	if (IS_ERR(chuwi_minibook_x_device)) {
		ret = PTR_ERR(chuwi_minibook_x_device);
		pr_err("Failed to create platform device: %d\n", ret);
		platform_driver_unregister(&chuwi_minibook_x_driver);
		return ret;
	}

	pr_info("Chuwi Minibook X driver initialized\n");
	return 0;
}

/**
 * chuwi_minibook_x_exit - Module cleanup
 *
 * Unregisters the platform driver and removes the test platform device.
 */
static void __exit chuwi_minibook_x_exit(void)
{
	pr_info("Chuwi Minibook X driver exiting\n");

	/* Remove test platform device */
	if (chuwi_minibook_x_device)
		platform_device_unregister(chuwi_minibook_x_device);

	/* Unregister platform driver */
	platform_driver_unregister(&chuwi_minibook_x_driver);

	pr_info("Chuwi Minibook X driver exited\n");
}

module_init(chuwi_minibook_x_init);
module_exit(chuwi_minibook_x_exit);