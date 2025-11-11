/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Mathematical Calculation Functions
 * 
 * Implements 3D vector mathematics, angle calculations, and utility functions
 * for accelerometer data processing and hinge angle determination.
 * 
 * Copyright (c) 2025 Armando DiCianno <armando@noonshy.com>
 */

#include "cmxd-calculations.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/*
 * =============================================================================
 * BASIC 3D VECTOR OPERATIONS
 * =============================================================================
 */

/* Calculate the magnitude of a 3D vector */
double cmxd_calculate_magnitude(double x, double y, double z)
{
    return sqrt(x * x + y * y + z * z);
}

/* Normalize a 3D vector - returns 0 on success, -1 if magnitude too small */
int cmxd_normalize_vector(double x, double y, double z, 
                         double *norm_x, double *norm_y, double *norm_z)
{
    double magnitude = cmxd_calculate_magnitude(x, y, z);
    
    if (magnitude < 1e-6) {
        /* Magnitude too small - avoid division by zero */
        *norm_x = 0.0;
        *norm_y = 0.0;
        *norm_z = 0.0;
        return -1;
    }
    
    *norm_x = x / magnitude;
    *norm_y = y / magnitude;
    *norm_z = z / magnitude;
    
    return 0;
}

/* Calculate dot product of two 3D vectors */
double cmxd_calculate_dot_product(double x1, double y1, double z1,
                                 double x2, double y2, double z2)
{
    return x1 * x2 + y1 * y2 + z1 * z2;
}

/*
 * =============================================================================
 * ANGLE CALCULATIONS
 * =============================================================================
 */

/* Calculate tilt angle from horizontal plane - returns 0°=flat, 90°=vertical */
double cmxd_calculate_tilt_angle(double x, double y, double z)
{
    /* Calculate the angle between the Z-axis and the gravity vector */
    double magnitude = cmxd_calculate_magnitude(x, y, z);
    if (magnitude < 1.0) {
        return -1.0; /* Invalid reading */
    }
    
    /* Z-component normalized gives cos(tilt_angle) */
    double cos_tilt = fabs(z) / magnitude;
    
    /* Clamp to valid range to avoid numerical errors */
    cos_tilt = cmxd_clamp(cos_tilt, 0.0, 1.0);
    
    /* Convert to degrees */
    double tilt_angle = acos(cos_tilt) * 180.0 / M_PI;
    
    return tilt_angle;
}

/*
 * =============================================================================
 * UTILITY FUNCTIONS
 * =============================================================================
 */

/* Clamp a value to a specified range */
double cmxd_clamp(double value, double min_val, double max_val)
{
    if (value < min_val) return min_val;
    if (value > max_val) return max_val;
    return value;
}

/* Convert radians to degrees */
double cmxd_rad_to_deg(double radians)
{
    return radians * 180.0 / M_PI;
}

/* Convert degrees to radians */
double cmxd_deg_to_rad(double degrees)
{
    return degrees * M_PI / 180.0;
}

/*
 * =============================================================================
 * LOGGING CONFIGURATION
 * =============================================================================
 */

/* Logging function (will be set by main) */
static void (*log_debug_func)(const char *fmt, ...) = NULL;

/* Set the debug logging function */
void cmxd_calculations_set_log_debug(void (*func)(const char *fmt, ...))
{
    log_debug_func = func;
}

/* Internal debug logging */
static void debug_log(const char *fmt, ...)
{
    if (log_debug_func) {
        va_list args;
        va_start(args, fmt);
        char buffer[512];
        vsnprintf(buffer, sizeof(buffer), fmt, args);
        va_end(args);
        log_debug_func("%s", buffer);
    }
}

/*
 * =============================================================================
 * DEVICE TILT COMPENSATION
 * =============================================================================
 */

/* Detect if device is being tilted (rotated as a whole unit) vs hinge movement */
bool cmxd_detect_device_rotation(const struct cmxd_accel_sample *base, const struct cmxd_accel_sample *lid,
                                double base_scale, double lid_scale)
{
    /* Convert to m/s² */
    double base_x = base->x * base_scale;
    double base_y = base->y * base_scale;
    double base_z = base->z * base_scale;
    
    double lid_x = lid->x * lid_scale;
    double lid_y = lid->y * lid_scale;
    double lid_z = lid->z * lid_scale;
    
    /* Calculate magnitudes to check for unusual gravity readings */
    double base_mag = cmxd_calculate_magnitude(base_x, base_y, base_z);
    double lid_mag = cmxd_calculate_magnitude(lid_x, lid_y, lid_z);
    
    /* Calculate the relative angle between the sensors */
    double dot_product = cmxd_calculate_dot_product(base_x, base_y, base_z, lid_x, lid_y, lid_z);
    double cos_angle = dot_product / (base_mag * lid_mag);
    cos_angle = cmxd_clamp(cos_angle, -1.0, 1.0);
    double sensor_angle = acos(cos_angle) * 180.0 / M_PI;
    
    /* Enhanced detection logic:
     * 1. Check if we're in the problematic laptop angle range (90-110°) where mode changes should occur
     * 2. Look for signs that normal hinge physics are being disrupted by device tilt
     * 3. Be more permissive during likely transition scenarios
     */
    
    /* If the sensors show they should be in laptop/flat transition zone but motion is restricted */
    bool in_transition_zone = (sensor_angle >= 90.0 && sensor_angle <= 110.0);
    
    /* Check for signs of device tilt affecting readings:
     * - Base sensor showing unexpected horizontal components 
     * - Lid sensor showing non-standard orientation
     * 
     * Make thresholds much higher to avoid false compensation on normal variations
     */
    double base_horizontal = sqrt(base_x * base_x + base_y * base_y);
    double lid_horizontal = sqrt(lid_x * lid_x + lid_y * lid_y);
    
    bool base_unusual = (base_horizontal > 6.0);  /* Base showing significant horizontal acceleration */
    bool lid_unusual = (lid_horizontal > 8.0);    /* Lid showing major horizontal components */
    
    /* Apply gravity compensation if we're in transition zone and see signs of device tilt */
    bool should_compensate = in_transition_zone && (base_unusual || lid_unusual);
    
    if (should_compensate) {
        debug_log("Device rotation detected - angle=%.1f°, base_h=%.1f, lid_h=%.1f", 
                 sensor_angle, base_horizontal, lid_horizontal);
    }
    
    return should_compensate;
}

/* Gravity-compensated hinge angle calculation */
/* Attempts to get truer hinge angle by compensating for device tilt */
double cmxd_calculate_gravity_compensated_hinge_angle(const struct cmxd_accel_sample *base, const struct cmxd_accel_sample *lid,
                                                     double base_scale, double lid_scale)
{
    /* First check if device appears to be rotated as a unit */
    if (cmxd_detect_device_rotation(base, lid, base_scale, lid_scale)) {
        /* Device is being rotated - try to compensate by estimating true hinge angle */
        debug_log("Device rotation detected - applying gravity compensation");
        
        /* Convert to m/s² */
        double base_x = base->x * base_scale;
        double base_y = base->y * base_scale;
        
        double lid_x = lid->x * lid_scale;
        double lid_y = lid->y * lid_scale;
        
        /* Calculate normal angle for reference */
        double normal_angle = cmxd_calculate_hinge_angle(base, lid, base_scale, lid_scale);
        
        /* Enhanced compensation approach: 
         * When device is tilted, the apparent angle is often smaller than real hinge angle
         * Apply empirical correction based on horizontal acceleration magnitude
         */
        
        double base_horizontal = sqrt(base_x * base_x + base_y * base_y);
        double lid_horizontal = sqrt(lid_x * lid_x + lid_y * lid_y);
        double total_horizontal = base_horizontal + lid_horizontal;
        
        /* Calculate compensation factor based on device tilt severity - be much more conservative */
        double tilt_factor = 1.0;
        
        if (total_horizontal > 10.0) {
            /* Very significant device tilt detected - apply minimal progressive compensation */
            /* Only compensate when horizontal acceleration is very high */
            tilt_factor = 1.0 + (total_horizontal - 10.0) * 0.05; /* 5% per m/s² of excess horizontal */
            tilt_factor = cmxd_clamp(tilt_factor, 1.0, 1.3); /* Cap at 30% increase */
        }
        
        /* Reduce transition zone boost significantly */
        double zone_boost = 0.0;
        if (normal_angle >= 95.0 && normal_angle <= 110.0) {
            /* In the sticky transition zone - add minimal boost */
            zone_boost = (normal_angle - 95.0) * 0.5; /* Up to 7.5° boost at 110° instead of 30° */
        }
        
        double compensated_angle = normal_angle * tilt_factor + zone_boost;
        compensated_angle = cmxd_clamp(compensated_angle, normal_angle, normal_angle + 50.0);
        
        if (compensated_angle > normal_angle + 2.0) {
            debug_log("Applied tilt compensation: raw=%.1f° -> compensated=%.1f° (factor=%.2f, boost=%.1f°)", 
                     normal_angle, compensated_angle, tilt_factor, zone_boost);
            return compensated_angle;
        } else {
            debug_log("Compensation calculated but minimal: raw=%.1f° (factor=%.2f, boost=%.1f°)", 
                     normal_angle, tilt_factor, zone_boost);
        }
    }
    
    /* No compensation needed or beneficial - use normal calculation */
    return cmxd_calculate_hinge_angle(base, lid, base_scale, lid_scale);
}

/*
 * =============================================================================
 * SIMPLIFIED HINGE ANGLE CALCULATIONS
 * =============================================================================
 */

/* Calculate hinge angle from base and lid accelerometer readings */
/* Uses simple dot product method - now that mount matrices are correct! */
double cmxd_calculate_hinge_angle(const struct cmxd_accel_sample *base, const struct cmxd_accel_sample *lid,
                                 double base_scale, double lid_scale)
{
    /* Convert raw accelerometer values to m/s² using provided scale factors */
    double base_x = base->x * base_scale;
    double base_y = base->y * base_scale;
    double base_z = base->z * base_scale;
    
    double lid_x = lid->x * lid_scale;
    double lid_y = lid->y * lid_scale;
    double lid_z = lid->z * lid_scale;
    
    /* Calculate magnitudes */
    double base_magnitude = cmxd_calculate_magnitude(base_x, base_y, base_z);
    double lid_magnitude = cmxd_calculate_magnitude(lid_x, lid_y, lid_z);
    
    if (base_magnitude < 1.0 || lid_magnitude < 1.0) {
        debug_log("Invalid accelerometer readings: base_mag=%.3f, lid_mag=%.3f", 
                 base_magnitude, lid_magnitude);
        return -1.0;
    }
    
    /* Calculate dot product of gravity vectors */
    double dot_product = cmxd_calculate_dot_product(base_x, base_y, base_z, lid_x, lid_y, lid_z);
    
    /* Normalize by magnitudes to get cosine of angle */
    double cos_angle = dot_product / (base_magnitude * lid_magnitude);
    
    /* Clamp to valid range to avoid numerical errors */
    cos_angle = cmxd_clamp(cos_angle, -1.0, 1.0);
    
    /* Convert to degrees (0-180°) */
    double angle = acos(cos_angle) * 180.0 / M_PI;
    
    /* Verbose debug output removed to clean up format - shown in main loop */
    
    return angle;
}

/* Calculate full 0-360° hinge angle by considering laptop orientation */
double cmxd_calculate_hinge_angle_360(const struct cmxd_accel_sample *base, const struct cmxd_accel_sample *lid,
                                     double base_scale, double lid_scale)
{
    /* Use gravity-compensated calculation for better accuracy during transitions */
    double base_angle = cmxd_calculate_gravity_compensated_hinge_angle(base, lid, base_scale, lid_scale);
    
    if (base_angle < 0) {
        return base_angle; /* Error case */
    }
    
    /* Convert raw values to m/s² for analysis using provided scale factors */
    double base_x = base->x * base_scale;
    double base_z = base->z * base_scale;
    
    double lid_x = lid->x * lid_scale;
    double lid_z = lid->z * lid_scale;
    
    /* Calculate the cross product to determine hinge direction */
    /* Cross product base × lid gives us the hinge axis direction */
    double cross_y = base_z * lid_x - base_x * lid_z;
    
    /* Use the Y component of cross product to determine fold direction */
    /* When laptop opens normally: cross_y should be positive */
    /* When folding back (tent/tablet): cross_y becomes negative */
    
    double angle_360;
    
    /* Check if we're in the "folded back" region (tent/tablet modes) */
    /* 
     * Use cross product Y-component to detect fold direction.
     * When opening normally (0-180°): cross_y > 0
     * When folding back (180-360°): cross_y < 0
     * 
     * Add hysteresis to prevent rapid oscillation near 180°
     */
    
    bool is_folded_back;
    static bool was_folded_back = false;
    
    if (was_folded_back) {
        /* Currently in fold-back mode - need cross_y clearly positive to exit */
        is_folded_back = (cross_y < 5.0);
    } else {
        /* Currently in normal mode - need cross_y clearly negative to enter fold-back */
        is_folded_back = (cross_y < -5.0);
    }
    
    was_folded_back = is_folded_back;
    
    if (is_folded_back) {
        /* We're in the "folded back" region - convert to 180-360° range */
        angle_360 = 360.0 - base_angle;
        debug_log("*** FOLD-BACK: cross_y=%.1f base=%.1f° -> %.1f°", cross_y, base_angle, angle_360);
    } else {
        /* Normal opening range 0-180° */
        angle_360 = base_angle;
        debug_log("*** NORMAL: cross_y=%.1f -> %.1f°", cross_y, angle_360);
    }
    
    return angle_360;
}