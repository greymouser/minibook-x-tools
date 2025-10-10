# Chuwi MiniBook X Minimal Test Driver

## Overview

This is a minimal test version of the Chuwi MiniBook X platform driver that focuses only on hardware identification and basic accelerometer detection. All tablet mode detection, orientation calculation, and input device functionality has been removed for testing purposes.

## What Was Removed

### Functionality Removed:
- **Tablet mode detection** - All angle calculation and threshold logic
- **Screen orientation detection** - All orientation state tracking and reporting  
- **Input device setup** - No input events or SW_TABLET_MODE reporting
- **Periodic polling** - No work queues or continuous sensor monitoring
- **IIO channel setup** - No active accelerometer data reading
- **Hinge angle calculations** - All math utilities and calibration
- **Module parameters** - Removed all tuning parameters for tablet/orientation mode

### Code Removed:
- `chuwi_minibook_x_read_accelerometers()` function
- `chuwi_minibook_x_work_handler()` periodic work function  
- `chuwi_minibook_x_setup_iio_channels()` IIO setup
- `chuwi_minibook_x_setup_input_device()` input device setup
- All orientation detection structures and enums
- All tablet mode detection logic
- Power management functions (suspend/resume)

## What Was Kept

### Core Functionality Preserved:
- **System identification** - DMI table matching for hardware detection
- **ACPI device detection** - MDA6655 device verification
- **Hardware enumeration** - I2C device discovery and counting
- **Basic device instantiation** - MXC4005 accelerometer setup
- **Platform driver framework** - Probe/remove functions
- **Hardware verification** - Simple presence testing

### Files Modified:
- `chuwi-minibook-x-main.c` - Simplified to minimal hardware detection
- `chuwi-minibook-x-minimal.h` - New minimal header with basic structures
- `chuwi-minibook-x-sysfs-minimal.c` - Basic sysfs interface (status only)
- `chuwi-minibook-x-debugfs-minimal.c` - Minimal debugfs interface
- `Makefile` - Updated to build minimal driver (`chuwi-minibook-x-minimal.ko`)

## Current Features

### Hardware Detection:
- DMI system identification (Chuwi MiniBook X detection)
- ACPI device verification (MDA6655 accelerometer device)
- I2C device enumeration and counting
- Basic hardware presence testing

### Interfaces:
- **sysfs**: `/sys/bus/platform/drivers/chuwi-minibook-x/*/`
  - `hardware_status` - Overall hardware detection status
  - `enabled` - Enable/disable driver 
  - `accel_count` - Number of detected accelerometers
- **debugfs**: `/sys/kernel/debug/chuwi-minibook-x/device/`
  - `hardware_status` - Detailed hardware information

### Module Information:
- **Module name**: `chuwi-minibook-x-minimal.ko`
- **Dependencies**: `mxc4005`, `serial_multi_instantiate` (soft dependencies)
- **ACPI matching**: MDA6655 device
- **DMI matching**: Chuwi MiniBook X systems

## Usage

### Build:
```bash
make all
```

### Load/Test:
```bash
make load     # Load the driver
make test     # Load and run basic tests  
make status   # Check driver status
make dmesg    # Show recent kernel messages
make unload   # Unload the driver
```

### Expected Output:
When loaded successfully, the driver should:
1. Detect Chuwi MiniBook X hardware via DMI
2. Find MDA6655 ACPI device 
3. Enumerate and potentially instantiate MXC4005 accelerometers
4. Report hardware detection status via sysfs/debugfs
5. Log hardware discovery to kernel log

## Hardware Mapping Verified

Based on ACPI analysis and real-world verification:
- **ACPI I2C0** (PCI 0000:00:15.0) = **Linux i2c-12** = **BASE ACCELEROMETER**  
- **ACPI I2C1** (PCI 0000:00:15.1) = **Linux i2c-13** = **LID ACCELEROMETER**

Current system shows:
- `iio:device0` → `i2c-13/i2c-MDA6655:00` (lid accelerometer, visible)
- `iio:device1` → Expected on `i2c-12` (base accelerometer, may need instantiation)

## Next Steps

This minimal driver provides the foundation for:
1. **Hardware verification** - Confirming accelerometer detection works
2. **I2C bus validation** - Verifying our ACPI-to-Linux bus mapping
3. **Device instantiation testing** - Ensuring both accelerometers are accessible  
4. **Platform driver framework** - Base for adding back functionality later

Once hardware detection is validated, tablet mode and orientation functionality can be incrementally added back.