/* SPDX-License-Identifier: GPL-2.0 */
/*
 * System paths and constants for CMXD (Chuwi Minibook X Daemon)
 * 
 * Centralized location for all system paths used throughout the application
 * 
 * Copyright (c) 2025 Armando DiCianno <armando@noonshy.com>
 */

#ifndef CMXD_PATHS_H
#define CMXD_PATHS_H

/* Configuration file paths */
#define CMXD_DEFAULT_CONFIG_FILE        "/etc/default/cmxd"

/* Unix domain socket paths */
#define CMXD_RUNTIME_DIR                "/run/cmxd"
#define CMXD_SOCKET_PATH                CMXD_RUNTIME_DIR "/events.sock"

/* Default kernel module sysfs path */
#define CMXD_DEFAULT_SYSFS_PATH         "/sys/devices/platform/cmx"

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

#define IIO_SCAN_ACCEL_X_EN_TEMPLATE    IIO_DEVICES_PATH "/%s/scan_elements/in_accel_x_en"
#define IIO_SCAN_ACCEL_Y_EN_TEMPLATE    IIO_DEVICES_PATH "/%s/scan_elements/in_accel_y_en"
#define IIO_SCAN_ACCEL_Z_EN_TEMPLATE    IIO_DEVICES_PATH "/%s/scan_elements/in_accel_z_en"
#define IIO_SCAN_TIMESTAMP_EN_TEMPLATE  IIO_DEVICES_PATH "/%s/scan_elements/in_timestamp_en"

/* IIO buffer and trigger path templates */
#define IIO_TRIGGER_CURRENT_TEMPLATE    IIO_DEVICES_PATH "/%s/trigger/current_trigger"
#define IIO_BUFFER_ENABLE_TEMPLATE      IIO_DEVICES_PATH "/%s/buffer/enable"

/* Specific trigger paths */
#define IIO_TRIGGER0_PATH               IIO_DEVICES_PATH "/trigger0"

/* Diagnostic command templates */
#define IIO_DEVICES_LIST_CMD            "ls -la " IIO_DEVICES_PATH "/ 2>/dev/null | head -10"
#define IIO_DEV_LIST_CMD                "ls -la " IIO_DEV_BASE_PATH "/iio:device*"

/* Message strings for error reporting */
#define IIO_DEVICES_LIST_MSG            IIO_DEVICES_PATH "/"
#define IIO_DEV_CHAR_MSG                IIO_DEV_BASE_PATH "/iio:device*"

#endif /* CMXD_PATHS_H */