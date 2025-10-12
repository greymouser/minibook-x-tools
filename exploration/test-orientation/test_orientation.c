#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <stdint.h>

/* Simple test program to check actual orientation codes */
static int get_device_orientation(double x, double y, double z) {
    double abs_x = fabs(x);
    double abs_y = fabs(y);
    double abs_z = fabs(z);
    
    printf("Raw: X=%.1f Y=%.1f Z=%.1f | Abs: X=%.1f Y=%.1f Z=%.1f | ", 
           x, y, z, abs_x, abs_y, abs_z);
    
    /* Find the axis with the largest magnitude (closest to gravity) */
    if (abs_z > abs_x && abs_z > abs_y) {
        int code = (z > 0) ? 2 : 5;
        printf("Z dominant -> code %d\n", code);
        return code;
    } else if (abs_y > abs_x) {
        int code = (y > 0) ? 1 : 4;
        printf("Y dominant -> code %d\n", code);
        return code;
    } else {
        int code = (x > 0) ? 0 : 3;
        printf("X dominant -> code %d\n", code);
        return code;
    }
}

int main() {
    int fd;
    char buffer[32];
    int16_t x, y, z;
    
    /* Open lid sensor */
    fd = open("/sys/bus/iio/devices/iio:device0/in_accel_x_raw", O_RDONLY);
    if (fd < 0) {
        perror("Failed to open lid X sensor");
        return 1;
    }
    close(fd);
    
    printf("Testing lid sensor orientation detection...\n");
    printf("Rotate your laptop and watch the orientation codes:\n");
    printf("Expected: 0=X-up, 1=Y-up, 2=Z-up, 3=X-down, 4=Y-down, 5=Z-down\n\n");
    
    for (int i = 0; i < 50; i++) {
        /* Read X */
        fd = open("/sys/bus/iio/devices/iio:device0/in_accel_x_raw", O_RDONLY);
        read(fd, buffer, sizeof(buffer));
        x = atoi(buffer);
        close(fd);
        
        /* Read Y */
        fd = open("/sys/bus/iio/devices/iio:device0/in_accel_y_raw", O_RDONLY);
        read(fd, buffer, sizeof(buffer));
        y = atoi(buffer);
        close(fd);
        
        /* Read Z */
        fd = open("/sys/bus/iio/devices/iio:device0/in_accel_z_raw", O_RDONLY);
        read(fd, buffer, sizeof(buffer));
        z = atoi(buffer);
        close(fd);
        
        printf("Sample %2d: ", i+1);
        get_device_orientation((double)x, (double)y, (double)z);
        
        usleep(500000); /* 0.5 second delay */
    }
    
    return 0;
}