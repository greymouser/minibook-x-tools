/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Mathematical calculations for CMXD (Chuwi Minibook X Daemon)
 * 
 * Shared mathematical functions for accelerometer data processing
 * 
 * Copyright (c) 2025 Armando DiCianno <armando@noonshy.com>
 */

#include "cmxd-calculations.h"

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