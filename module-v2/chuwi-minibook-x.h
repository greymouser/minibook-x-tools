/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Chuwi Minibook X Platform Driver
 *
 * Copyright (C) 2025 Your Name
 *
 * This driver provides tablet mode detection and screen orientation support
 * for the Chuwi Minibook X convertible laptop using dual accelerometers.
 */

#ifndef _CHUWI_MINIBOOK_X_H_
#define _CHUWI_MINIBOOK_X_H_

#include <linux/types.h>
#include <linux/mutex.h>
#include <linux/workqueue.h>
#include <linux/input.h>
#include <linux/iio/consumer.h>

#define CHUWI_MINIBOOK_X_DRIVER_NAME "chuwi-minibook-x"

/* 3D vector structure for accelerometer data */
struct vec3 {
	s32 x, y, z;
};

/* Screen orientation enumeration */
enum screen_orientation {
	ORIENTATION_NORMAL = 0,   /* 0° - landscape, normal */
	ORIENTATION_LEFT = 1,     /* 90° - portrait, rotated left */
	ORIENTATION_INVERTED = 2, /* 180° - landscape, inverted */
	ORIENTATION_RIGHT = 3,    /* 270° - portrait, rotated right */
	ORIENTATION_UNKNOWN = 4   /* unknown/transitional state */
};

/* Orientation detection configuration */
struct orientation_config {
	unsigned int hysteresis_deg;       /* hysteresis for orientation changes */
	unsigned int stability_ms;         /* time to wait before changing orientation */
	unsigned int confidence_threshold; /* minimum confidence percentage */
	bool enabled;                      /* orientation detection enabled */
};

/* Orientation detection state */
struct orientation_state {
	enum screen_orientation current_orientation;
	unsigned long last_change_time;   /* jiffies of last orientation change */
	unsigned long orientation_changes; /* total number of orientation changes */
	unsigned long rejected_changes;    /* number of rejected orientation changes */
	unsigned int confidence;           /* current confidence percentage */
	bool stable;                       /* whether current orientation is stable */
};

/* Main driver context structure */
struct chuwi_minibook_x {
	/* Platform device */
	struct platform_device *pdev;
	
	/* Input device for tablet mode and orientation events */
	struct input_dev *input;
	
	/* IIO channels for accelerometers */
	struct iio_channel *base_accel_channels[3];  /* x, y, z */
	struct iio_channel *lid_accel_channels[3];   /* x, y, z */
	
	/* Work queue for periodic sensor polling */
	struct delayed_work poll_work;
	struct mutex lock;
	
	/* Current sensor data */
	struct vec3 base_accel;
	struct vec3 lid_accel;
	
	/* Tablet mode detection */
	bool tablet_mode;
	unsigned int current_angle;
	unsigned int enter_threshold;  /* angle threshold to enter tablet mode */
	unsigned int exit_threshold;   /* angle threshold to exit tablet mode */
	unsigned int hysteresis;       /* hysteresis for mode switching */
	
	/* Hinge axis calibration */
	struct vec3 hinge_axis;
	bool auto_calibrated;
	
	/* Orientation detection */
	struct orientation_config orient_config;
	struct orientation_state orient_state;
	
	/* Module configuration */
	unsigned int poll_interval_ms;
	bool enabled;
	int force_tablet_mode;        /* -1=auto, 0=laptop, 1=tablet */
	int force_orientation;        /* -1=auto, 0-3=forced orientation */
	bool signed_mode;            /* use 0-360° instead of 0-180° */
	
	/* Debugging */
#ifdef CONFIG_CHUWI_MINIBOOK_X_DEBUGFS
	struct dentry *debugfs_dir;
	struct dentry *debugfs_raw_data;
	struct dentry *debugfs_calculations;
#endif
};

/* Function declarations */

/* Main module functions */
int chuwi_minibook_x_probe(struct platform_device *pdev);
void chuwi_minibook_x_remove(struct platform_device *pdev);

/* Sensor reading functions */
int chuwi_minibook_x_read_accelerometers(struct chuwi_minibook_x *chip);

/* Tablet mode detection */
unsigned int chuwi_minibook_x_calculate_angle(const struct vec3 *base, 
					       const struct vec3 *lid,
					       const struct vec3 *hinge_axis,
					       bool signed_mode);
bool chuwi_minibook_x_should_enter_tablet_mode(struct chuwi_minibook_x *chip);
bool chuwi_minibook_x_should_exit_tablet_mode(struct chuwi_minibook_x *chip);

/* Orientation detection */
bool chuwi_minibook_x_update_orientation(struct chuwi_minibook_x *chip);
enum screen_orientation chuwi_minibook_x_detect_orientation(struct chuwi_minibook_x *chip);
const char *chuwi_minibook_x_orientation_name(enum screen_orientation orient);

/* Auto-calibration */
bool chuwi_minibook_x_auto_calibrate_hinge(struct chuwi_minibook_x *chip);

/* Input event reporting */
void chuwi_minibook_x_report_tablet_mode(struct chuwi_minibook_x *chip, bool tablet_mode);
void chuwi_minibook_x_report_orientation(struct chuwi_minibook_x *chip, 
					  enum screen_orientation orientation);

/* Sysfs interface */
int chuwi_minibook_x_create_sysfs(struct chuwi_minibook_x *chip);
void chuwi_minibook_x_remove_sysfs(struct chuwi_minibook_x *chip);

/* Math utilities and orientation detection functions */
u64 chuwi_minibook_x_vec3_magnitude(const struct vec3 *v);
int chuwi_minibook_x_vec3_normalize(const struct vec3 *v, struct vec3 *result);
s64 chuwi_minibook_x_vec3_dot(const struct vec3 *a, const struct vec3 *b);
u32 chuwi_minibook_x_calculate_angle(const struct vec3 *base_accel,
				     const struct vec3 *lid_accel,
				     const struct vec3 *hinge_axis,
				     bool signed_mode);
u64 chuwi_minibook_x_acos_scaled(s64 x);
u32 chuwi_minibook_x_calculate_orientation_confidence(struct chuwi_minibook_x *chip,
						      enum screen_orientation orientation);
void chuwi_minibook_x_filter_angle_stability(struct chuwi_minibook_x *chip);

/* Main driver functions */
int chuwi_minibook_x_read_accelerometers(struct chuwi_minibook_x *chip);

#ifdef CONFIG_CHUWI_MINIBOOK_X_DEBUGFS
/* Debugfs interface */
int chuwi_minibook_x_debugfs_init(struct chuwi_minibook_x *chip);
void chuwi_minibook_x_debugfs_cleanup(struct chuwi_minibook_x *chip);
#else
static inline int chuwi_minibook_x_debugfs_init(struct chuwi_minibook_x *chip) { return 0; }
static inline void chuwi_minibook_x_debugfs_cleanup(struct chuwi_minibook_x *chip) {}
#endif

#endif /* _CHUWI_MINIBOOK_X_H_ */