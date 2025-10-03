// SPDX-License-Identifier: GPL-2.0
/* chuwi-minibook-x-tablet-mode: simple hinge-angle → SW_TABLET_MODE helper
 *
 * Design:
 *  - Kernel module registers an input switch device (SW_TABLET_MODE).
 *  - Maintains two gravity vectors (base & lid) provided via sysfs.
 *  - Computes unsigned (0..180) or signed (0..360) hinge angle; applies hysteresis.
 *  - Exposes sysfs knobs: enter/exit thresholds, hysteresis, poll, force, state, angle,
 *    raw vectors, enable, signed_mode, hinge_axis (unit), one-shot calibration.
 *
 * v1 feeds vectors from two MXC4005 IIO devices via a userspace helper.
 * v2 can replace the feeder by reading IIO directly in-kernel.
 *
 * Later extension: bind to ACPI HID "MDA6655" and read IIO directly.
 *
 * Copyright (c) 2025 Armando DiCianno <armando@noonshy.com>
 */

#include <linux/init.h>
#ifdef CONFIG_CHUWI_MINIBOOK_X_TABLET_MODE_DEBUGFS
#include <linux/debugfs.h>
#endif
#include <linux/input.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/math64.h>
#include <linux/minmax.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/platform_device.h>
#include <linux/pm.h>
#include <linux/sysfs.h>
#include <linux/workqueue.h>

#define DRV_NAME "chuwi-minibook-x-tablet-mode"

/* Module parameters for configurable defaults */
static unsigned int default_poll_ms = 200;
module_param_named(poll_ms, default_poll_ms, uint, 0644);
MODULE_PARM_DESC(poll_ms, "Default polling interval in milliseconds (20-5000, default: 200)");

static unsigned int default_enter_deg = 300;
module_param_named(enter_deg, default_enter_deg, uint, 0644);
MODULE_PARM_DESC(enter_deg, "Default enter tablet mode threshold in degrees (0-360, default: 300)");

static unsigned int default_exit_deg = 60;
module_param_named(exit_deg, default_exit_deg, uint, 0644);
MODULE_PARM_DESC(exit_deg, "Default exit tablet mode threshold in degrees (0-360, default: 60)");

static unsigned int default_hysteresis_deg = 10;
module_param_named(hysteresis_deg, default_hysteresis_deg, uint, 0644);
MODULE_PARM_DESC(hysteresis_deg, "Default hysteresis in degrees (0-90, default: 10)");

static bool default_signed_mode = true;
module_param_named(signed_mode, default_signed_mode, bool, 0644);
MODULE_PARM_DESC(signed_mode, "Default signed angle mode (0=unsigned 0-180°, 1=signed 0-360°, default: 1)");

static bool default_enabled = true;
module_param_named(enabled, default_enabled, bool, 0644);
MODULE_PARM_DESC(enabled, "Default polling enabled state (default: 1)");

static struct input_dev *tm_input;
static struct kobject *tm_kobj;
static DEFINE_MUTEX(tm_lock);

#ifdef CONFIG_CHUWI_MINIBOOK_X_TABLET_MODE_DEBUGFS
/* Debugfs support */
static struct dentry *debugfs_root;
static struct dentry *debugfs_raw_data;
static struct dentry *debugfs_calculations;
#endif

/* Power management */
static struct platform_device *tm_pdev;
static bool was_enabled_before_suspend;

/* Last known gravity vectors (scaled ~1e6 for fixed-point ops). */
struct vec3 {
	s32 x, y, z;
};

/* Reasonable defaults: base “down”, lid “up” */
static struct vec3 g_base = { 0, 0, 1000000 };
static struct vec3 g_lid = { 0, 0, -1000000 };

static int enabled = 1;			/* polling enabled */
static int force_tablet = -1;		/* -1=auto, 0=force laptop, 1=force tablet */
static unsigned int poll_ms = 200;	/* polling period (ms) */

/* Signed-mode defaults (0..360): enter high, exit low */
static unsigned int enter_deg = 300;	/* ≥ this → enter tablet */
static unsigned int exit_deg = 60;	/* ≤ this → exit tablet */
static unsigned int hysteresis_deg = 10; /* extra guard, degrees */

static int cur_tablet = 0;		/* current switch state */
static unsigned int last_angle = 0;	/* last computed angle (deg) */

/*
 * Signed-angle mode & hinge axis (unit vector ~1e6).
 * Default axis along +Y (common hinge direction).
 */
static int signed_mode = 1;			/* 1=use 0..360 signed angle */
static struct vec3 hinge_axis_unit = { 0, 1000000, 0 };

static struct delayed_work poll_work;

/* Initialize default values from module parameters */
static void init_module_defaults(void)
{
	/* Validate and set module parameter defaults */
	if (default_poll_ms < 20 || default_poll_ms > 5000) {
		pr_warn(DRV_NAME ": invalid poll_ms parameter %u, using 200\n", default_poll_ms);
		default_poll_ms = 200;
	}
	poll_ms = default_poll_ms;

	if (default_enter_deg > 360) {
		pr_warn(DRV_NAME ": invalid enter_deg parameter %u, using 300\n", default_enter_deg);
		default_enter_deg = 300;
	}
	enter_deg = default_enter_deg;

	if (default_exit_deg > 360) {
		pr_warn(DRV_NAME ": invalid exit_deg parameter %u, using 60\n", default_exit_deg);
		default_exit_deg = 60;
	}
	exit_deg = default_exit_deg;

	if (default_hysteresis_deg > 90) {
		pr_warn(DRV_NAME ": invalid hysteresis_deg parameter %u, using 10\n", default_hysteresis_deg);
		default_hysteresis_deg = 10;
	}
	hysteresis_deg = default_hysteresis_deg;

	signed_mode = default_signed_mode ? 1 : 0;
	enabled = default_enabled ? 1 : 0;
}

/* ------------------------- Math helpers ------------------------- */

#define ABS64(x) ((x) < 0 ? -(x) : (x))

static s64 dot3(const struct vec3 *a, const struct vec3 *b)
{
	return (s64)a->x * b->x + (s64)a->y * b->y + (s64)a->z * b->z;
}

/* magnitude (Q0.0 int), 64-bit to avoid overflow */
static u64 mag(const struct vec3 *v)
{
	s64 xx = (s64)v->x * v->x;
	s64 yy = (s64)v->y * v->y;
	s64 zz = (s64)v->z * v->z;
	return int_sqrt64(xx + yy + zz);
}

/* normalize v to have magnitude ~1e6 (returns 0 if zero vector) */
static int normalize1e6(struct vec3 *v)
{
	u64 m = mag(v);
	s64 x = v->x, y = v->y, z = v->z;
	u64 s;

	if (!m)
		return 0;

	/* Avoid overflow in scaling calculation */
	if (m > (U64_MAX >> 20)) {
		/* For very large vectors, scale down first */
		v->x = (s32)(x / 1000);
		v->y = (s32)(y / 1000);
		v->z = (s32)(z / 1000);
		m = mag(v);
		x = v->x; y = v->y; z = v->z;
		if (!m)
			return 0;
	}

	/* scale factor s = 1e6 / m, done as Q20 to keep precision */
	s = div64_u64(1000000ULL << 20, m);
	v->x = (s32)((x * (s64)s) >> 20);
	v->y = (s32)((y * (s64)s) >> 20);
	v->z = (s32)((z * (s64)s) >> 20);
	return 1;
}

/* circular distance between angles in degrees [0..360) */
static unsigned int circ_dist(unsigned int a, unsigned int b)
{
	unsigned int d = (a > b) ? (a - b) : (b - a);
	return min(d, 360u - d);
}

/* Project v onto plane orthogonal to unit axis au (|au| ≈ 1e6).
 * v_proj = v - au * (dot(v,au)/|au|^2); since |au|^2 ≈ 1e12, divide by 1e12.
 */
static struct vec3 project_onto_plane(const struct vec3 *v, const struct vec3 *au)
{
	s64 d = dot3(v, au);			/* ~ |v| * 1e6 */
	s64 au_mag_sq = dot3(au, au);		/* should be ~ 1e12 */
	s64 scale;
	struct vec3 r;

	/* Avoid division by zero or very small values */
	if (au_mag_sq < 100000000000LL) {	/* 1e11 threshold */
		/* Axis too small, return original vector */
		return *v;
	}

	scale = div_s64(d, div_s64(au_mag_sq, 1000000LL));
	r.x = v->x - (s32)(div_s64((s64)au->x * scale, 1000000LL));
	r.y = v->y - (s32)(div_s64((s64)au->y * scale, 1000000LL));
	r.z = v->z - (s32)(div_s64((s64)au->z * scale, 1000000LL));
	return r;
}

/* Unsigned angle in degrees (0..180) between vectors a and b. */
static unsigned int angle_deg_u(const struct vec3 *a, const struct vec3 *b)
{
	s64 dot = dot3(a, b);
	u64 ma = mag(a), mb = mag(b);
	long long denom, cosq, deg;

	if (!ma || !mb)
		return 0;

	denom = (long long)div64_u64((u64)ma * (u64)mb, 1000000ULL);
	if (!denom)
		return 0;

	cosq = div_s64(dot, denom); /* approx cos * 1e6 */
	if (cosq > 1000000LL) cosq = 1000000LL;
	if (cosq < -1000000LL) cosq = -1000000LL;

	/* acos(x) ≈ π/2 - x - 0.167*x^3  (x scaled by 1e6; radians scaled 1e6) */
	{
		s64 x = cosq;
		s64 x3 = div_s64((x * x) / 1000000LL * x, 1000000LL);
		s64 rad_q6 = 1570796LL/2 - x - div_s64(167000LL * x3, 1000000LL);
		deg = div_s64(rad_q6 * 180LL, 3141593LL);
	}

	if (deg < 0) deg = 0;
	if (deg > 180) deg = 180;
	return (unsigned int)deg;
}

/* Scalar triple product: axis · (a × b) in s64.
 * Used for signed angle direction without constructing a cross in s32.
 */
static s64 triple_prod_s64(const struct vec3 *a, const struct vec3 *b, const struct vec3 *axis)
{
	s64 cx = (s64)a->y * b->z - (s64)a->z * b->y;
	s64 cy = (s64)a->z * b->x - (s64)a->x * b->z;
	s64 cz = (s64)a->x * b->y - (s64)a->y * b->x;
	return (s64)axis->x * cx + (s64)axis->y * cy + (s64)axis->z * cz;
}

/* Cross product computed in s64 then scaled down to s32 safely.
 * Caller should normalize afterwards.
 */
static struct vec3 cross3_scaled(const struct vec3 *a, const struct vec3 *b)
{
	s64 cx = (s64)a->y * b->z - (s64)a->z * b->y;
	s64 cy = (s64)a->z * b->x - (s64)a->x * b->z;
	s64 cz = (s64)a->x * b->y - (s64)a->y * b->x;
	s64 m = max_t(s64, ABS64(cx), max_t(s64, ABS64(cy), ABS64(cz)));
	struct vec3 r = { 0, 0, 0 };
	s64 div;

	if (!m)
		return r;

	/* ceil(m / 1e6) so the largest component becomes ≤ ~1e6 */
	div = div64_s64(m + 999999, 1000000LL);
	if (div < 1)
		div = 1;

	r.x = (s32)(cx / div);
	r.y = (s32)(cy / div);
	r.z = (s32)(cz / div);
	return r;
}


/* Signed hinge angle in degrees (0..360):
 * 1) Project base & lid onto plane ⟂ axis (unit ~1e6),
 * 2) Compute unsigned angle on the plane,
 * 3) Use sign(axis · (pb × pl)) to choose  ang or 360-ang.
 */
static unsigned int angle_deg_signed(const struct vec3 *base,
                                     const struct vec3 *lid,
                                     const struct vec3 *axis_unit)
{
	/* 1) Project onto the hinge plane */
	struct vec3 pb = project_onto_plane(base, axis_unit);
	struct vec3 pl = project_onto_plane(lid,  axis_unit);

	/* 2) Normalize projections to ~1e6 (keeps triple-product scale predictable) */
	(void)normalize1e6(&pb);
	(void)normalize1e6(&pl);

	/* 3) Unsigned angle on the plane (0..180) */
	unsigned int ang = angle_deg_u(&pb, &pl);

	/* 4) Scalar triple product for sign */
	s64 t = triple_prod_s64(&pb, &pl, axis_unit);

	/* Dynamic deadband:
	 * With pb,pl,axis all ~1e6, the true |t| scale is ~1e18 * sin(theta).
	 * Pick an epsilon that dwarfs noise but is tiny vs real motion.
	 */
	const s64 EPS = 100000000000000LL; /* 1e14 */

	/* Candidate branches for signed angle */
	unsigned int a_pos = ang;                 /* t ≥ 0 → θ */
	unsigned int a_neg = ang ? 360 - ang : 0; /* t < 0 → 360-θ */

	/* 5) Stable branch selection (deadband + continuity) */
	if (t > EPS) {
		return a_pos;
	} else if (t < -EPS) {
		return a_neg;
	} else {
		/* Within deadband → pick the branch closest to the last reported angle */
		unsigned int d_pos = circ_dist(a_pos, last_angle);
		unsigned int d_neg = circ_dist(a_neg, last_angle);
		return (d_pos <= d_neg) ? a_pos : a_neg;
	}
}


/* ------------------------- Reporting & eval ------------------------- */

static void report_switch(int tablet)
{
	if (!tm_input)
		return;

	input_report_switch(tm_input, SW_TABLET_MODE, tablet);
	input_sync(tm_input);

	pr_info(DRV_NAME ": state -> %s (angle=%u)\n",
		tablet ? "tablet" : "laptop", last_angle);
}

static void evaluate_and_report(void)
{
	int new_state;
	unsigned int a;

	if (signed_mode) {
		/* Ensure hinge axis is sensible; default +Y if bad. */
		struct vec3 axis = hinge_axis_unit;
		if (!normalize1e6(&axis)) {
			axis.x = 0; axis.y = 1000000; axis.z = 0;
		}
		a = angle_deg_signed(&g_base, &g_lid, &axis);   /* 0..360 */
	} else {
		a = angle_deg_u(&g_base, &g_lid);               /* 0..180 */
	}

	last_angle = a;

	/* thresholds local copies (we'll treat ranges below) */
	/* block here limits the lifetime/scope of enter and exit to that block, */
	/* and avoids accidental reuse/shadowing elsewhere in the function */
	{
		unsigned int enter = enter_deg;
		unsigned int exit = exit_deg;

		/* In unsigned mode, widen enter by hysteresis when not in tablet. */
		if (hysteresis_deg && !signed_mode && !cur_tablet)
			enter = min(enter + hysteresis_deg, 180u);

		/* Decide new state */
		if (force_tablet == 0) {
			new_state = 0;
		} else if (force_tablet == 1) {
			new_state = 1;
		} else if (signed_mode) {
			/* Signed space is circular: 0°≈closed, 360°≈tablet.
			 * Tablet region = [enter..360] ∪ [0..exit].
			 * Enter when we cross into the region; exit when we leave it.
			 */
			bool in_tablet_region = (a >= enter) || (a <= exit);

			if (!cur_tablet && in_tablet_region)
				new_state = 1;
			else if (cur_tablet && !in_tablet_region)
				new_state = 0;
			else
				new_state = cur_tablet;
		} else {
			/* Unsigned space (0..180): classic high/low compare. */
			if (!cur_tablet && a >= enter)
				new_state = 1;
			else if (cur_tablet && a <= exit)
				new_state = 0;
			else
				new_state = cur_tablet;
		}
	}

	if (new_state != cur_tablet) {
		cur_tablet = new_state;
		report_switch(cur_tablet);
	}
}

static void poll_work_fn(struct work_struct *w)
{
	mutex_lock(&tm_lock);
	evaluate_and_report();
	mutex_unlock(&tm_lock);

	if (enabled)
		schedule_delayed_work(&poll_work, msecs_to_jiffies(poll_ms));
}


#ifdef CONFIG_CHUWI_MINIBOOK_X_TABLET_MODE_DEBUGFS
/* ------------------------- Debugfs ------------------------- */

static int debugfs_raw_data_show(struct seq_file *s, void *unused)
{
	struct vec3 axis;
	u64 base_mag, lid_mag, axis_mag;
	s64 base_lid_dot;

	mutex_lock(&tm_lock);

	/* Calculate magnitudes */
	base_mag = mag(&g_base);
	lid_mag = mag(&g_lid);
	axis = hinge_axis_unit;
	axis_mag = mag(&axis);
	base_lid_dot = dot3(&g_base, &g_lid);

	seq_printf(s, "Raw Sensor Data:\n");
	seq_printf(s, "  Base vector:    [%8d, %8d, %8d] (mag: %llu)\n",
		   g_base.x, g_base.y, g_base.z, base_mag);
	seq_printf(s, "  Lid vector:     [%8d, %8d, %8d] (mag: %llu)\n",
		   g_lid.x, g_lid.y, g_lid.z, lid_mag);
	seq_printf(s, "  Hinge axis:     [%8d, %8d, %8d] (mag: %llu)\n",
		   hinge_axis_unit.x, hinge_axis_unit.y, hinge_axis_unit.z, axis_mag);
	seq_printf(s, "  Base•Lid dot:   %lld\n", base_lid_dot);
	seq_printf(s, "\nCurrent State:\n");
	seq_printf(s, "  Tablet mode:    %s\n", cur_tablet ? "yes" : "no");
	seq_printf(s, "  Last angle:     %u degrees\n", last_angle);
	seq_printf(s, "  Polling:        %s\n", enabled ? "enabled" : "disabled");
	seq_printf(s, "  Force mode:     %s\n",
		   force_tablet == -1 ? "auto" :
		   force_tablet == 0 ? "laptop" : "tablet");

	mutex_unlock(&tm_lock);
	return 0;
}

static int debugfs_calculations_show(struct seq_file *s, void *unused)
{
	struct vec3 axis, pb, pl;
	unsigned int unsigned_angle, signed_angle;
	s64 triple_prod;
	bool in_tablet_region;
	unsigned int enter, exit;

	mutex_lock(&tm_lock);

	/* Perform the same calculations as evaluate_and_report */
	axis = hinge_axis_unit;
	if (!normalize1e6(&axis)) {
		axis.x = 0; axis.y = 1000000; axis.z = 0;
	}

	/* Project vectors onto plane */
	pb = project_onto_plane(&g_base, &axis);
	pl = project_onto_plane(&g_lid, &axis);
	(void)normalize1e6(&pb);
	(void)normalize1e6(&pl);

	/* Calculate angles */
	unsigned_angle = angle_deg_u(&g_base, &g_lid);
	signed_angle = signed_mode ? angle_deg_signed(&g_base, &g_lid, &axis) : 0;
	triple_prod = triple_prod_s64(&pb, &pl, &axis);

	/* Threshold calculations */
	enter = enter_deg;
	exit = exit_deg;
	if (hysteresis_deg && !signed_mode && !cur_tablet)
		enter = min(enter + hysteresis_deg, 180u);

	if (signed_mode) {
		in_tablet_region = (last_angle >= enter) || (last_angle <= exit);
	} else {
		in_tablet_region = (last_angle >= enter);
	}

	seq_printf(s, "Calculation Details:\n");
	seq_printf(s, "  Signed mode:       %s\n", signed_mode ? "yes" : "no");
	seq_printf(s, "  Normalized axis:   [%8d, %8d, %8d]\n",
		   axis.x, axis.y, axis.z);
	seq_printf(s, "  Projected base:    [%8d, %8d, %8d]\n", pb.x, pb.y, pb.z);
	seq_printf(s, "  Projected lid:     [%8d, %8d, %8d]\n", pl.x, pl.y, pl.z);
	seq_printf(s, "  Unsigned angle:    %u degrees\n", unsigned_angle);
	if (signed_mode)
		seq_printf(s, "  Signed angle:      %u degrees\n", signed_angle);
	seq_printf(s, "  Triple product:    %lld\n", triple_prod);
	seq_printf(s, "\nThreshold Analysis:\n");
	seq_printf(s, "  Enter threshold:   %u degrees\n", enter);
	seq_printf(s, "  Exit threshold:    %u degrees\n", exit);
	seq_printf(s, "  Hysteresis:        %u degrees\n", hysteresis_deg);
	seq_printf(s, "  In tablet region:  %s\n", in_tablet_region ? "yes" : "no");

	mutex_unlock(&tm_lock);
	return 0;
}

DEFINE_SHOW_ATTRIBUTE(debugfs_raw_data);
DEFINE_SHOW_ATTRIBUTE(debugfs_calculations);

static void create_debugfs_interface(void)
{
	debugfs_root = debugfs_create_dir(DRV_NAME, NULL);
	if (IS_ERR_OR_NULL(debugfs_root)) {
		pr_warn(DRV_NAME ": failed to create debugfs directory\n");
		debugfs_root = NULL;
		return;
	}

	debugfs_raw_data = debugfs_create_file("raw_data", 0444, debugfs_root,
					       NULL, &debugfs_raw_data_fops);
	debugfs_calculations = debugfs_create_file("calculations", 0444, debugfs_root,
						   NULL, &debugfs_calculations_fops);

	pr_info(DRV_NAME ": debugfs interface created at /sys/kernel/debug/%s/\n", DRV_NAME);
}

static void remove_debugfs_interface(void)
{
	debugfs_remove_recursive(debugfs_root);
	debugfs_root = NULL;
}
#endif /* CONFIG_CHUWI_MINIBOOK_X_TABLET_MODE_DEBUGFS */

/* ------------------------- Power Management ------------------------- */

static int tm_suspend(struct device *dev)
{
	pr_debug(DRV_NAME ": entering suspend\n");

	mutex_lock(&tm_lock);
	was_enabled_before_suspend = enabled;
	if (enabled) {
		enabled = 0;
		cancel_delayed_work_sync(&poll_work);
	}
	mutex_unlock(&tm_lock);

	pr_info(DRV_NAME ": suspended (was %s)\n",
		was_enabled_before_suspend ? "enabled" : "disabled");
	return 0;
}

static int tm_resume(struct device *dev)
{
	pr_debug(DRV_NAME ": entering resume\n");

	mutex_lock(&tm_lock);
	if (was_enabled_before_suspend) {
		enabled = 1;
		/* Re-evaluate state immediately after resume */
		evaluate_and_report();
		/* Restart polling */
		schedule_delayed_work(&poll_work, msecs_to_jiffies(poll_ms));
	}
	mutex_unlock(&tm_lock);

	pr_info(DRV_NAME ": resumed (restored to %s)\n",
		enabled ? "enabled" : "disabled");
	return 0;
}

static SIMPLE_DEV_PM_OPS(tm_pm_ops, tm_suspend, tm_resume);

static struct platform_driver tm_driver = {
	.driver = {
		.name = DRV_NAME,
		.pm = &tm_pm_ops,
	},
};

static ssize_t show_state(struct kobject *k, struct kobj_attribute *a, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%d\n", cur_tablet);
}

static ssize_t show_angle(struct kobject *k, struct kobj_attribute *a, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%u\n", last_angle);
}

static ssize_t show_enable(struct kobject *k, struct kobj_attribute *a, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%d\n", enabled);
}

static ssize_t store_enable(struct kobject *k, struct kobj_attribute *a,
			     const char *b, size_t l)
{
	int v;
	if (kstrtoint(b, 0, &v))
		return -EINVAL;
	mutex_lock(&tm_lock);
	enabled = !!v;
	mutex_unlock(&tm_lock);
	if (enabled)
		schedule_delayed_work(&poll_work, msecs_to_jiffies(poll_ms));
	else
		cancel_delayed_work_sync(&poll_work);
	return l;
}

static ssize_t show_force(struct kobject *k, struct kobj_attribute *a, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%d\n", force_tablet);
}

static ssize_t store_force(struct kobject *k, struct kobj_attribute *a,
			   const char *b, size_t l)
{
	int v;

	if (kstrtoint(b, 0, &v))
		return -EINVAL;
	if (v < -1 || v > 1)
		return -ERANGE;

	mutex_lock(&tm_lock);
	force_tablet = v;
	mutex_unlock(&tm_lock);

	/* immediate eval so it flips right away */
	schedule_delayed_work(&poll_work, 0);
	return l;
}

#define DEF_ATTR_RW(name) \
static struct kobj_attribute name##_attr = __ATTR(name, 0644, show_##name, store_##name)
#define DEF_ATTR_RO(name) \
static struct kobj_attribute name##_attr = __ATTR(name, 0444, show_##name, NULL)

/* thresholds */
static ssize_t show_enter_deg(struct kobject *k, struct kobj_attribute *a, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%u\n", enter_deg);
}
static ssize_t store_enter_deg(struct kobject *k, struct kobj_attribute *a,
				const char *b, size_t l)
{
	unsigned int v;

	if (kstrtouint(b, 0, &v))
		return -EINVAL;
	if (v > 360)
		return -ERANGE;

	mutex_lock(&tm_lock);
	enter_deg = v;
	mutex_unlock(&tm_lock);
	schedule_delayed_work(&poll_work, 0);
	return l;
}

static ssize_t show_exit_deg(struct kobject *k, struct kobj_attribute *a, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%u\n", exit_deg);
}

static ssize_t store_exit_deg(struct kobject *k, struct kobj_attribute *a,
			      const char *b, size_t l)
{
	unsigned int v;

	if (kstrtouint(b, 0, &v))
		return -EINVAL;
	if (v > 360)
		return -ERANGE;

	mutex_lock(&tm_lock);
	exit_deg = v;
	mutex_unlock(&tm_lock);
	schedule_delayed_work(&poll_work, 0);
	return l;
}

static ssize_t show_hysteresis_deg(struct kobject *k, struct kobj_attribute *a, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%u\n", hysteresis_deg);
}

static ssize_t store_hysteresis_deg(struct kobject *k, struct kobj_attribute *a,
				    const char *b, size_t l)
{
	unsigned int v;

	if (kstrtouint(b, 0, &v))
		return -EINVAL;
	if (v > 90)
		return -ERANGE;

	mutex_lock(&tm_lock);
	hysteresis_deg = v;
	mutex_unlock(&tm_lock);
	schedule_delayed_work(&poll_work, 0);
	return l;
}

/* signed_mode */
static ssize_t show_signed_mode(struct kobject *k, struct kobj_attribute *a, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%d\n", signed_mode);
}
static ssize_t store_signed_mode(struct kobject *k, struct kobj_attribute *a,
				  const char *b, size_t l)
{
	int v;

	if (kstrtoint(b, 0, &v))
		return -EINVAL;

	mutex_lock(&tm_lock);
	signed_mode = !!v;
	mutex_unlock(&tm_lock);
	schedule_delayed_work(&poll_work, 0);
	return l;
}

/* hinge_axis: 3 ints (micro-units), will be normalized to ~1e6 */
static ssize_t show_hinge_axis(struct kobject *k, struct kobj_attribute *a, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%d %d %d\n",
			 hinge_axis_unit.x, hinge_axis_unit.y, hinge_axis_unit.z);
}
static ssize_t store_hinge_axis(struct kobject *k, struct kobj_attribute *a,
				 const char *b, size_t l)
{
	struct vec3 v;
	int n;

	n = sscanf(b, "%d %d %d", &v.x, &v.y, &v.z);
	if (n != 3)
		return -EINVAL;

	/* Check for reasonable vector magnitudes to prevent overflow */
	if (abs(v.x) > 100000000 || abs(v.y) > 100000000 || abs(v.z) > 100000000)
		return -ERANGE;

	if (!normalize1e6(&v))
		return -EINVAL;

	mutex_lock(&tm_lock);
	hinge_axis_unit = v;
	mutex_unlock(&tm_lock);
	schedule_delayed_work(&poll_work, 0);
	return l;
}

/* one-shot calibration: write "1" while hinge is between ~60..120 degrees.
 * Sets axis = normalize( cross(g_base, g_lid) ), then re-evaluate.
 */
static ssize_t store_calibrate_signed(struct kobject *k, struct kobj_attribute *a,
				       const char *b, size_t l)
{
	int v;
	struct vec3 ax;

	if (kstrtoint(b, 0, &v))
		return -EINVAL;

	if (v) {
		mutex_lock(&tm_lock);
		ax = cross3_scaled(&g_base, &g_lid);
		if (!normalize1e6(&ax)) {
			mutex_unlock(&tm_lock);
			return -EINVAL;
		}
		hinge_axis_unit = ax;
		mutex_unlock(&tm_lock);

		pr_info(DRV_NAME ": calibrated hinge axis to [%d %d %d]\n",
			hinge_axis_unit.x, hinge_axis_unit.y, hinge_axis_unit.z);
		schedule_delayed_work(&poll_work, 0);
	}
	return l;
}

/* poll interval */
static ssize_t show_poll_ms(struct kobject *k, struct kobj_attribute *a, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%u\n", poll_ms);
}
static ssize_t store_poll_ms(struct kobject *k, struct kobj_attribute *a,
			      const char *b, size_t l)
{
	unsigned int v;

	if (kstrtouint(b, 0, &v))
		return -EINVAL;
	if (v < 20 || v > 5000)
		return -ERANGE;

	mutex_lock(&tm_lock);
	poll_ms = v;
	mutex_unlock(&tm_lock);

	if (enabled) {
		cancel_delayed_work_sync(&poll_work);
		schedule_delayed_work(&poll_work, msecs_to_jiffies(poll_ms));
	}
	return l;
}

/* raw vectors: expect three integers per write (micro-g units) */
static ssize_t show_base_vec(struct kobject *k, struct kobj_attribute *a, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%d %d %d\n", g_base.x, g_base.y, g_base.z);
}
static ssize_t store_base_vec(struct kobject *k, struct kobj_attribute *a,
			       const char *b, size_t l)
{
	struct vec3 v;
	int n;

	n = sscanf(b, "%d %d %d", &v.x, &v.y, &v.z);
	if (n != 3)
		return -EINVAL;

	mutex_lock(&tm_lock);
	g_base = v;
	mutex_unlock(&tm_lock);
	schedule_delayed_work(&poll_work, 0);
	return l;
}

static ssize_t show_lid_vec(struct kobject *k, struct kobj_attribute *a, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%d %d %d\n", g_lid.x, g_lid.y, g_lid.z);
}

static ssize_t store_lid_vec(struct kobject *k, struct kobj_attribute *a,
			     const char *b, size_t l)
{
	struct vec3 v;
	int n;

	n = sscanf(b, "%d %d %d", &v.x, &v.y, &v.z);
	if (n != 3)
		return -EINVAL;

	mutex_lock(&tm_lock);
	g_lid = v;
	mutex_unlock(&tm_lock);
	schedule_delayed_work(&poll_work, 0);
	return l;
}

/* Attribute declarations & group */
DEF_ATTR_RO(state);
DEF_ATTR_RO(angle);
DEF_ATTR_RW(enable);
DEF_ATTR_RW(force);
DEF_ATTR_RW(enter_deg);
DEF_ATTR_RW(exit_deg);
DEF_ATTR_RW(hysteresis_deg);
DEF_ATTR_RW(poll_ms);
DEF_ATTR_RW(base_vec);
DEF_ATTR_RW(lid_vec);
DEF_ATTR_RW(signed_mode);

static struct kobj_attribute hinge_axis_attr =
	__ATTR(hinge_axis, 0644, show_hinge_axis, store_hinge_axis);
static struct kobj_attribute calibrate_signed_attr =
	__ATTR(calibrate_signed, 0200, NULL, store_calibrate_signed);

static struct attribute *tm_attrs[] = {
	&state_attr.attr,
	&angle_attr.attr,
	&enable_attr.attr,
	&force_attr.attr,
	&enter_deg_attr.attr,
	&exit_deg_attr.attr,
	&hysteresis_deg_attr.attr,
	&poll_ms_attr.attr,
	&base_vec_attr.attr,
	&lid_vec_attr.attr,
	&signed_mode_attr.attr,
	&hinge_axis_attr.attr,
	&calibrate_signed_attr.attr,
	NULL,
};
ATTRIBUTE_GROUPS(tm);

/* ------------------------- Init / Exit ------------------------- */

static int __init tm_init(void)
{
	int err;

	/* Initialize defaults from module parameters */
	init_module_defaults();

	/* Register platform driver for power management */
	err = platform_driver_register(&tm_driver);
	if (err) {
		pr_err(DRV_NAME ": failed to register platform driver: %d\n", err);
		return err;
	}

	/* Create platform device */
	tm_pdev = platform_device_register_simple(DRV_NAME, -1, NULL, 0);
	if (IS_ERR(tm_pdev)) {
		err = PTR_ERR(tm_pdev);
		pr_err(DRV_NAME ": failed to register platform device: %d\n", err);
		goto err_driver;
	}

	tm_input = input_allocate_device();
	if (!tm_input) {
		pr_err(DRV_NAME ": failed to allocate input device\n");
		err = -ENOMEM;
		goto err_pdev;
	}

	tm_input->name = "Chuwi Minibook X Tablet Mode";
	tm_input->phys = "chuwi-minibook-x-tablet-mode/input0";
	tm_input->id.bustype = BUS_HOST;

	__set_bit(EV_SW, tm_input->evbit);
	__set_bit(SW_TABLET_MODE, tm_input->swbit);

	err = input_register_device(tm_input);
	if (err) {
		pr_err(DRV_NAME ": failed to register input device: %d\n", err);
		input_free_device(tm_input);
		tm_input = NULL;
		goto err_pdev;
	}

	/* Clamp thresholds for current mode. */
	if (signed_mode) {
		if (enter_deg > 360) { pr_warn(DRV_NAME ": enter_deg>360, clamping\n"); enter_deg = 360; }
		if (exit_deg  > 360) { pr_warn(DRV_NAME ": exit_deg>360, clamping\n");  exit_deg  = 360; }
	} else {
		if (enter_deg > 180) { pr_warn(DRV_NAME ": enter_deg>180, clamping\n"); enter_deg = 180; }
		if (exit_deg  > 180) { pr_warn(DRV_NAME ": exit_deg>180, clamping\n");  exit_deg  = 180; }
	}

	tm_kobj = kobject_create_and_add(DRV_NAME, kernel_kobj);
	if (!tm_kobj) {
		pr_err(DRV_NAME ": failed to create sysfs kobject\n");
		err = -ENOMEM;
		goto err_input;
	}

	err = sysfs_create_groups(tm_kobj, tm_groups);
	if (err) {
		pr_err(DRV_NAME ": failed to create sysfs groups: %d\n", err);
		goto err_kobj;
	}

	/* Create debugfs interface */
#ifdef CONFIG_CHUWI_MINIBOOK_X_TABLET_MODE_DEBUGFS
	create_debugfs_interface();
#endif

	INIT_DELAYED_WORK(&poll_work, poll_work_fn);

	/* Seed a first report so user space sees initial state. */
	mutex_lock(&tm_lock);
	evaluate_and_report();
	mutex_unlock(&tm_lock);

	if (enabled)
		schedule_delayed_work(&poll_work, msecs_to_jiffies(poll_ms));

	pr_info(DRV_NAME ": loaded (enter=%u exit=%u poll=%ums signed=%d enabled=%d)\n",
		enter_deg, exit_deg, poll_ms, signed_mode, enabled);
	return 0;

err_kobj:
	kobject_put(tm_kobj);
err_input:
	input_unregister_device(tm_input);
	tm_input = NULL;
err_pdev:
	platform_device_unregister(tm_pdev);
err_driver:
	platform_driver_unregister(&tm_driver);
	return err;
}

static void __exit tm_exit(void)
{
	cancel_delayed_work_sync(&poll_work);
#ifdef CONFIG_CHUWI_MINIBOOK_X_TABLET_MODE_DEBUGFS
	remove_debugfs_interface();
#endif
	if (tm_kobj) {
		sysfs_remove_groups(tm_kobj, tm_groups);
		kobject_put(tm_kobj);
	}
	if (tm_input)
		input_unregister_device(tm_input);
	if (tm_pdev)
		platform_device_unregister(tm_pdev);
	platform_driver_unregister(&tm_driver);
	pr_info(DRV_NAME ": unloaded\n");
}

module_init(tm_init);
module_exit(tm_exit);

MODULE_AUTHOR("Armando DiCianno");
MODULE_DESCRIPTION("Use hinge angle on Minibook X to trigger SW_TABLET_MODE");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");
