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

/* Mode boundary angles - 0-360° system */
const double CMXD_MODE_CLOSING_MAX = 45.0;      /* 0°-45°: Closing mode */
const double CMXD_MODE_LAPTOP_MAX = 145.0;      /* 45°-145°: Laptop mode */
const double CMXD_MODE_FLAT_MAX = 225.0;        /* 145°-225°: Flat mode */
const double CMXD_MODE_TENT_MAX = 330.0;        /* 225°-315°: Tent mode */
const double CMXD_MODE_TABLET_MAX = 360.0;      /* 315°-360° (& 0°-45°): Tablet mode */

/* Hysteresis amount to prevent spurious mode switching */
const double CMXD_MODE_HYSTERESIS = 15.0;          /* Standard: 15° of hysteresis around boundaries */

/* Spurious data filtering - require consistency before mode changes */
const int CMXD_MODE_STABILITY_SAMPLES = 4;         /* Standard: 5 consistent readings required */

/* Orientation change detection to prevent mode changes during device rotation */
const int CMXD_ORIENTATION_FREEZE_DURATION = 6;    /* Number of samples to freeze after orientation change */

/* Module state */
static const char* last_stable_mode = CMXD_MODE_LAPTOP;  /* Default to laptop mode */
static int last_detected_orientation = CMXD_MODE_UNKNOWN;
static int orientation_freeze_samples = 0;
static int stability_count = 0;
static const char* candidate_mode = NULL;
static bool verbose_logging = false;

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
    verbose_logging = false;
}

/* Reset mode detection state */
void cmxd_modes_reset(void)
{
    cmxd_modes_init();
}

/* Get device mode based on hinge angle with 0-360° system */
const char* cmxd_get_device_mode(double angle, const char* current_mode)
{
    if (angle < 0) {
        return CMXD_MODE_LAPTOP;  /* Invalid angle - default to laptop */
    }
    
    /* Prevent spurious wrap-around when in tablet mode */
    if (current_mode && strcmp(current_mode, CMXD_MODE_TABLET) == 0) {
        if (angle < 90.0) {  /* If angle appears to be near 0° but we're in tablet mode */
            debug_log("Tablet mode wrap-around prevention: %.1f° -> treating as ~360°", angle);
            angle = 360.0 - angle;  /* Treat as being near 360° instead */
            debug_log("Adjusted angle: %.1f°", angle);
        }
    }
    
    /* Use hysteresis to prevent spurious mode switching */
    double hysteresis = CMXD_MODE_HYSTERESIS;  /* 15.0 degrees */
    
    debug_log("Mode detection: angle=%.1f°, current_mode=%s", angle, current_mode ? current_mode : "NULL");
    
    /* 0-360° mode detection with hysteresis */
    if (current_mode && strcmp(current_mode, CMXD_MODE_CLOSING) == 0) {
        if (angle >= (CMXD_MODE_CLOSING_MAX + hysteresis)) {
            debug_log("Closing->Laptop transition: %.1f° >= %.1f°", angle, CMXD_MODE_CLOSING_MAX + hysteresis);
            return CMXD_MODE_LAPTOP;
        }
        debug_log("Staying in closing mode");
        return CMXD_MODE_CLOSING;
    } else if (current_mode && strcmp(current_mode, CMXD_MODE_LAPTOP) == 0) {
        if (angle < CMXD_MODE_CLOSING_MAX) {  /* Always return to closing for angles < 45° */
            debug_log("Laptop->Closing transition: %.1f° < %.1f°", angle, CMXD_MODE_CLOSING_MAX);
            return CMXD_MODE_CLOSING;
        } else if (angle >= (CMXD_MODE_LAPTOP_MAX + hysteresis)) {
            debug_log("Laptop->Flat transition: %.1f° >= %.1f°", angle, CMXD_MODE_LAPTOP_MAX + hysteresis);
            return CMXD_MODE_FLAT;
        }
        debug_log("Staying in laptop mode");
        return CMXD_MODE_LAPTOP;
    } else if (current_mode && strcmp(current_mode, CMXD_MODE_FLAT) == 0) {
        if (angle < CMXD_MODE_CLOSING_MAX) {  /* Always return to closing for angles < 45° */
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
        if (angle < CMXD_MODE_CLOSING_MAX) {  /* Always return to closing for angles < 45° */
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
    
    /* Fallback logic for initial mode detection */
    debug_log("Using fallback logic for angle %.1f°", angle);
    if (angle < CMXD_MODE_CLOSING_MAX) {
        debug_log("Fallback: Closing mode (%.1f° < %.1f°)", angle, CMXD_MODE_CLOSING_MAX);
        return CMXD_MODE_CLOSING;       /* 0°-45° */
    } else if (angle < CMXD_MODE_LAPTOP_MAX) {
        debug_log("Fallback: Laptop mode (%.1f° < %.1f°)", angle, CMXD_MODE_LAPTOP_MAX);
        return CMXD_MODE_LAPTOP;        /* 45°-145° */
    } else if (angle < CMXD_MODE_FLAT_MAX) {
        debug_log("Fallback: Flat mode (%.1f° < %.1f°)", angle, CMXD_MODE_FLAT_MAX);
        return CMXD_MODE_FLAT;          /* 145°-225° */
    } else if (angle < CMXD_MODE_TENT_MAX) {
        debug_log("Fallback: Tent mode (%.1f° < %.1f°)", angle, CMXD_MODE_TENT_MAX);
        return CMXD_MODE_TENT;          /* 225°-315° */
    } else {
        debug_log("Fallback: Tablet mode (%.1f° >= %.1f°)", angle, CMXD_MODE_TENT_MAX);
        return CMXD_MODE_TABLET;        /* 315°-360° */
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
    
    /* Determine required stability samples - standard for all modes */
    int required_stability = CMXD_MODE_STABILITY_SAMPLES;  /* 5 samples for all modes */
    
    const char* new_mode = cmxd_get_device_mode(angle, last_stable_mode);
    
    /* Special case: tent mode wrap-around prevention for tablet transitions */
    if (strcmp(last_stable_mode, CMXD_MODE_TENT) == 0 && 
        candidate_mode && strcmp(candidate_mode, CMXD_MODE_TABLET) == 0 &&
        angle < 90.0 && stability_count > 0) {
        /* We're in tent mode, building tablet stability, and got a low angle that could be wrap-around */
        debug_log("Tent->Tablet wrap-around: treating %.1f° as tablet stability sample", angle);
        new_mode = CMXD_MODE_TABLET;  /* Treat this as a tablet reading for stability */
    }
    
    /* If mode hasn't changed, update tracking and return */
    if (strcmp(new_mode, last_stable_mode) == 0) {
        /* Reset stability tracking since we're staying in current mode */
        stability_count = 0;
        candidate_mode = NULL;
        return new_mode;
    }
    
    /* Check if this is a valid transition - 0-360° system */
    bool valid_transition = false;
    
    if (strcmp(last_stable_mode, CMXD_MODE_CLOSING) == 0) {
        valid_transition = (strcmp(new_mode, CMXD_MODE_LAPTOP) == 0);  /* Only laptop mode */
    } else if (strcmp(last_stable_mode, CMXD_MODE_LAPTOP) == 0) {
        valid_transition = (strcmp(new_mode, CMXD_MODE_CLOSING) == 0 || strcmp(new_mode, CMXD_MODE_FLAT) == 0);
    } else if (strcmp(last_stable_mode, CMXD_MODE_FLAT) == 0) {
        valid_transition = (strcmp(new_mode, CMXD_MODE_LAPTOP) == 0 || strcmp(new_mode, CMXD_MODE_TENT) == 0);
    } else if (strcmp(last_stable_mode, CMXD_MODE_TENT) == 0) {
        valid_transition = (strcmp(new_mode, CMXD_MODE_FLAT) == 0 || strcmp(new_mode, CMXD_MODE_TABLET) == 0);
    } else if (strcmp(last_stable_mode, CMXD_MODE_TABLET) == 0) {
        valid_transition = (strcmp(new_mode, CMXD_MODE_TENT) == 0);  /* Only tent mode */
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