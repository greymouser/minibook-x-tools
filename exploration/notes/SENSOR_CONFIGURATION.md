# Chuwi Minibook X Sensor Configuration

## Overview

This document records the definitive sensor orientation analysis for the Chuwi Minibook X laptop's accelerometer sensors, which are critical for tablet mode detection.

## Hardware Configuration

### Sensor Details
- **Lid Sensor**: MXC4005 accelerometer on I2C bus 13, address 0x15 (iio:device0)
- **Base Sensor**: MXC4005 accelerometer on I2C bus 12, address 0x15 (iio:device1)

### Display Orientation
- **Display**: Rotated 90° counter-clockwise from standard laptop orientation
- **Impact**: Both sensors require coordinate transformation for proper tablet mode detection

## Sensor Orientation Analysis

### Lid Sensor (iio:device0)
- **Rotation**: 90° counter-clockwise (90° CCW)
- **Coordinate Mapping**: 
  - Standard laptop X+ (right) → Sensor Y+ 
  - Standard laptop Y+ (back) → Sensor X-
  - Standard laptop Z+ (up) → Sensor Z+

### Base Sensor (iio:device1) 
- **Rotation**: 90° clockwise (90° CW)
- **Coordinate Mapping**:
  - X+ = BACK (opposite of standard)
  - Y+ = LEFT (opposite of standard) 
  - Z+ = UP (same as standard)

## Coordinate Transformation Requirements

### Standard Laptop Coordinates
```
X+ = Right side of laptop
Y+ = Back edge (hinge side)
Z+ = Up (away from table)
```

### Lid Sensor Transformation (90° CCW)
```
Standard → Lid Sensor
X+ (right) → Y+
Y+ (back)  → X-
Z+ (up)    → Z+
```

### Base Sensor Transformation (90° CW)
```
Standard → Base Sensor  
X+ (right) → Y-
Y+ (back)  → X+
Z+ (up)    → Z+
```

## Test Results Summary

### Test Data Analysis (Date: October 9, 2025)
```
LEFT/RIGHT TILT ANALYSIS:
- Y-axis shows consistent opposite responses: +1049 vs -1009
- Total Y response: 2058 (highest magnitude)
- Direction consistency: Y=1 (perfect opposite directions)
- Result: Y+ points toward LEFT side of laptop

FRONT/BACK TILT ANALYSIS:  
- X-axis shows consistent opposite responses: -1031 vs +994
- Total X response: 2025 (highest magnitude)
- Direction consistency: X=1 (perfect opposite directions)  
- Result: X+ points toward BACK of laptop

VERIFICATION:
- 90° rotation tests confirmed primary analysis
- Consistent response patterns across all test positions
```

## Tablet Mode Detection Implications

### Complex Configuration Required
- **Challenge**: Base and lid sensors have opposite rotations (90° CW vs 90° CCW)
- **Solution**: Each sensor requires different coordinate transformation logic
- **Implementation**: Cannot use unified transformation - must handle each sensor separately

### Recommended Detection Algorithm

#### For Lid Sensor (90° CCW):
```bash
# Transform lid sensor readings to standard coordinates
lid_std_x = lid_sensor_y    # lid Y becomes standard X
lid_std_y = -lid_sensor_x   # lid -X becomes standard Y  
lid_std_z = lid_sensor_z    # lid Z stays standard Z
```

#### For Base Sensor (90° CW):
```bash  
# Transform base sensor readings to standard coordinates
base_std_x = -base_sensor_y  # base -Y becomes standard X
base_std_y = base_sensor_x   # base X becomes standard Y
base_std_z = base_sensor_z   # base Z stays standard Z
```

#### Tablet Mode Detection Logic:
```bash
# After coordinate transformation, check for horizontal orientation
if (abs(lid_std_z) < threshold && abs(base_std_z) < threshold); then
    # Both sensors show near-zero Z (horizontal) = Tablet mode
    tablet_mode_detected=true
fi
```

## Module Parameters

### Current Configuration
```bash
# Module parameters for I2C device instantiation
lid_bus=13      # I2C bus for lid sensor
lid_addr=0x15   # I2C address for lid sensor  
base_bus=12     # I2C bus for base sensor
base_addr=0x15  # I2C address for base sensor
```

### Loading Module with Custom Parameters
```bash
sudo modprobe chuwi-minibook-x lid_bus=13 lid_addr=0x15 base_bus=12 base_addr=0x15
```

## Diagnostic Tools

### Available Scripts
- `sensor_test.sh` - Interactive sensor identification and testing
- `base_axis_test.sh` - Comprehensive base sensor orientation analysis  
- `sensor_status.sh` - Quick sensor status and orientation display

### Verification Commands
```bash
# Check sensor readings
cat /sys/bus/iio/devices/iio:device0/in_accel_*_raw  # Lid sensor
cat /sys/bus/iio/devices/iio:device1/in_accel_*_raw  # Base sensor

# View hex addresses
cat /sys/devices/platform/chuwi-minibook-x/lid_addr_hex
cat /sys/devices/platform/chuwi-minibook-x/base_addr_hex
```

## Key Findings

1. **Opposite Rotations**: Base sensor (90° CW) and lid sensor (90° CCW) have opposite orientations
2. **Complex Transformation**: Each sensor requires different coordinate transformation matrix
3. **Reliable Detection**: Both sensors provide strong, consistent gravity responses for orientation detection
4. **Verification Successful**: 90° rotation tests confirm coordinate mapping accuracy

## Implementation Notes

- Both sensors are fully functional and provide reliable orientation data
- Coordinate transformations are essential for accurate tablet mode detection
- The opposite rotations make unified sensor handling impossible
- Custom logic required for each sensor's coordinate system

---

**Document Generated**: October 9, 2025  
**Test Environment**: Chuwi Minibook X with module-v2 driver  
**Verification Status**: ✅ Complete sensor orientation analysis confirmed