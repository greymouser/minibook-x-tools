// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Chuwi Minibook X Platform Driver - Debugfs Interface
 *
 * Copyright (C) 2025 Your Name
 *
 * Debugfs interface for basic debugging.
 * Only compiled when CONFIG_CHUWI_MINIBOOK_X_DEBUGFS is enabled.
 */

#ifdef CONFIG_CHUWI_MINIBOOK_X_DEBUGFS

#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>

#include "chuwi-minibook-x.h"

static struct dentry *chuwi_minibook_x_debugfs_root;

/**
 * Hardware status debugging
 */
static int chuwi_minibook_x_debugfs_hardware_show(struct seq_file *s, void *data)
{
	struct chuwi_minibook_x *chip = s->private;
	int i;

	seq_printf(s, "=== Chuwi Minibook X Hardware Status ===\n");
	seq_printf(s, "Driver enabled: %s\n", chip->enabled ? "yes" : "no");
	seq_printf(s, "Accelerometer count: %d\n", chip->accel_count);
	
	seq_printf(s, "\n=== ACPI Device Information ===\n");
	for (i = 0; i < chip->accel_count && i < 2; i++) {
		seq_printf(s, "Device %d:\n", i);
		seq_printf(s, "  Name: %s\n", chip->accel_i2c_info[i].name);
		seq_printf(s, "  I2C Bus: %d\n", chip->accel_i2c_info[i].bus_nr);
		seq_printf(s, "  Address: 0x%02x\n", chip->accel_i2c_info[i].addr);
	}

	return 0;
}

static int chuwi_minibook_x_debugfs_hardware_open(struct inode *inode, struct file *file)
{
	return single_open(file, chuwi_minibook_x_debugfs_hardware_show, inode->i_private);
}

static const struct file_operations chuwi_minibook_x_debugfs_hardware_fops = {
	.owner		= THIS_MODULE,
	.open		= chuwi_minibook_x_debugfs_hardware_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

/**
 * chuwi_minibook_x_debugfs_init - Initialize debugfs interface
 * @chip: Device context
 *
 * Creates debugfs entries for debugging and monitoring.
 *
 * Returns: 0 on success, negative error code on failure
 */
int chuwi_minibook_x_debugfs_init(struct chuwi_minibook_x *chip)
{
	struct device *dev = &chip->pdev->dev;
	
	/* Create debugfs root directory if it doesn't exist */
	if (!chuwi_minibook_x_debugfs_root) {
		chuwi_minibook_x_debugfs_root = debugfs_create_dir("chuwi-minibook-x", NULL);
		if (!chuwi_minibook_x_debugfs_root) {
			dev_warn(dev, "Failed to create debugfs root directory\n");
			return -ENOMEM;
		}
	}

	/* Create device-specific debugfs directory */
	chip->debugfs_dir = debugfs_create_dir("device", chuwi_minibook_x_debugfs_root);
	if (!chip->debugfs_dir) {
		dev_warn(dev, "Failed to create device debugfs directory\n");
		return -ENOMEM;
	}

	/* Create hardware status file */
	chip->debugfs_raw_data = debugfs_create_file("hardware_status", 0444,
						     chip->debugfs_dir, chip,
						     &chuwi_minibook_x_debugfs_hardware_fops);
	if (!chip->debugfs_raw_data) {
		dev_warn(dev, "Failed to create hardware_status debugfs file\n");
		debugfs_remove_recursive(chip->debugfs_dir);
		return -ENOMEM;
	}

	dev_info(dev, "Minimal debugfs interface created\n");
	return 0;
}

/**
 * chuwi_minibook_x_debugfs_cleanup - Cleanup debugfs interface
 * @chip: Device context
 *
 * Removes all debugfs entries.
 */
void chuwi_minibook_x_debugfs_cleanup(struct chuwi_minibook_x *chip)
{
	struct device *dev = &chip->pdev->dev;

	if (chip->debugfs_dir) {
		debugfs_remove_recursive(chip->debugfs_dir);
		chip->debugfs_dir = NULL;
		chip->debugfs_raw_data = NULL;
		chip->debugfs_calculations = NULL;
	}

	dev_info(dev, "Minimal debugfs interface removed\n");
}

#endif /* CONFIG_CHUWI_MINIBOOK_X_DEBUGFS */