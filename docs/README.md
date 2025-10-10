# Chuwi Minibook X Tools & Documentation

This repository is a collection of tools, scripts, and information related to the **Chuwi Minibook X**.  
The goal is to document quirks of this platform and provide utilities to interact with system features that are otherwise undocumented.

> ⚠️ **Warning**  
> Many of these tools interact directly with low-level system components such as the Embedded Controller (EC).  
> **Use at your own risk.** Misuse can permanently damage your device.

---

## Current Project Structure

### 🚀 **chuwi-minibook-x** - Integrated Platform Driver (v3) - **RECOMMENDED**

The unified platform driver that combines the best aspects of all previous approaches:

- **Complete tablet mode detection** with hinge angle calculation and input events
- **Mount matrix coordinate transformation** (90° sensor rotations automatically corrected)
- **Event-driven architecture** using delayed work queues (minimal CPU usage)
- **Proper I2C device instantiation** with mount matrices
- **Comprehensive sysfs interface** for configuration and status

```bash
cd chuwi-minibook-x
make
sudo insmod chuwi-minibook-x-integrated.ko
```

### 📡 **chuwi-minibook-x-daemon** - Userspace Data Feeder

Companion userspace daemon that:
- Reads accelerometer data from IIO devices with mount matrix transformations
- Feeds pre-transformed coordinates to the kernel module
- Triggers immediate tablet mode evaluation (event-driven)

```bash
cd chuwi-minibook-x-daemon
make
sudo make install
sudo systemctl enable chuwi-minibook-x-tablet-mode
```

---

## Previous Development Iterations

### **module-v2** - Mount Matrix Implementation
- Advanced I2C device instantiation with mount matrix support
- Coordinate transformation for 90° rotated sensors  
- Hardware detection and DMI matching
- Foundation for the integrated v3 approach

### **module-v1** - Original Tablet Mode Detection
- Complete tablet mode detection algorithm
- Input device events and screen orientation
- Comprehensive sysfs interface
- Proved the feasibility of the approach

---

## Key Features of the Integrated Solution

### 🎯 **Event-Driven Architecture**
No continuous polling! Data processing triggered only when new accelerometer readings arrive:
```
IIO Devices → Mount Matrix → Daemon → Kernel sysfs → Delayed Work → Input Events
```

### 🔄 **Mount Matrix Coordinate Transformation** 
Handles the hardware quirk where sensors are rotated 90°:
- **Lid sensor**: 90° counter-clockwise rotation → corrected automatically
- **Base sensor**: 90° clockwise rotation → corrected automatically
- **Result**: Standard laptop coordinates regardless of physical sensor orientation

### ⚡ **Optimized Performance**
- **Minimal CPU usage**: Event-driven processing only when needed
- **Low latency**: Direct sysfs write triggers immediate kernel processing
- **Battery friendly**: No unnecessary polling cycles

### 🖥️ **Desktop Integration**
- **SW_TABLET_MODE events**: Standard Linux input events for desktop environments
- **Screen orientation detection**: Automatic rotation support
- **Systemd integration**: Proper service management and auto-start

---

## Installation (Integrated v3 Solution)

### Quick Start
```bash
# Load the integrated kernel module
cd chuwi-minibook-x
make clean && make
sudo insmod chuwi-minibook-x-integrated.ko

# Start the userspace daemon
cd ../chuwi-minibook-x-daemon
make
sudo make install
sudo systemctl start chuwi-minibook-x-tablet-mode
```

### Verify Operation
```bash
# Check kernel module status
lsmod | grep chuwi
dmesg | tail -20

# Check mount matrices are applied
cat /sys/bus/iio/devices/iio:device*/in_accel_mount_matrix

# Monitor tablet mode detection
cat /sys/kernel/chuwi-minibook-x-tablet-mode/tablet_state
cat /sys/kernel/chuwi-minibook-x-tablet-mode/tablet_angle

# Watch input events
evtest /dev/input/eventX  # Find correct event device
```

### Configuration
```bash
# Adjust tablet mode thresholds
echo 200 | sudo tee /sys/kernel/chuwi-minibook-x-tablet-mode/enter_deg
echo 170 | sudo tee /sys/kernel/chuwi-minibook-x-tablet-mode/exit_deg

# Monitor real-time angle and status
watch -n 0.5 'cat /sys/kernel/chuwi-minibook-x-tablet-mode/tablet_angle /sys/kernel/chuwi-minibook-x-tablet-mode/tablet_state'
```

---

## Other Tools

### `n150-ec-byte-bios`

A Bash script for reading and writing a single byte in the Embedded Controller (EC) I/O map via debugfs.  
On the Minibook X (N150 platform), the EC byte at **offset `0xF0` (decimal 240)** controls BIOS options that are otherwise inaccessible.

#### Features
- Read a byte at a given EC offset
- Write a new byte value to an EC offset
- Display the full EC map via `od`
- Automatic detection and setup of the `ec_sys` kernel module
  - Ensures `write_support` is enabled
  - If the module is not loaded, attempts to load it safely with `write_support=n`
  - Reloads with `write_support=y` only when necessary
- Safety features:
  - Requires root
  - Dry-run mode (`-n`) to preview without writing
  - Explicit `--i-understand-this-could-destroy-my-ec` flag required to confirm risk
  - If omitted, shows a 5-second countdown warning before performing a write
- Restores `ec_sys` module state on exit if it was reloaded by the script

#### Requirements
- Linux kernel with `CONFIG_ACPI_EC_DEBUGFS` enabled (built-in or as a module)
- Root privileges
- `debugfs` mounted at `/sys/kernel/debug`

#### Usage

```bash
# Show help
sudo ./tools/n150-ec-byte-bios -h

# Just do it (-i sets explicit acknowledgement this might hose your EC)
sudo ./tools/n150-ec-byte-bios -w -i

# Show EC map
sudo ./tools/n150-ec-byte-bios -m

# Read byte at default offset (0xF0 / 240)
sudo ./tools/n150-ec-byte-bios -r

# Write 0xAA to offset 0xF0 (-i sets explicit acknowledgement this might hose your EC)
sudo ./tools/n150-ec-byte-bios -w -t 0xAA -i

# Dry-run a write (no actual change)
sudo ./tools/n150-ec-byte-bios -w -t 0xAA -n
```

## Repository Layout
- tools/ — Scripts and utilities (currently: n150-ec-byte-bios)
- docs/ — Documentation, research notes, EC maps, BIOS information
README.md — This file

## Future Work
- Expand EC reverse-engineering notes
- Collect BIOS settings and quirks
- Add more utilities for power management and hardware toggles

## Disclaimer
These tools are experimental and intended for advanced users.
By using them, you acknowledge the risk of permanent hardware damage.
Neither the authors nor contributors are responsible for damage to your device.
