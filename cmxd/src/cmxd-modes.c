/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Simplified Mode Detection for CMXD (Chuwi Minibook X Daemon)
 * 
 * Simple device mode detection based on hinge angle calculations.
 * Now simplified thanks to correct mount matrices providing reliable angles!
 * 
 * Copyright (c) 2025 Armando DiCianno <armando@noonshy.com>
 */

#include "cmxd-modes.h"
#include "cmxd-protocol.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/* 
 * Correct mode boundaries based on physical hinge angles:
 *   Closing: 0° to 45°     (device closing to nearly closed)
 *   Laptop:  45° to 160°   (traditional laptop position, 90° ideal)
 *   Flat:    160° to 240°  (opening towards flat, ~180° = completely flat)
 *   Tent:    240° to 345°  (tent mode - lid folded back, extended range for inverted tent)
 *   Tablet:  345° to 360°  (fully folded back for tablet use)
 *
 * These ranges account for the 360° hinge angle calculation.
 */
const double CMXD_MODE_CLOSING_MAX = 45.0;      
const double CMXD_MODE_LAPTOP_MAX = 160.0;      
const double CMXD_MODE_FLAT_MAX = 240.0;        
const double CMXD_MODE_TENT_MAX = 345.0;        /* Extended for inverted tent configurations */
const double CMXD_MODE_TABLET_MAX = 360.0;      

/* Simple hysteresis to prevent mode jitter - reduced for better closing mode transitions */
const double CMXD_MODE_HYSTERESIS = 6.0;

/* Gravity vector confidence thresholds */
/* Gravity confidence detection constants */
const double CMXD_GRAVITY_MIN_CONFIDENCE = 7.5;   /* Minimum magnitude for reliable gravity reading (lowered for tent) */
const double CMXD_GRAVITY_MAX_CONFIDENCE = 13.0;  /* Maximum magnitude for reliable gravity reading */
const double CMXD_GRAVITY_TILT_THRESHOLD = 20.0;  /* Max horizontal acceleration for stable readings - match tent mode */

/* Mode-specific tilt tolerance constants - increased to reduce false indeterminate mode */
const double CMXD_LAPTOP_TILT_TOLERANCE = 12.0;   /* Laptop can handle significant tilt */
const double CMXD_CLOSING_TILT_TOLERANCE = 12.0;  /* Closing same as laptop */
const double CMXD_FLAT_TILT_TOLERANCE = 18.0;     /* Flat mode higher tolerance for transitions to tent/tablet */
const double CMXD_TENT_TILT_TOLERANCE = 20.0;     /* Tent mode has high horizontal acceleration due to sensor orientation */
const double CMXD_TABLET_TILT_TOLERANCE = 15.0;   /* Tablet higher tolerance for handheld use */

/* Require a few stable readings before mode change */
const int CMXD_MODE_STABILITY_SAMPLES = 3;

/* Mode sequence for preventing jumps - now includes indeterminate state */
static const char* const mode_sequence[] = {
    CMXD_PROTOCOL_MODE_CLOSING,
    CMXD_PROTOCOL_MODE_LAPTOP,
    CMXD_PROTOCOL_MODE_FLAT,
    CMXD_PROTOCOL_MODE_TENT,
    CMXD_PROTOCOL_MODE_TABLET,
    CMXD_MODE_INDETERMINATE,  /* Special transition state */
    NULL
};
static const int mode_sequence_length = sizeof(mode_sequence) / sizeof(mode_sequence[0]);

/* Get mode index in sequence */
static int get_mode_index(const char* mode)
{
    if (!mode) return 1; /* Default to laptop */
    
    for (int i = 0; i < mode_sequence_length; i++) {
        if (strcmp(mode, mode_sequence[i]) == 0) {
            return i;
        }
    }
    return 1; /* Default to laptop if not found */
}

/* Check if mode transition is allowed (prevents jumping) */
static bool is_mode_transition_allowed(const char* from_mode, const char* to_mode)
{
    if (!from_mode || !to_mode) return true;
    
    /* Allow any transition FROM indeterminate state */
    if (strcmp(from_mode, CMXD_MODE_INDETERMINATE) == 0) {
        return true;
    }
    
    /* Note: to_mode will never be indeterminate when called from cmxd_get_device_mode() */
    /* since that function only returns protocol modes */
    
    int from_idx = get_mode_index(from_mode);
    int to_idx = get_mode_index(to_mode);
    
    /* Allow adjacent transitions or staying in same mode */
    int diff = abs(to_idx - from_idx);
    if (diff <= 1) {
        return true;
    }
    
    /* Special case: Allow direct laptop ↔ tent transitions */
    /* This handles convertible laptops that can fold directly from laptop to tent mode */
    bool is_laptop_tent = (strcmp(from_mode, CMXD_PROTOCOL_MODE_LAPTOP) == 0 && strcmp(to_mode, CMXD_PROTOCOL_MODE_TENT) == 0) ||
                         (strcmp(from_mode, CMXD_PROTOCOL_MODE_TENT) == 0 && strcmp(to_mode, CMXD_PROTOCOL_MODE_LAPTOP) == 0);
    
    return is_laptop_tent;
}

/* Mode-aware gravity confidence checking - uses different tilt tolerances per mode */
static bool is_gravity_confident_for_mode(double base_mag, double lid_mag, double total_horizontal, const char* current_mode)
{
    /* Special handling for tent mode - relaxed gravity magnitude requirements */
    if (strcmp(current_mode, CMXD_PROTOCOL_MODE_TENT) == 0) {
        /* Tent mode often has lower magnitude readings in inverted position */
        /* Allow down to 5.5 for tent mode to handle inverted tent configurations with very low base readings */
        bool base_good = (base_mag >= 5.5 && base_mag <= CMXD_GRAVITY_MAX_CONFIDENCE);
        bool lid_good = (lid_mag >= 5.5 && lid_mag <= CMXD_GRAVITY_MAX_CONFIDENCE);
        bool stable = (total_horizontal < CMXD_TENT_TILT_TOLERANCE);
        return (base_good && lid_good && stable);
    }
    
    /* Standard gravity magnitude check for other modes */
    bool base_good = (base_mag >= CMXD_GRAVITY_MIN_CONFIDENCE && base_mag <= CMXD_GRAVITY_MAX_CONFIDENCE);
    bool lid_good = (lid_mag >= CMXD_GRAVITY_MIN_CONFIDENCE && lid_mag <= CMXD_GRAVITY_MAX_CONFIDENCE);
    
    /* Mode-specific tilt tolerance */
    double tilt_tolerance = CMXD_GRAVITY_TILT_THRESHOLD; /* Default fallback */
    
    if (strcmp(current_mode, CMXD_PROTOCOL_MODE_LAPTOP) == 0) {
        tilt_tolerance = CMXD_LAPTOP_TILT_TOLERANCE;
    } else if (strcmp(current_mode, CMXD_PROTOCOL_MODE_CLOSING) == 0) {
        tilt_tolerance = CMXD_CLOSING_TILT_TOLERANCE;
    } else if (strcmp(current_mode, CMXD_PROTOCOL_MODE_FLAT) == 0) {
        tilt_tolerance = CMXD_FLAT_TILT_TOLERANCE;
    } else if (strcmp(current_mode, CMXD_PROTOCOL_MODE_TENT) == 0) {
        tilt_tolerance = CMXD_TENT_TILT_TOLERANCE;
    } else if (strcmp(current_mode, CMXD_PROTOCOL_MODE_TABLET) == 0) {
        tilt_tolerance = CMXD_TABLET_TILT_TOLERANCE;
    }
    
    bool stable = (total_horizontal < tilt_tolerance);
    
    return (base_good && lid_good && stable);
}

/* Not needed with reliable mount matrices */
const int CMXD_ORIENTATION_FREEZE_DURATION = 0;

/* Module state */
static const char* current_mode = CMXD_PROTOCOL_MODE_LAPTOP;
static const char* candidate_mode = NULL;
static int stability_count = 0;
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
        char buffer[256];
        vsnprintf(buffer, sizeof(buffer), fmt, args);
        va_end(args);
        log_debug_func("%s", buffer);
    }
}

void cmxd_modes_init(void)
{
    current_mode = CMXD_PROTOCOL_MODE_LAPTOP;
    candidate_mode = NULL;
    stability_count = 0;
    verbose_logging = false;
}

/* Simplified mode determination based on angle */
const char* cmxd_get_device_mode(double angle, const char* current_mode_param)
{
    if (angle < 0) {
        return CMXD_PROTOCOL_MODE_LAPTOP;  /* Default for invalid readings */
    }
    
    /* Determine base mode from angle */
    const char* new_mode;
    if (angle < CMXD_MODE_CLOSING_MAX) {
        new_mode = CMXD_PROTOCOL_MODE_CLOSING;
    } else if (angle < CMXD_MODE_LAPTOP_MAX) {
        new_mode = CMXD_PROTOCOL_MODE_LAPTOP;
    } else if (angle < CMXD_MODE_FLAT_MAX) {
        new_mode = CMXD_PROTOCOL_MODE_FLAT;
    } else if (angle < CMXD_MODE_TENT_MAX) {
        new_mode = CMXD_PROTOCOL_MODE_TENT;
    } else {
        new_mode = CMXD_PROTOCOL_MODE_TABLET;
    }
    
    /* Apply enhanced mode transition logic */
    if (current_mode_param) {
        /* First check if transition is allowed (prevents jumping) */
        if (!is_mode_transition_allowed(current_mode_param, new_mode)) {
            debug_log("Mode jump prevented: %s -> %s (not adjacent)", current_mode_param, new_mode);
            new_mode = current_mode_param;  /* Stay in current mode */
        } else {
            /* Transition is allowed - apply mode-specific hysteresis */
            double hysteresis = CMXD_MODE_HYSTERESIS;
            
            /* Use smaller hysteresis for closing/laptop transitions for better responsiveness */
            bool is_closing_transition = (strcmp(current_mode_param, CMXD_PROTOCOL_MODE_CLOSING) == 0 && strcmp(new_mode, CMXD_PROTOCOL_MODE_LAPTOP) == 0) ||
                                        (strcmp(current_mode_param, CMXD_PROTOCOL_MODE_LAPTOP) == 0 && strcmp(new_mode, CMXD_PROTOCOL_MODE_CLOSING) == 0);
            
            if (is_closing_transition) {
                hysteresis = 3.0;  /* Smaller hysteresis for closing transitions */
            }
            
            /* Check if we need hysteresis to prevent mode flipping */
            if (strcmp(current_mode_param, new_mode) != 0) {
                /* Mode would change - check if we're near a boundary with bidirectional hysteresis */
                
                /* Forward transitions (increasing angle) */
                if ((strcmp(new_mode, CMXD_PROTOCOL_MODE_LAPTOP) == 0 && strcmp(current_mode_param, CMXD_PROTOCOL_MODE_CLOSING) == 0)) {
                    if (angle < CMXD_MODE_CLOSING_MAX + hysteresis) {
                        new_mode = current_mode_param;  /* Stay in closing mode */
                    }
                } else if ((strcmp(new_mode, CMXD_PROTOCOL_MODE_FLAT) == 0 && strcmp(current_mode_param, CMXD_PROTOCOL_MODE_LAPTOP) == 0)) {
                    if (angle < CMXD_MODE_LAPTOP_MAX + hysteresis) {
                        new_mode = current_mode_param;  /* Stay in laptop mode */
                    }
                } else if ((strcmp(new_mode, CMXD_PROTOCOL_MODE_TENT) == 0 && strcmp(current_mode_param, CMXD_PROTOCOL_MODE_FLAT) == 0)) {
                    if (angle < CMXD_MODE_FLAT_MAX + hysteresis) {
                        new_mode = current_mode_param;  /* Stay in flat mode */
                    }
                } else if ((strcmp(new_mode, CMXD_PROTOCOL_MODE_TABLET) == 0 && strcmp(current_mode_param, CMXD_PROTOCOL_MODE_TENT) == 0)) {
                    if (angle < CMXD_MODE_TENT_MAX + hysteresis) {
                        new_mode = current_mode_param;  /* Stay in tent mode */
                    }
                }
                
                /* Reverse transitions (decreasing angle) */
                else if ((strcmp(new_mode, CMXD_PROTOCOL_MODE_CLOSING) == 0 && strcmp(current_mode_param, CMXD_PROTOCOL_MODE_LAPTOP) == 0)) {
                    if (angle > CMXD_MODE_CLOSING_MAX - hysteresis) {
                        new_mode = current_mode_param;  /* Stay in laptop mode */
                    }
                } else if ((strcmp(new_mode, CMXD_PROTOCOL_MODE_LAPTOP) == 0 && strcmp(current_mode_param, CMXD_PROTOCOL_MODE_FLAT) == 0)) {
                    if (angle > CMXD_MODE_LAPTOP_MAX - hysteresis) {
                        new_mode = current_mode_param;  /* Stay in flat mode */
                    }
                } else if ((strcmp(new_mode, CMXD_PROTOCOL_MODE_FLAT) == 0 && strcmp(current_mode_param, CMXD_PROTOCOL_MODE_TENT) == 0)) {
                    if (angle > CMXD_MODE_FLAT_MAX - hysteresis) {
                        new_mode = current_mode_param;  /* Stay in tent mode */
                    }
                } else if ((strcmp(new_mode, CMXD_PROTOCOL_MODE_TENT) == 0 && strcmp(current_mode_param, CMXD_PROTOCOL_MODE_TABLET) == 0)) {
                    if (angle > CMXD_MODE_TENT_MAX - hysteresis) {
                        new_mode = current_mode_param;  /* Stay in tablet mode */
                    }
                }
            }
        }
    }
    
    /* Mode detection debug output reduced for cleaner format */
    return new_mode;
}

/* Get stable device mode with minimal complexity */
const char* cmxd_get_stable_device_mode(double angle, int orientation)
{
    /* Use default gravity confidence values for backward compatibility */
    return cmxd_get_stable_device_mode_with_gravity(angle, orientation, 9.8, 9.8, 0.0);
}

/* Get stable device mode with gravity confidence checking and sticky mode behavior */
const char* cmxd_get_stable_device_mode_with_gravity(double angle, int orientation,
                                                    double base_mag, double lid_mag, double total_horizontal)
{
    (void)orientation;  /* Not used in simplified version */
    
    /* Use mode-aware gravity confidence checking - more tolerant for current mode */
    bool gravity_confident_for_current = is_gravity_confident_for_mode(base_mag, lid_mag, total_horizontal, current_mode);
    
    const char* new_mode;
    
    if (!gravity_confident_for_current) {
        /* Gravity vectors are unreliable for current mode - but check if they'd be OK for target mode */
        
        /* Determine what mode the angle suggests */
        const char* angle_based_mode;
        if (angle < CMXD_MODE_CLOSING_MAX) {
            angle_based_mode = CMXD_PROTOCOL_MODE_CLOSING;
        } else if (angle < CMXD_MODE_LAPTOP_MAX) {
            angle_based_mode = CMXD_PROTOCOL_MODE_LAPTOP;
        } else if (angle < CMXD_MODE_FLAT_MAX) {
            angle_based_mode = CMXD_PROTOCOL_MODE_FLAT;
        } else if (angle < CMXD_MODE_TENT_MAX) {
            angle_based_mode = CMXD_PROTOCOL_MODE_TENT;
        } else {
            angle_based_mode = CMXD_PROTOCOL_MODE_TABLET;
        }
        
        /* Check if gravity would be confident for the target mode */
        bool gravity_confident_for_target = is_gravity_confident_for_mode(base_mag, lid_mag, total_horizontal, angle_based_mode);
        
        if (gravity_confident_for_target) {
            /* Readings are OK for the target mode - allow normal mode detection */
            new_mode = cmxd_get_device_mode(angle, current_mode);
            debug_log("Gravity OK for target mode %s (h_accel=%.1f) -> transitioning", angle_based_mode, total_horizontal);
        } else {
            /* Use lenient confidence check to see if we should enter true indeterminate state */
            /* Check if readings would be acceptable for tent mode (most lenient) */
            bool gravity_severely_unreliable = !is_gravity_confident_for_mode(base_mag, lid_mag, total_horizontal, CMXD_PROTOCOL_MODE_TENT);
            
            if (gravity_severely_unreliable) {
                /* Truly unreliable conditions - not even tent mode can handle this */
                new_mode = CMXD_MODE_INDETERMINATE;
                debug_log("Gravity severely unreliable (base_mag=%.1f, lid_mag=%.1f, h_accel=%.1f) -> indeterminate", 
                         base_mag, lid_mag, total_horizontal);
            } else {
                /* Moderately unreliable for both current and target mode - stick to current mode */
                new_mode = current_mode;
                debug_log("Gravity unstable for both %s and target %s mode (h_accel=%.1f) -> staying in %s", 
                         current_mode, angle_based_mode, total_horizontal, current_mode);
            }
        }
    } else {
        /* Gravity is confident for current mode - normal mode detection */
        new_mode = cmxd_get_device_mode(angle, current_mode);
        
        /* If we're coming out of indeterminate state, allow any reasonable mode */
        if (strcmp(current_mode, CMXD_MODE_INDETERMINATE) == 0) {
            debug_log("Gravity restored, transitioning from indeterminate -> %s (angle=%.1f°)", new_mode, angle);
            /* Allow transition but still require stability */
        }
    }
    
    /* Simple stability check */
    if (strcmp(current_mode, new_mode) == 0) {
        /* Mode unchanged - reset stability counter */
        stability_count = 0;
        candidate_mode = NULL;
        return current_mode;
    }
    
    /* Mode change candidate */
    if (candidate_mode == NULL || strcmp(candidate_mode, new_mode) != 0) {
        /* New candidate mode */
        candidate_mode = new_mode;
        stability_count = 1;
        debug_log("New candidate mode: %s (need %d more samples)", new_mode, CMXD_MODE_STABILITY_SAMPLES - 1);
        return current_mode;  /* Keep current mode for now */
    }
    
    /* Same candidate as before */
    stability_count++;
    if (stability_count >= CMXD_MODE_STABILITY_SAMPLES) {
        /* Candidate mode is stable - make the switch */
        debug_log("Mode change confirmed: %s -> %s", current_mode, new_mode);
        current_mode = new_mode;
        candidate_mode = NULL;
        stability_count = 0;
        return current_mode;
    }
    
    debug_log("Candidate mode %s stability: %d/%d", new_mode, stability_count, CMXD_MODE_STABILITY_SAMPLES);
    return current_mode;  /* Keep current mode until stable */
}

const char* cmxd_get_last_mode(void)
{
    return current_mode;
}

void cmxd_modes_set_verbose(bool verbose)
{
    verbose_logging = verbose;
}