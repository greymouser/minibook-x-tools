/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Orientation detection for CMXD (Chuwi Minibook X Daemon)
 * 
 * Device orientation detection using lid accelerometer
 * 
 * Copyright (c) 2025 Armando DiCianno <armando@noonshy.com>
 */

#ifndef CMXD_ORIENTATION_H
#define CMXD_ORIENTATION_H

#include <stdint.h>
#include <stdbool.h>

/* Orientation constants */
#define CMXD_ORIENTATION_UNKNOWN -1

/* Device orientation codes (internal) */
typedef enum {
    CMXD_DEVICE_X_UP = 0,
    CMXD_DEVICE_Y_UP = 1,
    CMXD_DEVICE_Z_UP = 2,
    CMXD_DEVICE_X_DOWN = 3,
    CMXD_DEVICE_Y_DOWN = 4,
    CMXD_DEVICE_Z_DOWN = 5
} cmxd_device_orientation_t;

/* Platform orientation names */
#define CMXD_ORIENTATION_LANDSCAPE "landscape"
#define CMXD_ORIENTATION_LANDSCAPE_FLIPPED "landscape-flipped"
#define CMXD_ORIENTATION_PORTRAIT "portrait"
#define CMXD_ORIENTATION_PORTRAIT_FLIPPED "portrait-flipped"

/* Initialize orientation detection module */
void cmxd_orientation_init(void);

/* Reset orientation detection state */
void cmxd_orientation_reset(void);

/* Determine raw device orientation based on accelerometer readings */
/* Returns: 0=X-up, 1=Y-up, 2=Z-up, 3=X-down, 4=Y-down, 5=Z-down */
int cmxd_get_device_orientation(double x, double y, double z);

/* Map device orientation to standard platform terms */
/* Uses lid sensor orientation to determine screen orientation */
const char* cmxd_get_platform_orientation(int orientation_code);

/* Get orientation with tablet mode reading protection */
/* Prevents orientation changes FROM portrait in tablet mode when tilted > 45° for reading stability */
const char* cmxd_get_orientation_with_tablet_protection(double x, double y, double z, const char* current_mode);

/* Simple orientation detection without tablet protection */
const char* cmxd_get_orientation_simple(double x, double y, double z);

/* Check if orientation has changed from last reading */
bool cmxd_orientation_has_changed(int current_orientation);

/* Get the last detected orientation */
int cmxd_get_last_orientation(void);

/* Set verbose logging for orientation detection */
void cmxd_orientation_set_verbose(bool verbose);

/* Set the debug logging function for orientation module */
void cmxd_orientation_set_log_debug(void (*func)(const char *fmt, ...));

#endif /* CMXD_ORIENTATION_H */