#!/bin/bash
# Universal MXC4005 Sensor Movement Testing Script
# Interactive script to help determine sensor mounting orientation through guided movements
# Works on any system with MXC4005 accelerometers, no special drivers required

set -e

echo "=== Universal MXC4005 Sensor Movement Analysis ==="
echo "=================================================="
echo ""
echo "This interactive script will guide you through a series of laptop movements"
echo "to help determine how accelerometer sensors are mounted and which sensor"
echo "corresponds to the lid vs base in convertible laptops."
echo ""
echo "The test works by analyzing how sensor readings change during specific movements."
echo "No special drivers or root access required - just standard IIO sensor access."
echo ""

# Arrays to store readings for analysis
declare -A readings
mxc4005_devices=()

# Function to safely read a file
safe_read() {
    local file="$1"
    if [ -f "$file" ]; then
        cat "$file" 2>/dev/null || echo "0"
    else
        echo "0"
    fi
}

# Function to get I2C information
get_i2c_info() {
    local device_path="$1"
    local real_path=$(readlink -f "$device_path" 2>/dev/null || echo "$device_path")
    
    # Extract I2C bus number
    local i2c_info=$(echo "$real_path" | grep -o 'i2c-[0-9]\+' | head -1)
    local i2c_bus=$(echo "$i2c_info" | sed 's/i2c-//' 2>/dev/null || echo "unknown")
    
    # Extract device address
    local addr_part=$(echo "$real_path" | grep -o '[0-9a-f]\+-[0-9a-f]\+' | tail -1)
    local i2c_addr=""
    if [ -n "$addr_part" ]; then
        i2c_addr=$(echo "$addr_part" | cut -d'-' -f2)
    else
        i2c_addr=$(echo "$real_path" | sed -n 's/.*\/[0-9]\+-\([0-9a-f]\+\)\/.*/\1/p')
    fi
    
    echo "i2c-$i2c_bus:0x$i2c_addr"
}

# Function to find MXC4005 devices
find_mxc4005_devices() {
    echo "Scanning for MXC4005 accelerometer sensors..."
    
    for device_path in /sys/bus/iio/devices/iio:device*; do
        if [ -d "$device_path" ]; then
            device=$(basename "$device_path")
            name=$(safe_read "$device_path/name")
            
            if [ "$name" = "mxc4005" ]; then
                mxc4005_devices+=("$device")
                i2c_info=$(get_i2c_info "$device_path")
                echo "  Found: $device ($i2c_info)"
            fi
        fi
    done
    
    echo "  Total MXC4005 devices found: ${#mxc4005_devices[@]}"
    echo ""
    
    if [ "${#mxc4005_devices[@]}" -eq 0 ]; then
        echo "ERROR: No MXC4005 sensors found!"
        echo ""
        echo "This script requires MXC4005 accelerometers. Please verify:"
        echo "• System has MXC4005 hardware (common in Chuwi Minibook X)"
        echo "• Appropriate drivers are loaded"
        echo "• IIO subsystem is available"
        exit 1
    fi
    
    if [ "${#mxc4005_devices[@]}" -eq 1 ]; then
        echo "WARNING: Only found 1 MXC4005 sensor."
        echo "Convertible laptops typically have 2 sensors (lid + base)."
        echo "Proceeding anyway - results may be limited."
        echo ""
    fi
}

# Function to take a snapshot of all MXC4005 sensor readings
take_snapshot() {
    local label="$1"
    local store_key="$2"
    
    echo "=== $label ==="
    
    for device in "${mxc4005_devices[@]}"; do
        local device_path="/sys/bus/iio/devices/$device"
        
        # Read current values
        local x=$(safe_read "$device_path/in_accel_x_raw")
        local y=$(safe_read "$device_path/in_accel_y_raw")
        local z=$(safe_read "$device_path/in_accel_z_raw")
        
        # Get I2C info for display
        local i2c_info=$(get_i2c_info "$device_path")
        
        echo "  $device ($i2c_info): X=$x, Y=$y, Z=$z"
        
        # Store readings for analysis
        readings["${store_key}_${device}_x"]=$x
        readings["${store_key}_${device}_y"]=$y
        readings["${store_key}_${device}_z"]=$z
    done
    echo ""
}

# Function to calculate total change magnitude between two readings
calculate_change() {
    local device="$1"
    local key1="$2"
    local key2="$3"
    
    local x1=${readings["${key1}_${device}_x"]}
    local y1=${readings["${key1}_${device}_y"]}
    local z1=${readings["${key1}_${device}_z"]}
    
    local x2=${readings["${key2}_${device}_x"]}
    local y2=${readings["${key2}_${device}_y"]}
    local z2=${readings["${key2}_${device}_z"]}
    
    # Calculate absolute differences
    local dx=$(( (x2 - x1) ))
    local dy=$(( (y2 - y1) ))
    local dz=$(( (z2 - z1) ))
    
    # Convert to absolute values
    dx=${dx#-}
    dy=${dy#-}  
    dz=${dz#-}
    
    # Calculate total magnitude of change
    local total_change=$((dx + dy + dz))
    echo $total_change
}

# Function to analyze axis-specific changes
analyze_axis_changes() {
    local device="$1"
    local key1="$2"
    local key2="$3"
    
    local x1=${readings["${key1}_${device}_x"]}
    local y1=${readings["${key1}_${device}_y"]}
    local z1=${readings["${key1}_${device}_z"]}
    
    local x2=${readings["${key2}_${device}_x"]}
    local y2=${readings["${key2}_${device}_y"]}
    local z2=${readings["${key2}_${device}_z"]}
    
    local dx=$((x2 - x1))
    local dy=$((y2 - y1))
    local dz=$((z2 - z1))
    
    # Find dominant axis change
    local abs_dx=${dx#-}
    local abs_dy=${dy#-}
    local abs_dz=${dz#-}
    
    local max_change=$abs_dx
    local dominant_axis="X"
    local dominant_direction="+"
    
    if [ "$abs_dy" -gt "$max_change" ]; then
        max_change=$abs_dy
        dominant_axis="Y"
    fi
    
    if [ "$abs_dz" -gt "$max_change" ]; then
        max_change=$abs_dz
        dominant_axis="Z"
    fi
    
    # Determine direction
    case "$dominant_axis" in
        "X") [ "$dx" -lt 0 ] && dominant_direction="-" ;;
        "Y") [ "$dy" -lt 0 ] && dominant_direction="-" ;;
        "Z") [ "$dz" -lt 0 ] && dominant_direction="-" ;;
    esac
    
    echo "$dominant_axis$dominant_direction ($max_change)"
}

# Function to analyze all the movement data
analyze_results() {
    echo "=== MOVEMENT ANALYSIS ==="
    echo "========================="
    echo ""
    
    if [ "${#mxc4005_devices[@]}" -eq 1 ]; then
        echo "Single sensor analysis:"
        local device="${mxc4005_devices[0]}"
        
        echo "Movement responses:"
        local lid_change=$(calculate_change "$device" "baseline" "lid_tilted")
        local whole_change=$(calculate_change "$device" "lid_returned" "whole_tilted")
        local left_change=$(calculate_change "$device" "whole_returned" "left_tilt")
        
        echo "  Lid tilt: $lid_change"
        echo "  Whole laptop tilt (back): $whole_change"
        echo "  Left side tilt: $left_change"
        echo ""
        
        echo "Axis analysis:"
        echo "  Lid tilt - dominant change: $(analyze_axis_changes "$device" "baseline" "lid_tilted")"
        echo "  Whole tilt - dominant change: $(analyze_axis_changes "$device" "lid_returned" "whole_tilted")"
        echo "  Left tilt - dominant change: $(analyze_axis_changes "$device" "whole_returned" "left_tilt")"
        echo ""
        
        echo "Sensor mounting assessment:"
        if [ "$lid_change" -gt 100 ]; then
            echo "  ✓ Sensor responds to lid movement - likely LID sensor"
        else
            echo "  ? Weak response to lid movement - may be BASE sensor or poorly mounted"
        fi
        
        if [ "$whole_change" -gt 100 ]; then
            echo "  ✓ Sensor responds to whole laptop movement - normal behavior"
        else
            echo "  ⚠ Weak response to whole laptop movement - check sensor mounting"
        fi
        
    elif [ "${#mxc4005_devices[@]}" -eq 2 ]; then
        echo "Dual sensor analysis:"
        local dev1="${mxc4005_devices[0]}"
        local dev2="${mxc4005_devices[1]}"
        
        # Calculate changes for different movements
        local dev1_lid_change=$(calculate_change "$dev1" "baseline" "lid_tilted")
        local dev2_lid_change=$(calculate_change "$dev2" "baseline" "lid_tilted")
        
        local dev1_whole_change=$(calculate_change "$dev1" "lid_returned" "whole_tilted")
        local dev2_whole_change=$(calculate_change "$dev2" "lid_returned" "whole_tilted")
        
        local dev1_left_change=$(calculate_change "$dev1" "whole_returned" "left_tilt")
        local dev2_left_change=$(calculate_change "$dev2" "whole_returned" "left_tilt")
        
        echo "Change magnitudes:"
        echo "                    $dev1        $dev2"
        echo "  Lid tilt:         $dev1_lid_change           $dev2_lid_change"
        echo "  Whole tilt:       $dev1_whole_change           $dev2_whole_change"
        echo "  Left tilt:        $dev1_left_change           $dev2_left_change"
        echo ""
        
        echo "Axis analysis:"
        echo "  $dev1 lid tilt: $(analyze_axis_changes "$dev1" "baseline" "lid_tilted")"
        echo "  $dev2 lid tilt: $(analyze_axis_changes "$dev2" "baseline" "lid_tilted")"
        echo ""
        
        # Determine which sensor is which
        local lid_sensor=""
        local base_sensor=""
        local confidence="medium"
        
        if [ "$dev1_lid_change" -gt "$dev2_lid_change" ]; then
            if [ "$dev1_lid_change" -gt 150 ]; then
                lid_sensor="$dev1"
                base_sensor="$dev2"
                confidence="high"
            else
                lid_sensor="$dev1"
                base_sensor="$dev2"
                confidence="medium"
            fi
        else
            if [ "$dev2_lid_change" -gt 150 ]; then
                lid_sensor="$dev2"
                base_sensor="$dev1"
                confidence="high"
            else
                lid_sensor="$dev2"
                base_sensor="$dev1"
                confidence="medium"
            fi
        fi
        
        # Validation checks
        if [ "$dev1_whole_change" -lt 50 ] || [ "$dev2_whole_change" -lt 50 ]; then
            confidence="low"
            echo "  ⚠ Warning: One sensor showed little response to whole laptop movement"
        fi
        
        echo "=== SENSOR IDENTIFICATION ==="
        echo "============================="
        echo ""
        echo "Based on movement analysis (confidence: $confidence):"
        echo "  LID sensor:  $lid_sensor (responds more to lid-only movement)"
        echo "  BASE sensor: $base_sensor (more stable during lid-only movement)"
        echo ""
        
        # Get I2C information for recommendations
        local lid_i2c=$(get_i2c_info "/sys/bus/iio/devices/$lid_sensor")
        local base_i2c=$(get_i2c_info "/sys/bus/iio/devices/$base_sensor")
        
        echo "I2C Mapping:"
        echo "  LID sensor:  $lid_sensor -> $lid_i2c"
        echo "  BASE sensor: $base_sensor -> $base_i2c"
        echo ""
        
    else
        echo "Multiple sensor analysis:"
        echo "Found ${#mxc4005_devices[@]} sensors - this is unusual for typical laptops."
        echo "Analyzing each sensor individually:"
        echo ""
        
        for device in "${mxc4005_devices[@]}"; do
            local lid_change=$(calculate_change "$device" "baseline" "lid_tilted")
            local whole_change=$(calculate_change "$device" "lid_returned" "whole_tilted")
            
            echo "$device:"
            echo "  Lid movement response: $lid_change"
            echo "  Whole laptop response: $whole_change"
            echo "  Likely role: $([ "$lid_change" -gt 100 ] && echo "LID sensor" || echo "BASE sensor or other")"
            echo ""
        done
    fi
}

# Function to provide mount matrix analysis
analyze_mount_matrices() {
    echo "=== MOUNT MATRIX ANALYSIS ==="
    echo "============================="
    echo ""
    echo "Based on the movement analysis, here are the likely mount matrix orientations:"
    echo ""
    
    for device in "${mxc4005_devices[@]}"; do
        echo "$device:"
        
        # Analyze baseline orientation
        local x_base=${readings["baseline_${device}_x"]}
        local y_base=${readings["baseline_${device}_y"]}
        local z_base=${readings["baseline_${device}_z"]}
        
        local abs_x=${x_base#-}
        local abs_y=${y_base#-}
        local abs_z=${z_base#-}
        
        echo "  Baseline (laptop flat): X=$x_base, Y=$y_base, Z=$z_base"
        
        # Determine dominant axis in flat position
        local max_val=$abs_x
        local dominant="X"
        
        if [ "$abs_y" -gt "$max_val" ]; then
            max_val=$abs_y
            dominant="Y"
        fi
        
        if [ "$abs_z" -gt "$max_val" ]; then
            max_val=$abs_z
            dominant="Z"
        fi
        
        echo "  Gravity direction: $dominant-axis dominant"
        
        # Analyze movement patterns to infer mount matrix
        local lid_x_change=$(( ${readings["lid_tilted_${device}_x"]} - ${readings["baseline_${device}_x"]} ))
        local lid_y_change=$(( ${readings["lid_tilted_${device}_y"]} - ${readings["baseline_${device}_y"]} ))
        local lid_z_change=$(( ${readings["lid_tilted_${device}_z"]} - ${readings["baseline_${device}_z"]} ))
        
        echo "  Lid tilt response: ΔX=$lid_x_change, ΔY=$lid_y_change, ΔZ=$lid_z_change"
        
        # Infer likely mount matrix characteristics
        if [ "$dominant" = "Z" ] && [ "$z_base" -gt 0 ]; then
            echo "  Mount matrix: Likely standard orientation (Z+ = down)"
        elif [ "$dominant" = "Z" ] && [ "$z_base" -lt 0 ]; then
            echo "  Mount matrix: Likely inverted Z-axis (Z- = down)"
        elif [ "$dominant" = "X" ]; then
            echo "  Mount matrix: Likely 90° rotation around Z-axis"
        elif [ "$dominant" = "Y" ]; then
            echo "  Mount matrix: Likely 90° rotation around Z-axis (different orientation)"
        fi
        
        echo ""
    done
}

# Function to generate device report
generate_device_report() {
    echo "=== DEVICE COMPATIBILITY REPORT ==="
    echo "==================================="
    echo ""
    echo "System Information:"
    echo "  Kernel: $(uname -r)"
    echo "  Architecture: $(uname -m)"
    echo "  Date: $(date)"
    echo ""
    echo "Hardware Detection:"
    echo "  MXC4005 sensors found: ${#mxc4005_devices[@]}"
    for device in "${mxc4005_devices[@]}"; do
        local i2c_info=$(get_i2c_info "/sys/bus/iio/devices/$device")
        echo "    $device: $i2c_info"
    done
    echo ""
    echo "This report can be shared to help improve device support for your hardware."
}

# Check prerequisites
if [ ! -d "/sys/bus/iio/devices" ]; then
    echo "ERROR: IIO subsystem not found!"
    echo "This system may not have accelerometer support or required drivers."
    exit 1
fi

# Find MXC4005 devices
find_mxc4005_devices

# Provide movement instructions
echo "=== MOVEMENT TEST INSTRUCTIONS ==="
echo "=================================="
echo ""
echo "This test requires 5 specific movements. Please read all instructions first:"
echo ""
echo "REFERENCE ORIENTATION (for all movements):"
echo "• Sit at laptop in normal typing position"
echo "• RIGHT = your right hand side"
echo "• LEFT = your left hand side"  
echo "• FRONT = edge closest to you (where wrists rest)"
echo "• BACK = hinge side (where screen connects)"
echo ""
echo "MOVEMENTS YOU'LL PERFORM:"
echo "1. Baseline: Laptop flat on table, screen at normal angle"
echo "2. Lid tilt: Tilt ONLY the screen back 30-45° (base stays flat)"
echo "3. Return lid: Return screen to normal position"
echo "4. Whole tilt: Tilt ENTIRE laptop backward (lift front edge)"
echo "5. Left tilt: Tilt ENTIRE laptop to the left (lift right edge)"
echo ""
echo "Ready? Press Enter to begin..."
read -r

# Perform the movement test sequence
echo ""
echo "=== MOVEMENT TEST SEQUENCE ==="
echo "=============================="
echo ""

# Step 1: Baseline
echo "Step 1: Ensure laptop is flat on table with screen at normal angle"
echo "        Press Enter when ready..."
read -r
take_snapshot "Baseline reading (laptop flat)" "baseline"

# Step 2: Lid tilt
echo "Step 2: Tilt ONLY the screen/lid back about 30-45 degrees"
echo "        Keep the base flat on table - only move the screen!"
echo "        Press Enter when positioned..."
read -r
take_snapshot "Lid tilted back" "lid_tilted"

# Step 3: Return lid
echo "Step 3: Return the screen to normal position"
echo "        Press Enter when back to normal..."
read -r
take_snapshot "Lid returned to normal" "lid_returned"

# Step 4: Whole laptop tilt back
echo "Step 4: Tilt the ENTIRE laptop backward (lift the front edge)"
echo "        Keep screen and base together - tilt the whole unit"
echo "        Press Enter when tilted..."
read -r
take_snapshot "Whole laptop tilted back" "whole_tilted"

# Step 5: Return to flat
echo "Step 5: Return laptop to flat position"
echo "        Press Enter when flat..."
read -r
take_snapshot "Laptop returned to flat" "whole_returned"

# Step 6: Left tilt
echo "Step 6: Tilt the ENTIRE laptop to the left (lift right edge)"
echo "        Press Enter when tilted left..."
read -r
take_snapshot "Laptop tilted left" "left_tilt"

# Final position
echo "Step 7: Return to normal flat position"
echo "        Press Enter when finished..."
read -r
take_snapshot "Final position" "final"

# Analyze all the data
echo ""
analyze_results
echo ""
analyze_mount_matrices
echo ""
generate_device_report

echo ""
echo "=== TEST COMPLETE ==="
echo "===================="
echo ""
echo "If you're contributing this data for device compatibility:"
echo "• Copy the above output"
echo "• Include your laptop model (exact model name/year)"
echo "• Include BIOS version if available"
echo "• Note any special driver installation steps required"
echo ""
echo "This information helps improve support for convertible laptops!"