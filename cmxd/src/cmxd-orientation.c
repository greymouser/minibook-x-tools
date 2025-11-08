/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Device Orientation Detection Implementation
 * 
 * Implements accelerometer-based orientation detection with tablet mode
 * awareness and dual-sensor support. Provides platform-independent 
 * orientation mapping and stability protection.
 * 
 * Copyright (c) 2025 Armando DiCianno <armando@noonshy.com>
 */

#include "cmxd-orientation.h"
#include "cmxd-calculations.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>

/*
 * =============================================================================
 * MODULE STATE AND CONFIGURATION
 * =============================================================================
 */

/* Module state */
static const char* last_known_orientation = CMXD_ORIENTATION_LANDSCAPE;
static bool verbose_logging = false;

/* Logging function (set by main application) */
static void (*log_debug_func)(const char *fmt, ...) = NULL;

/*
 * =============================================================================
 * LOGGING AND INITIALIZATION
 * =============================================================================
 */

/* Set the debug logging function */
void cmxd_orientation_set_log_debug(void (*func)(const char *fmt, ...))
{
    log_debug_func = func;
}

/* Initialize orientation detection module */
void cmxd_orientation_init(void)
{
    last_known_orientation = CMXD_ORIENTATION_LANDSCAPE;
    verbose_logging = true;  /* Enable verbose logging for tablet protection */
}

/*
 * =============================================================================
 * CORE ORIENTATION DETECTION
 * =============================================================================
 */

/* Determine raw device orientation based on accelerometer readings */
int cmxd_get_device_orientation(double x, double y, double z)
{
    double abs_x = fabs(x);
    double abs_y = fabs(y);
    double abs_z = fabs(z);
    
    /* Find the axis with the largest magnitude (closest to gravity) */
    if (abs_z > abs_x && abs_z > abs_y) {
        return (z > 0) ? CMXD_DEVICE_Z_UP : CMXD_DEVICE_Z_DOWN;
    } else if (abs_y > abs_x) {
        return (y > 0) ? CMXD_DEVICE_Y_UP : CMXD_DEVICE_Y_DOWN;
    } else {
        return (x > 0) ? CMXD_DEVICE_X_UP : CMXD_DEVICE_X_DOWN;
    }
}

/*
 * =============================================================================
 * PLATFORM ORIENTATION MAPPING
 * =============================================================================
 */

/* Map device orientation to standard platform terms */
const char* cmxd_get_platform_orientation(int orientation_code)
{
    switch (orientation_code) {
        case CMXD_DEVICE_X_DOWN:  /* X-down - normal laptop landscape position */
            return CMXD_ORIENTATION_LANDSCAPE;
        case CMXD_DEVICE_X_UP:    /* X-up - laptop upside down landscape */
            return CMXD_ORIENTATION_LANDSCAPE_FLIPPED;
        case CMXD_DEVICE_Y_UP:    /* Y-up - laptop standing vertically (portrait) */
            return CMXD_ORIENTATION_PORTRAIT;
        case CMXD_DEVICE_Y_DOWN:  /* Y-down - laptop standing vertically (portrait-flipped) */
            return CMXD_ORIENTATION_PORTRAIT_FLIPPED;
        case CMXD_DEVICE_Z_UP:    /* Z-up - unusual orientation, default to landscape */
        case CMXD_DEVICE_Z_DOWN:  /* Z-down - unusual orientation, default to landscape */
        default:
            return CMXD_ORIENTATION_LANDSCAPE;  /* Default to landscape for edge cases */
    }
}

/* Simple orientation detection without tablet protection */
const char* cmxd_get_orientation_simple(double x, double y, double z)
{
    int orientation = cmxd_get_device_orientation(x, y, z);
    const char* orientation_name = cmxd_get_platform_orientation(orientation);
    
    /* Orientation debug output reduced for cleaner format */
    
    return orientation_name;
}

/* Get orientation with tablet mode reading protection */
/* Prevents orientation changes FROM portrait in tablet mode when tilted > 45° for reading stability */
/* Also implements general tilt protection to prevent orientation bouncing during device transitions */
const char* cmxd_get_orientation_with_tablet_protection(double x, double y, double z, const char* current_mode)
{
    /* Calculate current orientation first */
    int orientation = cmxd_get_device_orientation(x, y, z);
    const char* orientation_name = cmxd_get_platform_orientation(orientation);
    
    /* Calculate tilt angle for tablet mode protection */
    double tilt_angle = cmxd_calculate_tilt_angle(x, y, z);
    
    /* Option 3: Tilt-based orientation lock for tablet mode
     * When in tablet mode and starting in portrait, if tilt goes below 45° (lying flat),
     * lock orientation until it comes back above 45° to prevent unwanted landscape switches */
    if (current_mode && strcmp(current_mode, "tablet") == 0 && 
        last_known_orientation != NULL && 
        (strcmp(last_known_orientation, CMXD_ORIENTATION_PORTRAIT) == 0 || 
         strcmp(last_known_orientation, CMXD_ORIENTATION_PORTRAIT_FLIPPED) == 0) &&  /* Currently in portrait */
        tilt_angle < 45.0 &&  /* Tilted flat (lying on table) */
        (strcmp(orientation_name, CMXD_ORIENTATION_LANDSCAPE) == 0 || 
         strcmp(orientation_name, CMXD_ORIENTATION_LANDSCAPE_FLIPPED) == 0)) {         /* Trying to switch to landscape */
        
        /* Tablet tilt lock debug output reduced */
        return last_known_orientation;
    }
    
    /* Normal orientation detection - update last known orientation */
    last_known_orientation = orientation_name;
    
    /* Normal orientation debug output reduced */
    return orientation_name;
}

/* Get orientation with dual-sensor switching (enhanced for tablet mode) */
/* Uses actual device mode to switch between lid sensor and base sensor calculations */
const char* cmxd_get_orientation_with_sensor_switching(double lid_x, double lid_y, double lid_z,
                                                      double base_x, double base_y, double base_z,
                                                      const char* current_mode)
{    
    const char* orientation_name;
    
    /* Only use tablet-specific orientation method in actual tablet mode */
    if (current_mode && (strcmp(current_mode, "tablet") == 0 || strcmp(current_mode, "tent") == 0)) {
        /* Tablet/Tent mode: Use base sensor direct orientation with tablet protection */
        
        /* Get base sensor orientation with tablet protection */
        orientation_name = cmxd_get_orientation_with_tablet_protection(base_x, base_y, base_z, current_mode);
        
    } else {
        /* Non-tablet mode: Use simple lid-based orientation */
        orientation_name = cmxd_get_orientation_simple(lid_x, lid_y, lid_z);
    }
    
    return orientation_name;
}

/* Set verbose logging for orientation detection */
void cmxd_orientation_set_verbose(bool verbose)
{
    verbose_logging = verbose;
}