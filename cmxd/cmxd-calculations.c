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
 * HINGE ANGLE CALCULATIONS
 * =============================================================================
 */

/* Calculate hinge angle from base and lid accelerometer readings */
/* Uses simple dot product for reliable 0-180° range with state tracking */
double cmxd_calculate_hinge_angle(const struct cmxd_accel_sample *base, const struct cmxd_accel_sample *lid)
{
    /* Convert raw accelerometer values to normalized vectors */
    double base_magnitude = cmxd_calculate_magnitude((double)base->x, (double)base->y, (double)base->z);
    double lid_magnitude = cmxd_calculate_magnitude((double)lid->x, (double)lid->y, (double)lid->z);
    
    if (base_magnitude < 1.0 || lid_magnitude < 1.0) {
        debug_log("Invalid accelerometer readings: base_mag=%.3f, lid_mag=%.3f", base_magnitude, lid_magnitude);
        return -1.0; /* Invalid reading */
    }
    
    /* Normalize the vectors */
    double base_norm[3] = {
        base->x / base_magnitude,
        base->y / base_magnitude, 
        base->z / base_magnitude
    };
    
    double lid_norm[3] = {
        lid->x / lid_magnitude,
        lid->y / lid_magnitude,
        lid->z / lid_magnitude
    };
    
    /* Calculate the dot product between normalized vectors */
    double dot_product = cmxd_calculate_dot_product(base_norm[0], base_norm[1], base_norm[2],
                                                   lid_norm[0], lid_norm[1], lid_norm[2]);
    
    /* Clamp to valid range to avoid numerical errors in acos() */
    dot_product = cmxd_clamp(dot_product, -1.0, 1.0);
    
    /* Convert to degrees - reliable 0-180° range */
    double angle = acos(dot_product) * 180.0 / M_PI;
    
    debug_log("Hinge calculation (simple): base[%d,%d,%d] lid[%d,%d,%d] -> dot=%.3f, angle=%.1f°", 
             base->x, base->y, base->z, lid->x, lid->y, lid->z, dot_product, angle);
    
    return angle;
}

/*
 * =============================================================================
 * GRAVITY-AWARE COORDINATE TRANSFORMATION
 * =============================================================================
 */

/* Transform accelerometer reading to standard coordinate frame based on gravity direction */
static void transform_to_standard_frame(double x, double y, double z, int gravity_orientation, 
                                       double *std_x, double *std_y, double *std_z)
{
    /* Standard frame: X=forward/back, Y=left/right, Z=up/down (gravity down = +Z) */
    switch (gravity_orientation) {
        case 0: /* X_DOWN - normal orientation, X points to gravity */
            *std_x = x;
            *std_y = y; 
            *std_z = z;
            break;
        case 1: /* X_UP - upside down, X points away from gravity */
            *std_x = -x;
            *std_y = -y;
            *std_z = -z;
            break;
        case 2: /* Y_DOWN - left side down, Y points to gravity */
            *std_x = -y;  /* old Y becomes -X */
            *std_y = x;   /* old X becomes Y */
            *std_z = z;   /* Z unchanged */
            break;
        case 3: /* Y_UP - right side down, Y points away from gravity */
            *std_x = y;   /* old Y becomes X */
            *std_y = -x;  /* old X becomes -Y */
            *std_z = z;   /* Z unchanged */
            break;
        case 4: /* Z_DOWN - lying flat face down */
            *std_x = x;
            *std_y = y;
            *std_z = -z;  /* Flip Z */
            break;
        case 5: /* Z_UP - lying flat face up */
        default:
            *std_x = x;
            *std_y = y;
            *std_z = z;   /* Keep as-is */
            break;
    }
}

/* Detect gravity orientation for a sensor reading */
static int detect_gravity_orientation(double x, double y, double z)
{
    double abs_x = fabs(x);
    double abs_y = fabs(y);
    double abs_z = fabs(z);
    
    /* Find the axis with the largest magnitude (closest to gravity) */
    if (abs_z > abs_x && abs_z > abs_y) {
        return (z > 0) ? 5 : 4;  /* Z_UP : Z_DOWN */
    } else if (abs_y > abs_x) {
        return (y > 0) ? 3 : 2;  /* Y_UP : Y_DOWN */
    } else {
        return (x > 0) ? 1 : 0;  /* X_UP : X_DOWN */
    }
}

/* Public interface for gravity orientation detection */
int cmxd_detect_gravity_orientation(double x, double y, double z)
{
    return detect_gravity_orientation(x, y, z);
}

/* Calculate full 0-360° hinge angle from base and lid accelerometer readings */
/* Uses orientation-independent calculation to find angle between sensor mounting planes */
double cmxd_calculate_hinge_angle_360(const struct cmxd_accel_sample *base, const struct cmxd_accel_sample *lid)
{
    /* Convert raw accelerometer values to normalized vectors */
    double base_magnitude = cmxd_calculate_magnitude((double)base->x, (double)base->y, (double)base->z);
    double lid_magnitude = cmxd_calculate_magnitude((double)lid->x, (double)lid->y, (double)lid->z);
    
    if (base_magnitude < 1.0 || lid_magnitude < 1.0) {
        debug_log("Invalid accelerometer readings: base_mag=%.3f, lid_mag=%.3f", base_magnitude, lid_magnitude);
        return -1.0; /* Invalid reading */
    }

    /* Normalize the raw vectors */
    double base_norm[3] = {
        base->x / base_magnitude,
        base->y / base_magnitude, 
        base->z / base_magnitude
    };
    
    double lid_norm[3] = {
        lid->x / lid_magnitude,
        lid->y / lid_magnitude,
        lid->z / lid_magnitude
    };
    
    /* Calculate estimated gravity direction (average of both sensors) */
    double gravity[3] = {
        (base_norm[0] + lid_norm[0]) / 2.0,
        (base_norm[1] + lid_norm[1]) / 2.0,
        (base_norm[2] + lid_norm[2]) / 2.0
    };
    double gravity_mag = cmxd_calculate_magnitude(gravity[0], gravity[1], gravity[2]);
    if (gravity_mag > 0.1) {
        gravity[0] /= gravity_mag;
        gravity[1] /= gravity_mag;
        gravity[2] /= gravity_mag;
    }
    
    /* Project both sensor vectors onto the plane perpendicular to gravity */
    /* Projection formula: v_projected = v - (v · gravity) * gravity */
    double base_dot_gravity = cmxd_calculate_dot_product(base_norm[0], base_norm[1], base_norm[2],
                                                        gravity[0], gravity[1], gravity[2]);
    double lid_dot_gravity = cmxd_calculate_dot_product(lid_norm[0], lid_norm[1], lid_norm[2],
                                                       gravity[0], gravity[1], gravity[2]);
    
    double base_projected[3] = {
        base_norm[0] - base_dot_gravity * gravity[0],
        base_norm[1] - base_dot_gravity * gravity[1],
        base_norm[2] - base_dot_gravity * gravity[2]
    };
    
    double lid_projected[3] = {
        lid_norm[0] - lid_dot_gravity * gravity[0],
        lid_norm[1] - lid_dot_gravity * gravity[1],
        lid_norm[2] - lid_dot_gravity * gravity[2]
    };
    
    /* Normalize the projected vectors */
    double base_proj_mag = cmxd_calculate_magnitude(base_projected[0], base_projected[1], base_projected[2]);
    double lid_proj_mag = cmxd_calculate_magnitude(lid_projected[0], lid_projected[1], lid_projected[2]);
    
    if (base_proj_mag < 0.1 || lid_proj_mag < 0.1) {
        /* Projected vectors too small - both sensors mostly aligned with gravity */
        /* Fall back to a simplified approach using the difference between sensor vectors */
        double diff[3] = {
            base_norm[0] - lid_norm[0],
            base_norm[1] - lid_norm[1], 
            base_norm[2] - lid_norm[2]
        };
        double diff_mag = cmxd_calculate_magnitude(diff[0], diff[1], diff[2]);
        
        /* Convert difference magnitude to an approximate angle */
        /* When sensors are identical (diff=0), angle=0° */
        /* When sensors are perpendicular (diff=sqrt(2)≈1.414), angle=90° */
        /* When sensors are opposite (diff=2), angle=180° */
        double angle = 2.0 * asin(cmxd_clamp(diff_mag / 2.0, 0.0, 1.0)) * 180.0 / M_PI;
        
        debug_log("Fallback hinge calculation: base[%d,%d,%d] lid[%d,%d,%d] diff_mag=%.3f -> angle=%.1f°",
                 base->x, base->y, base->z, lid->x, lid->y, lid->z, diff_mag, angle);
        
        return angle;
    }
    
    base_projected[0] /= base_proj_mag;
    base_projected[1] /= base_proj_mag;
    base_projected[2] /= base_proj_mag;
    
    lid_projected[0] /= lid_proj_mag;
    lid_projected[1] /= lid_proj_mag;
    lid_projected[2] /= lid_proj_mag;
    
    /* Calculate the angle between the projected vectors */
    double dot_product = cmxd_calculate_dot_product(base_projected[0], base_projected[1], base_projected[2],
                                                   lid_projected[0], lid_projected[1], lid_projected[2]);
    
    /* Clamp to valid range to avoid numerical errors in acos() */
    dot_product = cmxd_clamp(dot_product, -1.0, 1.0);
    
    double projected_angle = acos(dot_product) * 180.0 / M_PI;
    
    /* For hinge angle calculation, when projected vectors are opposite (180°), 
     * this represents a 90° hinge opening. When they're identical (0°), 
     * this represents either 0° (closed) or 180° (flat) hinge opening.
     * So the actual hinge angle is: 180° - projected_angle */
    double angle = 180.0 - projected_angle;
    
    debug_log("Gravity-independent hinge calculation: base[%d,%d,%d] lid[%d,%d,%d] gravity=[%.3f,%.3f,%.3f] base_proj=[%.3f,%.3f,%.3f] lid_proj=[%.3f,%.3f,%.3f] dot=%.3f projected_angle=%.1f° -> hinge_angle=%.1f°",
             base->x, base->y, base->z, lid->x, lid->y, lid->z,
             gravity[0], gravity[1], gravity[2],
             base_projected[0], base_projected[1], base_projected[2],
             lid_projected[0], lid_projected[1], lid_projected[2],
             dot_product, projected_angle, angle);
    
    return angle;
}