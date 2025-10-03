// SPDX-License-Identifier: GPL-2.0
/* 
 * Chuwi Minibook X Tablet Mode Feeder
 * 
 * Userspace daemon that reads accelerometer data from IIO devices
 * and feeds it to the tablet mode detection kernel module.
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

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define PROGRAM_NAME "chuwi-minibook-x-tablet-mode"
#define VERSION "1.0"

/* Configuration */
struct config {
    char base_dev[PATH_MAX];
    char lid_dev[PATH_MAX];
    unsigned int poll_ms;
    int daemon_mode;
    int verbose;
    char sysfs_base[PATH_MAX];
};

/* Global state */
static volatile sig_atomic_t running = 1;
static struct config cfg = {
    .base_dev = "iio:device0",
    .lid_dev = "iio:device1", 
    .poll_ms = 100,
    .daemon_mode = 0,
    .verbose = 0,
    .sysfs_base = "/sys/kernel/chuwi-minibook-x-tablet-mode"
};

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

/* Safe file operations */
static FILE *safe_fopen(const char *path, const char *mode)
{
    FILE *f = fopen(path, mode);
    if (!f) {
        log_error("Failed to open %s: %s", path, strerror(errno));
    }
    return f;
}

static int safe_fclose(FILE *f, const char *path)
{
    if (fclose(f) != 0) {
        log_error("Failed to close %s: %s", path, strerror(errno));
        return -1;
    }
    return 0;
}

/* Read raw accelerometer axis from IIO sysfs */
static int read_iio_axis(const char *dev, const char *axis, int *out)
{
    char path[PATH_MAX];
    char buf[64];
    FILE *f;
    char *endptr;
    long value;
    
    if ((size_t)snprintf(path, sizeof(path), "/sys/bus/iio/devices/%s/in_accel_%s_raw", 
                 dev, axis) >= sizeof(path)) {
        log_error("Path too long for device %s axis %s", dev, axis);
        return -1;
    }
    
    f = safe_fopen(path, "r");
    if (!f) return -1;
    
    if (!fgets(buf, sizeof(buf), f)) {
        log_error("Failed to read from %s: %s", path, strerror(errno));
        safe_fclose(f, path);
        return -1;
    }
    
    safe_fclose(f, path);
    
    errno = 0;
    value = strtol(buf, &endptr, 10);
    
    if (errno != 0 || endptr == buf || value > INT_MAX || value < INT_MIN) {
        log_error("Invalid value in %s: '%s'", path, buf);
        return -1;
    }
    
    *out = (int)value;
    log_debug("Read %s/%s: %d", dev, axis, *out);
    return 0;
}

/* Read accelerometer scale factor */
static int read_iio_scale(const char *dev, double *scale)
{
    char path[PATH_MAX];
    FILE *f;
    
    if ((size_t)snprintf(path, sizeof(path), "/sys/bus/iio/devices/%s/in_accel_scale", 
                 dev) >= sizeof(path)) {
        log_error("Path too long for device %s scale", dev);
        return -1;
    }
    
    f = safe_fopen(path, "r");
    if (!f) {
        /* Scale file is optional */
        *scale = 0.0;
        return 0;
    }
    
    if (fscanf(f, "%lf", scale) != 1) {
        log_warn("Failed to read scale from %s", path);
        *scale = 0.0;
    }
    
    safe_fclose(f, path);
    log_debug("Read %s scale: %f", dev, *scale);
    return 0;
}

/* Read all axes from an IIO device */
static int read_iio_device(const char *dev, int *x, int *y, int *z, double *scale)
{
    if (read_iio_axis(dev, "x", x) < 0 ||
        read_iio_axis(dev, "y", y) < 0 ||
        read_iio_axis(dev, "z", z) < 0) {
        return -1;
    }
    
    read_iio_scale(dev, scale);
    return 0;
}

/* Apply scale factor and convert to microunits */
static void apply_scale(int raw_x, int raw_y, int raw_z, double scale,
                       int *scaled_x, int *scaled_y, int *scaled_z)
{
    if (scale > 0.0) {
        /* Convert to micro-g (μg) */
        *scaled_x = (int)(raw_x * scale * 1e6);
        *scaled_y = (int)(raw_y * scale * 1e6);
        *scaled_z = (int)(raw_z * scale * 1e6);
    } else {
        /* Fallback scaling */
        *scaled_x = raw_x * 1000;
        *scaled_y = raw_y * 1000;
        *scaled_z = raw_z * 1000;
    }
}

/* Write vector to kernel module sysfs */
static int write_vector(const char *name, int x, int y, int z)
{
    char path[PATH_MAX];
    FILE *f;
    
    if ((size_t)snprintf(path, sizeof(path), "%s/%s_vec", cfg.sysfs_base, name) >= sizeof(path)) {
        log_error("Path too long for %s vector", name);
        return -1;
    }
    
    f = safe_fopen(path, "w");
    if (!f) return -1;
    
    if (fprintf(f, "%d %d %d\n", x, y, z) < 0) {
        log_error("Failed to write to %s: %s", path, strerror(errno));
        safe_fclose(f, path);
        return -1;
    }
    
    if (safe_fclose(f, path) < 0) return -1;
    
    log_debug("Wrote %s: %d %d %d", name, x, y, z);
    return 0;
}

/* Validate that paths exist and are accessible */
static int validate_paths(void)
{
    char path[PATH_MAX];
    struct stat st;
    
    /* Check IIO devices */
    snprintf(path, sizeof(path), "/sys/bus/iio/devices/%s", cfg.base_dev);
    if (stat(path, &st) < 0) {
        log_error("Base IIO device not found: %s", path);
        return -1;
    }
    
    snprintf(path, sizeof(path), "/sys/bus/iio/devices/%s", cfg.lid_dev);
    if (stat(path, &st) < 0) {
        log_error("Lid IIO device not found: %s", path);
        return -1;
    }
    
    /* Check kernel module sysfs */
    if (stat(cfg.sysfs_base, &st) < 0) {
        log_error("Kernel module sysfs not found: %s", cfg.sysfs_base);
        return -1;
    }
    
    return 0;
}

/* Main processing loop */
static int run_feeder(void)
{
    int base_x, base_y, base_z;
    int lid_x, lid_y, lid_z;
    double base_scale, lid_scale;
    int base_xs, base_ys, base_zs;
    int lid_xs, lid_ys, lid_zs;
    unsigned int error_count = 0;
    const unsigned int max_errors = 10;
    
    log_info("Starting feeder loop (poll interval: %u ms)", cfg.poll_ms);
    
    while (running) {
        /* Read accelerometer data */
        if (read_iio_device(cfg.base_dev, &base_x, &base_y, &base_z, &base_scale) < 0 ||
            read_iio_device(cfg.lid_dev, &lid_x, &lid_y, &lid_z, &lid_scale) < 0) {
            
            error_count++;
            if (error_count >= max_errors) {
                log_error("Too many consecutive read errors (%u), exiting", error_count);
                return -1;
            }
            
            log_warn("Read error %u/%u, retrying...", error_count, max_errors);
            usleep(200000); /* 200ms recovery delay */
            continue;
        }
        
        /* Reset error count on successful read */
        error_count = 0;
        
        /* Apply scaling */
        apply_scale(base_x, base_y, base_z, base_scale, &base_xs, &base_ys, &base_zs);
        apply_scale(lid_x, lid_y, lid_z, lid_scale, &lid_xs, &lid_ys, &lid_zs);
        
        /* Write to kernel module */
        if (write_vector("base", base_xs, base_ys, base_zs) < 0 ||
            write_vector("lid", lid_xs, lid_ys, lid_zs) < 0) {
            log_error("Failed to write vectors to kernel module");
            return -1;
        }
        
        /* Sleep until next poll */
        usleep(cfg.poll_ms * 1000);
    }
    
    log_info("Feeder loop terminated");
    return 0;
}

/* Print usage information */
static void usage(void)
{
    printf("Usage: %s [OPTIONS]\n", PROGRAM_NAME);
    printf("\n");
    printf("Userspace feeder for Chuwi Minibook X tablet mode detection\n");
    printf("\n");
    printf("Options:\n");
    printf("  -b, --base-device DEV    Base accelerometer device (default: %s)\n", cfg.base_dev);
    printf("  -l, --lid-device DEV     Lid accelerometer device (default: %s)\n", cfg.lid_dev);
    printf("  -p, --poll-ms MS         Polling interval in milliseconds (default: %u)\n", cfg.poll_ms);
    printf("  -s, --sysfs-path PATH    Kernel module sysfs path (default: %s)\n", cfg.sysfs_base);
    printf("  -d, --daemon             Run as daemon\n");
    printf("  -v, --verbose            Verbose logging\n");
    printf("  -h, --help               Show this help\n");
    printf("  -V, --version            Show version\n");
    printf("\n");
    printf("Examples:\n");
    printf("  %s                                    # Use defaults\n", PROGRAM_NAME);
    printf("  %s -b iio:device0 -l iio:device1     # Specify devices\n", PROGRAM_NAME);
    printf("  %s -p 50 -v                          # 50ms polling, verbose\n", PROGRAM_NAME);
}

/* Parse command line arguments */
static int parse_args(int argc, char **argv)
{
    static struct option long_options[] = {
        {"base-device", required_argument, 0, 'b'},
        {"lid-device",  required_argument, 0, 'l'},
        {"poll-ms",     required_argument, 0, 'p'},
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
    
    while ((c = getopt_long(argc, argv, "b:l:p:s:dvhV", long_options, NULL)) != -1) {
        switch (c) {
            case 'b':
                if (strlen(optarg) >= sizeof(cfg.base_dev)) {
                    log_error("Base device name too long");
                    return -1;
                }
                strcpy(cfg.base_dev, optarg);
                break;
                
            case 'l':
                if (strlen(optarg) >= sizeof(cfg.lid_dev)) {
                    log_error("Lid device name too long");
                    return -1;
                }
                strcpy(cfg.lid_dev, optarg);
                break;
                
            case 'p':
                errno = 0;
                val = strtoul(optarg, &endptr, 10);
                if (errno != 0 || endptr == optarg || val == 0 || val > 10000) {
                    log_error("Invalid poll interval: %s (must be 1-10000 ms)", optarg);
                    return -1;
                }
                cfg.poll_ms = (unsigned int)val;
                break;
                
            case 's':
                if (strlen(optarg) >= sizeof(cfg.sysfs_base)) {
                    log_error("Sysfs path too long");
                    return -1;
                }
                strcpy(cfg.sysfs_base, optarg);
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
    /* Parse command line arguments */
    if (parse_args(argc, argv) < 0) {
        return 1;
    }
    
    /* Setup signal handlers */
    if (setup_signals() < 0) {
        return 1;
    }
    
    log_info("Starting %s %s", PROGRAM_NAME, VERSION);
    log_info("Configuration: base=%s lid=%s poll=%ums sysfs=%s", 
             cfg.base_dev, cfg.lid_dev, cfg.poll_ms, cfg.sysfs_base);
    
    /* Validate paths */
    if (validate_paths() < 0) {
        log_error("Path validation failed");
        return 1;
    }
    
    /* Run main loop */
    int ret = run_feeder();
    
    log_info("Exiting with code %d", ret == 0 ? 0 : 1);
    return ret == 0 ? 0 : 1;
}