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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/* 
 * Correct mode boundaries based on physical hinge angles:
 *   Closing: 0° to 45°     (device closing to nearly closed)
 *   Laptop:  45° to 160°   (traditional laptop position, 90° ideal)
 *   Flat:    160° to 240°  (opening towards flat, ~180° = completely flat)
 *   Tent:    240° to 330°  (tent mode - lid folded back)
 *   Tablet:  330° to 360°  (fully folded back for tablet use)
 *
 * These ranges account for the 360° hinge angle calculation.
 */
const double CMXD_MODE_CLOSING_MAX = 45.0;      
const double CMXD_MODE_LAPTOP_MAX = 160.0;      
const double CMXD_MODE_FLAT_MAX = 240.0;        
const double CMXD_MODE_TENT_MAX = 330.0;        
const double CMXD_MODE_TABLET_MAX = 360.0;      

/* Simple hysteresis to prevent mode jitter */
const double CMXD_MODE_HYSTERESIS = 10.0;

/* Require a few stable readings before mode change */
const int CMXD_MODE_STABILITY_SAMPLES = 3;

/* Not needed with reliable mount matrices */
const int CMXD_ORIENTATION_FREEZE_DURATION = 0;

/* Module state */
static const char* current_mode = CMXD_MODE_LAPTOP;
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
    current_mode = CMXD_MODE_LAPTOP;
    candidate_mode = NULL;
    stability_count = 0;
    verbose_logging = false;
}

void cmxd_modes_reset(void)
{
    cmxd_modes_init();
}

/* Simplified mode determination based on angle */
const char* cmxd_get_device_mode(double angle, const char* current_mode_param)
{
    if (angle < 0) {
        return CMXD_MODE_LAPTOP;  /* Default for invalid readings */
    }
    
    /* Determine base mode from angle */
    const char* new_mode;
    if (angle < CMXD_MODE_CLOSING_MAX) {
        new_mode = CMXD_MODE_CLOSING;
    } else if (angle < CMXD_MODE_LAPTOP_MAX) {
        new_mode = CMXD_MODE_LAPTOP;
    } else if (angle < CMXD_MODE_FLAT_MAX) {
        new_mode = CMXD_MODE_FLAT;
    } else if (angle < CMXD_MODE_TENT_MAX) {
        new_mode = CMXD_MODE_TENT;
    } else {
        new_mode = CMXD_MODE_TABLET;
    }
    
    /* Apply simple hysteresis if we have a current mode */
    if (current_mode_param) {
        double hysteresis = CMXD_MODE_HYSTERESIS;
        
        /* Check if we need hysteresis to prevent mode flipping */
        if (strcmp(current_mode_param, new_mode) != 0) {
            /* Mode would change - check if we're near a boundary */
            if ((strcmp(new_mode, CMXD_MODE_LAPTOP) == 0 && strcmp(current_mode_param, CMXD_MODE_CLOSING) == 0)) {
                if (angle < CMXD_MODE_CLOSING_MAX + hysteresis) {
                    new_mode = current_mode_param;  /* Stay in current mode */
                }
            } else if ((strcmp(new_mode, CMXD_MODE_FLAT) == 0 && strcmp(current_mode_param, CMXD_MODE_LAPTOP) == 0)) {
                if (angle < CMXD_MODE_LAPTOP_MAX + hysteresis) {
                    new_mode = current_mode_param;
                }
            } else if ((strcmp(new_mode, CMXD_MODE_TENT) == 0 && strcmp(current_mode_param, CMXD_MODE_FLAT) == 0)) {
                if (angle < CMXD_MODE_FLAT_MAX + hysteresis) {
                    new_mode = current_mode_param;
                }
            } else if ((strcmp(new_mode, CMXD_MODE_TABLET) == 0 && strcmp(current_mode_param, CMXD_MODE_TENT) == 0)) {
                if (angle < CMXD_MODE_TENT_MAX + hysteresis) {
                    new_mode = current_mode_param;
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
    (void)orientation;  /* Not used in simplified version */
    
    const char* new_mode = cmxd_get_device_mode(angle, current_mode);
    
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

/* Check if mode has changed from last reading */
bool cmxd_mode_has_changed(const char* mode)
{
    static const char* last_reported_mode = CMXD_MODE_LAPTOP;
    
    bool changed = (strcmp(mode, last_reported_mode) != 0);
    if (changed) {
        debug_log("Mode change: %s -> %s", last_reported_mode, mode);
        last_reported_mode = mode;
    }
    
    return changed;
}

const char* cmxd_get_last_mode(void)
{
    return current_mode;
}

/* Check if mode represents tablet-like usage */
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