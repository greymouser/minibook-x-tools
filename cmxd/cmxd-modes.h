/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Mode detection for CMXD (Chuwi Minibook X Daemon)
 * 
 * Device mode detection based on hinge angle calculations using a 0-360°
 * measurement system with hysteresis and stability filtering.
 * 
 * Copyright (c) 2025 Armando DiCianno <armando@noonshy.com>
 */

#ifndef CMXD_MODES_H
#define CMXD_MODES_H

#include <stdbool.h>

/* Device mode constants */
#define CMXD_MODE_CLOSING "closing"
#define CMXD_MODE_LAPTOP "laptop"
#define CMXD_MODE_FLAT "flat"
#define CMXD_MODE_TENT "tent"
#define CMXD_MODE_TABLET "tablet"

#define CMXD_MODE_UNKNOWN -1

/* 
 * Mode boundary angles - these match the constants defined in cmxd-modes.c
 * See the implementation file for detailed angle range descriptions.
 */
extern const double CMXD_MODE_CLOSING_MAX;
extern const double CMXD_MODE_LAPTOP_MAX;
extern const double CMXD_MODE_FLAT_MAX;
extern const double CMXD_MODE_TENT_MAX;
extern const double CMXD_MODE_TABLET_MAX;

/* Filtering and stability constants */
extern const double CMXD_MODE_HYSTERESIS;
extern const int CMXD_MODE_STABILITY_SAMPLES;
extern const int CMXD_ORIENTATION_FREEZE_DURATION;

void cmxd_modes_init(void);
void cmxd_modes_reset(void);

const char* cmxd_get_device_mode(double angle, const char* current_mode);
const char* cmxd_get_stable_device_mode(double angle, int orientation);

bool cmxd_mode_has_changed(const char* current_mode);
const char* cmxd_get_last_mode(void);
bool cmxd_is_tablet_mode(const char* mode);

void cmxd_modes_set_verbose(bool verbose);
void cmxd_modes_set_log_debug(void (*func)(const char *fmt, ...));

#endif /* CMXD_MODES_H */