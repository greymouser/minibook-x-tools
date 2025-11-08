/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Data Handling Module - File I/O and IIO Operations
 * 
 * Provides centralized data access functions including file operations,
 * IIO device management, and kernel module communication via sysfs.
 * Handles accelerometer data reading, scaling, and buffer management.
 */

#include "cmxd-data.h"
#include "cmxd-paths.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <endian.h>
#include <stdarg.h>
#include <math.h>

/* Module state */
static struct cmxd_data_config *data_config = NULL;
static log_func_t log_function = NULL;

/* Logging macros using the configured log function */
#define log_error(fmt, ...) do { if (log_function) log_function("ERROR", fmt, ##__VA_ARGS__); } while(0)
#define log_warn(fmt, ...)  do { if (log_function) log_function("WARN", fmt, ##__VA_ARGS__); } while(0)
#define log_info(fmt, ...)  do { if (log_function) log_function("INFO", fmt, ##__VA_ARGS__); } while(0)
#define log_debug(fmt, ...) do { if (log_function) log_function("DEBUG", fmt, ##__VA_ARGS__); } while(0)

/*
 * =============================================================================
 * MODULE INITIALIZATION
 * =============================================================================
 */

/* Initialize data module */
void cmxd_data_init(struct cmxd_data_config *config, log_func_t log_func)
{
    data_config = config;
    log_function = log_func;
}

/*
 * =============================================================================
 * SAFE FILE OPERATIONS
 * =============================================================================
 */

/* Safe file operations */
FILE *cmxd_safe_fopen(const char *path, const char *mode)
{
    FILE *f = fopen(path, mode);
    if (!f) {
        log_error("Failed to open %s: %s", path, strerror(errno));
    }
    return f;
}

int cmxd_safe_fclose(FILE *f, const char *path)
{
    if (fclose(f) != 0) {
        log_error("Failed to close %s: %s", path, strerror(errno));
        return -1;
    }
    return 0;
}

/*
 * =============================================================================
 * DATA PROCESSING AND SCALING
 * =============================================================================
 */

/* Read accelerometer scale factor from IIO device */
double cmxd_read_accel_scale(const char *device_name)
{
    char scale_path[PATH_MAX];
    FILE *fp;
    double scale = 0.0;
    
    snprintf(scale_path, sizeof(scale_path), 
             "/sys/bus/iio/devices/%s/in_accel_scale", device_name);
    
    fp = cmxd_safe_fopen(scale_path, "r");
    if (fp) {
        if (fscanf(fp, "%lf", &scale) != 1) {
            log_warn("Failed to read scale from %s", scale_path);
            scale = 0.0;
        } else {
            log_debug("Read scale %f from %s", scale, device_name);
        }
        cmxd_safe_fclose(fp, scale_path);
    } else {
        log_warn("Failed to open scale file %s", scale_path);
    }
    
    return scale;
}

/* Apply scale factor and convert to microunits */
void cmxd_apply_scale(int raw_x, int raw_y, int raw_z, double scale,
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

/*
 * =============================================================================
 * KERNEL MODULE COMMUNICATION (SYSFS)
 * =============================================================================
 */

/* Write vector to kernel module sysfs */
int cmxd_write_vector(const char *name, int x, int y, int z)
{
    char path[PATH_MAX];
    FILE *f;
    
    if (!data_config) {
        log_error("Data module not initialized");
        return -1;
    }
    
    if ((size_t)snprintf(path, sizeof(path), "%s/%s_vec", data_config->sysfs_path, name) >= sizeof(path)) {
        log_error("Path too long for %s vector", name);
        return -1;
    }
    
    f = cmxd_safe_fopen(path, "w");
    if (!f) return -1;
    
    if (fprintf(f, "%d %d %d\n", x, y, z) < 0) {
        log_error("Failed to write to %s: %s", path, strerror(errno));
        cmxd_safe_fclose(f, path);
        return -1;
    }
    
    if (cmxd_safe_fclose(f, path) < 0) return -1;

    /* Sensor write debug output reduced for cleaner format */
    return 0;
}

/* Write mode to kernel module sysfs */
int cmxd_write_mode(const char *mode)
{
    char path[PATH_MAX];
    FILE *f;
    
    if (!data_config) {
        log_error("Data module not initialized");
        return -1;
    }
    
    if ((size_t)snprintf(path, sizeof(path), "%s/mode", data_config->sysfs_path) >= sizeof(path)) {
        log_error("Path too long for mode");
        return -1;
    }
    
    f = cmxd_safe_fopen(path, "w");
    if (!f) return -1;
    
    if (fprintf(f, "%s\n", mode) < 0) {
        log_error("Failed to write mode to %s: %s", path, strerror(errno));
        cmxd_safe_fclose(f, path);
        return -1;
    }
    
    if (cmxd_safe_fclose(f, path) < 0) return -1;
    
    /* Mode write debug output shown in main loop */
    return 0;
}

/* Write orientation to kernel module sysfs */
int cmxd_write_orientation(const char *orientation)
{
    char path[PATH_MAX];
    FILE *f;
    
    if (!data_config) {
        log_error("Data module not initialized");
        return -1;
    }
    
    if ((size_t)snprintf(path, sizeof(path), "%s/orientation", data_config->sysfs_path) >= sizeof(path)) {
        log_error("Path too long for orientation");
        return -1;
    }
    
    f = cmxd_safe_fopen(path, "w");
    if (!f) return -1;
    
    if (fprintf(f, "%s\n", orientation) < 0) {
        log_error("Failed to write orientation to %s: %s", path, strerror(errno));
        cmxd_safe_fclose(f, path);
        return -1;
    }
    
    if (cmxd_safe_fclose(f, path) < 0) return -1;
    
    /* Orientation write debug output shown in main loop */
    return 0;
}

/*
 * =============================================================================
 * IIO DEVICE DISCOVERY AND MANAGEMENT
 * =============================================================================
 */

/* Find IIO device for given I2C bus/address */
int cmxd_find_iio_device_for_i2c(int bus, int addr, char *device_name, size_t name_size)
{
    char path[PATH_MAX];
    char link_target[PATH_MAX];
    char i2c_name[64];
    ssize_t link_len;
    
    /* Expected I2C device name format */
    snprintf(i2c_name, sizeof(i2c_name), "%d-%04x", bus, addr);
    
    /* Search through IIO devices to find one with matching I2C device */
    for (int i = 0; i < 10; i++) {
        snprintf(path, sizeof(path), IIO_DEVICE_TEMPLATE, i);
        
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

/*
 * =============================================================================
 * IIO BUFFER MANAGEMENT
 * =============================================================================
 */

/* Ensure IIO sysfs trigger exists (create if needed, but don't manage lifecycle) */
int cmxd_ensure_iio_trigger_exists(void) {
    char path[PATH_MAX];
    int trigger_id;
    
    /* Check if any trigger already exists (0-9) */
    for (trigger_id = 0; trigger_id < 10; trigger_id++) {
        snprintf(path, sizeof(path), IIO_TRIGGER_TEMPLATE, trigger_id);
        if (access(path, F_OK) == 0) {
            log_debug("Using existing trigger: sysfstrig%d", trigger_id);
            return 0;  /* Found existing trigger */
        }
    }
    
    /* No trigger exists, create trigger0 */
    FILE *fp = fopen(IIO_SYSFS_TRIGGER_ADD_PATH, "w");
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
    snprintf(path, sizeof(path), IIO_TRIGGER0_PATH);
    if (access(path, F_OK) == 0) {
        log_info("Created persistent IIO trigger: sysfstrig0");
        return 0;
    } else {
        log_error("Trigger creation failed - trigger0 not found");
        return -1;
    }
}

/* Setup IIO buffer for a device */
int cmxd_setup_iio_buffer(struct iio_buffer *buf, const char *device_name) {
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
        snprintf(path, sizeof(path), IIO_TRIGGER_TEMPLATE, trigger_id);
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
    snprintf(path, sizeof(path), IIO_SCAN_ACCEL_X_INDEX_TEMPLATE, device_name);
    fp = fopen(path, "r");
    if (!fp || fscanf(fp, "%d", &buf->x_index) != 1) {
        log_error("Failed to read X index for %s", device_name);
        if (fp) fclose(fp);
        return -1;
    }
    fclose(fp);
    
    snprintf(path, sizeof(path), IIO_SCAN_ACCEL_Y_INDEX_TEMPLATE, device_name);
    fp = fopen(path, "r");
    if (!fp || fscanf(fp, "%d", &buf->y_index) != 1) {
        log_error("Failed to read Y index for %s", device_name);
        if (fp) fclose(fp);
        return -1;
    }
    fclose(fp);
    
    snprintf(path, sizeof(path), IIO_SCAN_ACCEL_Z_INDEX_TEMPLATE, device_name);
    fp = fopen(path, "r");
    if (!fp || fscanf(fp, "%d", &buf->z_index) != 1) {
        log_error("Failed to read Z index for %s", device_name);
        if (fp) fclose(fp);
        return -1;
    }
    fclose(fp);
    
    snprintf(path, sizeof(path), IIO_SCAN_TIMESTAMP_INDEX_TEMPLATE, device_name);
    fp = fopen(path, "r");
    if (!fp || fscanf(fp, "%d", &buf->timestamp_index) != 1) {
        log_error("Failed to read timestamp index for %s", device_name);
        if (fp) fclose(fp);
        return -1;
    }
    fclose(fp);
    
    /* Enable scan elements */
    snprintf(path, sizeof(path), IIO_SCAN_ACCEL_X_EN_TEMPLATE, device_name);
    fp = fopen(path, "w");
    if (!fp || fprintf(fp, "1") < 0) {
        log_error("Failed to enable X scan element for %s", device_name);
        if (fp) fclose(fp);
        return -1;
    }
    fclose(fp);
    
    snprintf(path, sizeof(path), IIO_SCAN_ACCEL_Y_EN_TEMPLATE, device_name);
    fp = fopen(path, "w");
    if (!fp || fprintf(fp, "1") < 0) {
        log_error("Failed to enable Y scan element for %s", device_name);
        if (fp) fclose(fp);
        return -1;
    }
    fclose(fp);
    
    snprintf(path, sizeof(path), IIO_SCAN_ACCEL_Z_EN_TEMPLATE, device_name);
    fp = fopen(path, "w");
    if (!fp || fprintf(fp, "1") < 0) {
        log_error("Failed to enable Z scan element for %s", device_name);
        if (fp) fclose(fp);
        return -1;
    }
    fclose(fp);
    
    snprintf(path, sizeof(path), IIO_SCAN_TIMESTAMP_EN_TEMPLATE, device_name);
    fp = fopen(path, "w");
    if (!fp || fprintf(fp, "1") < 0) {
        log_error("Failed to enable timestamp scan element for %s", device_name);
        if (fp) fclose(fp);
        return -1;
    }
    fclose(fp);
    
    /* Set current trigger */
    snprintf(path, sizeof(path), IIO_TRIGGER_CURRENT_TEMPLATE, device_name);
    fp = fopen(path, "w");
    if (!fp || fprintf(fp, "%s", buf->trigger_name) < 0) {
        log_error("Failed to set trigger for %s", device_name);
        if (fp) fclose(fp);
        return -1;
    }
    fclose(fp);
    
    /* Enable buffer */
    snprintf(path, sizeof(path), IIO_BUFFER_ENABLE_TEMPLATE, device_name);
    fp = fopen(path, "w");
    if (!fp || fprintf(fp, "1") < 0) {
        log_error("Failed to enable buffer for %s", device_name);
        if (fp) fclose(fp);
        return -1;
    }
    fclose(fp);
    
    /* Open buffer for reading */
    snprintf(path, sizeof(path), IIO_DEV_CHAR_TEMPLATE, device_name);
    buf->buffer_fd = open(path, O_RDONLY | O_NONBLOCK);
    if (buf->buffer_fd < 0) {
        if (errno == ENOENT) {
            log_error("Device file %s does not exist", path);
            log_error("The cmx kernel module may not be loaded or compiled into the kernel");
            log_error("Check if the module provides IIO devices:");
            log_error("  ls -la %s", IIO_DEVICES_LIST_MSG);
            log_error("  %s", IIO_DEV_LIST_CMD);
            log_error("If no devices exist, try loading the module: sudo modprobe cmx");
        } else if (errno == EACCES) {
            log_error("Permission denied accessing %s", path);
            log_error("Try running as root or check device permissions");
        } else {
            log_error("Failed to open buffer for %s: %s", device_name, strerror(errno));
        }
        return -1;
    }
    
    /* Open trigger for control (optional) */
    buf->trigger_fd = -1;  /* We don't need to control the sysfs trigger directly */
    
    buf->sample_size = 16; /* 3 * 2 bytes + 8 bytes timestamp + padding */
    buf->enabled = 1;
    
    log_debug("IIO buffer setup complete for %s", device_name);
    return 0;
}

/* Read sample from IIO buffer */
int cmxd_read_iio_buffer_sample(struct iio_buffer *buf, struct accel_sample *sample) {
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
    sample->x = cmxd_parse_accel_value(&buffer[buf->x_index * 2]);
    sample->y = cmxd_parse_accel_value(&buffer[buf->y_index * 2]);
    sample->z = cmxd_parse_accel_value(&buffer[buf->z_index * 2]);
    
    /* Parse timestamp (little-endian 64-bit) */
    sample->timestamp = le64toh(*(uint64_t*)&buffer[buf->timestamp_index * 2]);
    
    return 1; /* Sample read successfully */
}

/* Cleanup IIO buffer */
void cmxd_cleanup_iio_buffer(struct iio_buffer *buf) {
    char path[PATH_MAX];
    FILE *fp;
    
    if (!buf->enabled) {
        return;
    }
    
    /* Disable buffer */
    snprintf(path, sizeof(path), IIO_BUFFER_ENABLE_TEMPLATE, buf->device_name);
    fp = fopen(path, "w");
    if (fp) {
        fprintf(fp, "0");
        fclose(fp);
    }
    
    /* Clear trigger */
    snprintf(path, sizeof(path), IIO_TRIGGER_CURRENT_TEMPLATE, buf->device_name);
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

/* Parse accelerometer value from buffer data (big-endian 12-bit) */
int cmxd_parse_accel_value(const uint8_t *data) {
    uint16_t raw = be16toh(*(uint16_t*)data);
    raw >>= 4; /* Right shift by 4 bits for be:s12/16>>4 format */
    
    /* Sign extend from 12-bit to 16-bit */
    if (raw & 0x800) {
        raw |= 0xF000;
    }
    
    return (int16_t)raw;
}

/*
 * =============================================================================
 * UTILITY FUNCTIONS
 * =============================================================================
 */

/* Wait for a path to exist */
int cmxd_wait_for_path(const char *path, int timeout_sec)
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

/* Validate that paths exist and are accessible */
int cmxd_validate_paths(const char *base_dev, const char *lid_dev)
{
    char path[PATH_MAX];
    struct stat st;
    
    if (!data_config) {
        log_error("Data module not initialized");
        return -1;
    }
    
    /* Check IIO devices */
    snprintf(path, sizeof(path), IIO_DEVICE_PATH_TEMPLATE, base_dev);
    if (stat(path, &st) < 0) {
        log_error("Base IIO device not found: %s", path);
        return -1;
    }
    
    snprintf(path, sizeof(path), IIO_DEVICE_PATH_TEMPLATE, lid_dev);
    if (stat(path, &st) < 0) {
        log_error("Lid IIO device not found: %s", path);
        return -1;
    }
    
    /* Check kernel module sysfs */
    if (stat(data_config->sysfs_path, &st) < 0) {
        log_error("Kernel module sysfs not found: %s", data_config->sysfs_path);
        return -1;
    }
    
    return 0;
}

/* Read device assignments from kernel module sysfs */
int cmxd_read_kernel_device_assignments(char *base_dev, size_t base_size, 
                                        char *lid_dev, size_t lid_size)
{
    char path[PATH_MAX];
    FILE *fp;
    char device_info[64];
    int bus;
    unsigned int addr;
    
    if (!data_config) {
        log_error("Data module not initialized");
        return -1;
    }
    
    log_info("Reading device assignments from kernel module...");
    
    /* Read base device assignment */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"
    snprintf(path, sizeof(path), "%s/iio_base_device", data_config->sysfs_path);
#pragma GCC diagnostic pop
    fp = cmxd_safe_fopen(path, "r");
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
            strncpy(base_dev, device_info, base_size - 1);
            base_dev[base_size - 1] = '\0';
            log_info("Base device from kernel: %s", base_dev);
        } else if (sscanf(device_info, "i2c-%d:0x%x", &bus, &addr) == 2) {
            /* Fallback: I2C info, need to find IIO device */
            if (cmxd_find_iio_device_for_i2c(bus, addr, base_dev, base_size) == 0) {
                log_info("Base device from kernel: %s (i2c %d-0x%02x)", base_dev, bus, addr);
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
    snprintf(path, sizeof(path), "%s/iio_lid_device", data_config->sysfs_path);
#pragma GCC diagnostic pop
    fp = cmxd_safe_fopen(path, "r");
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
            strncpy(lid_dev, device_info, lid_size - 1);
            lid_dev[lid_size - 1] = '\0';
            log_info("Lid device from kernel: %s", lid_dev);
        } else if (sscanf(device_info, "i2c-%d:0x%x", &bus, &addr) == 2) {
            /* Fallback: I2C info, need to find IIO device */
            if (cmxd_find_iio_device_for_i2c(bus, addr, lid_dev, lid_size) == 0) {
                log_info("Lid device from kernel: %s (i2c %d-0x%02x)", lid_dev, bus, addr);
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