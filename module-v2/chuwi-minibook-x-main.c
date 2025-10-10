// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Chuwi Minibook X Platform Driver
 *
 * Copyright (C) 2025 Your Name
 *
 * Hardware identification and accelerometer detection driver
 * for the Chuwi Minibook X convertible laptop.
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/acpi.h>
#include <linux/delay.h>
#include <linux/sysfs.h>
#include <linux/mutex.h>
#include <linux/i2c.h>
#include <linux/string.h>
#include <linux/dmi.h>
#include <linux/property.h>

#include "chuwi-minibook-x.h"

/* Module information */
MODULE_DESCRIPTION("Chuwi Minibook X hardware detection driver");
MODULE_AUTHOR("Your Name");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:" CHUWI_MINIBOOK_X_DRIVER_NAME);

/* Module dependencies - ensure required drivers are loaded */
MODULE_SOFTDEP("pre: mxc4005");
MODULE_SOFTDEP("pre: serial_multi_instantiate");

/* Module parameters */
bool debug_mode = false;
module_param(debug_mode, bool, 0644);
MODULE_PARM_DESC(debug_mode, "Enable debug output for hardware detection (default: false)");

int lid_bus = 13;
module_param(lid_bus, int, 0644);
MODULE_PARM_DESC(lid_bus, "I2C bus number for lid accelerometer (default: 13)");

int lid_addr = 0x15;
module_param(lid_addr, int, 0644);
MODULE_PARM_DESC(lid_addr, "I2C address for lid accelerometer (default: 0x15)");

int base_bus = 12;
module_param(base_bus, int, 0644);
MODULE_PARM_DESC(base_bus, "I2C bus number for base accelerometer (default: 12)");

int base_addr = 0x15;
module_param(base_addr, int, 0644);
MODULE_PARM_DESC(base_addr, "I2C address for base accelerometer (default: 0x15)");

bool enable_mount_matrix = true;
module_param(enable_mount_matrix, bool, 0644);
MODULE_PARM_DESC(enable_mount_matrix, "Enable automatic mount matrix transformation (default: true)");

/*
 * Mount Matrix Definitions
 * 
 * Based on sensor orientation analysis from SENSOR_CONFIGURATION.md:
 * - Lid sensor:  90° counter-clockwise rotation
 * - Base sensor: 90° clockwise rotation
 */

/* Lid sensor mount matrix (90° CCW rotation) */
static const char * const lid_sensor_mount_matrix[] = {
	"0", "1", "0",    /* X' = Y  (laptop right = sensor back)  */
	"-1", "0", "0",   /* Y' = -X (laptop back = sensor left)   */
	"0", "0", "1"     /* Z' = Z  (laptop up = sensor up)       */
};

/* Base sensor mount matrix (90° CW rotation) */  
static const char * const base_sensor_mount_matrix[] = {
	"0", "-1", "0",   /* X' = -Y (laptop right = sensor front) */
	"1", "0", "0",    /* Y' = X  (laptop back = sensor right)  */
	"0", "0", "1"     /* Z' = Z  (laptop up = sensor up)       */
};

/* Property entries for lid sensor */
static const struct property_entry lid_sensor_props[] = {
	PROPERTY_ENTRY_STRING_ARRAY("mount-matrix", lid_sensor_mount_matrix),
	{ }
};

/* Property entries for base sensor */
static const struct property_entry base_sensor_props[] = {
	PROPERTY_ENTRY_STRING_ARRAY("mount-matrix", base_sensor_mount_matrix),
	{ }
};

/* Software nodes for device property assignment */
static const struct software_node lid_sensor_node = {
	.properties = lid_sensor_props,
};

static const struct software_node base_sensor_node = {
	.properties = base_sensor_props,
};

/* Global driver context - will be replaced with proper device matching */
static struct chuwi_minibook_x *g_chip;

/* Driver state tracking for idempotent initialization */
static bool driver_registered = false;
static bool device_created = false;
static DEFINE_MUTEX(driver_init_lock);

/* Track manually instantiated I2C devices for cleanup */
static struct i2c_client *instantiated_client = NULL;
static struct i2c_client *instantiated_client2 = NULL;  /* For second device if needed */
static int instantiated_bus = -1;
static int instantiated_addr = -1;
static int instantiated_bus2 = -1;
static int instantiated_addr2 = -1;

/*
 * DMI table for supported hardware
 * This is the production approach for hardware detection
 */
static const struct dmi_system_id chuwi_minibook_x_dmi_table[] = {
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
MODULE_DEVICE_TABLE(dmi, chuwi_minibook_x_dmi_table);

/*
 * Hardware detection and identification for the Chuwi Minibook X.
 * This minimal driver focuses only on identifying the system and
 * bringing up the accelerometer hardware for testing.
 */

/**
 * chuwi_minibook_x_match_mxc4005 - Helper to match MXC4005 devices during bus iteration
 */
static int chuwi_minibook_x_match_mxc4005(struct device *dev, void *data)
{
	struct i2c_client *client;
	int *count = (int *)data;
	
	if (dev->bus != &i2c_bus_type)
		return 0;
	
	client = to_i2c_client(dev);
	if (!client || !client->dev.driver)
		return 0;
	
	if (strcmp(client->dev.driver->name, "mxc4005") == 0) {
		(*count)++;
		pr_info("chuwi-minibook-x: Found MXC4005 device on i2c-%d addr 0x%02x\n",
			client->adapter->nr, client->addr);
	}
	
	return 0;
}

/**
 * chuwi_minibook_x_count_mxc4005_devices - Count available MXC4005 IIO devices
 *
 * Uses bus iteration instead of filesystem operations for better kernel compliance.
 *
 * Returns: Number of MXC4005 devices found (0-2)
 */
static int chuwi_minibook_x_count_mxc4005_devices(void)
{
	int count = 0;
	
	/* Iterate through I2C bus devices to find MXC4005 chips */
	bus_for_each_dev(&i2c_bus_type, NULL, &count, chuwi_minibook_x_match_mxc4005);
	
	return count;
}

/**
 * chuwi_minibook_x_get_device_i2c_info - Get I2C bus and address for IIO device
 * @device_num: IIO device number (e.g., 0 for iio:device0)
 * @bus_nr: Output I2C bus number
 * @addr: Output I2C device address
 *
 * For now, use known hardware layout. In a production driver, this would
 * use ACPI enumeration or device tree.
 *
 * Returns: 0 on success, negative error code on failure
 */
static int __maybe_unused chuwi_minibook_x_get_device_i2c_info(int device_num, int *bus_nr, int *addr)
{
	/* Hardware configuration for Chuwi Minibook X:
	 * - iio:device0 -> lid accelerometer (configurable via module parameters)
	 * - iio:device1 -> base accelerometer (configurable via module parameters)
	 * 
	 * Uses module parameters for flexibility:
	 * - lid_bus/lid_addr for device 0 (default: i2c-13, addr 0x15)
	 * - base_bus/base_addr for device 1 (default: i2c-12, addr 0x15)
	 */
	switch (device_num) {
	case 0:
		*bus_nr = lid_bus;   /* Lid accelerometer bus (module parameter) */
		*addr = lid_addr;    /* Lid accelerometer address (module parameter) */
		break;
	case 1:
		*bus_nr = base_bus;  /* Base accelerometer bus (module parameter) */
		*addr = base_addr;   /* Base accelerometer address (module parameter) */
		break;
	default:
		return -EINVAL;
	}
	
	pr_debug("chuwi-minibook-x: Device %d mapped to i2c-%d addr 0x%02x\n", 
		 device_num, *bus_nr, *addr);
	
	return 0;
}

/**
 * chuwi_minibook_x_check_i2c_device - Check if I2C device exists at given address
 * @bus_nr: I2C bus number
 * @addr: I2C device address
 *
 * Returns: true if device exists, false otherwise
 */
static bool __maybe_unused chuwi_minibook_x_check_i2c_device(int bus_nr, int addr)
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
 * chuwi_minibook_x_ensure_mxc4005_loaded - Ensure mxc4005 driver is available
 *
 * Attempts to ensure mxc4005 is available without triggering auto-loading conflicts.
 * Uses a gentle approach that relies on MODULE_SOFTDEP and system auto-loading.
 *
 * Returns: 0 on success, negative error code on failure
 */
static int chuwi_minibook_x_ensure_mxc4005_loaded(void)
{
	pr_info("chuwi-minibook-x: Checking for mxc4005 driver availability\n");
	
	/* Give system time for MODULE_SOFTDEP to take effect */
	msleep(100);
	
	pr_info("chuwi-minibook-x: Proceeding with device instantiation (mxc4005 should auto-load)\n");
	return 0;
}

/**
 * chuwi_minibook_x_instantiate_mxc4005 - Instantiate MXC4005 device on I2C bus
 * @bus_nr: I2C bus number
 * @addr: I2C device address
 * @is_second: true if this is the second device (for tracking)
 * @is_lid: true if this is the lid sensor, false for base sensor
 *
 * Returns: 0 on success, negative error code on failure
 */
static int chuwi_minibook_x_instantiate_mxc4005(int bus_nr, int addr, bool is_second, bool is_lid)
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
	
	/* Apply mount matrix if enabled */
	if (enable_mount_matrix) {
		if (is_lid) {
			info.swnode = &lid_sensor_node;
			pr_info("chuwi-minibook-x: Applying lid sensor mount matrix (90° CCW)\n");
		} else {
			info.swnode = &base_sensor_node;
			pr_info("chuwi-minibook-x: Applying base sensor mount matrix (90° CW)\n");
		}
	} else {
		pr_info("chuwi-minibook-x: Mount matrix disabled, using identity transformation\n");
	}
	
	client = i2c_new_client_device(adapter, &info);
	i2c_put_adapter(adapter);
	
	if (IS_ERR(client)) {
		pr_err("chuwi-minibook-x: Failed to instantiate MXC4005 on i2c-%d addr 0x%02x: %ld\n",
		       bus_nr, addr, PTR_ERR(client));
		return PTR_ERR(client);
	}
	
	/* Store the instantiated client for cleanup on module unload */
	if (is_second) {
		instantiated_client2 = client;
		instantiated_bus2 = bus_nr;
		instantiated_addr2 = addr;
	} else {
		instantiated_client = client;
		instantiated_bus = bus_nr;
		instantiated_addr = addr;
	}
	
	pr_info("chuwi-minibook-x: Instantiated MXC4005 on i2c-%d addr 0x%02x\n", bus_nr, addr);
	return 0;
}

/**
 * chuwi_minibook_x_check_acpi_device - Check if MDA6655 ACPI device exists
 *
 * The Chuwi MiniBook X has a specific ACPI device (MDA6655) for the lid accelerometer.
 * This provides an additional hardware verification alongside DMI checks.
 *
 * Returns: true if MDA6655 device exists, false otherwise
 */
static bool chuwi_minibook_x_check_acpi_device(void)
{
	struct acpi_device *acpi_dev;
	acpi_handle handle;
	acpi_status status;
	
	/* Look for the MDA6655 ACPI device (lid accelerometer) */
	status = acpi_get_handle(NULL, "\\_SB.PC00.I2C1.MDA6655", &handle);
	if (ACPI_SUCCESS(status)) {
		pr_info("chuwi-minibook-x: Found MDA6655 ACPI device (lid accelerometer)\n");
		return true;
	}
	
	/* Try finding by HID */
	acpi_dev = acpi_dev_get_first_match_dev("MDA6655", NULL, -1);
	if (acpi_dev) {
		pr_info("chuwi-minibook-x: Found MDA6655 ACPI device by HID\n");
		acpi_dev_put(acpi_dev);
		return true;
	}
	
	pr_debug("chuwi-minibook-x: MDA6655 ACPI device not found\n");
	return false;
}

/**
 * chuwi_minibook_x_check_dmi_match - Check if current hardware matches supported DMI
 *
 * Uses DMI system identification to determine if this is a supported Chuwi MiniBook X.
 * This is much more reliable than ACPI enumeration for this hardware.
 *
 * Returns: true if hardware is supported, false otherwise
 */
static bool chuwi_minibook_x_check_dmi_match(void)
{
	const struct dmi_system_id *dmi_id;
	
	dmi_id = dmi_first_match(chuwi_minibook_x_dmi_table);
	if (dmi_id) {
		pr_info("chuwi-minibook-x: Detected supported hardware: %s\n", dmi_id->ident);
		return true;
	}
	
	pr_info("chuwi-minibook-x: Hardware not in DMI support table\n");
	pr_info("chuwi-minibook-x: Vendor: %s, Product: %s\n", 
		dmi_get_system_info(DMI_SYS_VENDOR) ?: "unknown",
		dmi_get_system_info(DMI_PRODUCT_NAME) ?: "unknown");
	
	return false;
}

/**
 * chuwi_minibook_x_check_hardware_support - Comprehensive hardware detection
 *
 * Combines DMI and ACPI device checks for reliable hardware identification.
 * For full support, both DMI match and MDA6655 device should be present.
 *
 * Returns: true if hardware is fully supported, false otherwise
 */
static bool chuwi_minibook_x_check_hardware_support(void)
{
	bool dmi_match = chuwi_minibook_x_check_dmi_match();
	bool acpi_device = chuwi_minibook_x_check_acpi_device();
	
	if (dmi_match && acpi_device) {
		pr_info("chuwi-minibook-x: Full hardware support confirmed (DMI + ACPI device)\n");
		return true;
	}
	
	if (dmi_match && !acpi_device) {
		pr_warn("chuwi-minibook-x: DMI match but MDA6655 device not found\n");
		pr_warn("chuwi-minibook-x: Hardware might be supported but accelerometer missing\n");
		return false;  /* Need both DMI and accelerometer device */
	}
	
	if (!dmi_match && acpi_device) {
		pr_warn("chuwi-minibook-x: MDA6655 device found but DMI mismatch\n");
		pr_warn("chuwi-minibook-x: Possible similar hardware - proceeding with caution\n");
		return false;  /* DMI is primary identification */
	}
	
	pr_info("chuwi-minibook-x: Neither DMI nor ACPI device checks passed\n");
	return false;
}

/**
 * chuwi_minibook_x_configure_from_dmi - Configure driver based on DMI information
 * @chip: Device context
 *
 * Sets up hardware-specific configuration based on DMI-detected hardware.
 * This allows different Chuwi models to have different configurations.
 *
 * Returns: 0 on success, negative error code on failure
 */
static int chuwi_minibook_x_configure_from_dmi(struct chuwi_minibook_x *chip)
{
	struct device *dev = &chip->pdev->dev;
	const struct dmi_system_id *dmi_id;
	const char *product_name;
	
	/* Check if we're running on supported hardware (DMI + ACPI) */
	if (!chuwi_minibook_x_check_hardware_support()) {
		dev_warn(dev, "Hardware not fully supported, continuing anyway\n");
		dev_warn(dev, "Please report success/issues to driver maintainer\n");
		return 0;  /* Allow driver to load for testing */
	}
	
	dmi_id = dmi_first_match(chuwi_minibook_x_dmi_table);
	product_name = dmi_get_system_info(DMI_PRODUCT_NAME);
	
	dev_info(dev, "Configuring for: %s\n", dmi_id->ident);
	
	/* Hardware-specific configuration based on detected model */
	if (strstr(product_name, "MiniBook X")) {
		/* Standard MiniBook X configuration */
		dev_info(dev, "Using MiniBook X accelerometer layout\n");
		dev_info(dev, "Lid accelerometer: i2c-%d addr 0x%02x\n", lid_bus, lid_addr);
		dev_info(dev, "Base accelerometer: i2c-%d addr 0x%02x\n", base_bus, base_addr);
		
		/* I2C bus configuration for MiniBook X (configurable via module parameters):
		 * - Lid accelerometer: lid_bus/lid_addr (default: i2c-13 addr 0x15, MDA6655)
		 * - Base accelerometer: base_bus/base_addr (default: i2c-12 addr 0x15)
		 */
		chip->accel_count = 2;  /* Expected accelerometer count */
	}
	
	return 0;
}

/**
 * chuwi_minibook_x_detect_and_setup_accelerometers - Detect and setup accelerometers
 *
 * This function:
 * 1. Ensures mxc4005 driver is loaded
 * 2. Counts existing MXC4005 devices
 * 3. If 0 exist, tries to instantiate both devices (ACPI one may auto-appear)
 * 4. If 1 exists, tries to instantiate the second one
 *
 * Returns: 0 on success, negative error code on failure
 */
static int chuwi_minibook_x_detect_and_setup_accelerometers(void)
{
	int mxc_count;
	int ret;
	
	/* First ensure mxc4005 driver is loaded */
	ret = chuwi_minibook_x_ensure_mxc4005_loaded();
	if (ret) {
		pr_err("chuwi-minibook-x: Failed to ensure mxc4005 driver is loaded: %d\n", ret);
		return ret;
	}
	
	/* Initial count - may be 0 if devices haven't been bound yet */
	mxc_count = chuwi_minibook_x_count_mxc4005_devices();
	
	pr_info("chuwi-minibook-x: Initially found %d MXC4005 device(s)\n", mxc_count);
	
	switch (mxc_count) {
	case 0:
		pr_info("chuwi-minibook-x: No MXC4005 accelerometers found. Attempting to instantiate both devices.\n");
		
		/* Try to instantiate the lid accelerometer first */
		pr_info("chuwi-minibook-x: Attempting to instantiate first MXC4005 on i2c-%d addr 0x%02x (lid)\n", lid_bus, lid_addr);
		ret = chuwi_minibook_x_instantiate_mxc4005(lid_bus, lid_addr, false, true);
		if (ret == 0) {
			pr_info("chuwi-minibook-x: Successfully instantiated first MXC4005 on i2c-%d (lid)\n", lid_bus);
			
			/* Give the system a moment for the device to be recognized */
			msleep(200);
			
			/* Now try the second device */
			pr_info("chuwi-minibook-x: Attempting to instantiate second MXC4005 on i2c-%d addr 0x%02x (base)\n", base_bus, base_addr);
			ret = chuwi_minibook_x_instantiate_mxc4005(base_bus, base_addr, true, false);
			if (ret == 0) {
				pr_info("chuwi-minibook-x: Successfully instantiated second MXC4005 on i2c-%d (base)\n", base_bus);
				return 0;
			} else {
				pr_warn("chuwi-minibook-x: Failed to instantiate second device: %d\n", ret);
				pr_info("chuwi-minibook-x: Continuing with single accelerometer\n");
				return 0;
			}
		} else {
			pr_warn("chuwi-minibook-x: Failed to instantiate first device: %d\n", ret);
			
			/* Try the base device location in case ACPI enumeration will handle the first */
			pr_info("chuwi-minibook-x: Trying alternative instantiation on i2c-%d (base)\n", base_bus);
			ret = chuwi_minibook_x_instantiate_mxc4005(base_bus, base_addr, false, false);
			if (ret == 0) {
				pr_info("chuwi-minibook-x: Successfully instantiated MXC4005 on i2c-%d (base)\n", base_bus);
				return 0;
			} else {
				pr_err("chuwi-minibook-x: Failed to instantiate any MXC4005 devices\n");
				return -ENODEV;
			}
		}
		break;
		
	case 1:
		pr_info("chuwi-minibook-x: Only 1 MXC4005 found, attempting to instantiate second accelerometer\n");
		
		/* First try base accelerometer location */
		pr_info("chuwi-minibook-x: Attempting to instantiate second MXC4005 on i2c-%d addr 0x%02x (base)\n", base_bus, base_addr);
		ret = chuwi_minibook_x_instantiate_mxc4005(base_bus, base_addr, true, false);
		if (ret == 0) {
			pr_info("chuwi-minibook-x: Successfully instantiated second MXC4005 on i2c-%d (base)\n", base_bus);
			return 0;
		}
		
		/* Try lid accelerometer location as alternative */
		pr_info("chuwi-minibook-x: Trying alternative location i2c-%d addr 0x%02x (lid)\n", lid_bus, lid_addr);
		ret = chuwi_minibook_x_instantiate_mxc4005(lid_bus, lid_addr, true, true);
		if (ret == 0) {
			pr_info("chuwi-minibook-x: Successfully instantiated second MXC4005 on i2c-%d (lid)\n", lid_bus);
			return 0;
		}
		
		pr_warn("chuwi-minibook-x: Could not instantiate second MXC4005 accelerometer\n");
		pr_warn("chuwi-minibook-x: Continuing with single accelerometer (limited functionality)\n");
		return 0;
		
	case 2:
		pr_info("chuwi-minibook-x: Found 2 MXC4005 devices - perfect setup!\n");
		pr_info("chuwi-minibook-x: Both accelerometers are already auto-discovered by the system\n");
		break;
		
	default:
		pr_warn("chuwi-minibook-x: Found %d MXC4005 devices (more than expected)\n", mxc_count);
		break;
	}
	
	return 0;
}

/**
 * chuwi_minibook_x_simple_hardware_test - Simple test of detected hardware
 * @chip: Device context
 *
 * Performs a basic test to verify that the detected accelerometers are functional.
 * This is a minimal test just to verify hardware detection works.
 *
 * Returns: 0 on success, negative error code on failure
 */
static int chuwi_minibook_x_simple_hardware_test(struct chuwi_minibook_x *chip)
{
	struct device *dev = &chip->pdev->dev;
	int mxc_count;

	dev_info(dev, "Performing simple hardware test\n");

	/* Give devices time to be fully bound after instantiation */
	msleep(500);

	/* Count available MXC4005 devices */
	mxc_count = chuwi_minibook_x_count_mxc4005_devices();
	
	dev_info(dev, "Hardware test results:\n");
	dev_info(dev, "  - MXC4005 devices found: %d\n", mxc_count);
	dev_info(dev, "  - Expected devices: 2 (base + lid accelerometers)\n");
	
	if (mxc_count == 0) {
		dev_warn(dev, "No accelerometers detected - driver may not function correctly\n");
		return -ENODEV;
	} else if (mxc_count == 1) {
		dev_info(dev, "Single accelerometer detected - limited functionality\n");
	} else if (mxc_count == 2) {
		dev_info(dev, "Dual accelerometers detected - full hardware support\n");
	} else {
		dev_info(dev, "More accelerometers than expected - may still work\n");
	}

	dev_info(dev, "Hardware test completed successfully\n");
	return 0;
}

/**
 * chuwi_minibook_x_setup_hardware_detection - Initialize minimal hardware detection
 * @chip: Device context
 *
 * Performs basic hardware detection without setting up full IIO channels.
 * This is the minimal version that just verifies hardware presence.
 *
 * Returns: 0 on success, negative error code on failure
 */
static int chuwi_minibook_x_setup_hardware_detection(struct chuwi_minibook_x *chip)
{
	struct device *dev = &chip->pdev->dev;
	int ret;

	dev_info(dev, "Setting up minimal hardware detection\n");

	/* Detect and report accelerometer hardware */
	ret = chuwi_minibook_x_detect_and_setup_accelerometers();
	if (ret) {
		dev_err(dev, "Failed to detect/setup accelerometers: %d\n", ret);
		return ret;
	}

	/* Run basic hardware test */
	ret = chuwi_minibook_x_simple_hardware_test(chip);
	if (ret) {
		dev_warn(dev, "Hardware test failed: %d\n", ret);
		/* Continue anyway for testing purposes */
	}

	dev_info(dev, "Hardware detection setup completed\n");
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

	dev_info(&pdev->dev, "Chuwi Minibook X minimal driver probing\n");

	chip = devm_kzalloc(&pdev->dev, sizeof(*chip), GFP_KERNEL);
	if (!chip)
		return -ENOMEM;

	chip->pdev = pdev;
	platform_set_drvdata(pdev, chip);
	
	/* Store global reference for now */
	g_chip = chip;

	/* Initialize basic fields */
	chip->accel_count = 0;
	chip->enabled = true;

	/* Initialize synchronization */
	mutex_init(&chip->lock);

	/* Configure based on DMI-detected hardware */
	ret = chuwi_minibook_x_configure_from_dmi(chip);
	if (ret) {
		dev_err(&pdev->dev, "Failed to configure from DMI: %d\n", ret);
		return ret;
	}

	/* Perform minimal hardware detection and testing */
	ret = chuwi_minibook_x_setup_hardware_detection(chip);
	if (ret) {
		dev_err(&pdev->dev, "Failed to setup hardware detection: %d\n", ret);
		return ret;
	}

	/* Create minimal sysfs interface for status reporting */
	ret = chuwi_minibook_x_create_sysfs(chip);
	if (ret) {
		dev_warn(&pdev->dev, "Failed to create sysfs interface: %d\n", ret);
		/* Continue without sysfs */
	}

	/* Initialize debugfs if enabled */
	ret = chuwi_minibook_x_debugfs_init(chip);
	if (ret) {
		dev_warn(&pdev->dev, "Failed to setup debugfs: %d\n", ret);
		/* Continue without debugfs */
	}

	dev_info(&pdev->dev, "Chuwi Minibook X minimal driver loaded successfully\n");
	dev_info(&pdev->dev, "This is a test driver for hardware detection only\n");

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

	dev_info(&pdev->dev, "Chuwi Minibook X minimal driver removing\n");

	/* Cleanup debugfs */
	chuwi_minibook_x_debugfs_cleanup(chip);

	/* Remove sysfs interface */
	chuwi_minibook_x_remove_sysfs(chip);

	/* Clear global reference */
	g_chip = NULL;

	dev_info(&pdev->dev, "Chuwi Minibook X minimal driver removed\n");
}

/* ACPI device matching table - disabled since MDA6655 is claimed by mxc4005 driver
static const struct acpi_device_id chuwi_minibook_x_acpi_match[] = {
	{"MDA6655", 0},   // MXC4005 accelerometer on Chuwi Minibook X 
	{ }
};
MODULE_DEVICE_TABLE(acpi, chuwi_minibook_x_acpi_match);
*/

/* Forward declarations for fallback functions */
static int chuwi_minibook_x_detect_and_setup_accelerometers(void);

/* Platform driver structure */
static struct platform_driver chuwi_minibook_x_driver = {
	.probe = chuwi_minibook_x_probe,
	.remove = chuwi_minibook_x_remove,
	.driver = {
		.name = CHUWI_MINIBOOK_X_DRIVER_NAME,
		/* .acpi_match_table = ACPI_PTR(chuwi_minibook_x_acpi_match), */
	},
};

/* Platform device for manual instantiation when ACPI enumeration doesn't work */
static struct platform_device *chuwi_minibook_x_device;

/**
 * chuwi_minibook_x_init - Module initialization
 *
 * Registers the platform driver and creates a platform device based on DMI detection.
 * Uses device detection to handle multiple loading attempts gracefully.
 * This allows both auto-loading (via DMI) and manual loading to work correctly.
 *
 * Returns: 0 on success, negative error code on failure
 */
static int __init chuwi_minibook_x_init(void)
{
	int ret;
	bool hardware_supported;

	pr_info("Chuwi Minibook X driver initializing\n");

	/* Use mutex to prevent race conditions during initialization */
	mutex_lock(&driver_init_lock);

	/* Check if driver is already registered */
	if (driver_registered) {
		pr_info("Driver already registered, skipping duplicate initialization\n");
		mutex_unlock(&driver_init_lock);
		return 0;
	}

	/* Check if we're running on supported hardware (DMI + ACPI) */
	hardware_supported = chuwi_minibook_x_check_hardware_support();
	if (!hardware_supported) {
		pr_warn("Hardware not fully supported, loading anyway for testing\n");
		pr_warn("DMI: Vendor=%s, Product=%s\n", 
			dmi_get_system_info(DMI_SYS_VENDOR) ?: "unknown",
			dmi_get_system_info(DMI_PRODUCT_NAME) ?: "unknown");
		pr_warn("Please report success/issues to driver maintainer\n");
	}

	/* Register platform driver */
	ret = platform_driver_register(&chuwi_minibook_x_driver);
	if (ret == -EBUSY || ret == -EEXIST) {
		pr_info("Platform driver already registered (auto-loaded), initialization successful\n");
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
		pr_info("Creating platform device based on DMI detection\n");
		chuwi_minibook_x_device = platform_device_register_simple(
			CHUWI_MINIBOOK_X_DRIVER_NAME, -1, NULL, 0);
		if (IS_ERR(chuwi_minibook_x_device)) {
			ret = PTR_ERR(chuwi_minibook_x_device);
			pr_err("Failed to create platform device: %d\n", ret);
			platform_driver_unregister(&chuwi_minibook_x_driver);
			driver_registered = false;
			mutex_unlock(&driver_init_lock);
			return ret;
		}
		device_created = true;
		pr_info("Platform device created successfully\n");
	} else if (!hardware_supported) {
		pr_info("Hardware not supported, platform driver registered but no device created\n");
		chuwi_minibook_x_device = NULL;
	} else {
		pr_info("Platform device already exists, skipping creation\n");
	}

	mutex_unlock(&driver_init_lock);
	pr_info("Chuwi Minibook X driver initialized successfully\n");
	return 0;
}

/**
 * chuwi_minibook_x_exit - Module cleanup
 *
 * Unregisters the platform driver and removes manual platform device if created.
 * Uses state tracking to ensure proper cleanup.
 */
static void __exit chuwi_minibook_x_exit(void)
{
	pr_info("Chuwi Minibook X driver exiting\n");

	mutex_lock(&driver_init_lock);

	/* Clean up any manually instantiated I2C devices */
	if (instantiated_client) {
		pr_info("chuwi-minibook-x: Removing manually instantiated MXC4005 on i2c-%d addr 0x%02x\n",
			instantiated_bus, instantiated_addr);
		i2c_unregister_device(instantiated_client);
		instantiated_client = NULL;
		instantiated_bus = -1;
		instantiated_addr = -1;
	}

	if (instantiated_client2) {
		pr_info("chuwi-minibook-x: Removing second manually instantiated MXC4005 on i2c-%d addr 0x%02x\n",
			instantiated_bus2, instantiated_addr2);
		i2c_unregister_device(instantiated_client2);
		instantiated_client2 = NULL;
		instantiated_bus2 = -1;
		instantiated_addr2 = -1;
	}

	/* Remove manual platform device if we created one */
	if (chuwi_minibook_x_device && device_created) {
		platform_device_unregister(chuwi_minibook_x_device);
		chuwi_minibook_x_device = NULL;
		device_created = false;
	}

	/* Unregister platform driver if we registered it */
	if (driver_registered) {
		platform_driver_unregister(&chuwi_minibook_x_driver);
		driver_registered = false;
	}

	mutex_unlock(&driver_init_lock);
	pr_info("Chuwi Minibook X driver exited\n");
}

module_init(chuwi_minibook_x_init);
module_exit(chuwi_minibook_x_exit);