#!/bin/bash
# Test script for tablet mode transitions

set -e

MODULE_PATH="/home/greymouser/Projects/noonshy/repos/minibook-x-tools/cmx/cmx.ko"
SYSFS_MODE="/sys/devices/platform/cmx/mode"

echo "=== Tablet Mode Transition Test ==="
echo

# Remove old module if loaded
echo "Removing old module..."
sudo rmmod cmx 2>/dev/null || true
sleep 1

# Load new module
echo "Loading module..."
sudo insmod "$MODULE_PATH"
sleep 2

# Check if sysfs entry exists
if [ ! -f "$SYSFS_MODE" ]; then
    echo "ERROR: Sysfs mode entry not found at $SYSFS_MODE"
    exit 1
fi

# Show initial state
echo "Initial mode:"
cat "$SYSFS_MODE"
echo

# Test transition to tablet mode
echo "Switching to tablet mode..."
echo "tablet" | sudo tee "$SYSFS_MODE" > /dev/null
echo "Current mode: $(cat $SYSFS_MODE)"
echo

# Wait 10 seconds
echo "Waiting 10 seconds..."
sleep 10

# Switch back to laptop mode
echo "Switching back to laptop mode..."
echo "laptop" | sudo tee "$SYSFS_MODE" > /dev/null
echo "Current mode: $(cat $SYSFS_MODE)"
echo

echo "=== Test Complete ==="
echo "Check dmesg for SW_TABLET_MODE events:"
sudo dmesg | tail -20 | grep -i "tablet\|chuwi"
