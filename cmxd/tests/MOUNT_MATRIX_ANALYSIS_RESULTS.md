# Mount Matrix Analysis Results

## Summary
The reverse engineering analysis of the accelerometer data reveals **significant problems** with the current mount matrices, particularly for the base sensor.

## Key Findings

### 1. **Lid Sensor (device0) - MOSTLY CORRECT**
- **Current mount matrix**: Identity (no transformation)
  ```
  [1, 0, 0]
  [0, 1, 0] 
  [0, 0, 1]
  ```
- **Z-axis behavior**: ✅ CORRECT
  - flat_screen_up: +8.05 m/s² (should be ~+9.8)
  - flat_screen_down: -11.45 m/s² (should be ~-9.8)
  - Direction is correct, magnitude slightly off but acceptable

### 2. **Base Sensor (device1) - MAJOR PROBLEM**
- **Current mount matrix**: 90° CW rotation
  ```
  [ 0, -1,  0]
  [1, 0, 0]
  [0, 0, 1]
  ```
- **Z-axis behavior**: ❌ **INVERTED**
  - flat_screen_up: -12.96 m/s² (should be ~+9.8)
  - flat_screen_down: +6.50 m/s² (should be ~-9.8)
  - **The Z-axis is completely backwards!**

## Recommended Fix

### Base Sensor Mount Matrix
The base sensor needs its Z-axis **inverted**. The corrected mount matrix should be:

```c
/* Base sensor mount matrix (90° CW rotation + Z inversion) */  
static const char * const base_sensor_mount_matrix[] = {
    "0", "-1", "0",   /* X' = -Y (laptop right = sensor front) */
    "1", "0", "0",    /* Y' = X  (laptop back = sensor right)  */
    "0", "0", "-1"    /* Z' = -Z (laptop up = sensor DOWN)     */
};
```

This changes the last row from `"0", "0", "1"` to `"0", "0", "-1"`.

## Evidence from Physical Orientations

### Flat Positions (Most Reliable)
- **Both sensors should read same gravity direction when laptop is flat**
- **Current**: Lid reads +Z up, Base reads +Z down → Opposite directions!
- **After fix**: Both would read +Z up when screen faces up

### Laptop Normal Position
- Lid sensor: Primarily -X (tilted back) - **makes sense**
- Base sensor: Primarily -Z (pointing down) - **makes sense after Z inversion**

### Side Positions  
- **Both sensors agree on Y-axis direction** - this is good!
- Left side: Both read +Y (~+9.8)
- Right side: Both read -Y (~-9.8)
- This confirms the X/Y transformations are working correctly

## Next Steps

1. **Update the mount matrix** in `cmx/cmx.c` for the base sensor
2. **Test with the corrected matrix** 
3. **Verify that flat positions show consistent gravity directions**
4. **Update the copilot instructions** to reflect the correct understanding

## Impact

This fix should resolve:
- ❌ Impossible gravity readings in flat positions
- ❌ Inconsistent hinge angle calculations 
- ❌ Poor tablet mode detection reliability
- ❌ Magnitude variations (6-13 vs expected ~9.8)

The system should become much more reliable for tablet mode detection after this correction.