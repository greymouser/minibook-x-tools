/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Device Orientation Detection Module
 * 
 * Provides dual-sensor orientation detection for the Chuwi Minibook X,
 * supporting both single accelerometer and dual-accelerometer configurations.
 * Includes tablet mode awareness and tilt-based sensor switching.
 * 
 * Copyright (c) 2025 Armando DiCianno <armando@noonshy.com>
 */

#ifndef CMXD_ORIENTATION_H
#define CMXD_ORIENTATION_H

#include <stdint.h>
#include <stdbool.h>

#define CMXD_ORIENTATION_UNKNOWN -1

/* Raw device orientation codes based on accelerometer dominant axis */
typedef enum {
    CMXD_DEVICE_X_UP = 0,    /* X-axis pointing up */
    CMXD_DEVICE_Y_UP = 1,    /* Y-axis pointing up */
    CMXD_DEVICE_Z_UP = 2,    /* Z-axis pointing up (flat) */
    CMXD_DEVICE_X_DOWN = 3,  /* X-axis pointing down */
    CMXD_DEVICE_Y_DOWN = 4,  /* Y-axis pointing down */
    CMXD_DEVICE_Z_DOWN = 5   /* Z-axis pointing down (upside down) */
} cmxd_device_orientation_t;

/* Standard platform orientation strings */
#define CMXD_ORIENTATION_LANDSCAPE "landscape"
#define CMXD_ORIENTATION_LANDSCAPE_FLIPPED "landscape-flipped"
#define CMXD_ORIENTATION_PORTRAIT "portrait"
#define CMXD_ORIENTATION_PORTRAIT_FLIPPED "portrait-flipped"

/* Module initialization */
void cmxd_orientation_init(void);

/* Core orientation detection functions */
int cmxd_get_device_orientation(double x, double y, double z);
const char* cmxd_get_platform_orientation(int orientation_code);

/* Enhanced orientation detection with tablet mode awareness */
const char* cmxd_get_orientation_with_tablet_protection(double x, double y, double z, const char* current_mode);
const char* cmxd_get_orientation_with_sensor_switching(double lid_x, double lid_y, double lid_z,
                                                      double base_x, double base_y, double base_z,
                                                      const char* current_mode);

/* Simple orientation detection */
const char* cmxd_get_orientation_simple(double x, double y, double z);

/* Module configuration */
void cmxd_orientation_set_verbose(bool verbose);
void cmxd_orientation_set_log_debug(void (*func)(const char *fmt, ...));

#endif /* CMXD_ORIENTATION_H */