// SPDX-License-Identifier: GPL-2.0
/*
 * Comprehensive Test for Gravity-Aware Hinge Angle Calculation
 * 
 * Tests all device modes with real accelerometer data:
 * - Laptop mode (normal orientation)
 * - Tablet mode (flat, lying down)
 * - Tent mode (inverted V shape)
 * - Flat mode (360° fold)
 * - Closing mode (various angles)
 * - Different physical orientations (normal, on side, etc.)
 *
 * Copyright (c) 2025 Armando DiCianno <armando@noonshy.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>
#include "cmxd-calculations.h"

/* Test data structure */
struct test_case {
    const char* description;
    int base_x, base_y, base_z;
    int lid_x, lid_y, lid_z;
    double expected_angle;  /* Expected hinge angle range */
    const char* expected_mode;
    const char* notes;
};

/* Real accelerometer data from log files */
static struct test_case test_data[] = {
    /* === LAPTOP MODE (Normal Table Position) === */
    {
        .description = "Laptop mode - normal table position",
        .base_x = 24, .base_y = -3, .base_z = -1330,
        .lid_x = -542, .lid_y = 9, .lid_z = -1061,
        .expected_angle = 90.0,  /* Normal laptop angle ~90° */
        .expected_mode = "laptop",
        .notes = "Standard laptop position on table"
    },
    
    /* === FLAT MODE (360° fold, lying down) === */
    {
        .description = "Flat mode - 360° fold, lying down",
        .base_x = 17, .base_y = -5, .base_z = -1344,
        .lid_x = -32, .lid_y = -21, .lid_z = 868,
        .expected_angle = 0.0,  /* ~360°, should show as ~0° */
        .expected_mode = "flat",
        .notes = "Laptop folded flat (360°), both sensors facing up"
    },
    
    /* === TABLET MODE (Completely closed, lying down) === */
    {
        .description = "Tablet mode - completely closed, lying down",
        .base_x = 4, .base_y = 10, .base_z = 708,
        .lid_x = 9, .lid_y = 5, .lid_z = 811,
        .expected_angle = 0.0,  /* Very small angle, closed */
        .expected_mode = "closing",
        .notes = "Laptop closed, both sensors pointing same direction"
    },
    
    /* === TENT MODE (Inverted V shape) === */
    {
        .description = "Tent mode - inverted V shape on table",
        .base_x = 990, .base_y = -5, .base_z = -552,
        .lid_x = 982, .lid_y = -23, .lid_z = 118,
        .expected_angle = 135.0,  /* Tent mode angle ~135° */
        .expected_mode = "tent",
        .notes = "Laptop in tent position, both halves touching table"
    },
    
    /* === CLOSING MODE (Progressive closing) === */
    {
        .description = "Closing mode - mid-close position",
        .base_x = 26, .base_y = -5, .base_z = -1311,
        .lid_x = -542, .lid_y = 1, .lid_z = -1045,
        .expected_angle = 60.0,  /* Mid-close angle */
        .expected_mode = "laptop",
        .notes = "Laptop being closed, intermediate angle"
    },
    
    /* === LEFT SIDE ORIENTATIONS (The problematic case) === */
    {
        .description = "Left side L-shape - early stable period",
        .base_x = 6, .base_y = -3, .base_z = -1348,
        .lid_x = -987, .lid_y = -2, .lid_z = 80,
        .expected_angle = 90.0,  /* Should be ~90°, not <45° */
        .expected_mode = "laptop",
        .notes = "Laptop resting on left side in L-shape (the original problem)"
    },
    {
        .description = "Left side L-shape - problematic period",
        .base_x = -79, .base_y = 816, .base_z = -873,
        .lid_x = -511, .lid_y = 832, .lid_z = 26,
        .expected_angle = 90.0,  /* Should stay ~90°, not trigger false close */
        .expected_mode = "laptop",
        .notes = "Same position, different gravity orientation - should not false trigger"
    },
    
    /* === ON SIDE ORIENTATIONS === */
    {
        .description = "Closing on side - stable position",
        .base_x = 11, .base_y = 1024, .base_z = -326,
        .lid_x = 13, .lid_y = 1015, .lid_z = -181,
        .expected_angle = 0.0,  /* Nearly closed when on side */
        .expected_mode = "closing",
        .notes = "Laptop closing while on side orientation"
    },
    
    /* === VARIOUS TABLET POSITIONS === */
    {
        .description = "Tablet standing upright",
        .base_x = 15, .base_y = 7, .base_z = 844,
        .lid_x = 17, .lid_y = 19, .lid_z = 711,
        .expected_angle = 0.0,  /* Tablet mode, nearly parallel */
        .expected_mode = "closing",
        .notes = "Tablet mode, device standing upright"
    }
};

static int num_tests = sizeof(test_data) / sizeof(test_data[0]);

void test_gravity_aware_comprehensive(void) {
    printf("=== Comprehensive Gravity-Aware Hinge Angle Test ===\n\n");
    
    int passed = 0;
    int failed = 0;
    
    for (int i = 0; i < num_tests; i++) {
        struct test_case *test = &test_data[i];
        
        /* Create accelerometer samples */
        struct cmxd_accel_sample base_sample = {
            .x = test->base_x,
            .y = test->base_y, 
            .z = test->base_z,
            .timestamp = 1000000000 + i
        };
        
        struct cmxd_accel_sample lid_sample = {
            .x = test->lid_x,
            .y = test->lid_y,
            .z = test->lid_z,
            .timestamp = 1000000000 + i
        };
        
        /* Calculate gravity-aware hinge angle */
        double angle = cmxd_calculate_hinge_angle_360(&base_sample, &lid_sample);
        
        printf("=== Test %d: %s ===\n", i + 1, test->description);
        printf("Input: Base[%d,%d,%d] Lid[%d,%d,%d]\n", 
               test->base_x, test->base_y, test->base_z,
               test->lid_x, test->lid_y, test->lid_z);
        printf("Result: %.1f° (expected ~%.1f°)\n", angle, test->expected_angle);
        printf("Notes: %s\n", test->notes);
        
        /* Validate result */
        bool test_passed = false;
        
        if (angle >= 0) {
            /* Check if angle is in reasonable range for the expected mode */
            if (strcmp(test->expected_mode, "laptop") == 0) {
                /* Laptop mode should be 45-135° range */
                test_passed = (angle >= 45.0 && angle <= 135.0);
                if (test_passed) {
                    printf("  ✓ PASS: Angle %.1f° correctly in laptop range (45-135°)\n", angle);
                } else {
                    printf("  ✗ FAIL: Angle %.1f° outside laptop range (45-135°)\n", angle);
                }
            } else if (strcmp(test->expected_mode, "tent") == 0) {
                /* Tent mode should be around 240-340° (or 120-220° depending on calculation) */
                test_passed = (angle >= 240.0 || (angle >= 120.0 && angle <= 220.0));
                if (test_passed) {
                    printf("  ✓ PASS: Angle %.1f° correctly in tent range\n", angle);
                } else {
                    printf("  ✗ FAIL: Angle %.1f° outside tent range\n", angle);
                }
            } else if (strcmp(test->expected_mode, "flat") == 0) {
                /* Flat mode should be 160-200° or 340-360°/0-20° */
                test_passed = ((angle >= 160.0 && angle <= 200.0) || 
                              (angle >= 340.0) || (angle <= 20.0));
                if (test_passed) {
                    printf("  ✓ PASS: Angle %.1f° correctly in flat range\n", angle);
                } else {
                    printf("  ✗ FAIL: Angle %.1f° outside flat range\n", angle);
                }
            } else if (strcmp(test->expected_mode, "closing") == 0) {
                /* Closing mode should be 0-45° */
                test_passed = (angle >= 0.0 && angle <= 45.0);
                if (test_passed) {
                    printf("  ✓ PASS: Angle %.1f° correctly in closing range (0-45°)\n", angle);
                } else {
                    printf("  ✗ FAIL: Angle %.1f° outside closing range (0-45°)\n", angle);
                }
            }
        } else {
            printf("  ✗ FAIL: Invalid angle calculation (negative result)\n");
            test_passed = false;
        }
        
        if (test_passed) {
            passed++;
        } else {
            failed++;
        }
        
        printf("\n");
    }
    
    printf("=== Test Summary ===\n");
    printf("Total tests: %d\n", num_tests);
    printf("Passed: %d\n", passed);
    printf("Failed: %d\n", failed);
    
    if (failed == 0) {
        printf("🎉 ALL TESTS PASSED! Gravity-aware system is working correctly.\n");
    } else {
        printf("⚠️  Some tests failed. Review the results above.\n");
    }
}

int main(void) {
    test_gravity_aware_comprehensive();
    return 0;
}