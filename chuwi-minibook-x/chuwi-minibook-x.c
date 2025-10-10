// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Chuwi Minibook X Integrated Platform Driver
 *
 * Copyright (C) 2025 Your Name
 *
 * Integrated hardware detection, accelerometer instantiation with mount matrices,
 * and tablet mode detection for the Chuwi Minibook X convertible laptop.
 *
 * This driver combines:
 * - Proper I2C device instantiation with mount matrix transformations (v2)
 * - Full tablet mode detection and screen orientation (v1)
 * - Event-driven architecture using delayed work queues
 * - Comprehensive sysfs interface for configuration and data input
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
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/jiffies.h>
#include <linux/math64.h>
#include <linux/minmax.h>
#include <linux/kobject.h>
#ifdef CONFIG_CHUWI_MINIBOOK_X_DEBUGFS
#include <linux/debugfs.h>
#endif

#include "chuwi-minibook-x.h"

#define DRV_NAME "chuwi-minibook-x-tablet-mode"

/* Module information */
MODULE_DESCRIPTION("Chuwi Minibook X integrated platform driver with tablet mode detection");
MODULE_AUTHOR("Your Name");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:" CHUWI_MINIBOOK_X_DRIVER_NAME);

/* Module dependencies - ensure required drivers are loaded */
MODULE_SOFTDEP("pre: mxc4005");
MODULE_SOFTDEP("pre: serial_multi_instantiate");

/* Module parameters for hardware configuration */
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

/* Module parameters for tablet mode detection */
static unsigned int default_poll_ms = 200;
module_param_named(poll_ms, default_poll_ms, uint, 0644);
MODULE_PARM_DESC(poll_ms, "Default polling interval in milliseconds (20-5000, default: 200)");

static unsigned int default_enter_deg = 200;
module_param_named(enter_deg, default_enter_deg, uint, 0644);
MODULE_PARM_DESC(enter_deg, "Default enter tablet mode threshold in degrees (0-360, default: 200)");

static unsigned int default_exit_deg = 170;
module_param_named(exit_deg, default_exit_deg, uint, 0644);
MODULE_PARM_DESC(exit_deg, "Default exit tablet mode threshold in degrees (0-360, default: 170)");

static unsigned int default_hysteresis_deg = 10;
module_param_named(hysteresis_deg, default_hysteresis_deg, uint, 0644);
MODULE_PARM_DESC(hysteresis_deg, "Default hysteresis in degrees (0-90, default: 10)");

static bool default_signed_mode = true;
module_param_named(signed_mode, default_signed_mode, bool, 0644);
MODULE_PARM_DESC(signed_mode, "Default signed angle mode (0=unsigned 0-180°, 1=signed 0-360°, default: 1)");

static bool default_enabled = true;
module_param_named(enabled, default_enabled, bool, 0644);
MODULE_PARM_DESC(enabled, "Default polling enabled state (default: 1)");

bool debug_mode = false;
module_param(debug_mode, bool, 0644);
MODULE_PARM_DESC(debug_mode, "Enable debug output for hardware detection (default: false)");

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

/*
 * 3D Vector structure for accelerometer data (micro-g units)
 */
struct vec3 {
	s32 x, y, z;
};

/*
 * Global state variables for tablet mode detection
 */

/** @tm_input: Input device for tablet mode and orientation events */
static struct input_dev *tm_input;

/** @tm_kobj: Sysfs kobject for module configuration */
static struct kobject *tm_kobj;

/** @tm_lock: Mutex protecting global state during operations */
static DEFINE_MUTEX(tm_lock);

#ifdef CONFIG_CHUWI_MINIBOOK_X_DEBUGFS
/** @debugfs_root: Root debugfs directory */
static struct dentry *debugfs_root;
/** @debugfs_raw_data: Debugfs entry for raw accelerometer data */
static struct dentry *debugfs_raw_data;
/** @debugfs_calculations: Debugfs entry for angle calculations */
static struct dentry *debugfs_calculations;
#endif

/** @g_base: Current gravity vector from base accelerometer (laptop keyboard) */
static struct vec3 g_base = { 0, 0, 1000000 };

/** @g_lid: Current gravity vector from lid accelerometer (screen) */
static struct vec3 g_lid = { 0, 0, -1000000 };

/*
 * Tablet mode configuration and state  
 */

/** @enabled: Whether polling is currently enabled */
static int enabled = 1;

/** @force_tablet: Force tablet mode (-1=auto, 0=laptop, 1=tablet) */
static int force_tablet = -1;

/** @poll_ms: Polling interval in milliseconds */
static unsigned int poll_ms = 200;

/** @enter_deg: Hinge angle threshold to enter tablet mode */
static unsigned int enter_deg = 300;

/** @exit_deg: Hinge angle threshold to exit tablet mode */
static unsigned int exit_deg = 60;

/** @hysteresis_deg: Hysteresis guard angle in degrees */
static unsigned int hysteresis_deg = 10;

/** @cur_tablet: Current tablet mode state (0=laptop, 1=tablet) */
static int cur_tablet = 0;

/** @last_angle: Last computed hinge angle in degrees */
static unsigned int last_angle = 0;

/*
 * Hinge angle calculation parameters
 */

/** @signed_mode: Use signed angle mode (0=0-180°, 1=0-360°) */
static int signed_mode = 1;

/** @hinge_axis_unit: Unit vector representing hinge rotation axis */
static struct vec3 hinge_axis_unit = { 0, 1000000, 0 };

/** @poll_work: Delayed work queue for periodic polling */
static struct delayed_work poll_work;

/** @auto_calibration_done: Whether initial hinge axis calibration completed */
static bool auto_calibration_done = false;

/*
 * Angle stability filtering to prevent rapid state changes in flat positions
 */

/** @last_stable_angle: Last stable angle measurement */
static unsigned int last_stable_angle = 0;

/** @angle_instability_count: Count of unstable angle readings */
static int angle_instability_count = 0;

#define ANGLE_JUMP_THRESHOLD 100  /* degrees - detect major angle jumps */
#define MAX_INSTABILITY_COUNT 5   /* max unstable readings before using last stable */

/* Global driver context and device tracking */
static struct chuwi_minibook_x *g_chip;

/* Driver state tracking for idempotent initialization */
static bool driver_registered = false;
static bool device_created = false;
static DEFINE_MUTEX(driver_init_lock);

/* Track manually instantiated I2C devices for cleanup */
static struct i2c_client *instantiated_client = NULL;
static struct i2c_client *instantiated_client2 = NULL;
static int instantiated_bus = -1;
static int instantiated_addr = -1;
static int instantiated_bus2 = -1;
static int instantiated_addr2 = -1;

/*
 * DMI table for supported hardware
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
 * Forward declarations
 */
static void poll_work_fn(struct work_struct *w);
static int report_tablet_mode(int state);

// TODO: Add all the math functions from v1 (dot product, cross product, angle calculations)
// TODO: Add all the sysfs interface functions from v1 
// TODO: Add tablet mode detection logic from v1
// TODO: Add input device creation and management
// TODO: Add mount matrix device instantiation from v2
// TODO: Add DMI detection and hardware setup from v2

/*
 * This is a placeholder - the integration will continue with:
 * 1. Math utility functions from v1
 * 2. Tablet mode detection algorithm from v1  
 * 3. Sysfs interface for gravity data input from v1
 * 4. Hardware detection and device instantiation from v2
 * 5. Event-driven architecture using delayed work queues
 */

/* Initialize default values from module parameters */
static void init_module_defaults(void)
{
	/* Validate and set module parameter defaults */
	if (default_poll_ms < 20 || default_poll_ms > 5000) {
		pr_warn(DRV_NAME ": invalid poll_ms parameter %u, using 200\n", default_poll_ms);
		default_poll_ms = 200;
	}
	poll_ms = default_poll_ms;

	if (default_enter_deg > 360) {
		pr_warn(DRV_NAME ": invalid enter_deg parameter %u, using 200\n", default_enter_deg);
		default_enter_deg = 200;
	}
	enter_deg = default_enter_deg;

	if (default_exit_deg > 360) {
		pr_warn(DRV_NAME ": invalid exit_deg parameter %u, using 170\n", default_exit_deg);
		default_exit_deg = 170;
	}
	exit_deg = default_exit_deg;

	if (default_hysteresis_deg > 90) {
		pr_warn(DRV_NAME ": invalid hysteresis_deg parameter %u, using 10\n", default_hysteresis_deg);
		default_hysteresis_deg = 10;
	}
	hysteresis_deg = default_hysteresis_deg;

	signed_mode = default_signed_mode ? 1 : 0;
	enabled = default_enabled ? 1 : 0;
}

/* Placeholder work function - will be populated with tablet mode detection logic */
static void poll_work_fn(struct work_struct *w)
{
	mutex_lock(&tm_lock);
	
	/* TODO: Add tablet mode evaluation logic from v1 */
	pr_debug(DRV_NAME ": Poll work triggered (placeholder)\n");
	
	mutex_unlock(&tm_lock);

	if (enabled)
		schedule_delayed_work(&poll_work, msecs_to_jiffies(poll_ms));
}

/* Placeholder sysfs functions - will be populated with full interface from v1 */
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
	
	/* Trigger immediate evaluation using event-driven approach */
	schedule_delayed_work(&poll_work, 0);
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
	
	/* Trigger immediate evaluation using event-driven approach */
	schedule_delayed_work(&poll_work, 0);
	return l;
}

/* Basic attribute definitions */
static struct kobj_attribute base_vec_attr = __ATTR(base_vec, 0644, show_base_vec, store_base_vec);
static struct kobj_attribute lid_vec_attr = __ATTR(lid_vec, 0644, show_lid_vec, store_lid_vec);

static struct attribute *tablet_mode_attrs[] = {
	&base_vec_attr.attr,
	&lid_vec_attr.attr,
	NULL,
};

static struct attribute_group tablet_mode_attr_group = {
	.attrs = tablet_mode_attrs,
};

/* Hardware detection and device instantiation from v2 */

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
		pr_info(DRV_NAME ": Found MXC4005 device on i2c-%d addr 0x%02x\n",
			client->adapter->nr, client->addr);
	}
	
	return 0;
}

/**
 * chuwi_minibook_x_count_mxc4005_devices - Count available MXC4005 IIO devices
 */
static int chuwi_minibook_x_count_mxc4005_devices(void)
{
	int count = 0;
	
	/* Iterate through I2C bus devices to find MXC4005 chips */
	bus_for_each_dev(&i2c_bus_type, NULL, &count, chuwi_minibook_x_match_mxc4005);
	
	return count;
}

/**
 * chuwi_minibook_x_instantiate_mxc4005 - Instantiate MXC4005 device on I2C bus with mount matrix
 */
static int chuwi_minibook_x_instantiate_mxc4005(int bus_nr, int addr, bool is_second, bool is_lid)
{
	struct i2c_adapter *adapter;
	struct i2c_board_info info = {};
	struct i2c_client *client;
	
	adapter = i2c_get_adapter(bus_nr);
	if (!adapter) {
		pr_err(DRV_NAME ": I2C adapter %d not found\n", bus_nr);
		return -ENODEV;
	}
	
	snprintf(info.type, sizeof(info.type), "mxc4005");
	info.addr = addr;
	
	/* Apply mount matrix if enabled */
	if (enable_mount_matrix) {
		if (is_lid) {
			info.swnode = &lid_sensor_node;
			pr_info(DRV_NAME ": Applying lid sensor mount matrix (90° CCW)\n");
		} else {
			info.swnode = &base_sensor_node;
			pr_info(DRV_NAME ": Applying base sensor mount matrix (90° CW)\n");
		}
	} else {
		pr_info(DRV_NAME ": Mount matrix disabled, using identity transformation\n");
	}
	
	client = i2c_new_client_device(adapter, &info);
	i2c_put_adapter(adapter);
	
	if (IS_ERR(client)) {
		pr_err(DRV_NAME ": Failed to instantiate MXC4005 on i2c-%d addr 0x%02x: %ld\n",
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
	
	pr_info(DRV_NAME ": Instantiated MXC4005 on i2c-%d addr 0x%02x\n", bus_nr, addr);
	return 0;
}

/**
 * chuwi_minibook_x_check_dmi_match - Check if current hardware matches supported DMI
 */
static bool chuwi_minibook_x_check_dmi_match(void)
{
	const struct dmi_system_id *dmi_id;
	
	dmi_id = dmi_first_match(chuwi_minibook_x_dmi_table);
	if (dmi_id) {
		pr_info(DRV_NAME ": Detected supported hardware: %s\n", dmi_id->ident);
		return true;
	}
	
	pr_info(DRV_NAME ": Hardware not in DMI support table\n");
	pr_info(DRV_NAME ": Vendor: %s, Product: %s\n", 
		dmi_get_system_info(DMI_SYS_VENDOR) ?: "unknown",
		dmi_get_system_info(DMI_PRODUCT_NAME) ?: "unknown");
	
	return false;
}

/**
 * chuwi_minibook_x_setup_accelerometers - Setup accelerometer hardware with mount matrices
 */
static int chuwi_minibook_x_setup_accelerometers(void)
{
	int mxc_count;
	int ret;
	
	/* Give system time for MODULE_SOFTDEP to take effect */
	msleep(100);
	
	/* Initial count - may be 0 if devices haven't been bound yet */
	mxc_count = chuwi_minibook_x_count_mxc4005_devices();
	
	pr_info(DRV_NAME ": Initially found %d MXC4005 device(s)\n", mxc_count);
	
	if (mxc_count < 2) {
		pr_info(DRV_NAME ": Attempting to instantiate missing accelerometer devices\n");
		
		/* Try to instantiate the lid accelerometer first */
		if (mxc_count == 0) {
			pr_info(DRV_NAME ": Instantiating lid accelerometer on i2c-%d addr 0x%02x\n", lid_bus, lid_addr);
			ret = chuwi_minibook_x_instantiate_mxc4005(lid_bus, lid_addr, false, true);
			if (ret) {
				pr_warn(DRV_NAME ": Failed to instantiate lid accelerometer: %d\n", ret);
			}
			msleep(200); /* Give device time to be recognized */
		}
		
		/* Try to instantiate the base accelerometer */
		pr_info(DRV_NAME ": Instantiating base accelerometer on i2c-%d addr 0x%02x\n", base_bus, base_addr);
		ret = chuwi_minibook_x_instantiate_mxc4005(base_bus, base_addr, true, false);
		if (ret) {
			pr_warn(DRV_NAME ": Failed to instantiate base accelerometer: %d\n", ret);
		}
	}
	
	/* Final count after instantiation attempts */
	msleep(500);
	mxc_count = chuwi_minibook_x_count_mxc4005_devices();
	pr_info(DRV_NAME ": Final accelerometer count: %d\n", mxc_count);
	
	return 0;
}

/**
 * chuwi_minibook_x_init_tablet_mode - Initialize tablet mode detection system
 */
static int chuwi_minibook_x_init_tablet_mode(struct platform_device *pdev)
{
	int ret;
	
	pr_info(DRV_NAME ": Initializing tablet mode detection\n");
	
	/* Initialize default values from module parameters */
	init_module_defaults();
	
	/* Create input device for tablet mode events */
	tm_input = devm_input_allocate_device(&pdev->dev);
	if (!tm_input) {
		pr_err(DRV_NAME ": Failed to allocate input device\n");
		return -ENOMEM;
	}
	
	tm_input->name = "Chuwi Minibook X Tablet Mode";
	tm_input->phys = "chuwi-minibook-x/input0";
	tm_input->id.bustype = BUS_HOST;
	tm_input->dev.parent = &pdev->dev;
	
	/* Set up input capabilities */
	input_set_capability(tm_input, EV_SW, SW_TABLET_MODE);
	
	ret = input_register_device(tm_input);
	if (ret) {
		pr_err(DRV_NAME ": Failed to register input device: %d\n", ret);
		return ret;
	}
	
	/* Create sysfs interface */
	tm_kobj = kobject_create_and_add("chuwi-minibook-x-tablet-mode", kernel_kobj);
	if (!tm_kobj) {
		pr_err(DRV_NAME ": Failed to create sysfs kobject\n");
		input_unregister_device(tm_input);
		return -ENOMEM;
	}
	
	ret = sysfs_create_group(tm_kobj, &tablet_mode_attr_group);
	if (ret) {
		pr_err(DRV_NAME ": Failed to create sysfs group: %d\n", ret);
		kobject_put(tm_kobj);
		input_unregister_device(tm_input);
		return ret;
	}
	
	/* Initialize delayed work */
	INIT_DELAYED_WORK(&poll_work, poll_work_fn);
	
	pr_info(DRV_NAME ": Tablet mode detection initialized successfully\n");
	
	return 0;
}

/**
 * chuwi_minibook_x_cleanup_tablet_mode - Cleanup tablet mode detection system
 */
static void chuwi_minibook_x_cleanup_tablet_mode(void)
{
	/* Cancel any pending work */
	cancel_delayed_work_sync(&poll_work);
	
	/* Remove sysfs interface */
	if (tm_kobj) {
		sysfs_remove_group(tm_kobj, &tablet_mode_attr_group);
		kobject_put(tm_kobj);
		tm_kobj = NULL;
	}
	
	/* Input device will be cleaned up automatically via devm */
}

/*
 * Stub implementations for sysfs and debugfs functions
 * These will be replaced with full implementations later
 */

/**
 * chuwi_minibook_x_create_sysfs - Create basic hardware sysfs interface (stub)
 */
int chuwi_minibook_x_create_sysfs(struct chuwi_minibook_x *chip)
{
	/* For now, just log that this is a stub */
	pr_debug(DRV_NAME ": Hardware sysfs interface creation skipped (stub)\n");
	return 0;
}

/**
 * chuwi_minibook_x_remove_sysfs - Remove hardware sysfs interface (stub)
 */
void chuwi_minibook_x_remove_sysfs(struct chuwi_minibook_x *chip)
{
	/* For now, just log that this is a stub */
	pr_debug(DRV_NAME ": Hardware sysfs interface removal skipped (stub)\n");
}

/**
 * chuwi_minibook_x_probe - Platform device probe function
 */
int chuwi_minibook_x_probe(struct platform_device *pdev)
{
	struct chuwi_minibook_x *chip;
	int ret;

	dev_info(&pdev->dev, "Chuwi Minibook X integrated driver probing\n");

	chip = devm_kzalloc(&pdev->dev, sizeof(*chip), GFP_KERNEL);
	if (!chip)
		return -ENOMEM;

	chip->pdev = pdev;
	platform_set_drvdata(pdev, chip);
	g_chip = chip;

	/* Initialize basic fields */
	chip->accel_count = 0;
	chip->enabled = true;
	mutex_init(&chip->lock);

	/* Check DMI support */
	if (!chuwi_minibook_x_check_dmi_match()) {
		dev_warn(&pdev->dev, "Hardware not fully supported, continuing anyway\n");
	}

	/* Setup accelerometer hardware with mount matrices */
	ret = chuwi_minibook_x_setup_accelerometers();
	if (ret) {
		dev_err(&pdev->dev, "Failed to setup accelerometers: %d\n", ret);
		return ret;
	}

	/* Initialize tablet mode detection */
	ret = chuwi_minibook_x_init_tablet_mode(pdev);
	if (ret) {
		dev_err(&pdev->dev, "Failed to initialize tablet mode detection: %d\n", ret);
		return ret;
	}

	/* Create basic sysfs interface for hardware status */
	ret = chuwi_minibook_x_create_sysfs(chip);
	if (ret) {
		dev_warn(&pdev->dev, "Failed to create hardware sysfs interface: %d\n", ret);
		/* Continue without basic sysfs */
	}

	/* Initialize debugfs if enabled */
	ret = chuwi_minibook_x_debugfs_init(chip);
	if (ret) {
		dev_warn(&pdev->dev, "Failed to setup debugfs: %d\n", ret);
		/* Continue without debugfs */
	}

	dev_info(&pdev->dev, "Chuwi Minibook X integrated driver loaded successfully\n");
	dev_info(&pdev->dev, "Mount matrix support: %s\n", enable_mount_matrix ? "enabled" : "disabled");
	dev_info(&pdev->dev, "Tablet mode detection initialized with %ums polling\n", poll_ms);

	return 0;
}

/**
 * chuwi_minibook_x_remove - Platform device remove function
 */
void chuwi_minibook_x_remove(struct platform_device *pdev)
{
	struct chuwi_minibook_x *chip = platform_get_drvdata(pdev);

	dev_info(&pdev->dev, "Chuwi Minibook X integrated driver removing\n");

	/* Cleanup tablet mode detection */
	chuwi_minibook_x_cleanup_tablet_mode();

	/* Cleanup debugfs */
	chuwi_minibook_x_debugfs_cleanup(chip);

	/* Remove basic sysfs interface */
	chuwi_minibook_x_remove_sysfs(chip);

	/* Clear global reference */
	g_chip = NULL;

	dev_info(&pdev->dev, "Chuwi Minibook X integrated driver removed\n");
}

/* Platform driver structure */
static struct platform_driver chuwi_minibook_x_driver = {
	.probe = chuwi_minibook_x_probe,
	.remove = chuwi_minibook_x_remove,
	.driver = {
		.name = CHUWI_MINIBOOK_X_DRIVER_NAME,
	},
};

/* Platform device for manual instantiation when ACPI enumeration doesn't work */
static struct platform_device *chuwi_minibook_x_device;

/**
 * chuwi_minibook_x_init - Module initialization
 */
static int __init chuwi_minibook_x_init(void)
{
	int ret;
	bool hardware_supported;

	pr_info(DRV_NAME " integrated driver initializing\n");

	/* Use mutex to prevent race conditions during initialization */
	mutex_lock(&driver_init_lock);

	/* Check if driver is already registered */
	if (driver_registered) {
		pr_info("Driver already registered, skipping duplicate initialization\n");
		mutex_unlock(&driver_init_lock);
		return 0;
	}

	/* Check if we're running on supported hardware */
	hardware_supported = chuwi_minibook_x_check_dmi_match();
	if (!hardware_supported) {
		pr_warn("Hardware not fully supported, loading anyway for testing\n");
	}

	/* Register platform driver */
	ret = platform_driver_register(&chuwi_minibook_x_driver);
	if (ret == -EBUSY || ret == -EEXIST) {
		pr_info("Platform driver already registered, initialization successful\n");
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
	}

	mutex_unlock(&driver_init_lock);
	pr_info(DRV_NAME " integrated driver initialized successfully\n");
	return 0;
}

/**
 * chuwi_minibook_x_exit - Module cleanup
 */
static void __exit chuwi_minibook_x_exit(void)
{
	pr_info(DRV_NAME " integrated driver exiting\n");

	mutex_lock(&driver_init_lock);

	/* Clean up any manually instantiated I2C devices */
	if (instantiated_client) {
		pr_info(DRV_NAME ": Removing manually instantiated MXC4005 on i2c-%d addr 0x%02x\n",
			instantiated_bus, instantiated_addr);
		i2c_unregister_device(instantiated_client);
		instantiated_client = NULL;
	}

	if (instantiated_client2) {
		pr_info(DRV_NAME ": Removing second manually instantiated MXC4005 on i2c-%d addr 0x%02x\n",
			instantiated_bus2, instantiated_addr2);
		i2c_unregister_device(instantiated_client2);
		instantiated_client2 = NULL;
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
	pr_info(DRV_NAME " integrated driver exited\n");
}

module_init(chuwi_minibook_x_init);
module_exit(chuwi_minibook_x_exit);