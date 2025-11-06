/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Mathematical calculations for CMXD (Chuwi Minibook X Daemon)
 * 
 * Shared mathematical functions for accelerometer data processing
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

/* Calculate the tilt angle of the device from horizontal plane */
/* Returns angle in degrees: 0° = flat, 90° = vertical */
/* Returns -1.0 for invalid readings */
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

/* Calculate the magnitude of a 3D vector */
double cmxd_calculate_magnitude(double x, double y, double z)
{
    return sqrt(x * x + y * y + z * z);
}

/* Normalize a 3D vector */
/* Returns 0 on success, -1 if magnitude is too small */
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

/* Calculate full 0-360° hinge angle from base and lid accelerometer readings */
/* Uses cross product to determine direction and provide full range */
double cmxd_calculate_hinge_angle_360(const struct cmxd_accel_sample *base, const struct cmxd_accel_sample *lid)
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
    
    /* Calculate base angle from dot product (0-180°) */
    double angle = acos(dot_product) * 180.0 / M_PI;
    
    /* Calculate cross product to determine which side of 180° we're on */
    /* Cross product gives us the direction of rotation */
    /* Only need Y component since hinge rotates around Y axis */
    double cross_y = base_norm[2] * lid_norm[0] - base_norm[0] * lid_norm[2];
    
    /* Use the Y component of cross product to determine fold direction */
    /* This assumes the hinge rotates around the Y axis */
    if (cross_y < 0) {
        /* Fold-back direction: 180° + angle gives us 180-360° range */
        angle = 360.0 - angle;
    }
    
    debug_log("Hinge calculation (360°): base[%d,%d,%d] lid[%d,%d,%d] -> dot=%.3f, cross_y=%.3f, angle=%.1f°", 
             base->x, base->y, base->z, lid->x, lid->y, lid->z, dot_product, cross_y, angle);
    
    return angle;
}