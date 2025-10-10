# Chuwi Minibook X Integrated Platform Driver (v3)# Chuwi Minibook X Platform Driver (Module V2)



This is the unified platform driver that combines the best aspects of both previous approaches:This is the kernel-integrated version of the Chuwi Minibook X tablet mode and orientation detection driver. This module is designed to be compiled as part of the Linux kernel build system.



## Integration Overview## Features



**From module-v2:**- **Dual Tablet Mode Detection**: Automatic laptop/tablet mode switching based on hinge angle

- Proper I2C device instantiation with mount matrix support- **Screen Orientation Detection**: Automatic rotation detection (normal, left, inverted, right)

- Hardware detection via DMI matching- **Bidirectional Angle Calculation**: Support for both signed (0-360°) and unsigned (0-180°) modes

- Mount matrix coordinate transformation (90° rotations)- **Linux Input Integration**: Standard SW_TABLET_MODE and ABS_MISC rotation events

- Basic hardware sysfs interface- **Comprehensive Configuration**: Extensive sysfs interface for tuning

- **Optional Debugfs Support**: Detailed debugging interface when enabled

**From module-v1:**- **Power Management**: Proper suspend/resume support

- Complete tablet mode detection with hinge angle calculation

- Screen orientation detection## Architecture

- Input device events (SW_TABLET_MODE)

- Event-driven architecture using delayed work queues### Core Components

- Comprehensive sysfs interface for configuration and data input

- **chuwi-minibook-x-main.c**: Main platform driver with device management

## Key Features- **orientation-detection.c**: Accelerometer data processing and orientation detection

- **chuwi-minibook-x-sysfs.c**: Configuration and status interface

### 1. Event-Driven Architecture- **chuwi-minibook-x-debugfs.c**: Debug interface (optional)

- Uses delayed work queues instead of continuous polling- **chuwi-minibook-x.h**: Comprehensive driver API definitions

- Triggered immediately when new accelerometer data arrives via sysfs

- Configurable polling interval (20-5000ms, default: 200ms)### Build System Integration

- Minimizes CPU usage while maintaining responsiveness

- **Kconfig**: Kernel configuration options

### 2. Mount Matrix Support- **Makefile**: Kernel build system integration

- Automatic coordinate transformation for both accelerometers- Depends on: X86, ACPI, INPUT, IIO, MXC4005 driver

- Lid sensor: 90° counter-clockwise rotation correction

- Base sensor: 90° clockwise rotation correction## Prerequisites

- Produces standard laptop coordinates regardless of sensor orientation

- Linux kernel with IIO subsystem support

### 3. Comprehensive Tablet Mode Detection- MXC4005 accelerometer driver

- Hinge angle calculation from dual accelerometer data- Two MXC4005 accelerometers (base and lid)

- Configurable thresholds and hysteresis- X86_64 architecture with ACPI support

- Stability filtering to prevent spurious switches

- Auto-calibration of hinge axis## Kernel Configuration



### 4. Full Input Integration### Required Options

- SW_TABLET_MODE events for desktop environment integration

- Screen orientation events for automatic rotation```

- Standard Linux input subsystem complianceCONFIG_X86=y

CONFIG_ACPI=y

## ArchitectureCONFIG_INPUT=y

CONFIG_IIO=y

```CONFIG_MXC4005=y

chuwi-minibook-x-daemon (userspace)CONFIG_CHUWI_MINIBOOK_X=y

    │```

    └── reads IIO devices with mount matrix transformations

    │### Optional Debug Support

    └── writes gravity vectors to sysfs interface

            │```

            ▼CONFIG_CHUWI_MINIBOOK_X_DEBUGFS=y

chuwi-minibook-x-integrated.ko (kernel)CONFIG_DEBUG_FS=y

    │```

    ├── delayed work queue (event-driven)

    ├── tablet mode detection## Installation

    ├── input device events

    └── hardware instantiation with mount matrices### Method 1: In-Tree Build

```

1. Copy the module directory to `drivers/platform/x86/`:

## Module Parameters   ```bash

   cp -r module-v2 /path/to/kernel/drivers/platform/x86/chuwi-minibook-x

### Hardware Configuration   ```

- `lid_bus=13`: I2C bus for lid accelerometer

- `lid_addr=0x15`: I2C address for lid accelerometer  2. Add to parent Kconfig:

- `base_bus=12`: I2C bus for base accelerometer   ```

- `base_addr=0x15`: I2C address for base accelerometer   source "drivers/platform/x86/chuwi-minibook-x/Kconfig"

- `enable_mount_matrix=1`: Enable mount matrix transformations   ```



### Tablet Mode Detection3. Add to parent Makefile:

- `poll_ms=200`: Polling interval in milliseconds (20-5000)   ```

- `enter_deg=200`: Angle threshold to enter tablet mode (0-360)   obj-$(CONFIG_CHUWI_MINIBOOK_X) += chuwi-minibook-x/

- `exit_deg=170`: Angle threshold to exit tablet mode (0-360)   ```

- `hysteresis_deg=10`: Hysteresis in degrees (0-90)

- `signed_mode=1`: Use signed angle mode (0=0-180°, 1=0-360°)4. Configure and build kernel:

- `enabled=1`: Enable tablet mode detection   ```bash

   make menuconfig  # Enable CONFIG_CHUWI_MINIBOOK_X

### Debug   make

- `debug_mode=0`: Enable debug output   make modules_install

   make install

## Sysfs Interface   ```



### Data Input (from userspace daemon)### Method 2: Out-of-Tree Build

- `/sys/kernel/chuwi-minibook-x-tablet-mode/base_vec`: Base accelerometer gravity vector

- `/sys/kernel/chuwi-minibook-x-tablet-mode/lid_vec`: Lid accelerometer gravity vector1. Create external module Makefile:

   ```makefile

### Status and Configuration   obj-m += chuwi-minibook-x.o

- `/sys/kernel/chuwi-minibook-x-tablet-mode/tablet_state`: Current tablet mode (0/1)   chuwi-minibook-x-y := chuwi-minibook-x-main.o orientation-detection.o chuwi-minibook-x-sysfs.o

- `/sys/kernel/chuwi-minibook-x-tablet-mode/tablet_angle`: Current hinge angle   chuwi-minibook-x-$(CONFIG_CHUWI_MINIBOOK_X_DEBUGFS) += chuwi-minibook-x-debugfs.o

- `/sys/kernel/chuwi-minibook-x-tablet-mode/enable`: Enable/disable detection   

- (Additional configuration attributes to be added)   all:

   	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

## Event-Driven Operation   

   clean:

The key optimization is the event-driven approach:   	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

   ```

1. **Userspace daemon** reads accelerometer data from IIO devices

2. **Mount matrices** automatically transform coordinates to laptop standard2. Build and install:

3. **Daemon writes** transformed data to kernel sysfs interface   ```bash

4. **Kernel module** immediately schedules delayed work (event-driven)   make

5. **Work handler** performs tablet mode detection and reports events   sudo insmod chuwi-minibook-x.ko

6. **No continuous polling** - only processes data when it changes   ```



This approach minimizes CPU usage compared to continuous polling while maintaining responsiveness.## Configuration Interface



## Build and Installation### Module Parameters



```bash#### Hardware Configuration

make clean- `lid_bus`: I2C bus number for lid accelerometer (default: 13)

make- `lid_addr`: I2C address for lid accelerometer (default: 0x15)

sudo insmod chuwi-minibook-x-integrated.ko- `base_bus`: I2C bus number for base accelerometer (default: 12)

```- `base_addr`: I2C address for base accelerometer (default: 0x15)



## Integration Status#### Detection Configuration

- `debug_mode`: Enable debug output for hardware detection (default: false)

This is the initial integration framework. The following components need to be completed:

#### Behavioral Configuration (if full orientation detection is enabled)

- [ ] Math utility functions from v1 (dot product, cross product, angle calculations)- `poll_interval`: Polling interval in milliseconds (default: 200)

- [ ] Complete tablet mode detection algorithm from v1- `enter_threshold`: Angle to enter tablet mode in degrees (default: 200)

- [ ] Full sysfs interface with all configuration options from v1- `exit_threshold`: Angle to exit tablet mode in degrees (default: 170)

- [ ] Screen orientation detection from v1- `hysteresis`: Hysteresis for switching in degrees (default: 10)

- [ ] Debugfs interface from v1- `signed_mode`: Use signed angle mode 0-360° (default: true)

- [ ] Proper error handling and edge cases- `enable_orientation`: Enable orientation detection (default: true)

- [ ] Testing with mount matrix transformations

Example hardware configuration:

## Companion Daemon```bash

# Load with custom I2C configuration

The userspace daemon in `../chuwi-minibook-x-daemon/` provides the data feed for this kernel module. It reads from IIO devices with mount matrix support and feeds transformed coordinates to the kernel module via sysfs.sudo insmod chuwi-minibook-x.ko lid_bus=14 lid_addr=0x16 base_bus=11 base_addr=0x14



## Future Optimizations# Load with debug enabled

sudo insmod chuwi-minibook-x.ko debug_mode=1

- Investigate IIO device event notifications for even more event-driven operation

- Consider inotify or similar mechanisms for sysfs write detection# Load with custom behavioral parameters (if orientation detection enabled)

- Evaluate interrupt-based approaches where supported by hardwaresudo modprobe chuwi-minibook-x poll_interval=100 enter_threshold=210

- Optimize work queue scheduling for minimal latency vs. power consumption```

### Sysfs Interface

Located at `/sys/devices/platform/chuwi-minibook-x/`

#### Hardware Status and Configuration
- `hardware_status`: Overall driver and hardware status
- `enabled`: Enable/disable driver functionality (read/write)
- `accel_count`: Number of detected accelerometers
- `i2c_config`: I2C bus/address configuration (shows addresses in hex format)

#### Module Parameter Display
The kernel displays module parameters in decimal format at `/sys/module/chuwi_minibook_x/parameters/`, but you can view them in hex format using:
```bash
cat /sys/devices/platform/chuwi-minibook-x/i2c_config
# or use the helper script
./show_params.sh
```

#### Tablet Mode Configuration (if full orientation detection enabled)
- `tablet_enter_threshold`: Angle threshold to enter tablet mode
- `tablet_exit_threshold`: Angle threshold to exit tablet mode  
- `tablet_hysteresis`: Hysteresis for tablet mode switching
- `tablet_signed_mode`: Enable signed angle mode (0-360°)
- `tablet_force_mode`: Force tablet mode (auto/laptop/tablet)

#### Orientation Configuration
- `orientation_enabled`: Enable automatic orientation detection
- `orientation_hysteresis`: Hysteresis for orientation changes
- `orientation_stability`: Stability time in milliseconds
- `orientation_confidence_threshold`: Minimum confidence percentage
- `orientation_force`: Force specific orientation (auto/normal/left/inverted/right)

#### Status Information
- `current_angle`: Current hinge angle in degrees
- `current_tablet_mode`: Current tablet mode state (0/1)
- `orientation_current`: Current screen orientation
- `orientation_confidence`: Current orientation confidence (0-100%)
- `orientation_changes`: Number of orientation changes
- `orientation_rejected`: Number of rejected orientation changes
- `base_accel`: Raw base accelerometer data (x y z)
- `lid_accel`: Raw lid accelerometer data (x y z)

### Debugfs Interface (Optional)

When `CONFIG_CHUWI_MINIBOOK_X_DEBUGFS=y`, additional debug files are available at `/sys/kernel/debug/chuwi-minibook-x/`:

- `state`: Comprehensive driver state information
- `calibrate`: Manual hinge axis calibration (write "auto" or "x y z")
- `raw_data`: Real-time accelerometer readings
- `test`: Test commands for development
- Direct parameter access files

## Diagnostic Tools

The module includes several diagnostic scripts to help verify sensor configuration and troubleshoot issues:

### Sensor Analysis Scripts

#### `sensor_status.sh` - Quick Status Check
Provides instant analysis of current sensor configuration:
```bash
./sensor_status.sh
```
- Shows current sensor readings and orientations
- Indicates which sensors are mapped to lid/base
- Provides orientation analysis for the 90° rotated display
- No user interaction required

#### `sensor_test.sh` - Interactive Sensor Identification
Automated test to determine which sensor is in the lid vs base:
```bash
./sensor_test.sh
```
- Guides through physical movements (lid tilt, laptop tilt)
- Automatically analyzes sensor response patterns
- Provides recommendations for correct parameter mapping
- Shows confidence level of the analysis
- Suggests module parameter corrections if needed

**Advanced Option**: Base sensor axis orientation test:
```bash
./sensor_test.sh
# Select option 2 for base axis test
# OR
./base_axis_test.sh
```
- Determines exact X/Y axis orientations of the base sensor
- Tests whether base sensor has any rotation in "laptop mode"
- Guides through rotating the base in different directions
- Confirms coordinate system mapping for tablet mode detection

#### `sensor_analysis.sh` - Detailed Technical Analysis
Technical analysis of sensor mounting and orientation:
```bash
./sensor_analysis.sh
```
- Detailed accelerometer data analysis
- Gravity vector analysis
- Hardware mounting orientation assessment
- Useful for understanding the 90° display rotation impact

#### `show_params.sh` - Parameter Display Helper
Shows module parameters in both decimal and hex formats:
```bash
./show_params.sh
```
- Displays addresses in hex format (more intuitive for I2C)
- Shows current configuration from kernel messages
- Useful for parameter verification

#### `base_axis_test.sh` - Base Sensor Coordinate System Test
Determines the exact orientation of the base sensor's coordinate system:
```bash
./base_axis_test.sh
```
- Interactive test requiring physical laptop rotation
- Maps sensor X/Y axes to physical laptop directions
- Determines if base sensor has any rotation relative to standard laptop coordinates
- Critical for understanding tablet mode detection requirements
- Compares base sensor orientation with known lid sensor rotation

**Test Process:**
1. Baseline reading (laptop flat)
2. Right side up test
3. Left side up test  
4. Front edge up test
5. Back edge up test
6. Automatic analysis of coordinate mapping

**Results Help Determine:**
- Whether base sensor follows standard laptop coordinate system
- Any rotation applied to base sensor mounting
- Coordinate transformation requirements for tablet mode
- Relative orientation between lid and base sensors

### Hardware Configuration Verification

If you suspect the sensor mapping is incorrect, follow this process:

1. **Quick Check**: Run `./sensor_status.sh` to see current state
2. **Physical Test**: Run `./sensor_test.sh` for automated analysis
3. **Apply Fix**: If mapping is wrong, the test script will provide exact commands to fix it

Example of correcting sensor mapping:
```bash
# If the test determines the mapping is swapped
sudo rmmod chuwi-minibook-x
sudo insmod chuwi-minibook-x.ko lid_bus=12 base_bus=13
```

## Usage Examples

### Basic Configuration

```bash
# Set tablet mode thresholds
echo 190 > /sys/devices/platform/chuwi-minibook-x/tablet_enter_threshold
echo 160 > /sys/devices/platform/chuwi-minibook-x/tablet_exit_threshold

# Enable orientation detection with 1-second stability
echo 1 > /sys/devices/platform/chuwi-minibook-x/orientation_enabled
echo 1000 > /sys/devices/platform/chuwi-minibook-x/orientation_stability

# Force tablet mode for testing
echo tablet > /sys/devices/platform/chuwi-minibook-x/tablet_force_mode

# Return to automatic mode
echo auto > /sys/devices/platform/chuwi-minibook-x/tablet_force_mode
```

### Monitoring Status

```bash
# Watch tablet mode changes
watch cat /sys/devices/platform/chuwi-minibook-x/current_tablet_mode

# Monitor orientation
watch "cat /sys/devices/platform/chuwi-minibook-x/orientation_current; \
       echo -n 'Confidence: '; \
       cat /sys/devices/platform/chuwi-minibook-x/orientation_confidence"

# View raw accelerometer data
cat /sys/devices/platform/chuwi-minibook-x/base_accel
cat /sys/devices/platform/chuwi-minibook-x/lid_accel
```

### Desktop Integration

The driver reports standard Linux input events:

- **SW_TABLET_MODE**: For desktop environment tablet mode detection
- **ABS_MISC**: For screen rotation (0=normal, 1=left, 2=inverted, 3=right)

Most desktop environments (GNOME, KDE, etc.) automatically respond to these events.

## Development Notes

### Current Limitations

1. **IIO Channel Setup**: The current implementation includes placeholder code for IIO channel detection. Proper ACPI device matching needs to be implemented.

2. **Device Instantiation**: Currently uses manual platform device creation for testing. Production version should use ACPI enumeration.

3. **Hinge Calibration**: Auto-calibration is simplified. More sophisticated calibration based on device movement patterns could be implemented.

### Architecture Decisions

- **Fixed-Point Math**: Uses scaled integer arithmetic (1e6) for mathematical operations to avoid floating-point in kernel space
- **Periodic Polling**: Uses delayed work queue for sensor polling rather than interrupt-driven approach
- **Input Abstraction**: Uses Linux input subsystem for event reporting to maintain compatibility
- **Modular Design**: Separates orientation detection, sysfs interface, and debugfs for maintainability

### Future Enhancements

1. **ACPI Integration**: Proper ACPI device tree integration for automatic device detection
2. **Interrupt Support**: Use accelerometer interrupt capabilities for more efficient operation
3. **Thermal Awareness**: Monitor system thermal state to adjust polling frequency
4. **Machine Learning**: Implement more sophisticated orientation detection algorithms
5. **Calibration Persistence**: Store calibration data across reboots

## Troubleshooting

### Module Loading Issues

```bash
# Check if MXC4005 driver is loaded
lsmod | grep mxc4005

# Check for ACPI devices
ls /sys/bus/acpi/devices/ | grep -i accel

# Verify IIO devices
ls /sys/bus/iio/devices/
```

### Debug Information

```bash
# Enable dynamic debugging (if supported)
echo 'module chuwi_minibook_x +p' > /sys/kernel/debug/dynamic_debug/control

# Check kernel logs
dmesg | grep -i chuwi
journalctl -k | grep -i minibook
```

### Common Issues

1. **No accelerometer data**: Check if MXC4005 driver is properly loaded and devices are detected
2. **Incorrect orientation**: Verify accelerometer mounting and adjust orientation logic if needed
3. **Tablet mode not working**: Check angle thresholds and ensure signed mode is configured correctly

## Related Projects

- **module-v1**: Reference implementation with extensive testing and development history
- **MXC4005 Driver**: Required IIO driver for accelerometer access
- **GNOME Shell**: Desktop environment with tablet mode support
- **iio-sensor-proxy**: User-space daemon for sensor data processing (alternative approach)

## License

GPL-2.0-or-later

## Contributing

This driver is part of the Chuwi Minibook X tools project. Contributions should:

1. Follow Linux kernel coding standards
2. Include proper SPDX license headers
3. Test on actual hardware when possible
4. Update documentation for API changes

## References

- Linux IIO Subsystem Documentation
- Linux Input Subsystem Documentation  
- MXC4005 Accelerometer Datasheet
- Chuwi Minibook X Hardware Documentation