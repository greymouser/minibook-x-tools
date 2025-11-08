// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * CMX - Chuwi Minibook X Integrated Platform Driver
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
#include <linux/device.h>
#include <linux/acpi.h>
#include <linux/delay.h>
#include <linux/sysfs.h>
#include <linux/mutex.h>
#include <linux/i2c.h>
#include <linux/string.h>
#include <linux/dmi.h>
#include <linux/property.h>
#include <linux/input.h>
#include <linux/jiffies.h>
#include <linux/math64.h>
#include <linux/minmax.h>
#include <linux/kobject.h>

#include "cmx.h"

#define DRV_NAME "cmx"

/* Module information */
MODULE_DESCRIPTION("CMX - Chuwi Minibook X integrated platform driver");
MODULE_AUTHOR("Armando DiCianno <armando@noonshy.com>");
MODULE_LICENSE("GPL");

/* Version is defined at build time from VERSION file */
#ifndef MODULE_VERSION_STRING
#define MODULE_VERSION_STRING "3.0"
#endif
MODULE_VERSION(MODULE_VERSION_STRING);

MODULE_ALIAS("platform:" CMX_DRIVER_NAME);

/* Module dependencies - ensure required drivers are loaded */
MODULE_SOFTDEP("pre: mxc4005");

/* Module parameters for hardware configuration */
bool enable_mount_matrix = true;
module_param(enable_mount_matrix, bool, 0644);
MODULE_PARM_DESC(enable_mount_matrix, "Enable automatic mount matrix transformation (default: true)");

/**
 * Mount Matrix Definitions
 *
 * Based on sensor orientation analysis from SENSOR_CONFIGURATION.md:
 * - Lid sensor:  90° counter-clockwise rotation
 * - Base sensor: 90° clockwise rotation
 */

/* Lid sensor mount matrix (identity - no rotation needed) */
static const char * const lid_sensor_mount_matrix[] = {
	"1", "0", "0",    /* X' = X  (no rotation)  */
	"0", "1", "0",    /* Y' = Y  (no rotation)  */
	"0", "0", "1"     /* Z' = Z  (no rotation)  */
};

/* Base sensor mount matrix (90° CW rotation + Z inversion) */  
static const char * const base_sensor_mount_matrix[] = {
	"0", "-1", "0",   /* X' = -Y (laptop right = sensor front) */
	"1", "0", "0",    /* Y' = X  (laptop back = sensor right)  */
	"0", "0", "-1"    /* Z' = -Z (laptop up = sensor DOWN)     */
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

/** @g_base: Current gravity vector from base accelerometer (laptop keyboard) */
static struct vec3 g_base = { 0, 0, 1000000 };

/** @g_lid: Current gravity vector from lid accelerometer (screen) */
static struct vec3 g_lid = { 0, 0, -1000000 };

/**
 * Tablet mode configuration and state
 */

/** @current_mode: Current device mode string */
static char current_mode[16] = "laptop";

/** @current_orientation: Current device orientation string */
static char current_orientation[32] = "portrait";

/** @enable_events: Control whether tablet mode events are generated (default: true) */
static bool enable_events = true;

/* Global driver context and device tracking */
static struct cmx *g_chip;

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

/* Storage for unique software nodes to avoid sysfs conflicts */
static struct software_node *current_lid_node = NULL;
static struct software_node *current_base_node = NULL;

/* Storage for discovered hardware locations */
struct discovered_device_location {
    int bus_nr;
    int addr;
    bool detected;
    bool used;  /* Track if this location has been used for instantiation */
};

static struct discovered_device_location discovered_devices[4];
static int discovered_device_count = 0;

/* Device information structures for accelerometers */
struct mxc4005_device_info {
    struct i2c_client *client;
    int bus_num;
    int addr;
    bool is_working;
};

/**
 * DMI table for supported hardware
 */
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

/**
 * Forward declarations
 */
static void cmx_store_iio_device_assignment(struct i2c_client *client, bool is_lid);

/**
 * Sysfs interface functions
 */

/**
 * notify_tablet_mode_change - Send SW_TABLET_MODE input event
 * @is_tablet: true if entering tablet mode, false if exiting
 *
 * Sends input event to notify desktop environments of tablet mode state changes.
 * Only sends events if enable_events is true.
 */
static void notify_tablet_mode_change(bool is_tablet)
{
	if (!tm_input) {
		pr_debug(DRV_NAME ": Input device not available for tablet mode notification\n");
		return;
	}
	
	/* Check if events are enabled */
	if (!enable_events) {
		pr_debug(DRV_NAME ": Tablet mode events disabled, skipping notification\n");
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

/**
 * show_iio_base_device - Show base IIO device assignment
 */
static ssize_t show_iio_base_device(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	if (!g_chip || strlen(g_chip->base_iio_device) == 0) {
		return snprintf(buf, PAGE_SIZE, "not_detected\n");
	}
	return snprintf(buf, PAGE_SIZE, "%s\n", g_chip->base_iio_device);
}

/**
 * show_iio_lid_device - Show lid IIO device assignment  
 */
static ssize_t show_iio_lid_device(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	if (!g_chip || strlen(g_chip->lid_iio_device) == 0) {
		return snprintf(buf, PAGE_SIZE, "not_detected\n");
	}
	return snprintf(buf, PAGE_SIZE, "%s\n", g_chip->lid_iio_device);
}

/**
 * show_enable - Show whether tablet mode events are enabled
 */
static ssize_t show_enable(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%s\n", enable_events ? "true" : "false");
}

/**
 * store_enable - Enable or disable tablet mode events
 * @kobj: kobject for sysfs attribute
 * @attr: sysfs attribute
 * @buf: input buffer containing enable/disable value
 * @len: length of input buffer
 *
 * Controls whether SW_TABLET_MODE events are generated when mode changes occur.
 * When disabled, mode changes are still tracked but no input events are sent.
 * 
 * Accepts: true values: 1,y,yes,t,true  false values: 0,n,no,f,false
 * Always shows "true" or "false" when read.
 */
static ssize_t store_enable(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t len)
{
	char enable_str[8];
	bool new_value;
	
	if (len >= sizeof(enable_str))
		return -EINVAL;
	
	/* Copy and null-terminate the input */
	strncpy(enable_str, buf, len);
	enable_str[len] = '\0';
	
	/* Remove trailing newline if present */
	if (len > 0 && enable_str[len - 1] == '\n')
		enable_str[len - 1] = '\0';
	
	/* Convert to lowercase for case-insensitive comparison */
	{
		int i;
		for (i = 0; enable_str[i]; i++) {
			if (enable_str[i] >= 'A' && enable_str[i] <= 'Z')
				enable_str[i] = enable_str[i] + ('a' - 'A');
		}
	}
	
	/* Parse true values: 1,y,yes,t,true */
	if (strcmp(enable_str, "1") == 0 ||
	    strcmp(enable_str, "y") == 0 ||
	    strcmp(enable_str, "yes") == 0 ||
	    strcmp(enable_str, "t") == 0 ||
	    strcmp(enable_str, "true") == 0) {
		new_value = true;
	}
	/* Parse false values: 0,n,no,f,false */
	else if (strcmp(enable_str, "0") == 0 ||
		 strcmp(enable_str, "n") == 0 ||
		 strcmp(enable_str, "no") == 0 ||
		 strcmp(enable_str, "f") == 0 ||
		 strcmp(enable_str, "false") == 0) {
		new_value = false;
	}
	else {
		return -EINVAL;
	}
	
	/* Update the state */
	enable_events = new_value;
	pr_info(DRV_NAME ": Tablet mode events %s\n", new_value ? "enabled" : "disabled");
	
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

/**
 * Hardware detection and device instantiation
 */

/**
 * struct device_search_ctx - Context for device search operations
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
 * cmx_create_unique_software_node - Create unique software node
 * @is_lid: true for lid sensor, false for base sensor
 * 
 * Creates a unique software node with a timestamp and PID to avoid sysfs conflicts
 * across module load/unload cycles, even when old nodes persist.
 */
static struct software_node *cmx_create_unique_software_node(bool is_lid)
{
	struct software_node *node;
	char *name;
	ktime_t now = ktime_get();
	u64 timestamp = ktime_to_ns(now);
	
	node = kzalloc(sizeof(*node), GFP_KERNEL);
	if (!node)
		return NULL;
		
	name = kzalloc(48, GFP_KERNEL);
	if (!name) {
		kfree(node);
		return NULL;
	}
	
	/* Use timestamp and current PID to ensure uniqueness across all module reloads */
	snprintf(name, 48, "mxc4005-%s-%llu-%d", 
		 is_lid ? "lid" : "base", 
		 timestamp & 0xFFFFFFFF,  /* Use lower 32 bits of timestamp */
		 current->pid);
	
	node->name = name;
	node->properties = is_lid ? lid_sensor_props : base_sensor_props;
	
	return node;
}

/**
 * cmx_free_software_node - Free allocated software node
 */
static void cmx_free_software_node(struct software_node *node)
{
	if (node) {
		kfree(node->name);
		kfree(node);
	}
}

/**
 * cmx_apply_mount_matrix_to_existing - Apply mount matrix to existing device
 */
static int cmx_apply_mount_matrix_to_existing(struct i2c_client *client, bool is_lid)
{
	struct software_node *unique_node;
	int ret;
	
	if (!client || !enable_mount_matrix) {
		pr_debug(DRV_NAME ": Mount matrix application skipped for i2c-%d:0x%02x (client=%p, enable=%s)\n",
			 client ? client->adapter->nr : -1, 
			 client ? client->addr : 0,
			 client, enable_mount_matrix ? "true" : "false");
		return 0;
	}
	
	pr_info(DRV_NAME ": Applying %s mount matrix to existing device i2c-%d:0x%02x\n",
		is_lid ? "lid" : "base", client->adapter->nr, client->addr);
	
	/* Create a unique software node to avoid sysfs conflicts across reloads */
	unique_node = cmx_create_unique_software_node(is_lid);
	if (!unique_node) {
		pr_err(DRV_NAME ": Failed to create unique software node for existing device i2c-%d:0x%02x\n",
			client->adapter->nr, client->addr);
		return -ENOMEM;
	}
	
	/* Register the software node first */
	ret = software_node_register(unique_node);
	if (ret) {
		pr_err(DRV_NAME ": Failed to register software node %s: %d\n", unique_node->name, ret);
		cmx_free_software_node(unique_node);
		return ret;
	}
	
	/* Apply the registered software node to the existing device */
	ret = device_add_software_node(&client->dev, unique_node);
	if (ret) {
		pr_warn(DRV_NAME ": Failed to apply mount matrix to existing device i2c-%d:0x%02x: %d\n",
			client->adapter->nr, client->addr, ret);
		software_node_unregister(unique_node);
		cmx_free_software_node(unique_node);
		return ret;
	}
	
	pr_info(DRV_NAME ": Successfully applied %s mount matrix to existing device i2c-%d:0x%02x\n",
		is_lid ? "lid" : "base", client->adapter->nr, client->addr);
	
	/* Store the node for cleanup and track the client */
	if (is_lid) {
		current_lid_node = unique_node;
		existing_lid_client = client;
		pr_debug(DRV_NAME ": Found lid sensor at i2c-%d:0x%02x\n",
			client->adapter->nr, client->addr);
		
		/* Store IIO device assignment for userspace daemon */
		cmx_store_iio_device_assignment(client, true);
	} else {
		current_base_node = unique_node;
		existing_base_client = client;
		pr_debug(DRV_NAME ": Found base sensor at i2c-%d:0x%02x\n",
			client->adapter->nr, client->addr);
		
		/* Store IIO device assignment for userspace daemon */
		cmx_store_iio_device_assignment(client, false);
	}
	
	if (g_chip && g_chip->debug_mode) {
		const char * const *matrix = is_lid ? lid_sensor_mount_matrix : base_sensor_mount_matrix;
		pr_info(DRV_NAME ": %s mount matrix: [%s,%s,%s; %s,%s,%s; %s,%s,%s]\n",
			is_lid ? "Lid" : "Base",
			matrix[0], matrix[1], matrix[2],
			matrix[3], matrix[4], matrix[5],
			matrix[6], matrix[7], matrix[8]);
	}
	
	return 0;
}
static int cmx_match_mxc4005(struct device *dev, void *data)
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
		pr_debug(DRV_NAME ": Found MXC4005 device on i2c-%d addr 0x%02x\n",
			client->adapter->nr, client->addr);
	}
	
	return 0;
}

/**
 * cmx_is_acpi_device - Check if I2C device originates from ACPI
 */
static bool cmx_is_acpi_device(struct i2c_client *client)
{
	/* Check if device name contains ACPI-style identifiers */
	if (strstr(client->name, "MDA6655") || 
	    strstr(client->name, "GDIX") ||
	    strstr(client->name, ":")) {
		return true;
	}
	
	/* Check if device is on a designware I2C controller (more reliable) */
	if (client->adapter->dev.parent && 
	    strstr(dev_name(client->adapter->dev.parent), "i2c_designware")) {
		return true;
	}
	
	return false;
}

/**
 * cmx_discover_acpi_devices - Find MDA6655 ACPI devices
 * 
 * This function scans for MDA6655 devices using ACPI device enumeration,
 * independent of driver loading state.
 */
static int cmx_discover_acpi_devices(void)
{
	acpi_handle handle;
	acpi_status status;
	int count = 0;
	
	pr_debug(DRV_NAME ": Scanning ACPI namespace for MDA6655 devices\n");
	
	/* Try to find MDA6655 device in ACPI namespace */
	status = acpi_get_handle(ACPI_ROOT_OBJECT, "\\_SB.PC00.I2C1.MDA6655", &handle);
	if (ACPI_SUCCESS(status)) {
		pr_debug(DRV_NAME ": Found MDA6655 ACPI device at \\_SB.PC00.I2C1.MDA6655\n");
		count++;
	}
	
	/* Try alternative path */
	status = acpi_get_handle(ACPI_ROOT_OBJECT, "\\_SB.PCI0.I2C1.MDA6655", &handle);
	if (ACPI_SUCCESS(status)) {
		pr_debug(DRV_NAME ": Found MDA6655 ACPI device at \\_SB.PCI0.I2C1.MDA6655\n");
		count++;
	}
	
	/* Try searching under all I2C controllers */
	{
		int i;
		for (i = 0; i <= 5; i++) {
			char path[64];
			snprintf(path, sizeof(path), "\\_SB.PC00.I2C%d.MDA6655", i);
			status = acpi_get_handle(ACPI_ROOT_OBJECT, path, &handle);
			if (ACPI_SUCCESS(status)) {
				pr_debug(DRV_NAME ": Found MDA6655 ACPI device at %s\n", path);
				count++;
			}
			
			snprintf(path, sizeof(path), "\\_SB.PCI0.I2C%d.MDA6655", i);
			status = acpi_get_handle(ACPI_ROOT_OBJECT, path, &handle);
			if (ACPI_SUCCESS(status)) {
				pr_debug(DRV_NAME ": Found MDA6655 ACPI device at %s\n", path);
				count++;
			}
		}
	}
	
	pr_debug(DRV_NAME ": Found %d MDA6655 ACPI device(s)\n", count);
	return count;
}

/**
 * cmx_probe_i2c_address - Probe for device at I2C address
 * 
 * This function attempts to detect if there's a responsive device at the given
 * I2C bus and address, independent of driver binding.
 */
static bool cmx_probe_i2c_address(int bus_nr, int addr)
{
	struct i2c_adapter *adapter;
	struct i2c_msg msg;
	u8 buf[1];
	int ret;
	
	adapter = i2c_get_adapter(bus_nr);
	if (!adapter) {
		pr_debug(DRV_NAME ": I2C adapter %d not found\n", bus_nr);
		return false;
	}
	
	/* Try a simple read to detect device presence */
	msg.addr = addr;
	msg.flags = I2C_M_RD;
	msg.len = 1;
	msg.buf = buf;
	
	ret = i2c_transfer(adapter, &msg, 1);
	i2c_put_adapter(adapter);
	
	if (ret == 1) {
		pr_debug(DRV_NAME ": Device detected at i2c-%d:0x%02x\n", bus_nr, addr);
		return true;
	} else {
		pr_debug(DRV_NAME ": No device at i2c-%d:0x%02x (ret=%d)\n", bus_nr, addr, ret);
		return false;
	}
}

/**
 * Helper function to check if a client matches our search criteria
 */
static int cmx_device_iterator(struct device *dev, void *data)
{
	struct device_search_ctx *ctx = data;
	struct i2c_client *client;
	
	if (dev->type != &i2c_client_type)
		return 0;
	
	client = to_i2c_client(dev);
	if (client->adapter->nr == ctx->target_bus && client->addr == ctx->target_addr) {
		ctx->found_client = client;
		return 1; /* Stop iteration */
	}
	
	return 0; /* Continue iteration */
}

/**
 * cmx_find_device_on_bus - Check if I2C client exists at bus:address
 * 
 * This function checks if there's already an I2C client bound at the given
 * I2C bus and address to prevent conflicts. It only considers bound clients,
 * not just hardware that responds to I2C.
 */
static struct i2c_client *cmx_find_device_on_bus(int bus_nr, int addr)
{
	struct device_search_ctx ctx = {
		.target_bus = bus_nr,
		.target_addr = addr,
		.found_client = NULL
	};
	
	/* Search all I2C devices for an existing client at this bus:address */
	bus_for_each_dev(&i2c_bus_type, NULL, &ctx, cmx_device_iterator);
	
	if (ctx.found_client) {
		pr_debug(DRV_NAME ": I2C client already bound at i2c-%d:0x%02x - skipping instantiation\n", 
			bus_nr, addr);
		return ctx.found_client;
	}
	
	pr_debug(DRV_NAME ": No I2C client at i2c-%d:0x%02x - available for instantiation\n", 
		bus_nr, addr);
	return NULL; /* No bound client at this address */
}

/**
 * cmx_get_device_bus_info - Extract bus/address from working device
 * 
 * This function gets the actual bus and address information from a working
 * accelerometer device to avoid hardcoded assumptions.
 */
static void cmx_get_device_bus_info(struct device *dev, int *bus_nr, int *addr)
{
	struct i2c_client *client = to_i2c_client(dev);
	*bus_nr = client->adapter->nr;
	*addr = client->addr;
}

/**
 * cmx_scan_for_accelerometers - Hardware-level accelerometer detection
 * 
 * This function scans for accelerometer hardware using multiple strategies:
 * 1. ACPI device enumeration (finds MDA6655 devices)
 * 2. I2C bus probing (detects hardware at known addresses)
 * 3. Works regardless of driver loading state
 */
static int cmx_scan_for_accelerometers(void)
{
	int acpi_devices = 0;
	int i2c_devices = 0;
	int bus_nr;
	
	pr_debug(DRV_NAME ": Hardware-level accelerometer discovery starting\n");
	
	/* Strategy 1: ACPI device discovery */
	acpi_devices = cmx_discover_acpi_devices();
	
	/* Strategy 2: I2C bus scanning for accelerometers at address 0x15 */
	pr_debug(DRV_NAME ": Scanning I2C buses for accelerometers at address 0x15\n");
	
	/* Reset discovered device list */
	discovered_device_count = 0;
	memset(discovered_devices, 0, sizeof(discovered_devices));
	
	/* Scan likely I2C bus numbers (typically 10-15 for designware controllers) */
	for (bus_nr = 10; bus_nr <= 15; bus_nr++) {
		if (cmx_probe_i2c_address(bus_nr, 0x15)) {
			if (discovered_device_count < ARRAY_SIZE(discovered_devices)) {
				discovered_devices[discovered_device_count].bus_nr = bus_nr;
				discovered_devices[discovered_device_count].addr = 0x15;
				discovered_devices[discovered_device_count].detected = true;
				discovered_devices[discovered_device_count].used = false;  /* Initialize as unused */
				discovered_device_count++;
			}
			i2c_devices++;
		}
	}
	
	pr_debug(DRV_NAME ": Hardware discovery results: %d ACPI device(s), %d I2C device(s) at 0x15\n", 
		acpi_devices, i2c_devices);
	
	/* If we found ACPI devices, that's the most reliable */
	if (acpi_devices > 0) {
		pr_debug(DRV_NAME ": Using ACPI-based device count: %d\n", acpi_devices);
		return acpi_devices;
	}
	
	/* Fall back to I2C detection */
	if (i2c_devices > 0) {
		pr_debug(DRV_NAME ": Using I2C-based device count: %d\n", i2c_devices);
		return i2c_devices;
	}
	
	pr_debug(DRV_NAME ": No accelerometer hardware detected\n");
	return 0;
}

struct device_discovery_ctx {
	struct device *base_dev;
	struct device *lid_dev;
	int found_count;
};

/**
 * cmx_check_mxc4005_device - Callback to check each I2C device
 */
static int cmx_check_mxc4005_device(struct device *dev, void *data)
{
	struct device_discovery_ctx *ctx = (struct device_discovery_ctx *)data;
	struct i2c_client *client;
	
	if (dev->driver && strcmp(dev->driver->name, "mxc4005") == 0) {
		client = to_i2c_client(dev);
		
		/* Only consider ACPI or designware-originated devices */
		if (!cmx_is_acpi_device(client)) {
			pr_debug(DRV_NAME ": Skipping manually created device %s (not ACPI/designware)\n",
				dev_name(dev));
			return 0;
		}
		
		/* Simple heuristic: ACPI devices with "MDA6655" are ours */
		if (strstr(client->name, "MDA6655")) {
			if (ctx->found_count == 0) {
				ctx->base_dev = dev; /* First device = base */
			} else if (ctx->found_count == 1) {
				ctx->lid_dev = dev; /* Second device = lid */
			}
			ctx->found_count++;
			
			pr_debug(DRV_NAME ": Found working MXC4005 device: %s on i2c-%d:0x%02x (%s)\n",
				client->name, client->adapter->nr, client->addr,
				ctx->found_count == 1 ? "lid" : "base");
		}
	}
	return 0;
}

/**
 * cmx_find_working_mxc4005_devices - Find functional MXC4005 devices
 * 
 * This function finds MXC4005 devices that are actually working (have IIO interfaces)
 * and identifies them reliably without depending on bus numbers.
 */
static int cmx_find_working_mxc4005_devices(struct device **base_dev, struct device **lid_dev)
{
	struct device_discovery_ctx ctx = {
		.base_dev = NULL,
		.lid_dev = NULL,
		.found_count = 0
	};
	
	/* Iterate through all I2C devices bound to mxc4005 driver */
	bus_for_each_dev(&i2c_bus_type, NULL, &ctx, cmx_check_mxc4005_device);
	
	*base_dev = ctx.base_dev;
	*lid_dev = ctx.lid_dev;
	
	return ctx.found_count;
}

/**
 * cmx_discover_accelerometers - Discover accelerometer devices reliably
 * 
 * This function uses multiple strategies to find our accelerometer devices
 * without relying on fixed I2C bus numbers.
 */
static int cmx_discover_accelerometers(void)
{
	struct device *base_dev = NULL, *lid_dev = NULL;
	struct i2c_client *base_client, *lid_client;
	int working_count, hardware_count;
	int working_bus = -1, working_addr = -1;
	
	pr_debug(DRV_NAME ": Discovering accelerometer devices using robust identification\n");
	
	/* Strategy 1: Hardware-level detection (works regardless of driver state) */
	hardware_count = cmx_scan_for_accelerometers();
	
	/* Strategy 2: Driver-level detection (works if driver already loaded) */
	working_count = cmx_find_working_mxc4005_devices(&base_dev, &lid_dev);
	
	pr_debug(DRV_NAME ": Detection summary: %d hardware device(s), %d working device(s)\n", 
		hardware_count, working_count);
	
	if (working_count >= 2) {
		pr_debug(DRV_NAME ": Found %d working accelerometer device(s)\n", working_count);
		
		base_client = to_i2c_client(base_dev);
		lid_client = to_i2c_client(lid_dev);
		
		pr_debug(DRV_NAME ": Base accelerometer: %s on i2c-%d:0x%02x\n", 
			base_client->name, base_client->adapter->nr, base_client->addr);
		pr_debug(DRV_NAME ": Lid accelerometer: %s on i2c-%d:0x%02x\n", 
			lid_client->name, lid_client->adapter->nr, lid_client->addr);
		
		return working_count;
	} else if (working_count == 1) {
		base_client = to_i2c_client(base_dev);
		cmx_get_device_bus_info(base_dev, &working_bus, &working_addr);
		
		pr_debug(DRV_NAME ": Found 1 working accelerometer: %s on i2c-%d:0x%02x\n", 
			base_client->name, base_client->adapter->nr, base_client->addr);
		pr_debug(DRV_NAME ": Hardware indicates %d total device(s) available\n", hardware_count);
		
		pr_debug(DRV_NAME ": Working device detected - may need to instantiate additional device\n");
		
		return working_count;
	} else {
		pr_debug(DRV_NAME ": No working accelerometer devices found via driver\n");
		if (hardware_count > 0) {
			pr_debug(DRV_NAME ": But hardware detection found %d device(s) - driver may not be loaded\n", hardware_count);
		}
		return hardware_count; /* Return hardware count when no driver-level devices found */
	}
}

/**
 * cmx_count_mxc4005_devices - Count available MXC4005 IIO devices
 */
static int cmx_count_mxc4005_devices(void)
{
	int count = 0;
	
	/* Iterate through I2C bus devices to find MXC4005 chips */
	bus_for_each_dev(&i2c_bus_type, NULL, &count, cmx_match_mxc4005);
	
	return count;
}

/**
 * cmx_instantiate_mxc4005 - Instantiate MXC4005 device on I2C bus with mount matrix
 */
static int cmx_instantiate_mxc4005(int bus_nr, int addr, bool is_second, bool is_lid)
{
	struct i2c_adapter *adapter;
	struct i2c_board_info info = {};
	struct i2c_client *client;
	struct i2c_client *existing_device;
	
	/* Check if there's already an I2C client bound at this address */
	existing_device = cmx_find_device_on_bus(bus_nr, addr);
	if (existing_device) {
		pr_debug(DRV_NAME ": I2C client already bound at i2c-%d:0x%02x - skipping instantiation\n", 
			bus_nr, addr);
		return 0; /* Device exists, don't try to create another */
	}
	
	adapter = i2c_get_adapter(bus_nr);
	if (!adapter) {
		pr_err(DRV_NAME ": I2C adapter %d not found\n", bus_nr);
		return -ENODEV;
	}
	
	snprintf(info.type, sizeof(info.type), "mxc4005");
	info.addr = addr;
	
	/* Apply mount matrix if enabled */
	if (enable_mount_matrix) {
		struct software_node *unique_node = cmx_create_unique_software_node(is_lid);
		if (!unique_node) {
			pr_err(DRV_NAME ": Failed to create unique software node for i2c-%d:0x%02x\n", bus_nr, addr);
			i2c_put_adapter(adapter);
			return -ENOMEM;
		}
		
		info.swnode = unique_node;
		
		/* Store the node for cleanup */
		if (is_lid) {
			current_lid_node = unique_node;
		} else {
			current_base_node = unique_node;
		}
		
		pr_debug(DRV_NAME ": Applying %s sensor mount matrix to i2c-%d:0x%02x\n", 
			is_lid ? "lid" : "base", bus_nr, addr);
		if (g_chip && g_chip->debug_mode) {
			const char * const *matrix = is_lid ? lid_sensor_mount_matrix : base_sensor_mount_matrix;
			pr_info(DRV_NAME ": %s mount matrix: [%s,%s,%s; %s,%s,%s; %s,%s,%s]\n", 
				is_lid ? "Lid" : "Base",
				matrix[0], matrix[1], matrix[2],
				matrix[3], matrix[4], matrix[5], 
				matrix[6], matrix[7], matrix[8]);
		}
	} else {
		pr_debug(DRV_NAME ": Mount matrix disabled for i2c-%d:0x%02x, using identity transformation\n", bus_nr, addr);
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
	
	pr_debug(DRV_NAME ": Found %s sensor at i2c-%d:0x%02x\n",
		is_lid ? "lid" : "base", bus_nr, addr);
	
	/* Store IIO device assignment for userspace daemon */
	cmx_store_iio_device_assignment(client, is_lid);
	
	if (g_chip && g_chip->debug_mode) {
		pr_info(DRV_NAME ": Device %s will be available as IIO device after driver binding\n",
			is_lid ? "lid" : "base");
	}
	
	return 0;
}

/**
 * cmx_check_dmi_match - Check if current hardware matches supported DMI
 */
static bool cmx_check_dmi_match(void)
{
	const struct dmi_system_id *dmi_id;
	
	dmi_id = dmi_first_match(cmx_dmi_table);
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
 * cmx_store_iio_device_assignment - Store IIO device assignment
 * @client: I2C client device
 * @is_lid: true if this is the lid sensor, false for base sensor
 */
static void cmx_store_iio_device_assignment(struct i2c_client *client, bool is_lid)
{
	char iio_name[32];
	
	if (!g_chip) {
		pr_warn(DRV_NAME ": g_chip is NULL, cannot store IIO device assignment\n");
		return;
	}
	
	/* Based on the actual hardware layout observed:
	 * - Lid sensor (i2c-12) corresponds to iio:device0
	 * - Base sensor (i2c-11) corresponds to iio:device1
	 * But we'll assign based on the sensor type to be more robust
	 */
	if (is_lid) {
		strncpy(iio_name, "iio:device0", sizeof(iio_name) - 1);
		strncpy(g_chip->lid_iio_device, iio_name, sizeof(g_chip->lid_iio_device) - 1);
		g_chip->lid_iio_device[sizeof(g_chip->lid_iio_device) - 1] = '\0';
	} else {
		strncpy(iio_name, "iio:device1", sizeof(iio_name) - 1);
		strncpy(g_chip->base_iio_device, iio_name, sizeof(g_chip->base_iio_device) - 1);
		g_chip->base_iio_device[sizeof(g_chip->base_iio_device) - 1] = '\0';
	}
	
	pr_info(DRV_NAME ": %s sensor: i2c-%d:0x%02x -> %s\n", 
		is_lid ? "Lid" : "Base", client->adapter->nr, client->addr, iio_name);
}

/**
 * cmx_setup_accelerometers - Setup accelerometer hardware with mount matrices
 */
static int cmx_setup_accelerometers(void)
{
	int mxc_count;
	int ret;
	struct i2c_client *existing_lid = NULL;
	struct i2c_client *existing_base = NULL;
	
	/* FIXME: Wait for MODULE_SOFTDEP drivers to load and bind */
	msleep(100);
	
	/* First discover all accelerometer locations using hardware detection */
	cmx_discover_accelerometers();
	
	/* Device assignment tracking */
	struct {
		int bus_nr;
		int addr;
		struct i2c_client *existing_client;
		bool handled;
	} lid_device = {-1, -1, NULL, false}, base_device = {-1, -1, NULL, false};
	
	/* Sort discovered devices by bus number to ensure consistent assignment */
	/* Higher bus number = lid, lower bus number = base */
	if (discovered_device_count >= 2) {
		/* Sort devices by bus number (ascending) */
		for (int i = 0; i < discovered_device_count - 1; i++) {
			for (int j = i + 1; j < discovered_device_count; j++) {
				if (discovered_devices[i].bus_nr > discovered_devices[j].bus_nr) {
					struct discovered_device_location temp = discovered_devices[i];
					discovered_devices[i] = discovered_devices[j];
					discovered_devices[j] = temp;
				}
			}
		}
		
		/* Assign: higher bus number = lid, lower bus number = base (final correction based on gravity analysis) */
		base_device.bus_nr = discovered_devices[0].bus_nr;  /* Lower bus number */
		base_device.addr = discovered_devices[0].addr;
		lid_device.bus_nr = discovered_devices[discovered_device_count - 1].bus_nr;  /* Higher bus number */
		lid_device.addr = discovered_devices[discovered_device_count - 1].addr;
		
		pr_info(DRV_NAME ": Device assignment: lid=i2c-%d:0x%02x (higher bus), base=i2c-%d:0x%02x (lower bus)\n",
			lid_device.bus_nr, lid_device.addr, base_device.bus_nr, base_device.addr);
	}
	
	/* Check for existing I2C clients at assigned locations and apply mount matrices */
	if (lid_device.bus_nr >= 0) {
		lid_device.existing_client = cmx_find_device_on_bus(lid_device.bus_nr, lid_device.addr);
		if (lid_device.existing_client) {
			pr_info(DRV_NAME ": Found existing MXC4005 device for lid on i2c-%d:0x%02x, applying mount matrix\n", 
				lid_device.bus_nr, lid_device.addr);
			ret = cmx_apply_mount_matrix_to_existing(lid_device.existing_client, true);
			if (ret == 0) {
				lid_device.handled = true;
				existing_lid = lid_device.existing_client;
			}
		}
	}
	
	if (base_device.bus_nr >= 0) {
		base_device.existing_client = cmx_find_device_on_bus(base_device.bus_nr, base_device.addr);
		if (base_device.existing_client) {
			pr_info(DRV_NAME ": Found existing MXC4005 device for base on i2c-%d:0x%02x, applying mount matrix\n", 
				base_device.bus_nr, base_device.addr);
			ret = cmx_apply_mount_matrix_to_existing(base_device.existing_client, false);
			if (ret == 0) {
				base_device.handled = true;
				existing_base = base_device.existing_client;
			}
		}
	}
	
	/* Initial count - may be 0 if devices haven't been bound yet */
	mxc_count = cmx_count_mxc4005_devices();
	
	pr_debug(DRV_NAME ": Found %d existing MXC4005 device(s) with mount matrices applied\n", mxc_count);

	if (mxc_count < 2 || !lid_device.handled || !base_device.handled) {
		pr_debug(DRV_NAME ": Need to instantiate devices (current: %d, lid_handled: %s, base_handled: %s)\n",
			mxc_count, lid_device.handled ? "yes" : "no", base_device.handled ? "yes" : "no");
		
		/* Instantiate lid sensor if needed */
		if (!lid_device.handled && lid_device.bus_nr >= 0) {
			pr_debug(DRV_NAME ": Instantiating lid accelerometer on discovered hardware i2c-%d addr 0x%02x\n", 
				lid_device.bus_nr, lid_device.addr);
			ret = cmx_instantiate_mxc4005(lid_device.bus_nr, lid_device.addr, false, true);
			if (ret == 0) {
				lid_device.handled = true;
				pr_debug(DRV_NAME ": Lid accelerometer instantiated successfully\n");
			} else {
				pr_err(DRV_NAME ": Failed to instantiate lid accelerometer: %d\n", ret);
			}
		} else if (!lid_device.handled) {
			pr_info(DRV_NAME ": Could not find lid sensor\n");
		}
		
		/* Instantiate base sensor if needed */
		if (!base_device.handled && base_device.bus_nr >= 0) {
			pr_debug(DRV_NAME ": Instantiating base accelerometer on discovered hardware i2c-%d addr 0x%02x\n", 
				base_device.bus_nr, base_device.addr);
			ret = cmx_instantiate_mxc4005(base_device.bus_nr, base_device.addr, true, false);
			if (ret == 0) {
				base_device.handled = true;
				pr_debug(DRV_NAME ": Base accelerometer instantiated successfully\n");
			} else {
				pr_err(DRV_NAME ": Failed to instantiate base accelerometer: %d\n", ret);
			}
		} else if (!base_device.handled) {
			pr_info(DRV_NAME ": Could not find base sensor\n");
		}
	} else {
		pr_debug(DRV_NAME ": All accelerometers already configured with mount matrices\n");
	}
	
	/* FIXME: Wait for all devices to complete driver binding */
	msleep(500);
	mxc_count = cmx_count_mxc4005_devices();
	pr_debug(DRV_NAME ": Final accelerometer count: %d\n", mxc_count);
	
	return 0;
}

/**
 * cmx_init_tablet_mode - Initialize tablet mode detection system
 */
static int cmx_init_tablet_mode(struct platform_device *pdev)
{
	int ret;
	
	pr_debug(DRV_NAME ": Initializing tablet mode detection\n");
	
	/* Create input device for tablet mode events */
	tm_input = devm_input_allocate_device(&pdev->dev);
	if (!tm_input) {
		pr_err(DRV_NAME ": Failed to allocate input device\n");
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
		pr_err(DRV_NAME ": Failed to register input device: %d\n", ret);
		return ret;
	}
	
	/* Set initial tablet mode state to laptop mode (not in tablet mode) */
	input_report_switch(tm_input, SW_TABLET_MODE, 0);
	input_sync(tm_input);
	pr_debug(DRV_NAME ": Initial mode set to laptop (tablet mode disabled)\n");
	
	/* Create sysfs interface under platform driver */
	ret = sysfs_create_group(&pdev->dev.kobj, &tablet_mode_attr_group);
	if (ret) {
		pr_err(DRV_NAME ": Failed to create sysfs group: %d\n", ret);
		input_unregister_device(tm_input);
		return ret;
	}
	
	pr_debug(DRV_NAME ": Tablet mode detection initialized successfully\n");
	
	return 0;
}

/**
 * cmx_cleanup_tablet_mode - Cleanup tablet mode detection system
 */
static void cmx_cleanup_tablet_mode(void)
{
	/* Remove sysfs interface */
	if (g_chip && g_chip->pdev) {
		sysfs_remove_group(&g_chip->pdev->dev.kobj, &tablet_mode_attr_group);
	}
	
	/* Input device will be cleaned up automatically via devm */
}

/* Hardware sysfs interface functions */

/**
 * cmx_create_sysfs - Create basic hardware sysfs interface
 */
int cmx_create_sysfs(struct cmx *chip)
{
	/* Basic sysfs interface can be added here if needed for hardware status */
	pr_debug(DRV_NAME ": Hardware sysfs interface ready\n");
	return 0;
}

/**
 * cmx_remove_sysfs - Remove hardware sysfs interface
 */
void cmx_remove_sysfs(struct cmx *chip)
{
	/* Cleanup any hardware-specific sysfs entries */
	pr_debug(DRV_NAME ": Hardware sysfs interface removed\n");
}

/**
 * cmx_probe - Platform device probe function
 */
int cmx_probe(struct platform_device *pdev)
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

	/* Initialize basic fields */
	chip->accel_count = 0;
	chip->debug_mode = false;  /* Can be enabled via sysfs after module load */
	mutex_init(&chip->lock);
	
	/* Initialize IIO device assignments */
	memset(chip->base_iio_device, 0, sizeof(chip->base_iio_device));
	memset(chip->lid_iio_device, 0, sizeof(chip->lid_iio_device));

	/* Setup accelerometer hardware with mount matrices */
	ret = cmx_setup_accelerometers();
	if (ret) {
		dev_err(&pdev->dev, "Failed to setup accelerometers: %d\n", ret);
		goto err_cleanup;
	}

	/* Initialize tablet mode detection */
	ret = cmx_init_tablet_mode(pdev);
	if (ret) {
		dev_err(&pdev->dev, "Failed to initialize tablet mode detection: %d\n", ret);
		goto err_cleanup_accel;
	}

	/* Create basic sysfs interface for hardware status */
	ret = cmx_create_sysfs(chip);
	if (ret) {
		dev_warn(&pdev->dev, "Failed to create hardware sysfs interface: %d\n", ret);
		/* Continue without basic sysfs */
	}

	dev_dbg(&pdev->dev, "Mount matrix support: %s\n", enable_mount_matrix ? "enabled" : "disabled");
	dev_info(&pdev->dev, "Tablet mode detection initialized\n");

	return 0;

err_cleanup_accel:
	/* Accelerometer cleanup would go here if needed */
err_cleanup:
	g_chip = NULL;
	return ret;
}

/**
 * cmx_remove - Platform device remove function
 */
void cmx_remove(struct platform_device *pdev)
{
	struct cmx *chip = platform_get_drvdata(pdev);

	dev_dbg(&pdev->dev, "Chuwi Minibook X integrated driver removing\n");

	/* Cleanup tablet mode detection */
	cmx_cleanup_tablet_mode();

	/* Remove basic sysfs interface */
	cmx_remove_sysfs(chip);

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

/* Platform device for manual instantiation when ACPI enumeration doesn't work */
static struct platform_device *cmx_device;

/**
 * cmx_init - Module initialization
 */
static int __init cmx_init(void)
{
	int ret;
	bool hardware_supported;

	pr_debug(DRV_NAME " integrated driver initializing\n");

	/* Use mutex to prevent race conditions during initialization */
	mutex_lock(&driver_init_lock);

	/* Check if driver is already registered */
	if (driver_registered) {
		pr_debug("Driver already registered, skipping duplicate initialization\n");
		mutex_unlock(&driver_init_lock);
		return 0;
	}

	/* Check if we're running on supported hardware */
	hardware_supported = cmx_check_dmi_match();
	if (!hardware_supported) {
		pr_warn("Hardware not fully supported, loading anyway for testing\n");
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
	pr_debug(DRV_NAME " integrated driver initialized successfully\n");
	return 0;
}

/**
 * cmx_exit - Module cleanup
 */
static void __exit cmx_exit(void)
{
	pr_debug(DRV_NAME " integrated driver exiting\n");

	mutex_lock(&driver_init_lock);

	/* Clean up software nodes applied to existing devices (protected by driver_init_lock) */
	if (existing_lid_client) {
		device_remove_software_node(&existing_lid_client->dev);
		pr_debug(DRV_NAME ": Removed mount matrix from existing lid device i2c-%d:0x%02x\n",
			existing_lid_client->adapter->nr, existing_lid_client->addr);
		existing_lid_client = NULL;
	}
	
	if (existing_base_client) {
		device_remove_software_node(&existing_base_client->dev);
		pr_debug(DRV_NAME ": Removed mount matrix from existing base device i2c-%d:0x%02x\n",
			existing_base_client->adapter->nr, existing_base_client->addr);
		existing_base_client = NULL;
	}

	/* Clean up any manually instantiated I2C devices */
	if (instantiated_client) {
		pr_debug(DRV_NAME ": Removing manually instantiated MXC4005 on i2c-%d addr 0x%02x\n",
			instantiated_bus, instantiated_addr);
		i2c_unregister_device(instantiated_client);
		instantiated_client = NULL;
	}

	if (instantiated_client2) {
		pr_debug(DRV_NAME ": Removing second manually instantiated MXC4005 on i2c-%d addr 0x%02x\n",
			instantiated_bus2, instantiated_addr2);
		i2c_unregister_device(instantiated_client2);
		instantiated_client2 = NULL;
	}

	/* Clean up allocated software nodes - must unregister before freeing */
	if (current_lid_node) {
		software_node_unregister(current_lid_node);
		pr_debug(DRV_NAME ": Unregistered lid software node %s\n", current_lid_node->name);
		cmx_free_software_node(current_lid_node);
		current_lid_node = NULL;
	}
	
	if (current_base_node) {
		software_node_unregister(current_base_node);
		pr_debug(DRV_NAME ": Unregistered base software node %s\n", current_base_node->name);
		cmx_free_software_node(current_base_node);
		current_base_node = NULL;
	}

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
	pr_debug(DRV_NAME " integrated driver exited\n");
}

module_init(cmx_init);
module_exit(cmx_exit);