#!/bin/bash

# Test current CMXD property values
echo "=== Testing CMXD Property Values ==="
echo ""

echo "Testing service availability:"
busctl --system list | grep -E "(com.noonshy|net.hadess.SensorProxy)"
echo ""

echo "Testing property access methods:"

# Method 1: Using busctl get-property
echo "Method 1 - busctl get-property:"
echo -n "  TabletMode: "
busctl --system get-property com.noonshy.cmxd /com/noonshy/cmxd com.noonshy.TabletMode1 TabletMode 2>/dev/null || echo "FAILED"

echo -n "  DeviceMode: "  
busctl --system get-property com.noonshy.cmxd /com/noonshy/cmxd com.noonshy.TabletMode1 DeviceMode 2>/dev/null || echo "FAILED"

echo -n "  AccelerometerOrientation: "
busctl --system get-property net.hadess.SensorProxy /net/hadess/SensorProxy net.hadess.SensorProxy AccelerometerOrientation 2>/dev/null || echo "FAILED"

echo ""

# Method 2: Using Properties interface directly
echo "Method 2 - Properties interface:"
echo -n "  TabletMode: "
busctl --system call com.noonshy.cmxd /com/noonshy/cmxd org.freedesktop.DBus.Properties Get ss "com.noonshy.TabletMode1" "TabletMode" 2>/dev/null || echo "FAILED"

echo -n "  DeviceMode: "
busctl --system call com.noonshy.cmxd /com/noonshy/cmxd org.freedesktop.DBus.Properties Get ss "com.noonshy.TabletMode1" "DeviceMode" 2>/dev/null || echo "FAILED"

echo ""

# Method 3: Check object paths and interfaces
echo "Method 3 - Introspection:"
echo "Available object paths:"
busctl --system tree com.noonshy.cmxd

echo ""
echo "Available interfaces on /com/noonshy/cmxd:"
busctl --system introspect com.noonshy.cmxd /com/noonshy/cmxd | grep -E "(interface|property)"

echo ""
echo "=== Raw DBus monitoring for 15 seconds ==="
echo "Try moving your device now..."

timeout 15 dbus-monitor --system \
    "type='signal',sender='com.noonshy.cmxd'" \
    "type='signal',sender='net.hadess.SensorProxy'" 2>/dev/null | \
grep -E "(signal|string|boolean)" | head -20

echo ""
echo "Test complete!"