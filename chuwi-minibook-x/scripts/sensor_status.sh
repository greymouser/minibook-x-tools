#!/bin/bash
# Quick Sensor Status Check
# Provides instant analysis of current sensor configuration and orientation

echo "=== Quick Sensor Status Check ==="
echo "================================="
echo ""

# Check if driver is loaded
if [ ! -d "/sys/devices/platform/chuwi-minibook-x" ]; then
    echo "Error: chuwi-minibook-x driver not loaded"
    exit 1
fi

# Get current module parameters
lid_bus=$(cat /sys/module/chuwi_minibook_x/parameters/lid_bus)
lid_addr=$(cat /sys/module/chuwi_minibook_x/parameters/lid_addr)
base_bus=$(cat /sys/module/chuwi_minibook_x/parameters/base_bus)
base_addr=$(cat /sys/module/chuwi_minibook_x/parameters/base_addr)

echo "Current Module Configuration:"
echo "  lid_bus: $lid_bus, lid_addr: 0x$(printf '%02x' $lid_addr)"
echo "  base_bus: $base_bus, base_addr: 0x$(printf '%02x' $base_addr)"
echo ""

# Analyze current sensor readings
echo "Current Sensor Readings:"
for device in iio:device0 iio:device1; do
    if [ -d "/sys/bus/iio/devices/$device" ]; then
        # Get I2C info
        i2c_path=$(readlink -f /sys/bus/iio/devices/$device)
        i2c_info=$(echo $i2c_path | grep -o 'i2c-[0-9]\+' | head -1)
        i2c_bus=$(echo $i2c_info | sed 's/i2c-//')
        
        # Debug: if still empty, extract from full path
        if [ -z "$i2c_bus" ]; then
            i2c_bus=$(echo $i2c_path | sed -n 's/.*\/i2c-\([0-9]\+\)\/.*/\1/p')
        fi
        
        # Read current values
        x=$(cat /sys/bus/iio/devices/$device/in_accel_x_raw 2>/dev/null || echo "0")
        y=$(cat /sys/bus/iio/devices/$device/in_accel_y_raw 2>/dev/null || echo "0")
        z=$(cat /sys/bus/iio/devices/$device/in_accel_z_raw 2>/dev/null || echo "0")
        
        # Calculate absolute values for dominance analysis
        abs_x=${x#-}
        abs_y=${y#-}
        abs_z=${z#-}
        
        # Determine dominant axis
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
        
        # Determine orientation description
        orientation=""
        case $dominant_axis in
            "X")
                if [ "$x" -gt 0 ]; then
                    orientation="tilted right (toward right hand)"
                else
                    orientation="tilted left (toward left hand)"
                fi
                ;;
            "Y")
                if [ "$y" -gt 0 ]; then
                    orientation="tilted away from user (back/hinge down)"
                else
                    orientation="tilted toward user (front edge down)"
                fi
                ;;
            "Z")
                if [ "$z" -gt 0 ]; then
                    orientation="face up"
                else
                    orientation="face down"
                fi
                ;;
        esac
        
        echo "  $device ($i2c_info):"
        echo "    Raw: X=$x, Y=$y, Z=$z"
        echo "    Dominant: $dominant_axis-axis ($orientation)"
        
        # Determine which module parameter this corresponds to
        mapping=""
        if [ "$i2c_bus" = "$lid_bus" ]; then
            mapping="→ MAPPED AS LID"
        elif [ "$i2c_bus" = "$base_bus" ]; then
            mapping="→ MAPPED AS BASE"
        else
            mapping="→ NOT MAPPED"
        fi
        echo "    Module mapping: $mapping"
        echo ""
    fi
done

# Provide orientation analysis
echo "Orientation Analysis:"
echo "  For laptop flat on table, expect Z-dominant sensors"
echo "  X/Y dominance suggests sensors mounted for 90° rotated display"
echo "  This is normal for your hardware configuration"
echo ""

echo "To verify sensor mapping accuracy:"
echo "  Run: ./sensor_test.sh (interactive test with movement)"
echo "  Or gently tilt the lid and re-run this script to see which sensor changes"
echo ""
echo "Reference: When sitting at laptop normally:"
echo "  • Right side = where your right hand rests"
echo "  • Left side = where your left hand rests"
echo "  • Front = edge closest to you (where palms rest)"
echo "  • Back = hinge side (farthest from you)"
echo ""

# Quick stability check
echo "=== Quick Movement Detection ==="
echo "Gently tilt the laptop slightly and run this script again"
echo "The sensor readings should change, helping identify which is which"