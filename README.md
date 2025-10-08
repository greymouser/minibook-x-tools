# Chuwi Minibook X Tools & Documentation

This repository is a collection of tools, scripts, and information related to the **Chuwi Minibook X**.  
The goal is to document quirks of this platform and provide utilities to interact with system features that are otherwise undocumented.

> ⚠️ **Warning**  
> Many of these tools interact directly with low-level system components such as the Embedded Controller (EC).  
> **Use at your own risk.** Misuse can permanently damage your device.

---

## Tools

### `chuwi-minibook-x-tablet-mode` - Intelligent Laptop/Tablet Mode Switching

A sophisticated kernel module that provides automatic laptop/tablet mode switching based on hinge angle detection using dual accelerometers. Features true 360° angle detection with bidirectional mode switching and intelligent auto-calibration.

#### Features
- **True 360° Angle Detection**: Accurate hinge angle measurement from 0° to 360° without dead zones or downward spirals
- **Bidirectional Mode Switching**: 
  - Enters tablet mode when hinge angle crosses **200°** (past typical 180° laptop position)
  - Returns to laptop mode when hinge angle crosses back below **170°** 
  - 30° hysteresis prevents oscillation between modes
- **Auto-Calibration**: Automatically determines hinge axis orientation from real accelerometer data
- **Enhanced Orientation Processing**: Dual-sensor orientation detection with moving average filtering
- **Flexible Force Modes**: Manual override support ("laptop", "tablet", "auto") with both string and numeric input
- **Comprehensive sysfs Interface**: Real-time angle monitoring, threshold adjustment, calibration status

#### Quick Installation
```bash
# Clone and install with optimized defaults
git clone <this-repo>
cd minibook-x-tools
sudo ./install.sh
```

#### Manual Installation
```bash
cd module-v1/module
make
sudo rmmod chuwi_minibook_x_tablet_mode 2>/dev/null || true
sudo insmod chuwi-minibook-x-tablet-mode.ko
```

#### Usage
The module works automatically once loaded. Monitor status:
```bash
# Check current status
cat /sys/kernel/chuwi-minibook-x-tablet-mode/angle   # Current hinge angle (0-360°)
cat /sys/kernel/chuwi-minibook-x-tablet-mode/state   # Current mode (0=laptop, 1=tablet)

# Adjust thresholds if needed
echo 200 | sudo tee /sys/kernel/chuwi-minibook-x-tablet-mode/enter_deg  # Tablet mode entry
echo 170 | sudo tee /sys/kernel/chuwi-minibook-x-tablet-mode/exit_deg   # Laptop mode exit

# Force specific mode
echo "laptop" | sudo tee /sys/kernel/chuwi-minibook-x-tablet-mode/force
echo "tablet" | sudo tee /sys/kernel/chuwi-minibook-x-tablet-mode/force
echo "auto" | sudo tee /sys/kernel/chuwi-minibook-x-tablet-mode/force
```

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
