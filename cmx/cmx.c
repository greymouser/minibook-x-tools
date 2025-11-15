// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * CMX - Chuwi Minibook X Platform Driver
 *
 * Copyright (C) 2025 Armando DiCianno
 *
 * Platform driver for tablet mode detection on the Chuwi Minibook X
 * convertible laptop. Provides SW_TABLET_MODE input events and sysfs
 * interface for userspace tablet mode daemon communication.
 *
 * This driver discovers IIO accelerometer devices created by the
 * serial-multi-instantiate driver and exposes their paths via sysfs
 * for use by the userspace daemon (cmxd) which performs the actual
 * accelerometer data processing and tablet mode detection.
 *
*/

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/sysfs.h>
#include <linux/mutex.h>
#include <linux/string.h>
#include <linux/dmi.h>
#include <linux/input.h>
#include <linux/kobject.h>

#define CMX_DRIVER_NAME "cmx"

/* Valid mode and orientation strings for validation */
static const char * const valid_modes[] = {
	"closing", "laptop", "flat", "tent", "tablet", NULL
};

static const char * const valid_orientations[] = {
	"portrait", "landscape", "portrait-flipped", "landscape-flipped", NULL
};

/* IIO device paths for accelerometers */
#define LID_IIO_DEVICE_PATH  "/sys/bus/iio/devices/iio:device0"
#define BASE_IIO_DEVICE_PATH "/sys/bus/iio/devices/iio:device1"

/**
 * struct cmx - Driver context structure
 * @pdev: Platform device
 * @base_iio_device: Base IIO device path for userspace daemon
 * @lid_iio_device: Lid IIO device path for userspace daemon
 * @lock: Synchronization mutex
 */
struct cmx {
	struct platform_device *pdev;
	char base_iio_device[64];
	char lid_iio_device[64];
	struct mutex lock;
};

/* Module information */
MODULE_DESCRIPTION("CMX - Chuwi Minibook X integrated platform driver");
MODULE_AUTHOR("Armando DiCianno <armando@noonshy.com>");
MODULE_LICENSE("GPL");

/* Version is defined at build time from VERSION file */
#ifndef MODULE_VERSION_STRING
#define MODULE_VERSION_STRING "5.0.0"
#endif
MODULE_VERSION(MODULE_VERSION_STRING);

MODULE_ALIAS("platform:" CMX_DRIVER_NAME);

/* Ensure serial_multi_instantiate loads before this driver */
MODULE_SOFTDEP("pre: serial_multi_instantiate");

/**
 * struct vec3 - 3D accelerometer vector in micro-g units
 * @x: X-axis acceleration in micro-g
 * @y: Y-axis acceleration in micro-g  
 * @z: Z-axis acceleration in micro-g
 *
 * Used for storing accelerometer data from userspace daemon.
 */
struct vec3 {
	s32 x, y, z;
};

/*
 * Global state variables for tablet mode detection.
 * These maintain current state for communication with userspace via sysfs.
 */

/* Input device for SW_TABLET_MODE events */
static struct input_dev *tm_input;

/* Protects global state variables */  
static DEFINE_MUTEX(tm_lock);

/* Current gravity vector from base accelerometer (micro-g) */
static struct vec3 g_base = { 0, 0, 1000000 };

/* Current gravity vector from lid accelerometer (micro-g) */
static struct vec3 g_lid = { 0, 0, -1000000 };

/* Current device mode string */
static char current_mode[16] = "laptop";

/* Current device orientation string */
static char current_orientation[32] = "landscape";

/* Enable/disable SW_TABLET_MODE input events */
static bool enable_events = true;

/* Global driver context */
static struct cmx *g_chip;

/* Driver state tracking for idempotent initialization */
static bool driver_registered = false;
static bool device_created = false;
static DEFINE_MUTEX(driver_init_lock);

/* DMI table for supported hardware */
static const struct dmi_system_id cmx_dmi_table[] = {
	{
		.ident = "CHUWI MiniBook X",
		.matches = {
			DMI_MATCH(DMI_SYS_VENDOR, "CHUWI Innovation And Technology(ShenZhen)co.,Ltd"),
			DMI_MATCH(DMI_PRODUCT_NAME, "MiniBook X"),
		},
		.driver_data = NULL,
	},
	{ }
};
MODULE_DEVICE_TABLE(dmi, cmx_dmi_table);

/*
 * is_valid_string - Check if string is in valid array
 * @str: String to validate
 * @valid_strings: NULL-terminated array of valid strings
 * Returns: true if valid, false otherwise
 */
static bool is_valid_string(const char *str, const char * const *valid_strings)
{
	int i;
	for (i = 0; valid_strings[i]; i++) {
		if (strcmp(str, valid_strings[i]) == 0)
			return true;
	}
	return false;
}

/*
 * parse_bool_string - Parse boolean value from string
 * @str: String to parse (modified to lowercase)
 * Returns: true/false if valid, -EINVAL if invalid
 */
static int parse_bool_string(char *str)
{
	/* Convert to lowercase */
	int i;
	for (i = 0; str[i]; i++) {
		if (str[i] >= 'A' && str[i] <= 'Z')
			str[i] = str[i] + ('a' - 'A');
	}
	
	/* Parse true values: 1,y,yes,t,true */
	if (strcmp(str, "1") == 0 || strcmp(str, "y") == 0 ||
	    strcmp(str, "yes") == 0 || strcmp(str, "t") == 0 ||
	    strcmp(str, "true") == 0)
		return 1;
	
	/* Parse false values: 0,n,no,f,false */
	if (strcmp(str, "0") == 0 || strcmp(str, "n") == 0 ||
	    strcmp(str, "no") == 0 || strcmp(str, "f") == 0 ||
	    strcmp(str, "false") == 0)
		return 0;
	
	return -EINVAL;
}

/*
 * notify_tablet_mode_change - Send SW_TABLET_MODE input event
 * @is_tablet: true if entering tablet mode, false if exiting
 *
 * Sends input event to notify desktop environments of tablet mode changes.
 * Only sends events if enable_events is true.
 */
static void notify_tablet_mode_change(bool is_tablet)
{
	if (!tm_input) {
		pr_debug(CMX_DRIVER_NAME ": Input device not available for tablet mode notification\n");
		return;
	}
	
	/* Check if events are enabled */
	if (!enable_events) {
		pr_debug(CMX_DRIVER_NAME ": Tablet mode events disabled, skipping notification\n");
		return;
	}
	
	input_report_switch(tm_input, SW_TABLET_MODE, is_tablet ? 1 : 0);
	input_sync(tm_input);
	
	pr_info(CMX_DRIVER_NAME ": Tablet mode %s\n", is_tablet ? "ENABLED" : "DISABLED");
}

/*
 * is_tablet_mode - Check if mode string represents tablet mode
 * @mode: Mode string to check  
 *
 * Returns: true if mode is "tablet" or "tent", false otherwise
 */
static bool is_tablet_mode(const char *mode)
{
	return (strcmp(mode, "tablet") == 0 || strcmp(mode, "tent") == 0);
}

static ssize_t show_base_vec(struct kobject *k, struct kobj_attribute *a, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%d %d %d\n", g_base.x, g_base.y, g_base.z);
}

static ssize_t store_base_vec(struct kobject *k, struct kobj_attribute *a,
			       const char *b, size_t l)
{
	struct vec3 v;
	int n;

	n = sscanf(b, "%d %d %d", &v.x, &v.y, &v.z);
	if (n != 3)
		return -EINVAL;

	mutex_lock(&tm_lock);
	g_base = v;
	mutex_unlock(&tm_lock);
	
	return l;
}

static ssize_t show_lid_vec(struct kobject *k, struct kobj_attribute *a, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%d %d %d\n", g_lid.x, g_lid.y, g_lid.z);
}

static ssize_t store_lid_vec(struct kobject *k, struct kobj_attribute *a,
			     const char *b, size_t l)
{
	struct vec3 v;
	int n;

	n = sscanf(b, "%d %d %d", &v.x, &v.y, &v.z);
	if (n != 3)
		return -EINVAL;

	mutex_lock(&tm_lock);
	g_lid = v;
	mutex_unlock(&tm_lock);
	
	return l;
}

/* show_mode - Show current device mode */
static ssize_t show_mode(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%s\n", current_mode);
}

/* store_mode - Set device mode and send input events */
static ssize_t store_mode(struct kobject *kobj, struct kobj_attribute *attr,
			  const char *buf, size_t len)
{
	char mode_str[16], old_mode[16];
	bool old_is_tablet, new_is_tablet;
	int ret;
	
	ret = sscanf(buf, "%15s", mode_str);
	if (ret != 1)
		return -EINVAL;
	
	if (!is_valid_string(mode_str, valid_modes))
		return -EINVAL;
	
	mutex_lock(&tm_lock);
	strcpy(old_mode, current_mode);
	strcpy(current_mode, mode_str);
	mutex_unlock(&tm_lock);
	
	/* Send input event only if tablet mode state changed */
	old_is_tablet = is_tablet_mode(old_mode);
	new_is_tablet = is_tablet_mode(mode_str);
	if (old_is_tablet != new_is_tablet)
		notify_tablet_mode_change(new_is_tablet);
	
	return len;
}

/* show_orientation - Show current device orientation */
static ssize_t show_orientation(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%s\n", current_orientation);
}

/* store_orientation - Set device orientation with validation */
static ssize_t store_orientation(struct kobject *kobj, struct kobj_attribute *attr,
				 const char *buf, size_t len)
{
	char orientation_str[32];
	int ret;
	
	ret = sscanf(buf, "%31s", orientation_str);
	if (ret != 1)
		return -EINVAL;
	
	if (!is_valid_string(orientation_str, valid_orientations))
		return -EINVAL;
	
	mutex_lock(&tm_lock);
	strcpy(current_orientation, orientation_str);
	mutex_unlock(&tm_lock);
	
	return len;
}

/* show_iio_base_device - Show base IIO device path */
static ssize_t show_iio_base_device(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	if (!g_chip || strlen(g_chip->base_iio_device) == 0) {
		return snprintf(buf, PAGE_SIZE, "not_detected\n");
	}
	return snprintf(buf, PAGE_SIZE, "%s\n", g_chip->base_iio_device);
}

/* show_iio_lid_device - Show lid IIO device path */
static ssize_t show_iio_lid_device(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	if (!g_chip || strlen(g_chip->lid_iio_device) == 0) {
		return snprintf(buf, PAGE_SIZE, "not_detected\n");
	}
	return snprintf(buf, PAGE_SIZE, "%s\n", g_chip->lid_iio_device);
}

/* show_enable - Show whether tablet mode events are enabled */
static ssize_t show_enable(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%s\n", enable_events ? "true" : "false");
}

/* store_enable - Enable or disable tablet mode events */
static ssize_t store_enable(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t len)
{
	char enable_str[8];
	int result;
	
	if (len >= sizeof(enable_str))
		return -EINVAL;
	
	strncpy(enable_str, buf, len);
	enable_str[len] = '\0';
	
	/* Remove trailing newline if present */
	if (len > 0 && enable_str[len - 1] == '\n')
		enable_str[len - 1] = '\0';
	
	result = parse_bool_string(enable_str);
	if (result < 0)
		return result;
	
	enable_events = result;
	pr_info(CMX_DRIVER_NAME ": Tablet mode events %s\n", result ? "enabled" : "disabled");
	
	return len;
}

/* Basic attribute definitions */
static struct kobj_attribute base_vec_attr = __ATTR(base_vec, 0644, show_base_vec, store_base_vec);
static struct kobj_attribute lid_vec_attr = __ATTR(lid_vec, 0644, show_lid_vec, store_lid_vec);
static struct kobj_attribute mode_attr = __ATTR(mode, 0644, show_mode, store_mode);
static struct kobj_attribute orientation_attr = __ATTR(orientation, 0644, show_orientation, store_orientation);
static struct kobj_attribute iio_base_device_attr = __ATTR(iio_base_device, 0444, show_iio_base_device, NULL);
static struct kobj_attribute iio_lid_device_attr = __ATTR(iio_lid_device, 0444, show_iio_lid_device, NULL);
static struct kobj_attribute enable_attr = __ATTR(enable, 0644, show_enable, store_enable);

static struct attribute *tablet_mode_attrs[] = {
	&base_vec_attr.attr,
	&lid_vec_attr.attr,
	&mode_attr.attr,
	&orientation_attr.attr,
	&iio_base_device_attr.attr,
	&iio_lid_device_attr.attr,
	&enable_attr.attr,
	NULL,
};

static struct attribute_group tablet_mode_attr_group = {
	.attrs = tablet_mode_attrs,
};

/*
 * cmx_discover_iio_devices - Find existing IIO accelerometer devices
 *
 * Stores paths to IIO devices created by serial_multi_instantiate for
 * use by userspace daemon. Device assignment based on creation order:
 * - iio:device0 = lid sensor (first device created)
 * - iio:device1 = base sensor (second device created)
 *
 * Returns: 0 on success, negative error code on failure
 */
static int cmx_discover_iio_devices(void)
{
	struct cmx *chip = g_chip;
	
	if (!chip) {
		pr_err(CMX_DRIVER_NAME ": No chip context available\n");
		return -EINVAL;
	}
	
	/* Devices are created by serial_multi_instantiate in order */
	strcpy(chip->lid_iio_device, LID_IIO_DEVICE_PATH);
	strcpy(chip->base_iio_device, BASE_IIO_DEVICE_PATH);
	
	pr_info(CMX_DRIVER_NAME ": IIO device assignments: lid=%s, base=%s\n",
		chip->lid_iio_device, chip->base_iio_device);
	
	return 0;
}

/*
 * cmx_init_tablet_mode - Initialize tablet mode detection
 * @pdev: Platform device for device registration
 *
 * Creates and registers input device for SW_TABLET_MODE events.
 * Creates sysfs attributes for userspace daemon communication.
 *
 * Returns: 0 on success, negative error code on failure
 */
static int cmx_init_tablet_mode(struct platform_device *pdev)
{
	int ret;
	
	pr_debug(CMX_DRIVER_NAME ": Initializing tablet mode detection\n");
	
	/* Create input device for tablet mode events */
	tm_input = devm_input_allocate_device(&pdev->dev);
	if (!tm_input) {
		pr_err(CMX_DRIVER_NAME ": Failed to allocate input device\n");
		return -ENOMEM;
	}
	
	tm_input->name = "Chuwi Minibook X input";
	tm_input->phys = "cmx/input0";
	tm_input->id.bustype = BUS_HOST;
	tm_input->dev.parent = &pdev->dev;
	
	/* Set up input capabilities */
	input_set_capability(tm_input, EV_SW, SW_TABLET_MODE);
	
	ret = input_register_device(tm_input);
	if (ret) {
		pr_err(CMX_DRIVER_NAME ": Failed to register input device: %d\n", ret);
		return ret;
	}
	
	/* Set initial tablet mode state to laptop mode (not in tablet mode) */
	input_report_switch(tm_input, SW_TABLET_MODE, 0);
	input_sync(tm_input);
	pr_debug(CMX_DRIVER_NAME ": Initial mode set to laptop (tablet mode disabled)\n");
	
	/* Create sysfs interface under platform driver */
	ret = sysfs_create_group(&pdev->dev.kobj, &tablet_mode_attr_group);
	if (ret) {
		pr_err(CMX_DRIVER_NAME ": Failed to create sysfs group: %d\n", ret);
		input_unregister_device(tm_input);
		return ret;
	}
	
	pr_debug(CMX_DRIVER_NAME ": Tablet mode detection initialized successfully\n");
	
	return 0;
}

/*
 * cmx_cleanup_tablet_mode - Clean up tablet mode resources
 *
 * Removes sysfs attribute group. Input device cleanup handled via devm.
 */
static void cmx_cleanup_tablet_mode(void)
{
	/* Remove sysfs interface */
	if (g_chip && g_chip->pdev) {
		sysfs_remove_group(&g_chip->pdev->dev.kobj, &tablet_mode_attr_group);
	}
	
	/* Input device will be cleaned up automatically via devm */
}

/*
 * cmx_probe - Platform device probe and initialization
 * @pdev: Platform device being probed
 *
 * Allocates driver context, discovers IIO devices, and initializes
 * tablet mode detection with input device and sysfs interface.
 *
 * Returns: 0 on success, negative error code on failure
 */
static int cmx_probe(struct platform_device *pdev)
{
	struct cmx *chip;
	int ret;

	dev_dbg(&pdev->dev, "Chuwi Minibook X integrated driver probing\n");

	chip = devm_kzalloc(&pdev->dev, sizeof(*chip), GFP_KERNEL);
	if (!chip)
		return -ENOMEM;

	chip->pdev = pdev;
	platform_set_drvdata(pdev, chip);
	g_chip = chip;
	mutex_init(&chip->lock);

	/* Discover existing IIO devices created by serial_multi_instantiate */
	ret = cmx_discover_iio_devices();
	if (ret) {
		dev_err(&pdev->dev, "Failed to setup accelerometers: %d\n", ret);
		goto err_cleanup;
	}

	/* Initialize tablet mode detection */
	ret = cmx_init_tablet_mode(pdev);
	if (ret) {
		dev_err(&pdev->dev, "Failed to initialize tablet mode detection: %d\n", ret);
		goto err_cleanup;
	}

	dev_info(&pdev->dev, "Tablet mode detection initialized\n");
	return 0;

err_cleanup:
	g_chip = NULL;
	return ret;
}

/*
 * cmx_remove - Platform device removal and cleanup
 * @pdev: Platform device being removed
 *
 * Cleans up tablet mode detection and clears global context.
 */
static void cmx_remove(struct platform_device *pdev)
{
	dev_dbg(&pdev->dev, "Chuwi Minibook X integrated driver removing\n");

	/* Cleanup tablet mode detection */
	cmx_cleanup_tablet_mode();

	/* Clear global reference */
	g_chip = NULL;

	dev_dbg(&pdev->dev, "Chuwi Minibook X integrated driver removed\n");
}

/* Platform driver structure */
static struct platform_driver cmx_driver = {
	.probe = cmx_probe,
	.remove = cmx_remove,
	.driver = {
		.name = CMX_DRIVER_NAME,
	},
};

/* Platform device for manual instantiation */
static struct platform_device *cmx_device;

/*
 * cmx_init - Module initialization
 *
 * Checks DMI table for hardware compatibility, registers platform driver,
 * and creates platform device for supported hardware.
 *
 * Returns: 0 on success, negative error code on failure
 */
static int __init cmx_init(void)
{
	int ret;
	bool hardware_supported;

	pr_debug(CMX_DRIVER_NAME " integrated driver initializing\n");

	/* Use mutex to prevent race conditions during initialization */
	mutex_lock(&driver_init_lock);

	/* Check if driver is already registered */
	if (driver_registered) {
		pr_debug("Driver already registered, skipping duplicate initialization\n");
		mutex_unlock(&driver_init_lock);
		return 0;
	}

	/* Check if we're running on supported hardware via DMI table */
	hardware_supported = dmi_first_match(cmx_dmi_table) != NULL;
	if (!hardware_supported) {
		pr_warn("Hardware not in DMI support table, loading anyway for testing\n");
	}

	/* Register platform driver */
	ret = platform_driver_register(&cmx_driver);
	if (ret == -EBUSY || ret == -EEXIST) {
		pr_debug("Platform driver already registered, initialization successful\n");
		mutex_unlock(&driver_init_lock);
		return 0;
	} else if (ret) {
		pr_err("Failed to register platform driver: %d\n", ret);
		mutex_unlock(&driver_init_lock);
		return ret;
	}
	driver_registered = true;

	/* Create platform device manually based on DMI detection */
	if (hardware_supported && !device_created) {
		pr_debug("Creating platform device based on DMI detection\n");
		cmx_device = platform_device_register_simple(
			CMX_DRIVER_NAME, -1, NULL, 0);
		if (IS_ERR(cmx_device)) {
			ret = PTR_ERR(cmx_device);
			pr_err("Failed to create platform device: %d\n", ret);
			platform_driver_unregister(&cmx_driver);
			driver_registered = false;
			mutex_unlock(&driver_init_lock);
			return ret;
		}
		device_created = true;
		pr_debug("Platform device created successfully\n");
	} else if (!hardware_supported) {
		pr_debug("Hardware not supported, platform driver registered but no device created\n");
		cmx_device = NULL;
	}

	mutex_unlock(&driver_init_lock);
	pr_debug(CMX_DRIVER_NAME " integrated driver initialized successfully\n");
	return 0;
}

/*
 * cmx_exit - Module cleanup
 *
 * Unregisters platform device and driver.
 */
static void __exit cmx_exit(void)
{
	pr_debug(CMX_DRIVER_NAME " integrated driver exiting\n");

	mutex_lock(&driver_init_lock);

	/* Remove manual platform device if we created one */
	if (cmx_device && device_created) {
		platform_device_unregister(cmx_device);
		cmx_device = NULL;
		device_created = false;
	}

	/* Unregister platform driver if we registered it */
	if (driver_registered) {
		platform_driver_unregister(&cmx_driver);
		driver_registered = false;
	}

	mutex_unlock(&driver_init_lock);
	pr_debug(CMX_DRIVER_NAME " integrated driver exited\n");
}

module_init(cmx_init);
module_exit(cmx_exit);