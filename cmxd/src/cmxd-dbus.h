/* SPDX-License-Identifier: GPL-2.0 */
/*
 * DBus Integration Module for CMXD
 * 
 * Provides DBus interface compatibility with iio-sensor-proxy and custom
 * tablet mode interface. Implements both net.hadess.SensorProxy for GNOME
 * compatibility and com.noonshy.TabletMode1 for extended functionality.
 * 
 * Copyright (c) 2025 Armando DiCianno <armando@noonshy.com>
 */

#ifndef CMXD_DBUS_H
#define CMXD_DBUS_H

#ifdef ENABLE_DBUS

#include <stdbool.h>
#include <dbus/dbus.h>

/* DBus service and interface names */
#define CMXD_DBUS_SERVICE_NAME          "com.noonshy.cmxd"
#define CMXD_DBUS_OBJECT_PATH           "/com/noonshy/cmxd"

/* iio-sensor-proxy compatibility interface */
#define SENSOR_PROXY_INTERFACE          "net.hadess.SensorProxy"
#define SENSOR_PROXY_OBJECT_PATH        "/net/hadess/SensorProxy"

/* Custom tablet mode interface */
#define TABLET_MODE_INTERFACE           "com.noonshy.TabletMode1" 
#define FREEDESKTOP_TABLET_INTERFACE    "org.freedesktop.TabletMode1"

/* DBus orientation values (iio-sensor-proxy compatibility) */
#define DBUS_ORIENTATION_NORMAL         "normal"
#define DBUS_ORIENTATION_LEFT_UP        "left-up"
#define DBUS_ORIENTATION_RIGHT_UP       "right-up"
#define DBUS_ORIENTATION_BOTTOM_UP      "bottom-up"

/* Configuration structure */
struct cmxd_dbus_config {
    bool enable_sensor_proxy;       /* Mirror iio-sensor-proxy interface */
    bool enable_tablet_mode;        /* Custom tablet mode interface */
    bool enable_freedesktop;        /* org.freedesktop.TabletMode1 interface */
    int verbose;                    /* Debug logging */
};

/* Logging function type */
typedef void (*dbus_log_func_t)(const char *level, const char *fmt, ...);

/* Module initialization and cleanup */
int cmxd_dbus_init(struct cmxd_dbus_config *config, dbus_log_func_t log_func);
void cmxd_dbus_cleanup(void);

/* Event publishing functions */
int cmxd_dbus_publish_orientation(const char *orientation);
int cmxd_dbus_publish_tablet_mode(bool is_tablet_mode);
int cmxd_dbus_publish_device_mode(const char *device_mode);

/* Property getters (for DBus introspection) */
const char *cmxd_dbus_get_current_orientation(void);
bool cmxd_dbus_get_tablet_mode(void);
const char *cmxd_dbus_get_device_mode(void);
bool cmxd_dbus_has_accelerometer(void);

/* Orientation conversion utilities */
const char *cmxd_dbus_convert_orientation_to_sensor_proxy(const char *cmxd_orientation);
const char *cmxd_dbus_convert_orientation_from_sensor_proxy(const char *dbus_orientation);

/* DBus main loop integration */
int cmxd_dbus_process_events(int timeout_ms);
int cmxd_dbus_get_fd(void);

#endif /* ENABLE_DBUS */

#endif /* CMXD_DBUS_H */