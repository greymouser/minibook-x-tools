#!/bin/bash

# Fixed Quick CMXD DBus Monitor
# Now properly extracts values from DBus variant containers

echo "=== Fixed Quick CMXD DBus Monitor ==="
echo "Properly parsing variant containers for property values"
echo "Try rotating your device to see orientation changes!"
echo "Press Ctrl+C to stop"
echo ""

# Check if services are available
echo "Checking services..."
if busctl --system list | grep -q "com.noonshy.cmxd"; then
    echo "✓ com.noonshy.cmxd service is available"
else
    echo "✗ com.noonshy.cmxd service is NOT available"
    echo "  Make sure cmxd is running: sudo cmxd -v"
    exit 1
fi

if busctl --system list | grep -q "net.hadess.SensorProxy"; then
    echo "✓ net.hadess.SensorProxy service is available"
else
    echo "✗ net.hadess.SensorProxy service is NOT available"
fi

echo ""
echo "Current state (via introspection):"
busctl --system introspect com.noonshy.cmxd /com/noonshy/cmxd | grep -E "(DeviceMode|TabletMode).*property" | while read line; do
    if echo "$line" | grep -q "DeviceMode"; then
        value=$(echo "$line" | grep -o '"[^"]*"' | tr -d '"')
        echo "  DeviceMode: $value"
    elif echo "$line" | grep -q "TabletMode"; then
        value=$(echo "$line" | awk '{print $4}')
        echo "  TabletMode: $value"
    fi
done

echo ""
echo "Monitoring live events (optimized for variant parsing)..."
echo ""

# Monitor with proper variant container parsing
dbus-monitor --system \
    "type='signal',sender='com.noonshy.cmxd'" \
    "type='signal',sender='net.hadess.SensorProxy'" \
    "type='signal',interface='org.freedesktop.DBus.Properties'" 2>/dev/null | \
{
    property_name=""
    
    while read -r line; do
        timestamp=$(date '+%H:%M:%S')
        
        # Extract property names
        if echo "$line" | grep -q 'string "TabletMode"'; then
            property_name="TabletMode"
        elif echo "$line" | grep -q 'string "DeviceMode"'; then
            property_name="DeviceMode"
        elif echo "$line" | grep -q 'string "AccelerometerOrientation"'; then
            property_name="AccelerometerOrientation"
        elif echo "$line" | grep -q 'string "HasAccelerometer"'; then
            property_name="HasAccelerometer"
        fi
        
        # Look for direct TabletModeChanged signal (non-PropertiesChanged)
        if echo "$line" | grep -q "member=TabletModeChanged"; then
            echo "[$timestamp] 🔄 Custom TabletModeChanged signal"
        elif echo "$line" | grep -q "boolean true" && [ "$property_name" = "" ]; then
            # This is likely the TabletModeChanged direct signal
            echo "[$timestamp] 📱 DIRECT TabletModeChanged → TRUE (TABLET)"
        elif echo "$line" | grep -q "boolean false" && [ "$property_name" = "" ]; then
            # This is likely the TabletModeChanged direct signal
            echo "[$timestamp] 💻 DIRECT TabletModeChanged → FALSE (LAPTOP)"
        fi
        
        # Extract values from variant containers
        if echo "$line" | grep -q "variant.*boolean true"; then
            if [ "$property_name" = "TabletMode" ]; then
                echo "[$timestamp] 📱 TabletMode property → TRUE (TABLET)"
            elif [ "$property_name" = "HasAccelerometer" ]; then
                echo "[$timestamp] ⚡ HasAccelerometer → TRUE"
            fi
            property_name=""
        elif echo "$line" | grep -q "variant.*boolean false"; then
            if [ "$property_name" = "TabletMode" ]; then
                echo "[$timestamp] 💻 TabletMode property → FALSE (LAPTOP)"
            elif [ "$property_name" = "HasAccelerometer" ]; then
                echo "[$timestamp] ❌ HasAccelerometer → FALSE"
            fi
            property_name=""
        elif echo "$line" | grep -q "variant.*string"; then
            # Extract string value: variant string "value"
            value=$(echo "$line" | sed -n 's/.*variant.*string[[:space:]]*"\([^"]*\)".*/\1/p')
            
            if [ -n "$value" ]; then
                case "$value" in
                    "normal")
                        echo "[$timestamp] 📐 Orientation → NORMAL (upright)"
                        ;;
                    "left-up")
                        echo "[$timestamp] 📐 Orientation → LEFT-UP (90° CCW)"
                        ;;
                    "right-up") 
                        echo "[$timestamp] 📐 Orientation → RIGHT-UP (90° CW)"
                        ;;
                    "bottom-up")
                        echo "[$timestamp] 📐 Orientation → BOTTOM-UP (upside down)"
                        ;;
                    "laptop")
                        echo "[$timestamp] 🖥️  DeviceMode → LAPTOP (normal use)"
                        ;;
                    "flat")
                        echo "[$timestamp] 📋 DeviceMode → FLAT (180° open)"
                        ;;
                    "tent")
                        echo "[$timestamp] ⛺ DeviceMode → TENT (keyboard folded back)"
                        ;;
                    "tablet")
                        echo "[$timestamp] 📱 DeviceMode → TABLET (fully folded)"
                        ;;
                    *)
                        if [ "$property_name" != "" ]; then
                            echo "[$timestamp] 📊 $property_name → \"$value\""
                        else
                            echo "[$timestamp] 📊 Unknown string value: \"$value\""
                        fi
                        ;;
                esac
                property_name=""
            fi
        fi
    done
}