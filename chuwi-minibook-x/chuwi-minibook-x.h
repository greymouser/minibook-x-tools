/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Chuwi Minibook X Integrated Platform Driver
 *
 * Copyright (C) 2025 Armando DiCianno
 *
 * Integrated hardware detection, accelerometer instantiation with mount matrices,
 * and tablet mode detection for the Chuwi Minibook X convertible laptop.
 */

#ifndef _CHUWI_MINIBOOK_X_H_
#define _CHUWI_MINIBOOK_X_H_

#include <linux/types.h>
#include <linux/mutex.h>

#define CHUWI_MINIBOOK_X_DRIVER_NAME "chuwi-minibook-x"

/* Module parameters (defined in chuwi-minibook-x.c) */
extern int lid_bus;
extern int lid_addr;
extern int base_bus;
extern int base_addr;
extern bool enable_mount_matrix;

/* ACPI device information structure */
struct accel_i2c_info {
	int bus_nr;
	int addr;
	char name[32];
};

/* Driver context structure */
struct chuwi_minibook_x {
	/* Platform device */
	struct platform_device *pdev;
	
	/* ACPI device information */
	struct accel_i2c_info accel_i2c_info[2];  /* Maximum 2 accelerometers */
	int accel_count;      /* Number of accelerometers found via ACPI */
	
	/* Synchronization */
	struct mutex lock;
	
	/* Module configuration */
	bool debug_mode;      /* Debug output control */
};

/* Function declarations */

/* Main module functions */
int chuwi_minibook_x_probe(struct platform_device *pdev);
void chuwi_minibook_x_remove(struct platform_device *pdev);

/* Sysfs interface */
int chuwi_minibook_x_create_sysfs(struct chuwi_minibook_x *chip);
void chuwi_minibook_x_remove_sysfs(struct chuwi_minibook_x *chip);

#endif /* _CHUWI_MINIBOOK_X_H_ */