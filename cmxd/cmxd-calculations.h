/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Mathematical Calculation Module
 * 
 * Provides mathematical functions for 3D vector operations, tilt calculations,
 * and hinge angle determination using dual accelerometer data.
 * 
 * Copyright (c) 2025 Armando DiCianno <armando@noonshy.com>
 */

#ifndef CMXD_CALCULATIONS_H
#define CMXD_CALCULATIONS_H

#include <math.h>
#include <stdint.h>

/* Accelerometer data sample structure */
struct cmxd_accel_sample {
    int x, y, z;           /* Raw accelerometer values */
    uint64_t timestamp;    /* Sample timestamp */
};

/* Basic 3D vector operations */
double cmxd_calculate_magnitude(double x, double y, double z);
int cmxd_normalize_vector(double x, double y, double z, 
                         double *norm_x, double *norm_y, double *norm_z);
double cmxd_calculate_dot_product(double x1, double y1, double z1,
                                 double x2, double y2, double z2);

/* Angle calculations */
double cmxd_calculate_tilt_angle(double x, double y, double z);
double cmxd_calculate_hinge_angle(const struct cmxd_accel_sample *base, const struct cmxd_accel_sample *lid);
double cmxd_calculate_hinge_angle_360(const struct cmxd_accel_sample *base, const struct cmxd_accel_sample *lid);

/* Utility functions */
double cmxd_clamp(double value, double min_val, double max_val);
double cmxd_rad_to_deg(double radians);
double cmxd_deg_to_rad(double degrees);

/* Module configuration */
void cmxd_calculations_set_log_debug(void (*func)(const char *fmt, ...));

#endif /* CMXD_CALCULATIONS_H */