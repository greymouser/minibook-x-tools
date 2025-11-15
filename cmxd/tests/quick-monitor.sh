#!/bin/bash

# Quick CMXD DBus Monitor
# Simple script to monitor key DBus events from cmxd daemon

echo "=== Quick CMXD DBus Monitor ==="
echo "Monitoring key events from cmxd daemon..."
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
echo "Current state:"

# Show current tablet mode
echo -n "  TabletMode: "
if result=$(busctl --system get-property com.noonshy.cmxd /com/noonshy/cmxd com.noonshy.TabletMode1 TabletMode 2>/dev/null); then
    if echo "$result" | grep -q "true"; then
        echo "TABLET"
    else
        echo "LAPTOP"
    fi
else
    echo "ERROR"
fi

# Show current orientation
echo -n "  Orientation: "
if result=$(busctl --system get-property net.hadess.SensorProxy /net/hadess/SensorProxy net.hadess.SensorProxy AccelerometerOrientation 2>/dev/null); then
    orientation=$(echo "$result" | cut -d'"' -f2)
    echo "$orientation"
else
    echo "ERROR"
fi

echo ""
echo "Monitoring live events (timestamps will show)..."
echo ""

# Enhanced signal monitoring with property value extraction
dbus-monitor --system \
    "type='signal',sender='com.noonshy.cmxd'" \
    "type='signal',sender='net.hadess.SensorProxy'" \
    "type='signal',interface='org.freedesktop.DBus.Properties'" 2>/dev/null | \
{
    interface_context=""
    property_name=""
    
    while read -r line; do
        timestamp=$(date '+%H:%M:%S')
        
        # Detect which interface is changing
        if echo "$line" | grep -q "interface=org.freedesktop.DBus.Properties"; then
            if echo "$line" | grep -q "com.noonshy.cmxd"; then
                interface_context="cmxd"
            elif echo "$line" | grep -q "net.hadess.SensorProxy"; then
                interface_context="sensor_proxy"
            fi
        fi
        
        # Extract interface name from PropertiesChanged signal
        if echo "$line" | grep -q "string.*TabletMode1"; then
            interface_context="tablet_mode"
        elif echo "$line" | grep -q "string.*SensorProxy"; then
            interface_context="sensor_proxy"
        fi
        
        # Extract property names
        if echo "$line" | grep -q "string.*TabletMode\""; then
            property_name="TabletMode"
        elif echo "$line" | grep -q "string.*DeviceMode\""; then
            property_name="DeviceMode"
        elif echo "$line" | grep -q "string.*AccelerometerOrientation\""; then
            property_name="AccelerometerOrientation"
        elif echo "$line" | grep -q "string.*HasAccelerometer\""; then
            property_name="HasAccelerometer"
        fi
        
        # Look for custom tablet mode signal (direct signal, not PropertiesChanged)
        if echo "$line" | grep -q "member=TabletModeChanged"; then
            echo "[$timestamp] � Direct TabletModeChanged signal"
        fi
        
        # Extract and display property values
        if echo "$line" | grep -q "boolean.*true"; then
            if [ "$property_name" = "TabletMode" ]; then
                echo "[$timestamp] 📱 TabletMode → TRUE (TABLET)"
                property_name=""
            elif [ "$property_name" = "HasAccelerometer" ]; then
                echo "[$timestamp] ⚡ HasAccelerometer → TRUE"
                property_name=""
            else
                echo "[$timestamp] ✅ Boolean property → TRUE"
            fi
        elif echo "$line" | grep -q "boolean.*false"; then
            if [ "$property_name" = "TabletMode" ]; then
                echo "[$timestamp] 💻 TabletMode → FALSE (LAPTOP)"
                property_name=""
            elif [ "$property_name" = "HasAccelerometer" ]; then
                echo "[$timestamp] ❌ HasAccelerometer → FALSE"
                property_name=""
            else
                echo "[$timestamp] ❌ Boolean property → FALSE"
            fi
        fi
        
        # Extract string values for orientations and device modes
        if echo "$line" | grep -q "string.*\".*\""; then
            value=$(echo "$line" | grep -o '"[^"]*"' | tail -1 | tr -d '"')
            
            case "$value" in
                "normal")
                    echo "[$timestamp] 📐 Orientation → NORMAL"
                    ;;
                "left-up")
                    echo "[$timestamp] 📐 Orientation → LEFT-UP (90° CCW)"
                    ;;
                "right-up") 
                    echo "[$timestamp] 📐 Orientation → RIGHT-UP (90° CW)"
                    ;;
                "bottom-up")
                    echo "[$timestamp] 📐 Orientation → BOTTOM-UP (180°)"
                    ;;
                "laptop")
                    echo "[$timestamp] 🖥️  DeviceMode → LAPTOP"
                    ;;
                "flat")
                    echo "[$timestamp] 📋 DeviceMode → FLAT"
                    ;;
                "tent")
                    echo "[$timestamp] ⛺ DeviceMode → TENT"
                    ;;
                "tablet")
                    echo "[$timestamp] 📱 DeviceMode → TABLET"
                    ;;
                *)
                    if [ "$property_name" != "" ] && [ "$value" != "" ]; then
                        echo "[$timestamp] � $property_name → \"$value\""
                        property_name=""
                    fi
                    ;;
            esac
        fi
        
        # Generic PropertiesChanged detection (as fallback)
        if echo "$line" | grep -q "member=PropertiesChanged" && [ "$interface_context" != "" ]; then
            case "$interface_context" in
                "tablet_mode")
                    echo "[$timestamp] 📊 TabletMode interface properties changing..."
                    ;;
                "sensor_proxy")
                    echo "[$timestamp] � SensorProxy interface properties changing..."
                    ;;
                *)
                    echo "[$timestamp] 📊 Properties changing on $interface_context..."
                    ;;
            esac
        fi
    done
}