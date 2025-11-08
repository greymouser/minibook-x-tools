# Chuwi Minibook X Tools - AI Coding Assistant Instructions

## Project Overview

This is a Linux hardware support project for the **Chuwi Minibook X** convertible laptop, implementing tablet mode detection through accelerometer data and mount matrix transformations. The system consists of a kernel module (`cmx`) and userspace daemon (`cmxd`) working together.

In the future, there may also be a desktop integration component (`tablet-mode-daemon`) to translate kernel events into desktop environment actions.

## Architecture

### Two-Component System
- **`cmx/`** - Kernel module that handles hardware detection, I2C device instantiation, mount matrix setup, and provides sysfs interface
  - The mount matrices may or may not be correct. They are likely correct. We should collect evidence that proves their correctness.
- **`cmxd/`** - Userspace daemon that processes accelerometer data, calculates hinge angles, and communicates with kernel module
- **`tablet-mode-daemon/`** - Desktop integration daemon that translates kernel events to desktop environment actions

### Hardware Quirk: 90° Sensor Rotations
The Minibook X has both accelerometers rotated 90°:
- **Lid sensor**: 90° counter-clockwise (CCW) 
- **Base sensor**: 90° clockwise (CW)

There is a chance these rotations are incorrect and need to be changed.

Mount matrices in `cmx.c` handle coordinate transformations:
```c
// Lid sensor (90° CCW): X'=Y, Y'=-X, Z'=Z
static const char * const lid_sensor_mount_matrix[] = {
    "0", "1", "0", "-1", "0", "0", "0", "0", "1"
};
```

## Key Development Patterns

### Build System
```bash
# Kernel module (uses Kbuild integration)
cd cmx && make && sudo make load

# Userspace daemon  
cd cmxd && make && sudo make install

# Master build from root
make all  # builds all components
```

### Critical Paths
- **IIO devices**: `/sys/bus/iio/devices/iio:device[01]/` - accelerometer raw data
- **Kernel sysfs**: `/sys/devices/platform/cmx/` - module configuration and data input
- **Socket interface**: `/run/cmxd/events.sock` - daemon event communication

### Testing Infrastructure
Modular test files in `cmxd/test-*.c`:
```bash
cd cmxd
make test-integration  # builds test-integration.c
./test-integration     # runs specific angle calculation tests
```

## Data Flow Architecture

```
IIO Devices → Mount Matrix → cmxd daemon → sysfs write → cmx kernel → Input Events
```

1. **cmxd** reads from `/sys/bus/iio/devices/iio:device[01]/in_accel_[xyz]_raw`
2. Applies coordinate transformations for 90° rotations
3. Calculates hinge angle using gravity-aware 3D vector math (`cmxd-calculations.c`)
4. Writes results to `/sys/devices/platform/cmx/accel_data` 
5. **cmx** kernel module processes and generates `SW_TABLET_MODE` input events

## Code Organization

### cmxd/ - Userspace Components
- **`cmxd.c`** - Main daemon with signal handling, configuration
- **`cmxd-calculations.c`** - 3D vector math, hinge angle algorithms 
- **`cmxd-data.c`** - IIO device data reading, mount matrix handling
- **`cmxd-events.c`** - Unix socket and DBus event system
- **`cmxd-modes.c`** - Tablet/laptop mode detection logic
- **`cmxd-orientation.c`** - Screen orientation detection

### cmx/ - Kernel Module
- **`cmx.c`** - Platform driver, sysfs interface, input device management
- **`cmx.h`** - Shared structures and constants
- Uses delayed work queues for event-driven processing (no polling)

## Development Workflows

### Hardware Testing
```bash
# Monitor accelerometer data
cat /sys/bus/iio/devices/iio:device0/in_accel_*_raw

# Check mount matrices applied
cat /sys/bus/iio/devices/iio:device*/in_accel_mount_matrix

# Monitor laptop mode state - closing <-> laptop <-> flat <-> tent <-> tablet
watch 'cat /sys/devices/platform/cmx/mode'
```

### Debugging Daemon Issues
```bash
# Run daemon in foreground with verbose output
sudo cmxd -v

# Monitor sysfs communication
sudo strace -e write cmxd 2>&1 | grep sysfs
```

If you are asked to check the cmxd.log, you will need to process the entire thing, not just head or tail a bit of it; there is too much data generated for a snapshot to be useful.

### Kernel Module Development
- Use `make dmesg` in `cmx/` to view module messages
- Check `/proc/bus/input/devices` for input device creation
- Use `evtest` to monitor SW_TABLET_MODE events

## Project-Specific Conventions

### Error Handling
- Kernel module: Uses `dev_err()`, `dev_warn()`, `dev_info()` consistently
- Userspace: Custom logging functions in each module with verbosity levels

### Memory Management  
- Kernel: Proper cleanup on module unload, uses devm_* functions where possible
- Userspace: No dynamic allocation in critical paths, stack-based structures

### Configuration
- Module parameters in `cmx.c` for hardware detection overrides
- Systemd service files with security hardening (`cmxd.service`)
- Config file support in `/etc/default/cmxd`

## Legacy Components

- **`exploration/`** - Research notes and test utilities (reference only)
- **`tablet-mode-daemon/`** - Alternative desktop integration approach  
- **`tools/`** - EC debugging utilities for BIOS interaction

The main development focus is on `cmx/` + `cmxd/` integration.