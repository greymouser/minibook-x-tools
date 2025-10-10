# Base Axis Test Script Fixes

## Issues Found and Fixed

### 1. **Duplicate/Conflicting Y-Axis Analysis**
**Problem**: There was old Y-axis analysis code using deprecated logic that interfered with the new comprehensive analysis.

**Fix**: Removed the duplicate Y-axis analysis section that was using old variables (`y_direction`) and old thresholds (800).

### 2. **Coordinate Mapping Logic Flaw**
**Problem**: The coordinate mapping logic could only set X or Y points from one direction test. If Y-axis responded to left/right tilts, then `y_points` would be set but `x_points` would remain empty.

**Fix**: 
- Added logic to prevent overwriting coordinates if already determined
- Added conflict detection for when same axis responds to both directions
- Added default values for unknown coordinates
- Added proper error handling for incomplete/conflicting data

### 3. **Incomplete Error Handling**
**Problem**: Script didn't properly handle cases where measurements were insufficient or conflicting.

**Fix**: Added comprehensive error cases:
- `CONFLICT`: Same axis responds to both left/right and front/back
- `INCOMPLETE`: One or both axes couldn't be determined  
- `UNKNOWN`: Coordinates that couldn't be determined
- Better diagnostic messages for each error type

## Key Improvements

### Coordinate Mapping Logic
```bash
# Old logic (flawed)
case "$lr_direction" in
    "X+=RIGHT") x_points="RIGHT" ;;
esac
case "$fb_direction" in  
    "X+=FRONT") x_points="FRONT" ;;  # Could overwrite!
esac

# New logic (robust)
case "$lr_direction" in
    "X+=RIGHT") x_points="RIGHT" ;;
esac
case "$fb_direction" in
    "X+=FRONT") 
        if [ -z "$x_points" ]; then  # Only set if not already set
            x_points="FRONT"
        fi
        ;;
esac
```

### Conflict Detection
```bash
# Check if same axis identified for both directions
lr_axis_name="${lr_direction%+=*}"  # Extract "X", "Y", or "Z"
fb_axis_name="${fb_direction%+=*}"

if [ "$lr_axis_name" = "$fb_axis_name" ]; then
    echo "WARNING: Same axis detected for both directions!"
    x_points="CONFLICT"
    y_points="CONFLICT"
fi
```

### Enhanced Error Messages
- **CONFLICT**: Explains that same axis responded to both tests
- **INCOMPLETE**: Shows which coordinates were determined vs unknown
- **UNCLEAR**: Provides specific detected values for debugging

## Result
The script now:
1. **Properly maps both X and Y coordinates** without interference
2. **Detects and reports measurement conflicts** 
3. **Provides specific diagnostic information** for troubleshooting
4. **Handles edge cases gracefully** with clear error messages
5. **Gives actionable feedback** for retrying with better movements

This should resolve the issue where X was being detected but Y was not.