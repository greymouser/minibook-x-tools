// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Chuwi Minibook X Platform Driver - Debugfs Interface
 *
 * Copyright (C) 2025 Your Name
 *
 * Debugfs interface for detailed debugging and monitoring.
 * Only compiled when CONFIG_CHUWI_MINIBOOK_X_DEBUGFS is enabled.
 */

#ifdef CONFIG_CHUWI_MINIBOOK_X_DEBUGFS

#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>

#include "chuwi-minibook-x.h"

static struct dentry *chuwi_minibook_x_debugfs_root;

/**
 * debugfs_state_show - Show comprehensive driver state
 */
static int debugfs_state_show(struct seq_file *s, void *data)
{
	struct chuwi_minibook_x *chip = s->private;
	
	mutex_lock(&chip->lock);
	
	seq_printf(s, "=== Chuwi Minibook X Driver State ===\n\n");
	
	/* Basic status */
	seq_printf(s, "Driver Status:\n");
	seq_printf(s, "  Enabled: %s\n", chip->enabled ? "yes" : "no");
	seq_printf(s, "  Poll Interval: %u ms\n", chip->poll_interval_ms);
	seq_printf(s, "  Auto-calibrated: %s\n", chip->auto_calibrated ? "yes" : "no");
	seq_printf(s, "\n");
	
	/* Tablet mode */
	seq_printf(s, "Tablet Mode:\n");
	seq_printf(s, "  Current State: %s\n", chip->tablet_mode ? "tablet" : "laptop");
	seq_printf(s, "  Current Angle: %u°\n", chip->current_angle);
	seq_printf(s, "  Enter Threshold: %u°\n", chip->enter_threshold);
	seq_printf(s, "  Exit Threshold: %u°\n", chip->exit_threshold);
	seq_printf(s, "  Hysteresis: %u°\n", chip->hysteresis);
	seq_printf(s, "  Signed Mode: %s\n", chip->signed_mode ? "yes" : "no");
	seq_printf(s, "  Force Mode: ");
	switch (chip->force_tablet_mode) {
	case 0:
		seq_printf(s, "laptop\n");
		break;
	case 1:
		seq_printf(s, "tablet\n");
		break;
	default:
		seq_printf(s, "auto\n");
		break;
	}
	seq_printf(s, "\n");
	
	/* Hinge axis */
	seq_printf(s, "Hinge Axis:\n");
	seq_printf(s, "  X: %d\n", chip->hinge_axis.x);
	seq_printf(s, "  Y: %d\n", chip->hinge_axis.y);
	seq_printf(s, "  Z: %d\n", chip->hinge_axis.z);
	seq_printf(s, "\n");
	
	/* Orientation configuration */
	seq_printf(s, "Orientation Detection:\n");
	seq_printf(s, "  Enabled: %s\n", chip->orient_config.enabled ? "yes" : "no");
	seq_printf(s, "  Hysteresis: %u°\n", chip->orient_config.hysteresis_deg);
	seq_printf(s, "  Stability Time: %u ms\n", chip->orient_config.stability_ms);
	seq_printf(s, "  Confidence Threshold: %u%%\n", chip->orient_config.confidence_threshold);
	seq_printf(s, "  Force Orientation: ");
	switch (chip->force_orientation) {
	case ORIENTATION_NORMAL:
		seq_printf(s, "normal\n");
		break;
	case ORIENTATION_LEFT:
		seq_printf(s, "left\n");
		break;
	case ORIENTATION_INVERTED:
		seq_printf(s, "inverted\n");
		break;
	case ORIENTATION_RIGHT:
		seq_printf(s, "right\n");
		break;
	default:
		seq_printf(s, "auto\n");
		break;
	}
	seq_printf(s, "\n");
	
	/* Orientation state */
	seq_printf(s, "Orientation State:\n");
	seq_printf(s, "  Current: ");
	switch (chip->orient_state.current_orientation) {
	case ORIENTATION_NORMAL:
		seq_printf(s, "normal\n");
		break;
	case ORIENTATION_LEFT:
		seq_printf(s, "left\n");
		break;
	case ORIENTATION_INVERTED:
		seq_printf(s, "inverted\n");
		break;
	case ORIENTATION_RIGHT:
		seq_printf(s, "right\n");
		break;
	default:
		seq_printf(s, "unknown\n");
		break;
	}
	seq_printf(s, "  Confidence: %u%%\n", chip->orient_state.confidence);
	seq_printf(s, "  Stable: %s\n", chip->orient_state.stable ? "yes" : "no");
	seq_printf(s, "  Changes: %u\n", chip->orient_state.orientation_changes);
	seq_printf(s, "  Rejected: %u\n", chip->orient_state.rejected_changes);
	seq_printf(s, "  Last Change: %lu jiffies ago\n", 
		   jiffies - chip->orient_state.last_change_time);
	seq_printf(s, "\n");
	
	/* Accelerometer data */
	seq_printf(s, "Accelerometer Data:\n");
	seq_printf(s, "  Base (X,Y,Z): %d, %d, %d\n",
		   chip->base_accel.x, chip->base_accel.y, chip->base_accel.z);
	seq_printf(s, "  Lid (X,Y,Z): %d, %d, %d\n",
		   chip->lid_accel.x, chip->lid_accel.y, chip->lid_accel.z);
	
	/* Calculate magnitudes */
	u64 base_mag = chuwi_minibook_x_vec3_magnitude(&chip->base_accel);
	u64 lid_mag = chuwi_minibook_x_vec3_magnitude(&chip->lid_accel);
	seq_printf(s, "  Base Magnitude: %llu\n", base_mag);
	seq_printf(s, "  Lid Magnitude: %llu\n", lid_mag);
	seq_printf(s, "\n");
	
	/* IIO channel status */
	seq_printf(s, "IIO Channels:\n");
	seq_printf(s, "  Base Channels: ");
	if (chip->base_accel_channels[0] && chip->base_accel_channels[1] && chip->base_accel_channels[2]) {
		seq_printf(s, "connected\n");
	} else {
		seq_printf(s, "not connected\n");
	}
	seq_printf(s, "  Lid Channels: ");
	if (chip->lid_accel_channels[0] && chip->lid_accel_channels[1] && chip->lid_accel_channels[2]) {
		seq_printf(s, "connected\n");
	} else {
		seq_printf(s, "not connected\n");
	}
	seq_printf(s, "\n");
	
	mutex_unlock(&chip->lock);
	
	return 0;
}

DEFINE_SHOW_ATTRIBUTE(debugfs_state);

/**
 * debugfs_calibrate_write - Manual hinge calibration trigger
 */
static ssize_t debugfs_calibrate_write(struct file *file,
				       const char __user *user_buf,
				       size_t count, loff_t *ppos)
{
	struct chuwi_minibook_x *chip = file->private_data;
	char buf[32];
	int x, y, z;
	int ret;
	
	if (count >= sizeof(buf))
		return -EINVAL;
		
	if (copy_from_user(buf, user_buf, count))
		return -EFAULT;
		
	buf[count] = '\0';
	
	/* Check for "auto" command */
	if (strncmp(buf, "auto", 4) == 0) {
		mutex_lock(&chip->lock);
		chip->auto_calibrated = false;
		chuwi_minibook_x_auto_calibrate_hinge(chip);
		mutex_unlock(&chip->lock);
		return count;
	}
	
	/* Parse manual calibration: "x y z" */
	ret = sscanf(buf, "%d %d %d", &x, &y, &z);
	if (ret == 3) {
		mutex_lock(&chip->lock);
		chip->hinge_axis.x = x;
		chip->hinge_axis.y = y;
		chip->hinge_axis.z = z;
		chip->auto_calibrated = true;
		mutex_unlock(&chip->lock);
		return count;
	}
	
	return -EINVAL;
}

static const struct file_operations debugfs_calibrate_fops = {
	.open = simple_open,
	.write = debugfs_calibrate_write,
	.llseek = default_llseek,
};

/**
 * debugfs_raw_data_show - Show raw accelerometer readings over time
 */
static int debugfs_raw_data_show(struct seq_file *s, void *data)
{
	struct chuwi_minibook_x *chip = s->private;
	int i;
	
	seq_printf(s, "# Real-time accelerometer data (timestamp, base_x, base_y, base_z, lid_x, lid_y, lid_z, angle)\n");
	
	/* Show 10 samples with 100ms intervals */
	for (i = 0; i < 10; i++) {
		mutex_lock(&chip->lock);
		
		/* Force a reading */
		chuwi_minibook_x_read_accelerometers(chip);
		
		/* Calculate current angle */
		u32 angle = chuwi_minibook_x_calculate_angle(&chip->base_accel,
							     &chip->lid_accel,
							     &chip->hinge_axis,
							     chip->signed_mode);
		
		seq_printf(s, "%lu %d %d %d %d %d %d %u\n",
			   jiffies,
			   chip->base_accel.x, chip->base_accel.y, chip->base_accel.z,
			   chip->lid_accel.x, chip->lid_accel.y, chip->lid_accel.z,
			   angle);
		
		mutex_unlock(&chip->lock);
		
		if (i < 9) {
			msleep(100);
		}
	}
	
	return 0;
}

DEFINE_SHOW_ATTRIBUTE(debugfs_raw_data);

/**
 * debugfs_test_write - Test various driver functions
 */
static ssize_t debugfs_test_write(struct file *file,
				  const char __user *user_buf,
				  size_t count, loff_t *ppos)
{
	struct chuwi_minibook_x *chip = file->private_data;
	char buf[64];
	char command[32];
	int value;
	int ret;
	
	if (count >= sizeof(buf))
		return -EINVAL;
		
	if (copy_from_user(buf, user_buf, count))
		return -EFAULT;
		
	buf[count] = '\0';
	
	ret = sscanf(buf, "%31s %d", command, &value);
	if (ret < 1)
		return -EINVAL;
	
	mutex_lock(&chip->lock);
	
	if (strcmp(command, "tablet") == 0) {
		/* Force tablet mode state */
		if (ret == 2) {
			bool new_state = (value != 0);
			chip->tablet_mode = new_state;
			chuwi_minibook_x_report_tablet_mode(chip, new_state);
		}
	} else if (strcmp(command, "orientation") == 0) {
		/* Force orientation detection */
		if (ret == 2 && value >= 0 && value <= 3) {
			enum orientation new_orient = (enum orientation)value;
			chip->orient_state.current_orientation = new_orient;
			
			/* Report to input subsystem */
			if (chip->input) {
				input_report_abs(chip->input, ABS_MISC, value);
				input_sync(chip->input);
			}
		}
	} else if (strcmp(command, "reset") == 0) {
		/* Reset statistics */
		chip->orient_state.orientation_changes = 0;
		chip->orient_state.rejected_changes = 0;
		chip->orient_state.last_change_time = jiffies;
	} else {
		mutex_unlock(&chip->lock);
		return -EINVAL;
	}
	
	mutex_unlock(&chip->lock);
	return count;
}

static const struct file_operations debugfs_test_fops = {
	.open = simple_open,
	.write = debugfs_test_write,
	.llseek = default_llseek,
};

/**
 * chuwi_minibook_x_debugfs_init - Initialize debugfs interface
 * @chip: Device context
 *
 * Creates debugfs entries for detailed driver debugging.
 * This function only compiles when CONFIG_CHUWI_MINIBOOK_X_DEBUGFS is enabled.
 *
 * Returns: 0 on success, negative error code on failure
 */
int chuwi_minibook_x_debugfs_init(struct chuwi_minibook_x *chip)
{
	struct dentry *dir;
	
	/* Create root directory */
	chuwi_minibook_x_debugfs_root = debugfs_create_dir("chuwi-minibook-x", NULL);
	if (IS_ERR(chuwi_minibook_x_debugfs_root)) {
		dev_err(&chip->pdev->dev, "Failed to create debugfs root directory\n");
		return PTR_ERR(chuwi_minibook_x_debugfs_root);
	}
	
	dir = chuwi_minibook_x_debugfs_root;
	
	/* Create debugfs files */
	debugfs_create_file("state", 0444, dir, chip, &debugfs_state_fops);
	debugfs_create_file("calibrate", 0200, dir, chip, &debugfs_calibrate_fops);
	debugfs_create_file("raw_data", 0444, dir, chip, &debugfs_raw_data_fops);
	debugfs_create_file("test", 0200, dir, chip, &debugfs_test_fops);
	
	/* Create simple value files */
	debugfs_create_u32("poll_interval_ms", 0644, dir, &chip->poll_interval_ms);
	debugfs_create_u32("enter_threshold", 0644, dir, &chip->enter_threshold);
	debugfs_create_u32("exit_threshold", 0644, dir, &chip->exit_threshold);
	debugfs_create_u32("hysteresis", 0644, dir, &chip->hysteresis);
	debugfs_create_bool("signed_mode", 0644, dir, &chip->signed_mode);
	debugfs_create_bool("enabled", 0644, dir, &chip->enabled);
	
	debugfs_create_bool("orient_enabled", 0644, dir, &chip->orient_config.enabled);
	debugfs_create_u32("orient_hysteresis", 0644, dir, &chip->orient_config.hysteresis_deg);
	debugfs_create_u32("orient_stability", 0644, dir, &chip->orient_config.stability_ms);
	debugfs_create_u32("orient_confidence", 0644, dir, &chip->orient_config.confidence_threshold);
	
	dev_info(&chip->pdev->dev, "Debugfs interface created at /sys/kernel/debug/chuwi-minibook-x/\n");
	
	return 0;
}

/**
 * chuwi_minibook_x_debugfs_cleanup - Remove debugfs interface
 * @chip: Device context
 *
 * Removes all debugfs entries created by the driver.
 */
void chuwi_minibook_x_debugfs_cleanup(struct chuwi_minibook_x *chip)
{
	debugfs_remove_recursive(chuwi_minibook_x_debugfs_root);
	chuwi_minibook_x_debugfs_root = NULL;
}

#else /* CONFIG_CHUWI_MINIBOOK_X_DEBUGFS */

/* Stub functions when debugfs is disabled */
int chuwi_minibook_x_debugfs_init(struct chuwi_minibook_x *chip)
{
	return 0;
}

void chuwi_minibook_x_debugfs_cleanup(struct chuwi_minibook_x *chip)
{
}

#endif /* CONFIG_CHUWI_MINIBOOK_X_DEBUGFS */