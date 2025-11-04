/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Orientation detection for CMXD (Chuwi Minibook X Daemon)
 * 
 * Device orientation detection using lid accelerometer
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

/* Module state */
static int last_detected_orientation = CMXD_ORIENTATION_UNKNOWN;
static const char* last_known_orientation = CMXD_ORIENTATION_LANDSCAPE;
static bool verbose_logging = false;

/* Tablet mode protection state */
static const char* stable_orientation = NULL;
static int stable_count = 0;
static const int STABILITY_THRESHOLD = 10;  /* Need 10 consistent samples */

/* Logging function (will be set by main) */
static void (*log_debug_func)(const char *fmt, ...) = NULL;

/* Set the debug logging function */
void cmxd_orientation_set_log_debug(void (*func)(const char *fmt, ...))
{
    log_debug_func = func;
}

/* Internal debug logging */
static void debug_log(const char *fmt, ...)
{
    if (verbose_logging && log_debug_func) {
        va_list args;
        va_start(args, fmt);
        char buffer[512];
        vsnprintf(buffer, sizeof(buffer), fmt, args);
        va_end(args);
        log_debug_func("%s", buffer);
    }
}

/* Initialize orientation detection module */
void cmxd_orientation_init(void)
{
    last_detected_orientation = CMXD_ORIENTATION_UNKNOWN;
    last_known_orientation = CMXD_ORIENTATION_LANDSCAPE;
    stable_orientation = NULL;
    stable_count = 0;
}

/* Reset orientation detection state */
void cmxd_orientation_reset(void)
{
    cmxd_orientation_init();
}

/* Determine raw device orientation based on accelerometer readings */
/* Returns: 0=X-up, 1=Y-up, 2=Z-up, 3=X-down, 4=Y-down, 5=Z-down */
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

/* Map device orientation to standard platform terms */
/* Uses lid sensor orientation to determine screen orientation */
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
    
    debug_log("Simple orientation: %s (device_code=%d)", orientation_name, orientation);
    
    return orientation_name;
}

/* Get orientation with tablet mode reading protection */
/* Prevents orientation changes FROM portrait in tablet mode when tilted > 45° for reading stability */
const char* cmxd_get_orientation_with_tablet_protection(double x, double y, double z, const char* current_mode)
{
    /* Calculate current orientation first */
    int orientation = cmxd_get_device_orientation(x, y, z);
    const char* orientation_name = cmxd_get_platform_orientation(orientation);
    
    /* Calculate tilt angle for tablet mode protection */
    double tilt_angle = cmxd_calculate_tilt_angle(x, y, z);
    
    /* Enhanced tablet reading protection: Only protect against orientation changes during 
       actual reading scenarios with stability (not during active rotation) */
    
    /* Track orientation stability */
    if (stable_orientation == NULL || strcmp(orientation_name, stable_orientation) != 0) {
        stable_orientation = orientation_name;
        stable_count = 1;
    } else {
        stable_count++;
    }
    
    /* Only apply protection if:
     * 1. In tablet mode
     * 2. High tilt angle (reading position)  
     * 3. Currently stable in portrait (not actively rotating)
     * 4. Trying to switch to landscape
     * 5. Have been stable for a while (not just started rotation) */
    if (current_mode && strcmp(current_mode, "tablet") == 0 && 
        tilt_angle > 70.0 &&  /* Higher threshold for actual reading positions */
        stable_count >= STABILITY_THRESHOLD &&  /* Must be stable, not actively rotating */
        last_known_orientation != NULL && 
        (strcmp(last_known_orientation, CMXD_ORIENTATION_PORTRAIT) == 0 || 
         strcmp(last_known_orientation, CMXD_ORIENTATION_PORTRAIT_FLIPPED) == 0) &&  /* Currently in portrait */
        (strcmp(orientation_name, CMXD_ORIENTATION_LANDSCAPE) == 0 || 
         strcmp(orientation_name, CMXD_ORIENTATION_LANDSCAPE_FLIPPED) == 0)) {         /* Trying to switch to landscape */
        
        debug_log("Tablet reading protection: maintaining %s (tilt %.1f° > 70°, stable %d samples), blocking switch to %s", 
                 last_known_orientation, tilt_angle, stable_count, orientation_name);
        return last_known_orientation;
    }
    
    /* Normal orientation detection - update last known orientation */
    last_known_orientation = orientation_name;
    
    debug_log("Normal orientation: %s (tilt %.1f°, mode %s, stable %d)", 
             orientation_name, tilt_angle, current_mode ? current_mode : "unknown", stable_count);
    return orientation_name;
}

/* Check if orientation has changed from last reading */
bool cmxd_orientation_has_changed(int current_orientation)
{
    if (last_detected_orientation == CMXD_ORIENTATION_UNKNOWN) {
        last_detected_orientation = current_orientation;
        return true;  /* First reading counts as a change */
    }
    
    bool changed = (current_orientation != last_detected_orientation);
    if (changed) {
        debug_log("Orientation change detected: %d -> %d", last_detected_orientation, current_orientation);
        last_detected_orientation = current_orientation;
    }
    
    return changed;
}

/* Get the last detected orientation */
int cmxd_get_last_orientation(void)
{
    return last_detected_orientation;
}

/* Set verbose logging for orientation detection */
void cmxd_orientation_set_verbose(bool verbose)
{
    verbose_logging = verbose;
}