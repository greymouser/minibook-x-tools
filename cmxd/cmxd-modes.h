/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Mode detection for CMXD (Chuwi Minibook X Daemon)
 * 
 * Device mode detection based on hinge angle calculations
 * 
 * Copyright (c) 2025 Armando DiCianno <armando@noonshy.com>
 */

#ifndef CMXD_MODES_H
#define CMXD_MODES_H

#include <stdbool.h>

/* Mode string constants */
#define CMXD_MODE_CLOSING "closing"
#define CMXD_MODE_LAPTOP "laptop"
#define CMXD_MODE_FLAT "flat"
#define CMXD_MODE_TENT "tent"
#define CMXD_MODE_TABLET "tablet"

/* Mode unknown state */
#define CMXD_MODE_UNKNOWN -1

/* Mode boundary angles */
extern const double CMXD_MODE_CLOSING_MAX;    /* 0°-45°: Closing mode */
extern const double CMXD_MODE_LAPTOP_MAX;     /* 45°-145°: Laptop mode */
extern const double CMXD_MODE_FLAT_MAX;       /* 145°-180°: Flat mode */
extern const double CMXD_MODE_TENT_MAX;       /* State-tracked: past flat detection */
extern const double CMXD_MODE_TABLET_MAX;     /* State-tracked: fully folded detection */

/* Hysteresis constants */
extern const double CMXD_MODE_HYSTERESIS;          /* Standard hysteresis */
extern const double CMXD_TABLET_MODE_HYSTERESIS;   /* Enhanced tablet mode hysteresis */

/* Stability requirements */
extern const int CMXD_MODE_STABILITY_SAMPLES;         /* Standard stability requirement */
extern const int CMXD_TABLET_MODE_STABILITY_SAMPLES;  /* Enhanced tablet mode stability */

/* Orientation freeze duration */
extern const int CMXD_ORIENTATION_FREEZE_DURATION;    /* Samples to freeze after orientation change */

/* Initialize mode detection module */
void cmxd_modes_init(void);

/* Reset mode detection state */
void cmxd_modes_reset(void);

/* Get device mode based on hinge angle with hysteresis */
const char* cmxd_get_device_mode(double angle, const char* current_mode);

/* Get stable device mode with enhanced validation and filtering */
const char* cmxd_get_stable_device_mode(double angle, int orientation);

/* Check if mode has changed from last reading */
bool cmxd_mode_has_changed(const char* current_mode);

/* Get the last detected mode */
const char* cmxd_get_last_mode(void);

/* Check if a mode represents tablet mode */
bool cmxd_is_tablet_mode(const char* mode);

/* Set verbose logging for mode detection */
void cmxd_modes_set_verbose(bool verbose);

/* Set the debug logging function for mode module */
void cmxd_modes_set_log_debug(void (*func)(const char *fmt, ...));

#endif /* CMXD_MODES_H */