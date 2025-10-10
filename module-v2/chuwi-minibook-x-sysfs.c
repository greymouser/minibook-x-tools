// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Chuwi Minibook X Platform Driver - Sysfs Interface
 *
 * Copyright (C) 2025 Your Name
 *
 * Sysfs interface for basic status monitoring.
 */

#include <linux/device.h>
#include <linux/sysfs.h>
#include <linux/string.h>
#include <linux/platform_device.h>

#include "chuwi-minibook-x.h"

/* Basic status attributes */

static ssize_t hardware_status_show(struct device *dev,
				    struct device_attribute *attr,
				    char *buf)
{
	struct chuwi_minibook_x *chip = dev_get_drvdata(dev);
	
	return sprintf(buf, "driver_loaded: %s\naccel_count: %d\nenabled: %s\n",
		       "yes",
		       chip->accel_count,
		       chip->enabled ? "yes" : "no");
}

static ssize_t enabled_show(struct device *dev,
			   struct device_attribute *attr,
			   char *buf)
{
	struct chuwi_minibook_x *chip = dev_get_drvdata(dev);
	
	return sprintf(buf, "%d\n", chip->enabled ? 1 : 0);
}

static ssize_t enabled_store(struct device *dev,
			    struct device_attribute *attr,
			    const char *buf, size_t count)
{
	struct chuwi_minibook_x *chip = dev_get_drvdata(dev);
	bool value;
	int ret;
	
	ret = kstrtobool(buf, &value);
	if (ret)
		return ret;
	
	mutex_lock(&chip->lock);
	chip->enabled = value;
	mutex_unlock(&chip->lock);
	
	dev_info(dev, "Driver %s\n", value ? "enabled" : "disabled");
	
	return count;
}

static ssize_t accel_count_show(struct device *dev,
			       struct device_attribute *attr,
			       char *buf)
{
	struct chuwi_minibook_x *chip = dev_get_drvdata(dev);
	
	return sprintf(buf, "%d\n", chip->accel_count);
}

static ssize_t i2c_config_show(struct device *dev,
			      struct device_attribute *attr,
			      char *buf)
{
	return sprintf(buf, "lid_bus: %d\nlid_addr: 0x%02x\nbase_bus: %d\nbase_addr: 0x%02x\n",
		       lid_bus, lid_addr, base_bus, base_addr);
}

static ssize_t sensor_diagnostic_show(struct device *dev,
				     struct device_attribute *attr,
				     char *buf)
{
	int len = 0;
	
	len += sprintf(buf + len, "=== Sensor Diagnostic Guide ===\n");
	len += sprintf(buf + len, "Current I2C mapping:\n");
	len += sprintf(buf + len, "  lid -> i2c-%d:0x%02x (mapped to iio:device0)\n", lid_bus, lid_addr);
	len += sprintf(buf + len, "  base -> i2c-%d:0x%02x (mapped to iio:device1)\n", base_bus, base_addr);
	len += sprintf(buf + len, "\n");
	
	len += sprintf(buf + len, "To verify sensor mapping:\n");
	len += sprintf(buf + len, "1. Use sensor_analysis.sh or sensor_test.sh scripts\n");
	len += sprintf(buf + len, "2. Check sensor readings:\n");
	len += sprintf(buf + len, "   cat /sys/bus/iio/devices/iio:device0/in_accel_*_raw\n");
	len += sprintf(buf + len, "   cat /sys/bus/iio/devices/iio:device1/in_accel_*_raw\n");
	len += sprintf(buf + len, "\n");
	
	len += sprintf(buf + len, "Expected behavior (laptop flat on table):\n");
	len += sprintf(buf + len, "- Both sensors should show gravity on Z-axis\n");
	len += sprintf(buf + len, "- LID sensor: changes when lid moves\n");
	len += sprintf(buf + len, "- BASE sensor: stable when only lid moves\n");
	len += sprintf(buf + len, "\n");
	
	len += sprintf(buf + len, "Note: Your device has 90° rotated display, so sensors\n");
	len += sprintf(buf + len, "may show X or Y dominant instead of Z dominant.\n");
	len += sprintf(buf + len, "This is normal for the hardware orientation.\n");
	
	return len;
}

/* Device attributes */
static DEVICE_ATTR_RO(hardware_status);
static DEVICE_ATTR_RW(enabled);
static DEVICE_ATTR_RO(accel_count);
static DEVICE_ATTR_RO(i2c_config);
static DEVICE_ATTR_RO(sensor_diagnostic);

static struct attribute *chuwi_minibook_x_attrs[] = {
	&dev_attr_hardware_status.attr,
	&dev_attr_enabled.attr,
	&dev_attr_accel_count.attr,
	&dev_attr_i2c_config.attr,
	&dev_attr_sensor_diagnostic.attr,
	NULL
};

static const struct attribute_group chuwi_minibook_x_attr_group = {
	.attrs = chuwi_minibook_x_attrs,
};

/**
 * chuwi_minibook_x_create_sysfs - Create sysfs interface
 * @chip: Device context
 *
 * Creates a minimal sysfs interface for basic status monitoring.
 *
 * Returns: 0 on success, negative error code on failure
 */
int chuwi_minibook_x_create_sysfs(struct chuwi_minibook_x *chip)
{
	struct device *dev = &chip->pdev->dev;
	int ret;

	ret = sysfs_create_group(&dev->kobj, &chuwi_minibook_x_attr_group);
	if (ret) {
		dev_err(dev, "Failed to create sysfs group: %d\n", ret);
		return ret;
	}

	dev_info(dev, "Minimal sysfs interface created\n");
	return 0;
}

/**
 * chuwi_minibook_x_remove_sysfs - Remove sysfs interface
 * @chip: Device context
 *
 * Removes the sysfs interface.
 */
void chuwi_minibook_x_remove_sysfs(struct chuwi_minibook_x *chip)
{
	struct device *dev = &chip->pdev->dev;

	sysfs_remove_group(&dev->kobj, &chuwi_minibook_x_attr_group);
	dev_info(dev, "Minimal sysfs interface removed\n");
}