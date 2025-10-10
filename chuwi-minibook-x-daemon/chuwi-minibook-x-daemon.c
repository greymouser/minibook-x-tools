// SPDX-License-Identifier: GPL-2.0
/* 
 * Chuwi Minibook X Daemon
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
#include <poll.h>
#include <fcntl.h>
#include <endian.h>
#include <stdint.h>
#include <sys/types.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define PROGRAM_NAME "chuwi-minibook-x-daemon"
#define VERSION "1.0"

/* Device name maximum length */
#define DEVICE_NAME_MAX 128

/* Forward declarations */
static int find_iio_device_for_i2c(int bus, int addr, char *device_name, size_t name_size);

/* Configuration */
struct config {
    char base_dev[DEVICE_NAME_MAX];
    char lid_dev[DEVICE_NAME_MAX];
    unsigned int poll_ms;
    int daemon_mode;
    int verbose;
    char sysfs_base[PATH_MAX];
};

/* IIO Buffer for event-driven reading */
struct iio_buffer {
    char device_name[DEVICE_NAME_MAX];
    int buffer_fd;
    int trigger_fd;
    char trigger_name[64];
    int x_index, y_index, z_index, timestamp_index;
    int sample_size;
    int enabled;
};

/* Accelerometer sample data */
struct accel_sample {
    int x, y, z;
    uint64_t timestamp;
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

/* Parse accelerometer value from buffer data (big-endian 12-bit) */
static int parse_accel_value(const uint8_t *data) {
    uint16_t raw = be16toh(*(uint16_t*)data);
    raw >>= 4; /* Right shift by 4 bits for be:s12/16>>4 format */
    
    /* Sign extend from 12-bit to 16-bit */
    if (raw & 0x800) {
        raw |= 0xF000;
    }
    
    return (int16_t)raw;
}

/* Setup IIO buffer for a device */
static int setup_iio_buffer(struct iio_buffer *buf, const char *device_name) {
    char path[PATH_MAX];
    char buffer[256];
    FILE *fp;
    
    memset(buf, 0, sizeof(*buf));
    strncpy(buf->device_name, device_name, sizeof(buf->device_name) - 1);
    buf->buffer_fd = -1;
    buf->trigger_fd = -1;
    
    /* Get device number from device name */
    int device_num;
    if (sscanf(device_name, "iio:device%d", &device_num) != 1) {
        log_error("Invalid device name format: %s", device_name);
        return -1;
    }
    
    /* Create trigger name - use the available sysfs trigger */
    snprintf(buf->trigger_name, sizeof(buf->trigger_name), "sysfstrig0");
    
    /* Read scan element indices */
    snprintf(path, sizeof(path), "/sys/bus/iio/devices/%s/scan_elements/in_accel_x_index", device_name);
    fp = fopen(path, "r");
    if (!fp || fscanf(fp, "%d", &buf->x_index) != 1) {
        log_error("Failed to read X index for %s", device_name);
        if (fp) fclose(fp);
        return -1;
    }
    fclose(fp);
    
    snprintf(path, sizeof(path), "/sys/bus/iio/devices/%s/scan_elements/in_accel_y_index", device_name);
    fp = fopen(path, "r");
    if (!fp || fscanf(fp, "%d", &buf->y_index) != 1) {
        log_error("Failed to read Y index for %s", device_name);
        if (fp) fclose(fp);
        return -1;
    }
    fclose(fp);
    
    snprintf(path, sizeof(path), "/sys/bus/iio/devices/%s/scan_elements/in_accel_z_index", device_name);
    fp = fopen(path, "r");
    if (!fp || fscanf(fp, "%d", &buf->z_index) != 1) {
        log_error("Failed to read Z index for %s", device_name);
        if (fp) fclose(fp);
        return -1;
    }
    fclose(fp);
    
    snprintf(path, sizeof(path), "/sys/bus/iio/devices/%s/scan_elements/in_timestamp_index", device_name);
    fp = fopen(path, "r");
    if (!fp || fscanf(fp, "%d", &buf->timestamp_index) != 1) {
        log_error("Failed to read timestamp index for %s", device_name);
        if (fp) fclose(fp);
        return -1;
    }
    fclose(fp);
    
    /* Enable scan elements */
    snprintf(path, sizeof(path), "/sys/bus/iio/devices/%s/scan_elements/in_accel_x_en", device_name);
    fp = fopen(path, "w");
    if (!fp || fprintf(fp, "1") < 0) {
        log_error("Failed to enable X scan element for %s", device_name);
        if (fp) fclose(fp);
        return -1;
    }
    fclose(fp);
    
    snprintf(path, sizeof(path), "/sys/bus/iio/devices/%s/scan_elements/in_accel_y_en", device_name);
    fp = fopen(path, "w");
    if (!fp || fprintf(fp, "1") < 0) {
        log_error("Failed to enable Y scan element for %s", device_name);
        if (fp) fclose(fp);
        return -1;
    }
    fclose(fp);
    
    snprintf(path, sizeof(path), "/sys/bus/iio/devices/%s/scan_elements/in_accel_z_en", device_name);
    fp = fopen(path, "w");
    if (!fp || fprintf(fp, "1") < 0) {
        log_error("Failed to enable Z scan element for %s", device_name);
        if (fp) fclose(fp);
        return -1;
    }
    fclose(fp);
    
    snprintf(path, sizeof(path), "/sys/bus/iio/devices/%s/scan_elements/in_timestamp_en", device_name);
    fp = fopen(path, "w");
    if (!fp || fprintf(fp, "1") < 0) {
        log_error("Failed to enable timestamp scan element for %s", device_name);
        if (fp) fclose(fp);
        return -1;
    }
    fclose(fp);
    
    /* Set current trigger */
    snprintf(path, sizeof(path), "/sys/bus/iio/devices/%s/trigger/current_trigger", device_name);
    fp = fopen(path, "w");
    if (!fp || fprintf(fp, "%s", buf->trigger_name) < 0) {
        log_error("Failed to set trigger for %s", device_name);
        if (fp) fclose(fp);
        return -1;
    }
    fclose(fp);
    
    /* Enable buffer */
    snprintf(path, sizeof(path), "/sys/bus/iio/devices/%s/buffer/enable", device_name);
    fp = fopen(path, "w");
    if (!fp || fprintf(fp, "1") < 0) {
        log_error("Failed to enable buffer for %s", device_name);
        if (fp) fclose(fp);
        return -1;
    }
    fclose(fp);
    
    /* Open buffer for reading */
    snprintf(path, sizeof(path), "/dev/%s", device_name);
    buf->buffer_fd = open(path, O_RDONLY | O_NONBLOCK);
    if (buf->buffer_fd < 0) {
        log_error("Failed to open buffer for %s: %s", device_name, strerror(errno));
        return -1;
    }
    
    /* Open trigger for control (optional) */
    buf->trigger_fd = -1;  /* We don't need to control the sysfs trigger directly */
    
    buf->sample_size = 16; /* 3 * 2 bytes + 8 bytes timestamp + padding */
    buf->enabled = 1;
    
    log_info("IIO buffer setup complete for %s", device_name);
    return 0;
}

/* Read sample from IIO buffer */
static int read_iio_buffer_sample(struct iio_buffer *buf, struct accel_sample *sample) {
    uint8_t buffer[16];
    ssize_t bytes_read;
    
    if (!buf->enabled || buf->buffer_fd < 0) {
        return -1;
    }
    
    bytes_read = read(buf->buffer_fd, buffer, buf->sample_size);
    if (bytes_read < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return 0; /* No data available */
        }
        log_error("Failed to read from buffer: %s", strerror(errno));
        return -1;
    }
    
    if (bytes_read != buf->sample_size) {
        log_warn("Unexpected buffer read size: %zd (expected %d)", bytes_read, buf->sample_size);
        return -1;
    }
    
    /* Parse accelerometer values based on indices */
    sample->x = parse_accel_value(&buffer[buf->x_index * 2]);
    sample->y = parse_accel_value(&buffer[buf->y_index * 2]);
    sample->z = parse_accel_value(&buffer[buf->z_index * 2]);
    
    /* Parse timestamp (little-endian 64-bit) */
    sample->timestamp = le64toh(*(uint64_t*)&buffer[buf->timestamp_index * 2]);
    
    return 1; /* Sample read successfully */
}

/* Cleanup IIO buffer */
static void cleanup_iio_buffer(struct iio_buffer *buf) {
    char path[PATH_MAX];
    FILE *fp;
    
    if (!buf->enabled) {
        return;
    }
    
    /* Disable buffer */
    snprintf(path, sizeof(path), "/sys/bus/iio/devices/%s/buffer/enable", buf->device_name);
    fp = fopen(path, "w");
    if (fp) {
        fprintf(fp, "0");
        fclose(fp);
    }
    
    /* Clear trigger */
    snprintf(path, sizeof(path), "/sys/bus/iio/devices/%s/trigger/current_trigger", buf->device_name);
    fp = fopen(path, "w");
    if (fp) {
        fprintf(fp, "");
        fclose(fp);
    }
    
    /* Close file descriptors */
    if (buf->buffer_fd >= 0) {
        close(buf->buffer_fd);
        buf->buffer_fd = -1;
    }
    
    if (buf->trigger_fd >= 0) {
        close(buf->trigger_fd);
        buf->trigger_fd = -1;
    }
    
    buf->enabled = 0;
    log_info("IIO buffer cleaned up for %s", buf->device_name);
}

/* Main processing loop with event-driven IIO reading */
static int run_feeder(void)
{
    struct iio_buffer base_buf, lid_buf;
    struct accel_sample base_sample, lid_sample;
    struct pollfd poll_fds[2];
    int base_xs, base_ys, base_zs;
    int lid_xs, lid_ys, lid_zs;
    unsigned int error_count = 0;
    const unsigned int max_errors = 10;
    int poll_timeout = 1000; /* 1 second timeout */
    
    log_info("Setting up IIO buffers for event-driven reading...");
    
    /* Setup IIO buffers */
    if (setup_iio_buffer(&base_buf, cfg.base_dev) < 0) {
        log_error("Failed to setup IIO buffer for base device %s", cfg.base_dev);
        return -1;
    }
    
    if (setup_iio_buffer(&lid_buf, cfg.lid_dev) < 0) {
        log_error("Failed to setup IIO buffer for lid device %s", cfg.lid_dev);
        cleanup_iio_buffer(&base_buf);
        return -1;
    }
    
    /* Setup poll file descriptors */
    poll_fds[0].fd = base_buf.buffer_fd;
    poll_fds[0].events = POLLIN;
    poll_fds[1].fd = lid_buf.buffer_fd;
    poll_fds[1].events = POLLIN;
    
    log_info("Starting event-driven feeder loop...");
    
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
            /* Timeout - continue to check running flag */
            continue;
        }
        
        /* Check for base sensor data */
        if (poll_fds[0].revents & POLLIN) {
            int result = read_iio_buffer_sample(&base_buf, &base_sample);
            if (result < 0) {
                error_count++;
                if (error_count >= max_errors) {
                    log_error("Too many consecutive base read errors (%u), exiting", error_count);
                    break;
                }
                log_warn("Base read error %u/%u", error_count, max_errors);
                continue;
            } else if (result > 0) {
                /* Apply scaling - use fixed scale for now since IIO buffer data is already scaled */
                apply_scale(base_sample.x, base_sample.y, base_sample.z, 1.0, 
                           &base_xs, &base_ys, &base_zs);
                
                log_debug("Base: X=%d, Y=%d, Z=%d", base_sample.x, base_sample.y, base_sample.z);
                
                /* Write to kernel module */
                if (write_vector("base", base_xs, base_ys, base_zs) < 0) {
                    log_error("Failed to write base vector to kernel module");
                    break;
                }
                
                /* Reset error count on successful read */
                error_count = 0;
            }
        }
        
        /* Check for lid sensor data */
        if (poll_fds[1].revents & POLLIN) {
            int result = read_iio_buffer_sample(&lid_buf, &lid_sample);
            if (result < 0) {
                error_count++;
                if (error_count >= max_errors) {
                    log_error("Too many consecutive lid read errors (%u), exiting", error_count);
                    break;
                }
                log_warn("Lid read error %u/%u", error_count, max_errors);
                continue;
            } else if (result > 0) {
                /* Apply scaling - use fixed scale for now since IIO buffer data is already scaled */
                apply_scale(lid_sample.x, lid_sample.y, lid_sample.z, 1.0,
                           &lid_xs, &lid_ys, &lid_zs);
                
                log_debug("Lid: X=%d, Y=%d, Z=%d", lid_sample.x, lid_sample.y, lid_sample.z);
                
                /* Write to kernel module */
                if (write_vector("lid", lid_xs, lid_ys, lid_zs) < 0) {
                    log_error("Failed to write lid vector to kernel module");
                    break;
                }
                
                /* Reset error count on successful read */
                error_count = 0;
            }
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
    cleanup_iio_buffer(&base_buf);
    cleanup_iio_buffer(&lid_buf);
    
    log_info("Event-driven feeder loop terminated");
    return 0;
}

/* Auto-detect MXC4005 devices */
static int auto_detect_devices(void)
{
    char path[PATH_MAX];
    char name[64];
    FILE *fp;
    int device_count = 0;
    char devices[2][DEVICE_NAME_MAX];
    
    log_info("Auto-detecting MXC4005 devices...");
    
    for (int i = 0; i < 10; i++) {
        snprintf(path, sizeof(path), "/sys/bus/iio/devices/iio:device%d/name", i);
        
        fp = safe_fopen(path, "r");
        if (!fp) continue;
        
        if (fgets(name, sizeof(name), fp) && strstr(name, "mxc4005")) {
            snprintf(devices[device_count], sizeof(devices[device_count]), "iio:device%d", i);
            log_info("Found MXC4005 device: %s", devices[device_count]);
            device_count++;
            if (device_count >= 2) {
                fclose(fp);
                break;
            }
        }
        fclose(fp);
    }
    
    if (device_count >= 2) {
        strcpy(cfg.base_dev, devices[0]);
        strcpy(cfg.lid_dev, devices[1]);
        log_info("Auto-configured: base=%s, lid=%s", cfg.base_dev, cfg.lid_dev);
        return 0;
    }
    
    log_warn("Could not auto-detect enough MXC4005 devices (found %d, need 2)", device_count);
    return -1;
}

/* Read device assignments from kernel module sysfs */
static int read_kernel_device_assignments(void)
{
    char path[PATH_MAX];
    FILE *fp;
    char device_info[64];
    int bus, addr;
    
    log_info("Reading device assignments from kernel module...");
    
    /* Read base device assignment */
    snprintf(path, sizeof(path), "%s/iio_base_device", cfg.sysfs_base);
    fp = safe_fopen(path, "r");
    if (!fp) {
        log_warn("Cannot read base device assignment from %s", path);
        return -1;
    }
    
    if (fgets(device_info, sizeof(device_info), fp)) {
        /* Remove trailing newline */
        char *newline = strchr(device_info, '\n');
        if (newline) *newline = '\0';
        
        /* Check if it's already an IIO device name */
        if (strncmp(device_info, "iio:device", 10) == 0) {
            strncpy(cfg.base_dev, device_info, sizeof(cfg.base_dev) - 1);
            cfg.base_dev[sizeof(cfg.base_dev) - 1] = '\0';
            log_info("Base device from kernel: %s", cfg.base_dev);
        } else if (sscanf(device_info, "i2c-%d:0x%x", &bus, &addr) == 2) {
            /* Fallback: I2C info, need to find IIO device */
            if (find_iio_device_for_i2c(bus, addr, cfg.base_dev, sizeof(cfg.base_dev)) == 0) {
                log_info("Base device from kernel: %s (i2c %d-0x%02x)", cfg.base_dev, bus, addr);
            } else {
                log_warn("Could not find IIO device for base i2c %d-0x%02x", bus, addr);
                fclose(fp);
                return -1;
            }
        } else {
            log_warn("Invalid base device format in kernel module: %s", device_info);
            fclose(fp);
            return -1;
        }
    }
    fclose(fp);
    
    /* Read lid device assignment */
    snprintf(path, sizeof(path), "%s/iio_lid_device", cfg.sysfs_base);
    fp = safe_fopen(path, "r");
    if (!fp) {
        log_warn("Cannot read lid device assignment from %s", path);
        return -1;
    }
    
    if (fgets(device_info, sizeof(device_info), fp)) {
        /* Remove trailing newline */
        char *newline = strchr(device_info, '\n');
        if (newline) *newline = '\0';
        
        /* Check if it's already an IIO device name */
        if (strncmp(device_info, "iio:device", 10) == 0) {
            strncpy(cfg.lid_dev, device_info, sizeof(cfg.lid_dev) - 1);
            cfg.lid_dev[sizeof(cfg.lid_dev) - 1] = '\0';
            log_info("Lid device from kernel: %s", cfg.lid_dev);
        } else if (sscanf(device_info, "i2c-%d:0x%x", &bus, &addr) == 2) {
            /* Fallback: I2C info, need to find IIO device */
            if (find_iio_device_for_i2c(bus, addr, cfg.lid_dev, sizeof(cfg.lid_dev)) == 0) {
                log_info("Lid device from kernel: %s (i2c %d-0x%02x)", cfg.lid_dev, bus, addr);
            } else {
                log_warn("Could not find IIO device for lid i2c %d-0x%02x", bus, addr);
                fclose(fp);
                return -1;
            }
        } else {
            log_warn("Invalid lid device format in kernel module: %s", device_info);
            fclose(fp);
            return -1;
        }
    }
    fclose(fp);
    
    return 0;
}

/* Find IIO device for given I2C bus/address */
static int find_iio_device_for_i2c(int bus, int addr, char *device_name, size_t name_size)
{
    char path[PATH_MAX];
    char link_target[PATH_MAX];
    char i2c_name[64];
    ssize_t link_len;
    
    /* Expected I2C device name format */
    snprintf(i2c_name, sizeof(i2c_name), "%d-%04x", bus, addr);
    
    /* Search through IIO devices to find one with matching I2C device */
    for (int i = 0; i < 10; i++) {
        snprintf(path, sizeof(path), "/sys/bus/iio/devices/iio:device%d/device", i);
        
        /* Read the symlink to see if it points to our I2C device */
        link_len = readlink(path, link_target, sizeof(link_target) - 1);
        if (link_len > 0) {
            link_target[link_len] = '\0';
            
            /* Check if the link target contains our I2C device name */
            if (strstr(link_target, i2c_name)) {
                snprintf(device_name, name_size, "iio:device%d", i);
                return 0;
            }
        }
    }
    
    return -1;
}

/* Wait for a path to exist */
static int wait_for_path(const char *path, int timeout_sec)
{
    int tries = timeout_sec * 2; /* Check every 0.5 seconds */
    
    while (tries-- > 0) {
        if (access(path, F_OK) == 0) {
            return 0;
        }
        usleep(500000); /* 0.5 seconds */
    }
    
    return -1;
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
        if (strcmp(key, "POLL_MS") == 0) {
            unsigned long poll_ms = strtoul(value, NULL, 10);
            if (poll_ms > 0 && poll_ms <= 10000) {
                cfg.poll_ms = (unsigned int)poll_ms;
            }
        } else if (strcmp(key, "SYSFS_DIR") == 0) {
            strncpy(cfg.sysfs_base, value, sizeof(cfg.sysfs_base) - 1);
            cfg.sysfs_base[sizeof(cfg.sysfs_base) - 1] = '\0';
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
    printf("Userspace feeder for Chuwi Minibook X tablet mode detection\n");
    printf("Device assignments are automatically detected from kernel module.\n");
    printf("\n");
    printf("Options:\n");
    printf("  -p, --poll-ms MS         Polling interval in milliseconds (default: %u)\n", cfg.poll_ms);
    printf("  -s, --sysfs-path PATH    Kernel module sysfs path (default: %s)\n", cfg.sysfs_base);
    printf("  -d, --daemon             Run as daemon\n");
    printf("  -v, --verbose            Verbose logging\n");
    printf("  -h, --help               Show this help\n");
    printf("  -V, --version            Show version\n");
    printf("\n");
    printf("Examples:\n");
    printf("  %s                       # Use defaults with auto-detected devices\n", PROGRAM_NAME);
    printf("  %s -p 50 -v             # 50ms polling, verbose\n", PROGRAM_NAME);
}

/* Parse command line arguments */
static int parse_args(int argc, char **argv)
{
    static struct option long_options[] = {
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
    
    while ((c = getopt_long(argc, argv, "p:s:dvhV", long_options, NULL)) != -1) {
        switch (c) {
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
    /* Load configuration file first */
    load_config_file("/etc/default/chuwi-minibook-x-daemon");
    
    /* Parse command line arguments (overrides config file) */
    if (parse_args(argc, argv) < 0) {
        return 1;
    }
    
    /* Wait for kernel module sysfs interface to be available */
    char sysfs_path[PATH_MAX];
    snprintf(sysfs_path, sizeof(sysfs_path), "%s/base_vec", cfg.sysfs_base);
    if (wait_for_path(sysfs_path, 30) < 0) {
        log_error("Kernel module sysfs interface not found: %s", cfg.sysfs_base);
        return 1;
    }
    
    /* Read device assignments from kernel module */
    if (read_kernel_device_assignments() < 0) {
        log_info("Kernel device assignments not available, falling back to auto-detection...");
        
        /* Auto-detect devices if kernel assignments failed */
        char base_path[PATH_MAX], lid_path[PATH_MAX];
        snprintf(base_path, sizeof(base_path), "/sys/bus/iio/devices/%s", cfg.base_dev);
        snprintf(lid_path, sizeof(lid_path), "/sys/bus/iio/devices/%s", cfg.lid_dev);
        
        if (access(base_path, F_OK) != 0 || access(lid_path, F_OK) != 0) {
            log_info("Configured devices not found, attempting auto-detection...");
            if (auto_detect_devices() < 0) {
                log_error("Failed to detect accelerometer devices");
                return 1;
            }
        }
    }
    
    /* Wait for devices to be ready */
    char base_path[PATH_MAX], lid_path[PATH_MAX];
    snprintf(base_path, sizeof(base_path), "/sys/bus/iio/devices/%s/in_accel_x_raw", cfg.base_dev);
    if (wait_for_path(base_path, 30) < 0) {
        log_error("Base IIO device not ready: %s", cfg.base_dev);
        return 1;
    }
    
    snprintf(lid_path, sizeof(lid_path), "/sys/bus/iio/devices/%s/in_accel_x_raw", cfg.lid_dev);
    if (wait_for_path(lid_path, 30) < 0) {
        log_error("Lid IIO device not ready: %s", cfg.lid_dev);
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