#!/bin/bash

# CMXD DBus Debug Monitor
# Shows detailed DBus signal structure for debugging

echo "=== CMXD DBus Debug Monitor ==="
echo "This shows raw DBus signals with detailed parsing"
echo "Press Ctrl+C to stop"
echo ""

# Check if services are available
if ! busctl --system list | grep -q "com.noonshy.cmxd"; then
    echo "✗ com.noonshy.cmxd service is NOT available"
    exit 1
fi

echo "✓ Services available, monitoring detailed signals..."
echo ""

# Monitor with detailed output
dbus-monitor --system \
    "type='signal',sender='com.noonshy.cmxd'" \
    "type='signal',sender='net.hadess.SensorProxy'" \
    "type='signal',interface='org.freedesktop.DBus.Properties'" 2>/dev/null | \
{
    signal_context=""
    line_number=0
    
    while read -r line; do
        timestamp=$(date '+%H:%M:%S.%3N')
        ((line_number++))
        
        # Start of new signal
        if echo "$line" | grep -q "^signal"; then
            echo ""
            echo "[$timestamp] === NEW SIGNAL ==="
            echo "  Raw: $line"
            signal_context="new"
            
            # Parse signal header
            if echo "$line" | grep -q "sender=com.noonshy.cmxd"; then
                echo "  Source: CMXD Custom Service"
            elif echo "$line" | grep -q "sender=net.hadess.SensorProxy"; then
                echo "  Source: Sensor Proxy Compatibility"
            fi
            
            if echo "$line" | grep -q "member=PropertiesChanged"; then
                echo "  Type: Properties Changed"
                signal_context="properties"
            elif echo "$line" | grep -q "member=TabletModeChanged"; then
                echo "  Type: Custom TabletMode Signal"
                signal_context="tablet_mode"
            fi
            
        # Parse signal parameters
        elif [ "$signal_context" = "properties" ]; then
            if echo "$line" | grep -q "string.*TabletMode1"; then
                echo "  Interface: com.noonshy.TabletMode1 (Custom)"
            elif echo "$line" | grep -q "string.*SensorProxy"; then
                echo "  Interface: net.hadess.SensorProxy (Compatibility)"
            elif echo "$line" | grep -q "array \["; then
                echo "  → Changed Properties Array:"
            elif echo "$line" | grep -q "dict entry"; then
                echo "    → Property Entry:"
            elif echo "$line" | grep -q "string.*TabletMode\""; then
                echo "      Property: TabletMode"
            elif echo "$line" | grep -q "string.*DeviceMode\""; then
                echo "      Property: DeviceMode"  
            elif echo "$line" | grep -q "string.*AccelerometerOrientation\""; then
                echo "      Property: AccelerometerOrientation"
            elif echo "$line" | grep -q "string.*HasAccelerometer\""; then
                echo "      Property: HasAccelerometer"
            elif echo "$line" | grep -q "variant"; then
                echo "      → Value Variant:"
            elif echo "$line" | grep -q "boolean.*true"; then
                echo "        Value: TRUE (boolean)"
            elif echo "$line" | grep -q "boolean.*false"; then
                echo "        Value: FALSE (boolean)"
            elif echo "$line" | grep -q "string.*\".*\""; then
                value=$(echo "$line" | grep -o '"[^"]*"' | tr -d '"')
                echo "        Value: \"$value\" (string)"
            fi
            
        # Parse direct tablet mode signal
        elif [ "$signal_context" = "tablet_mode" ]; then
            if echo "$line" | grep -q "boolean.*true"; then
                echo "  Direct Value: TABLET MODE (true)"
            elif echo "$line" | grep -q "boolean.*false"; then
                echo "  Direct Value: LAPTOP MODE (false)"
            fi
            
        # Show any other interesting lines
        elif echo "$line" | grep -qE "(string|boolean|variant|array|dict)"; then
            echo "  Data: $line"
        fi
        
        # Add some spacing for readability
        if echo "$line" | grep -q "^$"; then
            echo "  [End of signal]"
        fi
    done
}