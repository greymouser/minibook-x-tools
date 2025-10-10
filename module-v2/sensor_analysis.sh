#!/bin/bash
# Sensor Analysis Script for Chuwi Minibook X
# This script reads accelerometer data to determine sensor mounting and orientation

echo "=== Chuwi Minibook X Sensor Analysis ==="
echo "========================================"
echo ""

# Check if the driver is loaded
if [ ! -d "/sys/devices/platform/chuwi-minibook-x" ]; then
    echo "Error: chuwi-minibook-x driver not loaded"
    exit 1
fi

# Show current I2C configuration
echo "Current I2C Configuration:"
cat /sys/devices/platform/chuwi-minibook-x/i2c_config
echo ""

# Find IIO devices
echo "Available IIO devices:"
ls -la /sys/bus/iio/devices/ | grep iio:device
echo ""

# Function to read accelerometer data
read_accel_data() {
    local device=$1
    local name=$2
    
    if [ -d "/sys/bus/iio/devices/$device" ]; then
        echo "=== $name ($device) ==="
        
        # Check if this is an MXC4005 device
        if [ -f "/sys/bus/iio/devices/$device/name" ]; then
            device_name=$(cat /sys/bus/iio/devices/$device/name)
            echo "Device name: $device_name"
        fi
        
        # Get I2C info
        if [ -L "/sys/bus/iio/devices/$device" ]; then
            i2c_path=$(readlink -f /sys/bus/iio/devices/$device)
            i2c_info=$(echo $i2c_path | grep -o 'i2c-[0-9]*' | head -1)
            i2c_addr=$(echo $i2c_path | grep -o '[0-9a-f]*-[0-9a-f]*' | cut -d'-' -f2)
            echo "I2C location: $i2c_info, address: 0x$i2c_addr"
        fi
        
        # Read accelerometer values
        if [ -f "/sys/bus/iio/devices/$device/in_accel_x_raw" ]; then
            x_raw=$(cat /sys/bus/iio/devices/$device/in_accel_x_raw)
            y_raw=$(cat /sys/bus/iio/devices/$device/in_accel_y_raw)
            z_raw=$(cat /sys/bus/iio/devices/$device/in_accel_z_raw)
            
            echo "Raw accelerometer data:"
            echo "  X: $x_raw"
            echo "  Y: $y_raw"
            echo "  Z: $z_raw"
            
            # Calculate magnitude
            magnitude=$(echo "scale=2; sqrt($x_raw*$x_raw + $y_raw*$y_raw + $z_raw*$z_raw)" | bc -l)
            echo "  Magnitude: $magnitude"
            
            # Determine dominant axis (gravity direction)
            abs_x=$(echo $x_raw | tr -d '-')
            abs_y=$(echo $y_raw | tr -d '-')
            abs_z=$(echo $z_raw | tr -d '-')
            
            max_val=$abs_x
            dominant_axis="X"
            
            if [ "$abs_y" -gt "$max_val" ]; then
                max_val=$abs_y
                dominant_axis="Y"
            fi
            
            if [ "$abs_z" -gt "$max_val" ]; then
                max_val=$abs_z
                dominant_axis="Z"
            fi
            
            echo "  Dominant axis: $dominant_axis (value: $max_val)"
            
            # Gravity analysis for laptop on table
            echo "  Gravity analysis:"
            if [ "$dominant_axis" = "Z" ]; then
                if [ "$z_raw" -gt 0 ]; then
                    echo "    Z+ dominant: Sensor facing UP (normal laptop lid/base orientation)"
                else
                    echo "    Z- dominant: Sensor facing DOWN (inverted orientation)"
                fi
            elif [ "$dominant_axis" = "Y" ]; then
                if [ "$y_raw" -gt 0 ]; then
                    echo "    Y+ dominant: Sensor tilted toward hinge/away from user"
                else
                    echo "    Y- dominant: Sensor tilted away from hinge/toward user"
                fi
            elif [ "$dominant_axis" = "X" ]; then
                if [ "$x_raw" -gt 0 ]; then
                    echo "    X+ dominant: Sensor tilted to the right"
                else
                    echo "    X- dominant: Sensor tilted to the left"
                fi
            fi
        else
            echo "No accelerometer data available"
        fi
        echo ""
    else
        echo "$device not found"
        echo ""
    fi
}

# Read data from both potential devices
read_accel_data "iio:device0" "First Accelerometer"
read_accel_data "iio:device1" "Second Accelerometer"

# Additional devices (in case there are more)
for dev in /sys/bus/iio/devices/iio:device*; do
    if [ -d "$dev" ]; then
        device_num=$(basename "$dev")
        if [ "$device_num" != "iio:device0" ] && [ "$device_num" != "iio:device1" ]; then
            read_accel_data "$device_num" "Additional Accelerometer"
        fi
    fi
done

echo "=== Analysis Summary ==="
echo "For a laptop in normal position on a table:"
echo "- LID sensor should show Z+ dominant (gravity pointing down through lid)"
echo "- BASE sensor should show Z+ dominant (gravity pointing down through base)"
echo "- Both should have similar Z values but potentially different X/Y values"
echo "- The sensor with more stable readings is likely the BASE (not moving)"
echo "- The sensor that would change more with lid movement is the LID sensor"
echo ""
echo "Current module parameter mapping:"
echo "- lid_bus/lid_addr -> should correspond to the LID accelerometer"
echo "- base_bus/base_addr -> should correspond to the BASE accelerometer"
echo ""
echo "If the mapping seems incorrect, you can test by:"
echo "1. Gently tilting the lid while keeping base stable"
echo "2. Observing which sensor values change"
echo "3. Adjusting module parameters if needed"