#!/bin/bash
# Quick test to verify the mount matrix fix
# This should be run with laptop in flat position

echo "=== Mount Matrix Fix Verification ==="
echo

# Get current readings
LID_PATH="/sys/bus/iio/devices/iio:device0"
BASE_PATH="/sys/bus/iio/devices/iio:device1"

if [[ ! -d "$LID_PATH" || ! -d "$BASE_PATH" ]]; then
    echo "❌ Error: Accelerometer devices not found"
    exit 1
fi

echo "Current accelerometer readings (laptop should be roughly flat):"
echo

lid_x=$(cat ${LID_PATH}/in_accel_x_raw)
lid_y=$(cat ${LID_PATH}/in_accel_y_raw)  
lid_z=$(cat ${LID_PATH}/in_accel_z_raw)

base_x=$(cat ${BASE_PATH}/in_accel_x_raw)
base_y=$(cat ${BASE_PATH}/in_accel_y_raw)
base_z=$(cat ${BASE_PATH}/in_accel_z_raw)

scale=$(cat ${LID_PATH}/in_accel_scale)

# Convert to m/s²
lid_x_ms2=$(echo "scale=2; $lid_x * $scale" | bc)
lid_y_ms2=$(echo "scale=2; $lid_y * $scale" | bc)
lid_z_ms2=$(echo "scale=2; $lid_z * $scale" | bc)

base_x_ms2=$(echo "scale=2; $base_x * $scale" | bc)
base_y_ms2=$(echo "scale=2; $base_y * $scale" | bc)
base_z_ms2=$(echo "scale=2; $base_z * $scale" | bc)

echo "Lid sensor:  [${lid_x_ms2}, ${lid_y_ms2}, ${lid_z_ms2}] m/s²"
echo "Base sensor: [${base_x_ms2}, ${base_y_ms2}, ${base_z_ms2}] m/s²"
echo

# Check Z-axis directions
if (( $(echo "$lid_z_ms2 > 5" | bc -l) )) && (( $(echo "$base_z_ms2 > 5" | bc -l) )); then
    echo "✅ SUCCESS: Both sensors show positive Z gravity (screen facing up)"
elif (( $(echo "$lid_z_ms2 < -5" | bc -l) )) && (( $(echo "$base_z_ms2 < -5" | bc -l) )); then
    echo "✅ SUCCESS: Both sensors show negative Z gravity (screen facing down)"  
else
    echo "❌ POTENTIAL ISSUE: Z-axis directions don't match"
    echo "   This could indicate:"
    echo "   - Laptop not in flat position"
    echo "   - Mount matrix still incorrect"
    echo "   - Other calibration issues"
fi

echo
echo "Expected behavior:"
echo "- When laptop is flat with screen up: both Z values should be positive (~+8 to +10)"
echo "- When laptop is flat with screen down: both Z values should be negative (~-8 to -10)"
echo "- Z values should have same sign and similar magnitude when laptop is flat"