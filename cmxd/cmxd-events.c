/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Event System Implementation for CMXD
 * 
 * Handles event publishing for mode and orientation changes via Unix Domain
 * Sockets and DBus. Maintains state tracking to prevent redundant notifications.
 * Uses a dedicated pthread for Unix domain socket server operations.
 */

#define _POSIX_C_SOURCE 200809L
#define _GNU_SOURCE

#include "cmxd-events.h"
#include "cmxd-data.h"
#include "cmxd-paths.h"
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
#include <pthread.h>
#include <sys/select.h>
#include <signal.h>

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

/* Threading state */
static pthread_t socket_thread;
static pthread_mutex_t client_list_mutex = PTHREAD_MUTEX_INITIALIZER;
static volatile int socket_thread_running = 0;
static volatile int socket_thread_should_stop = 0;

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

/* Add a client to the client list (thread-safe) */
static int add_client(int client_fd)
{
    pthread_mutex_lock(&client_list_mutex);
    
    /* Grow client array if needed */
    if (client_count >= client_capacity) {
        int new_capacity = client_capacity == 0 ? 4 : client_capacity * 2;
        int *new_fds = realloc(client_fds, new_capacity * sizeof(int));
        if (!new_fds) {
            log_error("Failed to allocate memory for client list");
            pthread_mutex_unlock(&client_list_mutex);
            return -1;
        }
        client_fds = new_fds;
        client_capacity = new_capacity;
    }
    
    client_fds[client_count++] = client_fd;
    log_debug("Added client fd %d, total clients: %d", client_fd, client_count);
    
    pthread_mutex_unlock(&client_list_mutex);
    return 0;
}

/* Remove a client from the client list (thread-safe) */
static void remove_client(int client_fd)
{
    pthread_mutex_lock(&client_list_mutex);
    
    for (int i = 0; i < client_count; i++) {
        if (client_fds[i] == client_fd) {
            /* Move last client to this position */
            client_fds[i] = client_fds[client_count - 1];
            client_count--;
            log_debug("Removed client fd %d, remaining clients: %d", client_fd, client_count);
            break;
        }
    }
    
    pthread_mutex_unlock(&client_list_mutex);
    close(client_fd);
}

/* Socket server thread function */
static void *socket_server_thread(void *arg)
{
    fd_set read_fds, master_fds;
    int max_fd;
    struct timeval timeout;
    
    (void)arg; /* Unused parameter */
    
    log_info("Socket server thread started");
    socket_thread_running = 1;
    
    /* Initialize file descriptor sets */
    FD_ZERO(&master_fds);
    FD_SET(server_socket_fd, &master_fds);
    max_fd = server_socket_fd;
    
    while (!socket_thread_should_stop) {
        /* Copy master set for select() */
        read_fds = master_fds;
        
        /* Set timeout for select() to allow periodic checking of stop condition */
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        
        int activity = select(max_fd + 1, &read_fds, NULL, NULL, &timeout);
        
        if (activity < 0 && errno != EINTR) {
            if (!socket_thread_should_stop) {
                log_error("select() error in socket thread: %s", strerror(errno));
            }
            break;
        }
        
        if (activity == 0) {
            /* Timeout - continue to check stop condition */
            continue;
        }
        
        /* Check for new connections on server socket */
        if (FD_ISSET(server_socket_fd, &read_fds)) {
            struct sockaddr_un client_addr;
            socklen_t client_len = sizeof(client_addr);
            int client_fd;
            
            /* Accept new connection */
            client_fd = accept(server_socket_fd, (struct sockaddr*)&client_addr, &client_len);
            if (client_fd >= 0) {
                /* Set client socket to non-blocking */
                int flags = fcntl(client_fd, F_GETFL, 0);
                if (flags >= 0) {
                    fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);
                }
                
                if (add_client(client_fd) < 0) {
                    close(client_fd);
                } else {
                    log_info("New client connected (fd %d)", client_fd);
                    
                    /* Add to master set for monitoring */
                    FD_SET(client_fd, &master_fds);
                    if (client_fd > max_fd) {
                        max_fd = client_fd;
                    }
                }
            } else if (errno != EAGAIN && errno != EWOULDBLOCK) {
                if (!socket_thread_should_stop) {
                    log_error("Error accepting client connection: %s", strerror(errno));
                }
            }
        }
        
        /* Check for data on client sockets (detect disconnections) */
        pthread_mutex_lock(&client_list_mutex);
        for (int i = client_count - 1; i >= 0; i--) {
            int client_fd = client_fds[i];
            
            if (FD_ISSET(client_fd, &read_fds)) {
                /* Try to read to detect disconnection */
                char buffer[1];
                ssize_t result = recv(client_fd, buffer, sizeof(buffer), MSG_PEEK | MSG_DONTWAIT);
                
                if (result == 0) {
                    /* Client disconnected */
                    log_info("Client fd %d disconnected", client_fd);
                    FD_CLR(client_fd, &master_fds);
                    
                    /* Remove client (this will also close the fd) */
                    for (int j = 0; j < client_count; j++) {
                        if (client_fds[j] == client_fd) {
                            client_fds[j] = client_fds[client_count - 1];
                            client_count--;
                            break;
                        }
                    }
                    close(client_fd);
                    
                    /* Recalculate max_fd */
                    max_fd = server_socket_fd;
                    for (int j = 0; j < client_count; j++) {
                        if (client_fds[j] > max_fd) {
                            max_fd = client_fds[j];
                        }
                    }
                } else if (result < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
                    /* Error - treat as disconnection */
                    log_warn("Error on client fd %d: %s", client_fd, strerror(errno));
                    FD_CLR(client_fd, &master_fds);
                    
                    /* Remove client */
                    for (int j = 0; j < client_count; j++) {
                        if (client_fds[j] == client_fd) {
                            client_fds[j] = client_fds[client_count - 1];
                            client_count--;
                            break;
                        }
                    }
                    close(client_fd);
                }
            }
        }
        pthread_mutex_unlock(&client_list_mutex);
    }
    
    log_info("Socket server thread stopping");
    socket_thread_running = 0;
    return NULL;
}

/* Initialize Unix domain socket */
static int init_unix_socket(void)
{
    struct sockaddr_un addr;
    int flags;
    
    if (!events_config->enable_unix_socket) {
        log_debug("Unix domain socket disabled");
        return 0;
    }
    
    log_info("Initializing Unix domain socket server: %s", events_config->unix_socket_path);
    
    /* Create runtime directory if it doesn't exist */
    char runtime_dir[256];
    strncpy(runtime_dir, events_config->unix_socket_path, sizeof(runtime_dir) - 1);
    runtime_dir[sizeof(runtime_dir) - 1] = '\0';
    
    /* Find the last '/' to get directory path */
    char *last_slash = strrchr(runtime_dir, '/');
    if (last_slash) {
        *last_slash = '\0';
        
        /* Create directory with proper permissions */
        if (mkdir(runtime_dir, 0755) < 0 && errno != EEXIST) {
            log_error("Failed to create runtime directory %s: %s", runtime_dir, strerror(errno));
            return -1;
        }
        
        /* Ensure directory has correct permissions */
        if (chmod(runtime_dir, 0755) < 0) {
            log_warn("Failed to set permissions on %s: %s", runtime_dir, strerror(errno));
        }
        
        log_debug("Runtime directory ready: %s", runtime_dir);
    }
    
    /* Remove existing socket if it exists */
    if (unlink(events_config->unix_socket_path) < 0 && errno != ENOENT) {
        log_warn("Failed to remove existing socket %s: %s", 
                 events_config->unix_socket_path, strerror(errno));
    }
    
    /* Create server socket */
    server_socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_socket_fd < 0) {
        log_error("Failed to create Unix domain socket: %s", strerror(errno));
        return -1;
    }
    
    /* Set server socket to non-blocking */
    flags = fcntl(server_socket_fd, F_GETFL, 0);
    if (flags >= 0) {
        fcntl(server_socket_fd, F_SETFL, flags | O_NONBLOCK);
    }
    
    /* Set up socket address */
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, events_config->unix_socket_path, sizeof(addr.sun_path) - 1);
    
    /* Bind socket */
    if (bind(server_socket_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        log_error("Failed to bind Unix domain socket to %s: %s", 
                  events_config->unix_socket_path, strerror(errno));
        close(server_socket_fd);
        server_socket_fd = -1;
        return -1;
    }
    
    /* Listen for connections */
    if (listen(server_socket_fd, 10) < 0) {
        log_error("Failed to listen on Unix domain socket: %s", strerror(errno));
        close(server_socket_fd);
        server_socket_fd = -1;
        unlink(events_config->unix_socket_path);
        return -1;
    }
    
    /* Set socket permissions: readable/writable by everyone for connections */
    if (chmod(events_config->unix_socket_path, 0666) < 0) {
        log_warn("Failed to set socket permissions: %s", strerror(errno));
    }
    
    /* Start socket server thread */
    socket_thread_should_stop = 0;
    if (pthread_create(&socket_thread, NULL, socket_server_thread, NULL) != 0) {
        log_error("Failed to create socket server thread: %s", strerror(errno));
        close(server_socket_fd);
        server_socket_fd = -1;
        unlink(events_config->unix_socket_path);
        return -1;
    }
    
    /* Wait for thread to start */
    int timeout = 50; /* 5 second timeout */
    while (!socket_thread_running && timeout-- > 0) {
        usleep(100000); /* 100ms */
    }
    
    if (!socket_thread_running) {
        log_error("Socket server thread failed to start");
        socket_thread_should_stop = 1;
        pthread_join(socket_thread, NULL);
        close(server_socket_fd);
        server_socket_fd = -1;
        unlink(events_config->unix_socket_path);
        return -1;
    }
    
    log_info("Unix domain socket server listening: %s", events_config->unix_socket_path);
    return 0;
}

/* Initialize DBus connection (placeholder for now) */
static int init_dbus(void)
{
    if (!events_config->enable_dbus) {
        log_debug("DBus disabled");
        return 0;
    }
    
    log_debug("DBus initialization deferred for future implementation");
    return 0;
}

/* Send Unix domain socket event */
static int send_unix_socket_event(const struct cmxd_event *event)
{
    char message[512];
    struct timespec ts;
    int ret, sent_count = 0, failed_count = 0;
    
    if (!events_config->enable_unix_socket || server_socket_fd < 0) {
        return 0;
    }
    
    /* Get timestamp */
    if (clock_gettime(CLOCK_REALTIME, &ts) < 0) {
        log_warn("Failed to get timestamp: %s", strerror(errno));
        ts.tv_sec = 0;
        ts.tv_nsec = 0;
    }
    
    /* Format message as JSON for easy parsing by clients */
    ret = snprintf(message, sizeof(message),
                   "{"
                   "\"timestamp\":%ld.%09ld,"
                   "\"type\":\"%s\","
                   "\"value\":\"%s\""
                   "%s%s%s"
                   "}\n",
                   ts.tv_sec, ts.tv_nsec,
                   (event->type == CMXD_EVENT_MODE_CHANGE) ? "mode" : "orientation",
                   event->value,
                   event->previous_value ? ",\"previous\":\"" : "",
                   event->previous_value ? event->previous_value : "",
                   event->previous_value ? "\"" : "");
    
    if (ret >= (int)sizeof(message)) {
        log_warn("Unix socket message truncated");
    }
    
    /* Thread-safe client broadcasting */
    pthread_mutex_lock(&client_list_mutex);
    
    log_debug("Broadcasting Unix socket event to %d clients: %s", client_count, message);
    
    /* Send to all connected clients */
    for (int i = client_count - 1; i >= 0; i--) {
        ssize_t sent = send(client_fds[i], message, strlen(message), MSG_NOSIGNAL);
        if (sent < 0) {
            if (errno == EPIPE || errno == ECONNRESET || errno == ENOTCONN) {
                log_debug("Client fd %d disconnected", client_fds[i]);
                remove_client(client_fds[i]);
                /* Note: remove_client shifts array, so we continue with same index */
                failed_count++;
            } else if (errno != EAGAIN && errno != EWOULDBLOCK) {
                log_warn("Failed to send to client fd %d: %s", client_fds[i], strerror(errno));
                failed_count++;
            }
        } else if (sent == (ssize_t)strlen(message)) {
            sent_count++;
        } else {
            log_warn("Partial send to client fd %d: %zd/%zu bytes", 
                     client_fds[i], sent, strlen(message));
            failed_count++;
        }
    }
    
    if (sent_count > 0) {
        log_info("Event broadcast successful: %s changed to %s (sent to %d clients)", 
                 (event->type == CMXD_EVENT_MODE_CHANGE) ? "mode" : "orientation",
                 event->value, sent_count);
    }
    
    if (failed_count > 0) {
        log_debug("Event broadcast had %d failures", failed_count);
    }
    
    pthread_mutex_unlock(&client_list_mutex);
    
    return (sent_count > 0 || client_count == 0) ? 0 : -1;
}

/* Send DBus event (placeholder for now) */
static int send_dbus_event(const struct cmxd_event *event)
{
    if (!events_config->enable_dbus) {
        return 0;
    }
    
    log_debug("DBus event: type=%d, value='%s', previous='%s'", 
              event->type, event->value, event->previous_value ? event->previous_value : "none");
    
    /* TODO: Implement actual DBus sending */
    return 0;
}

/*
 * =============================================================================
 * PUBLIC API FUNCTIONS
 * =============================================================================
 */

/* Initialize the event system */
int cmxd_events_init(struct cmxd_events_config *config, events_log_func_t log_func)
{
    if (!config) {
        return -1;
    }
    
    events_config = config;
    log_function = log_func;
    
    log_info("Initializing event system - Unix socket: %s, DBus: %s",
             config->enable_unix_socket ? "enabled" : "disabled",
             config->enable_dbus ? "enabled" : "disabled");
    
    /* Initialize Unix domain socket */
    if (init_unix_socket() < 0) {
        log_error("Failed to initialize Unix domain socket");
        return -1;
    }
    
    /* Initialize DBus */
    if (init_dbus() < 0) {
        log_error("Failed to initialize DBus");
        return -1;
    }
    
    /* Clear current state */
    memset(current_mode, 0, sizeof(current_mode));
    memset(current_orientation, 0, sizeof(current_orientation));
    
    log_info("Event system initialized successfully");
    return 0;
}

/* Cleanup the event system */
void cmxd_events_cleanup(void)
{
    log_debug("Cleaning up event system");
    
    /* Stop socket server thread first */
    if (socket_thread_running) {
        log_debug("Stopping socket server thread");
        socket_thread_should_stop = 1;
        
        /* Give thread some time to stop gracefully */
        int timeout = 50; /* 5 seconds */
        while (socket_thread_running && timeout-- > 0) {
            usleep(100000); /* 100ms */
        }
        
        if (socket_thread_running) {
            log_warn("Socket thread did not stop gracefully, canceling");
            pthread_cancel(socket_thread);
        }
        
        pthread_join(socket_thread, NULL);
        log_debug("Socket server thread stopped");
        socket_thread_running = 0;
    }
    
    /* Send shutdown event to all connected clients */
    if (events_config && events_config->enable_unix_socket && server_socket_fd >= 0) {
        struct cmxd_event shutdown_event = {
            .type = CMXD_EVENT_MODE_CHANGE,  /* Use mode change type for shutdown */
            .value = "shutdown",
            .previous_value = NULL
        };
        
        /* Send shutdown notification to clients */
        send_unix_socket_event(&shutdown_event);
        
        /* Close all client connections */
        pthread_mutex_lock(&client_list_mutex);
        for (int i = 0; i < client_count; i++) {
            close(client_fds[i]);
        }
        free(client_fds);
        client_fds = NULL;
        client_count = 0;
        client_capacity = 0;
        pthread_mutex_unlock(&client_list_mutex);
        
        /* Close server socket */
        close(server_socket_fd);
        server_socket_fd = -1;
        
        /* Remove socket file */
        if (unlink(events_config->unix_socket_path) < 0 && errno != ENOENT) {
            log_warn("Failed to remove socket file %s: %s", 
                     events_config->unix_socket_path, strerror(errno));
        }
        
        log_debug("Unix domain socket cleaned up: %s", events_config->unix_socket_path);
    }
    
    /* TODO: Cleanup DBus connections */
    
    events_config = NULL;
    log_function = NULL;
}

/* Send events for mode and orientation changes */
int cmxd_send_events(cmxd_event_type_t type, const char *new_value, const char *old_value)
{
    struct cmxd_event event;
    int result = 0;
    
    if (!events_config) {
        return -1;
    }
    
    event.type = type;
    event.value = new_value;
    event.previous_value = old_value;
    
    log_info("Sending event: %s changed from '%s' to '%s'",
             (type == CMXD_EVENT_MODE_CHANGE) ? "mode" : "orientation",
             old_value ? old_value : "none", new_value);
    
    /* Send via Unix domain socket */
    if (send_unix_socket_event(&event) < 0) {
        log_warn("Failed to send Unix socket event");
        result = -1;
    }
    
    /* Send via DBus */
    if (send_dbus_event(&event) < 0) {
        log_warn("Failed to send DBus event");
        result = -1;
    }
    
    return result;
}

/* Enhanced write mode function with state tracking and event sending */
int cmxd_write_mode_with_events(const char *mode)
{
    char old_mode[sizeof(current_mode)];
    
    if (!mode) {
        log_error("Mode cannot be NULL");
        return -1;
    }
    
    /* Save old mode for comparison */
    strncpy(old_mode, current_mode, sizeof(old_mode) - 1);
    old_mode[sizeof(old_mode) - 1] = '\0';
    
    /* Check if mode has actually changed */
    if (strcmp(current_mode, mode) == 0) {
        log_debug("Mode unchanged (%s), skipping write and events", mode);
        return 0;
    }
    
    /* Write to kernel module */
    if (cmxd_write_mode(mode) < 0) {
        return -1;
    }
    
    /* Update current state */
    strncpy(current_mode, mode, sizeof(current_mode) - 1);
    current_mode[sizeof(current_mode) - 1] = '\0';
    
    /* Send events */
    if (cmxd_send_events(CMXD_EVENT_MODE_CHANGE, mode, old_mode[0] ? old_mode : NULL) < 0) {
        log_warn("Failed to send mode change events");
    }
    
    return 0;
}

/* Enhanced write orientation function with state tracking and event sending */
int cmxd_write_orientation_with_events(const char *orientation)
{
    char old_orientation[sizeof(current_orientation)];
    
    if (!orientation) {
        log_error("Orientation cannot be NULL");
        return -1;
    }
    
    /* Save old orientation for comparison */
    strncpy(old_orientation, current_orientation, sizeof(old_orientation) - 1);
    old_orientation[sizeof(old_orientation) - 1] = '\0';
    
    /* Check if orientation has actually changed */
    if (strcmp(current_orientation, orientation) == 0) {
        log_debug("Orientation unchanged (%s), skipping write and events", orientation);
        return 0;
    }
    
    /* Write to kernel module */
    if (cmxd_write_orientation(orientation) < 0) {
        return -1;
    }
    
    /* Update current state */
    strncpy(current_orientation, orientation, sizeof(current_orientation) - 1);
    current_orientation[sizeof(current_orientation) - 1] = '\0';
    
    /* Send events */
    if (cmxd_send_events(CMXD_EVENT_ORIENTATION_CHANGE, orientation, 
                        old_orientation[0] ? old_orientation : NULL) < 0) {
        log_warn("Failed to send orientation change events");
    }
    
    return 0;
}

/* Get current cached mode */
const char *cmxd_get_current_mode(void)
{
    return current_mode[0] ? current_mode : NULL;
}

/* Get current cached orientation */
const char *cmxd_get_current_orientation(void)
{
    return current_orientation[0] ? current_orientation : NULL;
}