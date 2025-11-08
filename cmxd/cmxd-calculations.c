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
    
    debug_log("Hinge angle: base[%.2f,%.2f,%.2f] lid[%.2f,%.2f,%.2f] cos=%.3f -> %.1f°", 
             base_x, base_y, base_z, lid_x, lid_y, lid_z, cos_angle, angle);
    
    return angle;
}

/* Calculate full 0-360° hinge angle by considering laptop orientation */
double cmxd_calculate_hinge_angle_360(const struct cmxd_accel_sample *base, const struct cmxd_accel_sample *lid,
                                     double base_scale, double lid_scale)
{
    /* First get the basic 0-180° angle */
    double base_angle = cmxd_calculate_hinge_angle(base, lid, base_scale, lid_scale);
    
    if (base_angle < 0) {
        return base_angle; /* Error case */
    }
    
    /* Convert raw values to m/s² for analysis using provided scale factors */
    double base_x = base->x * base_scale;
    double base_y = base->y * base_scale;
    double base_z = base->z * base_scale;
    
    double lid_x = lid->x * lid_scale;
    double lid_y = lid->y * lid_scale;
    double lid_z = lid->z * lid_scale;
    
    /* Calculate the cross product to determine hinge direction */
    /* Cross product base × lid gives us the hinge axis direction */
    double cross_x = base_y * lid_z - base_z * lid_y;
    double cross_y = base_z * lid_x - base_x * lid_z;
    double cross_z = base_x * lid_y - base_y * lid_x;
    
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
        debug_log("Folded back mode: base_z=%.2f lid_z=%.2f cross_y=%.3f base_angle=%.1f° -> 360°-angle=%.1f°", 
                 base_z, lid_z, cross_y, base_angle, angle_360);
    } else {
        /* Normal opening range 0-180° */
        angle_360 = base_angle;
        debug_log("Normal opening: base_z=%.2f lid_z=%.2f cross_y=%.3f -> %.1f°", 
                 base_z, lid_z, cross_y, angle_360);
    }
    
    /* Alternative method using cross product magnitude for validation */
    double cross_magnitude = sqrt(cross_x*cross_x + cross_y*cross_y + cross_z*cross_z);
    
    debug_log("Hinge 360°: base[%.2f,%.2f,%.2f] lid[%.2f,%.2f,%.2f] cross_y=%.3f cross_mag=%.3f base_angle=%.1f° -> %.1f°", 
             base_x, base_y, base_z, lid_x, lid_y, lid_z, cross_y, cross_magnitude, base_angle, angle_360);
    
    return angle_360;
}