/* Test the fixed 360° angle calculation */

#include "cmxd-calculations.h"
#include <stdio.h>

void test_360_angle_progression() {
    // Test progression from laptop → flat → tent → tablet
    
    // Laptop mode (~95°): base pointing down, lid pointing up
    struct cmxd_accel_sample base_laptop = {991, 2, -1346};     // base_z = -12.9 (down)
    struct cmxd_accel_sample lid_laptop = {-938, -38, 254};     // lid_z = +2.4 (up)
    
    // Flat mode (~145°): base pointing down, lid pointing up more
    struct cmxd_accel_sample base_flat = {957, -12, -611};      // base_z = -5.9 (down) 
    struct cmxd_accel_sample lid_flat = {-254, -10, 766};       // lid_z = +7.3 (up)
    
    // Tent mode (should be ~220°): base pointing down, lid pointing down (folded back)
    struct cmxd_accel_sample base_tent = {991, 2, -1346};       // base_z = -12.9 (down)
    struct cmxd_accel_sample lid_tent = {938, 38, -254};        // lid_z = -2.4 (down) - flipped from laptop
    
    // Tablet mode (should be ~270°): base pointing down, lid pointing strongly down
    struct cmxd_accel_sample base_tablet = {991, 2, -1346};     // base_z = -12.9 (down)
    struct cmxd_accel_sample lid_tablet = {254, 10, -766};      // lid_z = -7.3 (down) - flipped from flat
    
    double scale = 0.009582;
    
    printf("Testing 360° angle calculation fix:\n\n");
    
    double laptop_angle = cmxd_calculate_hinge_angle_360(&base_laptop, &lid_laptop, scale, scale);
    printf("Laptop mode: %.1f° (expected ~95°)\n", laptop_angle);
    
    double flat_angle = cmxd_calculate_hinge_angle_360(&base_flat, &lid_flat, scale, scale);
    printf("Flat mode: %.1f° (expected ~145°)\n", flat_angle);
    
    double tent_angle = cmxd_calculate_hinge_angle_360(&base_tent, &lid_tent, scale, scale);
    printf("Tent mode: %.1f° (expected ~220°)\n", tent_angle);
    
    double tablet_angle = cmxd_calculate_hinge_angle_360(&base_tablet, &lid_tablet, scale, scale);
    printf("Tablet mode: %.1f° (expected ~270°)\n", tablet_angle);
    
    printf("\nMode classification with corrected boundaries:\n");
    const char* modes[] = {"laptop", "flat", "tent", "tablet"};
    double angles[] = {laptop_angle, flat_angle, tent_angle, tablet_angle};
    
    for (int i = 0; i < 4; i++) {
        const char* mode;
        if (angles[i] < 45) mode = "closing";
        else if (angles[i] < 160) mode = "laptop";
        else if (angles[i] < 240) mode = "flat";
        else if (angles[i] < 330) mode = "tent";
        else mode = "tablet";
        
        printf("%s: %.1f° -> %s mode\n", modes[i], angles[i], mode);
    }
}

int main() {
    // Set up debug logging (stub)
    cmxd_calculations_set_log_debug(NULL);
    
    test_360_angle_progression();
    return 0;
}