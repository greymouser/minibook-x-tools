#!/bin/bash

# CMXD DBus Event Monitor
# Monitors all DBus signals and property changes from the cmxd daemon
# 
# Usage: ./monitor-cmxd-dbus.sh [options]
# Options:
#   -v, --verbose     Show verbose output including property queries
#   -t, --timeout N   Monitor for N seconds (default: continuous)
#   -h, --help        Show this help

set -euo pipefail

# Configuration
VERBOSE=false
TIMEOUT=""
SHOW_HELP=false

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -v|--verbose)
            VERBOSE=true
            shift
            ;;
        -t|--timeout)
            TIMEOUT="$2"
            shift 2
            ;;
        -h|--help)
            SHOW_HELP=true
            shift
            ;;
        *)
            echo "Unknown option $1"
            SHOW_HELP=true
            shift
            ;;
    esac
done

if [ "$SHOW_HELP" = true ]; then
    echo "CMXD DBus Event Monitor"
    echo ""
    echo "Usage: $0 [options]"
    echo "Options:"
    echo "  -v, --verbose     Show verbose output including property queries"
    echo "  -t, --timeout N   Monitor for N seconds (default: continuous)"
    echo "  -h, --help        Show this help"
    echo ""
    echo "This script monitors all DBus signals from the cmxd daemon including:"
    echo "  - Tablet mode changes"
    echo "  - Orientation changes"
    echo "  - Device mode changes"
    echo "  - Property change signals"
    echo ""
    exit 0
fi

# Function to log with timestamp and color
log() {
    local level="$1"
    local color="$2"
    shift 2
    local timestamp=$(date '+%H:%M:%S.%3N')
    printf "${color}[%s %s]${NC} %s\n" "$timestamp" "$level" "$*"
}

info() { log "INFO" "$BLUE" "$@"; }
warn() { log "WARN" "$YELLOW" "$@"; }
error() { log "ERROR" "$RED" "$@"; }
success() { log "OK" "$GREEN" "$@"; }
event() { log "EVENT" "$CYAN" "$@"; }

# Function to check if services are available
check_services() {
    info "Checking if CMXD services are available..."
    
    # Check com.noonshy.cmxd service
    if busctl --system list | grep -q "com.noonshy.cmxd"; then
        success "Service com.noonshy.cmxd is available"
    else
        error "Service com.noonshy.cmxd is NOT available"
        warn "Make sure cmxd is running with DBus support: sudo cmxd -v"
        return 1
    fi
    
    # Check net.hadess.SensorProxy service
    if busctl --system list | grep -q "net.hadess.SensorProxy"; then
        success "Service net.hadess.SensorProxy is available"
    else
        warn "Service net.hadess.SensorProxy is NOT available"
        warn "Sensor proxy compatibility may not be enabled"
    fi
    
    echo ""
}

# Function to show current state
show_current_state() {
    if [ "$VERBOSE" = true ]; then
        info "Querying current state..."
        
        # Get current tablet mode from custom interface
        echo -n "  Current TabletMode: "
        if result=$(busctl --system get-property com.noonshy.cmxd /com/noonshy/cmxd com.noonshy.TabletMode1 TabletMode 2>/dev/null); then
            if echo "$result" | grep -q "true"; then
                echo -e "${GREEN}TABLET${NC}"
            else
                echo -e "${BLUE}LAPTOP${NC}"
            fi
        else
            echo -e "${RED}ERROR${NC}"
        fi
        
        # Get current device mode
        echo -n "  Current DeviceMode: "
        if result=$(busctl --system get-property com.noonshy.cmxd /com/noonshy/cmxd com.noonshy.TabletMode1 DeviceMode 2>/dev/null); then
            mode=$(echo "$result" | cut -d'"' -f2)
            echo -e "${CYAN}$mode${NC}"
        else
            echo -e "${RED}ERROR${NC}"
        fi
        
        # Get current orientation from sensor proxy
        echo -n "  Current Orientation: "
        if result=$(busctl --system get-property net.hadess.SensorProxy /net/hadess/SensorProxy net.hadess.SensorProxy AccelerometerOrientation 2>/dev/null); then
            orientation=$(echo "$result" | cut -d'"' -f2)
            echo -e "${YELLOW}$orientation${NC}"
        else
            echo -e "${RED}ERROR${NC}"
        fi
        
        # Check if accelerometer is available
        echo -n "  HasAccelerometer: "
        if result=$(busctl --system get-property net.hadess.SensorProxy /net/hadess/SensorProxy net.hadess.SensorProxy HasAccelerometer 2>/dev/null); then
            if echo "$result" | grep -q "true"; then
                echo -e "${GREEN}YES${NC}"
            else
                echo -e "${RED}NO${NC}"
            fi
        else
            echo -e "${RED}ERROR${NC}"
        fi
        
        echo ""
    fi
}

# Function to decode PropertiesChanged signals
decode_properties_changed() {
    local line="$1"
    
    # Extract interface name
    if echo "$line" | grep -q "net.hadess.SensorProxy"; then
        if echo "$line" | grep -q "AccelerometerOrientation"; then
            # Extract orientation value
            if orientation=$(echo "$line" | grep -o '"[^"]*"' | tail -n1 | tr -d '"'); then
                event "Orientation changed to: ${YELLOW}$orientation${NC}"
            fi
        elif echo "$line" | grep -q "HasAccelerometer"; then
            if echo "$line" | grep -q "true"; then
                event "Accelerometer became available"
            else
                event "Accelerometer became unavailable"
            fi
        fi
    elif echo "$line" | grep -q "com.noonshy.TabletMode1"; then
        if echo "$line" | grep -q "TabletMode"; then
            if echo "$line" | grep -q "true"; then
                event "Device switched to: ${GREEN}TABLET MODE${NC}"
            else
                event "Device switched to: ${BLUE}LAPTOP MODE${NC}"
            fi
        elif echo "$line" | grep -q "DeviceMode"; then
            # Extract device mode value - look for any quoted string after DeviceMode
            if mode=$(echo "$line" | sed -n 's/.*DeviceMode.*string[[:space:]]*"\([^"]*\)".*/\1/p'); then
                if [ -n "$mode" ]; then
                    event "Device mode changed to: ${CYAN}$mode${NC}"
                fi
            fi
        fi
    fi
    
    # Also check for raw string values (for debugging)
    if echo "$line" | grep -qE "string.*\"(laptop|flat|tent|tablet|normal|left-up|right-up|bottom-up)\""; then
        value=$(echo "$line" | grep -oE "\"(laptop|flat|tent|tablet|normal|left-up|right-up|bottom-up)\"" | tr -d '"')
        case "$value" in
            "laptop")
                event "Detected DeviceMode: ${CYAN}LAPTOP${NC}"
                ;;
            "flat") 
                event "Detected DeviceMode: ${CYAN}FLAT${NC}"
                ;;
            "tent")
                event "Detected DeviceMode: ${CYAN}TENT${NC}"
                ;;
            "tablet")
                event "Detected DeviceMode: ${CYAN}TABLET${NC}"
                ;;
            "normal")
                event "Detected Orientation: ${YELLOW}NORMAL${NC}"
                ;;
            "left-up")
                event "Detected Orientation: ${YELLOW}LEFT-UP${NC}"
                ;;
            "right-up")
                event "Detected Orientation: ${YELLOW}RIGHT-UP${NC}"
                ;;
            "bottom-up")
                event "Detected Orientation: ${YELLOW}BOTTOM-UP${NC}"
                ;;
        esac
    fi
}

# Function to monitor DBus signals
monitor_signals() {
    info "Starting DBus signal monitoring..."
    info "Try rotating your device or changing tablet/laptop mode to see events!"
    echo ""
    
    local timeout_args=""
    if [ -n "$TIMEOUT" ]; then
        timeout_args="timeout $TIMEOUT"
        info "Monitoring for $TIMEOUT seconds..."
    else
        info "Monitoring continuously (Ctrl+C to stop)..."
    fi
    
    echo ""
    
    # Monitor signals with optional timeout
    $timeout_args dbus-monitor --system \
        "type='signal',sender='com.noonshy.cmxd'" \
        "type='signal',sender='net.hadess.SensorProxy'" \
        "type='signal',interface='org.freedesktop.DBus.Properties'" 2>/dev/null | \
    while read -r line; do
        # Filter for our signals
        if echo "$line" | grep -q "signal.*sender="; then
            # Check if it's from our services
            if echo "$line" | grep -q "sender=com.noonshy.cmxd\|sender=net.hadess.SensorProxy"; then
                event "Raw signal: $line"
            fi
        elif echo "$line" | grep -q "member=PropertiesChanged"; then
            event "Properties are changing..."
        elif echo "$line" | grep -q "member=TabletModeChanged"; then
            event "Custom tablet mode signal detected"
        elif echo "$line" | grep -q "string\|boolean\|variant"; then
            # Try to decode property values
            decode_properties_changed "$line"
        fi
    done
}

# Function to start background property monitoring
start_property_monitor() {
    if [ "$VERBOSE" = true ]; then
        info "Starting periodic property monitoring in background..."
        
        # Monitor properties every 2 seconds in background
        (
            while true; do
                sleep 2
                show_current_state > /dev/null 2>&1
            done
        ) &
        
        # Save background job PID
        echo $! > /tmp/cmxd-monitor-bg.pid
    fi
}

# Function to cleanup background processes
cleanup() {
    info "Cleaning up..."
    
    # Kill background property monitor if it exists
    if [ -f /tmp/cmxd-monitor-bg.pid ]; then
        if kill "$(cat /tmp/cmxd-monitor-bg.pid)" 2>/dev/null; then
            info "Stopped background property monitor"
        fi
        rm -f /tmp/cmxd-monitor-bg.pid
    fi
    
    echo ""
    info "Monitor stopped"
    exit 0
}

# Set up signal handlers
trap cleanup SIGINT SIGTERM

# Main execution
echo -e "${CYAN}"
echo "╔══════════════════════════════════════════════════════════════╗"
echo "║                    CMXD DBus Event Monitor                   ║"
echo "║                                                              ║"
echo "║  This script monitors all DBus events from the cmxd daemon  ║"
echo "║  Try rotating your device to see orientation changes!       ║"
echo "╚══════════════════════════════════════════════════════════════╝"
echo -e "${NC}"
echo ""

# Check if services are available
if ! check_services; then
    error "Required services not available. Exiting."
    exit 1
fi

# Show current state
show_current_state

# Start background monitoring if verbose
# start_property_monitor

# Start signal monitoring (this blocks)
monitor_signals

# Cleanup (though this shouldn't be reached due to the blocking monitor)
cleanup