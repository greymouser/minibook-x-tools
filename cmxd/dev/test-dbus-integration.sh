#!/bin/bash
# Test script for CMXD DBus integration
# 
# This script tests both the iio-sensor-proxy compatibility interface
# and the custom tablet mode interface.

set -e

# Color output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}=== CMXD DBus Integration Test ===${NC}"

# Check if DBus tools are available
if ! command -v dbus-send &> /dev/null; then
    echo -e "${RED}Error: dbus-send not found. Please install dbus-utils${NC}"
    exit 1
fi

if ! command -v dbus-monitor &> /dev/null; then
    echo -e "${RED}Error: dbus-monitor not found. Please install dbus-utils${NC}"
    exit 1
fi

# Function to test service availability
test_service() {
    local service=$1
    local object_path=$2
    local interface=$3
    
    echo -e "${YELLOW}Testing $service service...${NC}"
    
    # Try system bus first, then session bus
    for bus in "--system" "--session"; do
        echo -e "${BLUE}  Checking $bus bus...${NC}"
        if dbus-send $bus --print-reply --dest="$service" "$object_path" org.freedesktop.DBus.Introspectable.Introspect 2>/dev/null | grep -q "$interface"; then
            echo -e "${GREEN}✓ Service $service is available on $bus bus${NC}"
            return 0
        fi
    done
    
    echo -e "${RED}✗ Service $service is not available on either bus${NC}"
    return 1
}

# Function to get property value
get_property() {
    local service=$1
    local object_path=$2
    local interface=$3
    local property=$4
    
    echo -e "${BLUE}Getting $property from $interface...${NC}"
    
    # Try system bus first, then session bus
    for bus in "--system" "--session"; do
        if dbus-send $bus --print-reply --dest="$service" "$object_path" \
            org.freedesktop.DBus.Properties.Get string:"$interface" string:"$property" 2>/dev/null; then
            return 0
        fi
    done
    
    echo -e "${RED}Failed to get $property from either bus${NC}"
    return 1
}

# Function to monitor signals
monitor_signals() {
    echo -e "${YELLOW}Monitoring DBus signals for 15 seconds...${NC}"
    echo "Try changing tablet orientation or mode to see signals"
    
    # Monitor both system and session buses
    for bus in "system" "session"; do
        echo -e "${BLUE}Monitoring $bus bus for 5 seconds...${NC}"
        timeout 5 dbus-monitor --$bus \
            "type='signal',interface='org.freedesktop.DBus.Properties',member='PropertiesChanged'" \
            "type='signal',interface='com.noonshy.TabletMode1',member='TabletModeChanged'" 2>/dev/null || true
    done
    
    echo -e "${BLUE}Signal monitoring complete${NC}"
}

echo
echo -e "${BLUE}1. Testing iio-sensor-proxy compatibility interface...${NC}"
test_service "net.hadess.SensorProxy" "/net/hadess/SensorProxy" "net.hadess.SensorProxy"

echo
echo -e "${BLUE}2. Testing custom tablet mode interface...${NC}"
test_service "com.noonshy.cmxd" "/com/noonshy/cmxd" "com.noonshy.TabletMode1"

echo
echo -e "${BLUE}3. Getting current properties...${NC}"
get_property "net.hadess.SensorProxy" "/net/hadess/SensorProxy" "net.hadess.SensorProxy" "AccelerometerOrientation"
get_property "net.hadess.SensorProxy" "/net/hadess/SensorProxy" "net.hadess.SensorProxy" "HasAccelerometer"
get_property "com.noonshy.cmxd" "/com/noonshy/cmxd" "com.noonshy.TabletMode1" "TabletMode" 
get_property "com.noonshy.cmxd" "/com/noonshy/cmxd" "com.noonshy.TabletMode1" "DeviceMode"

echo
echo -e "${BLUE}4. Monitoring for live signals...${NC}"
monitor_signals

echo
echo -e "${GREEN}DBus integration test complete!${NC}"
echo "If cmxd is running with DBus support, you should see:"
echo "- Available services responding to introspection"
echo "- Property values for orientation and tablet mode"
echo "- Live signals when changing device orientation/mode"