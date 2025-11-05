/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Mode detection for CMXD (Chuwi Minibook X Daemon)
 * 
 * Device mode detection based on hinge angle calculations
 * 
 * Copyright (c) 2025 Armando DiCianno <armando@noonshy.com>
 */

#include "cmxd-modes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/* Mode boundary angles - hybrid approach with state tracking for tent/tablet */
const double CMXD_MODE_CLOSING_MAX = 45.0;      /* 0°-45°: Closing mode */
const double CMXD_MODE_LAPTOP_MAX = 145.0;      /* 45°-145°: Laptop mode */
const double CMXD_MODE_FLAT_MAX = 180.0;        /* 145°-180°: Flat mode */
const double CMXD_MODE_TENT_MAX = 145.0;        /* State-tracked: 145°-120° on return journey */
const double CMXD_MODE_TABLET_MAX = 120.0;      /* State-tracked: 120°-90° on return journey */

/* Hysteresis amount to prevent spurious mode switching */
const double CMXD_MODE_HYSTERESIS = 15.0;          /* Reduced: 15° of hysteresis around boundaries */
const double CMXD_TABLET_MODE_HYSTERESIS = 25.0;   /* Reduced: 25° hysteresis for tablet mode stability */

/* Spurious data filtering - require consistency before mode changes */
const int CMXD_MODE_STABILITY_SAMPLES = 5;         /* Increased: 5 consistent readings required */
const int CMXD_TABLET_MODE_STABILITY_SAMPLES = 8;  /* Enhanced: 8 stability requirement for tablet mode exits */

/* Orientation change detection to prevent mode changes during device rotation */
const int CMXD_ORIENTATION_FREEZE_DURATION = 8;    /* Number of samples to freeze after orientation change */

/* Module state */
static const char* last_stable_mode = CMXD_MODE_LAPTOP;  /* Default to laptop mode */
static int last_detected_orientation = CMXD_MODE_UNKNOWN;
static int orientation_freeze_samples = 0;
static int stability_count = 0;
static const char* candidate_mode = NULL;
static bool verbose_logging = false;

/* State tracking for tent/tablet detection */
static bool has_reached_flat = false;           /* Track if we've been in flat mode */
static int flat_exit_direction = 0;            /* 1 = exited flat toward fold-back, -1 = toward laptop, 0 = unknown */
static double last_angle = 0.0;                /* Track angle changes for direction detection */

/* Logging function (will be set by main) */
static void (*log_debug_func)(const char *fmt, ...) = NULL;

/* Set the debug logging function */
void cmxd_modes_set_log_debug(void (*func)(const char *fmt, ...))
{
    log_debug_func = func;
}

/* Internal debug logging */
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

/* Initialize mode detection module */
void cmxd_modes_init(void)
{
    last_stable_mode = CMXD_MODE_LAPTOP;
    last_detected_orientation = CMXD_MODE_UNKNOWN;
    orientation_freeze_samples = 0;
    stability_count = 0;
    candidate_mode = NULL;
    verbose_logging = true;  /* Enable verbose logging for mode detection */
}

/* Reset mode detection state */
void cmxd_modes_reset(void)
{
    cmxd_modes_init();
}

/* Get device mode based on hinge angle with hybrid state tracking */
const char* cmxd_get_device_mode(double angle, const char* current_mode)
{
    if (angle < 0) {
        return CMXD_MODE_LAPTOP;  /* Invalid angle - default to laptop */
    }
    
    /* Track angle changes for direction detection */
    if (last_angle > 0) {
        if (angle > last_angle + 2.0) {
            /* Moving toward higher angles (toward flat/fold-back) */
            if (angle > CMXD_MODE_FLAT_MAX - 10.0) {
                flat_exit_direction = 1;  /* Exiting flat toward fold-back */
                debug_log("Detected fold-back direction: angle %.1f° > %.1f°", angle, CMXD_MODE_FLAT_MAX - 10.0);
            }
        } else if (angle < last_angle - 2.0) {
            /* Moving toward lower angles (toward laptop/closing) */
            if (has_reached_flat && angle < CMXD_MODE_FLAT_MAX - 20.0) {
                flat_exit_direction = -1; /* Exiting flat toward laptop */
                debug_log("Detected laptop direction: angle %.1f° < %.1f°", angle, CMXD_MODE_FLAT_MAX - 20.0);
            }
        }
    }
    last_angle = angle;
    
    /* Track if we've reached flat mode */
    if (angle >= CMXD_MODE_FLAT_MAX - 5.0) {
        if (!has_reached_flat) {
            debug_log("Reached flat mode: angle %.1f°", angle);
        }
        has_reached_flat = true;
    }
    
    /* Reset state when we return to laptop mode normally */
    if (angle < CMXD_MODE_LAPTOP_MAX - 20.0 && flat_exit_direction != 1) {
        if (has_reached_flat) {
            debug_log("Reset state: returned to laptop normally at %.1f°", angle);
        }
        has_reached_flat = false;
        flat_exit_direction = 0;
    }
    
    /* Use enhanced hysteresis */
    double hysteresis = CMXD_MODE_HYSTERESIS;  /* Default: 15.0 degrees */
    if (current_mode && (strcmp(current_mode, CMXD_MODE_TENT) == 0 || strcmp(current_mode, CMXD_MODE_TABLET) == 0)) {
        hysteresis = CMXD_TABLET_MODE_HYSTERESIS;  /* Enhanced: 25.0 degrees */
    } else if (current_mode && strcmp(current_mode, CMXD_MODE_LAPTOP) == 0) {
        hysteresis = CMXD_MODE_HYSTERESIS;  /* Keep standard: 15.0 degrees for laptop mode */
    }
    
    /* Tent/tablet mode detection: if we've been to flat and are coming back with fold-back direction */
    if (has_reached_flat && flat_exit_direction == 1) {
        debug_log("Fold-back mode detection: angle %.1f°, has_flat=%d, direction=%d", angle, has_reached_flat, flat_exit_direction);
        if (angle <= CMXD_MODE_TABLET_MAX) {
            return CMXD_MODE_TABLET;  /* Fully folded back */
        } else if (angle <= CMXD_MODE_TENT_MAX) {
            return CMXD_MODE_TENT;    /* Partially folded back */
        } else {
            /* Still in higher angle range but coming from fold-back - stay in tent mode */
            return CMXD_MODE_TENT;
        }
    }
    
    /* Standard mode detection with hysteresis */
    if (current_mode && strcmp(current_mode, CMXD_MODE_CLOSING) == 0) {
        if (angle >= (CMXD_MODE_CLOSING_MAX + hysteresis)) {
            return CMXD_MODE_LAPTOP;
        }
        return CMXD_MODE_CLOSING;
    } else if (current_mode && strcmp(current_mode, CMXD_MODE_LAPTOP) == 0) {
        if (angle < (CMXD_MODE_CLOSING_MAX - hysteresis)) {
            return CMXD_MODE_CLOSING;
        } else if (angle >= (CMXD_MODE_LAPTOP_MAX + hysteresis)) {
            return CMXD_MODE_FLAT;
        }
        return CMXD_MODE_LAPTOP;
    } else if (current_mode && strcmp(current_mode, CMXD_MODE_FLAT) == 0) {
        if (angle < (CMXD_MODE_LAPTOP_MAX - hysteresis)) {
            return CMXD_MODE_LAPTOP;
        }
        return CMXD_MODE_FLAT;
    } else if (current_mode && strcmp(current_mode, CMXD_MODE_TENT) == 0) {
        if (angle >= (CMXD_MODE_TENT_MAX + hysteresis)) {
            return CMXD_MODE_FLAT;  /* Return to flat if angle increases */
        } else if (angle <= (CMXD_MODE_TABLET_MAX - hysteresis)) {
            return CMXD_MODE_TABLET;
        }
        return CMXD_MODE_TENT;
    } else if (current_mode && strcmp(current_mode, CMXD_MODE_TABLET) == 0) {
        if (angle >= (CMXD_MODE_TABLET_MAX + hysteresis)) {
            return CMXD_MODE_TENT;
        }
        return CMXD_MODE_TABLET;
    }
    
    /* Fallback to standard logic */
    if (angle < CMXD_MODE_CLOSING_MAX) {
        return CMXD_MODE_CLOSING;       
    } else if (angle < CMXD_MODE_LAPTOP_MAX) {
        return CMXD_MODE_LAPTOP;        
    } else {
        return CMXD_MODE_FLAT;          
    }
}

/* Get stable device mode with enhanced validation and filtering */
const char* cmxd_get_stable_device_mode(double angle, int orientation)
{
    /* Check for orientation changes and freeze mode changes during transitions */
    if (last_detected_orientation != CMXD_MODE_UNKNOWN && orientation != last_detected_orientation) {
        debug_log("Orientation change detected: %d -> %d, freezing mode changes for %d samples", 
                 last_detected_orientation, orientation, CMXD_ORIENTATION_FREEZE_DURATION);
        orientation_freeze_samples = CMXD_ORIENTATION_FREEZE_DURATION;
        last_detected_orientation = orientation;
    } else if (last_detected_orientation == CMXD_MODE_UNKNOWN) {
        last_detected_orientation = orientation;
    }
    
    /* If we're in orientation freeze, maintain current mode */
    if (orientation_freeze_samples > 0) {
        orientation_freeze_samples--;
        debug_log("Mode frozen due to orientation change (remaining: %d samples), maintaining mode", 
                 orientation_freeze_samples);
        /* Reset stability tracking during freeze to prevent spurious changes */
        stability_count = 0;
        candidate_mode = NULL;
        return last_stable_mode;
    }
    
    /* Determine required stability samples based on current mode */
    int required_stability = CMXD_MODE_STABILITY_SAMPLES;  /* Default: 5 */
    if (strcmp(last_stable_mode, CMXD_MODE_TABLET) == 0 || strcmp(last_stable_mode, CMXD_MODE_TENT) == 0 || strcmp(last_stable_mode, CMXD_MODE_FLAT) == 0) {
        required_stability = CMXD_TABLET_MODE_STABILITY_SAMPLES;  /* Enhanced: 8 */
    }
    
    const char* new_mode = cmxd_get_device_mode(angle, last_stable_mode);
    
    /* If mode hasn't changed, update tracking and return */
    if (strcmp(new_mode, last_stable_mode) == 0) {
        /* Reset stability tracking since we're staying in current mode */
        stability_count = 0;
        candidate_mode = NULL;
        return new_mode;
    }
    
    /* Check if this is a valid transition - hybrid 5-mode system */
    bool valid_transition = false;
    
    if (strcmp(last_stable_mode, CMXD_MODE_CLOSING) == 0) {
        valid_transition = (strcmp(new_mode, CMXD_MODE_LAPTOP) == 0);
    } else if (strcmp(last_stable_mode, CMXD_MODE_LAPTOP) == 0) {
        valid_transition = (strcmp(new_mode, CMXD_MODE_CLOSING) == 0 || strcmp(new_mode, CMXD_MODE_FLAT) == 0 || strcmp(new_mode, CMXD_MODE_TENT) == 0);
    } else if (strcmp(last_stable_mode, CMXD_MODE_FLAT) == 0) {
        valid_transition = (strcmp(new_mode, CMXD_MODE_LAPTOP) == 0 || strcmp(new_mode, CMXD_MODE_TENT) == 0);
    } else if (strcmp(last_stable_mode, CMXD_MODE_TENT) == 0) {
        valid_transition = (strcmp(new_mode, CMXD_MODE_FLAT) == 0 || strcmp(new_mode, CMXD_MODE_TABLET) == 0 || strcmp(new_mode, CMXD_MODE_LAPTOP) == 0);
    } else if (strcmp(last_stable_mode, CMXD_MODE_TABLET) == 0) {
        valid_transition = (strcmp(new_mode, CMXD_MODE_TENT) == 0);
    }
    
    if (!valid_transition) {
        debug_log("Invalid mode jump blocked: %s -> %s (angle %.1f°)", last_stable_mode, new_mode, angle);
        /* Reset stability tracking since this transition is not allowed */
        stability_count = 0;
        candidate_mode = NULL;
        return last_stable_mode;
    }
    
    /* Valid transition - now check for stability to prevent spurious changes */
    if (candidate_mode == NULL || strcmp(candidate_mode, new_mode) != 0) {
        /* New candidate mode - start stability tracking */
        candidate_mode = new_mode;
        stability_count = 1;
        debug_log("New mode candidate: %s (stability: %d/%d, angle %.1f°)", 
                 candidate_mode, stability_count, required_stability, angle);
        return last_stable_mode;  /* Keep current mode until stable */
    } else {
        /* Same candidate mode - increment stability counter */
        stability_count++;
        debug_log("Mode candidate stability: %s (%d/%d, angle %.1f°)", 
                 candidate_mode, stability_count, required_stability, angle);
        
        if (stability_count >= required_stability) {
            /* Stable mode change confirmed */
            debug_log("Mode change confirmed: %s -> %s (angle %.1f°, stable for %d samples)", 
                     last_stable_mode, new_mode, angle, stability_count);
            last_stable_mode = new_mode;
            stability_count = 0;
            candidate_mode = NULL;
            return new_mode;
        } else {
            /* Not stable yet, keep current mode */
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
        return true;  /* First reading counts as a change */
    }
    
    bool changed = (strcmp(current_mode, last_reported_mode) != 0);
    if (changed) {
        debug_log("Mode change detected: %s -> %s", last_reported_mode, current_mode);
        last_reported_mode = current_mode;
    }
    
    return changed;
}

/* Get the last detected mode */
const char* cmxd_get_last_mode(void)
{
    return last_stable_mode;
}

/* Check if a mode represents tablet mode */
bool cmxd_is_tablet_mode(const char* mode)
{
    if (mode == NULL) {
        return false;
    }
    /* Tent and tablet modes are considered tablet-like for orientation purposes */
    return (strcmp(mode, CMXD_MODE_TABLET) == 0 || strcmp(mode, CMXD_MODE_TENT) == 0);
}

/* Set verbose logging for mode detection */
void cmxd_modes_set_verbose(bool verbose)
{
    verbose_logging = verbose;
}