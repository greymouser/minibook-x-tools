#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <poll.h>
#include <stdint.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <math.h>
#include <endian.h>

#define DEVICE_PATH "/sys/bus/iio/devices/iio:device0"
#define BUFFER_PATH DEVICE_PATH "/buffer"
#define TRIGGER_PATH DEVICE_PATH "/trigger"
#define SCAN_ELEMENTS_PATH DEVICE_PATH "/scan_elements"

/* Parse accelerometer data from IIO buffer 
 * Format: be:s12/16>>4 means big-endian signed 12-bit in 16-bit container, right-shifted by 4
 */
static int16_t parse_accel_value(uint16_t raw_be) {
    /* Convert from big-endian to host byte order */
    uint16_t raw_host = be16toh(raw_be);
    
    /* The 12-bit value is right-shifted by 4 bits, so left-shift to restore */
    int16_t value = (int16_t)(raw_host << 4);
    
    /* Since we left-shifted by 4, we now need to arithmetic right-shift by 4 
     * to get the proper signed 12-bit value extended to 16-bit */
    value = value >> 4;
    
    return value;
}

static volatile int running = 1;

static void signal_handler(int sig) {
    (void)sig; /* Suppress unused parameter warning */
    running = 0;
}

/* Write a string to a sysfs file */
static int write_sysfs_string(const char *path, const char *value) {
    FILE *fp = fopen(path, "w");
    if (!fp) {
        fprintf(stderr, "Failed to open %s: %s\n", path, strerror(errno));
        return -1;
    }
    
    if (fprintf(fp, "%s", value) < 0) {
        fprintf(stderr, "Failed to write to %s: %s\n", path, strerror(errno));
        fclose(fp);
        return -1;
    }
    
    fclose(fp);
    return 0;
}

/* Read a string from a sysfs file */
static int read_sysfs_string(const char *path, char *buffer, size_t size) {
    FILE *fp = fopen(path, "r");
    if (!fp) {
        fprintf(stderr, "Failed to open %s: %s\n", path, strerror(errno));
        return -1;
    }
    
    if (!fgets(buffer, size, fp)) {
        fprintf(stderr, "Failed to read from %s: %s\n", path, strerror(errno));
        fclose(fp);
        return -1;
    }
    
    fclose(fp);
    
    /* Remove trailing newline */
    char *newline = strchr(buffer, '\n');
    if (newline) *newline = '\0';
    
    return 0;
}

/* Setup IIO buffered reading */
static int setup_iio_buffer(void) {
    char path[256];
    
    printf("Setting up IIO buffer for accelerometer data...\n");
    
    /* Create a sysfs trigger */
    if (write_sysfs_string("/sys/bus/iio/devices/iio_sysfs_trigger/add_trigger", "0") < 0) {
        fprintf(stderr, "Warning: Could not create trigger (may already exist)\n");
    }
    
    /* Set the trigger for our device */
    if (write_sysfs_string(TRIGGER_PATH "/current_trigger", "sysfstrig0") < 0) {
        fprintf(stderr, "Failed to set trigger\n");
        return -1;
    }
    
    /* Enable X, Y, Z accelerometer channels */
    snprintf(path, sizeof(path), "%s/in_accel_x_en", SCAN_ELEMENTS_PATH);
    if (write_sysfs_string(path, "1") < 0) return -1;
    
    snprintf(path, sizeof(path), "%s/in_accel_y_en", SCAN_ELEMENTS_PATH);
    if (write_sysfs_string(path, "1") < 0) return -1;
    
    snprintf(path, sizeof(path), "%s/in_accel_z_en", SCAN_ELEMENTS_PATH);
    if (write_sysfs_string(path, "1") < 0) return -1;
    
    /* Enable timestamp */
    snprintf(path, sizeof(path), "%s/in_timestamp_en", SCAN_ELEMENTS_PATH);
    if (write_sysfs_string(path, "1") < 0) return -1;
    
    /* Set buffer length */
    snprintf(path, sizeof(path), "%s/length", BUFFER_PATH);
    if (write_sysfs_string(path, "128") < 0) return -1;
    
    /* Enable the buffer */
    snprintf(path, sizeof(path), "%s/enable", BUFFER_PATH);
    if (write_sysfs_string(path, "1") < 0) return -1;
    
    printf("✓ IIO buffer configured successfully\n");
    return 0;
}

/* Cleanup IIO buffer */
static void cleanup_iio_buffer(void) {
    char path[256];
    
    printf("\nCleaning up IIO buffer...\n");
    
    /* Disable the buffer */
    snprintf(path, sizeof(path), "%s/enable", BUFFER_PATH);
    write_sysfs_string(path, "0");
    
    /* Remove trigger */
    write_sysfs_string(TRIGGER_PATH "/current_trigger", "");
    
    /* Disable scan elements */
    snprintf(path, sizeof(path), "%s/in_accel_x_en", SCAN_ELEMENTS_PATH);
    write_sysfs_string(path, "0");
    snprintf(path, sizeof(path), "%s/in_accel_y_en", SCAN_ELEMENTS_PATH);
    write_sysfs_string(path, "0");
    snprintf(path, sizeof(path), "%s/in_accel_z_en", SCAN_ELEMENTS_PATH);
    write_sysfs_string(path, "0");
    snprintf(path, sizeof(path), "%s/in_timestamp_en", SCAN_ELEMENTS_PATH);
    write_sysfs_string(path, "0");
    
    printf("✓ IIO buffer cleaned up\n");
}

/* Clear terminal and position cursor */
static void clear_screen(void) {
    printf("\033[2J\033[H");
}

/* Move cursor to specific line */
static void move_cursor(int line) {
    printf("\033[%d;1H", line);
}

/* Main event loop */
static int run_event_loop(void) {
    int dev_fd;
    char dev_path[256];
    struct pollfd pfd;
    uint8_t buffer[1024];
    
    /* Open the device file for reading */
    snprintf(dev_path, sizeof(dev_path), "/dev/iio:device0");
    dev_fd = open(dev_path, O_RDONLY | O_NONBLOCK);
    if (dev_fd < 0) {
        fprintf(stderr, "Failed to open %s: %s\n", dev_path, strerror(errno));
        return -1;
    }
    
    pfd.fd = dev_fd;
    pfd.events = POLLIN;
    
    clear_screen();
    printf("=== IIO Event-Driven Accelerometer Test ===\n");
    printf("Reading from iio:device0 (lid accelerometer)\n");
    printf("Press Ctrl+C to stop\n\n");
    printf("Waiting for trigger events...\n");
    
    int sample_count = 0;
    
    while (running) {
        /* Trigger a reading by writing to the sysfs trigger */
        if (write_sysfs_string("/sys/bus/iio/devices/trigger0/trigger_now", "1") < 0) {
            /* Try alternative trigger path */
            write_sysfs_string("/sys/bus/iio/devices/iio_sysfs_trigger/trigger_now", "1");
        }
        
        /* Wait for data to be available */
        int ret = poll(&pfd, 1, 100); /* 100ms timeout */
        
        if (ret > 0 && (pfd.revents & POLLIN)) {
            /* Read data from buffer */
            ssize_t bytes_read = read(dev_fd, buffer, sizeof(buffer));
            
            if (bytes_read > 0) {
                /* Each sample contains: X(2 bytes) + Y(2 bytes) + Z(2 bytes) + timestamp(8 bytes) = 14 bytes
                 * But may be padded to alignment boundaries */
                int sample_size = 2 + 2 + 2 + 8; /* 14 bytes per sample */
                int num_samples = bytes_read / sample_size;
                
                for (int i = 0; i < num_samples; i++) {
                    uint8_t *sample_data = buffer + (i * sample_size);
                    
                    /* Parse accelerometer values (big-endian 12-bit in 16-bit containers) */
                    uint16_t x_raw = *(uint16_t*)(sample_data + 0);
                    uint16_t y_raw = *(uint16_t*)(sample_data + 2); 
                    uint16_t z_raw = *(uint16_t*)(sample_data + 4);
                    
                    /* Convert to properly signed values */
                    int16_t x = parse_accel_value(x_raw);
                    int16_t y = parse_accel_value(y_raw);
                    int16_t z = parse_accel_value(z_raw);
                    
                    /* Parse timestamp (little-endian 64-bit) */
                    int64_t timestamp = *(int64_t*)(sample_data + 6);
                    timestamp = le64toh(timestamp);
                    
                    /* Update display in place */
                    move_cursor(6);
                    printf("Sample #%06d: (bytes_read: %zd)                     \n", ++sample_count, bytes_read);
                    printf("  X: %6d   Y: %6d   Z: %6d                     \n", x, y, z);
                    printf("  |X|: %4d   |Y|: %4d   |Z|: %4d                \n", 
                           abs(x), abs(y), abs(z));
                    
                    /* Calculate approximate tilt angle */
                    double angle = atan2(sqrt(x*x + y*y), abs(z)) * 180.0 / 3.14159;
                    printf("  Tilt angle: %.1f°                              \n", angle);
                    printf("  Timestamp: %lld                               \n", (long long)timestamp);
                    
                    fflush(stdout);
                }
            }
        } else if (ret == 0) {
            /* Timeout - normal when no new data */
            usleep(50000); /* 50ms sleep */
        } else {
            fprintf(stderr, "Poll error: %s\n", strerror(errno));
            break;
        }
    }
    
    close(dev_fd);
    return 0;
}

int main(int argc, char **argv) {
    (void)argc; /* Suppress unused parameter warning */
    
    /* Setup signal handling */
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    printf("IIO Event-Based Accelerometer Test\n");
    printf("===================================\n\n");
    
    /* Check if running as root */
    if (geteuid() != 0) {
        fprintf(stderr, "This program requires root privileges to access IIO devices.\n");
        fprintf(stderr, "Please run with: sudo %s\n", argv[0]);
        return 1;
    }
    
    /* Setup IIO buffer */
    if (setup_iio_buffer() < 0) {
        fprintf(stderr, "Failed to setup IIO buffer\n");
        return 1;
    }
    
    /* Run the event loop */
    int ret = run_event_loop();
    
    /* Cleanup */
    cleanup_iio_buffer();
    
    clear_screen();
    printf("IIO event test completed.\n");
    
    return ret;
}