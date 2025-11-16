/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Mathematical Calculation Module
 * 
 * Provides simplified mathematical functions for hinge angle determination 
 * using dual accelerometer data. Now simplified with correct mount matrices!
 * 
 * Copyright (c) 2025 Armando DiCianno <armando@noonshy.com>
 */

#ifndef CMXD_CALCULATIONS_H
#define CMXD_CALCULATIONS_H

#include <math.h>
#include <stdint.h>
#include <stdbool.h>

/* Use the common accelerometer sample structure from data module */
#include "cmxd-data.h"

/* Alias for backward compatibility in calculations */
typedef struct accel_sample cmxd_accel_sample;

/* Basic 3D vector operations */
double cmxd_calculate_magnitude(double x, double y, double z);
int cmxd_normalize_vector(double x, double y, double z, 
                         double *norm_x, double *norm_y, double *norm_z);
double cmxd_calculate_dot_product(double x1, double y1, double z1,
                                 double x2, double y2, double z2);

/* Simplified hinge angle calculations */
double cmxd_calculate_hinge_angle(const cmxd_accel_sample *base, const cmxd_accel_sample *lid, 
                                 double base_scale, double lid_scale);
double cmxd_calculate_hinge_angle_360(const cmxd_accel_sample *base, const cmxd_accel_sample *lid,
                                     double base_scale, double lid_scale);

/* Gravity-aware hinge calculations */
bool cmxd_detect_device_rotation(const cmxd_accel_sample *base, const cmxd_accel_sample *lid,
                                double base_scale, double lid_scale);
double cmxd_calculate_gravity_compensated_hinge_angle(const cmxd_accel_sample *base, const cmxd_accel_sample *lid,
                                                     double base_scale, double lid_scale);

/* Utility functions */
double cmxd_calculate_tilt_angle(double x, double y, double z);
double cmxd_clamp(double value, double min_val, double max_val);

/* Helper functions for sensor data processing */
void cmxd_convert_to_ms2(const cmxd_accel_sample *sample, double scale, 
                         double *x_ms, double *y_ms, double *z_ms);
double cmxd_calculate_horizontal_magnitude(double x_ms, double y_ms);

/* Module configuration */
void cmxd_calculations_set_log_debug(void (*func)(const char *fmt, ...));

#endif /* CMXD_CALCULATIONS_H */