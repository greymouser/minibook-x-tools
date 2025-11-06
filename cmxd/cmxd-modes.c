/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Mode detection for CMXD (Chuwi Minibook X Daemon)
 * 
 * Implements device mode detection based on hinge angle calculations using a 0-360°
 * measurement system. Includes hysteresis, stability filtering, and orientation-based
 * mode change freezing to prevent spurious mode transitions.
 * 
 * Copyright (c) 2025 Armando DiCianno <armando@noonshy.com>
 */

#include "cmxd-modes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/* 
 * Mode boundary angles for 0-360° hinge measurement system
 * These define the angle ranges for each device mode:
 *   Closing: 0° to 45° (device nearly closed)
 *   Laptop:  45° to 145° (traditional laptop position) 
 *   Flat:    145° to 225° (device opened flat)
 *   Tent:    225° to 330° (inverted tent configuration)
 *   Tablet:  330° to 360° (folded back for tablet use)
 */
const double CMXD_MODE_CLOSING_MAX = 45.0;      
const double CMXD_MODE_LAPTOP_MAX = 145.0;      
const double CMXD_MODE_FLAT_MAX = 225.0;        
const double CMXD_MODE_TENT_MAX = 330.0;        
const double CMXD_MODE_TABLET_MAX = 360.0;      

/* 
 * Hysteresis to prevent spurious mode switching near boundaries.
 * When transitioning between modes, the angle must exceed the boundary 
 * by this amount to prevent oscillation around threshold values.
 */
const double CMXD_MODE_HYSTERESIS = 15.0;

/* 
 * Stability filtering: number of consecutive readings required before
 * confirming a mode change. Prevents brief sensor glitches from 
 * triggering unwanted mode transitions.
 */
const int CMXD_MODE_STABILITY_SAMPLES = 4;

/* 
 * Duration to freeze mode changes after orientation transitions.
 * When device orientation changes, mode detection is paused for this
 * many samples to prevent erratic behavior during device rotation.
 */
const int CMXD_ORIENTATION_FREEZE_DURATION = 6;

/* Module state variables */
static const char* last_stable_mode = CMXD_MODE_LAPTOP;
static int last_detected_orientation = CMXD_MODE_UNKNOWN;
static int orientation_freeze_samples = 0;
static int stability_count = 0;
static const char* candidate_mode = NULL;
static bool verbose_logging = false;

static void (*log_debug_func)(const char *fmt, ...) = NULL;

/* Set the debug logging function */
void cmxd_modes_set_log_debug(void (*func)(const char *fmt, ...))
{
    log_debug_func = func;
}

static void debug_log(const char *fmt, ...)
{
    if (verbose_logging && log_debug_func) {
        va_list args;
        va_start(args, fmt);
        char buffer[512];
        vsnprintf(buffer, sizeof(buffer), fmt, args);
        va_end(args);
        log_debug_func("%s", buffer);
    }
}

void cmxd_modes_init(void)
{
    last_stable_mode = CMXD_MODE_LAPTOP;
    last_detected_orientation = CMXD_MODE_UNKNOWN;
    orientation_freeze_samples = 0;
    stability_count = 0;
    candidate_mode = NULL;
    verbose_logging = false;
}

void cmxd_modes_reset(void)
{
    cmxd_modes_init();
}

/* 
 * Determine device mode based on hinge angle using 0-360° system.
 * Applies hysteresis to prevent oscillation around mode boundaries.
 */
const char* cmxd_get_device_mode(double angle, const char* current_mode)
{
    if (angle < 0) {
        return CMXD_MODE_LAPTOP;
    }
    
    /* Handle potential wrap-around issues when transitioning from tablet mode */
    if (current_mode && strcmp(current_mode, CMXD_MODE_TABLET) == 0) {
        if (angle < 90.0) {
            debug_log("Tablet mode wrap-around prevention: %.1f° -> treating as ~360°", angle);
            angle = 360.0 - angle;
            debug_log("Adjusted angle: %.1f°", angle);
        }
    }
    
    double hysteresis = CMXD_MODE_HYSTERESIS;
    
    debug_log("Mode detection: angle=%.1f°, current_mode=%s", angle, current_mode ? current_mode : "NULL");
    
    /* Mode transitions with hysteresis applied */
    if (current_mode && strcmp(current_mode, CMXD_MODE_CLOSING) == 0) {
        if (angle >= (CMXD_MODE_CLOSING_MAX + hysteresis)) {
            debug_log("Closing->Laptop transition: %.1f° >= %.1f°", angle, CMXD_MODE_CLOSING_MAX + hysteresis);
            return CMXD_MODE_LAPTOP;
        }
        debug_log("Staying in closing mode");
        return CMXD_MODE_CLOSING;
    } else if (current_mode && strcmp(current_mode, CMXD_MODE_LAPTOP) == 0) {
        if (angle < CMXD_MODE_CLOSING_MAX) {
            debug_log("Laptop->Closing transition: %.1f° < %.1f°", angle, CMXD_MODE_CLOSING_MAX);
            return CMXD_MODE_CLOSING;
        } else if (angle >= (CMXD_MODE_LAPTOP_MAX + hysteresis)) {
            debug_log("Laptop->Flat transition: %.1f° >= %.1f°", angle, CMXD_MODE_LAPTOP_MAX + hysteresis);
            return CMXD_MODE_FLAT;
        }
        debug_log("Staying in laptop mode");
        return CMXD_MODE_LAPTOP;
    } else if (current_mode && strcmp(current_mode, CMXD_MODE_FLAT) == 0) {
        if (angle < CMXD_MODE_CLOSING_MAX) {
            debug_log("Flat->Closing transition: %.1f° < %.1f°", angle, CMXD_MODE_CLOSING_MAX);
            return CMXD_MODE_CLOSING;
        } else if (angle < (CMXD_MODE_LAPTOP_MAX - hysteresis)) {
            debug_log("Flat->Laptop transition: %.1f° < %.1f°", angle, CMXD_MODE_LAPTOP_MAX - hysteresis);
            return CMXD_MODE_LAPTOP;
        } else if (angle >= (CMXD_MODE_FLAT_MAX + hysteresis)) {
            debug_log("Flat->Tent transition: %.1f° >= %.1f°", angle, CMXD_MODE_FLAT_MAX + hysteresis);
            return CMXD_MODE_TENT;
        }
        debug_log("Staying in flat mode");
        return CMXD_MODE_FLAT;
    } else if (current_mode && strcmp(current_mode, CMXD_MODE_TENT) == 0) {
        if (angle < CMXD_MODE_CLOSING_MAX) {
            debug_log("Tent->Closing transition: %.1f° < %.1f°", angle, CMXD_MODE_CLOSING_MAX);
            return CMXD_MODE_CLOSING;
        } else if (angle < (CMXD_MODE_FLAT_MAX - hysteresis)) {
            debug_log("Tent->Flat transition: %.1f° < %.1f°", angle, CMXD_MODE_FLAT_MAX - hysteresis);
            return CMXD_MODE_FLAT;
        } else if (angle >= (CMXD_MODE_TENT_MAX + hysteresis)) {
            debug_log("Tent->Tablet transition: %.1f° >= %.1f°", angle, CMXD_MODE_TENT_MAX + hysteresis);
            return CMXD_MODE_TABLET;
        }
        debug_log("Staying in tent mode");
        return CMXD_MODE_TENT;
    } else if (current_mode && strcmp(current_mode, CMXD_MODE_TABLET) == 0) {
        if (angle < (CMXD_MODE_TENT_MAX - hysteresis)) {
            debug_log("Tablet->Tent transition: %.1f° < %.1f°", angle, CMXD_MODE_TENT_MAX - hysteresis);
            return CMXD_MODE_TENT;
        } else if (angle < (CMXD_MODE_CLOSING_MAX - hysteresis) && angle < 180.0) {
            debug_log("Tablet->Closing transition (wrap): %.1f° < %.1f°", angle, CMXD_MODE_CLOSING_MAX - hysteresis);
            return CMXD_MODE_CLOSING;
        }
        debug_log("Staying in tablet mode");
        return CMXD_MODE_TABLET;
    }
    
    /* Initial mode detection when no current mode is available */
    debug_log("Using fallback logic for angle %.1f°", angle);
    if (angle < CMXD_MODE_CLOSING_MAX) {
        debug_log("Fallback: Closing mode (%.1f° < %.1f°)", angle, CMXD_MODE_CLOSING_MAX);
        return CMXD_MODE_CLOSING;
    } else if (angle < CMXD_MODE_LAPTOP_MAX) {
        debug_log("Fallback: Laptop mode (%.1f° < %.1f°)", angle, CMXD_MODE_LAPTOP_MAX);
        return CMXD_MODE_LAPTOP;
    } else if (angle < CMXD_MODE_FLAT_MAX) {
        debug_log("Fallback: Flat mode (%.1f° < %.1f°)", angle, CMXD_MODE_FLAT_MAX);
        return CMXD_MODE_FLAT;
    } else if (angle < CMXD_MODE_TENT_MAX) {
        debug_log("Fallback: Tent mode (%.1f° < %.1f°)", angle, CMXD_MODE_TENT_MAX);
        return CMXD_MODE_TENT;
    } else {
        debug_log("Fallback: Tablet mode (%.1f° >= %.1f°)", angle, CMXD_MODE_TENT_MAX);
        return CMXD_MODE_TABLET;
    }
}

/*
 * Get stable device mode with enhanced validation and filtering.
 * Applies orientation change freezing and stability requirements 
 * to prevent spurious mode transitions.
 */
const char* cmxd_get_stable_device_mode(double angle, int orientation)
{
    /* Freeze mode changes during orientation transitions */
    if (last_detected_orientation != CMXD_MODE_UNKNOWN && orientation != last_detected_orientation) {
        debug_log("Orientation change detected: %d -> %d, freezing mode changes for %d samples", 
                 last_detected_orientation, orientation, CMXD_ORIENTATION_FREEZE_DURATION);
        orientation_freeze_samples = CMXD_ORIENTATION_FREEZE_DURATION;
        last_detected_orientation = orientation;
    } else if (last_detected_orientation == CMXD_MODE_UNKNOWN) {
        last_detected_orientation = orientation;
    }
    
    if (orientation_freeze_samples > 0) {
        orientation_freeze_samples--;
        debug_log("Mode frozen due to orientation change (remaining: %d samples), maintaining mode", 
                 orientation_freeze_samples);
        stability_count = 0;
        candidate_mode = NULL;
        return last_stable_mode;
    }
    
    int required_stability = CMXD_MODE_STABILITY_SAMPLES;
    
    const char* new_mode = cmxd_get_device_mode(angle, last_stable_mode);
    
    /* Handle tent-to-tablet wrap-around edge case */
    if (strcmp(last_stable_mode, CMXD_MODE_TENT) == 0 && 
        candidate_mode && strcmp(candidate_mode, CMXD_MODE_TABLET) == 0 &&
        angle < 90.0 && stability_count > 0) {
        debug_log("Tent->Tablet wrap-around: treating %.1f° as tablet stability sample", angle);
        new_mode = CMXD_MODE_TABLET;
    }
    
    /* No mode change - reset stability tracking */
    if (strcmp(new_mode, last_stable_mode) == 0) {
        stability_count = 0;
        candidate_mode = NULL;
        return new_mode;
    }
    
    /* Validate transition is allowed (prevents invalid jumps) */
    bool valid_transition = false;
    
    if (strcmp(last_stable_mode, CMXD_MODE_CLOSING) == 0) {
        valid_transition = (strcmp(new_mode, CMXD_MODE_LAPTOP) == 0);
    } else if (strcmp(last_stable_mode, CMXD_MODE_LAPTOP) == 0) {
        valid_transition = (strcmp(new_mode, CMXD_MODE_CLOSING) == 0 || strcmp(new_mode, CMXD_MODE_FLAT) == 0);
    } else if (strcmp(last_stable_mode, CMXD_MODE_FLAT) == 0) {
        valid_transition = (strcmp(new_mode, CMXD_MODE_LAPTOP) == 0 || strcmp(new_mode, CMXD_MODE_TENT) == 0);
    } else if (strcmp(last_stable_mode, CMXD_MODE_TENT) == 0) {
        valid_transition = (strcmp(new_mode, CMXD_MODE_FLAT) == 0 || strcmp(new_mode, CMXD_MODE_TABLET) == 0);
    } else if (strcmp(last_stable_mode, CMXD_MODE_TABLET) == 0) {
        valid_transition = (strcmp(new_mode, CMXD_MODE_TENT) == 0);
    }
    
    if (!valid_transition) {
        debug_log("Invalid mode jump blocked: %s -> %s (angle %.1f°)", last_stable_mode, new_mode, angle);
        stability_count = 0;
        candidate_mode = NULL;
        return last_stable_mode;
    }
    
    /* Apply stability filtering for valid transitions */
    if (candidate_mode == NULL || strcmp(candidate_mode, new_mode) != 0) {
        candidate_mode = new_mode;
        stability_count = 1;
        debug_log("New mode candidate: %s (stability: %d/%d, angle %.1f°)", 
                 candidate_mode, stability_count, required_stability, angle);
        return last_stable_mode;
    } else {
        stability_count++;
        debug_log("Mode candidate stability: %s (%d/%d, angle %.1f°)", 
                 candidate_mode, stability_count, required_stability, angle);
        
        if (stability_count >= required_stability) {
            debug_log("Mode change confirmed: %s -> %s (angle %.1f°, stable for %d samples)", 
                     last_stable_mode, new_mode, angle, stability_count);
            last_stable_mode = new_mode;
            stability_count = 0;
            candidate_mode = NULL;
            return new_mode;
        } else {
            return last_stable_mode;
        }
    }
}

/* Check if mode has changed from last reading */
bool cmxd_mode_has_changed(const char* current_mode)
{
    static const char* last_reported_mode = NULL;
    
    if (last_reported_mode == NULL) {
        last_reported_mode = current_mode;
        return true;
    }
    
    bool changed = (strcmp(current_mode, last_reported_mode) != 0);
    if (changed) {
        debug_log("Mode change detected: %s -> %s", last_reported_mode, current_mode);
        last_reported_mode = current_mode;
    }
    
    return changed;
}

const char* cmxd_get_last_mode(void)
{
    return last_stable_mode;
}

/* 
 * Check if a mode represents tablet-like usage.
 * Both tent and tablet modes are considered tablet-like for orientation purposes.
 */
bool cmxd_is_tablet_mode(const char* mode)
{
    if (mode == NULL) {
        return false;
    }
    return (strcmp(mode, CMXD_MODE_TABLET) == 0 || strcmp(mode, CMXD_MODE_TENT) == 0);
}

void cmxd_modes_set_verbose(bool verbose)
{
    verbose_logging = verbose;
}