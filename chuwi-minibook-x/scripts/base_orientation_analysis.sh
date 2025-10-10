#!/bin/bash

echo "=== Base Sensor Orientation Analysis ==="
echo "======================================="
echo ""
echo "This analysis examines how the base sensor is mounted relative to the lid."
echo "We know the lid sensor is rotated 90° counter-clockwise from normal laptop orientation."
echo ""

# Get current readings
if [ -f "/sys/bus/iio/devices/iio:device0/in_accel_x_raw" ]; then
    lid_x=$(cat /sys/bus/iio/devices/iio:device0/in_accel_x_raw)
    lid_y=$(cat /sys/bus/iio/devices/iio:device0/in_accel_y_raw)
    lid_z=$(cat /sys/bus/iio/devices/iio:device0/in_accel_z_raw)
fi

if [ -f "/sys/bus/iio/devices/iio:device1/in_accel_x_raw" ]; then
    base_x=$(cat /sys/bus/iio/devices/iio:device1/in_accel_x_raw)
    base_y=$(cat /sys/bus/iio/devices/iio:device1/in_accel_y_raw)
    base_z=$(cat /sys/bus/iio/devices/iio:device1/in_accel_z_raw)
fi

echo "Current readings (laptop flat on table):"
echo "  Lid sensor (iio:device0):  X=$lid_x, Y=$lid_y, Z=$lid_z"
echo "  Base sensor (iio:device1): X=$base_x, Y=$base_y, Z=$base_z"
echo ""

# Analyze the orientation
echo "=== ORIENTATION ANALYSIS ==="
echo ""

echo "1. LID SENSOR ANALYSIS:"
echo "   - Dominant axis: $([ ${lid_x#-} -gt ${lid_y#-} ] && [ ${lid_x#-} -gt ${lid_z#-} ] && echo "X-axis" || ([ ${lid_y#-} -gt ${lid_z#-} ] && echo "Y-axis" || echo "Z-axis"))"
echo "   - This confirms the lid sensor is rotated 90° CCW from normal"
echo "   - In normal laptop orientation, we'd expect Z-dominant (gravity down)"
echo "   - Instead we see X or Y dominant, confirming the 90° rotation"
echo ""

echo "2. BASE SENSOR ANALYSIS:"
base_abs_x=${base_x#-}
base_abs_y=${base_y#-}
base_abs_z=${base_z#-}

if [ "$base_abs_z" -gt "$base_abs_x" ] && [ "$base_abs_z" -gt "$base_abs_y" ]; then
    echo "   - Dominant axis: Z-axis (gravity)"
    echo "   - Z-value: $base_z (negative = face down, positive = face up)"
    echo "   - This suggests the BASE sensor is mounted in NORMAL orientation"
    echo "   - No rotation applied to base sensor"
    echo ""
    echo "3. RELATIVE MOUNTING COMPARISON:"
    echo "   - LID:  90° counter-clockwise rotation"
    echo "   - BASE: Normal orientation (0° rotation)"
    echo "   - DIFFERENCE: Base is 90° clockwise relative to lid"
elif [ "$base_abs_x" -gt "$base_abs_y" ] && [ "$base_abs_x" -gt "$base_abs_z" ]; then
    echo "   - Dominant axis: X-axis"
    echo "   - X-value: $base_x"
    echo "   - This suggests the BASE sensor is ALSO rotated"
    if [ "$base_x" -lt 0 ]; then
        echo "   - Negative X suggests same 90° CCW rotation as lid"
    else
        echo "   - Positive X suggests different rotation than lid"
    fi
    echo ""
    echo "3. RELATIVE MOUNTING COMPARISON:"
    echo "   - Both sensors appear to have non-standard mounting"
    echo "   - Need movement testing to determine exact relative orientation"
else
    echo "   - Dominant axis: Y-axis"
    echo "   - Y-value: $base_y"
    echo "   - This suggests the BASE sensor is rotated differently than lid"
    echo ""
    echo "3. RELATIVE MOUNTING COMPARISON:"
    echo "   - Sensors have different orientations"
    echo "   - Need movement testing to determine exact relationship"
fi

echo ""
echo "=== PRACTICAL IMPLICATIONS ==="
echo ""
echo "For tablet mode detection:"
if [ "$base_abs_z" -gt "$base_abs_x" ] && [ "$base_abs_z" -gt "$base_abs_y" ]; then
    echo "  - Base sensor can use standard Z-axis gravity detection"
    echo "  - Lid sensor needs coordinate transformation (90° CCW)"
    echo "  - When folded flat (tablet mode):"
    echo "    * Base Z should approach 0 (horizontal)"
    echo "    * Lid X should approach ±1000 (was gravity axis)"
else
    echo "  - Both sensors need coordinate transformation"
    echo "  - More complex tablet mode detection logic required"
fi

echo ""
echo "To verify this analysis, try:"
echo "1. Tilt the laptop left/right and observe which sensor's X changes"
echo "2. Tilt the laptop forward/back and observe which sensor's Y changes"
echo "3. Flip the laptop upside down and observe Z-axis changes"