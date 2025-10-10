#!/bin/bash
# Interactive Sensor Identification Test
# This will help determine which sensor is in the lid vs base

# Check for command line argument
if [ "$1" = "--base-axis-test" ]; then
    exec ./base_axis_test.sh
fi

echo "=== Interactive Sensor Identification Test ==="
echo "=============================================="
echo ""
echo "Choose test type:"
echo "1. Lid/Base identification test (default)"
echo "2. Base sensor axis orientation test"
echo ""
echo -n "Enter choice (1-2, or Enter for default): "
read choice

case "$choice" in
    "2")
        echo "Starting base axis orientation test..."
        exec ./base_axis_test.sh
        ;;
    "1"|"")
        echo "Starting lid/base identification test..."
        ;;
    *)
        echo "Invalid choice. Starting default test..."
        ;;
esac

echo ""
echo "This test will help identify which sensor is in the lid vs base."
echo "We'll take readings and analyze the data automatically."
echo ""
echo "=== LAPTOP ORIENTATION REFERENCE ==="
echo "When sitting at the laptop in normal typing position:"
echo "• RIGHT = Where your RIGHT HAND naturally rests"
echo "• LEFT  = Where your LEFT HAND naturally rests"  
echo "• FRONT = Edge closest to you (where palms/wrists rest)"
echo "• BACK  = Hinge side (where screen connects, farthest from you)"
echo ""

# Arrays to store readings for analysis
declare -A readings
declare -A analysis_data

# Function to take a snapshot of accelerometer data
take_snapshot() {
    local label=$1
    local store_key=$2
    echo "=== $label ==="
    
    for device in iio:device0 iio:device1; do
        if [ -d "/sys/bus/iio/devices/$device" ]; then
            # Get I2C info
            i2c_path=$(readlink -f /sys/bus/iio/devices/$device)
            i2c_info=$(echo $i2c_path | grep -o 'i2c-[0-9]*' | head -1)
            
            # Read values
            x=$(cat /sys/bus/iio/devices/$device/in_accel_x_raw 2>/dev/null || echo "0")
            y=$(cat /sys/bus/iio/devices/$device/in_accel_y_raw 2>/dev/null || echo "0")
            z=$(cat /sys/bus/iio/devices/$device/in_accel_z_raw 2>/dev/null || echo "0")
            
            echo "$device ($i2c_info): X=$x, Y=$y, Z=$z"
            
            # Store readings for analysis
            readings["${store_key}_${device}_x"]=$x
            readings["${store_key}_${device}_y"]=$y
            readings["${store_key}_${device}_z"]=$z
        fi
    done
    echo ""
}

# Function to calculate magnitude change between two readings
calculate_change() {
    local device=$1
    local key1=$2
    local key2=$3
    
    local x1=${readings["${key1}_${device}_x"]}
    local y1=${readings["${key1}_${device}_y"]}
    local z1=${readings["${key1}_${device}_z"]}
    
    local x2=${readings["${key2}_${device}_x"]}
    local y2=${readings["${key2}_${device}_y"]}
    local z2=${readings["${key2}_${device}_z"]}
    
    # Calculate differences
    local dx=$((x2 - x1))
    local dy=$((y2 - y1))
    local dz=$((z2 - z1))
    
    # Calculate absolute differences
    dx=${dx#-}  # Remove negative sign
    dy=${dy#-}
    dz=${dz#-}
    
    # Calculate approximate magnitude of change
    local total_change=$((dx + dy + dz))
    echo $total_change
}

# Function to analyze the results
analyze_results() {
    echo "=== AUTOMATED ANALYSIS ==="
    echo "=========================="
    echo ""
    
    # Calculate changes during lid movement
    local dev0_lid_change=$(calculate_change "iio:device0" "baseline" "lid_tilted")
    local dev1_lid_change=$(calculate_change "iio:device1" "baseline" "lid_tilted")
    
    # Calculate changes during whole laptop tilt
    local dev0_laptop_change=$(calculate_change "iio:device0" "lid_returned" "laptop_tilted")
    local dev1_laptop_change=$(calculate_change "iio:device1" "lid_returned" "laptop_tilted")
    
    echo "Change Analysis:"
    echo "  During lid movement:"
    echo "    iio:device0 change: $dev0_lid_change"
    echo "    iio:device1 change: $dev1_lid_change"
    echo ""
    echo "  During whole laptop tilt:"
    echo "    iio:device0 change: $dev0_laptop_change"
    echo "    iio:device1 change: $dev1_laptop_change"
    echo ""
    
    # Determine which sensor is which based on change patterns
    local lid_sensor=""
    local base_sensor=""
    local confidence="unknown"
    
    # The lid sensor should change more during lid movement
    # The base sensor should be more stable during lid-only movement
    if [ "$dev0_lid_change" -gt "$dev1_lid_change" ]; then
        if [ "$dev0_lid_change" -gt 100 ]; then  # Significant change threshold
            lid_sensor="iio:device0"
            base_sensor="iio:device1"
            confidence="high"
        else
            lid_sensor="iio:device0"
            base_sensor="iio:device1"
            confidence="medium"
        fi
    else
        if [ "$dev1_lid_change" -gt 100 ]; then  # Significant change threshold
            lid_sensor="iio:device1"
            base_sensor="iio:device0"
            confidence="high"
        else
            lid_sensor="iio:device1"
            base_sensor="iio:device0"
            confidence="medium"
        fi
    fi
    
    # Additional validation: both sensors should respond to whole laptop movement
    local both_respond="yes"
    if [ "$dev0_laptop_change" -lt 50 ] || [ "$dev1_laptop_change" -lt 50 ]; then
        both_respond="no"
        confidence="low"
    fi
    
    echo "=== RECOMMENDATION ==="
    echo "====================="
    echo ""
    echo "Based on movement analysis:"
    echo "  LID sensor:  $lid_sensor (changes more with lid movement)"
    echo "  BASE sensor: $base_sensor (more stable during lid movement)"
    echo ""
    echo "Confidence level: $confidence"
    if [ "$both_respond" = "no" ]; then
        echo "Warning: One or both sensors showed little response to movement"
    fi
    echo ""
    
    # Get current module mapping
    local current_lid_bus=$(cat /sys/module/chuwi_minibook_x/parameters/lid_bus)
    local current_base_bus=$(cat /sys/module/chuwi_minibook_x/parameters/base_bus)
    
    # Determine I2C buses for each device
    local dev0_bus=""
    local dev1_bus=""
    if [ -L "/sys/bus/iio/devices/iio:device0" ]; then
        dev0_path=$(readlink -f /sys/bus/iio/devices/iio:device0)
        dev0_bus=$(echo $dev0_path | sed -n 's|.*/i2c-\([0-9]\+\)/.*|\1|p')
    fi
    if [ -L "/sys/bus/iio/devices/iio:device1" ]; then
        dev1_path=$(readlink -f /sys/bus/iio/devices/iio:device1)
        dev1_bus=$(echo $dev1_path | sed -n 's|.*/i2c-\([0-9]\+\)/.*|\1|p')
    fi
    
    echo "Current module parameter mapping:"
    echo "  lid_bus=$current_lid_bus (should map to LID sensor)"
    echo "  base_bus=$current_base_bus (should map to BASE sensor)"
    echo ""
    echo "Detected I2C mapping:"
    echo "  iio:device0 -> i2c-$dev0_bus"
    echo "  iio:device1 -> i2c-$dev1_bus"
    echo ""
    
    # Check if current mapping matches recommendation
    local mapping_correct="unknown"
    if [ "$lid_sensor" = "iio:device0" ] && [ "$current_lid_bus" = "$dev0_bus" ]; then
        mapping_correct="yes"
    elif [ "$lid_sensor" = "iio:device1" ] && [ "$current_lid_bus" = "$dev1_bus" ]; then
        mapping_correct="yes"
    else
        mapping_correct="no"
    fi
    
    echo "=== FINAL ASSESSMENT ==="
    echo "======================="
    if [ "$mapping_correct" = "yes" ]; then
        echo "✓ CURRENT MAPPING APPEARS CORRECT"
        echo "  Your module parameters correctly map lid/base sensors"
    elif [ "$mapping_correct" = "no" ]; then
        echo "✗ CURRENT MAPPING MAY BE INCORRECT"
        echo ""
        echo "Suggested correction:"
        if [ "$lid_sensor" = "iio:device0" ]; then
            echo "  lid_bus should be $dev0_bus (currently $current_lid_bus)"
            echo "  base_bus should be $dev1_bus (currently $current_base_bus)"
            echo ""
            echo "To fix, reload module with:"
            echo "  sudo rmmod chuwi-minibook-x"
            echo "  sudo insmod chuwi-minibook-x.ko lid_bus=$dev0_bus base_bus=$dev1_bus"
        else
            echo "  lid_bus should be $dev1_bus (currently $current_lid_bus)"
            echo "  base_bus should be $dev0_bus (currently $current_base_bus)"
            echo ""
            echo "To fix, reload module with:"
            echo "  sudo rmmod chuwi-minibook-x"
            echo "  sudo insmod chuwi-minibook-x.ko lid_bus=$dev1_bus base_bus=$dev0_bus"
        fi
    else
        echo "? UNABLE TO DETERMINE MAPPING ACCURACY"
        echo "  Analysis was inconclusive - manual verification needed"
    fi
    
    if [ "$confidence" = "low" ]; then
        echo ""
        echo "Note: Low confidence result. Consider:"
        echo "  - Ensuring sensors are working properly"
        echo "  - Making more pronounced movements during testing"
        echo "  - Running the test again"
    fi
}

# Take initial reading
echo "Step 1: Taking baseline reading with laptop in normal position..."
take_snapshot "Baseline (laptop flat on table)" "baseline"

echo "Step 2: Now, GENTLY tilt the LID back about 30-45 degrees"
echo "        (Keep the base flat on the table, only move the lid)"
echo "        REFERENCE: Tilt the screen portion away from you, toward the back"
echo "        (as if opening the laptop wider than normal)"
echo "        Press Enter when ready..."
read -r

take_snapshot "Lid tilted back" "lid_tilted"

echo "Step 3: Return the lid to normal position and press Enter..."
read -r

take_snapshot "Lid returned to normal" "lid_returned"

echo "Step 4: Now tilt the ENTIRE laptop (base + lid together) to the left"
echo "        Press Enter when ready..."
read -r

take_snapshot "Entire laptop tilted left" "laptop_tilted"

echo "Step 5: Return laptop to flat position and press Enter..."
read -r

take_snapshot "Final position" "final"

# Perform automated analysis
analyze_results

echo ""
echo "=== RAW DATA SUMMARY ==="
echo "If you want to manually verify the analysis above:"
echo ""
echo "Baseline readings:"
echo "  device0: X=${readings[baseline_iio:device0_x]}, Y=${readings[baseline_iio:device0_y]}, Z=${readings[baseline_iio:device0_z]}"
echo "  device1: X=${readings[baseline_iio:device1_x]}, Y=${readings[baseline_iio:device1_y]}, Z=${readings[baseline_iio:device1_z]}"
echo ""
echo "Lid tilted readings:"
echo "  device0: X=${readings[lid_tilted_iio:device0_x]}, Y=${readings[lid_tilted_iio:device0_y]}, Z=${readings[lid_tilted_iio:device0_z]}"
echo "  device1: X=${readings[lid_tilted_iio:device1_x]}, Y=${readings[lid_tilted_iio:device1_y]}, Z=${readings[lid_tilted_iio:device1_z]}"