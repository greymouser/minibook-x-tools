/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Data handling for CMXD (Chuwi Minibook X Daemon)
 * 
 * File I/O operations, IIO device handling, and sysfs communication
 * 
 * Copyright (c) 2025 Armando DiCianno <armando@noonshy.com>
 */

#ifndef CMXD_DATA_H
#define CMXD_DATA_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

/* Device name maximum length */
#define DEVICE_NAME_MAX 128

/* IIO Buffer structure for event-driven reading */
struct iio_buffer {
    char device_name[DEVICE_NAME_MAX];
    int buffer_fd;
    int trigger_fd;
    char trigger_name[64];
    int x_index, y_index, z_index, timestamp_index;
    int sample_size;
    int enabled;
};

/* Accelerometer sample data */
struct accel_sample {
    int x, y, z;
    uint64_t timestamp;
};

/* Configuration structure (needs to be passed to data functions) */
struct cmxd_data_config {
    char sysfs_path[PATH_MAX];
    int verbose;
};

/* Logging function pointer type */
typedef void (*log_func_t)(const char *level, const char *fmt, ...);

/* Initialize data module */
void cmxd_data_init(struct cmxd_data_config *config, log_func_t log_function);

/* Safe file operations */
FILE *cmxd_safe_fopen(const char *path, const char *mode);
int cmxd_safe_fclose(FILE *f, const char *path);

/* Scale and unit conversion */
void cmxd_apply_scale(int raw_x, int raw_y, int raw_z, double scale,
                      int *scaled_x, int *scaled_y, int *scaled_z);

/* Sysfs write operations */
int cmxd_write_vector(const char *name, int x, int y, int z);
int cmxd_write_mode(const char *mode);
int cmxd_write_orientation(const char *orientation);

/* IIO device discovery */
int cmxd_find_iio_device_for_i2c(int bus, int addr, char *device_name, size_t name_size);

/* IIO buffer management */
int cmxd_ensure_iio_trigger_exists(void);
int cmxd_setup_iio_buffer(struct iio_buffer *buf, const char *device_name);
int cmxd_read_iio_buffer_sample(struct iio_buffer *buf, struct accel_sample *sample);
void cmxd_cleanup_iio_buffer(struct iio_buffer *buf);

/* Utility functions */
int cmxd_parse_accel_value(const uint8_t *data);
int cmxd_wait_for_path(const char *path, int timeout_sec);
int cmxd_validate_paths(const char *base_dev, const char *lid_dev);

/* Kernel device assignment reading */
int cmxd_read_kernel_device_assignments(char *base_dev, size_t base_size, 
                                        char *lid_dev, size_t lid_size);

#endif /* CMXD_DATA_H */