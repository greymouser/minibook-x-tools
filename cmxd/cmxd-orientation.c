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

/* Tilt angle protection state */
static bool tilt_lock_active = false;
static int tilt_lock_count = 0;
static int stability_count = 0;  /* Count stable readings for unlock */
static double last_tilt_angle = 0.0;  /* Track tilt angle changes */
static const int TILT_LOCK_THRESHOLD = 3;   /* Need 3 consistent samples for tilt lock */
static const int STABILITY_UNLOCK_THRESHOLD = 10; /* Need 10 stable readings to unlock */
static const double TILT_CHANGE_THRESHOLD = 15.0; /* Lock if tilt changes > 15° rapidly */

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
    tilt_lock_active = false;
    tilt_lock_count = 0;
    verbose_logging = true;  /* Enable verbose logging for tilt protection */
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
/* Also implements general tilt protection to prevent orientation bouncing during device transitions */
const char* cmxd_get_orientation_with_tablet_protection(double x, double y, double z, const char* current_mode)
{
    /* Calculate current orientation first */
    int orientation = cmxd_get_device_orientation(x, y, z);
    const char* orientation_name = cmxd_get_platform_orientation(orientation);
    
    /* Calculate tilt angle for both tablet mode and general tilt protection */
    double tilt_angle = cmxd_calculate_tilt_angle(x, y, z);
    
    /* Stability-based tilt protection: Lock during rapid changes, unlock when stable */
    if (tilt_lock_active) {
        /* Already locked - check for stability-based unlock */
        double tilt_change = (last_tilt_angle != 0.0) ? fabs(tilt_angle - last_tilt_angle) : 0.0;
        
        if (tilt_change < 2.0) {  /* Stable reading (< 2° change) */
            stability_count++;
            if (stability_count >= STABILITY_UNLOCK_THRESHOLD) {
                tilt_lock_active = false;
                stability_count = 0;
                tilt_lock_count = 0;
                debug_log("Device stable for %d readings (tilt %.1f°), unlocking orientation changes", 
                         STABILITY_UNLOCK_THRESHOLD, tilt_angle);
            }
        } else {
            /* Not stable - reset stability counter */
            stability_count = 0;
        }
    } else {
        /* Not locked - check for rapid tilt changes that should trigger lock */
        double tilt_change = (last_tilt_angle != 0.0) ? fabs(tilt_angle - last_tilt_angle) : 0.0;
        
        if (tilt_change > TILT_CHANGE_THRESHOLD) {
            tilt_lock_count++;
            if (tilt_lock_count >= TILT_LOCK_THRESHOLD) {
                tilt_lock_active = true;
                tilt_lock_count = 0;
                stability_count = 0;
                debug_log("Rapid tilt change detected (%.1f° change), locking orientation changes", tilt_change);
            }
        } else {
            /* Stable movement - reset lock counter */
            if (tilt_lock_count > 0) {
                tilt_lock_count--;
            }
        }
    }
    
    /* Remember this tilt angle for next comparison */
    last_tilt_angle = tilt_angle;
    
    /* If general tilt lock is active, return the last known stable orientation */
    if (tilt_lock_active && last_known_orientation != NULL) {
        debug_log("General tilt lock active: maintaining %s (tilt %.1f°)", last_known_orientation, tilt_angle);
        return last_known_orientation;
    }
    
    /* Enhanced tablet reading protection: Only protect against orientation changes during 
       actual reading scenarios with stability (not during active rotation) */
    
    /* Track orientation stability */
    if (stable_orientation == NULL || strcmp(orientation_name, stable_orientation) != 0) {
        stable_orientation = orientation_name;
        stable_count = 1;
    } else {
        stable_count++;
    }
    
    /* Only apply tablet protection if:
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

/* Get orientation with dual-sensor switching (enhanced for tablet mode) */
/* Uses actual device mode to switch between lid sensor and base sensor calculations */
const char* cmxd_get_orientation_with_sensor_switching(double lid_x, double lid_y, double lid_z,
                                                      double base_x, double base_y, double base_z,
                                                      const char* current_mode)
{
    /* Calculate tilt angle using lid sensor only */
    double tilt_angle = cmxd_calculate_tilt_angle(lid_x, lid_y, lid_z);
    
    const char* orientation_name;
    
    /* Only use tablet-specific orientation method in actual tablet mode */
    if (current_mode && (strcmp(current_mode, "tablet") == 0 || strcmp(current_mode, "tent") == 0)) {
        /* Tablet/Tent mode: Use base sensor direct orientation */
        debug_log("Tablet/tent mode (%s, tilt %.1f°): using base sensor direct orientation", 
                 current_mode, tilt_angle);
        
        /* Get base sensor orientation directly */
        int base_orientation = cmxd_get_device_orientation(base_x, base_y, base_z);
        orientation_name = cmxd_get_platform_orientation(base_orientation);
        
        debug_log("Tablet base sensor orientation: %s (base X=%.1f, Y=%.1f, Z=%.1f)", 
                 orientation_name, base_x, base_y, base_z);
    } else {
        /* Non-tablet mode: Use simple lid-based orientation */
        debug_log("Non-tablet mode (%s, tilt %.1f°): using simple lid sensor orientation", 
                 current_mode ? current_mode : "unknown", tilt_angle);
        orientation_name = cmxd_get_orientation_simple(lid_x, lid_y, lid_z);
    }
    
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