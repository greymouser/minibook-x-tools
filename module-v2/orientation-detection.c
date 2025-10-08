// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Chuwi Minibook X Platform Driver - Orientation Detection
 *
 * Copyright (C) 2025 Your Name
 *
 * Screen orientation detection based on accelerometer data analysis.
 * Supports detection of normal, left, inverted, and right orientations
 * with configurable stability and confidence thresholds.
 */

#include <linux/device.h>
#include <linux/input.h>
#include <linux/math64.h>
#include <linux/jiffies.h>

#include "chuwi-minibook-x.h"

/* Mathematical constants scaled by 1e6 for fixed-point arithmetic */
#define PI_SCALED          3141592
#define GRAVITY_THRESHOLD  500000  /* Minimum gravity magnitude for valid orientation */

/**
 * chuwi_minibook_x_vec3_magnitude - Calculate vector magnitude
 * @v: Input vector
 *
 * Returns: Magnitude scaled by 1e6
 */
u64 chuwi_minibook_x_vec3_magnitude(const struct vec3 *v)
{
	u64 x2 = (u64)v->x * v->x;
	u64 y2 = (u64)v->y * v->y;
	u64 z2 = (u64)v->z * v->z;
	u64 sum = x2 + y2 + z2;
	
	/* Use integer square root for magnitude calculation */
	return int_sqrt64(sum);
}

/**
 * chuwi_minibook_x_vec3_normalize - Normalize vector to unit length
 * @v: Vector to normalize
 * @result: Normalized result (scaled by 1e6)
 *
 * Returns: 0 on success, -EINVAL if vector is too small
 */
int chuwi_minibook_x_vec3_normalize(const struct vec3 *v, struct vec3 *result)
{
	u64 magnitude = chuwi_minibook_x_vec3_magnitude(v);
	
	if (magnitude < 1000) {  /* Avoid division by very small numbers */
		return -EINVAL;
	}
	
	/* Scale by 1e6 and divide by magnitude */
	result->x = div64_u64((s64)v->x * 1000000, magnitude);
	result->y = div64_u64((s64)v->y * 1000000, magnitude);
	result->z = div64_u64((s64)v->z * 1000000, magnitude);
	
	return 0;
}

/**
 * chuwi_minibook_x_vec3_dot - Calculate dot product of two vectors
 * @a: First vector
 * @b: Second vector
 *
 * Returns: Dot product scaled appropriately
 */
s64 chuwi_minibook_x_vec3_dot(const struct vec3 *a, const struct vec3 *b)
{
	s64 dot = (s64)a->x * b->x + (s64)a->y * b->y + (s64)a->z * b->z;
	return dot;
}

/**
 * chuwi_minibook_x_calculate_angle - Calculate hinge angle between accelerometers
 * @base_accel: Base accelerometer reading
 * @lid_accel: Lid accelerometer reading  
 * @hinge_axis: Hinge rotation axis
 * @signed_mode: Whether to use signed angle calculation (0-360°) or unsigned (0-180°)
 *
 * Returns: Angle in degrees (0-360 for signed mode, 0-180 for unsigned mode)
 */
u32 chuwi_minibook_x_calculate_angle(const struct vec3 *base_accel,
				     const struct vec3 *lid_accel,
				     const struct vec3 *hinge_axis,
				     bool signed_mode)
{
	struct vec3 base_norm, lid_norm, hinge_norm;
	s64 dot_product, cross_dot;
	u64 angle_rad_scaled, angle_deg;
	struct vec3 cross_product;
	int ret;

	/* Normalize all vectors */
	ret = chuwi_minibook_x_vec3_normalize(base_accel, &base_norm);
	if (ret)
		return 0;
		
	ret = chuwi_minibook_x_vec3_normalize(lid_accel, &lid_norm);
	if (ret)
		return 0;
		
	ret = chuwi_minibook_x_vec3_normalize(hinge_axis, &hinge_norm);
	if (ret)
		return 0;

	/* Calculate dot product for angle */
	dot_product = chuwi_minibook_x_vec3_dot(&base_norm, &lid_norm);
	
	/* Clamp dot product to valid range for acos */
	if (dot_product > 1000000)
		dot_product = 1000000;
	else if (dot_product < -1000000)
		dot_product = -1000000;

	/* Convert to angle using lookup table or approximation */
	angle_rad_scaled = chuwi_minibook_x_acos_scaled(dot_product);
	angle_deg = div64_u64(angle_rad_scaled * 180, PI_SCALED);

	/* For signed mode, determine direction using cross product */
	if (signed_mode) {
		/* Calculate cross product: base × lid */
		cross_product.x = div64_s64((s64)base_norm.y * lid_norm.z - (s64)base_norm.z * lid_norm.y, 1000000);
		cross_product.y = div64_s64((s64)base_norm.z * lid_norm.x - (s64)base_norm.x * lid_norm.z, 1000000);
		cross_product.z = div64_s64((s64)base_norm.x * lid_norm.y - (s64)base_norm.y * lid_norm.x, 1000000);
		
		/* Check direction relative to hinge axis */
		cross_dot = chuwi_minibook_x_vec3_dot(&cross_product, &hinge_norm);
		
		if (cross_dot < 0) {
			angle_deg = 360 - angle_deg;
		}
	}

	return (u32)angle_deg;
}

/**
 * chuwi_minibook_x_acos_scaled - Approximation of arccos function
 * @x: Input value scaled by 1e6 (-1e6 to 1e6)
 *
 * Simple polynomial approximation of arccos for our use case.
 * Returns: Angle in radians scaled by 1e6
 */
u64 chuwi_minibook_x_acos_scaled(s64 x)
{
	/* Simple linear approximation: acos(x) ≈ π/2 - x */
	s64 result = PI_SCALED / 2 - div64_s64(x * PI_SCALED, 2000000);
	
	if (result < 0)
		result = 0;
	else if (result > PI_SCALED)
		result = PI_SCALED;
		
	return (u64)result;
}

/**
 * chuwi_minibook_x_auto_calibrate_hinge - Auto-calibrate hinge axis
 * @chip: Device context
 *
 * Attempts to determine the hinge rotation axis by analyzing accelerometer data.
 * This is a simplified implementation that assumes Y-axis rotation.
 *
 * Returns: true if calibration was performed, false if not ready
 */
bool chuwi_minibook_x_auto_calibrate_hinge(struct chuwi_minibook_x *chip)
{
	/* Simple auto-calibration: assume Y-axis for now */
	if (!chip->auto_calibrated) {
		chip->hinge_axis.x = 0;
		chip->hinge_axis.y = 1000000;  /* Unit vector in Y direction */
		chip->hinge_axis.z = 0;
		chip->auto_calibrated = true;
		return true;
	}
	return false;
}

/**
 * chuwi_minibook_x_should_enter_tablet_mode - Check if should enter tablet mode
 * @chip: Device context
 *
 * Returns: true if device should enter tablet mode based on current angle
 */
bool chuwi_minibook_x_should_enter_tablet_mode(struct chuwi_minibook_x *chip)
{
	if (chip->signed_mode) {
		/* In signed mode, check if angle is in tablet range with hysteresis */
		return (chip->current_angle >= chip->enter_threshold) ||
		       (chip->current_angle <= (360 - chip->enter_threshold));
	} else {
		/* In unsigned mode, simple threshold check */
		return chip->current_angle >= chip->enter_threshold;
	}
}

/**
 * chuwi_minibook_x_should_exit_tablet_mode - Check if should exit tablet mode
 * @chip: Device context
 *
 * Returns: true if device should exit tablet mode based on current angle
 */
bool chuwi_minibook_x_should_exit_tablet_mode(struct chuwi_minibook_x *chip)
{
	if (chip->signed_mode) {
		/* In signed mode, check if angle is in laptop range with hysteresis */
		return (chip->current_angle <= chip->exit_threshold) &&
		       (chip->current_angle >= (360 - chip->exit_threshold));
	} else {
		/* In unsigned mode, simple threshold check with hysteresis */
		return chip->current_angle <= chip->exit_threshold;
	}
}

/**
 * chuwi_minibook_x_report_tablet_mode - Report tablet mode state change
 * @chip: Device context
 * @tablet_mode: New tablet mode state
 */
void chuwi_minibook_x_report_tablet_mode(struct chuwi_minibook_x *chip, bool tablet_mode)
{
	if (chip->input) {
		input_report_switch(chip->input, SW_TABLET_MODE, tablet_mode);
		input_sync(chip->input);
	}
}

/**
 * chuwi_minibook_x_detect_orientation - Detect current screen orientation
 * @chip: Device context
 *
 * Analyzes lid accelerometer data to determine screen orientation.
 * Uses gravity vector analysis to determine which edge is down.
 *
 * Returns: Detected orientation
 */
enum screen_orientation chuwi_minibook_x_detect_orientation(struct chuwi_minibook_x *chip)
{
	struct vec3 gravity;
	u64 magnitude;
	s32 x_abs, y_abs, z_abs;

	/* Use lid accelerometer for orientation detection */
	gravity = chip->lid_accel;
	magnitude = chuwi_minibook_x_vec3_magnitude(&gravity);

	/* Check if we have sufficient gravity signal */
	if (magnitude < GRAVITY_THRESHOLD) {
		return ORIENTATION_UNKNOWN;
	}

	/* Get absolute values for comparison */
	x_abs = abs(gravity.x);
	y_abs = abs(gravity.y);
	z_abs = abs(gravity.z);

	/* Determine orientation based on strongest gravity component */
	if (z_abs > x_abs && z_abs > y_abs) {
		/* Z-axis dominant - device is flat */
		if (gravity.z > 0) {
			return ORIENTATION_NORMAL;  /* Screen facing up */
		} else {
			return ORIENTATION_INVERTED;  /* Screen facing down */
		}
	} else if (x_abs > y_abs) {
		/* X-axis dominant - rotated left or right */
		if (gravity.x > 0) {
			return ORIENTATION_LEFT;   /* Rotated 90° left */
		} else {
			return ORIENTATION_RIGHT;  /* Rotated 90° right */
		}
	} else {
		/* Y-axis dominant - top or bottom edge down */
		if (gravity.y > 0) {
			return ORIENTATION_INVERTED;  /* 180° rotation */
		} else {
			return ORIENTATION_NORMAL;    /* Normal orientation */
		}
	}
}

/**
 * chuwi_minibook_x_calculate_orientation_confidence - Calculate confidence score
 * @chip: Device context
 * @orientation: Detected orientation
 *
 * Calculates a confidence score (0-100) for the detected orientation
 * based on how clearly the gravity vector indicates the orientation.
 *
 * Returns: Confidence percentage (0-100)
 */
u32 chuwi_minibook_x_calculate_orientation_confidence(struct chuwi_minibook_x *chip,
						      enum screen_orientation orientation)
{
	struct vec3 gravity = chip->lid_accel;
	u64 magnitude = chuwi_minibook_x_vec3_magnitude(&gravity);
	s32 primary_component = 0;
	s32 x_abs, y_abs, z_abs;
	u32 confidence;

	if (magnitude < GRAVITY_THRESHOLD) {
		return 0;
	}

	x_abs = abs(gravity.x);
	y_abs = abs(gravity.y);
	z_abs = abs(gravity.z);

	/* Determine primary component based on orientation */
	switch (orientation) {
	case ORIENTATION_NORMAL:
	case ORIENTATION_INVERTED:
		primary_component = max(z_abs, y_abs);
		break;
	case ORIENTATION_LEFT:
	case ORIENTATION_RIGHT:
		primary_component = x_abs;
		break;
	default:
		return 0;
	}

	/* Calculate confidence as percentage of primary component vs total */
	confidence = div64_u64((u64)primary_component * 100, magnitude);
	
	/* Clamp to reasonable range */
	if (confidence > 100)
		confidence = 100;
	else if (confidence < 10)
		confidence = 10;

	return confidence;
}

/**
 * chuwi_minibook_x_filter_angle_stability - Filter angle for stability
 * @chip: Device context
 *
 * Applies stability filtering to prevent rapid oscillations in angle
 * calculation, particularly around the 0°/360° boundary.
 */
void chuwi_minibook_x_filter_angle_stability(struct chuwi_minibook_x *chip)
{
	static u32 last_stable_angle = 0;
	static unsigned long last_change_time = 0;
	u32 angle_diff;
	unsigned long now = jiffies;

	/* Calculate angular difference, handling 0°/360° wrap-around */
	if (chip->current_angle > last_stable_angle) {
		angle_diff = min(chip->current_angle - last_stable_angle,
				360 - chip->current_angle + last_stable_angle);
	} else {
		angle_diff = min(last_stable_angle - chip->current_angle,
				360 - last_stable_angle + chip->current_angle);
	}

	/* Only update if change is significant or enough time has passed */
	if (angle_diff > chip->hysteresis || 
	    time_after(now, last_change_time + msecs_to_jiffies(500))) {
		last_stable_angle = chip->current_angle;
		last_change_time = now;
	} else {
		/* Use last stable angle to prevent oscillation */
		chip->current_angle = last_stable_angle;
	}
}

/**
 * chuwi_minibook_x_update_orientation - Update screen orientation state
 * @chip: Device context
 *
 * Main orientation detection function that determines current orientation,
 * applies stability filtering, and reports changes to the input subsystem.
 */
bool chuwi_minibook_x_update_orientation(struct chuwi_minibook_x *chip)
{
	enum screen_orientation new_orientation;
	u32 confidence;
	unsigned long now = jiffies;
	bool orientation_changed = false;

	/* Use forced orientation if set */
	if (chip->force_orientation >= 0) {
		new_orientation = (enum screen_orientation)chip->force_orientation;
		confidence = 100;
	} else {
		/* Detect orientation from sensors */
		new_orientation = chuwi_minibook_x_detect_orientation(chip);
		confidence = chuwi_minibook_x_calculate_orientation_confidence(chip, new_orientation);
	}

	/* Update current confidence */
	chip->orient_state.confidence = confidence;

	/* Check confidence threshold */
	if (confidence < chip->orient_config.confidence_threshold) {
		chip->orient_state.rejected_changes++;
		return false;
	}

	/* Check if orientation actually changed */
	if (new_orientation != chip->orient_state.current_orientation) {
		/* Calculate hysteresis for this transition */
		bool allow_change = true;

		/* Apply hysteresis check if we have a previous orientation */
		if (chip->orient_state.current_orientation != ORIENTATION_UNKNOWN) {
			/* For now, allow all changes that meet confidence threshold */
			/* TODO: Implement more sophisticated hysteresis if needed */
		}

		if (allow_change) {
			/* Check stability requirement */
			if (time_after(now, chip->orient_state.last_change_time + 
				      msecs_to_jiffies(chip->orient_config.stability_ms))) {
				
				/* Orientation change is valid */
				chip->orient_state.current_orientation = new_orientation;
				chip->orient_state.last_change_time = now;
				chip->orient_state.orientation_changes++;
				chip->orient_state.stable = true;
				orientation_changed = true;
			} else {
				/* Still in stability period */
				chip->orient_state.stable = false;
			}
		} else {
			/* Change rejected due to hysteresis */
			chip->orient_state.rejected_changes++;
		}
	} else {
		/* Same orientation - mark as stable */
		chip->orient_state.stable = true;
	}

	/* Report orientation change to input subsystem */
	if (orientation_changed && chip->input) {
		u32 rotation_value;

		/* Convert orientation to rotation value for input subsystem */
		switch (new_orientation) {
		case ORIENTATION_NORMAL:
			rotation_value = 0;
			break;
		case ORIENTATION_LEFT:
			rotation_value = 1;
			break;
		case ORIENTATION_INVERTED:
			rotation_value = 2;
			break;
		case ORIENTATION_RIGHT:
			rotation_value = 3;
			break;
		default:
			return false;  /* Don't report unknown orientations */
		}

		input_report_abs(chip->input, ABS_MISC, rotation_value);
		input_sync(chip->input);
	}

	return orientation_changed;
}