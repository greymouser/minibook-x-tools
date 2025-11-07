// Quick integration test for gravity-aware cmxd
#include <stdio.h>
#include "cmxd-calculations.h"

int main(void) {
    printf("=== Testing Gravity-Aware Integration ===\n");
    
    // Test the problematic left-side L-shape case
    struct cmxd_accel_sample base_sample = {
        .x = -79, .y = 816, .z = -873, .timestamp = 1000000000
    };
    struct cmxd_accel_sample lid_sample = {
        .x = -511, .y = 832, .z = 26, .timestamp = 1000000000
    };
    
    printf("Testing left-side L-shape problematic case:\n");
    printf("Base[%d,%d,%d] Lid[%d,%d,%d]\n", 
           base_sample.x, base_sample.y, base_sample.z,
           lid_sample.x, lid_sample.y, lid_sample.z);
    
    double angle = cmxd_calculate_hinge_angle_360(&base_sample, &lid_sample);
    printf("Gravity-aware hinge angle: %.1f°\n", angle);
    
    if (angle >= 45.0 && angle <= 145.0) {
        printf("✅ SUCCESS: Angle %.1f° is in laptop range (45-145°)\n", angle);
        printf("✅ The left-side L-shape issue is FIXED!\n");
        printf("✅ Gravity-aware system is working in cmxd!\n");
        return 0;
    } else {
        printf("❌ FAIL: Angle %.1f° is outside laptop range\n", angle);
        return 1;
    }
}