# Chuwi Minibook X Kernel Module

A Linux kernel module providing hardware support for the Chuwi Minibook X convertible laptop, including accelerometer device instantiation with mount matrices and tablet mode detection infrastructure.

## Overview

This kernel module provides the foundation for tablet mode detection on the Chuwi Minibook X. It handles hardware detection, I2C device setup, mount matrix transformations, and provides infrastructure for tablet mode events.

**Important**: This module provides infrastructure only. It does not implement actual angle calculations or automatic tablet mode switching. External logic is required to read accelerometer data and update the module via sysfs.

## Features

- **Hardware Detection**: Automatic DMI-based detection of Chuwi Minibook X hardware
- **I2C Device Setup**: Instantiates MXC4005 accelerometer devices with proper mount matrices
- **Existing Device Support**: Applies mount matrices to already-instantiated devices
- **Mount Matrix Support**: Applies 90° rotations to correct sensor orientation
- **Input Device**: Creates `/dev/input/eventX` for `SW_TABLET_MODE` events
- **Sysfs Interface**: Provides runtime configuration and manual data input
- **Platform Driver**: Integrates with Linux platform device subsystem

## Quick Start

### Build and Load
```bash
# Build the module
make

# Load the module
sudo make load

# Check status
make dmesg
```

### Basic Usage
```bash
# Check current mode
cat /sys/kernel/chuwi-minibook-x/mode

# Test input device
evtest /dev/input/by-path/*tablet*

# Manually set tablet mode
echo "tablet" > /sys/kernel/chuwi-minibook-x/mode
```

## Module Parameters

Configure hardware detection and behavior at load time:

### Hardware Configuration
- `lid_bus=13` - I2C bus for lid accelerometer
- `lid_addr=0x15` - I2C address for lid accelerometer
- `base_bus=12` - I2C bus for base accelerometer
- `base_addr=0x15` - I2C address for base accelerometer
- `enable_mount_matrix=1` - Apply mount matrix transformations
- `debug_mode=0` - Enable debug output

### Tablet Mode Parameters
- `poll_ms=200` - Polling interval in milliseconds
- `enter_deg=200` - Tablet mode entry threshold
- `exit_deg=170` - Tablet mode exit threshold
- `hysteresis_deg=10` - Hysteresis in degrees
- `signed_mode=1` - Use 0-360° vs 0-180° angles
- `enabled=1` - Enable tablet mode detection

### Loading Examples
```bash
# Custom I2C configuration
sudo insmod chuwi-minibook-x.ko lid_bus=14 base_bus=11

# Enable debugging
sudo insmod chuwi-minibook-x.ko debug_mode=1

# Custom thresholds
sudo insmod chuwi-minibook-x.ko enter_deg=180 exit_deg=160
```

## Sysfs Interface

Runtime configuration and monitoring at `/sys/kernel/chuwi-minibook-x/`:

### Data Input/Output
- `base_vec` (rw) - Base accelerometer vector: "x y z" in micro-g
- `lid_vec` (rw) - Lid accelerometer vector: "x y z" in micro-g
- `mode` (rw) - Current mode: laptop, flat, tent, tablet, closing
- `orientation` (rw) - Current orientation: portrait, landscape, portrait-flipped, landscape-flipped

### Device Information
- `iio_base_device` (r) - IIO device name for base accelerometer
- `iio_lid_device` (r) - IIO device name for lid accelerometer

### Usage Examples
```bash
# Check current state
cat /sys/kernel/chuwi-minibook-x/mode
cat /sys/kernel/chuwi-minibook-x/orientation

# Set accelerometer data manually (for testing)
echo "0 0 1000000" > /sys/kernel/chuwi-minibook-x/base_vec
echo "500000 0 500000" > /sys/kernel/chuwi-minibook-x/lid_vec

# Force tablet mode
echo "tablet" > /sys/kernel/chuwi-minibook-x/mode
```

## Input Device

The module creates an input device that reports tablet mode changes:

```bash
# Find the input device
ls /dev/input/by-path/*tablet*

# Monitor events (install evtest if needed)
evtest /dev/input/by-path/*tablet*
```

Events reported:
- `SW_TABLET_MODE`: 0=laptop mode, 1=tablet mode

## Hardware Support

### Supported Devices
- **Vendor**: "CHUWI Innovation And Technology(ShenZhen)co.,Ltd"
- **Product**: "MiniBook X"

### Expected Hardware
Two MXC4005 accelerometer devices:
- **Lid sensor**: typically I2C bus 13, address 0x15
- **Base sensor**: typically I2C bus 12, address 0x15

### Mount Matrices
The module applies coordinate transformations:
- **Lid sensor**: 90° counter-clockwise rotation
- **Base sensor**: 90° clockwise rotation

This corrects for physical sensor mounting orientation.

## Build System

### Make Targets
- `make` or `make all` - Build the module
- `make clean` - Clean build artifacts
- `make install` - Install to system modules
- `make load` - Load the module
- `make unload` - Unload the module
- `make reload` - Unload and reload
- `make test` - Load and run basic tests
- `make dmesg` - Show recent kernel messages
- `make help` - Show available targets

### Dependencies
- Linux kernel with headers
- MXC4005 accelerometer driver
- I2C subsystem support

## Kernel Configuration

For in-tree builds:
```
CONFIG_CHUWI_MINIBOOK_X=m
CONFIG_CHUWI_MINIBOOK_X_DEBUGFS=y  # optional
```

Required kernel features:
- `CONFIG_X86=y`
- `CONFIG_ACPI=y`
- `CONFIG_INPUT=y`
- `CONFIG_IIO=y`
- `CONFIG_I2C=y`

## Debugging

### Basic Troubleshooting
```bash
# Check if module loaded
lsmod | grep chuwi

# Check kernel messages
dmesg | grep -i "chuwi\|minibook"

# Verify accelerometer devices
ls /sys/bus/i2c/devices/ | grep -E "(12|13)"

# Check input device
ls /dev/input/by-path/ | grep tablet
```

### Debug Mode
Load with `debug_mode=1` for detailed output:
```bash
sudo insmod chuwi-minibook-x.ko debug_mode=1
dmesg | tail -20
```

Debug mode provides additional information about:
- Mount matrix application for each accelerometer
- Device instantiation details and target configuration
- IIO device mapping and binding status
- Detection and handling of existing devices vs new instantiation

### Debugfs Support
If compiled with `CONFIG_CHUWI_MINIBOOK_X_DEBUGFS=y`:
```bash
# View debug information
cat /sys/kernel/debug/chuwi-minibook-x/raw_data
cat /sys/kernel/debug/chuwi-minibook-x/calculations
```

## Implementation Notes

### Current State
This module provides **infrastructure only**:
- ✅ Hardware detection and I2C device setup
- ✅ Mount matrix transformations
- ✅ Sysfs interface for data input/output
- ✅ Input device for tablet mode events
- ❌ Automatic angle calculations
- ❌ Automatic tablet mode switching

### Required External Logic
To implement full tablet mode detection, external software must:

1. **Read accelerometer data** from IIO devices
2. **Calculate hinge angles** from sensor readings
3. **Compare angles** against thresholds with hysteresis
4. **Update module** via sysfs interface

Example workflow:
```bash
# 1. Read from IIO devices (external daemon)
iio_device_0="/sys/bus/iio/devices/iio:device0/"
base_x=$(cat $iio_device_0/in_accel_x_raw)
# ... process mount matrix transformations

# 2. Calculate angle (external logic)
# angle = calculate_hinge_angle(base_vec, lid_vec)

# 3. Update kernel module
echo "$base_x $base_y $base_z" > /sys/kernel/chuwi-minibook-x/base_vec
echo "$lid_x $lid_y $lid_z" > /sys/kernel/chuwi-minibook-x/lid_vec
```

### Architecture
- **Platform Driver**: Standard Linux platform driver model
- **Work Queues**: Uses delayed work for periodic processing
- **Input Events**: Reports via standard Linux input subsystem
- **Fixed-Point Math**: Avoids floating-point operations in kernel

## Files

- `chuwi-minibook-x.c` - Main module implementation (981 lines)
- `chuwi-minibook-x.h` - Header with structure definitions
- `Makefile` - Build configuration
- `Kconfig` - Kernel configuration options

## License

GPL v2.0 or later. See individual file headers for details.

## Version

Current version: 3.0 (MODULE_VERSION defined in source)

## Contributing

When contributing:
1. Follow Linux kernel coding standards
2. Test on actual Chuwi Minibook X hardware
3. Update documentation for API changes
4. Ensure clean compilation without warnings