/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Minimal Event System Implementation for Testing
 * 
 * Contains only the Unix domain socket functionality without
 * dependencies on cmxd-data.c
 */

#define _POSIX_C_SOURCE 200809L

#include "cmxd-events.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

/* Module state */
static struct cmxd_events_config *events_config = NULL;
static events_log_func_t log_function = NULL;

/* Current state tracking */
static char current_mode[32] = "";
static char current_orientation[32] = "";

/* Unix domain socket state */
static int server_socket_fd = -1;
static int *client_fds = NULL;
static int client_count = 0;
static int client_capacity = 0;

/* Logging macros using the configured log function */
#define log_error(fmt, ...) do { if (log_function) log_function("ERROR", fmt, ##__VA_ARGS__); } while(0)
#define log_warn(fmt, ...)  do { if (log_function) log_function("WARN", fmt, ##__VA_ARGS__); } while(0)
#define log_info(fmt, ...)  do { if (log_function) log_function("INFO", fmt, ##__VA_ARGS__); } while(0)
#define log_debug(fmt, ...) do { if (log_function) log_function("DEBUG", fmt, ##__VA_ARGS__); } while(0)

/*
 * =============================================================================
 * PRIVATE HELPER FUNCTIONS
 * =============================================================================
 */

/* Add a client to the client list */
static int add_client(int client_fd)
{
    /* Grow client array if needed */
    if (client_count >= client_capacity) {
        int new_capacity = client_capacity == 0 ? 4 : client_capacity * 2;
        int *new_fds = realloc(client_fds, new_capacity * sizeof(int));
        if (!new_fds) {
            log_error("Failed to allocate memory for client list");
            return -1;
        }
        client_fds = new_fds;
        client_capacity = new_capacity;
    }
    
    client_fds[client_count++] = client_fd;
    log_debug("Added client fd %d, total clients: %d", client_fd, client_count);
    return 0;
}

/* Remove a client from the client list */
static void remove_client(int client_fd)
{
    for (int i = 0; i < client_count; i++) {
        if (client_fds[i] == client_fd) {
            /* Move last client to this position */
            client_fds[i] = client_fds[client_count - 1];
            client_count--;
            log_debug("Removed client fd %d, remaining clients: %d", client_fd, client_count);
            break;
        }
    }
    close(client_fd);
}

/* Accept new client connections (non-blocking) */
static void accept_new_clients(void)
{
    struct sockaddr_un client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_fd;
    
    while ((client_fd = accept(server_socket_fd, (struct sockaddr*)&client_addr, &client_len)) >= 0) {
        /* Set client socket to non-blocking */
        int flags = fcntl(client_fd, F_GETFL, 0);
        if (flags >= 0) {
            fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);
        }
        
        if (add_client(client_fd) < 0) {
            close(client_fd);
        } else {
            log_info("New client connected (fd %d)", client_fd);
        }
    }
    
    /* EAGAIN/EWOULDBLOCK means no more pending connections */
    if (errno != EAGAIN && errno != EWOULDBLOCK) {
        log_error("Error accepting client connection: %s", strerror(errno));
    }
}

/* Format and send event to Unix domain socket clients */
static int send_unix_socket_event(const struct cmxd_event *event)
{
    if (!events_config || !events_config->enable_unix_socket) {
        return 0;
    }
    
    /* Accept any pending new clients first */
    if (server_socket_fd >= 0) {
        accept_new_clients();
    }
    
    if (client_count == 0) {
        log_debug("No clients connected, event not sent");
        return 0;
    }
    
    /* Format event as JSON */
    char json_buffer[1024];
    time_t now = time(NULL);
    
    const char *event_type_str = (event->type == CMXD_EVENT_MODE_CHANGE) ? "mode" : "orientation";
    
    int json_len = snprintf(json_buffer, sizeof(json_buffer),
        "{\"timestamp\":%ld,\"type\": \"%s\",\"value\":\"%s\"",
        now, event_type_str, event->value);
    
    if (event->previous_value) {
        json_len += snprintf(json_buffer + json_len, sizeof(json_buffer) - json_len,
            ",\"previous\":\"%s\"", event->previous_value);
    }
    
    json_len += snprintf(json_buffer + json_len, sizeof(json_buffer) - json_len, "}\n");
    
    if (json_len >= (int)sizeof(json_buffer)) {
        log_error("Event JSON too large for buffer");
        return -1;
    }
    
    /* Send to all connected clients */
    int sent_count = 0;
    for (int i = client_count - 1; i >= 0; i--) {
        ssize_t sent = send(client_fds[i], json_buffer, json_len, MSG_NOSIGNAL);
        if (sent < 0) {
            if (errno == EPIPE || errno == ECONNRESET) {
                log_info("Client fd %d disconnected", client_fds[i]);
                remove_client(client_fds[i]);
            } else {
                log_warn("Failed to send to client fd %d: %s", client_fds[i], strerror(errno));
            }
        } else if (sent != json_len) {
            log_warn("Partial send to client fd %d: %zd/%d bytes", client_fds[i], sent, json_len);
        } else {
            sent_count++;
        }
    }
    
    log_debug("Event sent to %d clients: %s", sent_count, json_buffer);
    return sent_count > 0 ? 0 : -1;
}

/* Initialize Unix domain socket */
static int init_unix_socket(void)
{
    struct sockaddr_un addr;
    int ret;
    
    /* Create socket */
    server_socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_socket_fd < 0) {
        log_error("Failed to create Unix domain socket: %s", strerror(errno));
        return -1;
    }
    
    /* Set socket to non-blocking */
    int flags = fcntl(server_socket_fd, F_GETFL, 0);
    if (flags >= 0) {
        fcntl(server_socket_fd, F_SETFL, flags | O_NONBLOCK);
    }
    
    /* Create runtime directory */
    char runtime_dir[256];
    strncpy(runtime_dir, events_config->unix_socket_path, sizeof(runtime_dir) - 1);
    runtime_dir[sizeof(runtime_dir) - 1] = '\0';
    
    /* Find last slash to get directory */
    char *last_slash = strrchr(runtime_dir, '/');
    if (last_slash) {
        *last_slash = '\0';
        
        if (mkdir(runtime_dir, 0755) != 0 && errno != EEXIST) {
            log_error("Failed to create runtime directory %s: %s", runtime_dir, strerror(errno));
            close(server_socket_fd);
            server_socket_fd = -1;
            return -1;
        }
    }
    
    /* Remove any existing socket */
    unlink(events_config->unix_socket_path);
    
    /* Bind socket */
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, events_config->unix_socket_path, sizeof(addr.sun_path) - 1);
    
    ret = bind(server_socket_fd, (struct sockaddr*)&addr, sizeof(addr));
    if (ret < 0) {
        log_error("Failed to bind Unix domain socket to %s: %s", 
                 events_config->unix_socket_path, strerror(errno));
        close(server_socket_fd);
        server_socket_fd = -1;
        return -1;
    }
    
    /* Set socket permissions */
    if (chmod(events_config->unix_socket_path, 0666) != 0) {
        log_warn("Failed to set socket permissions: %s", strerror(errno));
    }
    
    /* Listen for connections */
    ret = listen(server_socket_fd, 10);
    if (ret < 0) {
        log_error("Failed to listen on Unix domain socket: %s", strerror(errno));
        unlink(events_config->unix_socket_path);
        close(server_socket_fd);
        server_socket_fd = -1;
        return -1;
    }
    
    log_info("Unix domain socket listening on %s", events_config->unix_socket_path);
    return 0;
}

/* Cleanup Unix domain socket */
static void cleanup_unix_socket(void)
{
    /* Close all client connections */
    for (int i = 0; i < client_count; i++) {
        close(client_fds[i]);
    }
    free(client_fds);
    client_fds = NULL;
    client_count = 0;
    client_capacity = 0;
    
    /* Close server socket */
    if (server_socket_fd >= 0) {
        close(server_socket_fd);
        server_socket_fd = -1;
    }
    
    /* Remove socket file */
    if (events_config && events_config->unix_socket_path[0]) {
        unlink(events_config->unix_socket_path);
    }
}

/*
 * =============================================================================
 * PUBLIC API FUNCTIONS
 * =============================================================================
 */

int cmxd_events_init(struct cmxd_events_config *config, events_log_func_t log_func)
{
    if (!config) {
        return -1;
    }
    
    events_config = config;
    log_function = log_func;
    
    log_info("Initializing event system...");
    
    /* Initialize Unix domain socket if enabled */
    if (events_config->enable_unix_socket) {
        if (init_unix_socket() < 0) {
            log_error("Failed to initialize Unix domain socket");
            return -1;
        }
        log_info("Unix domain socket initialized");
    }
    
    /* DBus initialization would go here */
    if (events_config->enable_dbus) {
        log_info("DBus support not implemented yet");
    }
    
    log_info("Event system initialization complete");
    return 0;
}

void cmxd_events_cleanup(void)
{
    if (!events_config) {
        return;
    }
    
    log_info("Cleaning up event system...");
    
    /* Cleanup Unix domain socket */
    if (events_config->enable_unix_socket) {
        cleanup_unix_socket();
        log_info("Unix domain socket cleaned up");
    }
    
    /* DBus cleanup would go here */
    
    events_config = NULL;
    log_function = NULL;
    
    /* Clear state */
    current_mode[0] = '\0';
    current_orientation[0] = '\0';
}

int cmxd_send_events(cmxd_event_type_t type, const char *new_value, const char *old_value)
{
    if (!events_config) {
        return -1;
    }
    
    struct cmxd_event event = {
        .type = type,
        .value = new_value,
        .previous_value = old_value
    };
    
    int result = 0;
    
    /* Send via Unix domain socket */
    if (events_config->enable_unix_socket) {
        if (send_unix_socket_event(&event) < 0) {
            result = -1;
        }
    }
    
    /* Send via DBus */
    if (events_config->enable_dbus) {
        /* DBus implementation would go here */
        log_debug("DBus event sending not implemented");
    }
    
    return result;
}

/* Stub implementations for compatibility */
const char *cmxd_get_current_mode(void)
{
    return current_mode;
}

const char *cmxd_get_current_orientation(void)
{
    return current_orientation;
}

/* Minimal stub functions - not used in test */
int cmxd_write_mode_with_events(const char *mode)
{
    /* Just update internal state for testing */
    strncpy(current_mode, mode, sizeof(current_mode) - 1);
    current_mode[sizeof(current_mode) - 1] = '\0';
    return 0;
}

int cmxd_write_orientation_with_events(const char *orientation)
{
    /* Just update internal state for testing */
    strncpy(current_orientation, orientation, sizeof(current_orientation) - 1);
    current_orientation[sizeof(current_orientation) - 1] = '\0';
    return 0;
}