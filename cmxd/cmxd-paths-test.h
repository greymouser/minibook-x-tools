/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Test paths and constants for CMXD (Chuwi Minibook X Daemon)
 * 
 * Test version that uses /tmp instead of /run for development testing
 * 
 * Copyright (c) 2025 Armando DiCianno <armando@noonshy.com>
 */

#ifndef CMXD_PATHS_H
#define CMXD_PATHS_H

/* Configuration file paths */
#define CMXD_DEFAULT_CONFIG_FILE        "/etc/default/cmxd"

/* Unix domain socket paths - using /tmp for testing */
#define CMXD_RUNTIME_DIR                "/tmp/cmxd"
#define CMXD_SOCKET_PATH                CMXD_RUNTIME_DIR "/events.sock"

/* Default kernel module sysfs path - using /tmp for testing */
#define CMXD_DEFAULT_SYSFS_PATH         "/tmp/cmxd"

/* IIO subsystem base paths */
#define IIO_BASE_PATH                   "/sys/bus/iio"
#define IIO_DEVICES_PATH                IIO_BASE_PATH "/devices"
#define IIO_SYSFS_TRIGGER_PATH          IIO_DEVICES_PATH "/iio_sysfs_trigger"
#define IIO_SYSFS_TRIGGER_ADD_PATH      IIO_SYSFS_TRIGGER_PATH "/add_trigger"

/* IIO device character device paths */
#define IIO_DEV_BASE_PATH               "/dev"
#define IIO_DEV_DEVICE0                 IIO_DEV_BASE_PATH "/iio:device0"
#define IIO_DEV_DEVICE1                 IIO_DEV_BASE_PATH "/iio:device1"

/* IIO device path templates (use with snprintf) */
#define IIO_DEVICE_TEMPLATE             IIO_DEVICES_PATH "/iio:device%d/device"
#define IIO_TRIGGER_TEMPLATE            IIO_DEVICES_PATH "/trigger%d"
#define IIO_TRIGGER_NOW_TEMPLATE        IIO_DEVICES_PATH "/trigger%d/trigger_now"
#define IIO_DEVICE_PATH_TEMPLATE        IIO_DEVICES_PATH "/%s"
#define IIO_DEV_CHAR_TEMPLATE           IIO_DEV_BASE_PATH "/%s"

/* IIO device attribute path templates */
#define IIO_ACCEL_X_RAW_TEMPLATE        IIO_DEVICES_PATH "/%s/in_accel_x_raw"
#define IIO_ACCEL_Y_RAW_TEMPLATE        IIO_DEVICES_PATH "/%s/in_accel_y_raw"
#define IIO_ACCEL_Z_RAW_TEMPLATE        IIO_DEVICES_PATH "/%s/in_accel_z_raw"

/* IIO scan elements path templates */
#define IIO_SCAN_ACCEL_X_INDEX_TEMPLATE IIO_DEVICES_PATH "/%s/scan_elements/in_accel_x_index"
#define IIO_SCAN_ACCEL_Y_INDEX_TEMPLATE IIO_DEVICES_PATH "/%s/scan_elements/in_accel_y_index"
#define IIO_SCAN_ACCEL_Z_INDEX_TEMPLATE IIO_DEVICES_PATH "/%s/scan_elements/in_accel_z_index"
#define IIO_SCAN_TIMESTAMP_INDEX_TEMPLATE IIO_DEVICES_PATH "/%s/scan_elements/in_timestamp_index"

/* IIO buffer management */
#define IIO_BUFFER_ENABLE_TEMPLATE      IIO_DEVICES_PATH "/%s/buffer/enable"
#define IIO_BUFFER_LENGTH_TEMPLATE      IIO_DEVICES_PATH "/%s/buffer/length"

/* IIO scan enable templates */
#define IIO_SCAN_ACCEL_X_EN_TEMPLATE    IIO_DEVICES_PATH "/%s/scan_elements/in_accel_x_en"
#define IIO_SCAN_ACCEL_Y_EN_TEMPLATE    IIO_DEVICES_PATH "/%s/scan_elements/in_accel_y_en"
#define IIO_SCAN_ACCEL_Z_EN_TEMPLATE    IIO_DEVICES_PATH "/%s/scan_elements/in_accel_z_en"
#define IIO_SCAN_TIMESTAMP_EN_TEMPLATE  IIO_DEVICES_PATH "/%s/scan_elements/in_timestamp_en"

/* Default configuration values */
#define CMXD_DEFAULT_TIMEOUT_MS         100
#define CMXD_DEFAULT_BUFFER_LENGTH      64
#define CMXD_DEFAULT_POLL_INTERVAL_MS   50

/* Error handling constants */
#define CMXD_MAX_RETRY_COUNT           3
#define CMXD_RETRY_DELAY_MS           1000

#endif /* CMXD_PATHS_H */