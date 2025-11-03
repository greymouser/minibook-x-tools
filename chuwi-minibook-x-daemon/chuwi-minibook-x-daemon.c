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
#include <stdbool.h>
#include <sys/types.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define PROGRAM_NAME "chuwi-minibook-x-daemon"
#define VERSION "1.0"

/* Device name maximum length */
#define DEVICE_NAME_MAX 128

/* Forward declarations */
static int find_iio_device_for_i2c(int bus, int addr, char *device_name, size_t name_size);
static int write_mode(const char *mode);
static int write_orientation(const char *orientation);

/* Configuration */
struct config {
    char base_dev[64];
    char lid_dev[64];
    char sysfs_path[PATH_MAX];
    unsigned int buffer_timeout_ms;
    int verbose;
    int daemon_mode;
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

/* Mode boundary angles - the core thresholds for mode detection */
static const double MODE_CLOSING_MAX = 45.0;    /* 0°-45°: Closing mode */
static const double MODE_LAPTOP_MAX = 135.0;    /* 45°-135°: Laptop mode */
static const double MODE_FLAT_MAX = 225.0;      /* 135°-225°: Flat mode */
static const double MODE_TENT_MAX = 315.0;      /* 225°-315°: Tent mode */
                                                 /* 315°-360°: Tablet mode */

/* Hysteresis amount to prevent spurious mode switching */
static const double MODE_HYSTERESIS = 10.0;     /* Degrees of hysteresis around boundaries */
static const double TABLET_MODE_HYSTERESIS = 25.0;  /* Extra large hysteresis for tablet mode stability */

/* Spurious data filtering - require consistency before mode changes */
static const int MODE_STABILITY_SAMPLES = 3;    /* Number of consistent readings required */
static const int TABLET_MODE_STABILITY_SAMPLES = 5;  /* Extra stability requirement for tablet mode exits */

/* Orientation change detection to prevent mode changes during device rotation */
static int last_detected_orientation = -1;            /* Track orientation changes */
static int orientation_freeze_samples = 0;            /* Freeze mode changes during orientation transitions */
static const int ORIENTATION_FREEZE_DURATION = 8;     /* Number of samples to freeze after orientation change */
static const int ORIENTATION_UNKNOWN = -1;            /* Unknown orientation constant */
static int stability_count = 0;                 /* Current consecutive count */
static const char* candidate_mode = NULL;       /* Mode candidate being evaluated */

/* Angle change rate limiting to prevent spurious calculations */
static const double MAX_ANGLE_CHANGE_PER_SAMPLE = 45.0;  /* Maximum degrees change per reading */

/* Mode stability tracking to prevent jumping between tablet/closing */
static const char* last_stable_mode = "laptop";  /* Default to laptop mode */
static double last_stable_mode_angle = 90.0;     /* Default to 90° hinge angle */

/* Last known orientation for tablet mode reading protection */
static const char* last_known_orientation = "landscape";  /* Default orientation */

static struct config cfg = {
    .base_dev = "iio:device0",
    .lid_dev = "iio:device1", 
    .buffer_timeout_ms = 100,
    .daemon_mode = 0,
    .verbose = 0,
    .sysfs_path = "/sys/devices/platform/chuwi-minibook-x"
};

/* Signal handlers */
static void cleanup_and_exit(void);  /* Forward declaration */

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
    
    if ((size_t)snprintf(path, sizeof(path), "%s/%s_vec", cfg.sysfs_path, name) >= sizeof(path)) {
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

/* Write mode to kernel module sysfs */
static int write_mode(const char *mode)
{
    char path[PATH_MAX];
    FILE *f;
    
    if ((size_t)snprintf(path, sizeof(path), "%s/mode", cfg.sysfs_path) >= sizeof(path)) {
        log_error("Path too long for mode");
        return -1;
    }
    
    f = safe_fopen(path, "w");
    if (!f) return -1;
    
    if (fprintf(f, "%s\n", mode) < 0) {
        log_error("Failed to write mode to %s: %s", path, strerror(errno));
        safe_fclose(f, path);
        return -1;
    }
    
    if (safe_fclose(f, path) < 0) return -1;
    
    log_debug("Wrote mode: %s", mode);
    return 0;
}

/* Write orientation to kernel module sysfs */
static int write_orientation(const char *orientation)
{
    char path[PATH_MAX];
    FILE *f;
    
    if ((size_t)snprintf(path, sizeof(path), "%s/orientation", cfg.sysfs_path) >= sizeof(path)) {
        log_error("Path too long for orientation");
        return -1;
    }
    
    f = safe_fopen(path, "w");
    if (!f) return -1;
    
    if (fprintf(f, "%s\n", orientation) < 0) {
        log_error("Failed to write orientation to %s: %s", path, strerror(errno));
        safe_fclose(f, path);
        return -1;
    }
    
    if (safe_fclose(f, path) < 0) return -1;
    
    log_debug("Wrote orientation: %s", orientation);
    return 0;
}

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
    if (write_mode("laptop") < 0) {
        log_warn("Failed to restore laptop mode during cleanup");
    }
    
    /* Force landscape orientation as safe default */
    if (write_orientation("landscape") < 0) {
        log_warn("Failed to restore landscape orientation during cleanup");
    }
    
    log_info("Cleanup complete - laptop mode restored");
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
    if (stat(cfg.sysfs_path, &st) < 0) {
        log_error("Kernel module sysfs not found: %s", cfg.sysfs_path);
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

/* Ensure IIO sysfs trigger exists (create if needed, but don't manage lifecycle) */
static int ensure_iio_trigger_exists(void) {
    char path[PATH_MAX];
    int trigger_id;
    
    /* Check if any trigger already exists (0-9) */
    for (trigger_id = 0; trigger_id < 10; trigger_id++) {
        snprintf(path, sizeof(path), "/sys/bus/iio/devices/trigger%d", trigger_id);
        if (access(path, F_OK) == 0) {
            log_debug("Using existing trigger: sysfstrig%d", trigger_id);
            return 0;  /* Found existing trigger */
        }
    }
    
    /* No trigger exists, create trigger0 */
    FILE *fp = fopen("/sys/bus/iio/devices/iio_sysfs_trigger/add_trigger", "w");
    if (!fp) {
        log_error("Failed to open trigger creation interface: %s", strerror(errno));
        return -1;
    }
    
    if (fprintf(fp, "0\n") < 0) {
        log_error("Failed to create trigger: %s", strerror(errno));
        fclose(fp);
        return -1;
    }
    
    fclose(fp);
    
    /* Verify the trigger was created */
    snprintf(path, sizeof(path), "/sys/bus/iio/devices/trigger0");
    if (access(path, F_OK) == 0) {
        log_info("Created persistent IIO trigger: sysfstrig0");
        return 0;
    } else {
        log_error("Trigger creation failed - trigger0 not found");
        return -1;
    }
}

/* Setup IIO buffer for a device */
static int setup_iio_buffer(struct iio_buffer *buf, const char *device_name) {
    char path[PATH_MAX];
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
    
    /* Use the first available trigger (we ensured one exists) */
    int trigger_id;
    for (trigger_id = 0; trigger_id < 10; trigger_id++) {
        snprintf(path, sizeof(path), "/sys/bus/iio/devices/trigger%d", trigger_id);
        if (access(path, F_OK) == 0) {
            snprintf(buf->trigger_name, sizeof(buf->trigger_name), "sysfstrig%d", trigger_id);
            log_debug("Using trigger: %s", buf->trigger_name);
            break;
        }
    }
    
    if (trigger_id >= 10) {
        log_error("No trigger found for %s - triggers must be available", device_name);
        return -1;
    }
    
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
        fprintf(fp, "\n");
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

/* Calculate the tilt angle of the device from horizontal plane */
/* Returns angle in degrees: 0° = flat, 90° = vertical */
static double calculate_tilt_angle(double x, double y, double z) {
    /* Calculate the angle between the Z-axis and the gravity vector */
    double magnitude = sqrt(x * x + y * y + z * z);
    if (magnitude < 1.0) {
        return -1.0; /* Invalid reading */
    }
    
    /* Z-component normalized gives cos(tilt_angle) */
    double cos_tilt = fabs(z) / magnitude;
    
    /* Clamp to valid range to avoid numerical errors */
    if (cos_tilt > 1.0) cos_tilt = 1.0;
    if (cos_tilt < 0.0) cos_tilt = 0.0;
    
    /* Convert to degrees */
    double tilt_angle = acos(cos_tilt) * 180.0 / M_PI;
    
    return tilt_angle;
}

/* Determine orientation of a device based on accelerometer readings */
/* Returns: 0=X-up, 1=Y-up, 2=Z-up, 3=X-down, 4=Y-down, 5=Z-down */
static int get_device_orientation(double x, double y, double z) {
    double abs_x = fabs(x);
    double abs_y = fabs(y);
    double abs_z = fabs(z);
    
    /* Find the axis with the largest magnitude (closest to gravity) */
    if (abs_z > abs_x && abs_z > abs_y) {
        return (z > 0) ? 2 : 5;  /* Z-up or Z-down */
    } else if (abs_y > abs_x) {
        return (y > 0) ? 1 : 4;  /* Y-up or Y-down */
    } else {
        return (x > 0) ? 0 : 3;  /* X-up or X-down */
    }
}

/* Map device orientation to standard platform terms */
/* Uses lid sensor orientation to determine screen orientation */
static const char* get_platform_orientation(int orientation_code) {
    switch (orientation_code) {
        case 3:  /* X-down - normal laptop landscape position */
            return "landscape";
        case 0:  /* X-up - laptop upside down landscape */
            return "landscape-flipped";
        case 1:  /* Y-up - laptop standing vertically (portrait) */
            return "portrait";
        case 4:  /* Y-down - laptop standing vertically (portrait-flipped) */
            return "portrait-flipped";
        case 2:  /* Z-up - unusual orientation, default to landscape */
        case 5:  /* Z-down - unusual orientation, default to landscape */
        default:
            return "landscape";  /* Default to landscape for edge cases */
    }
}

/* Get orientation with tablet mode reading protection */
/* Prevents orientation changes FROM portrait in tablet mode when tilted > 45° for reading stability */
static const char* get_orientation_with_tablet_protection(double x, double y, double z, const char* current_mode) {
    /* Calculate current orientation first */
    int orientation = get_device_orientation(x, y, z);
    const char* orientation_name = get_platform_orientation(orientation);
    
    /* Calculate tilt angle for tablet mode protection */
    double tilt_angle = calculate_tilt_angle(x, y, z);
    
    /* Enhanced tablet reading protection: Only protect against orientation changes during 
       actual reading scenarios with stability (not during active rotation) */
    static const char* stable_orientation = NULL;
    static int stable_count = 0;
    static const int STABILITY_THRESHOLD = 10;  /* Need 10 consistent samples */
    
    /* Track orientation stability */
    if (stable_orientation == NULL || strcmp(orientation_name, stable_orientation) != 0) {
        stable_orientation = orientation_name;
        stable_count = 1;
    } else {
        stable_count++;
    }
    
    /* Only apply protection if:
     * 1. In tablet mode
     * 2. High tilt angle (reading position)  
     * 3. Currently stable in portrait (not actively rotating)
     * 4. Trying to switch to landscape
     * 5. Have been stable for a while (not just started rotation) */
    if (strcmp(current_mode, "tablet") == 0 && 
        tilt_angle > 70.0 &&  /* Higher threshold for actual reading positions */
        stable_count >= STABILITY_THRESHOLD &&  /* Must be stable, not actively rotating */
        last_known_orientation != NULL && 
        (strcmp(last_known_orientation, "portrait") == 0 || strcmp(last_known_orientation, "portrait-flipped") == 0) &&  /* Currently in portrait */
        (strcmp(orientation_name, "landscape") == 0 || strcmp(orientation_name, "landscape-flipped") == 0)) {         /* Trying to switch to landscape */
        
        log_debug("Tablet reading protection: maintaining %s (tilt %.1f° > 70°, stable %d samples), blocking switch to %s", 
                 last_known_orientation, tilt_angle, stable_count, orientation_name);
        return last_known_orientation;
    }
    
    /* Normal orientation detection - update last known orientation */
    last_known_orientation = orientation_name;
    
    log_debug("Normal orientation: %s (tilt %.1f°, mode %s, stable %d)", 
             orientation_name, tilt_angle, current_mode, stable_count);
    return orientation_name;
}

/* Calculate hinge angle from base and lid accelerometer readings */
/* Uses orientation-independent dot product calculation with direction detection for full 0-360° range */
static double calculate_hinge_angle(const struct accel_sample *base, const struct accel_sample *lid) {
    static double last_base_angle = -1.0;
    static bool was_increasing = true;
    static bool definitely_folding_back = false;  /* Sticky fold-back state */
    
    /* Convert raw accelerometer values to normalized vectors */
    double base_magnitude = sqrt(base->x * base->x + base->y * base->y + base->z * base->z);
    double lid_magnitude = sqrt(lid->x * lid->x + lid->y * lid->y + lid->z * lid->z);
    
    if (base_magnitude < 1.0 || lid_magnitude < 1.0) {
        log_debug("Invalid accelerometer readings: base_mag=%.3f, lid_mag=%.3f", base_magnitude, lid_magnitude);
        return -1.0; /* Invalid reading */
    }
    
    /* Normalize the vectors */
    double base_norm[3] = {
        base->x / base_magnitude,
        base->y / base_magnitude, 
        base->z / base_magnitude
    };
    
    double lid_norm[3] = {
        lid->x / lid_magnitude,
        lid->y / lid_magnitude,
        lid->z / lid_magnitude
    };
    
    /* Calculate the dot product between normalized vectors */
    /* This gives us the cosine of the angle between the two accelerometer vectors */
    double dot_product = base_norm[0]*lid_norm[0] + base_norm[1]*lid_norm[1] + base_norm[2]*lid_norm[2];
    
    /* Clamp to valid range to avoid numerical errors in acos() */
    if (dot_product > 1.0) dot_product = 1.0;
    if (dot_product < -1.0) dot_product = -1.0;
    
    /* Convert to degrees - this gives us 0-180° range */
    double base_angle = acos(dot_product) * 180.0 / M_PI;
    
    /* Enhanced direction detection with realistic fold-back detection */
    bool folding_back = false;
    
    if (last_base_angle >= 0.0) {
        /* Determine if we're increasing or decreasing */
        bool currently_increasing = (base_angle > last_base_angle + 2.0);  /* 2° threshold for noise */
        bool currently_decreasing = (base_angle < last_base_angle - 2.0);
        
        /* Update trend if we have a clear direction change */
        if (currently_increasing && !was_increasing) {
            was_increasing = true;
            /* Exit fold-back mode when clearly increasing from small angles toward laptop range */
            if (base_angle > 50.0 && base_angle < 120.0) {  /* Mid-range indicates real unfolding */
                definitely_folding_back = false;
                log_debug("Trend change: now increasing at %.1f° - exiting fold-back mode (unfolding toward laptop)", base_angle);
            } else {
                log_debug("Trend change: now increasing at %.1f° - staying in fold-back mode", base_angle);
            }
        } else if (currently_decreasing && was_increasing) {
            /* FIXED: Look for decreasing trend from ANY angle in laptop+ range to detect fold-back
             * The real device shows base angles around 85-93° even when folding back */
            if (base_angle >= 80.0) {  /* Much more realistic threshold based on actual logs */
                was_increasing = false;
                definitely_folding_back = true;
                log_debug("Trend change: now decreasing from %.1f° - entering fold-back mode (realistic threshold)", base_angle);
            }
        }
        
        /* Fold-back exit: sustained increasing trend moving back toward laptop */
        if (currently_increasing && was_increasing && base_angle >= 70.0 && base_angle <= 110.0 && definitely_folding_back) {
            definitely_folding_back = false;
            log_debug("Fold-back exit: sustained increasing trend in laptop range %.1f° - exiting fold-back mode", base_angle);
        }
        
        /* We're in fold-back mode if:
         * 1. We're definitely folding back (sticky state), OR  
         * 2. We're decreasing and angle suggests fold-back territory */
        folding_back = definitely_folding_back || (!was_increasing && base_angle >= 75.0);
        
        /* Special handling: if we're decreasing from flat-ish angles, we're folding back */
        if (!was_increasing && last_base_angle >= 80.0 && base_angle >= 75.0) {
            folding_back = true;
            if (!definitely_folding_back) {
                log_debug("Detected fold-back motion: decreasing from %.1f° to %.1f°", last_base_angle, base_angle);
            }
        }
    }
    
    /* Final angle calculation */
    double final_angle;
    if (folding_back) {
        /* Map 0-180° to 180-360° range for folding back positions */
        final_angle = 360.0 - base_angle;
        log_debug("Hinge calculation (folding back): base[%d,%d,%d] lid[%d,%d,%d] -> dot=%.3f, base_angle=%.1f°, trend=%s, sticky=%s, final=%.1f°", 
                 base->x, base->y, base->z, lid->x, lid->y, lid->z, dot_product, base_angle, 
                 was_increasing ? "increasing" : "decreasing", definitely_folding_back ? "yes" : "no", final_angle);
    } else {
        /* Normal opening: use base angle directly */
        final_angle = base_angle;
        log_debug("Hinge calculation (normal opening): base[%d,%d,%d] lid[%d,%d,%d] -> dot=%.3f, base_angle=%.1f°, trend=%s, sticky=%s, final=%.1f°", 
                 base->x, base->y, base->z, lid->x, lid->y, lid->z, dot_product, base_angle,
                 was_increasing ? "increasing" : "decreasing", definitely_folding_back ? "yes" : "no", final_angle);
    }
    
    last_base_angle = base_angle;
    
    /* The enhanced approach gives us:
     * - Stable angle calculation independent of device orientation (dot product)
     * - Full 0-360° range for existing mode boundaries (trend-based direction detection)
     * - Sticky fold-back state to prevent premature exits from tent/tablet detection
     * - Special tablet mode handling for angles near 0°/360° */
    
    return final_angle;
}

/* Validate angle change to filter spurious readings */
/* Returns true if angle change seems valid, false if too large and should be ignored */
/* Handles 0-360° wrapping for trend-based direction detection */
/* SAFETY: Always allow reasonable laptop mode angles to prevent lockout */
static bool validate_angle_change(double new_angle, double last_angle, const char *context) {
    static double last_valid_angle = -1.0;
    
    /* Initialize on first call */
    if (last_valid_angle < 0.0) {
        last_valid_angle = new_angle;
        log_debug("Angle validation initialized: %.1f° (%s)", new_angle, context);
        return true;
    }
    
    /* SAFETY OVERRIDE: Always allow laptop mode angles (45-135°) to prevent lockout */
    if (new_angle >= 45.0 && new_angle <= 135.0) {
        last_valid_angle = new_angle;
        log_debug("Angle validation SAFETY OVERRIDE: %.1f° (laptop mode) - allowing to prevent lockout (%s)", 
                 new_angle, context);
        return true;
    }
    
    /* Calculate the change from last valid angle */
    double angle_change = fabs(new_angle - last_valid_angle);
    
    /* Handle trend-based 0-360° mapping transitions:
     * The key insight is that angles 180-360° are mapped from the same 0-180° base range
     * So a transition from 270° (folding back) to 90° (normal) represents the same base angle
     * and should be considered a small change, not a large jump */
    
    double min_change = angle_change;
    
    /* Check for folding-back to normal-opening transitions */
    if (last_valid_angle > 180.0 && new_angle < 180.0) {
        /* Transition from folding back (180-360°) to normal opening (0-180°) */
        /* Map both to base angles and compare */
        double last_base = 360.0 - last_valid_angle;  /* Convert folding-back to base angle */
        double new_base = new_angle;                   /* Already a base angle */
        double base_change = fabs(new_base - last_base);
        if (base_change < min_change) {
            min_change = base_change;
            log_debug("Fold-back to normal transition: %.1f° -> %.1f° (base: %.1f° -> %.1f°, change: %.1f°)", 
                     last_valid_angle, new_angle, last_base, new_base, base_change);
        }
    } else if (last_valid_angle < 180.0 && new_angle > 180.0) {
        /* Transition from normal opening (0-180°) to folding back (180-360°) */
        double last_base = last_valid_angle;           /* Already a base angle */
        double new_base = 360.0 - new_angle;          /* Convert folding-back to base angle */
        double base_change = fabs(new_base - last_base);
        if (base_change < min_change) {
            min_change = base_change;
            log_debug("Normal to fold-back transition: %.1f° -> %.1f° (base: %.1f° -> %.1f°, change: %.1f°)", 
                     last_valid_angle, new_angle, last_base, new_base, base_change);
        }
    }
    
    /* Also check standard wrapping around 0°/360° boundary */
    if (last_valid_angle > 300.0 && new_angle < 60.0) {
        /* Transition from high angle to low angle (360° -> 0° wrap) */
        double wrap_change = (360.0 - last_valid_angle) + new_angle;
        if (wrap_change < min_change) min_change = wrap_change;
    } else if (last_valid_angle < 60.0 && new_angle > 300.0) {
        /* Transition from low angle to high angle (0° -> 360° wrap) */
        double wrap_change = (360.0 - new_angle) + last_valid_angle;
        if (wrap_change < min_change) min_change = wrap_change;
    }
    
    /* Check if change is reasonable */
    if (min_change <= MAX_ANGLE_CHANGE_PER_SAMPLE) {
        last_valid_angle = new_angle;
        log_debug("Angle validation passed: %.1f° -> %.1f° (min_change: %.1f°) (%s)", 
                 last_angle, new_angle, min_change, context);
        return true;
    } else {
        log_debug("Angle validation FAILED: %.1f° -> %.1f° (min_change: %.1f° > max %.1f°) - ignoring (%s)", 
                 last_valid_angle, new_angle, min_change, MAX_ANGLE_CHANGE_PER_SAMPLE, context);
        return false;
    }
}

/* Get laptop mode based on hinge angle with hysteresis to prevent spurious mode changes */
static const char* get_laptop_mode(double angle, const char* current_mode) {
    if (angle < 0) {
        return "laptop";        /* Invalid angle - default to laptop */
    }
    
    /* Use enhanced hysteresis: different thresholds for entering vs staying in a mode
     * Tablet mode gets enhanced protection with larger hysteresis */
    double hysteresis = MODE_HYSTERESIS;  /* Default: 10.0 degrees */
    if (strcmp(current_mode, "tablet") == 0) {
        hysteresis = TABLET_MODE_HYSTERESIS;  /* Enhanced: 25.0 degrees */
    }
    
    if (strcmp(current_mode, "closing") == 0) {
        /* In closing mode: need significant movement to exit */
        if (angle >= (MODE_CLOSING_MAX + hysteresis)) {
            return "laptop";
        }
        return "closing";
    } else if (strcmp(current_mode, "laptop") == 0) {
        /* In laptop mode: need significant movement to exit */
        if (angle < (MODE_CLOSING_MAX - hysteresis)) {
            return "closing";
        } else if (angle >= (MODE_LAPTOP_MAX + hysteresis)) {
            return "flat";
        }
        return "laptop";
    } else if (strcmp(current_mode, "flat") == 0) {
        /* In flat mode: need significant movement to exit */
        if (angle < (MODE_LAPTOP_MAX - hysteresis)) {
            return "laptop";
        } else if (angle >= (MODE_FLAT_MAX + hysteresis)) {
            return "tent";
        }
        return "flat";
    } else if (strcmp(current_mode, "tent") == 0) {
        /* In tent mode: need significant movement to exit */
        if (angle < (MODE_FLAT_MAX - hysteresis)) {
            return "flat";
        } else if (angle >= (MODE_TENT_MAX + hysteresis)) {
            return "tablet";
        }
        return "tent";
    } else if (strcmp(current_mode, "tablet") == 0) {
        /* In tablet mode: need significant movement to exit with ENHANCED protection */
        if (angle < (MODE_TENT_MAX - hysteresis)) {  /* 25° hysteresis vs 10° default */
            log_debug("Tablet mode exit: angle %.1f° < threshold %.1f° (enhanced hysteresis %.1f°)", 
                     angle, MODE_TENT_MAX - hysteresis, hysteresis);
            return "tent";
        }
        return "tablet";
    }
    
    /* Fallback to original logic if current_mode is unknown */
    if (angle >= 0 && angle < MODE_CLOSING_MAX) {
        return "closing";       
    } else if (angle >= MODE_CLOSING_MAX && angle < MODE_LAPTOP_MAX) {
        return "laptop";        
    } else if (angle >= MODE_LAPTOP_MAX && angle < MODE_FLAT_MAX) {
        return "flat";          
    } else if (angle >= MODE_FLAT_MAX && angle < MODE_TENT_MAX) {
        return "tent";          
    } else {
        return "tablet";        
    }
}

/* Get stable laptop mode with hysteresis, sequential validation, and spurious data filtering */
/* Only allows transitions between adjacent modes: closing <-> laptop <-> flat <-> tent <-> tablet */
/* Uses hysteresis to prevent spurious mode changes from sensor noise */
/* Requires multiple consistent readings before allowing mode changes */
static const char* get_stable_laptop_mode(double angle, int orientation) {
    /* Check for orientation changes and freeze mode changes during transitions */
    if (last_detected_orientation != ORIENTATION_UNKNOWN && orientation != last_detected_orientation) {
        log_debug("Orientation change detected: %d -> %d, freezing mode changes for %d samples", 
                 last_detected_orientation, orientation, ORIENTATION_FREEZE_DURATION);
        orientation_freeze_samples = ORIENTATION_FREEZE_DURATION;
        last_detected_orientation = orientation;
    } else if (last_detected_orientation == ORIENTATION_UNKNOWN) {
        last_detected_orientation = orientation;
    }
    
    /* If we're in orientation freeze, maintain current mode */
    if (orientation_freeze_samples > 0) {
        orientation_freeze_samples--;
        log_debug("Mode frozen due to orientation change (remaining: %d samples), maintaining mode", 
                 orientation_freeze_samples);
        /* Reset stability tracking during freeze to prevent spurious changes */
        stability_count = 0;
        candidate_mode = NULL;
        return last_stable_mode;
    }
    
    /* Determine required stability samples based on current mode */
    int required_stability = MODE_STABILITY_SAMPLES;  /* Default: 3 */
    if (strcmp(last_stable_mode, "tablet") == 0) {
        required_stability = TABLET_MODE_STABILITY_SAMPLES;  /* Enhanced: 5 */
    }
    
    const char* new_mode = get_laptop_mode(angle, last_stable_mode);
    
    /* If mode hasn't changed, update tracking and return */
    if (strcmp(new_mode, last_stable_mode) == 0) {
        last_stable_mode_angle = angle;
        /* Reset stability tracking since we're staying in current mode */
        stability_count = 0;
        candidate_mode = NULL;
        return new_mode;
    }
    
    /* Check if this is a valid transition */
    int valid_transition = 0;
    
    if (strcmp(last_stable_mode, "closing") == 0) {
        /* closing can go to laptop */
        valid_transition = (strcmp(new_mode, "laptop") == 0);
    } else if (strcmp(last_stable_mode, "laptop") == 0) {
        /* laptop can go to closing or flat */
        valid_transition = (strcmp(new_mode, "closing") == 0 || strcmp(new_mode, "flat") == 0);
    } else if (strcmp(last_stable_mode, "flat") == 0) {
        /* flat can go to laptop or tent */
        valid_transition = (strcmp(new_mode, "laptop") == 0 || strcmp(new_mode, "tent") == 0);
    } else if (strcmp(last_stable_mode, "tent") == 0) {
        /* tent can go to flat or tablet */
        valid_transition = (strcmp(new_mode, "flat") == 0 || strcmp(new_mode, "tablet") == 0);
    } else if (strcmp(last_stable_mode, "tablet") == 0) {
        /* tablet can go to tent */
        valid_transition = (strcmp(new_mode, "tent") == 0);
    }
    
    if (!valid_transition) {
        log_debug("Invalid mode jump blocked: %s -> %s (angle %.1f°)", last_stable_mode, new_mode, angle);
        /* Reset stability tracking since this transition is not allowed */
        stability_count = 0;
        candidate_mode = NULL;
        return last_stable_mode;
    }
    
    /* Valid transition - now check for stability to prevent spurious changes */
    if (candidate_mode == NULL || strcmp(candidate_mode, new_mode) != 0) {
        /* New candidate mode - start stability tracking */
        candidate_mode = new_mode;
        stability_count = 1;
        log_debug("New mode candidate: %s (stability 1/%d, angle %.1f°, enhanced_tablet=%s)", 
                 new_mode, required_stability, angle, 
                 (required_stability == TABLET_MODE_STABILITY_SAMPLES) ? "yes" : "no");
        return last_stable_mode;  /* Stay in current mode until stable */
    } else {
        /* Same candidate mode - increment stability count */
        stability_count++;
        log_debug("Mode candidate: %s (stability %d/%d, angle %.1f°, enhanced_tablet=%s)", 
                 new_mode, stability_count, required_stability, angle,
                 (required_stability == TABLET_MODE_STABILITY_SAMPLES) ? "yes" : "no");
        
        if (stability_count >= required_stability) {
            /* Candidate is stable - make the transition */
            log_debug("Mode transition confirmed: %s -> %s (angle %.1f°, samples=%d)", 
                     last_stable_mode, new_mode, angle, required_stability);
            last_stable_mode = new_mode;
            last_stable_mode_angle = angle;
            /* Reset stability tracking */
            stability_count = 0;
            candidate_mode = NULL;
            return new_mode;
        } else {
            /* Not stable yet - stay in current mode */
            return last_stable_mode;
        }
    }
}

/* Trigger sysfs trigger to generate IIO buffer samples */
static int trigger_iio_sampling(void) {
    FILE *fp;
    char path[PATH_MAX];
    int trigger_id;
    
    /* Find the first available trigger */
    for (trigger_id = 0; trigger_id < 10; trigger_id++) {
        snprintf(path, sizeof(path), "/sys/bus/iio/devices/trigger%d/trigger_now", trigger_id);
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
static int run_feeder(void)
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
    
    /* Ensure IIO trigger exists (create if needed, but leave persistent) */
    log_info("Ensuring IIO trigger is available...");
    if (ensure_iio_trigger_exists() < 0) {
        log_error("Failed to ensure IIO trigger exists");
        return -1;
    }
    
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
            /* Timeout - trigger new samples and continue */
            trigger_iio_sampling();
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
                
                base_valid = 1;
                
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
                
                lid_valid = 1;
                
                /* Reset error count on successful read */
                error_count = 0;
            }
        }
        
        /* Calculate hinge angle if we have valid readings from both sensors */
        if (base_valid && lid_valid) {
            double angle = calculate_hinge_angle(&base_sample, &lid_sample);
            
            /* Validate angle change to filter spurious readings */
            if (angle >= 0 && validate_angle_change(angle, -1.0, "main_loop")) {
                /* Get device orientation (needed for mode freezing during orientation changes) */
                int orientation_value = get_device_orientation(lid_sample.x, lid_sample.y, lid_sample.z);
                
                /* Get stable mode with enhanced tablet protection and orientation awareness */
                const char* mode = get_stable_laptop_mode(angle, orientation_value);
                
                /* Get device orientation string with tablet mode reading protection */
                /* In tablet mode, prevents orientation changes when tilted > 45° for reading stability */
                const char* orientation = get_orientation_with_tablet_protection(
                    lid_sample.x, lid_sample.y, lid_sample.z, mode);
                
                log_info("Hinge angle: %.1f° (%s) - Orientation: %s - Base[%d,%d,%d] Lid[%d,%d,%d]", 
                        angle, mode, orientation,
                        base_sample.x, base_sample.y, base_sample.z,
                        lid_sample.x, lid_sample.y, lid_sample.z);
                
                /* Write detected mode to kernel module */
                if (write_mode(mode) < 0) {
                    log_warn("Failed to write mode '%s' to kernel module", mode);
                }
                
                /* Write detected orientation to kernel module */
                if (write_orientation(orientation) < 0) {
                    log_warn("Failed to write orientation '%s' to kernel module", orientation);
                }
            } else if (angle >= 0) {
                log_debug("Skipping mode update due to spurious angle reading: %.1f°", angle);
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
    cleanup_iio_buffer(&base_buf);
    cleanup_iio_buffer(&lid_buf);
    
    log_info("Event-driven feeder loop terminated");
    return 0;
}

/* Read device assignments from kernel module sysfs */
static int read_kernel_device_assignments(void)
{
    char path[PATH_MAX];
    FILE *fp;
    char device_info[64];
    int bus;
    unsigned int addr;
    
    log_info("Reading device assignments from kernel module...");
    
    /* Read base device assignment */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"
    snprintf(path, sizeof(path), "%s/iio_base_device", cfg.sysfs_path);
#pragma GCC diagnostic pop
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
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"
    snprintf(path, sizeof(path), "%s/iio_lid_device", cfg.sysfs_path);
#pragma GCC diagnostic pop
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
    printf("Userspace feeder for Chuwi Minibook X tablet mode detection\n");
    printf("Device assignments are automatically detected from kernel module.\n");
    printf("\n");
    printf("Options:\n");
    printf("  -t, --timeout-ms MS      Buffer read timeout in milliseconds (default: %u)\n", cfg.buffer_timeout_ms);
    printf("  -s, --sysfs-path PATH    Kernel module sysfs path (default: %s)\n", cfg.sysfs_path);
    printf("  -d, --daemon             Run as daemon\n");
    printf("  -v, --verbose            Verbose logging\n");
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
    /* Register cleanup function to run on normal exit */
    if (atexit(cleanup_and_exit) != 0) {
        log_warn("Failed to register cleanup function");
    }
    
    /* Load configuration file first */
    load_config_file("/etc/default/chuwi-minibook-x-daemon");
    
    /* Parse command line arguments (overrides config file) */
    if (parse_args(argc, argv) < 0) {
        return 1;
    }
    
    /* Wait for kernel module sysfs interface to be available */
    char sysfs_path[PATH_MAX];
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"
    snprintf(sysfs_path, sizeof(sysfs_path), "%s/base_vec", cfg.sysfs_path);
#pragma GCC diagnostic pop
    if (wait_for_path(sysfs_path, 30) < 0) {
        log_error("Kernel module sysfs interface not found: %s", cfg.sysfs_path);
        return 1;
    }
    
    /* Read device assignments from kernel module - REQUIRED */
    if (read_kernel_device_assignments() < 0) {
        log_error("Kernel device assignments not available - cannot continue");
        log_error("Make sure the kernel module is loaded and devices are detected");
        return 1;
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
    log_info("Configuration: base=%s lid=%s timeout=%ums sysfs=%s", 
             cfg.base_dev, cfg.lid_dev, cfg.buffer_timeout_ms, cfg.sysfs_path);
    
    /* Validate paths */
    if (validate_paths() < 0) {
        log_error("Path validation failed");
        return 1;
    }
    
    /* Initialize last known orientation to landscape (safe default) */
    last_known_orientation = "landscape";
    
    /* Run main loop */
    int ret = run_feeder();
    
    log_info("Main loop finished, performing cleanup...");
    cleanup_and_exit();
    
    log_info("Exiting with code %d", ret == 0 ? 0 : 1);
    return ret == 0 ? 0 : 1;
}