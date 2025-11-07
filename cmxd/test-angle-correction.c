/*
 * Test program to validate corrected hinge angle calculations
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdarg.h>

/* Include the calculation structures and functions */
struct cmxd_accel_sample {
    int x, y, z;
};

/* Function prototypes - copy from cmxd-calculations.h */
double cmxd_calculate_magnitude(double x, double y, double z);
double cmxd_calculate_dot_product(double x1, double y1, double z1, double x2, double y2, double z2);
double cmxd_clamp(double value, double min, double max);
double cmxd_calculate_hinge_angle_360(const struct cmxd_accel_sample *base, const struct cmxd_accel_sample *lid);
int detect_gravity_orientation(double x, double y, double z);
void transform_to_standard_frame(double x, double y, double z, int gravity_orientation,
                               double *out_x, double *out_y, double *out_z);

/* Simple implementations for testing */
double cmxd_calculate_magnitude(double x, double y, double z) {
    return sqrt(x*x + y*y + z*z);
}

double cmxd_calculate_dot_product(double x1, double y1, double z1, double x2, double y2, double z2) {
    return x1*x2 + y1*y2 + z1*z2;
}

double cmxd_clamp(double value, double min, double max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

int detect_gravity_orientation(double x, double y, double z) {
    double abs_x = fabs(x), abs_y = fabs(y), abs_z = fabs(z);
    
    if (abs_z > abs_x && abs_z > abs_y) {
        return (z > 0) ? 5 : 4;  /* Z_UP : Z_DOWN */
    } else if (abs_x > abs_y) {
        return (x > 0) ? 1 : 0;  /* X_UP : X_DOWN */
    } else {
        return (y > 0) ? 3 : 2;  /* Y_UP : Y_DOWN */
    }
}

void debug_log(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    printf("DEBUG: ");
    vprintf(fmt, args);
    printf("\n");
    va_end(args);
}

/* The orientation-independent hinge angle calculation */
double cmxd_calculate_hinge_angle_360(const struct cmxd_accel_sample *base, const struct cmxd_accel_sample *lid)
{
    /* Convert raw accelerometer values to normalized vectors */
    double base_magnitude = cmxd_calculate_magnitude((double)base->x, (double)base->y, (double)base->z);
    double lid_magnitude = cmxd_calculate_magnitude((double)lid->x, (double)lid->y, (double)lid->z);
    
    if (base_magnitude < 1.0 || lid_magnitude < 1.0) {
        printf("DEBUG: Invalid accelerometer readings: base_mag=%.3f, lid_mag=%.3f\n", base_magnitude, lid_magnitude);
        return -1.0; /* Invalid reading */
    }
    
    /* Normalize the raw vectors (these represent gravity direction for each sensor) */
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
    
    /* Detect gravity orientation for both sensors */
    int base_gravity = detect_gravity_orientation(base_norm[0], base_norm[1], base_norm[2]);
    int lid_gravity = detect_gravity_orientation(lid_norm[0], lid_norm[1], lid_norm[2]);
    
    /* Transform to normalized device coordinate system for orientation-independent calculation */
    double base_device[3], lid_device[3];
    transform_to_standard_frame(base_norm[0], base_norm[1], base_norm[2], base_gravity, 
                               &base_device[0], &base_device[1], &base_device[2]);
    transform_to_standard_frame(lid_norm[0], lid_norm[1], lid_norm[2], lid_gravity,
                               &lid_device[0], &lid_device[1], &lid_device[2]);
    
    /* Calculate the dot product between normalized device vectors */
    double dot_product = cmxd_calculate_dot_product(base_device[0], base_device[1], base_device[2],
                                                   lid_device[0], lid_device[1], lid_device[2]);
    
    /* Clamp to valid range to avoid numerical errors in acos() */
    dot_product = cmxd_clamp(dot_product, -1.0, 1.0);
    
    /* Calculate base angle from dot product (0-180°) */
    double angle = acos(fabs(dot_product)) * 180.0 / M_PI;
    
    /* Calculate cross product in device coordinates to determine hinge opening direction */
    double cross_y = base_device[2] * lid_device[0] - base_device[0] * lid_device[2];
    
    /* Use cross product Y component to determine 0-360° range */
    if (cross_y < 0) {
        angle = 360.0 - angle;
    }
    
    /* Special handling for edge cases */
    if (fabs(cross_y) < 0.05 && dot_product < -0.9) {
        angle = 180.0;
    }
    
    /* Ensure valid 0-360° range */
    if (angle < 0) angle = 0;
    if (angle > 360) angle = 360;
    
    printf("DEBUG: Orientation-independent hinge calculation: base[%d,%d,%d](grav=%d) lid[%d,%d,%d](grav=%d) -> "
             "device_base[%.3f,%.3f,%.3f] device_lid[%.3f,%.3f,%.3f] -> dot=%.3f, cross_y=%.3f, angle=%.1f°\n", 
             base->x, base->y, base->z, base_gravity, lid->x, lid->y, lid->z, lid_gravity,
             base_device[0], base_device[1], base_device[2], lid_device[0], lid_device[1], lid_device[2],
             dot_product, cross_y, angle);
    
    return angle;
}

/* Need to add the transform function */
void transform_to_standard_frame(double x, double y, double z, int gravity_orientation,
                               double *out_x, double *out_y, double *out_z) {
    /* Simple implementation for testing */
    switch(gravity_orientation) {
        case 0: /* X_DOWN */ *out_x = -1.0; *out_y = 0.0; *out_z = 0.0; break;
        case 1: /* X_UP */   *out_x = 1.0;  *out_y = 0.0; *out_z = 0.0; break;
        case 2: /* Y_DOWN */ *out_x = 0.0; *out_y = -1.0; *out_z = 0.0; break;
        case 3: /* Y_UP */   *out_x = 0.0; *out_y = 1.0;  *out_z = 0.0; break;
        case 4: /* Z_DOWN */ *out_x = 0.0; *out_y = 0.0; *out_z = -1.0; break;
        case 5: /* Z_UP */   *out_x = 0.0; *out_y = 0.0; *out_z = 1.0; break;
        default: *out_x = x; *out_y = y; *out_z = z; break;
    }
}

int main() {
    printf("Testing corrected hinge angle calculations:\n\n");
    
    /* Test case 1: The problematic data from cmxd.log */
    struct cmxd_accel_sample base1 = {42, -20, -960};
    struct cmxd_accel_sample lid1 = {969, -42, -58};
    
    printf("Test 1 - Laptop position from logs (should be ~90°):\n");
    printf("Base: %d, %d, %d  Lid: %d, %d, %d\n", base1.x, base1.y, base1.z, lid1.x, lid1.y, lid1.z);
    double angle1 = cmxd_calculate_hinge_angle_360(&base1, &lid1);
    printf("Result: %.1f°\n\n", angle1);
    
    /* Test case 2: Simulated closed position */
    struct cmxd_accel_sample base2 = {0, 0, -1000};  /* Base flat, Z down */
    struct cmxd_accel_sample lid2 = {0, 0, -1000};   /* Lid also flat, Z down */
    
    printf("Test 2 - Closed position (should be ~0°):\n");
    printf("Base: %d, %d, %d  Lid: %d, %d, %d\n", base2.x, base2.y, base2.z, lid2.x, lid2.y, lid2.z);
    double angle2 = cmxd_calculate_hinge_angle_360(&base2, &lid2);
    printf("Result: %.1f°\n\n", angle2);
    
    /* Test case 3: Simulated flat position */
    struct cmxd_accel_sample base3 = {0, 0, -1000};  /* Base flat, Z down */
    struct cmxd_accel_sample lid3 = {0, 0, 1000};    /* Lid flat, Z up */
    
    printf("Test 3 - Flat position (should be ~180°):\n");
    printf("Base: %d, %d, %d  Lid: %d, %d, %d\n", base3.x, base3.y, base3.z, lid3.x, lid3.y, lid3.z);
    double angle3 = cmxd_calculate_hinge_angle_360(&base3, &lid3);
    printf("Result: %.1f°\n\n", angle3);
    
    /* Test case 4: Simulated laptop position */
    struct cmxd_accel_sample base4 = {0, 0, -1000};  /* Base flat, Z down */
    struct cmxd_accel_sample lid4 = {-1000, 0, 0};   /* Lid vertical, X down */
    
    printf("Test 4 - Ideal laptop position (should be ~90°):\n");
    printf("Base: %d, %d, %d  Lid: %d, %d, %d\n", base4.x, base4.y, base4.z, lid4.x, lid4.y, lid4.z);
    double angle4 = cmxd_calculate_hinge_angle_360(&base4, &lid4);
    printf("Result: %.1f°\n\n", angle4);
    
    return 0;
}