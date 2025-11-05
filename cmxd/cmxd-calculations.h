/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Mathematical calculations for CMXD (Chuwi Minibook X Daemon)
 * 
 * Shared mathematical functions for accelerometer data processing
 * 
 * Copyright (c) 2025 Armando DiCianno <armando@noonshy.com>
 */

#ifndef CMXD_CALCULATIONS_H
#define CMXD_CALCULATIONS_H

#include <math.h>
#include <stdint.h>

/* Calculate the tilt angle of the device from horizontal plane */
/* Returns angle in degrees: 0° = flat, 90° = vertical */
/* Returns -1.0 for invalid readings */
double cmxd_calculate_tilt_angle(double x, double y, double z);

/* Calculate the magnitude of a 3D vector */
double cmxd_calculate_magnitude(double x, double y, double z);

/* Normalize a 3D vector */
/* Returns 0 on success, -1 if magnitude is too small */
int cmxd_normalize_vector(double x, double y, double z, 
                         double *norm_x, double *norm_y, double *norm_z);

/* Calculate dot product of two 3D vectors */
double cmxd_calculate_dot_product(double x1, double y1, double z1,
                                 double x2, double y2, double z2);

/* Clamp a value to a specified range */
double cmxd_clamp(double value, double min_val, double max_val);

/* Convert radians to degrees */
double cmxd_rad_to_deg(double radians);

/* Convert degrees to radians */
double cmxd_deg_to_rad(double degrees);

/* Accelerometer sample structure */
struct cmxd_accel_sample {
    int x, y, z;
    uint64_t timestamp;
};

/* Calculate hinge angle from base and lid accelerometer readings */
/* Uses orientation-independent dot product calculation with direction detection for full 0-360° range */
double cmxd_calculate_hinge_angle(const struct cmxd_accel_sample *base, const struct cmxd_accel_sample *lid);

/* Set the debug logging function for calculations module */
void cmxd_calculations_set_log_debug(void (*func)(const char *fmt, ...));

#endif /* CMXD_CALCULATIONS_H */