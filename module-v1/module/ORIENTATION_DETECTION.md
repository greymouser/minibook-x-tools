# Screen Orientation Detection for Chuwi Minibook X

This enhancement adds comprehensive screen orientation detection to the `chuwi-minibook-x-tablet-mode` kernel module, enabling userspace programs to automatically adjust screen rotation based on device orientation.

## Overview

The orientation detection system analyzes gravity data from the base accelerometer to determine screen orientation. It provides four orientations (normal, left, inverted, right) plus flat detection, with sophisticated stability controls to prevent unwanted rotations when the device is moved or set down.

## Key Features

### Automatic Orientation Detection
- **Four orientations**: Normal (0°), Left (90°), Inverted (180°), Right (270°)
- **Flat detection**: Prevents orientation changes when device is laying flat
- **Motion detection**: Prevents changes during device movement
- **Confidence tracking**: Only reports high-confidence orientation measurements

### Stability Controls
- **Hysteresis**: Prevents rapid oscillation between orientations
- **Stability timeout**: Requires orientation to be stable for configurable time
- **Motion settling**: Waits for motion to stop before allowing changes
- **Adjacent transitions**: Prefers 90° rotations over 180° flips

### Flexible Configuration
- **Tunable thresholds**: All timing and sensitivity parameters configurable
- **Force override**: Can force specific orientation for testing
- **Enable/disable**: Can disable orientation detection entirely
- **Statistics**: Tracks orientation changes and rejected attempts

## Usage

### Input Events

The module generates input events for screen orientation changes:

- **Event type**: `EV_ABS`
- **Event code**: `ABS_MT_ORIENTATION`
- **Values**: 0, 90, 180, 270 (degrees of rotation)

### sysfs Interface

#### Read-only Status Files

```bash
# Current orientation (string)
cat /sys/kernel/chuwi-minibook-x-tablet-mode/orientation
# Output: normal, left, inverted, right, flat, unknown

# Current orientation in degrees
cat /sys/kernel/chuwi-minibook-x-tablet-mode/orientation_degrees
# Output: 0, 90, 180, 270, or -1

# Configuration summary
cat /sys/kernel/chuwi-minibook-x-tablet-mode/orientation_config
# Output: hysteresis_deg=15 stability_time_ms=500 flat_threshold_deg=25 motion_threshold=100000 motion_settle_ms=200 min_confidence=70

# Statistics and diagnostics
cat /sys/kernel/chuwi-minibook-x-tablet-mode/orientation_stats  
# Output: changes=42 rejected=13 confidence=95
```

#### Configuration Files

```bash
# Enable/disable orientation detection
echo 1 > /sys/kernel/chuwi-minibook-x-tablet-mode/orientation_enabled
echo 0 > /sys/kernel/chuwi-minibook-x-tablet-mode/orientation_enabled

# Force specific orientation (for testing)
echo auto > /sys/kernel/chuwi-minibook-x-tablet-mode/force_orientation      # Auto-detect
echo normal > /sys/kernel/chuwi-minibook-x-tablet-mode/force_orientation    # Force normal (0°)
echo left > /sys/kernel/chuwi-minibook-x-tablet-mode/force_orientation      # Force left (90°)
echo inverted > /sys/kernel/chuwi-minibook-x-tablet-mode/force_orientation  # Force inverted (180°)
echo right > /sys/kernel/chuwi-minibook-x-tablet-mode/force_orientation     # Force right (270°)

# Numeric values also work
echo -1 > /sys/kernel/chuwi-minibook-x-tablet-mode/force_orientation  # Auto-detect
echo 0 > /sys/kernel/chuwi-minibook-x-tablet-mode/force_orientation   # Force normal
echo 1 > /sys/kernel/chuwi-minibook-x-tablet-mode/force_orientation   # Force left
echo 2 > /sys/kernel/chuwi-minibook-x-tablet-mode/force_orientation   # Force inverted
echo 3 > /sys/kernel/chuwi-minibook-x-tablet-mode/force_orientation   # Force right
```

## Integration Examples

### Monitor Orientation Changes

```bash
#!/bin/bash
# Monitor orientation input events
while read event; do
    if [[ $event == *"ABS_MT_ORIENTATION"* ]]; then
        degrees=$(echo $event | grep -o '[0-9]*' | tail -1)
        echo "Screen rotated to ${degrees}°"
        # Add your screen rotation command here
        # xrandr --output <display> --rotate <left|right|inverted|normal>
    fi
done < <(sudo evtest /dev/input/by-path/*tablet-mode* 2>/dev/null)
```

### Wayland/GNOME Integration

```bash
#!/bin/bash
# Example GNOME orientation handler
orientation=$(cat /sys/kernel/chuwi-minibook-x-tablet-mode/orientation)

case $orientation in
    "normal")
        gsettings set org.gnome.settings-daemon.peripherals.touchscreen orientation-lock false
        gsettings set org.gnome.settings-daemon.peripherals.touchscreen orientation 'normal'
        ;;
    "left")
        gsettings set org.gnome.settings-daemon.peripherals.touchscreen orientation 'left'
        ;;
    "inverted")
        gsettings set org.gnome.settings-daemon.peripherals.touchscreen orientation 'inverted'
        ;;
    "right")
        gsettings set org.gnome.settings-daemon.peripherals.touchscreen orientation 'right'
        ;;
    "flat")
        # Device is flat - maintain current orientation
        ;;
esac
```

### X11/Xrandr Integration

```bash
#!/bin/bash
# Example X11 orientation handler
degrees=$(cat /sys/kernel/chuwi-minibook-x-tablet-mode/orientation_degrees)

case $degrees in
    0)   xrandr --output eDP-1 --rotate normal ;;
    90)  xrandr --output eDP-1 --rotate left ;;
    180) xrandr --output eDP-1 --rotate inverted ;;
    270) xrandr --output eDP-1 --rotate right ;;
esac

# Also rotate touch input to match
case $degrees in
    0)   xinput set-prop "pointer:Goodix Capacitive TouchScreen" "Coordinate Transformation Matrix" 1 0 0 0 1 0 0 0 1 ;;
    90)  xinput set-prop "pointer:Goodix Capacitive TouchScreen" "Coordinate Transformation Matrix" 0 -1 1 1 0 0 0 0 1 ;;
    180) xinput set-prop "pointer:Goodix Capacitive TouchScreen" "Coordinate Transformation Matrix" -1 0 1 0 -1 1 0 0 1 ;;
    270) xinput set-prop "pointer:Goodix Capacitive TouchScreen" "Coordinate Transformation Matrix" 0 1 0 -1 0 1 0 0 1 ;;
esac
```

## Advanced Configuration

### Tuning Stability Parameters

The orientation detection can be tuned by modifying the configuration structure in code (requires recompilation):

```c
/* In orientation_detection.c - modify these defaults: */
#define DEFAULT_HYSTERESIS_DEG          15      /* Orientation change hysteresis */
#define DEFAULT_STABILITY_TIME_MS       500     /* Required stability time */
#define DEFAULT_FLAT_THRESHOLD_DEG      25      /* Flat detection threshold */
#define DEFAULT_MOTION_THRESHOLD        100000  /* Motion detection sensitivity */
#define DEFAULT_MOTION_SETTLE_MS        200     /* Post-motion settle time */
#define DEFAULT_MIN_CONFIDENCE          70      /* Minimum confidence percentage */
```

### Custom Orientation Thresholds

Orientation angle ranges can be customized by modifying the constants in `orientation_detection.c`:

```c
/* Orientation angle ranges (in degrees from horizontal) */
#define NORMAL_MIN      315     /* 315° - 45° (wrapping around 0°) */
#define NORMAL_MAX      45
#define LEFT_MIN        45      /* 45° - 135° */
#define LEFT_MAX        135
#define INVERTED_MIN    135     /* 135° - 225° */
#define INVERTED_MAX    225
#define RIGHT_MIN       225     /* 225° - 315° */
#define RIGHT_MAX       315
```

## Testing and Debugging

### Manual Testing

```bash
# Force different orientations to test your handler scripts
echo normal > /sys/kernel/chuwi-minibook-x-tablet-mode/force_orientation
echo left > /sys/kernel/chuwi-minibook-x-tablet-mode/force_orientation  
echo inverted > /sys/kernel/chuwi-minibook-x-tablet-mode/force_orientation
echo right > /sys/kernel/chuwi-minibook-x-tablet-mode/force_orientation
echo auto > /sys/kernel/chuwi-minibook-x-tablet-mode/force_orientation

# Monitor raw input events
sudo evtest /dev/input/by-path/*tablet-mode*

# Check orientation statistics
watch -n 1 cat /sys/kernel/chuwi-minibook-x-tablet-mode/orientation_stats
```

### Debug Information

```bash
# Monitor kernel messages for orientation changes
sudo dmesg -w | grep "orientation"

# Check current accelerometer readings
cat /sys/kernel/chuwi-minibook-x-tablet-mode/base_vec

# Monitor confidence and rejection reasons
cat /sys/kernel/chuwi-minibook-x-tablet-mode/orientation_stats
```

## Architecture Details

### Detection Algorithm

1. **Gravity Analysis**: Uses base accelerometer to determine "down" direction
2. **Angle Calculation**: Converts gravity vector to screen rotation angle
3. **Orientation Mapping**: Maps angle ranges to discrete orientations
4. **Confidence Assessment**: Evaluates measurement quality and stability
5. **Motion Detection**: Compares current and previous measurements
6. **Stability Filtering**: Applies hysteresis and timing requirements
7. **Event Generation**: Reports changes via input subsystem and sysfs

### Stability Mechanisms

- **Flat Detection**: When tilt < 25°, considers device "flat" and maintains current orientation
- **Motion Detection**: Monitors accelerometer changes to detect device movement
- **Confidence Tracking**: Only accepts measurements with sufficient signal quality
- **Hysteresis**: Requires 15° change to switch between adjacent orientations
- **Timing Requirements**: Orientation must be stable for 500ms before switching
- **Adjacent Preference**: Prefers 90° transitions over 180° flips to prevent accidental inversions

## Implementation Files

- **`orientation_detection.h`**: Header with data structures and function prototypes
- **`orientation_detection.c`**: Core orientation detection algorithms and logic
- **`tablet_mode_main.c`**: Main module with sysfs interface and input event generation
- **`Makefile`**: Updated to build multi-file module

The orientation detection integrates seamlessly with the existing tablet mode detection, providing a complete solution for 2-in-1 device orientation management.