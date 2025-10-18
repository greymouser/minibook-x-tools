// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Chuwi Minibook X Integrated Platform Driver
 *
 * Copyright (C) 2025 Armando DiCianno
 *
 * Integrated hardware detection, accelerometer instantiation with mount matrices,
 * and tablet mode detection for the Chuwi Minibook X convertible laptop.
 *
 * This driver combines:
 * - Proper I2C device instantiation with mount matrix transformations
 * - Full tablet mode detection and screen orientation
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

#define DRV_NAME "chuwi-minibook-x"

/* Module information */
MODULE_DESCRIPTION("Chuwi Minibook X integrated platform driver");
MODULE_AUTHOR("Armando DiCianno <armando@noonshy.com>");
MODULE_LICENSE("GPL");

/* Version is defined at build time from VERSION file */
#ifndef MODULE_VERSION_STRING
#define MODULE_VERSION_STRING "3.0"
#endif
MODULE_VERSION(MODULE_VERSION_STRING);

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

/**
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

/**
 * struct vec3 - 3D vector structure for accelerometer data (micro-g units)
 * @x: X-axis acceleration in micro-g
 * @y: Y-axis acceleration in micro-g
 * @z: Z-axis acceleration in micro-g
 */
struct vec3 {
	s32 x, y, z;
};

/**
 * Global state variables for tablet mode detection
 */

/** @tm_input: Input device for tablet mode and orientation events */
static struct input_dev *tm_input;

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

/**
 * Tablet mode configuration and state
 */

/** @enabled: Whether polling is currently enabled */
static int enabled = 1;

/** @poll_ms: Polling interval in milliseconds */
static unsigned int poll_ms = 200;

/** @enter_deg: Hinge angle threshold to enter tablet mode */
static unsigned int enter_deg = 300;

/** @exit_deg: Hinge angle threshold to exit tablet mode */
static unsigned int exit_deg = 60;

/** @hysteresis_deg: Hysteresis guard angle in degrees */
static unsigned int hysteresis_deg = 10;

/** @current_mode: Current device mode string */
static char current_mode[16] = "laptop";

/** @current_orientation: Current device orientation string */
static char current_orientation[32] = "portrait";

/**
 * Hinge angle calculation parameters
 */

/** @signed_mode: Use signed angle mode (0=0-180°, 1=0-360°) */
static int signed_mode = 1;

/** @poll_work: Delayed work queue for periodic polling */
static struct delayed_work poll_work;

/* Global driver context and device tracking */
static struct chuwi_minibook_x *g_chip;

/* Driver state tracking for idempotent initialization */
static bool driver_registered = false;
static bool device_created = false;
static DEFINE_MUTEX(driver_init_lock);

/* Track software nodes applied to existing devices for cleanup (protected by driver_init_lock) */
static struct i2c_client *existing_lid_client = NULL;
static struct i2c_client *existing_base_client = NULL;

/* Track manually instantiated I2C devices for cleanup */
static struct i2c_client *instantiated_client = NULL;
static struct i2c_client *instantiated_client2 = NULL;
static int instantiated_bus = -1;
static int instantiated_addr = -1;
static int instantiated_bus2 = -1;
static int instantiated_addr2 = -1;

/**
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

/**
 * Forward declarations
 */
static void poll_work_fn(struct work_struct *w);

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

/**
 * poll_work_fn - Work function for tablet mode detection
 * @w: Work structure
 *
 * This is currently a stub that provides infrastructure for future
 * in-kernel tablet mode detection. Currently, the chuwi-minibook-x-daemon
 * userspace program handles calculations and writes to sysfs.
 */
static void poll_work_fn(struct work_struct *w)
{
	mutex_lock(&tm_lock);
	
	/* Stub: Full tablet mode detection logic can be added here in the future */
	pr_debug(DRV_NAME ": Poll work triggered\n");
	
	mutex_unlock(&tm_lock);

	if (enabled)
		schedule_delayed_work(&poll_work, msecs_to_jiffies(poll_ms));
}

/**
 * Sysfs interface functions
 */

/**
 * notify_tablet_mode_change - Send SW_TABLET_MODE input event
 * @is_tablet: true if entering tablet mode, false if exiting
 *
 * Sends input event to notify desktop environments of tablet mode state changes.
 */
static void notify_tablet_mode_change(bool is_tablet)
{
	if (!tm_input) {
		pr_debug(DRV_NAME ": Input device not available for tablet mode notification\n");
		return;
	}
	
	input_report_switch(tm_input, SW_TABLET_MODE, is_tablet ? 1 : 0);
	input_sync(tm_input);
	
	pr_info(DRV_NAME ": Tablet mode %s\n", is_tablet ? "ENABLED" : "DISABLED");
}

/**
 * is_tablet_mode - Check if a mode string represents tablet mode
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

/**
 * show_iio_base_device - Show IIO device name for base accelerometer
 */
static ssize_t show_iio_base_device(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	/* Base is typically on i2c-12, which usually maps to iio:device1 */
	if (base_bus == 12) {
		return snprintf(buf, PAGE_SIZE, "iio:device1\n");
	} else if (base_bus == 13) {
		return snprintf(buf, PAGE_SIZE, "iio:device0\n");
	} else {
		/* Fallback - provide I2C info for userspace to resolve */
		return snprintf(buf, PAGE_SIZE, "i2c-%d:0x%02x\n", base_bus, base_addr);
	}
}

/**
 * show_iio_lid_device - Show IIO device name for lid accelerometer  
 */
static ssize_t show_iio_lid_device(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	/* Lid is typically on i2c-13, which usually maps to iio:device0 */
	if (lid_bus == 13) {
		return snprintf(buf, PAGE_SIZE, "iio:device0\n");
	} else if (lid_bus == 12) {
		return snprintf(buf, PAGE_SIZE, "iio:device1\n");
	} else {
		/* Fallback - provide I2C info for userspace to resolve */
		return snprintf(buf, PAGE_SIZE, "i2c-%d:0x%02x\n", lid_bus, lid_addr);
	}
}

/**
 * show_mode - Show current device mode
 */
static ssize_t show_mode(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%s\n", current_mode);
}

/**
 * store_mode - Set device mode with validation and send input events
 */
static ssize_t store_mode(struct kobject *kobj, struct kobj_attribute *attr,
			  const char *buf, size_t len)
{
	char mode_str[16];
	char old_mode[16];
	bool old_is_tablet, new_is_tablet;
	int ret;
	
	/* Copy and trim whitespace */
	ret = sscanf(buf, "%15s", mode_str);
	if (ret != 1)
		return -EINVAL;
	
	/* Validate mode */
	if (strcmp(mode_str, "closing") != 0 &&
	    strcmp(mode_str, "laptop") != 0 &&
	    strcmp(mode_str, "flat") != 0 &&
	    strcmp(mode_str, "tent") != 0 &&
	    strcmp(mode_str, "tablet") != 0) {
		return -EINVAL;
	}
	
	/* Check for mode transition and generate input events */
	mutex_lock(&tm_lock);
	
	/* Save old mode for comparison */
	strncpy(old_mode, current_mode, sizeof(old_mode) - 1);
	old_mode[sizeof(old_mode) - 1] = '\0';
	
	/* Update current mode */
	strncpy(current_mode, mode_str, sizeof(current_mode) - 1);
	current_mode[sizeof(current_mode) - 1] = '\0';
	
	mutex_unlock(&tm_lock);
	
	/* Determine if we're transitioning to/from tablet mode */
	old_is_tablet = is_tablet_mode(old_mode);
	new_is_tablet = is_tablet_mode(mode_str);
	
	/* Send input event only if tablet mode state changed */
	if (old_is_tablet != new_is_tablet) {
		notify_tablet_mode_change(new_is_tablet);
	}
	
	return len;
}

/**
 * show_orientation - Show current device orientation
 */
static ssize_t show_orientation(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%s\n", current_orientation);
}

/**
 * store_orientation - Set device orientation with validation
 */
static ssize_t store_orientation(struct kobject *kobj, struct kobj_attribute *attr,
				 const char *buf, size_t len)
{
	char orientation_str[32];
	int ret;
	
	/* Copy and trim whitespace */
	ret = sscanf(buf, "%31s", orientation_str);
	if (ret != 1)
		return -EINVAL;
	
	/* Validate orientation - using standard iOS/Windows terms */
	if (strcmp(orientation_str, "portrait") != 0 &&
	    strcmp(orientation_str, "landscape") != 0 &&
	    strcmp(orientation_str, "portrait-flipped") != 0 &&
	    strcmp(orientation_str, "landscape-flipped") != 0) {
		return -EINVAL;
	}
	
	/* Update current orientation */
	mutex_lock(&tm_lock);
	strncpy(current_orientation, orientation_str, sizeof(current_orientation) - 1);
	current_orientation[sizeof(current_orientation) - 1] = '\0';
	mutex_unlock(&tm_lock);
	
	return len;
}

/* Basic attribute definitions */
static struct kobj_attribute base_vec_attr = __ATTR(base_vec, 0644, show_base_vec, store_base_vec);
static struct kobj_attribute lid_vec_attr = __ATTR(lid_vec, 0644, show_lid_vec, store_lid_vec);
static struct kobj_attribute iio_base_device_attr = __ATTR(iio_base_device, 0444, show_iio_base_device, NULL);
static struct kobj_attribute iio_lid_device_attr = __ATTR(iio_lid_device, 0444, show_iio_lid_device, NULL);
static struct kobj_attribute mode_attr = __ATTR(mode, 0644, show_mode, store_mode);
static struct kobj_attribute orientation_attr = __ATTR(orientation, 0644, show_orientation, store_orientation);

static struct attribute *tablet_mode_attrs[] = {
	&base_vec_attr.attr,
	&lid_vec_attr.attr,
	&iio_base_device_attr.attr,
	&iio_lid_device_attr.attr,
	&mode_attr.attr,
	&orientation_attr.attr,
	NULL,
};

static struct attribute_group tablet_mode_attr_group = {
	.attrs = tablet_mode_attrs,
};

/**
 * Hardware detection and device instantiation
 */

/**
 * struct device_search_ctx - Context for finding existing I2C devices
 * @target_bus: I2C bus number to search
 * @target_addr: I2C address to search for
 * @found_client: Pointer to found I2C client (output)
 */
struct device_search_ctx {
	int target_bus;
	int target_addr;
	struct i2c_client *found_client;
};

/**
 * chuwi_minibook_x_find_device_callback - Callback to find specific I2C device
 */
static int chuwi_minibook_x_find_device_callback(struct device *dev, void *data)
{
	struct device_search_ctx *ctx = (struct device_search_ctx *)data;
	struct i2c_client *client;
	
	/* Only process I2C client devices, not adapters or other device types */
	if (dev->type != &i2c_client_type)
		return 0;
	
	client = to_i2c_client(dev);
	if (!client || !client->adapter)
		return 0;
	
	/* Check if this device matches our target bus and address */
	if (client->adapter->nr == ctx->target_bus && client->addr == ctx->target_addr) {
		/* Only consider devices with the mxc4005 driver */
		if (client->dev.driver && strcmp(client->dev.driver->name, "mxc4005") == 0) {
			ctx->found_client = client;
			return 1; /* Found it, stop iteration */
		}
	}
	
	return 0;
}

/**
 * chuwi_minibook_x_find_existing_device - Find existing I2C device on specified bus/address
 */
static struct i2c_client *chuwi_minibook_x_find_existing_device(int bus_nr, int addr)
{
	struct device_search_ctx ctx = {
		.target_bus = bus_nr,
		.target_addr = addr,
		.found_client = NULL
	};
	
	/* Search through I2C bus devices */
	bus_for_each_dev(&i2c_bus_type, NULL, &ctx, chuwi_minibook_x_find_device_callback);
	
	if (ctx.found_client && g_chip && g_chip->debug_mode) {
		pr_info(DRV_NAME ": Found existing device on i2c-%d:0x%02x (driver: %s)\n",
			bus_nr, addr, ctx.found_client->dev.driver ? ctx.found_client->dev.driver->name : "none");
	}
	
	return ctx.found_client;
}

/**
 * chuwi_minibook_x_apply_mount_matrix_to_existing - Apply mount matrix to existing device
 */
static int chuwi_minibook_x_apply_mount_matrix_to_existing(struct i2c_client *client, bool is_lid)
{
	const struct software_node *node;
	int ret;
	
	if (!client || !enable_mount_matrix) {
		return 0;
	}
	
	/* Select appropriate software node */
	node = is_lid ? &lid_sensor_node : &base_sensor_node;
	
	/* Apply the software node to the existing device */
	ret = device_add_software_node(&client->dev, node);
	if (ret) {
		pr_warn(DRV_NAME ": Failed to apply mount matrix to existing device i2c-%d:0x%02x: %d\n",
			client->adapter->nr, client->addr, ret);
		return ret;
	}
	
	/* Track the client for cleanup */
	if (is_lid) {
		existing_lid_client = client;
	} else {
		existing_base_client = client;
	}
	
	pr_info(DRV_NAME ": Applied %s sensor mount matrix (%s) to existing device i2c-%d:0x%02x\n",
		is_lid ? "lid" : "base",
		is_lid ? "90° CCW" : "90° CW",
		client->adapter->nr, client->addr);
	
	if (g_chip && g_chip->debug_mode) {
		pr_info(DRV_NAME ": %s mount matrix: [%s,%s,%s; %s,%s,%s; %s,%s,%s]\n",
			is_lid ? "Lid" : "Base",
			is_lid ? "0" : "0", is_lid ? "1" : "-1", "0",
			is_lid ? "-1" : "1", is_lid ? "0" : "0", "0",
			"0", "0", "1");
	}
	
	return 0;
}
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
			pr_info(DRV_NAME ": Applying lid sensor mount matrix (90° CCW) to i2c-%d:0x%02x\n", bus_nr, addr);
			if (g_chip && g_chip->debug_mode) {
				pr_info(DRV_NAME ": Lid mount matrix: [0,1,0; -1,0,0; 0,0,1]\n");
			}
		} else {
			info.swnode = &base_sensor_node;
			pr_info(DRV_NAME ": Applying base sensor mount matrix (90° CW) to i2c-%d:0x%02x\n", bus_nr, addr);
			if (g_chip && g_chip->debug_mode) {
				pr_info(DRV_NAME ": Base mount matrix: [0,-1,0; 1,0,0; 0,0,1]\n");
			}
		}
	} else {
		pr_info(DRV_NAME ": Mount matrix disabled for i2c-%d:0x%02x, using identity transformation\n", bus_nr, addr);
		if (g_chip && g_chip->debug_mode) {
			pr_info(DRV_NAME ": Identity matrix: [1,0,0; 0,1,0; 0,0,1]\n");
		}
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
	
	pr_info(DRV_NAME ": Instantiated MXC4005 on i2c-%d addr 0x%02x (%s sensor with %s)\n",
		bus_nr, addr, is_lid ? "lid" : "base",
		enable_mount_matrix ? (is_lid ? "90° CCW mount matrix" : "90° CW mount matrix") : "identity matrix");
	
	if (g_chip && g_chip->debug_mode) {
		pr_info(DRV_NAME ": Device %s will be available as IIO device after driver binding\n",
			is_lid ? "lid" : "base");
	}
	
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
	struct i2c_client *existing_lid = NULL;
	struct i2c_client *existing_base = NULL;
	bool lid_handled = false;
	bool base_handled = false;
	
	/* FIXME: Wait for MODULE_SOFTDEP drivers to load and bind */
	msleep(100);
	
	/* Check for existing devices first */
	existing_lid = chuwi_minibook_x_find_existing_device(lid_bus, lid_addr);
	existing_base = chuwi_minibook_x_find_existing_device(base_bus, base_addr);
	
	/* Apply mount matrices to existing devices */
	if (existing_lid) {
		pr_info(DRV_NAME ": Found existing lid accelerometer on i2c-%d:0x%02x\n", lid_bus, lid_addr);
		ret = chuwi_minibook_x_apply_mount_matrix_to_existing(existing_lid, true);
		if (ret == 0) {
			lid_handled = true;
		}
	}
	
	if (existing_base) {
		pr_info(DRV_NAME ": Found existing base accelerometer on i2c-%d:0x%02x\n", base_bus, base_addr);
		ret = chuwi_minibook_x_apply_mount_matrix_to_existing(existing_base, false);
		if (ret == 0) {
			base_handled = true;
		}
	}
	
	/* Initial count - may be 0 if devices haven't been bound yet */
	mxc_count = chuwi_minibook_x_count_mxc4005_devices();
	
	pr_info(DRV_NAME ": Initially found %d MXC4005 device(s)\n", mxc_count);

	if (mxc_count < 2 || !lid_handled || !base_handled) {
		pr_info(DRV_NAME ": Attempting to instantiate missing accelerometer devices\n");
		if (g_chip && g_chip->debug_mode) {
			pr_info(DRV_NAME ": Target configuration - Lid: i2c-%d:0x%02x, Base: i2c-%d:0x%02x\n",
				lid_bus, lid_addr, base_bus, base_addr);
			pr_info(DRV_NAME ": Mount matrix enabled: %s\n", enable_mount_matrix ? "yes" : "no");
			pr_info(DRV_NAME ": Lid handled: %s, Base handled: %s\n",
				lid_handled ? "yes" : "no", base_handled ? "yes" : "no");
		}
		
		/* Try to instantiate the lid accelerometer if not already handled */
		if (!lid_handled) {
			pr_info(DRV_NAME ": Instantiating lid accelerometer on i2c-%d addr 0x%02x\n", lid_bus, lid_addr);
			ret = chuwi_minibook_x_instantiate_mxc4005(lid_bus, lid_addr, false, true);
			if (ret) {
				pr_warn(DRV_NAME ": Failed to instantiate lid accelerometer: %d\n", ret);
			}
			/* FIXME: Wait for I2C device registration and driver binding */
			msleep(200);
		}
		
		/* Try to instantiate the base accelerometer if not already handled */
		if (!base_handled) {
			pr_info(DRV_NAME ": Instantiating base accelerometer on i2c-%d addr 0x%02x\n", base_bus, base_addr);
			ret = chuwi_minibook_x_instantiate_mxc4005(base_bus, base_addr, true, false);
			if (ret) {
				pr_warn(DRV_NAME ": Failed to instantiate base accelerometer: %d\n", ret);
			}
		}
	} else {
		pr_info(DRV_NAME ": All accelerometers already configured with mount matrices\n");
	}
	
	/* FIXME: Wait for all devices to complete driver binding */
	msleep(500);
	mxc_count = chuwi_minibook_x_count_mxc4005_devices();
	pr_info(DRV_NAME ": Final accelerometer count: %d\n", mxc_count);
	
	return 0;
}/**
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
	
	tm_input->name = "Chuwi Minibook X input";
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
	
	/* Set initial tablet mode state to laptop mode (not in tablet mode) */
	input_report_switch(tm_input, SW_TABLET_MODE, 0);
	input_sync(tm_input);
	pr_info(DRV_NAME ": Initial mode set to laptop (tablet mode disabled)\n");
	
	/* Create sysfs interface under platform driver */
	ret = sysfs_create_group(&pdev->dev.kobj, &tablet_mode_attr_group);
	if (ret) {
		pr_err(DRV_NAME ": Failed to create sysfs group: %d\n", ret);
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
	if (g_chip && g_chip->pdev) {
		sysfs_remove_group(&g_chip->pdev->dev.kobj, &tablet_mode_attr_group);
	}
	
	/* Input device will be cleaned up automatically via devm */
}

/* Hardware sysfs interface functions */

/**
 * chuwi_minibook_x_create_sysfs - Create basic hardware sysfs interface
 */
int chuwi_minibook_x_create_sysfs(struct chuwi_minibook_x *chip)
{
	/* Basic sysfs interface can be added here if needed for hardware status */
	pr_debug(DRV_NAME ": Hardware sysfs interface ready\n");
	return 0;
}

/**
 * chuwi_minibook_x_remove_sysfs - Remove hardware sysfs interface
 */
void chuwi_minibook_x_remove_sysfs(struct chuwi_minibook_x *chip)
{
	/* Cleanup any hardware-specific sysfs entries */
	pr_debug(DRV_NAME ": Hardware sysfs interface removed\n");
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
	chip->debug_mode = false;  /* Can be enabled via sysfs after module load */
	mutex_init(&chip->lock);

	/* Check DMI support */
	if (!chuwi_minibook_x_check_dmi_match()) {
		dev_warn(&pdev->dev, "Hardware not fully supported, continuing anyway\n");
	}

	/* Setup accelerometer hardware with mount matrices */
	ret = chuwi_minibook_x_setup_accelerometers();
	if (ret) {
		dev_err(&pdev->dev, "Failed to setup accelerometers: %d\n", ret);
		goto err_cleanup;
	}

	/* Initialize tablet mode detection */
	ret = chuwi_minibook_x_init_tablet_mode(pdev);
	if (ret) {
		dev_err(&pdev->dev, "Failed to initialize tablet mode detection: %d\n", ret);
		goto err_cleanup_accel;
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

err_cleanup_accel:
	/* Accelerometer cleanup would go here if needed */
err_cleanup:
	g_chip = NULL;
	return ret;
}

#ifdef CONFIG_CHUWI_MINIBOOK_X_DEBUGFS
/**
 * chuwi_minibook_x_debugfs_init - Initialize debugfs interface
 */
int chuwi_minibook_x_debugfs_init(struct chuwi_minibook_x *chip)
{
	char dirname[32];
	
	/* Create main debugfs directory */
	snprintf(dirname, sizeof(dirname), "chuwi-minibook-x");
	chip->debugfs_dir = debugfs_create_dir(dirname, NULL);
	if (IS_ERR_OR_NULL(chip->debugfs_dir)) {
		dev_warn(&chip->pdev->dev, "Failed to create debugfs directory\n");
		return -ENODEV;
	}
	
	/* Create debugfs entries for raw data and calculations */
	chip->debugfs_raw_data = debugfs_create_file("raw_data", 0444, chip->debugfs_dir, 
						     chip, NULL);
	chip->debugfs_calculations = debugfs_create_file("calculations", 0444, chip->debugfs_dir,
							  chip, NULL);
	
	dev_info(&chip->pdev->dev, "Debugfs interface created\n");
	return 0;
}

/**
 * chuwi_minibook_x_debugfs_cleanup - Cleanup debugfs interface
 */
void chuwi_minibook_x_debugfs_cleanup(struct chuwi_minibook_x *chip)
{
	if (chip->debugfs_dir) {
		debugfs_remove_recursive(chip->debugfs_dir);
		chip->debugfs_dir = NULL;
		dev_info(&chip->pdev->dev, "Debugfs interface removed\n");
	}
}
#endif

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

	/* Clean up software nodes applied to existing devices (protected by driver_init_lock) */
	if (existing_lid_client) {
		device_remove_software_node(&existing_lid_client->dev);
		pr_info(DRV_NAME ": Removed mount matrix from existing lid device i2c-%d:0x%02x\n",
			existing_lid_client->adapter->nr, existing_lid_client->addr);
		existing_lid_client = NULL;
	}
	
	if (existing_base_client) {
		device_remove_software_node(&existing_base_client->dev);
		pr_info(DRV_NAME ": Removed mount matrix from existing base device i2c-%d:0x%02x\n",
			existing_base_client->adapter->nr, existing_base_client->addr);
		existing_base_client = NULL;
	}

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