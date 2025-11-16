// SPDX-License-Identifier: GPL-2.0
/*
 * CHUWI Minibook X Session Daemon (cmxsd)
 * 
 * Monitors SW_TABLET_MODE input events and triggers desktop environment tablet mode 
 * behaviors including virtual keyboard, UI scaling, touch gestures, and window management.
 *
 * Copyright (c) 2025 Armando DiCianno <armando@noonshy.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <stdarg.h>
#include <time.h>
#include <dirent.h>
#include <poll.h>
#include <linux/input.h>
#include <libcmx/cmxd-protocol.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

/* Bit manipulation macros for input event handling */
#define NLONGS(x) (((x) + 8 * sizeof(long) - 1) / (8 * sizeof(long)))
#define test_bit(bit, array) ((array)[(bit) / (8 * sizeof(long))] & (1UL << ((bit) % (8 * sizeof(long)))))

/* VERSION and PROGRAM_NAME are provided by Makefile via -D flags */
#ifndef VERSION
#define VERSION "1.0.0"
#endif
#ifndef PROGRAM_NAME
#define PROGRAM_NAME "cmxsd"
#endif

#define DEFAULT_SOCKET_PATH "/run/cmxd/events.sock"

/* Configuration */
struct config {
    char tablet_device[PATH_MAX];
    char config_file[PATH_MAX];
    char on_tablet_script[PATH_MAX];
    char on_laptop_script[PATH_MAX];
    char on_rotate_script[PATH_MAX];
    char socket_path[PATH_MAX];
    int verbose;
    unsigned int debounce_ms;
};

/* Global state */
static volatile sig_atomic_t running = 1;
static struct config cfg = {
    .tablet_device = "",  /* Empty = auto-detect */
    .config_file = "",
    .on_tablet_script = "",
    .on_laptop_script = "",
    .on_rotate_script = "",
    .socket_path = DEFAULT_SOCKET_PATH,
    .verbose = 0,
    .debounce_ms = 500  /* 500ms debounce to avoid rapid switching */
};

/* Current state tracking */
static int last_tablet_state = -1;
static struct timespec last_event_time = {0, 0};

/* Forward declarations */
static int load_config(const char *config_file);
static int auto_detect_tablet_device(char *device_path, size_t path_size);

/* Signal handlers */
static void signal_handler(int sig)
{
    switch (sig) {
        case SIGTERM:
        case SIGINT:
            running = 0;
            break;
        case SIGHUP:
            /* Reload config in the future */
            break;
    }
}

/* Logging functions */
static void log_msg(const char *level, const char *fmt, ...)
{
    va_list args;
    
    if (!cfg.verbose && strcmp(level, "DEBUG") == 0)
        return;
        
    /* Always use standard stderr logging */
    fprintf(stderr, "%s[%d]: [%s] ", PROGRAM_NAME, getpid(), level);
    
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
    fflush(stderr);
}

#define log_error(fmt, ...) log_msg("ERROR", fmt, ##__VA_ARGS__)
#define log_warn(fmt, ...)  log_msg("WARN", fmt, ##__VA_ARGS__)
#define log_info(fmt, ...)  log_msg("INFO", fmt, ##__VA_ARGS__)
#define log_debug(fmt, ...) log_msg("DEBUG", fmt, ##__VA_ARGS__)

/* Execute script/command with proper error handling */
static int execute_command(const char *command, const char *action)
{
    int status;
    pid_t pid;
    
    if (!command || !*command) {
        log_debug("No %s command configured", action);
        return 0;
    }
    
    log_info("Executing %s command: %s", action, command);
    
    pid = fork();
    if (pid == 0) {
        /* Child process */
        execl("/bin/sh", "sh", "-c", command, NULL);
        log_error("Failed to execute command: %s", strerror(errno));
        exit(127);
    } else if (pid > 0) {
        /* Parent process */
        if (waitpid(pid, &status, 0) == -1) {
            log_error("Failed to wait for %s command: %s", action, strerror(errno));
            return -1;
        }
        
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            if (exit_code != 0) {
                log_warn("%s command exited with code %d", action, exit_code);
                return exit_code;
            }
            log_debug("%s command completed successfully", action);
            return 0;
        } else if (WIFSIGNALED(status)) {
            log_error("%s command terminated by signal %d", action, WTERMSIG(status));
            return -1;
        }
    } else {
        log_error("Failed to fork for %s command: %s", action, strerror(errno));
        return -1;
    }
    
    return 0;
}

/* Execute command with a parameter */
static int execute_command_with_param(const char *command, const char *action, const char *param)
{
    int status;
    pid_t pid;
    char full_command[PATH_MAX * 2];
    
    if (!command || !*command) {
        log_debug("No %s command configured", action);
        return 0;
    }
    
    /* Build full command with parameter */
    snprintf(full_command, sizeof(full_command), "%s %s", command, param);
    log_info("Executing %s command: %s", action, full_command);
    
    pid = fork();
    if (pid == 0) {
        /* Child process */
        execl("/bin/sh", "sh", "-c", full_command, NULL);
        log_error("Failed to execute command: %s", strerror(errno));
        exit(127);
    } else if (pid > 0) {
        /* Parent process */
        if (waitpid(pid, &status, 0) == -1) {
            log_error("Failed to wait for %s command: %s", action, strerror(errno));
            return -1;
        }
        
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            if (exit_code != 0) {
                log_warn("%s command exited with code %d", action, exit_code);
                return exit_code;
            }
            log_debug("%s command completed successfully", action);
            return 0;
        } else if (WIFSIGNALED(status)) {
            log_error("%s command terminated by signal %d", action, WTERMSIG(status));
            return -1;
        }
    } else {
        log_error("Failed to fork for %s command: %s", action, strerror(errno));
        return -1;
    }
    
    return 0;
}

/* Handle tablet mode state change */
static int handle_tablet_mode_change(int tablet_mode)
{
    struct timespec now;
    long long time_diff_ms;
    
    /* Get current time for debouncing */
    clock_gettime(CLOCK_MONOTONIC, &now);
    
    /* Check debouncing */
    if (last_event_time.tv_sec != 0) {
        time_diff_ms = (now.tv_sec - last_event_time.tv_sec) * 1000LL +
                       (now.tv_nsec - last_event_time.tv_nsec) / 1000000LL;
        
        if (time_diff_ms < cfg.debounce_ms) {
            log_debug("Ignoring rapid state change (debounce: %lld ms < %u ms)", 
                     time_diff_ms, cfg.debounce_ms);
            return 0;
        }
    }
    
    last_event_time = now;
    
    /* Skip if state hasn't actually changed */
    if (last_tablet_state == tablet_mode) {
        log_debug("State unchanged (%s mode), skipping", tablet_mode ? "tablet" : "laptop");
        return 0;
    }
    
    last_tablet_state = tablet_mode;
    
    log_info("Tablet mode changed: %s", tablet_mode ? "ENABLED" : "DISABLED");
    
    /* Execute custom scripts */
    if (tablet_mode) {
        return execute_command(cfg.on_tablet_script, "tablet");
    } else {
        return execute_command(cfg.on_laptop_script, "laptop");
    }
}

/* Check if device supports SW_TABLET_MODE events */
static int check_device_capabilities(int fd)
{
    unsigned long sw_bits[NLONGS(SW_CNT)];
    
    /* Get switch capabilities */
    if (ioctl(fd, EVIOCGBIT(EV_SW, SW_CNT), sw_bits) < 0) {
        log_debug("Device does not support switch events");
        return -1;
    }
    
    /* Check if SW_TABLET_MODE is supported */
    if (!test_bit(SW_TABLET_MODE, sw_bits)) {
        log_debug("Device does not support SW_TABLET_MODE");
        return -1;
    }
    
    return 0;
}

/* Get initial tablet mode state */
static int get_initial_state(int fd)
{
    unsigned long swstate[NLONGS(SW_MAX)] = {0};
    
    if (ioctl(fd, EVIOCGSW(sizeof(swstate)), swstate) < 0) {
        log_error("Failed to get initial switch state: %s", strerror(errno));
        return -1;
    }
    
    int tablet_mode = test_bit(SW_TABLET_MODE, swstate) ? 1 : 0;
    log_info("Initial tablet mode state: %s", tablet_mode ? "tablet" : "laptop");
    
    return tablet_mode;
}

/* Auto-detect tablet mode device by finding cmx platform device */
static int auto_detect_tablet_device(char *device_path, size_t path_size)
{
    DIR *cmx_input_dir;
    struct dirent *entry;
    char cmx_input_path[] = "/sys/devices/platform/cmx/input";
    char test_input_path[PATH_MAX];
    char dev_file_path[PATH_MAX];
    char dev_content[32];
    FILE *dev_file;
    int major, minor;
    int fd;
    int found = 0;
    
    log_debug("Auto-detecting tablet mode device via cmx sysfs...");
    log_debug("Checking cmx input directory: %s", cmx_input_path);
    
    /* First try to find the device via cmx sysfs */
    cmx_input_dir = opendir(cmx_input_path);
    if (cmx_input_dir) {
        log_debug("Found cmx input directory, scanning for input devices");
        
        while ((entry = readdir(cmx_input_dir)) != NULL) {
            /* Look for input* directories */
            if (strncmp(entry->d_name, "input", 5) != 0) {
                continue;
            }
            
            log_debug("Found input device: %s", entry->d_name);
            
            /* Look for event* subdirectories */
            snprintf(test_input_path, sizeof(test_input_path), 
                     "%s/%s", cmx_input_path, entry->d_name);
            
            DIR *event_dir = opendir(test_input_path);
            if (!event_dir) {
                log_debug("Cannot open input directory: %s", test_input_path);
                continue;
            }
            
            struct dirent *event_entry;
            while ((event_entry = readdir(event_dir)) != NULL) {
                if (strncmp(event_entry->d_name, "event", 5) != 0) {
                    continue;
                }
                
                /* Read the device major:minor from sysfs */
                int ret = snprintf(dev_file_path, sizeof(dev_file_path),
                                  "%s/%s/dev", test_input_path, event_entry->d_name);
                if (ret < 0 || (size_t)ret >= sizeof(dev_file_path)) {
                    log_debug("Path too long for device: %s", event_entry->d_name);
                    continue;
                }
                
                dev_file = fopen(dev_file_path, "r");
                if (!dev_file) {
                    log_debug("Cannot read device file: %s", dev_file_path);
                    continue;
                }
                
                if (fgets(dev_content, sizeof(dev_content), dev_file)) {
                    if (sscanf(dev_content, "%d:%d", &major, &minor) == 2) {
                        snprintf(device_path, path_size, "/dev/input/%s", event_entry->d_name);
                        log_debug("Found cmx device: %s (major=%d, minor=%d)", 
                                 device_path, major, minor);
                        
                        /* Verify the device exists and has correct capabilities */
                        fd = open(device_path, O_RDONLY);
                        if (fd >= 0) {
                            if (check_device_capabilities(fd) == 0) {
                                log_info("Found tablet mode device via cmx sysfs: %s", device_path);
                                found = 1;
                                close(fd);
                                fclose(dev_file);
                                closedir(event_dir);
                                closedir(cmx_input_dir);
                                return 0;
                            }
                            close(fd);
                        }
                    }
                }
                fclose(dev_file);
            }
            closedir(event_dir);
        }
        closedir(cmx_input_dir);
    } else {
        log_debug("Cannot access cmx input directory: %s (%s)", cmx_input_path, strerror(errno));
    }
    
    /* Fallback: scan /dev/input if cmx sysfs method failed */
    if (!found) {
        log_debug("cmx sysfs detection failed, falling back to /dev/input scan");
        DIR *input_dir = opendir("/dev/input");
        if (!input_dir) {
            log_error("Failed to open /dev/input: %s", strerror(errno));
            return -1;
        }
        
        log_debug("Scanning /dev/input for event devices");
        while ((entry = readdir(input_dir)) != NULL) {
            /* Only check event devices */
            if (strncmp(entry->d_name, "event", 5) != 0) {
                continue;
            }
            
            snprintf(device_path, path_size, "/dev/input/%s", entry->d_name);
            log_debug("Testing device: %s", device_path);
            
            /* Try to open the device */
            fd = open(device_path, O_RDONLY);
            if (fd < 0) {
                log_debug("Cannot open %s: %s", device_path, strerror(errno));
                continue;
            }
            
            /* Check if device supports SW_TABLET_MODE */
            if (check_device_capabilities(fd) == 0) {
                log_info("Found tablet mode device via fallback scan: %s", device_path);
                found = 1;
                close(fd);
                break;
            }
            
            close(fd);
        }
        
        closedir(input_dir);
    }
    
    if (!found) {
        log_error("No tablet mode device found");
        log_info("Check if cmx kernel module is loaded and functioning");
        return -1;
    }
    
    log_debug("Auto-detection completed successfully");
    return 0;
}

/* Connect to cmxd Unix domain socket */
static int connect_to_cmxd_socket(void)
{
    int sock_fd;
    struct sockaddr_un addr;
    
    log_debug("Attempting to connect to cmxd socket: %s", cfg.socket_path);
    
    sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        log_error("Failed to create socket: %s", strerror(errno));
        return -1;
    }
    
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    
    /* Ensure socket path fits in sun_path */
    size_t path_len = strlen(cfg.socket_path);
    if (path_len >= sizeof(addr.sun_path)) {
        log_error("Socket path too long: %s (max: %zu)", cfg.socket_path, sizeof(addr.sun_path) - 1);
        close(sock_fd);
        return -1;
    }
    memcpy(addr.sun_path, cfg.socket_path, path_len + 1);
    
    if (connect(sock_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        log_error("Failed to connect to cmxd socket %s: %s", cfg.socket_path, strerror(errno));
        close(sock_fd);
        return -1;
    }
    
    log_info("Connected to cmxd socket: %s", cfg.socket_path);
    return sock_fd;
}

/* Handle cmxd socket events */
static int handle_socket_event(int sock_fd)
{
    char buffer[CMXD_PROTOCOL_MAX_MESSAGE_SIZE];
    struct cmxd_protocol_message parsed;
    ssize_t bytes_read;
    
    bytes_read = read(sock_fd, buffer, sizeof(buffer) - 1);
    if (bytes_read <= 0) {
        if (bytes_read == 0) {
            log_info("cmxd socket closed");
        } else {
            log_error("Error reading from cmxd socket: %s", strerror(errno));
        }
        return -1;
    }
    
    buffer[bytes_read] = '\0';
    log_debug("Received from cmxd: %s", buffer);
    
    /* Parse the protocol message */
    if (cmxd_protocol_parse_message(buffer, &parsed) < 0) {
        log_warn("Failed to parse cmxd message: %s", buffer);
        return 0;  /* Continue, don't disconnect */
    }
    
    log_debug("Parsed message - type: %s, value: %s, previous: %s", 
             parsed.type, parsed.value, parsed.has_previous ? parsed.previous : "none");
    
    /* Handle mode change events */
    if (strcmp(parsed.type, CMXD_PROTOCOL_EVENT_MODE) == 0) {
        log_info("cmxd reports mode change: %s", parsed.value);
        
        /* Only trigger SW_TABLET_MODE for actual tablet mode */
        int is_tablet = cmxd_protocol_is_tablet_mode(parsed.value);
        handle_tablet_mode_change(is_tablet);
        
        /* Handle specific mode actions based on socket events */
        if (strcmp(parsed.value, CMXD_PROTOCOL_MODE_LAPTOP) == 0) {
            log_debug("Mode: laptop - normal laptop usage");
        } else if (strcmp(parsed.value, CMXD_PROTOCOL_MODE_FLAT) == 0) {
            log_debug("Mode: flat - device is flat/horizontal");
        } else if (strcmp(parsed.value, CMXD_PROTOCOL_MODE_TENT) == 0) {
            log_debug("Mode: tent - device in tent configuration");
        } else if (strcmp(parsed.value, CMXD_PROTOCOL_MODE_TABLET) == 0) {
            log_debug("Mode: tablet - device fully folded for tablet use");
        }
    }
    
    /* Handle orientation change events */
    if (strcmp(parsed.type, CMXD_PROTOCOL_EVENT_ORIENTATION) == 0) {
        log_info("cmxd reports orientation change: %s", parsed.value);
        /* Execute rotation script with orientation parameter */
        return execute_command_with_param(cfg.on_rotate_script, "rotate", parsed.value);
    }
    
    return 0;
}

/* Main event monitoring loop */
static int monitor_tablet_events(void)
{
    int fd, sock_fd = -1;
    struct input_event ev;
    ssize_t n;
    char final_device[PATH_MAX];
    struct pollfd poll_fds[2];
    int poll_count;
    
    log_debug("Starting monitor_tablet_events function");
    
    /* Auto-detect device if not specified */
    if (!cfg.tablet_device[0]) {
        log_debug("No device specified, starting auto-detection");
        if (auto_detect_tablet_device(final_device, sizeof(final_device)) < 0) {
            log_error("Auto-detection failed, exiting");
            return -1;
        }
        log_debug("Auto-detection succeeded: %s", final_device);
    } else {
        log_debug("Using configured device: %s", cfg.tablet_device);
        strncpy(final_device, cfg.tablet_device, sizeof(final_device) - 1);
        final_device[sizeof(final_device) - 1] = '\0';
    }
    
    log_debug("Opening device: %s", final_device);
    /* Open tablet mode device */
    fd = open(final_device, O_RDONLY);
    if (fd < 0) {
        log_error("Failed to open device %s: %s", final_device, strerror(errno));
        return -1;
    }
    log_debug("Device opened successfully, fd=%d", fd);
    
    /* Check device capabilities */
    log_debug("Checking device capabilities");
    if (check_device_capabilities(fd) < 0) {
        log_error("Device capability check failed");
        close(fd);
        return -1;
    }
    log_debug("Device capabilities verified");
    
    /* Try to connect to cmxd socket */
    sock_fd = connect_to_cmxd_socket();
    if (sock_fd < 0) {
        log_error("Cannot operate without cmxd socket connection");
        close(fd);
        return -1;
    }
    
    /* Setup poll file descriptors */
    poll_fds[0].fd = fd;
    poll_fds[0].events = POLLIN;
    poll_count = 1;
    
    poll_fds[1].fd = sock_fd;
    poll_fds[1].events = POLLIN;
    poll_count = 2;
    
    /* Get and handle initial state */
    log_debug("Getting initial tablet mode state");
    int initial_state = get_initial_state(fd);
    if (initial_state >= 0) {
        /* Set the initial state but don't execute scripts on startup */
        last_tablet_state = initial_state;
        log_info("Initial state set to %s mode (no script execution)", 
                 initial_state ? "tablet" : "laptop");
    }
    
    log_info("Monitoring tablet mode events on %s", final_device);
    if (sock_fd >= 0) {
        log_info("Also monitoring cmxd events from %s", cfg.socket_path);
    }
    log_debug("Entering main event loop");
    
    /* Event monitoring loop */
    while (running) {
        int poll_result = poll(poll_fds, poll_count, -1);
        
        if (poll_result < 0) {
            if (errno == EINTR) {
                /* Interrupted by signal - check if we should exit */
                log_debug("Poll interrupted by signal, running=%d", running);
                if (!running) {
                    log_debug("Received signal, exiting gracefully");
                    break;
                }
                continue;
            }
            log_error("Poll error: %s", strerror(errno));
            break;
        }
        
        if (poll_result == 0) {
            /* Timeout (shouldn't happen with -1 timeout) */
            continue;
        }
        
        /* Check input device events */
        if (poll_fds[0].revents & POLLIN) {
            n = read(fd, &ev, sizeof(ev));
            
            if (n < 0) {
                if (errno == EINTR) {
                    continue;
                }
                log_error("Error reading from device: %s", strerror(errno));
                break;
            }
            
            if (n != sizeof(ev)) {
                log_warn("Incomplete event read: %zd bytes", n);
                continue;
            }
            
            /* Process tablet mode switch events */
            if (ev.type == EV_SW && ev.code == SW_TABLET_MODE) {
                log_debug("SW_TABLET_MODE event: value=%d", ev.value);
                handle_tablet_mode_change(ev.value);
            }
        }
        
        /* Check cmxd socket events */
        if (poll_count > 1 && (poll_fds[1].revents & POLLIN)) {
            if (handle_socket_event(sock_fd) < 0) {
                log_error("cmxd socket disconnected, terminating");
                close(sock_fd);
                sock_fd = -1;
                break;
            }
        }
        
        /* Handle errors on input device */
        if (poll_fds[0].revents & (POLLERR | POLLHUP | POLLNVAL)) {
            log_error("Poll error on input device");
            break;
        }
        
        /* Handle errors on socket */
        if (poll_count > 1 && (poll_fds[1].revents & (POLLERR | POLLHUP | POLLNVAL))) {
            log_error("Lost connection to cmxd socket, terminating");
            close(sock_fd);
            sock_fd = -1;
            break;
        }
    }
    
    log_debug("Exiting main event loop");
    close(fd);
    if (sock_fd >= 0) {
        close(sock_fd);
    }
    return 0;
}

/* Load configuration with precedence: command line -> user config -> system config */
static int load_config_with_precedence(const char *config_file)
{
    char user_config[PATH_MAX];
    char system_config[] = "/etc/cmxsd/daemon.conf";
    const char *home;
    int loaded = 0;
    
    /* If specific config file provided via command line, use it exclusively */
    if (config_file && *config_file) {
        log_debug("Using command-line specified config: %s", config_file);
        return load_config(config_file);
    }
    
    /* Try user config first: ~/.config/cmxsd/daemon.conf */
    home = getenv("HOME");
    if (home) {
        snprintf(user_config, sizeof(user_config), "%s/.config/cmxsd/daemon.conf", home);
        if (access(user_config, R_OK) == 0) {
            log_debug("Loading user config: %s", user_config);
            if (load_config(user_config) == 0) {
                loaded = 1;
            }
        }
    }
    
    /* If no user config found, try system config: /etc/cmxsd/daemon.conf */
    if (!loaded && access(system_config, R_OK) == 0) {
        log_debug("Loading system config: %s", system_config);
        if (load_config(system_config) == 0) {
            loaded = 1;
        }
    }
    
    if (!loaded) {
        log_info("No configuration file found, using defaults");
        log_debug("Checked: %s, %s", user_config, system_config);
    }
    
    return 0;
}

/* Load configuration from file */
static int load_config(const char *config_file)
{
    FILE *fp;
    char line[512];
    char key[256], value[256];
    
    if (!config_file || !*config_file) {
        return 0;  /* No config file specified */
    }
    
    fp = fopen(config_file, "r");
    if (!fp) {
        if (errno != ENOENT) {
            log_error("Failed to open config file %s: %s", config_file, strerror(errno));
            return -1;
        }
        return 0;  /* Config file doesn't exist, use defaults */
    }
    
    log_info("Loading configuration from %s", config_file);
    
    while (fgets(line, sizeof(line), fp)) {
        /* Skip comments and empty lines */
        char *stripped = line;
        while (*stripped == ' ' || *stripped == '\t') stripped++;
        if (*stripped == '#' || *stripped == '\n' || *stripped == '\0') {
            continue;
        }
        
        /* Parse key=value pairs */
        if (sscanf(line, "%255[^=]=%255s", key, value) == 2) {
            /* Trim whitespace */
            char *k = key, *v = value;
            while (*k == ' ' || *k == '\t') k++;
            while (*v == ' ' || *v == '\t') v++;
            
            /* Process configuration options */
            if (strcmp(k, "tablet_device") == 0) {
                strncpy(cfg.tablet_device, v, sizeof(cfg.tablet_device) - 1);
            } else if (strcmp(k, "socket_path") == 0) {
                strncpy(cfg.socket_path, v, sizeof(cfg.socket_path) - 1);
            } else if (strcmp(k, "on_tablet_script") == 0) {
                strncpy(cfg.on_tablet_script, v, sizeof(cfg.on_tablet_script) - 1);
            } else if (strcmp(k, "on_laptop_script") == 0) {
                strncpy(cfg.on_laptop_script, v, sizeof(cfg.on_laptop_script) - 1);
            } else if (strcmp(k, "on_rotate_script") == 0) {
                strncpy(cfg.on_rotate_script, v, sizeof(cfg.on_rotate_script) - 1);
            } else if (strcmp(k, "debounce_ms") == 0) {
                cfg.debounce_ms = atoi(v);
            } else if (strcmp(k, "verbose") == 0) {
                cfg.verbose = atoi(v);
            } else {
                log_warn("Unknown config option: %s", k);
            }
        }
    }
    
    fclose(fp);
    return 0;
}

/* Setup signal handlers */
static int setup_signals(void)
{
    struct sigaction sa;
    
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    /* Don't use SA_RESTART - we want read() to be interrupted by signals */
    sa.sa_flags = 0;
    
    if (sigaction(SIGTERM, &sa, NULL) < 0 ||
        sigaction(SIGINT, &sa, NULL) < 0 ||
        sigaction(SIGHUP, &sa, NULL) < 0) {
        log_error("Failed to setup signal handlers: %s", strerror(errno));
        return -1;
    }
    
    return 0;
}

/* Usage help */
static void usage(const char *program)
{
    printf("Usage: %s [OPTIONS]\n", program);
    printf("\nHyprland Tablet Mode Integration Daemon\n");
    printf("Monitors SW_TABLET_MODE events and triggers Hyprland tablet mode behaviors.\n\n");
    printf("OPTIONS:\n");
    printf("  -d, --device DEVICE     Tablet mode input device (default: auto-detect)\n");
    printf("  -c, --config FILE       Configuration file path (overrides auto-detection)\n");
    printf("  -s, --socket PATH       cmxd socket path (default: %s)\n", DEFAULT_SOCKET_PATH);
    printf("  -t, --on-tablet CMD     Command to run when entering tablet mode\n");
    printf("  -l, --on-laptop CMD     Command to run when entering laptop mode\n");
    printf("  -b, --debounce MS       Debounce time in milliseconds (default: %u)\n", cfg.debounce_ms);
    printf("  -v, --verbose           Enable verbose logging\n");
    printf("  -h, --help              Show this help\n");
    printf("  -V, --version           Show version\n");
    printf("\nCONFIGURATION FILES:\n");
    printf("  Configuration files are checked in this order:\n");
    printf("  1. Command line specified file (-c option)\n");
    printf("  2. User config: ~/.config/cmxsd/daemon.conf\n");
    printf("  3. Default example scripts (if no config found)\n");
    printf("\nEXAMPLES:\n");
    printf("  %s                                           # Use auto-detected device\n", program);
    printf("  %s -v                                        # Verbose logging\n", program);
    printf("  %s -c ~/.config/cmxsd/daemon.conf            # Specific config\n", program);
    printf("  %s -t 'onboard' -l 'pkill onboard'          # Virtual keyboard\n", program);
    printf("\nSee cmxsd(8) for more information.\n");
}

/* Main function */
int main(int argc, char *argv[])
{
    int opt;
    const char *short_opts = "d:c:s:t:l:b:vhV";
    static struct option long_opts[] = {
        {"device",     required_argument, 0, 'd'},
        {"config",     required_argument, 0, 'c'},
        {"socket",     required_argument, 0, 's'},
        {"on-tablet",  required_argument, 0, 't'},
        {"on-laptop",  required_argument, 0, 'l'},
        {"debounce",   required_argument, 0, 'b'},
        {"verbose",    no_argument,       0, 'v'},
        {"help",       no_argument,       0, 'h'},
        {"version",    no_argument,       0, 'V'},
        {0, 0, 0, 0}
    };
    
    /* Parse command line arguments */
    while ((opt = getopt_long(argc, argv, short_opts, long_opts, NULL)) != -1) {
        switch (opt) {
            case 'd':
                strncpy(cfg.tablet_device, optarg, sizeof(cfg.tablet_device) - 1);
                break;
            case 'c':
                strncpy(cfg.config_file, optarg, sizeof(cfg.config_file) - 1);
                break;
            case 's':
                strncpy(cfg.socket_path, optarg, sizeof(cfg.socket_path) - 1);
                break;
            case 't':
                strncpy(cfg.on_tablet_script, optarg, sizeof(cfg.on_tablet_script) - 1);
                break;
            case 'l':
                strncpy(cfg.on_laptop_script, optarg, sizeof(cfg.on_laptop_script) - 1);
                break;
            case 'b':
                cfg.debounce_ms = atoi(optarg);
                break;
            case 'v':
                cfg.verbose = 1;
                break;
            case 'h':
                usage(argv[0]);
                exit(0);
            case 'V':
                printf("%s %s\n", PROGRAM_NAME, VERSION);
                exit(0);
            default:
                usage(argv[0]);
                exit(1);
        }
    }
    
    /* Load configuration file */
    log_debug("Loading configuration file...");
    if (load_config_with_precedence(cfg.config_file) < 0) {
        log_error("Failed to load configuration file");
        exit(1);
    }
    log_debug("Configuration loaded successfully");
    
    /* Setup signal handlers */
    log_debug("Setting up signal handlers...");
    if (setup_signals() < 0) {
        log_error("Failed to setup signal handlers");
        exit(1);
    }
    log_debug("Signal handlers setup successfully");
    
    log_info("Starting %s %s", PROGRAM_NAME, VERSION);
    log_debug("Process PID: %d", getpid());
    log_debug("User ID: %d, Group ID: %d", getuid(), getgid());
    log_debug("Verbose logging: %s", cfg.verbose ? "enabled" : "disabled");
    
    if (cfg.tablet_device[0]) {
        log_info("Device: %s (configured), Debounce: %ums", cfg.tablet_device, cfg.debounce_ms);
    } else {
        log_info("Device: auto-detect, Debounce: %ums", cfg.debounce_ms);
    }
    
    log_debug("Configuration summary:");
    log_debug("  Config file: %s", cfg.config_file[0] ? cfg.config_file : "(none specified)");
    log_debug("  On tablet script: %s", cfg.on_tablet_script[0] ? cfg.on_tablet_script : "(none)");
    log_debug("  On laptop script: %s", cfg.on_laptop_script[0] ? cfg.on_laptop_script : "(none)");
    
    log_debug("Starting main monitoring loop");
    
    /* Main monitoring loop */
    int ret = monitor_tablet_events();
    
    log_info("Exiting with code %d", ret == 0 ? 0 : 1);
    return ret == 0 ? 0 : 1;
}