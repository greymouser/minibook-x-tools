# Chuwi Minibook X Tools - AI Coding Assistant Instructions

## Project Overview

This is a Linux hardware support project for the **Chuwi Minibook X** convertible laptop, implementing tablet mode detection through accelerometer data and mount matrix transformations. The system consists of a kernel module (`cmx`) and userspace daemon (`cmxd`) working together. The kernel module is supported by a kernel patch (`cmx-kernel-patch`) turning off automatic ACPI scanning & loading for mxc4005 accelerometer devices and support for mxc4005 in serial-multi-instantiate, allowing multiple instances of the mxc4005 accelerometer devices to load. There is a desktop integration component (`tablet-mode-daemon`) to translate kernel events into desktop environment actions.

## Architecture

### Two-Component System
- **`cmx-kernel-patch/`** - Kernel patch to disable ACPI loading of mxc4005 devices and enable serial-multi-instantiate support for multiple mxc4005 devices.
- **`cmx/`** - Kernel module that provides sysfs interface and responds to tablet mode changes.
- **`cmxd/`** - Userspace daemon that processes accelerometer data, calculates hinge angles, and communicates with kernel module
- **`tablet-mode-daemon/`** - Desktop integration daemon that translates kernel events to desktop environment actions. Supports a custom UNIX Domain Sockets protocol as well as DBus.

### Hardware Quirk: Sensor Orientations
The Minibook X accelerometer sensors have specific physical orientations:
- **Lid sensor**: No rotation needed (identity matrix)
- **Base sensor**: No rotation needed (identity matrix) but with Z-axis inverted

**Note**: The mount matrices have been empirically determined and corrected through analysis.

Mount matrices in `cmx.c` handle coordinate transformations:
```c
// Lid sensor (identity): X'=X, Y'=Y, Z'=Z  
static const char * const lid_sensor_mount_matrix[] = {
    "1", "0", "0", "0", "1", "0", "0", "0", "1"
};

// Base sensor (90° CW + Z invert): X'=-Y, Y'=X, Z'=-Z
static const char * const base_sensor_mount_matrix[] = {
    "0", "-1", "0", "1", "0", "0", "0", "0", "-1"
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

## Unloading and loading Modules
```bash
# Unload kernel module
sudo rmmod cmx     # From cmx/
sudo rmmod mxc4005 # From cmx/ , also removes the supporting driver

# Load the module
sudo insmod ./cmx.ko  # From cmx/
```

### Running the Daemon

Running `make` in the `cmxd` directory builds all components. It places `cmxd` in the `cmxd/` directory. `cmxd` can be run from the `./cmxd` directory of the project.

```bash
# From the root directory of the project
sudo ./cmxd -v  # verbose output for debugging

## To run the daemon and capture logs using tee
sudo ./cmxd -v 2>&1 | tee cmxd.log

# To timeout after a period and capture the logs
sudo timeout 60 ./cmxd -v > cmxd.log 2>&1
```

### For Kernel Moduile or Daemon Development

The system can be left stuck in tablet mode (which will disable the keyboard and touchpad). To reset back to laptop mode, write "laptop" to the mode sysfs entry:

```bash
echo "laptop" | sudo tee /sys/devices/platform/cmx/mode
```

Note: AI Agents should be aware of this command when suggesting code that may leave the system in tablet mode during development or testing. They should complete tests that touch the sysfs mode entry by resetting it back to laptop mode afterward.

### Critical Paths
- **IIO devices**: `/sys/bus/iio/devices/iio:device[01]/` - accelerometer raw data
- **Kernel sysfs**: `/sys/devices/platform/cmx/` - module configuration and data input
- **Socket interface**: `/run/cmxd/events.sock` - daemon event communication

## Data Flow Architecture

```
IIO Devices → Mount Matrix → cmxd daemon → sysfs write → cmx kernel → Input Events
```

1. **cmxd** reads from `/sys/bus/iio/devices/iio:device[01]/in_accel_[xyz]_raw`
2. Applies coordinate transformations using mount matrices
3. Calculates hinge angle using gravity-aware 3D vector math (`cmxd-calculations.c`)
4. Writes results to files in `/sys/devices/platform/cmx/` 
5. **cmx** kernel module generates `SW_TABLET_MODE` input events

## Code Organization

### cmxd/ - Userspace Components
- **`dev/`** - Test scripts, monitoring tools, and mount matrix analysis utilities
- **`src/cmxd.c`** - Main daemon with signal handling, configuration
- **`src/cmxd-calculations.c`** - 3D vector math, hinge angle algorithms 
- **`src/cmxd-data.c`** - IIO device data reading, mount matrix handling
- **`src/cmxd-events.c`** - Unix socket and DBus event system
- **`src/cmxd-modes.c`** - Tablet/laptop mode detection logic
- **`src/cmxd-orientation.c`** - Screen orientation detection
- **`src/cmxd-dbus.c`** - DBus interface implementation for desktop integration
- **`src/cmxd-protocol.c`** - Communication protocol handling
- **`src/cmxd-paths.h`** - System paths and file locations
- **`support/cmxd.conf`** - Configuration file for daemon settings
- **`support/cmxd.service`** - Systemd service file for automatic startup
- **`support/cmxd-dbus.conf`** - DBus service configuration

### cmx/ - Kernel Module
- **`dev/`** - Test scripts and hardware exploration notes
- **`cmx.c`** - Platform driver, sysfs interface, input device management (single-file module)
- Uses delayed work queues for event-driven processing (no polling)

### Tablet Mode Daemon

- TBD

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
sudo ./cmxd -v  # This will run indefinitely and will need to be terminated manually

# Monitor sysfs communication
sudo strace -e write ./cmxd 2>&1 | grep sysfs
```

If the AI Agent is asked to check a file named cmxd.log, it will need to process the entire thing, not just head or tail a bit of it; there is too much data generated for a snapshot to be useful.

### Kernel Module Development
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
- Module writes `iio_lid_device` and `iio_base_device` sysfs files to allow the cmxd daemon to recognize which IIO device is the lid and base.
- Systemd service files with security hardening (`cmxd.service`)
- Config file support in `/etc/default/cmxd`

## EC Tools

- **`tools/`** - EC utility to set bit for advanced BIOS settings

## Dev stuff

- **`exploration/`** - Research notes and test utilities (reference only)
