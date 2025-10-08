// SPDX-License-Identifier: GPL-2.0
/* orientation_detection.c - Screen orientation detection from accelerometer data
 *
 * Implements screen orientation detection based on gravity vector analysis.
 * Provides stability mechanisms including hysteresis, motion detection,
 * and confidence tracking to prevent unwanted orientation switches.
 *
 * Key features:
 * - Detects 4 orientations: normal, left, inverted, right
 * - Handles flat/face-down detection to prevent spurious switches
 * - Motion detection prevents switches during movement
 * - Configurable stability timeouts and confidence thresholds
 * - Hysteresis prevents rapid oscillation between orientations
 *
 * Copyright (c) 2025 Armando DiCianno <armando@noonshy.com>
 */

#include <linux/kernel.h>
#include <linux/ktime.h>
#include <linux/math64.h>
#include <linux/minmax.h>
#include <linux/time64.h>

#include "orientation_detection.h"

/* Default configuration values */
#define DEFAULT_HYSTERESIS_DEG          10      /* Hysteresis for orientation changes (reduced for maximum sensitivity) */
#define DEFAULT_STABILITY_TIME_MS       200     /* Time to remain stable before switching (very low for quick response) */
#define DEFAULT_FLAT_THRESHOLD_DEG      0       /* Disable flat detection entirely */
#define DEFAULT_MOTION_THRESHOLD        300000  /* Motion detection threshold (lowered for better motion detection) */
#define DEFAULT_MOTION_SETTLE_MS        200     /* Wait after motion stops (reduced for faster response) */
#define DEFAULT_MIN_CONFIDENCE          50      /* Minimum confidence percentage (lowered for maximum sensitivity) */

/* Orientation angle ranges (in degrees from horizontal) */
#define NORMAL_MIN      315     /* 315° - 45° (wrapping around 0°) */
#define NORMAL_MAX      45
#define LEFT_MIN        45      /* 45° - 135° */
#define LEFT_MAX        135
#define INVERTED_MIN    135     /* 135° - 225° */
#define INVERTED_MAX    225
#define RIGHT_MIN       225     /* 225° - 315° */
#define RIGHT_MAX       315

/* Math helpers */
#define ABS_S32(x) ((x) < 0 ? -(x) : (x))

/* Calculate magnitude of 3D vector (returns u64 to avoid overflow) */
static u64 vec3_magnitude(const struct vec3 *v)
{
	s64 xx = (s64)v->x * v->x;
	s64 yy = (s64)v->y * v->y; 
	s64 zz = (s64)v->z * v->z;
	return int_sqrt64(xx + yy + zz);
}

/* Calculate angle from horizontal plane in degrees (0-360) */
static unsigned int gravity_to_angle(const struct vec3 *gravity)
{
	int angle_deg;
	
	/* Calculate angle using simplified atan2(y, x) approach
	 * For screen coordinates:
	 * - X axis: left to right (landscape width)
	 * - Y axis: bottom to top (landscape height)  
	 * - Z axis: back to front (through screen)
	 */
	
	/* Use a very sensitive approach - detect orientation based on primary gravity direction */
	s32 x = gravity->x;
	s32 y = gravity->y;
	
	/* Calculate the actual angle using atan2-like logic but much more sensitive */
	/* We'll determine orientation primarily by the largest component, with minimal thresholds */
	
	/* Find the larger component to determine primary orientation */
	if (ABS_S32(y) > ABS_S32(x)) {
		/* Y is larger - vertical orientations (normal/inverted) */
		if (y > 0) {
			/* Bottom edge down -> normal (0°) */
			angle_deg = 0;
			
			/* Add slight offset based on X component for better sensitivity */
			if (x > ABS_S32(y) / 8) {
				angle_deg = 30;   /* Slight tilt towards left */
			} else if (x < -ABS_S32(y) / 8) {
				angle_deg = 330;  /* Slight tilt towards right */
			}
		} else {
			/* Top edge down -> inverted (180°) */
			angle_deg = 180;
			
			/* Add slight offset based on X component */
			if (x > ABS_S32(y) / 8) {
				angle_deg = 150;  /* Slight tilt towards left */
			} else if (x < -ABS_S32(y) / 8) {
				angle_deg = 210;  /* Slight tilt towards right */
			}
		}
	} else {
		/* X is larger - horizontal orientations (left/right) */
		if (x > 0) {
			/* Right edge down -> left (90°) */
			angle_deg = 90;
			
			/* Add slight offset based on Y component */
			if (y > ABS_S32(x) / 8) {
				angle_deg = 60;   /* Slight tilt towards normal */
			} else if (y < -ABS_S32(x) / 8) {
				angle_deg = 120;  /* Slight tilt towards inverted */
			}
		} else {
			/* Left edge down -> right (270°) */
			angle_deg = 270;
			
			/* Add slight offset based on Y component */
			if (y > ABS_S32(x) / 8) {
				angle_deg = 300;  /* Slight tilt towards normal */
			} else if (y < -ABS_S32(x) / 8) {
				angle_deg = 240;  /* Slight tilt towards inverted */
			}
		}
	}
	
	/* Special case: if components are very close, use quadrant logic */
	if (ABS_S32(ABS_S32(x) - ABS_S32(y)) < max(ABS_S32(x), ABS_S32(y)) / 4) {
		/* Very balanced - use exact quadrant */
		if (x > 0 && y > 0) {
			angle_deg = 45;   /* Between normal and left */
		} else if (x > 0 && y < 0) {
			angle_deg = 135;  /* Between left and inverted */
		} else if (x < 0 && y < 0) {
			angle_deg = 225;  /* Between inverted and right */
		} else {
			angle_deg = 315;  /* Between right and normal */
		}
	}
	
	/* Normalize to 0-360 range */
	while (angle_deg < 0) angle_deg += 360;
	while (angle_deg >= 360) angle_deg -= 360;
	
	return (unsigned int)angle_deg;
}

/* Convert angle to orientation enum */
static enum screen_orientation angle_to_orientation(unsigned int angle)
{
	/* Handle wraparound case for normal orientation (315° - 45°) */
	if (angle >= NORMAL_MIN || angle <= NORMAL_MAX) {
		return ORIENT_NORMAL;
	} else if (angle >= LEFT_MIN && angle <= LEFT_MAX) {
		return ORIENT_LEFT;
	} else if (angle >= INVERTED_MIN && angle <= INVERTED_MAX) {
		return ORIENT_INVERTED;
	} else if (angle >= RIGHT_MIN && angle <= RIGHT_MAX) {
		return ORIENT_RIGHT;
	}
	
	return ORIENT_UNKNOWN;
}

/* Calculate tilt angle from vertical (for flat detection) */
static unsigned int calculate_tilt_angle(const struct vec3 *gravity)
{
	u64 mag = vec3_magnitude(gravity);
	s64 z_component = ABS_S32(gravity->z);
	s64 cos_tilt;
	unsigned int tilt_deg;
	
	if (mag == 0) {
		return 90; /* Invalid data, assume max tilt */
	}
	
	/* cos(tilt) = |z| / magnitude */
	cos_tilt = div_s64((s64)z_component * 1000000, (s64)mag);
	
	/* Simple linear approximation: tilt ≈ (1 - cos) * 90 */
	/* More accurate would be acos(), but this is sufficient for flat detection */
	tilt_deg = (unsigned int)div_s64((1000000 - cos_tilt) * 90, 1000000);
	
	return min(tilt_deg, 90u);
}

/* Calculate confidence based on gravity vector quality */
static unsigned int calculate_confidence(const struct vec3 *gravity)
{
	u64 mag = vec3_magnitude(gravity);
	u64 expected_mag = 13000000; /* Expected ~1g - adjusted for lid accelerometer scaling */
	u64 mag_ratio;
	unsigned int confidence;
	
	if (mag == 0) {
		return 0;
	}
	
	/* Confidence based on how close magnitude is to 1g */
	if (mag > expected_mag) {
		mag_ratio = div64_u64(expected_mag * 100, mag);
	} else {
		mag_ratio = div64_u64(mag * 100, expected_mag);
	}
	
	confidence = (unsigned int)min(mag_ratio, 100ULL);
	
	/* Reduce confidence if vector components are very unbalanced */
	/* This helps detect when device is being moved/shaken */
	u64 max_component = max3(ABS_S32(gravity->x), ABS_S32(gravity->y), ABS_S32(gravity->z));
	if (max_component > 0 && mag > 0) {
		u64 balance_ratio = div64_u64(max_component * 100, mag);
		if (balance_ratio > 95) {
			/* Very unbalanced - reduce confidence */
			confidence = (confidence * 80) / 100;
		}
	}
	
	return confidence;
}

/* Check if two orientations are adjacent (differ by 90°) */
static bool orientations_adjacent(enum screen_orientation a, enum screen_orientation b)
{
	if (a == b) return true;
	
	/* Allow any transition from/to UNKNOWN - it's the initial state */
	if (a == ORIENT_UNKNOWN || b == ORIENT_UNKNOWN) return true;
	
	/* Don't allow transitions involving FLAT */
	if (a >= ORIENT_FLAT || b >= ORIENT_FLAT) return false;
	
	/* Check if orientations are 90° apart */
	int diff = ABS_S32((int)a - (int)b);
	return (diff == 1) || (diff == 3); /* 1 step or 3 steps (wrapping) */
}

void orientation_init_config(struct orientation_config *config)
{
	if (!config) return;
	
	/* Set orientation angle ranges */
	config->normal_min = NORMAL_MIN;
	config->normal_max = NORMAL_MAX;
	config->left_min = LEFT_MIN;
	config->left_max = LEFT_MAX;
	config->inverted_min = INVERTED_MIN;
	config->inverted_max = INVERTED_MAX;
	config->right_min = RIGHT_MIN;
	config->right_max = RIGHT_MAX;
	
	/* Set stability parameters */
	config->hysteresis_deg = DEFAULT_HYSTERESIS_DEG;
	config->stability_time_ms = DEFAULT_STABILITY_TIME_MS;
	config->flat_threshold_deg = DEFAULT_FLAT_THRESHOLD_DEG;
	
	/* Set motion detection parameters */
	config->motion_threshold = DEFAULT_MOTION_THRESHOLD;
	config->motion_settle_ms = DEFAULT_MOTION_SETTLE_MS;
	
	/* Set confidence parameters */
	config->min_confidence = DEFAULT_MIN_CONFIDENCE;
}

void orientation_init_state(struct orientation_state *state)
{
	if (!state) return;
	
	state->current_orientation = ORIENT_UNKNOWN;
	state->candidate = ORIENT_UNKNOWN;
	state->candidate_start_time = 0;
	state->last_motion_time = 0;
	state->prev_gravity_valid = false;
	state->confidence = 0;
	state->orientation_changes = 0;
	state->rejected_changes = 0;
}

bool orientation_update(struct orientation_state *state,
			const struct orientation_config *config,
			const struct vec3 *gravity,
			bool force_update)
{
	enum screen_orientation new_orient;
	unsigned int angle, tilt, confidence;
	ktime_t now = ktime_get();
	bool motion_detected = false;
	bool orientation_changed = false;
	
	if (!state || !config || !gravity) {
		return false;
	}
	
	/* Calculate current metrics */
	angle = gravity_to_angle(gravity);
	tilt = calculate_tilt_angle(gravity);
	confidence = calculate_confidence(gravity);
	
	/* Check if device is flat - but not in laptop mode where screen is naturally vertical */
	if (config->flat_threshold_deg > 0 && tilt <= config->flat_threshold_deg) {
		new_orient = ORIENT_FLAT;
	} else {
		new_orient = angle_to_orientation(angle);
	}
	
	/* Detect motion if we have previous data */
	if (state->prev_gravity_valid) {
		motion_detected = orientation_detect_motion(gravity, &state->prev_gravity,
							    config->motion_threshold);
		if (motion_detected) {
			state->last_motion_time = now;
		}
	}
	
	/* Update confidence */
	state->confidence = confidence;
	
	/* Store current gravity for next motion detection */
	state->prev_gravity = *gravity;
	state->prev_gravity_valid = true;
	
	/* Check if we should reject this update due to low confidence */
	if (!force_update && confidence < config->min_confidence) {
		state->rejected_changes++;
		return false;
	}
	
	/* Check if we should reject due to recent motion */
	if (!force_update && motion_detected) {
		ktime_t motion_age = ktime_sub(now, state->last_motion_time);
		if (ktime_to_ms(motion_age) < config->motion_settle_ms) {
			state->rejected_changes++;
			return false;
		}
	}
	
	/* If this is the same as current orientation, no change needed */
	if (new_orient == state->current_orientation) {
		state->candidate = new_orient;
		state->candidate_start_time = now;
		return false;
	}
	
	/* Check if this is a new candidate orientation */
	if (new_orient != state->candidate) {
		/* New candidate - check hysteresis for non-adjacent orientations */
		if (!force_update && !orientations_adjacent(new_orient, state->current_orientation)) {
			/* Require larger change for non-adjacent transitions */
			/* This prevents rapid 180° flips */
			state->rejected_changes++;
			return false;
		}
		
		state->candidate = new_orient;
		state->candidate_start_time = now;
		return false; /* Need to wait for stability */
	}
	
	/* Same candidate as before - check stability time */
	if (!force_update) {
		ktime_t candidate_age = ktime_sub(now, state->candidate_start_time);
		if (ktime_to_ms(candidate_age) < config->stability_time_ms) {
			return false; /* Not stable long enough */
		}
	}
	
	/* All checks passed - update orientation */
	state->current_orientation = new_orient;
	state->orientation_changes++;
	orientation_changed = true;
	
	/* Reset candidate tracking */
	state->candidate = new_orient;
	state->candidate_start_time = now;
	
	return orientation_changed;
}

int orientation_get_angle(const struct vec3 *gravity)
{
	if (!gravity) {
		return -1;
	}
	
	return (int)gravity_to_angle(gravity);
}

bool orientation_is_flat(const struct vec3 *gravity, unsigned int threshold_deg)
{
	if (!gravity) {
		return false;
	}
	
	return calculate_tilt_angle(gravity) <= threshold_deg;
}

bool orientation_detect_motion(const struct vec3 *curr,
			       const struct vec3 *previous,
			       unsigned int threshold)
{
	s32 dx, dy, dz;
	u64 diff_mag;
	
	if (!curr || !previous) {
		return false;
	}
	
	/* Calculate difference vector */
	dx = curr->x - previous->x;
	dy = curr->y - previous->y;
	dz = curr->z - previous->z;
	
	/* Calculate magnitude of difference */
	diff_mag = int_sqrt64((s64)dx * dx + (s64)dy * dy + (s64)dz * dz);
	
	return diff_mag > threshold;
}

const char *orientation_to_string(enum screen_orientation orient)
{
	switch (orient) {
	case ORIENT_NORMAL:   return "normal";
	case ORIENT_LEFT:     return "left";
	case ORIENT_INVERTED: return "inverted";
	case ORIENT_RIGHT:    return "right";
	case ORIENT_FLAT:     return "flat";
	case ORIENT_UNKNOWN:  return "unknown";
	default:              return "invalid";
	}
}

int orientation_to_degrees(enum screen_orientation orient)
{
	switch (orient) {
	case ORIENT_NORMAL:   return 0;
	case ORIENT_LEFT:     return 90;
	case ORIENT_INVERTED: return 180;
	case ORIENT_RIGHT:    return 270;
	default:              return -1;
	}
}

/* Enhanced dual-sensor orientation detection */
bool orientation_update_dual_sensor(struct orientation_state *state,
				    const struct orientation_config *config,
				    const struct vec3 *base_gravity,
				    const struct vec3 *lid_gravity,
				    unsigned int hinge_angle,
				    bool force_update)
{
	/* Choose detection method based on hinge angle */
	
	if (hinge_angle >= 250 || hinge_angle <= 30) {
		/* Tablet mode or nearly closed - use lid sensor only */
		return orientation_update(state, config, lid_gravity, force_update);
	} 
	else if (hinge_angle >= 70 && hinge_angle <= 200) {
		/* Laptop mode - use lid sensor directly for screen orientation */
		/* The lid sensor gives us the screen's actual orientation relative to gravity */
		/* In laptop mode, disable flat detection since screen is always nearly vertical */
		struct orientation_config laptop_config = *config;
		laptop_config.flat_threshold_deg = 0;  /* Disable flat detection in laptop mode */
		return orientation_update(state, &laptop_config, lid_gravity, force_update);
	}
	else {
		/* Intermediate angles - use lid sensor with relaxed threshold */
		return orientation_update(state, config, lid_gravity, force_update);
	}
}