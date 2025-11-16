/* SPDX-License-Identifier: GPL-2.0 */
/*
 * DBus Integration Module for CMXD
 * 
 * Implements DBus interfaces for orientation and tablet mode detection.
 * Provides compatibility with iio-sensor-proxy and custom interfaces.
 * 
 * Copyright (c) 2025 Armando DiCianno <armando@noonshy.com>
 */

#ifdef ENABLE_DBUS

#include "cmxd-dbus.h"
#include "cmxd-protocol.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/select.h>

/* Module state */
static struct cmxd_dbus_config *dbus_config = NULL;
static dbus_log_func_t log_function = NULL;
static DBusConnection *connection = NULL;
static bool initialized = false;

/* Threading state */
static pthread_t dbus_thread;
static volatile int dbus_thread_running = 0;
static volatile int dbus_thread_should_stop = 0;

/* Current state tracking */
static char current_orientation[32] = DBUS_ORIENTATION_NORMAL;
static bool current_tablet_mode = false;
static char current_device_mode[32] = CMXD_PROTOCOL_MODE_LAPTOP;

/* Logging macros */
#define log_error(fmt, ...) do { if (log_function) log_function("ERROR", fmt, ##__VA_ARGS__); } while(0)
#define log_warn(fmt, ...)  do { if (log_function) log_function("WARN", fmt, ##__VA_ARGS__); } while(0)
#define log_info(fmt, ...)  do { if (log_function) log_function("INFO", fmt, ##__VA_ARGS__); } while(0)
#define log_debug(fmt, ...) do { if (log_function) log_function("DEBUG", fmt, ##__VA_ARGS__); } while(0)

/*
 * =============================================================================
 * ORIENTATION CONVERSION UTILITIES
 * =============================================================================
 */

/* Convert CMXD orientation to iio-sensor-proxy format */
const char *cmxd_dbus_convert_orientation_to_sensor_proxy(const char *cmxd_orientation)
{
    if (!cmxd_orientation) return DBUS_ORIENTATION_NORMAL;
    
    if (strcmp(cmxd_orientation, CMXD_PROTOCOL_ORIENTATION_LANDSCAPE) == 0) {
        return DBUS_ORIENTATION_NORMAL;
    } else if (strcmp(cmxd_orientation, CMXD_PROTOCOL_ORIENTATION_PORTRAIT) == 0) {
        return DBUS_ORIENTATION_RIGHT_UP;
    } else if (strcmp(cmxd_orientation, CMXD_PROTOCOL_ORIENTATION_PORTRAIT_FLIPPED) == 0) {
        return DBUS_ORIENTATION_LEFT_UP;
    } else if (strcmp(cmxd_orientation, CMXD_PROTOCOL_ORIENTATION_LANDSCAPE_FLIPPED) == 0) {
        return DBUS_ORIENTATION_BOTTOM_UP;
    }
    
    return DBUS_ORIENTATION_NORMAL;
}

/* Convert iio-sensor-proxy orientation to CMXD format */
const char *cmxd_dbus_convert_orientation_from_sensor_proxy(const char *dbus_orientation)
{
    if (!dbus_orientation) return CMXD_PROTOCOL_ORIENTATION_LANDSCAPE;
    
    if (strcmp(dbus_orientation, DBUS_ORIENTATION_NORMAL) == 0) {
        return CMXD_PROTOCOL_ORIENTATION_LANDSCAPE;
    } else if (strcmp(dbus_orientation, DBUS_ORIENTATION_RIGHT_UP) == 0) {
        return CMXD_PROTOCOL_ORIENTATION_PORTRAIT;
    } else if (strcmp(dbus_orientation, DBUS_ORIENTATION_LEFT_UP) == 0) {
        return CMXD_PROTOCOL_ORIENTATION_PORTRAIT_FLIPPED;
    } else if (strcmp(dbus_orientation, DBUS_ORIENTATION_BOTTOM_UP) == 0) {
        return CMXD_PROTOCOL_ORIENTATION_LANDSCAPE_FLIPPED;
    }
    
    return CMXD_PROTOCOL_ORIENTATION_LANDSCAPE;
}

/*
 * =============================================================================
 * DBUS MESSAGE PROCESSING THREAD
 * =============================================================================
 */

/* DBus message processing thread */
static void *dbus_message_thread(void *arg)
{
    (void)arg; /* Unused parameter */
    
    log_info("DBus message thread started");
    dbus_thread_running = 1;
    
    while (!dbus_thread_should_stop) {
        if (!connection) {
            usleep(100000); /* 100ms */
            continue;
        }
        
        /* Process pending DBus messages with a timeout */
        if (dbus_connection_read_write_dispatch(connection, 100)) {
            /* Continue processing */
            while (dbus_connection_dispatch(connection) == DBUS_DISPATCH_DATA_REMAINS) {
                /* Process all pending messages */
            }
        }
        
        /* Small sleep to prevent busy waiting */
        usleep(10000); /* 10ms */
    }
    
    log_info("DBus message thread stopping");
    dbus_thread_running = 0;
    return NULL;
}

/*
 * =============================================================================
 * DBUS MESSAGE HANDLING
 * =============================================================================
 */

/* Handle DBus method calls */
static DBusHandlerResult handle_method_call(DBusConnection *conn, DBusMessage *message, void *user_data __attribute__((unused)))
{
    const char *interface = dbus_message_get_interface(message);
    const char *method = dbus_message_get_member(message);
    const char *path = dbus_message_get_path(message);
    
    log_debug("DBus method call: %s.%s on %s", interface ? interface : "null", 
              method ? method : "null", path ? path : "null");
    
    /* Handle Properties.Get and Properties.GetAll */
    if (interface && strcmp(interface, "org.freedesktop.DBus.Properties") == 0) {
        if (method && strcmp(method, "Get") == 0) {
            /* Handle individual property requests */
            DBusMessage *reply = dbus_message_new_method_return(message);
            if (reply) {
                /* Add property value based on requested interface/property */
                dbus_connection_send(conn, reply, NULL);
                dbus_message_unref(reply);
                return DBUS_HANDLER_RESULT_HANDLED;
            }
        } else if (method && strcmp(method, "GetAll") == 0) {
            /* Handle get all properties requests */
            DBusMessage *reply = dbus_message_new_method_return(message);
            if (reply) {
                DBusMessageIter iter, dict_iter;
                dbus_message_iter_init_append(reply, &iter);
                dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY, "{sv}", &dict_iter);
                
                /* Add orientation property for sensor proxy interface */
                if (path && strcmp(path, SENSOR_PROXY_OBJECT_PATH) == 0) {
                    DBusMessageIter entry_iter, variant_iter;
                    const char *key = "AccelerometerOrientation";
                    const char *value = current_orientation;
                    
                    dbus_message_iter_open_container(&dict_iter, DBUS_TYPE_DICT_ENTRY, NULL, &entry_iter);
                    dbus_message_iter_append_basic(&entry_iter, DBUS_TYPE_STRING, &key);
                    dbus_message_iter_open_container(&entry_iter, DBUS_TYPE_VARIANT, "s", &variant_iter);
                    dbus_message_iter_append_basic(&variant_iter, DBUS_TYPE_STRING, &value);
                    dbus_message_iter_close_container(&entry_iter, &variant_iter);
                    dbus_message_iter_close_container(&dict_iter, &entry_iter);
                    
                    /* HasAccelerometer property */
                    key = "HasAccelerometer";
                    dbus_bool_t has_accel = TRUE;
                    dbus_message_iter_open_container(&dict_iter, DBUS_TYPE_DICT_ENTRY, NULL, &entry_iter);
                    dbus_message_iter_append_basic(&entry_iter, DBUS_TYPE_STRING, &key);
                    dbus_message_iter_open_container(&entry_iter, DBUS_TYPE_VARIANT, "b", &variant_iter);
                    dbus_message_iter_append_basic(&variant_iter, DBUS_TYPE_BOOLEAN, &has_accel);
                    dbus_message_iter_close_container(&entry_iter, &variant_iter);
                    dbus_message_iter_close_container(&dict_iter, &entry_iter);
                }
                
                /* Add tablet mode properties for tablet mode interface */
                if (path && strcmp(path, CMXD_DBUS_OBJECT_PATH) == 0) {
                    DBusMessageIter entry_iter, variant_iter;
                    const char *key = "TabletMode";
                    dbus_bool_t tablet_mode = current_tablet_mode;
                    
                    dbus_message_iter_open_container(&dict_iter, DBUS_TYPE_DICT_ENTRY, NULL, &entry_iter);
                    dbus_message_iter_append_basic(&entry_iter, DBUS_TYPE_STRING, &key);
                    dbus_message_iter_open_container(&entry_iter, DBUS_TYPE_VARIANT, "b", &variant_iter);
                    dbus_message_iter_append_basic(&variant_iter, DBUS_TYPE_BOOLEAN, &tablet_mode);
                    dbus_message_iter_close_container(&entry_iter, &variant_iter);
                    dbus_message_iter_close_container(&dict_iter, &entry_iter);
                    
                    /* DeviceMode property */
                    key = "DeviceMode";
                    const char *mode_value = current_device_mode;
                    dbus_message_iter_open_container(&dict_iter, DBUS_TYPE_DICT_ENTRY, NULL, &entry_iter);
                    dbus_message_iter_append_basic(&entry_iter, DBUS_TYPE_STRING, &key);
                    dbus_message_iter_open_container(&entry_iter, DBUS_TYPE_VARIANT, "s", &variant_iter);
                    dbus_message_iter_append_basic(&variant_iter, DBUS_TYPE_STRING, &mode_value);
                    dbus_message_iter_close_container(&entry_iter, &variant_iter);
                    dbus_message_iter_close_container(&dict_iter, &entry_iter);
                }
                
                dbus_message_iter_close_container(&iter, &dict_iter);
                dbus_connection_send(conn, reply, NULL);
                dbus_message_unref(reply);
                return DBUS_HANDLER_RESULT_HANDLED;
            }
        }
    }
    
    /* Handle Introspect method */
    if (interface && strcmp(interface, "org.freedesktop.DBus.Introspectable") == 0 &&
        method && strcmp(method, "Introspect") == 0) {
        
        const char *introspect_xml = 
            "<!DOCTYPE node PUBLIC \"-//freedesktop//DTD D-BUS Object Introspection 1.0//EN\"\n"
            " \"http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd\">\n"
            "<node>\n"
            "  <interface name=\"org.freedesktop.DBus.Introspectable\">\n"
            "    <method name=\"Introspect\">\n"
            "      <arg name=\"data\" direction=\"out\" type=\"s\"/>\n"
            "    </method>\n"
            "  </interface>\n"
            "  <interface name=\"org.freedesktop.DBus.Properties\">\n"
            "    <method name=\"Get\">\n"
            "      <arg name=\"interface\" direction=\"in\" type=\"s\"/>\n"
            "      <arg name=\"property\" direction=\"in\" type=\"s\"/>\n"
            "      <arg name=\"value\" direction=\"out\" type=\"v\"/>\n"
            "    </method>\n"
            "    <method name=\"GetAll\">\n"
            "      <arg name=\"interface\" direction=\"in\" type=\"s\"/>\n"
            "      <arg name=\"properties\" direction=\"out\" type=\"a{sv}\"/>\n"
            "    </method>\n"
            "    <signal name=\"PropertiesChanged\">\n"
            "      <arg name=\"interface\" type=\"s\"/>\n"
            "      <arg name=\"changed_properties\" type=\"a{sv}\"/>\n"
            "      <arg name=\"invalidated_properties\" type=\"as\"/>\n"
            "    </signal>\n"
            "  </interface>\n";
        
        char *full_xml;
        if (path && strcmp(path, SENSOR_PROXY_OBJECT_PATH) == 0) {
            /* Sensor proxy interface introspection */
            if (asprintf(&full_xml, "%s"
                "  <interface name=\"%s\">\n"
                "    <property name=\"AccelerometerOrientation\" type=\"s\" access=\"read\"/>\n"
                "    <property name=\"HasAccelerometer\" type=\"b\" access=\"read\"/>\n"
                "  </interface>\n"
                "</node>\n", introspect_xml, SENSOR_PROXY_INTERFACE) < 0) {
                full_xml = NULL;
            }
        } else {
            /* Tablet mode interface introspection */
            if (asprintf(&full_xml, "%s"
                "  <interface name=\"%s\">\n"
                "    <property name=\"TabletMode\" type=\"b\" access=\"read\"/>\n"
                "    <property name=\"DeviceMode\" type=\"s\" access=\"read\"/>\n"
                "    <signal name=\"TabletModeChanged\">\n"
                "      <arg name=\"tablet_mode\" type=\"b\"/>\n"
                "    </signal>\n"
                "  </interface>\n"
                "</node>\n", introspect_xml, TABLET_MODE_INTERFACE) < 0) {
                full_xml = NULL;
            }
        }
        
        DBusMessage *reply = dbus_message_new_method_return(message);
        if (reply && full_xml) {
            dbus_message_append_args(reply, DBUS_TYPE_STRING, &full_xml, DBUS_TYPE_INVALID);
            dbus_connection_send(conn, reply, NULL);
            dbus_message_unref(reply);
            free(full_xml);
            return DBUS_HANDLER_RESULT_HANDLED;
        }
        if (full_xml) free(full_xml);
    }
    
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

/*
 * =============================================================================
 * PUBLIC API IMPLEMENTATION
 * =============================================================================
 */

/* Initialize DBus module */
int cmxd_dbus_init(struct cmxd_dbus_config *config, dbus_log_func_t log_func)
{
    DBusError error;
    
    if (initialized) {
        return 0;  /* Already initialized */
    }
    
    dbus_config = config;
    log_function = log_func;
    
    dbus_error_init(&error);
    
    /* Connect to system bus (more appropriate for system daemon) */
    connection = dbus_bus_get(DBUS_BUS_SYSTEM, &error);
    if (dbus_error_is_set(&error)) {
        log_warn("Failed to connect to DBus system bus: %s", error.message);
        dbus_error_free(&error);
        
        /* Try session bus as fallback */
        dbus_error_init(&error);
        connection = dbus_bus_get(DBUS_BUS_SESSION, &error);
        if (dbus_error_is_set(&error)) {
            log_error("Failed to connect to DBus session bus: %s", error.message);
            dbus_error_free(&error);
            return -1;
        }
        log_info("Using DBus session bus as fallback");
    } else {
        log_info("Using DBus system bus");
    }
    
    if (!connection) {
        log_error("DBus connection is NULL");
        return -1;
    }
    
    /* Request service names */
    int ret = dbus_bus_request_name(connection, CMXD_DBUS_SERVICE_NAME, 
                                   DBUS_NAME_FLAG_REPLACE_EXISTING, &error);
    if (dbus_error_is_set(&error)) {
        log_error("Failed to request DBus service name %s: %s", CMXD_DBUS_SERVICE_NAME, error.message);
        dbus_error_free(&error);
        dbus_connection_unref(connection);
        connection = NULL;
        return -1;
    }
    
    if (ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
        log_error("DBus service name %s could not be acquired (reply: %d)", CMXD_DBUS_SERVICE_NAME, ret);
        dbus_connection_unref(connection);
        connection = NULL;
        return -1;
    }
    
    log_info("Successfully registered DBus service: %s", CMXD_DBUS_SERVICE_NAME);
    
    /* Register sensor proxy service name if enabled */
    if (config->enable_sensor_proxy) {
        ret = dbus_bus_request_name(connection, "net.hadess.SensorProxy", 
                                   DBUS_NAME_FLAG_REPLACE_EXISTING, &error);
        if (dbus_error_is_set(&error)) {
            log_error("Failed to request sensor proxy service name: %s", error.message);
            dbus_error_free(&error);
            dbus_connection_unref(connection);
            connection = NULL;
            return -1;
        }
        
        if (ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
            log_error("Sensor proxy service name could not be acquired (reply: %d)", ret);
            dbus_connection_unref(connection);
            connection = NULL;
            return -1;
        }
        
        log_info("Successfully registered sensor proxy service: net.hadess.SensorProxy");
    }
    
    /* Register object path handlers */
    DBusObjectPathVTable vtable = {
        .message_function = handle_method_call
    };
    
    if (!dbus_connection_register_object_path(connection, CMXD_DBUS_OBJECT_PATH, &vtable, NULL)) {
        log_error("Failed to register DBus object path %s", CMXD_DBUS_OBJECT_PATH);
        dbus_connection_unref(connection);
        connection = NULL;
        return -1;
    }
    
    /* Register sensor proxy path if enabled */
    if (config->enable_sensor_proxy) {
        if (!dbus_connection_register_object_path(connection, SENSOR_PROXY_OBJECT_PATH, &vtable, NULL)) {
            log_error("Failed to register sensor proxy object path %s", SENSOR_PROXY_OBJECT_PATH);
            dbus_connection_unref(connection);
            connection = NULL;
            return -1;
        } else {
            log_info("Registered sensor proxy compatibility interface");
        }
    }
    
    initialized = true;
    
    /* Start DBus message processing thread */
    if (pthread_create(&dbus_thread, NULL, dbus_message_thread, NULL) != 0) {
        log_error("Failed to create DBus message thread");
        dbus_connection_unref(connection);
        connection = NULL;
        initialized = false;
        return -1;
    }
    
    log_info("DBus module initialized successfully");
    
    return 0;
}

/* Cleanup DBus module */
void cmxd_dbus_cleanup(void)
{
    if (!initialized || !connection) {
        return;
    }
    
    /* Stop DBus message thread */
    if (dbus_thread_running) {
        dbus_thread_should_stop = 1;
        log_debug("Stopping DBus message thread");
        if (pthread_join(dbus_thread, NULL) == 0) {
            log_debug("DBus message thread stopped");
        } else {
            log_warn("Failed to join DBus message thread");
        }
    }
    
    /* Unregister object paths */
    dbus_connection_unregister_object_path(connection, CMXD_DBUS_OBJECT_PATH);
    if (dbus_config && dbus_config->enable_sensor_proxy) {
        dbus_connection_unregister_object_path(connection, SENSOR_PROXY_OBJECT_PATH);
    }
    
    /* Release service name */
    DBusError error;
    dbus_error_init(&error);
    dbus_bus_release_name(connection, CMXD_DBUS_SERVICE_NAME, &error);
    if (dbus_error_is_set(&error)) {
        log_warn("Error releasing DBus service name: %s", error.message);
        dbus_error_free(&error);
    }
    
    /* Close connection */
    dbus_connection_unref(connection);
    connection = NULL;
    initialized = false;
    
    log_info("DBus module cleaned up");
}

/* Publish orientation change */
int cmxd_dbus_publish_orientation(const char *orientation)
{
    if (!initialized || !connection || !orientation) {
        return -1;
    }
    
    const char *dbus_orientation = cmxd_dbus_convert_orientation_to_sensor_proxy(orientation);
    
    /* Update current state */
    strncpy(current_orientation, dbus_orientation, sizeof(current_orientation) - 1);
    current_orientation[sizeof(current_orientation) - 1] = '\0';
    
    /* Send PropertiesChanged signal for sensor proxy interface */
    if (dbus_config && dbus_config->enable_sensor_proxy) {
        DBusMessage *signal = dbus_message_new_signal(SENSOR_PROXY_OBJECT_PATH, 
                                                      "org.freedesktop.DBus.Properties", 
                                                      "PropertiesChanged");
        if (signal) {
            DBusMessageIter iter, dict_iter, entry_iter, variant_iter, array_iter;
            
            dbus_message_iter_init_append(signal, &iter);
            
            /* Interface name */
            const char *interface = SENSOR_PROXY_INTERFACE;
            dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &interface);
            
            /* Changed properties dictionary */
            dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY, "{sv}", &dict_iter);
            dbus_message_iter_open_container(&dict_iter, DBUS_TYPE_DICT_ENTRY, NULL, &entry_iter);
            
            const char *property = "AccelerometerOrientation";
            dbus_message_iter_append_basic(&entry_iter, DBUS_TYPE_STRING, &property);
            dbus_message_iter_open_container(&entry_iter, DBUS_TYPE_VARIANT, "s", &variant_iter);
            dbus_message_iter_append_basic(&variant_iter, DBUS_TYPE_STRING, &dbus_orientation);
            dbus_message_iter_close_container(&entry_iter, &variant_iter);
            dbus_message_iter_close_container(&dict_iter, &entry_iter);
            dbus_message_iter_close_container(&iter, &dict_iter);
            
            /* Invalidated properties (empty array) */
            dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY, "s", &array_iter);
            dbus_message_iter_close_container(&iter, &array_iter);
            
            dbus_connection_send(connection, signal, NULL);
            dbus_message_unref(signal);
            dbus_connection_flush(connection);
        }
    }
    
    log_debug("Published orientation change: %s -> %s", orientation, dbus_orientation);
    return 0;
}

/* Publish tablet mode change */
int cmxd_dbus_publish_tablet_mode(bool is_tablet_mode)
{
    if (!initialized || !connection) {
        return -1;
    }
    
    current_tablet_mode = is_tablet_mode;
    
    /* Send custom TabletModeChanged signal */
    DBusMessage *signal = dbus_message_new_signal(CMXD_DBUS_OBJECT_PATH, TABLET_MODE_INTERFACE, "TabletModeChanged");
    if (signal) {
        dbus_bool_t tablet_mode = is_tablet_mode;
        dbus_message_append_args(signal, DBUS_TYPE_BOOLEAN, &tablet_mode, DBUS_TYPE_INVALID);
        dbus_connection_send(connection, signal, NULL);
        dbus_message_unref(signal);
    }
    
    /* Send PropertiesChanged signal */
    signal = dbus_message_new_signal(CMXD_DBUS_OBJECT_PATH, 
                                    "org.freedesktop.DBus.Properties", 
                                    "PropertiesChanged");
    if (signal) {
        DBusMessageIter iter, dict_iter, entry_iter, variant_iter, array_iter;
        
        dbus_message_iter_init_append(signal, &iter);
        
        /* Interface name */
        const char *interface = TABLET_MODE_INTERFACE;
        dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &interface);
        
        /* Changed properties dictionary */
        dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY, "{sv}", &dict_iter);
        dbus_message_iter_open_container(&dict_iter, DBUS_TYPE_DICT_ENTRY, NULL, &entry_iter);
        
        const char *property = "TabletMode";
        dbus_bool_t tablet_mode = is_tablet_mode;
        dbus_message_iter_append_basic(&entry_iter, DBUS_TYPE_STRING, &property);
        dbus_message_iter_open_container(&entry_iter, DBUS_TYPE_VARIANT, "b", &variant_iter);
        dbus_message_iter_append_basic(&variant_iter, DBUS_TYPE_BOOLEAN, &tablet_mode);
        dbus_message_iter_close_container(&entry_iter, &variant_iter);
        dbus_message_iter_close_container(&dict_iter, &entry_iter);
        dbus_message_iter_close_container(&iter, &dict_iter);
        
        /* Invalidated properties (empty array) */
        dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY, "s", &array_iter);
        dbus_message_iter_close_container(&iter, &array_iter);
        
        dbus_connection_send(connection, signal, NULL);
        dbus_message_unref(signal);
    }
    
    dbus_connection_flush(connection);
    log_debug("Published tablet mode change: %s", is_tablet_mode ? "true" : "false");
    
    return 0;
}

/* Publish device mode change */
int cmxd_dbus_publish_device_mode(const char *device_mode)
{
    if (!initialized || !connection || !device_mode) {
        return -1;
    }
    
    strncpy(current_device_mode, device_mode, sizeof(current_device_mode) - 1);
    current_device_mode[sizeof(current_device_mode) - 1] = '\0';
    
    /* Update tablet mode based on device mode */
    bool is_tablet = (strcmp(device_mode, CMXD_PROTOCOL_MODE_TABLET) == 0);
    if (is_tablet != current_tablet_mode) {
        cmxd_dbus_publish_tablet_mode(is_tablet);
    }
    
    /* Send PropertiesChanged signal for DeviceMode */
    DBusMessage *signal = dbus_message_new_signal(CMXD_DBUS_OBJECT_PATH, 
                                                  "org.freedesktop.DBus.Properties", 
                                                  "PropertiesChanged");
    if (signal) {
        DBusMessageIter iter, dict_iter, entry_iter, variant_iter, array_iter;
        
        dbus_message_iter_init_append(signal, &iter);
        
        /* Interface name */
        const char *interface = TABLET_MODE_INTERFACE;
        dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &interface);
        
        /* Changed properties dictionary */
        dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY, "{sv}", &dict_iter);
        dbus_message_iter_open_container(&dict_iter, DBUS_TYPE_DICT_ENTRY, NULL, &entry_iter);
        
        const char *property = "DeviceMode";
        dbus_message_iter_append_basic(&entry_iter, DBUS_TYPE_STRING, &property);
        dbus_message_iter_open_container(&entry_iter, DBUS_TYPE_VARIANT, "s", &variant_iter);
        dbus_message_iter_append_basic(&variant_iter, DBUS_TYPE_STRING, &device_mode);
        dbus_message_iter_close_container(&entry_iter, &variant_iter);
        dbus_message_iter_close_container(&dict_iter, &entry_iter);
        dbus_message_iter_close_container(&iter, &dict_iter);
        
        /* Invalidated properties (empty array) */
        dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY, "s", &array_iter);
        dbus_message_iter_close_container(&iter, &array_iter);
        
        dbus_connection_send(connection, signal, NULL);
        dbus_message_unref(signal);
        dbus_connection_flush(connection);
    }
    
    log_debug("Published device mode change: %s", device_mode);
    return 0;
}

/* Property getters */
const char *cmxd_dbus_get_current_orientation(void)
{
    return current_orientation;
}

bool cmxd_dbus_get_tablet_mode(void)
{
    return current_tablet_mode;
}

const char *cmxd_dbus_get_device_mode(void)
{
    return current_device_mode;
}

bool cmxd_dbus_has_accelerometer(void)
{
    return true;  /* We always have accelerometer data */
}

/* Process DBus events */
int cmxd_dbus_process_events(int timeout_ms __attribute__((unused)))
{
    if (!initialized || !connection) {
        return -1;
    }
    
    /* Process pending messages */
    while (dbus_connection_dispatch(connection) == DBUS_DISPATCH_DATA_REMAINS) {
        /* Continue processing */
    }
    
    return 0;
}

/* Get DBus file descriptor for main loop integration */
int cmxd_dbus_get_fd(void)
{
    if (!initialized || !connection) {
        return -1;
    }
    
    /* DBus doesn't expose the raw FD in a simple way */
    /* This would require more complex integration with DBus main loop */
    return -1;
}

#endif /* ENABLE_DBUS */