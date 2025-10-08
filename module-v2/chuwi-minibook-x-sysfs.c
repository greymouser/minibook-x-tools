// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Chuwi Minibook X Platform Driver - Sysfs Interface
 *
 * Copyright (C) 2025 Your Name
 *
 * Sysfs interface for configuration and status monitoring.
 */

#include <linux/device.h>
#include <linux/sysfs.h>
#include <linux/string.h>
#include <linux/platform_device.h>

#include "chuwi-minibook-x.h"

/* Tablet mode configuration attributes */

static ssize_t tablet_enter_threshold_show(struct device *dev,
					   struct device_attribute *attr,
					   char *buf)
{
	struct chuwi_minibook_x *chip = dev_get_drvdata(dev);
	return sprintf(buf, "%u\n", chip->enter_threshold);
}

static ssize_t tablet_enter_threshold_store(struct device *dev,
					    struct device_attribute *attr,
					    const char *buf, size_t count)
{
	struct chuwi_minibook_x *chip = dev_get_drvdata(dev);
	unsigned int value;
	int ret;

	ret = kstrtouint(buf, 10, &value);
	if (ret)
		return ret;

	if (value > 360)
		return -EINVAL;

	mutex_lock(&chip->lock);
	chip->enter_threshold = value;
	mutex_unlock(&chip->lock);

	return count;
}

static ssize_t tablet_exit_threshold_show(struct device *dev,
					  struct device_attribute *attr,
					  char *buf)
{
	struct chuwi_minibook_x *chip = dev_get_drvdata(dev);
	return sprintf(buf, "%u\n", chip->exit_threshold);
}

static ssize_t tablet_exit_threshold_store(struct device *dev,
					   struct device_attribute *attr,
					   const char *buf, size_t count)
{
	struct chuwi_minibook_x *chip = dev_get_drvdata(dev);
	unsigned int value;
	int ret;

	ret = kstrtouint(buf, 10, &value);
	if (ret)
		return ret;

	if (value > 360)
		return -EINVAL;

	mutex_lock(&chip->lock);
	chip->exit_threshold = value;
	mutex_unlock(&chip->lock);

	return count;
}

static ssize_t tablet_hysteresis_show(struct device *dev,
				      struct device_attribute *attr,
				      char *buf)
{
	struct chuwi_minibook_x *chip = dev_get_drvdata(dev);
	return sprintf(buf, "%u\n", chip->hysteresis);
}

static ssize_t tablet_hysteresis_store(struct device *dev,
				       struct device_attribute *attr,
				       const char *buf, size_t count)
{
	struct chuwi_minibook_x *chip = dev_get_drvdata(dev);
	unsigned int value;
	int ret;

	ret = kstrtouint(buf, 10, &value);
	if (ret)
		return ret;

	if (value > 180)
		return -EINVAL;

	mutex_lock(&chip->lock);
	chip->hysteresis = value;
	mutex_unlock(&chip->lock);

	return count;
}

static ssize_t tablet_signed_mode_show(struct device *dev,
				       struct device_attribute *attr,
				       char *buf)
{
	struct chuwi_minibook_x *chip = dev_get_drvdata(dev);
	return sprintf(buf, "%d\n", chip->signed_mode ? 1 : 0);
}

static ssize_t tablet_signed_mode_store(struct device *dev,
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
	chip->signed_mode = value;
	mutex_unlock(&chip->lock);

	return count;
}

static ssize_t tablet_force_mode_show(struct device *dev,
				      struct device_attribute *attr,
				      char *buf)
{
	struct chuwi_minibook_x *chip = dev_get_drvdata(dev);
	const char *mode_str;

	switch (chip->force_tablet_mode) {
	case 0:
		mode_str = "laptop";
		break;
	case 1:
		mode_str = "tablet";
		break;
	default:
		mode_str = "auto";
		break;
	}

	return sprintf(buf, "%s\n", mode_str);
}

static ssize_t tablet_force_mode_store(struct device *dev,
				       struct device_attribute *attr,
				       const char *buf, size_t count)
{
	struct chuwi_minibook_x *chip = dev_get_drvdata(dev);
	char mode_str[16];
	int force_mode;

	if (sscanf(buf, "%15s", mode_str) != 1)
		return -EINVAL;

	if (strcmp(mode_str, "auto") == 0) {
		force_mode = -1;
	} else if (strcmp(mode_str, "laptop") == 0) {
		force_mode = 0;
	} else if (strcmp(mode_str, "tablet") == 0) {
		force_mode = 1;
	} else {
		return -EINVAL;
	}

	mutex_lock(&chip->lock);
	chip->force_tablet_mode = force_mode;
	mutex_unlock(&chip->lock);

	return count;
}

/* Status attributes */

static ssize_t current_angle_show(struct device *dev,
				  struct device_attribute *attr,
				  char *buf)
{
	struct chuwi_minibook_x *chip = dev_get_drvdata(dev);
	return sprintf(buf, "%u\n", chip->current_angle);
}

static ssize_t current_tablet_mode_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct chuwi_minibook_x *chip = dev_get_drvdata(dev);
	return sprintf(buf, "%d\n", chip->tablet_mode ? 1 : 0);
}

static ssize_t poll_interval_show(struct device *dev,
				  struct device_attribute *attr,
				  char *buf)
{
	struct chuwi_minibook_x *chip = dev_get_drvdata(dev);
	return sprintf(buf, "%u\n", chip->poll_interval_ms);
}

static ssize_t poll_interval_store(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t count)
{
	struct chuwi_minibook_x *chip = dev_get_drvdata(dev);
	unsigned int value;
	int ret;

	ret = kstrtouint(buf, 10, &value);
	if (ret)
		return ret;

	if (value < 50 || value > 5000)
		return -EINVAL;

	mutex_lock(&chip->lock);
	chip->poll_interval_ms = value;
	mutex_unlock(&chip->lock);

	return count;
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
	
	if (value) {
		/* Restart polling if enabling */
		schedule_delayed_work(&chip->poll_work, 
				      msecs_to_jiffies(chip->poll_interval_ms));
	} else {
		/* Stop polling if disabling */
		cancel_delayed_work(&chip->poll_work);
	}
	mutex_unlock(&chip->lock);

	return count;
}

/* Orientation configuration attributes */

static ssize_t orientation_enabled_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct chuwi_minibook_x *chip = dev_get_drvdata(dev);
	return sprintf(buf, "%d\n", chip->orient_config.enabled ? 1 : 0);
}

static ssize_t orientation_enabled_store(struct device *dev,
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
	chip->orient_config.enabled = value;
	mutex_unlock(&chip->lock);

	return count;
}

static ssize_t orientation_hysteresis_show(struct device *dev,
					   struct device_attribute *attr,
					   char *buf)
{
	struct chuwi_minibook_x *chip = dev_get_drvdata(dev);
	return sprintf(buf, "%u\n", chip->orient_config.hysteresis_deg);
}

static ssize_t orientation_hysteresis_store(struct device *dev,
					    struct device_attribute *attr,
					    const char *buf, size_t count)
{
	struct chuwi_minibook_x *chip = dev_get_drvdata(dev);
	unsigned int value;
	int ret;

	ret = kstrtouint(buf, 10, &value);
	if (ret)
		return ret;

	if (value > 90)
		return -EINVAL;

	mutex_lock(&chip->lock);
	chip->orient_config.hysteresis_deg = value;
	mutex_unlock(&chip->lock);

	return count;
}

static ssize_t orientation_stability_show(struct device *dev,
					  struct device_attribute *attr,
					  char *buf)
{
	struct chuwi_minibook_x *chip = dev_get_drvdata(dev);
	return sprintf(buf, "%u\n", chip->orient_config.stability_ms);
}

static ssize_t orientation_stability_store(struct device *dev,
					   struct device_attribute *attr,
					   const char *buf, size_t count)
{
	struct chuwi_minibook_x *chip = dev_get_drvdata(dev);
	unsigned int value;
	int ret;

	ret = kstrtouint(buf, 10, &value);
	if (ret)
		return ret;

	if (value > 10000)
		return -EINVAL;

	mutex_lock(&chip->lock);
	chip->orient_config.stability_ms = value;
	mutex_unlock(&chip->lock);

	return count;
}

static ssize_t orientation_confidence_threshold_show(struct device *dev,
						     struct device_attribute *attr,
						     char *buf)
{
	struct chuwi_minibook_x *chip = dev_get_drvdata(dev);
	return sprintf(buf, "%u\n", chip->orient_config.confidence_threshold);
}

static ssize_t orientation_confidence_threshold_store(struct device *dev,
						      struct device_attribute *attr,
						      const char *buf, size_t count)
{
	struct chuwi_minibook_x *chip = dev_get_drvdata(dev);
	unsigned int value;
	int ret;

	ret = kstrtouint(buf, 10, &value);
	if (ret)
		return ret;

	if (value > 100)
		return -EINVAL;

	mutex_lock(&chip->lock);
	chip->orient_config.confidence_threshold = value;
	mutex_unlock(&chip->lock);

	return count;
}

static ssize_t orientation_force_show(struct device *dev,
				      struct device_attribute *attr,
				      char *buf)
{
	struct chuwi_minibook_x *chip = dev_get_drvdata(dev);
	const char *orient_str;

	switch (chip->force_orientation) {
	case ORIENTATION_NORMAL:
		orient_str = "normal";
		break;
	case ORIENTATION_LEFT:
		orient_str = "left";
		break;
	case ORIENTATION_INVERTED:
		orient_str = "inverted";
		break;
	case ORIENTATION_RIGHT:
		orient_str = "right";
		break;
	default:
		orient_str = "auto";
		break;
	}

	return sprintf(buf, "%s\n", orient_str);
}

static ssize_t orientation_force_store(struct device *dev,
				       struct device_attribute *attr,
				       const char *buf, size_t count)
{
	struct chuwi_minibook_x *chip = dev_get_drvdata(dev);
	char orient_str[16];
	int force_orientation;

	if (sscanf(buf, "%15s", orient_str) != 1)
		return -EINVAL;

	if (strcmp(orient_str, "auto") == 0) {
		force_orientation = -1;
	} else if (strcmp(orient_str, "normal") == 0) {
		force_orientation = ORIENTATION_NORMAL;
	} else if (strcmp(orient_str, "left") == 0) {
		force_orientation = ORIENTATION_LEFT;
	} else if (strcmp(orient_str, "inverted") == 0) {
		force_orientation = ORIENTATION_INVERTED;
	} else if (strcmp(orient_str, "right") == 0) {
		force_orientation = ORIENTATION_RIGHT;
	} else {
		return -EINVAL;
	}

	mutex_lock(&chip->lock);
	chip->force_orientation = force_orientation;
	mutex_unlock(&chip->lock);

	return count;
}

/* Orientation status attributes */

static ssize_t orientation_current_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct chuwi_minibook_x *chip = dev_get_drvdata(dev);
	const char *orient_str;

	switch (chip->orient_state.current_orientation) {
	case ORIENTATION_NORMAL:
		orient_str = "normal";
		break;
	case ORIENTATION_LEFT:
		orient_str = "left";
		break;
	case ORIENTATION_INVERTED:
		orient_str = "inverted";
		break;
	case ORIENTATION_RIGHT:
		orient_str = "right";
		break;
	default:
		orient_str = "unknown";
		break;
	}

	return sprintf(buf, "%s\n", orient_str);
}

static ssize_t orientation_confidence_show(struct device *dev,
					   struct device_attribute *attr,
					   char *buf)
{
	struct chuwi_minibook_x *chip = dev_get_drvdata(dev);
	return sprintf(buf, "%u\n", chip->orient_state.confidence);
}

static ssize_t orientation_changes_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct chuwi_minibook_x *chip = dev_get_drvdata(dev);
	return sprintf(buf, "%lu\n", chip->orient_state.orientation_changes);
}

static ssize_t orientation_rejected_show(struct device *dev,
					 struct device_attribute *attr,
					 char *buf)
{
	struct chuwi_minibook_x *chip = dev_get_drvdata(dev);
	return sprintf(buf, "%lu\n", chip->orient_state.rejected_changes);
}

static ssize_t orientation_stable_show(struct device *dev,
				       struct device_attribute *attr,
				       char *buf)
{
	struct chuwi_minibook_x *chip = dev_get_drvdata(dev);
	return sprintf(buf, "%d\n", chip->orient_state.stable ? 1 : 0);
}

/* Raw accelerometer data attributes */

static ssize_t base_accel_show(struct device *dev,
			       struct device_attribute *attr,
			       char *buf)
{
	struct chuwi_minibook_x *chip = dev_get_drvdata(dev);
	return sprintf(buf, "%d %d %d\n", 
		       chip->base_accel.x, chip->base_accel.y, chip->base_accel.z);
}

static ssize_t lid_accel_show(struct device *dev,
			      struct device_attribute *attr,
			      char *buf)
{
	struct chuwi_minibook_x *chip = dev_get_drvdata(dev);
	return sprintf(buf, "%d %d %d\n", 
		       chip->lid_accel.x, chip->lid_accel.y, chip->lid_accel.z);
}

/* Device attribute definitions */
static DEVICE_ATTR_RW(tablet_enter_threshold);
static DEVICE_ATTR_RW(tablet_exit_threshold);
static DEVICE_ATTR_RW(tablet_hysteresis);
static DEVICE_ATTR_RW(tablet_signed_mode);
static DEVICE_ATTR_RW(tablet_force_mode);
static DEVICE_ATTR_RO(current_angle);
static DEVICE_ATTR_RO(current_tablet_mode);
static DEVICE_ATTR_RW(poll_interval);
static DEVICE_ATTR_RW(enabled);

static DEVICE_ATTR_RW(orientation_enabled);
static DEVICE_ATTR_RW(orientation_hysteresis);
static DEVICE_ATTR_RW(orientation_stability);
static DEVICE_ATTR_RW(orientation_confidence_threshold);
static DEVICE_ATTR_RW(orientation_force);
static DEVICE_ATTR_RO(orientation_current);
static DEVICE_ATTR_RO(orientation_confidence);
static DEVICE_ATTR_RO(orientation_changes);
static DEVICE_ATTR_RO(orientation_rejected);
static DEVICE_ATTR_RO(orientation_stable);

static DEVICE_ATTR_RO(base_accel);
static DEVICE_ATTR_RO(lid_accel);

/* Attribute group definition */
static struct attribute *chuwi_minibook_x_attrs[] = {
	/* Tablet mode configuration */
	&dev_attr_tablet_enter_threshold.attr,
	&dev_attr_tablet_exit_threshold.attr,
	&dev_attr_tablet_hysteresis.attr,
	&dev_attr_tablet_signed_mode.attr,
	&dev_attr_tablet_force_mode.attr,
	
	/* Status */
	&dev_attr_current_angle.attr,
	&dev_attr_current_tablet_mode.attr,
	&dev_attr_poll_interval.attr,
	&dev_attr_enabled.attr,
	
	/* Orientation configuration */
	&dev_attr_orientation_enabled.attr,
	&dev_attr_orientation_hysteresis.attr,
	&dev_attr_orientation_stability.attr,
	&dev_attr_orientation_confidence_threshold.attr,
	&dev_attr_orientation_force.attr,
	
	/* Orientation status */
	&dev_attr_orientation_current.attr,
	&dev_attr_orientation_confidence.attr,
	&dev_attr_orientation_changes.attr,
	&dev_attr_orientation_rejected.attr,
	&dev_attr_orientation_stable.attr,
	
	/* Raw data */
	&dev_attr_base_accel.attr,
	&dev_attr_lid_accel.attr,
	
	NULL
};

static const struct attribute_group chuwi_minibook_x_attr_group = {
	.attrs = chuwi_minibook_x_attrs,
};

/**
 * chuwi_minibook_x_create_sysfs - Create sysfs interface
 * @chip: Device context
 *
 * Creates the sysfs attribute group for device configuration and monitoring.
 *
 * Returns: 0 on success, negative error code on failure
 */
int chuwi_minibook_x_create_sysfs(struct chuwi_minibook_x *chip)
{
	return sysfs_create_group(&chip->pdev->dev.kobj, &chuwi_minibook_x_attr_group);
}

/**
 * chuwi_minibook_x_remove_sysfs - Remove sysfs interface
 * @chip: Device context
 *
 * Removes the sysfs attribute group.
 */
void chuwi_minibook_x_remove_sysfs(struct chuwi_minibook_x *chip)
{
	sysfs_remove_group(&chip->pdev->dev.kobj, &chuwi_minibook_x_attr_group);
}