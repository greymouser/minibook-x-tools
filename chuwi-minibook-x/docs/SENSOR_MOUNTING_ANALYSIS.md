# Base Sensor Mounting Analysis

## Summary of Findings

Based on our sensor data analysis, we can determine how the base sensor is mounted relative to the lid sensor:

### Lid Sensor (iio:device0, i2c-13)
- **Mounting**: 90° counter-clockwise rotation from normal laptop orientation
- **Evidence**: X-axis dominant (~-1000) when laptop is flat on table
- **Expected behavior**: In normal orientation, Z-axis should be dominant for gravity
- **Actual behavior**: X-axis carries the gravity vector instead

### Base Sensor (iio:device1, i2c-12)  
- **Mounting**: Normal orientation (0° rotation)
- **Evidence**: Z-axis dominant (~-1300) when laptop is flat on table
- **Expected behavior**: Z-axis dominant for gravity (face down = negative)
- **Actual behavior**: Matches expected normal orientation

### Relative Orientation
- **Lid to Base relationship**: Lid sensor is rotated 90° counter-clockwise relative to base
- **Base to Lid relationship**: Base sensor is rotated 90° clockwise relative to lid
- **Angular difference**: 90° between the two sensors

### Practical Implications

#### For Tablet Mode Detection:
1. **Base sensor**: Can use standard accelerometer logic
   - Z ≈ 0 when horizontal (tablet mode)
   - Z ≈ ±1000 when vertical (laptop mode)

2. **Lid sensor**: Requires coordinate transformation
   - X ≈ 0 when horizontal (tablet mode) 
   - X ≈ ±1000 when vertical (laptop mode)

#### For Screen Rotation:
- Base sensor follows standard laptop conventions
- Lid sensor needs 90° CCW coordinate transformation:
  ```
  Standard coords → Lid sensor coords
  X → Y
  Y → -X  
  Z → Z
  ```

### Verification Data
- **Baseline flat**: Base X=21, Lid X=-1000
- **Tilt right**: Base X=25 (+4 change), confirms normal X-axis behavior
- **Z-axis dominance**: Base consistently Z-dominant, Lid consistently X-dominant

This mounting difference explains why the Chuwi Minibook X requires special handling - the manufacturer mounted the lid accelerometer rotated relative to the base, likely to accommodate the 90° rotated display panel.