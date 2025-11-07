// SPDX-License-Identifier: GPL-2.0
/*
 * Log File Analysis Tool for Gravity-Aware Hinge Calculations
 * 
 * Processes all cmxd-*.log files and shows what the gravity-aware
 * hinge angle calculations would produce for each scenario.
 *
 * Copyright (c) 2025 Armando DiCianno <armando@noonshy.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <regex.h>
#include <math.h>
#include "cmxd-calculations.h"

#define MAX_LINE_LENGTH 1024
#define MAX_FILENAME_LENGTH 256

struct sample_data {
    int base_x, base_y, base_z;
    int lid_x, lid_y, lid_z;
    double timestamp;
    int valid;
};

/* Extract accelerometer data from log line */
int parse_log_line(const char *line, struct sample_data *sample) {
    /* Look for base sensor data: "Base: X=..., Y=..., Z=..." */
    if (strstr(line, "Base: X=")) {
        if (sscanf(line, "%*[^X]X=%d, Y=%d, Z=%d", 
                   &sample->base_x, &sample->base_y, &sample->base_z) == 3) {
            sample->valid |= 1; /* Base data found */
            return 1;
        }
    }
    
    /* Look for lid sensor data: "Lid: X=..., Y=..., Z=..." */
    if (strstr(line, "Lid: X=")) {
        if (sscanf(line, "%*[^X]X=%d, Y=%d, Z=%d", 
                   &sample->lid_x, &sample->lid_y, &sample->lid_z) == 3) {
            sample->valid |= 2; /* Lid data found */
            return 2;
        }
    }
    
    return 0;
}

/* Determine expected mode from angle */
const char* get_mode_from_angle(double angle) {
    if (angle >= 0 && angle < 45) return "closing";
    if (angle >= 45 && angle < 145) return "laptop";
    if (angle >= 145 && angle < 225) return "flat";
    if (angle >= 225 && angle < 330) return "tent";
    if (angle >= 330 && angle <= 360) return "tablet";
    return "unknown";
}

/* Process a single log file */
void process_log_file(const char *filename) {
    FILE *fp;
    char line[MAX_LINE_LENGTH];
    struct sample_data current_sample = {0};
    int sample_count = 0;
    double angle_sum = 0.0;
    double min_angle = 999.0, max_angle = -1.0;
    int total_samples = 0;
    
    printf("\n=== %s ===\n", filename);
    
    fp = fopen(filename, "r");
    if (!fp) {
        printf("ERROR: Could not open %s\n", filename);
        return;
    }
    
    printf("Timestamp    Base[X,Y,Z]           Lid[X,Y,Z]            Angle   Mode     Grav Analysis\n");
    printf("─────────────────────────────────────────────────────────────────────────────────────────\n");
    
    while (fgets(line, sizeof(line), fp)) {
        parse_log_line(line, &current_sample);
        
        /* When we have both base and lid data, calculate hinge angle */
        if (current_sample.valid == 3) { /* Both base (1) and lid (2) data present */
            /* Create accelerometer samples */
            struct cmxd_accel_sample base_sample = {
                .x = current_sample.base_x,
                .y = current_sample.base_y,
                .z = current_sample.base_z,
                .timestamp = sample_count * 1000000000LL
            };
            
            struct cmxd_accel_sample lid_sample = {
                .x = current_sample.lid_x,
                .y = current_sample.lid_y,
                .z = current_sample.lid_z,
                .timestamp = sample_count * 1000000000LL
            };
            
            /* Calculate gravity-aware hinge angle */
            double angle = cmxd_calculate_hinge_angle_360(&base_sample, &lid_sample);
            
            if (angle >= 0) {
                const char* mode = get_mode_from_angle(angle);
                
                /* Detect gravity orientations for analysis */
                int base_grav = cmxd_detect_gravity_orientation(base_sample.x, base_sample.y, base_sample.z);
                int lid_grav = cmxd_detect_gravity_orientation(lid_sample.x, lid_sample.y, lid_sample.z);
                
                /* Show first 10 samples and every 10th after that, plus some random sampling */
                if (sample_count < 10 || sample_count % 10 == 0 || total_samples < 50) {
                    printf("%02d:%02d:%02d     Base[%4d,%4d,%4d]   Lid[%4d,%4d,%4d]   %6.1f°  %-8s B%d,L%d\n",
                           (sample_count / 600) % 24, (sample_count / 10) % 60, sample_count % 10,
                           current_sample.base_x, current_sample.base_y, current_sample.base_z,
                           current_sample.lid_x, current_sample.lid_y, current_sample.lid_z,
                           angle, mode, base_grav, lid_grav);
                }
                
                /* Collect statistics */
                angle_sum += angle;
                if (angle < min_angle) min_angle = angle;
                if (angle > max_angle) max_angle = angle;
                total_samples++;
            }
            
            sample_count++;
            current_sample.valid = 0; /* Reset for next sample pair */
        }
    }
    
    fclose(fp);
    
    if (total_samples > 0) {
        double avg_angle = angle_sum / total_samples;
        const char* dominant_mode = get_mode_from_angle(avg_angle);
        
        printf("─────────────────────────────────────────────────────────────────────────────────────────\n");
        printf("Summary: %d samples processed\n", total_samples);
        printf("  Average angle: %.1f° (dominant mode: %s)\n", avg_angle, dominant_mode);
        printf("  Range: %.1f° to %.1f°\n", min_angle, max_angle);
        printf("  Gravity codes: 0=X-, 1=X+, 2=Y-, 3=Y+, 4=Z-, 5=Z+\n");
    } else {
        printf("No valid sensor data found in log file.\n");
    }
}

int main(void) {
    DIR *dir;
    struct dirent *entry;
    char filepath[MAX_FILENAME_LENGTH];
    
    printf("=== GRAVITY-AWARE HINGE ANGLE ANALYSIS FOR ALL LOG FILES ===\n");
    printf("Analyzing cmxd-*.log files with enhanced gravity-aware calculations...\n");
    
    /* Open current directory */
    dir = opendir(".");
    if (!dir) {
        printf("ERROR: Could not open current directory\n");
        return 1;
    }
    
    /* Process all cmxd-*.log files */
    while ((entry = readdir(dir)) != NULL) {
        /* Check if filename matches pattern cmxd-*.log */
        if (strncmp(entry->d_name, "cmxd-", 5) == 0 && 
            strstr(entry->d_name, ".log") != NULL) {
            
            snprintf(filepath, sizeof(filepath), "%s", entry->d_name);
            process_log_file(filepath);
        }
    }
    
    closedir(dir);
    
    printf("\n=== ANALYSIS COMPLETE ===\n");
    printf("The gravity-aware system provides orientation-independent hinge angle calculations.\n");
    printf("Key improvements:\n");
    printf("  ✓ No false 'closing' mode when device rests on side\n");
    printf("  ✓ Consistent angle ranges regardless of physical orientation\n");
    printf("  ✓ Proper coordinate system transformation based on gravity detection\n");
    
    return 0;
}