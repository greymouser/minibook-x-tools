// SPDX-License-Identifier: GPL-2.0
/* 
 * Chuwi Minibook X Daemon - Main Application
 * 
 * Primary daemon executable that coordinates accelerometer data collection,
 * mode detection, and kernel module communication. Handles signal management,
 * configuration, and the main event loop for continuous device monitoring.
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
#include <getopt.h>
#include <limits.h>
#include <math.h>
#include <stdarg.h>
#include <time.h>
#include <poll.h>
#include <fcntl.h>
#include <endian.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>

#define _POSIX_C_SOURCE 200809L
#include <sys/wait.h>

#include "cmxd-calculations.h"
#include "cmxd-orientation.h"
#include "cmxd-modes.h"
#include "cmxd-data.h"
#include "cmxd-events.h"
#include "cmxd-paths.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define PROGRAM_NAME "cmxd"
/* VERSION is defined by Makefile from VERSION file */

#define DEVICE_NAME_MAX 128

/*
 * =============================================================================
 * CONFIGURATION AND GLOBAL STATE
 * =============================================================================
 */

/* Configuration */
struct config {
    char base_dev[64];
    char lid_dev[64];
    char sysfs_path[PATH_MAX];
    unsigned int buffer_timeout_ms;
    int verbose;
    int daemon_mode;
    /* Event system configuration */
    int enable_unix_socket;
    int enable_dbus;
    char unix_socket_path[256];
};

/* Global state */
static volatile sig_atomic_t running = 1;

static struct config cfg = {
    .base_dev = "iio:device0",
    .lid_dev = "iio:device1", 
    .buffer_timeout_ms = 100,
    .daemon_mode = 0,
    .verbose = 0,
    .sysfs_path = CMXD_DEFAULT_SYSFS_PATH,
    .enable_unix_socket = 1,  /* Enable Unix domain socket by default */
    .enable_dbus = 1,         /* Enable DBus by default */
    .unix_socket_path = CMXD_SOCKET_PATH
};

/*
 * =============================================================================
 * SIGNAL HANDLING AND CLEANUP
 * =============================================================================
 */

/* Forward declaration for cleanup */
static void cleanup_and_exit(void);

static void signal_handler(int sig)
{
    switch (sig) {
        case SIGTERM:
        case SIGINT:
            /* Use printf since log macros aren't available yet */
            printf("[INFO] Received signal %d, shutting down...\n", sig);
            running = 0;
            cleanup_and_exit();
            break;
        case SIGHUP:
            /* Reload config in the future */
            break;
    }
}

/*
 * =============================================================================
 * LOGGING SYSTEM
 * =============================================================================
 */

/* Logging functions */
static void log_msg(const char *level, const char *fmt, ...)
{
    va_list args;
    struct timeval tv;
    char timestr[64];
    
    if (!cfg.verbose && strcmp(level, "DEBUG") == 0)
        return;
        
    gettimeofday(&tv, NULL);
    strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S", 
             localtime(&tv.tv_sec));
    
    fprintf(stderr, "[%s.%03ld] %s: ", timestr, tv.tv_usec / 1000, level);
    
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    
    fprintf(stderr, "\n");
}

#define log_error(fmt, ...) log_msg("ERROR", fmt, ##__VA_ARGS__)
#define log_warn(fmt, ...)  log_msg("WARN", fmt, ##__VA_ARGS__)
#define log_info(fmt, ...)  log_msg("INFO", fmt, ##__VA_ARGS__)
#define log_debug(fmt, ...) log_msg("DEBUG", fmt, ##__VA_ARGS__)

/* Wrapper function for orientation module logging */
static void orientation_log_debug(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char buffer[512];
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    log_debug("%s", buffer);
}

/*
 * =============================================================================
 * APPLICATION LIFECYCLE
 * =============================================================================
 */

/* Cleanup function to restore laptop mode on exit */
/* This prevents getting locked out in tablet mode when daemon exits */
static void cleanup_and_exit(void)
{
    static int cleanup_done = 0;
    
    /* Prevent multiple calls */
    if (cleanup_done) return;
    cleanup_done = 1;
    
    log_info("Performing cleanup: forcing laptop mode to prevent lockout");
    
    /* Force laptop mode as failsafe */
    if (cmxd_write_mode("laptop") < 0) {
        log_warn("Failed to restore laptop mode during cleanup");
    }
    
    /* Force landscape orientation as safe default */
    if (cmxd_write_orientation("landscape") < 0) {
        log_warn("Failed to restore landscape orientation during cleanup");
    }
    
    /* Cleanup event system */
    cmxd_events_cleanup();
    
    log_info("Cleanup complete - laptop mode restored");
}

/*
 * =============================================================================
 * MAIN PROCESSING LOOP
 * =============================================================================
 */

/* Trigger sysfs trigger to generate IIO buffer samples */
static int trigger_iio_sampling(void) {
    FILE *fp;
    char path[PATH_MAX];
    int trigger_id;
    
    /* Find the first available trigger */
    for (trigger_id = 0; trigger_id < 10; trigger_id++) {
        snprintf(path, sizeof(path), IIO_TRIGGER_NOW_TEMPLATE, trigger_id);
        fp = fopen(path, "w");
        if (fp) {
            if (fprintf(fp, "1") >= 0) {
                fclose(fp);
                return 0;
            }
            fclose(fp);
        }
    }
    
    log_error("No trigger available for sampling");
    return -1;
}

/* Main processing loop with event-driven IIO reading */
static int run_main_loop(void)
{
    struct iio_buffer base_buf, lid_buf;
    struct accel_sample base_sample, lid_sample;
    struct pollfd poll_fds[2];
    int base_xs, base_ys, base_zs;
    int lid_xs, lid_ys, lid_zs;
    unsigned int error_count = 0;
    const unsigned int max_errors = 10;
    int poll_timeout = cfg.buffer_timeout_ms; /* Use configured buffer timeout for poll() */
    int base_valid = 0, lid_valid = 0;
    double base_scale, lid_scale;
    
    /* Ensure IIO trigger exists (create if needed, but leave persistent) */
    log_debug("Ensuring IIO trigger is available...");
    if (cmxd_ensure_iio_trigger_exists() < 0) {
        log_error("Failed to ensure IIO trigger exists");
        return -1;
    }
    
    log_debug("Setting up IIO buffers for event-driven reading...");
    
    /* Setup IIO buffers */
    if (cmxd_setup_iio_buffer(&base_buf, cfg.base_dev) < 0) {
        log_error("Failed to setup IIO buffer for base device %s", cfg.base_dev);
        return -1;
    }
    
    if (cmxd_setup_iio_buffer(&lid_buf, cfg.lid_dev) < 0) {
        log_error("Failed to setup IIO buffer for lid device %s", cfg.lid_dev);
        cmxd_cleanup_iio_buffer(&base_buf);
        return -1;
    }
    
    /* Read scale factors for both devices */
    base_scale = cmxd_read_accel_scale(cfg.base_dev);
    lid_scale = cmxd_read_accel_scale(cfg.lid_dev);
    
    if (base_scale <= 0.0) {
        log_warn("Invalid base scale %f, using default 0.009582", base_scale);
        base_scale = 0.009582;
    }
    
    if (lid_scale <= 0.0) {
        log_warn("Invalid lid scale %f, using default 0.009582", lid_scale);
        lid_scale = 0.009582;
    }
    
    log_info("Using scales: base=%f, lid=%f", base_scale, lid_scale);
    
    /* Setup poll file descriptors */
    poll_fds[0].fd = base_buf.buffer_fd;
    poll_fds[0].events = POLLIN;
    poll_fds[1].fd = lid_buf.buffer_fd;
    poll_fds[1].events = POLLIN;
    
    log_debug("Starting event-driven main loop...");
    
    while (running) {
        int poll_result = poll(poll_fds, 2, poll_timeout);
        
        if (poll_result < 0) {
            if (errno == EINTR) {
                continue; /* Interrupted by signal */
            }
            log_error("Poll error: %s", strerror(errno));
            break;
        }
        
        if (poll_result == 0) {
            /* Timeout - trigger new samples and continue */
            trigger_iio_sampling();
            continue;
        }
        
        /* Check for base sensor data */
        if (poll_fds[0].revents & POLLIN) {
            int result = cmxd_read_iio_buffer_sample(&base_buf, &base_sample);
            if (result < 0) {
                error_count++;
                if (error_count >= max_errors) {
                    log_error("Too many consecutive base read errors (%u), exiting", error_count);
                    break;
                }
                log_warn("Base read error %u/%u", error_count, max_errors);
                continue;
            } else if (result > 0) {
                /* Apply actual scaling factor */
                cmxd_apply_scale(base_sample.x, base_sample.y, base_sample.z, base_scale, 
                           &base_xs, &base_ys, &base_zs);
                
                log_debug("Base: X=%d, Y=%d, Z=%d", base_sample.x, base_sample.y, base_sample.z);
                
                /* Write to kernel module */
                if (cmxd_write_vector("base", base_xs, base_ys, base_zs) < 0) {
                    log_error("Failed to write base vector to kernel module");
                    break;
                }
                
                base_valid = 1;
                
                /* Reset error count on successful read */
                error_count = 0;
            }
        }
        
        /* Check for lid sensor data */
        if (poll_fds[1].revents & POLLIN) {
            int result = cmxd_read_iio_buffer_sample(&lid_buf, &lid_sample);
            if (result < 0) {
                error_count++;
                if (error_count >= max_errors) {
                    log_error("Too many consecutive lid read errors (%u), exiting", error_count);
                    break;
                }
                log_warn("Lid read error %u/%u", error_count, max_errors);
                continue;
            } else if (result > 0) {
                /* Apply actual scaling factor */
                cmxd_apply_scale(lid_sample.x, lid_sample.y, lid_sample.z, lid_scale,
                           &lid_xs, &lid_ys, &lid_zs);
                
                log_debug("Lid: X=%d, Y=%d, Z=%d", lid_sample.x, lid_sample.y, lid_sample.z);
                
                /* Write to kernel module */
                if (cmxd_write_vector("lid", lid_xs, lid_ys, lid_zs) < 0) {
                    log_error("Failed to write lid vector to kernel module");
                    break;
                }
                
                lid_valid = 1;
                
                /* Reset error count on successful read */
                error_count = 0;
            }
        }
        
        /* Process sensor data if we have valid readings from both sensors */
        if (base_valid && lid_valid) {
            /* Convert to calculation structure format */
            struct cmxd_accel_sample base_calc_sample = {
                .x = base_sample.x,
                .y = base_sample.y,
                .z = base_sample.z,
                .timestamp = base_sample.timestamp
            };
            
            struct cmxd_accel_sample lid_calc_sample = {
                .x = lid_sample.x,
                .y = lid_sample.y,
                .z = lid_sample.z,
                .timestamp = lid_sample.timestamp
            };
            
            /* Calculate hinge angle for mode detection using 0-360° system */
            double hinge_angle = cmxd_calculate_hinge_angle_360(&base_calc_sample, &lid_calc_sample, base_scale, lid_scale);
            
            /* Calculate tilt angle for orientation display */
            double tilt_angle = cmxd_calculate_tilt_angle(lid_sample.x, lid_sample.y, lid_sample.z);
            
            /* Detect device mode using stable mode detection */
            const char* device_mode = "laptop";  /* Default fallback */
            if (hinge_angle >= 0) {
                /* Get orientation code for mode detection */
                int orientation_code = cmxd_get_device_orientation(lid_sample.x, lid_sample.y, lid_sample.z);
                device_mode = cmxd_get_stable_device_mode(hinge_angle, orientation_code);
            }
            
            /* Detect orientation using dual-sensor switching based on actual device mode */
            const char* orientation = cmxd_get_orientation_with_sensor_switching(
                lid_sample.x, lid_sample.y, lid_sample.z,
                base_sample.x, base_sample.y, base_sample.z, device_mode);
            
            log_debug("Processing sensor data - Base[%d,%d,%d] Lid[%d,%d,%d] Hinge=%.1f° Tilt=%.1f° -> Mode: %s, Orientation: %s", 
                    base_sample.x, base_sample.y, base_sample.z,
                    lid_sample.x, lid_sample.y, lid_sample.z, hinge_angle, tilt_angle, device_mode, orientation);
            
            /* Write detected mode to kernel module and send events */
            if (cmxd_write_mode_with_events(device_mode) < 0) {
                log_warn("Failed to write mode to kernel module");
            }
            
            /* Write detected orientation to kernel module and send events */
            if (cmxd_write_orientation_with_events(orientation) < 0) {
                log_warn("Failed to write orientation to kernel module");
            }
            
            /* Reset valid flags - we'll calculate again when new data arrives */
            base_valid = 0;
            lid_valid = 0;
        }
        
        /* Check for poll errors */
        if (poll_fds[0].revents & (POLLERR | POLLHUP | POLLNVAL)) {
            log_error("Poll error on base buffer");
            break;
        }
        if (poll_fds[1].revents & (POLLERR | POLLHUP | POLLNVAL)) {
            log_error("Poll error on lid buffer");
            break;
        }
    }
    
    log_info("Cleaning up IIO buffers...");
    cmxd_cleanup_iio_buffer(&base_buf);
    cmxd_cleanup_iio_buffer(&lid_buf);
    
    log_info("Event-driven main loop terminated");
    return 0;
}

/* Load configuration from file */
static int load_config_file(const char *config_path)
{
    FILE *fp;
    char line[256], key[64], value[192];
    char *eq_pos;
    
    fp = fopen(config_path, "r");
    if (!fp) {
        if (errno != ENOENT) {
            log_warn("Could not open config file %s: %s", config_path, strerror(errno));
        }
        return 0; /* Not finding config file is not an error */
    }
    
    log_info("Loading configuration from %s", config_path);
    
    while (fgets(line, sizeof(line), fp)) {
        /* Skip comments and empty lines */
        char *trimmed = line;
        while (*trimmed == ' ' || *trimmed == '\t') trimmed++;
        if (*trimmed == '#' || *trimmed == '\n' || *trimmed == '\0') continue;
        
        /* Find = separator */
        eq_pos = strchr(trimmed, '=');
        if (!eq_pos) continue;
        
        /* Extract key and value */
        *eq_pos = '\0';
        strncpy(key, trimmed, sizeof(key) - 1);
        key[sizeof(key) - 1] = '\0';
        strncpy(value, eq_pos + 1, sizeof(value) - 1);
        value[sizeof(value) - 1] = '\0';
        
        /* Remove trailing newline from value */
        char *newline = strchr(value, '\n');
        if (newline) *newline = '\0';
        
        /* Apply configuration */
        if (strcmp(key, "BUFFER_TIMEOUT_MS") == 0) {
            unsigned long timeout_ms = strtoul(value, NULL, 10);
            if (timeout_ms > 0 && timeout_ms <= 10000) {
                cfg.buffer_timeout_ms = (unsigned int)timeout_ms;
            }
        } else if (strcmp(key, "SYSFS_DIR") == 0) {
            strncpy(cfg.sysfs_path, value, sizeof(cfg.sysfs_path) - 1);
            cfg.sysfs_path[sizeof(cfg.sysfs_path) - 1] = '\0';
        }
    }
    
    fclose(fp);
    return 0;
}

/* Print usage information */
static void usage(void)
{
    printf("Usage: %s [OPTIONS]\n", PROGRAM_NAME);
    printf("\n");
    printf("Userspace daemon for Chuwi Minibook X tablet mode detection\n");
    printf("Device assignments are automatically detected from kernel module.\n");
    printf("\n");
    printf("Options:\n");
    printf("  -t, --timeout-ms MS      Buffer read timeout in milliseconds (default: %u)\n", cfg.buffer_timeout_ms);
    printf("  -s, --sysfs-path PATH    Kernel module sysfs path (default: %s)\n", cfg.sysfs_path);
    printf("  -d, --daemon             Run as daemon\n");
    printf("  -v, --verbose            Verbose logging (shows all debug information)\n");
    printf("  -h, --help               Show this help\n");
    printf("  -V, --version            Show version\n");
    printf("\n");
    printf("Examples:\n");
    printf("  %s                       # Use defaults with auto-detected devices\n", PROGRAM_NAME);
    printf("  %s -t 50 -v             # 50ms buffer timeout, verbose\n", PROGRAM_NAME);
}

/* Parse command line arguments */
static int parse_args(int argc, char **argv)
{
    static struct option long_options[] = {
        {"timeout-ms",  required_argument, 0, 't'},
        {"sysfs-path",  required_argument, 0, 's'},
        {"daemon",      no_argument,       0, 'd'},
        {"verbose",     no_argument,       0, 'v'},
        {"help",        no_argument,       0, 'h'},
        {"version",     no_argument,       0, 'V'},
        {0, 0, 0, 0}
    };
    
    int c;
    char *endptr;
    unsigned long val;
    
    while ((c = getopt_long(argc, argv, "t:s:dvhV", long_options, NULL)) != -1) {
        switch (c) {
            case 't':
                errno = 0;
                val = strtoul(optarg, &endptr, 10);
                if (errno != 0 || endptr == optarg || val == 0 || val > 10000) {
                    log_error("Invalid buffer timeout: %s (must be 1-10000 ms)", optarg);
                    return -1;
                }
                cfg.buffer_timeout_ms = (unsigned int)val;
                break;
                
            case 's':
                if (strlen(optarg) >= sizeof(cfg.sysfs_path)) {
                    log_error("Sysfs path too long");
                    return -1;
                }
                strcpy(cfg.sysfs_path, optarg);
                break;
                
            case 'd':
                cfg.daemon_mode = 1;
                break;
                
            case 'v':
                cfg.verbose = 1;
                break;
                
            case 'h':
                usage();
                exit(0);
                
            case 'V':
                printf("%s %s\n", PROGRAM_NAME, VERSION);
                exit(0);
                
            default:
                usage();
                return -1;
        }
    }
    
    if (optind < argc) {
        log_error("Unexpected argument: %s", argv[optind]);
        usage();
        return -1;
    }
    
    return 0;
}

/* Setup signal handlers */
static int setup_signals(void)
{
    struct sigaction sa;
    
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    
    if (sigaction(SIGTERM, &sa, NULL) < 0 ||
        sigaction(SIGINT, &sa, NULL) < 0 ||
        sigaction(SIGHUP, &sa, NULL) < 0) {
        log_error("Failed to setup signal handlers: %s", strerror(errno));
        return -1;
    }
    
    /* Ignore SIGPIPE */
    signal(SIGPIPE, SIG_IGN);
    
    return 0;
}

int main(int argc, char **argv)
{
    /* Parse command line arguments first to handle --help and --version cleanly */
    if (parse_args(argc, argv) < 0) {
        return 1;
    }
    
    /* Register cleanup function to run on normal exit */
    if (atexit(cleanup_and_exit) != 0) {
        log_warn("Failed to register cleanup function");
    }
    
    /* Load configuration file (may override some defaults) */
    load_config_file(CMXD_DEFAULT_CONFIG_FILE);
    
    /* Early validation: Check if IIO subsystem exists */
    if (access(IIO_BASE_PATH, F_OK) != 0) {
        log_error("IIO subsystem not found at %s", IIO_BASE_PATH);
        log_error("The Industrial I/O subsystem is required for accelerometer access");
        log_error("Make sure CONFIG_IIO is enabled in your kernel configuration");
        return 1;
    }
    
    /* Wait for kernel module sysfs interface to be available */
    char sysfs_path[PATH_MAX];
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"
    snprintf(sysfs_path, sizeof(sysfs_path), "%s/base_vec", cfg.sysfs_path);
#pragma GCC diagnostic pop
    if (cmxd_wait_for_path(sysfs_path, 2) < 0) {
        log_error("Kernel module sysfs interface not found: %s", cfg.sysfs_path);
        log_error("The cmx kernel module does not appear to be loaded");
        log_error("To load the module: sudo modprobe cmx");
        log_error("Or check if the module is available: modinfo cmx");
        return 1;
    }
    
    /* Initialize data module - needed for device assignment reading */
    struct cmxd_data_config data_cfg = {
        .verbose = cfg.verbose
    };
    snprintf(data_cfg.sysfs_path, sizeof(data_cfg.sysfs_path), "%s", cfg.sysfs_path);
    cmxd_data_init(&data_cfg, log_msg);
    log_debug("Data module initialized");
    
    /* Initialize event system */
    struct cmxd_events_config events_cfg = {
        .enable_unix_socket = cfg.enable_unix_socket,
        .enable_dbus = cfg.enable_dbus,
        .verbose = cfg.verbose
    };
    snprintf(events_cfg.unix_socket_path, sizeof(events_cfg.unix_socket_path), 
             "%s", cfg.unix_socket_path);
    
    if (cmxd_events_init(&events_cfg, log_msg) < 0) {
        log_error("Failed to initialize event system");
        return 1;
    }
    log_debug("Event system initialized");
    
    /* Read device assignments from kernel module - REQUIRED */
    if (cmxd_read_kernel_device_assignments(cfg.base_dev, sizeof(cfg.base_dev), 
                                            cfg.lid_dev, sizeof(cfg.lid_dev)) < 0) {
        log_error("Kernel device assignments not available - cannot continue");
        log_error("Make sure the kernel module is loaded and devices are detected");
        
        /* Provide helpful diagnostics */
        log_error("Diagnostic information:");
        log_error("  Expected sysfs path: %s", cfg.sysfs_path);
        
        /* Check if any IIO devices exist at all */
        if (access(IIO_DEVICES_PATH, F_OK) != 0) {
            log_error("  No IIO subsystem found (%s missing)", IIO_DEVICES_PATH);
            log_error("  The IIO subsystem may not be enabled in the kernel");
        } else {
            log_error("  IIO subsystem exists, checking for devices...");
            int result = system(IIO_DEVICES_LIST_CMD);
            (void)result; /* Suppress unused result warning - diagnostic only */
        }
        
        /* Check for accelerometer devices */
        if (access(IIO_DEV_DEVICE0, F_OK) != 0 && access(IIO_DEV_DEVICE1, F_OK) != 0) {
            log_error("  No IIO character devices found (%s)", IIO_DEV_CHAR_MSG);
            log_error("  Try: %s", IIO_DEV_LIST_CMD);
        }
        
        return 1;
    }
    
    /* Wait for devices to be ready */
    char base_path[PATH_MAX], lid_path[PATH_MAX];
    snprintf(base_path, sizeof(base_path), IIO_ACCEL_X_RAW_TEMPLATE, cfg.base_dev);
    if (cmxd_wait_for_path(base_path, 2) < 0) {
        log_error("Base IIO device not ready: %s", cfg.base_dev);
        return 1;
    }
    
    snprintf(lid_path, sizeof(lid_path), IIO_ACCEL_X_RAW_TEMPLATE, cfg.lid_dev);
    if (cmxd_wait_for_path(lid_path, 2) < 0) {
        log_error("Lid IIO device not ready: %s", cfg.lid_dev);
        return 1;
    }
    
    /* Setup signal handlers */
    if (setup_signals() < 0) {
        return 1;
    }
    
    log_info("Starting %s %s", PROGRAM_NAME, VERSION);
    log_info("Configuration: base=%s lid=%s timeout=%ums sysfs=%s", 
             cfg.base_dev, cfg.lid_dev, cfg.buffer_timeout_ms, cfg.sysfs_path);
    
    /* Validate paths */
    if (cmxd_validate_paths(cfg.base_dev, cfg.lid_dev) < 0) {
        log_error("Path validation failed");
        return 1;
    }
    
    /* Initialize orientation detection module */
    cmxd_orientation_init();
    cmxd_orientation_set_log_debug(orientation_log_debug);
    cmxd_orientation_set_verbose(cfg.verbose);
    log_debug("Orientation detection module initialized");
    
    /* Initialize mode detection module */
    cmxd_modes_init();
    cmxd_modes_set_log_debug(orientation_log_debug);
    cmxd_modes_set_verbose(cfg.verbose);
    log_debug("Mode detection module initialized");
    
    /* Initialize calculations module */
    cmxd_calculations_set_log_debug(orientation_log_debug);
    log_debug("Calculations module initialized");
    
    /* Run main loop */
    int ret = run_main_loop();
    
    log_info("Main loop finished, performing cleanup...");
    cleanup_and_exit();
    
    log_info("Exiting with code %d", ret == 0 ? 0 : 1);
    return ret == 0 ? 0 : 1;
}