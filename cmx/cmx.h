/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * CMX (Chuwi Minibook X) Integrated Platform Driver
 *
 * Copyright (C) 2025 Armando DiCianno
 *
 * Integrated hardware detection, accelerometer instantiation with mount matrices,
 * and tablet mode detection for the Chuwi Minibook X convertible laptop.
 */

#ifndef _CMX_H_
#define _CMX_H_

#include <linux/types.h>
#include <linux/mutex.h>

#define CMX_DRIVER_NAME "cmx"

/* Module parameters (defined in cmx.c) */
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
struct cmx {
	/* Platform device */
	struct platform_device *pdev;
	
	/* ACPI device information */
	struct accel_i2c_info accel_i2c_info[2];  /* Maximum 2 accelerometers */
	int accel_count;      /* Number of accelerometers found via ACPI */
	
	/* Detected IIO device assignments for userspace daemon */
	char base_iio_device[32];   /* Base IIO device name (e.g., "iio:device0") */
	char lid_iio_device[32];    /* Lid IIO device name (e.g., "iio:device1") */
	
	/* Synchronization */
	struct mutex lock;
	
	/* Module configuration */
	bool debug_mode;      /* Debug output control */
};

/* Function declarations */

/* Main module functions */
int cmx_probe(struct platform_device *pdev);
void cmx_remove(struct platform_device *pdev);

/* Sysfs interface */
int cmx_create_sysfs(struct cmx *chip);
void cmx_remove_sysfs(struct cmx *chip);

#endif /* _CMX_H_ */