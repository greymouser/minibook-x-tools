/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Event System for CMXD (Chuwi Minibook X Daemon)
 * 
 * Provides event publishing capabilities including Unix Domain Sockets
 * and DBus notifications for mode and orientation changes.
 * 
 * Copyright (c) 2025 Armando DiCianno <armando@noonshy.com>
 */

#ifndef CMXD_EVENTS_H
#define CMXD_EVENTS_H

/* Event types */
typedef enum {
    CMXD_EVENT_MODE_CHANGE,
    CMXD_EVENT_ORIENTATION_CHANGE
} cmxd_event_type_t;

/* Event data structure */
struct cmxd_event {
    cmxd_event_type_t type;
    const char *value;
    const char *previous_value;
};

/* Event system configuration */
struct cmxd_events_config {
    int enable_unix_socket;
    int enable_dbus;
    char unix_socket_path[256];
    int verbose;
};

/* Log function pointer type for events module */
typedef void (*events_log_func_t)(const char *level, const char *fmt, ...);

/*
 * Event system functions
 */

/* Initialize the event system */
int cmxd_events_init(struct cmxd_events_config *config, events_log_func_t log_func);

/* Cleanup the event system */
void cmxd_events_cleanup(void);

/* Send events for mode and orientation changes */
int cmxd_send_events(cmxd_event_type_t type, const char *new_value, const char *old_value);

/* Enhanced write functions with state tracking and event sending */
int cmxd_write_mode_with_events(const char *mode);
int cmxd_write_orientation_with_events(const char *orientation);

#endif /* CMXD_EVENTS_H */