/* SPDX-License-Identifier: GPL-2.0 */
/* orientation_detection.h - Screen orientation detection from accelerometer data
 *
 * Detects screen orientation based on gravity vector from base accelerometer.
 * Provides stability mechanisms to prevent rapid switching when device is
 * moved or set down on surfaces.
 *
 * Copyright (c) 2025 Armando DiCianno <armando@noonshy.com>
 */

#ifndef _ORIENTATION_DETECTION_H
#define _ORIENTATION_DETECTION_H

#include <linux/types.h>
#include <linux/time64.h>

/* Screen orientation states */
enum screen_orientation {
	ORIENT_NORMAL = 0,      /* 0°   - normal landscape */
	ORIENT_LEFT = 1,        /* 90°  - rotated left (portrait) */
	ORIENT_INVERTED = 2,    /* 180° - inverted landscape */
	ORIENT_RIGHT = 3,       /* 270° - rotated right (portrait) */
	ORIENT_FLAT = 4,        /* device laying flat (undefined orientation) */
	ORIENT_UNKNOWN = 5      /* insufficient data or error */
};

/* 3D vector structure (reuse from main module) */
struct vec3 {
	s32 x, y, z;
};

/* Orientation detection configuration */
struct orientation_config {
	/* Angle thresholds for orientation detection (degrees) */
	unsigned int normal_min, normal_max;     /* Normal landscape range */
	unsigned int left_min, left_max;         /* Left rotation range */
	unsigned int inverted_min, inverted_max; /* Inverted landscape range */
	unsigned int right_min, right_max;       /* Right rotation range */
	
	/* Stability requirements */
	unsigned int hysteresis_deg;      /* Required angle change to switch (degrees) */
	unsigned int stability_time_ms;   /* Time to remain stable before switching */
	unsigned int flat_threshold_deg;  /* If tilt < this, consider "flat" */
	
	/* Motion detection (prevent switching during movement) */
	unsigned int motion_threshold;    /* Acceleration change threshold */
	unsigned int motion_settle_ms;    /* Time to wait after motion stops */
	
	/* Confidence requirements */
	unsigned int min_confidence;     /* Minimum confidence to change orientation */
};

/* Orientation detection state */
struct orientation_state {
	enum screen_orientation current_orientation;   /* Current reported orientation */
	enum screen_orientation candidate;      /* Potential new orientation */
	
	/* Timing for stability */
	ktime_t candidate_start_time;          /* When candidate was first detected */
	ktime_t last_motion_time;              /* Last time significant motion detected */
	
	/* Previous measurements for motion detection */
	struct vec3 prev_gravity;              /* Previous gravity vector */
	bool prev_gravity_valid;               /* Whether prev_gravity is valid */
	
	/* Confidence tracking */
	unsigned int confidence;               /* Current measurement confidence (0-100) */
	
	/* Statistics */
	unsigned long orientation_changes;     /* Total orientation changes */
	unsigned long rejected_changes;        /* Changes rejected due to instability */
};

/* Function prototypes */

/**
 * orientation_init_config() - Initialize orientation detection configuration
 * @config: Configuration structure to initialize
 * 
 * Sets up default configuration values for orientation detection.
 */
void orientation_init_config(struct orientation_config *config);

/**
 * orientation_init_state() - Initialize orientation detection state  
 * @state: State structure to initialize
 *
 * Initializes the orientation detection state to defaults.
 */
void orientation_init_state(struct orientation_state *state);

/**
 * orientation_update() - Update orientation based on gravity vector
 * @state: Current orientation state
 * @config: Configuration parameters
 * @gravity: Current gravity vector from base accelerometer
 * @force_update: If true, bypass stability checks
 *
 * Analyzes the gravity vector to determine screen orientation.
 * Applies stability and confidence checks before reporting changes.
 *
 * Returns: true if orientation changed, false otherwise
 */
bool orientation_update(struct orientation_state *state,
			const struct orientation_config *config,
			const struct vec3 *gravity,
			bool force_update);

/**
 * orientation_update_dual_sensor() - Update orientation using both base and lid sensors
 * @state: Current orientation state
 * @config: Configuration parameters  
 * @base_gravity: Gravity vector from base accelerometer
 * @lid_gravity: Gravity vector from lid accelerometer
 * @hinge_angle: Current hinge angle in degrees (0-360)
 * @force_update: If true, bypass stability checks
 *
 * Enhanced orientation detection that uses both sensors and hinge angle
 * to determine screen orientation in various laptop positions and modes.
 *
 * Returns: true if orientation changed, false otherwise
 */
bool orientation_update_dual_sensor(struct orientation_state *state,
				    const struct orientation_config *config,
				    const struct vec3 *base_gravity,
				    const struct vec3 *lid_gravity, 
				    unsigned int hinge_angle,
				    bool force_update);

/**
 * orientation_get_angle() - Get the current screen rotation angle
 * @gravity: Current gravity vector from base accelerometer
 *
 * Returns the screen rotation angle in degrees (0, 90, 180, 270)
 * based on the gravity vector. Does not apply stability filtering.
 *
 * Returns: rotation angle in degrees, or negative value on error
 */
int orientation_get_angle(const struct vec3 *gravity);

/**
 * orientation_is_flat() - Check if device is laying flat
 * @gravity: Current gravity vector from base accelerometer
 * @threshold_deg: Maximum tilt angle to consider "flat"
 *
 * Determines if the device is laying flat on a surface.
 *
 * Returns: true if device is flat, false otherwise
 */
bool orientation_is_flat(const struct vec3 *gravity, unsigned int threshold_deg);

/**
 * orientation_detect_motion() - Detect if device is in motion
 * @curr: Current gravity vector
 * @previous: Previous gravity vector  
 * @threshold: Motion detection threshold
 *
 * Compares current and previous gravity measurements to detect motion.
 *
 * Returns: true if significant motion detected, false otherwise
 */
bool orientation_detect_motion(const struct vec3 *curr,
			       const struct vec3 *previous,
			       unsigned int threshold);

/**
 * orientation_to_string() - Convert orientation enum to string
 * @orient: Orientation value
 *
 * Returns: String representation of orientation
 */
const char *orientation_to_string(enum screen_orientation orient);

/**
 * orientation_to_degrees() - Convert orientation to rotation degrees
 * @orient: Orientation value
 *
 * Returns: Rotation angle in degrees (0, 90, 180, 270) or -1 for invalid
 */
int orientation_to_degrees(enum screen_orientation orient);

#endif /* _ORIENTATION_DETECTION_H */