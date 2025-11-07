/*
 * Test program for gravity-aware hinge angle calculation
 * Tests the new gravity-aware algorithm with the left-side orientation scenario
 */

#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include "cmxd-calculations.h"

static void log_debug(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf("\n");
}

void test_left_side_scenario(void)
{
    printf("\n=== Testing Left-Side L-Shape Scenario ===\n");
    printf("Simulating laptop in 90° L-shape resting on left side\n");
    
    /* Test data from the actual log - early stable readings */
    struct cmxd_accel_sample base_samples[] = {
        {.x = 6, .y = -3, .z = -1348},     /* Early stable reading */
        {.x = 18, .y = -3, .z = -1363},   /* Slightly different but similar */
        {.x = 25, .y = -12, .z = -1389},  /* More variation */
    };
    
    struct cmxd_accel_sample lid_samples[] = {
        {.x = -987, .y = -2, .z = 80},    /* Early stable reading */
        {.x = -991, .y = 5, .z = 87},     /* Slightly different but similar */
        {.x = -998, .y = 0, .z = -38},    /* More variation */
    };
    
    /* Test data from the problematic period */
    struct cmxd_accel_sample base_problem[] = {
        {.x = -5, .y = 744, .z = -1008},    /* When orientation changed */
        {.x = -79, .y = 816, .z = -873},    /* Further rotation */
        {.x = -132, .y = 1019, .z = -429}, /* Final problematic reading */
    };
    
    struct cmxd_accel_sample lid_problem[] = {
        {.x = -622, .y = 745, .z = -12},    /* When orientation changed */
        {.x = -511, .y = 832, .z = 26},     /* Further rotation */
        {.x = -109, .y = 1023, .z = 23},    /* Final problematic reading */
    };
    
    printf("\n--- Early Stable Period (should be ~90°) ---\n");
    for (int i = 0; i < 3; i++) {
        double angle = cmxd_calculate_hinge_angle_360(&base_samples[i], &lid_samples[i]);
        int base_grav = cmxd_detect_gravity_orientation(base_samples[i].x, base_samples[i].y, base_samples[i].z);
        int lid_grav = cmxd_detect_gravity_orientation(lid_samples[i].x, lid_samples[i].y, lid_samples[i].z);
        
        printf("Sample %d: Base[%d,%d,%d](grav=%d) Lid[%d,%d,%d](grav=%d) -> Hinge=%.1f°\n",
               i+1, base_samples[i].x, base_samples[i].y, base_samples[i].z, base_grav,
               lid_samples[i].x, lid_samples[i].y, lid_samples[i].z, lid_grav, angle);
    }
    
    printf("\n--- Problematic Period (should still be ~90°, not <45°) ---\n");
    for (int i = 0; i < 3; i++) {
        double angle = cmxd_calculate_hinge_angle_360(&base_problem[i], &lid_problem[i]);
        int base_grav = cmxd_detect_gravity_orientation(base_problem[i].x, base_problem[i].y, base_problem[i].z);
        int lid_grav = cmxd_detect_gravity_orientation(lid_problem[i].x, lid_problem[i].y, lid_problem[i].z);
        
        printf("Sample %d: Base[%d,%d,%d](grav=%d) Lid[%d,%d,%d](grav=%d) -> Hinge=%.1f°\n",
               i+1, base_problem[i].x, base_problem[i].y, base_problem[i].z, base_grav,
               lid_problem[i].x, lid_problem[i].y, lid_problem[i].z, lid_grav, angle);
        
        if (angle < 45.0) {
            printf("  ❌ PROBLEM: Angle %.1f° < 45° would trigger false 'closing' mode!\n", angle);
        } else {
            printf("  ✓ Good: Angle %.1f° would correctly stay in laptop mode\n", angle);
        }
    }
}

void test_normal_orientations(void)
{
    printf("\n=== Testing Normal Orientations ===\n");
    
    /* Normal laptop on table */
    struct cmxd_accel_sample base_normal = {.x = -1000, .y = 0, .z = 0};
    struct cmxd_accel_sample lid_90 = {.x = 0, .y = 0, .z = -1000};
    struct cmxd_accel_sample lid_180 = {.x = 1000, .y = 0, .z = 0};
    struct cmxd_accel_sample lid_270 = {.x = 0, .y = 0, .z = 1000};
    
    printf("Normal table orientation:\n");
    printf("  90°:  %.1f°\n", cmxd_calculate_hinge_angle_360(&base_normal, &lid_90));
    printf("  180°: %.1f°\n", cmxd_calculate_hinge_angle_360(&base_normal, &lid_180));
    printf("  270°: %.1f°\n", cmxd_calculate_hinge_angle_360(&base_normal, &lid_270));
}

void test_various_orientations(void)
{
    printf("\n=== Testing Various Physical Orientations ===\n");
    
    /* Test laptop on right side (opposite of left side) */
    struct cmxd_accel_sample base_right = {.x = 0, .y = 1000, .z = 0};  /* Y up */
    struct cmxd_accel_sample lid_right = {.x = 0, .y = 0, .z = -1000};  /* Z down */
    
    printf("Laptop on right side (90° L-shape): %.1f°\n", 
           cmxd_calculate_hinge_angle_360(&base_right, &lid_right));
    
    /* Test laptop upside down */
    struct cmxd_accel_sample base_upside = {.x = 1000, .y = 0, .z = 0};   /* X up */
    struct cmxd_accel_sample lid_upside = {.x = 0, .y = 0, .z = 1000};    /* Z up */
    
    printf("Laptop upside down (90° L-shape): %.1f°\n", 
           cmxd_calculate_hinge_angle_360(&base_upside, &lid_upside));
}

int main(void)
{
    printf("=== Gravity-Aware Hinge Angle Test ===\n");
    
    /* Set up debug logging */
    cmxd_calculations_set_log_debug(log_debug);
    
    test_left_side_scenario();
    test_normal_orientations();
    test_various_orientations();
    
    printf("\n=== Test Complete ===\n");
    return 0;
}