#!/bin/bash

# Test script for Chuwi MiniBook X mode and orientation stability
# 
# TABLET MODE READING PROTECTION
# ==============================
# Current implementation provides targeted protection for reading scenarios:
# - In tablet mode only: prevents orientation changes when tilted > 45°
# - Maintains last known orientation during protected periods
# - Normal orientation detection for all other modes and angles
# 
# Reading scenarios protected:
# - Holding tablet above you in bed (>45° upward tilt)
# - Laying tablet flat on table and tilting up (>45° tilt)
# - Any tablet mode position beyond 45° tilt angle

echo "=== Chuwi MiniBook X Mode Stability Test ==="
echo "Testing tablet mode reading protection (>45° tilt threshold)"
echo

if [ ! -f "./chuwi-minibook-x-daemon" ]; then
    echo "Error: chuwi-minibook-x-daemon binary not found"
    echo "Run 'make' first to build the daemon"
    exit 1
fi

# Check if we can read kernel module interface
SYSFS_PATH="/sys/module/chuwi_minibook_x"
if [ ! -d "$SYSFS_PATH" ]; then
    echo "Warning: Kernel module not loaded or not found at $SYSFS_PATH"
    echo "Some tests may not work properly"
fi

echo "Starting daemon in test mode..."
echo "Monitor the output for:"
echo "- Mode detection (closing/laptop/flat/tent/tablet)"
echo "- Orientation changes (landscape/portrait/inverted/etc)"
echo "- Tablet mode reading protection messages"
echo "- Tilt angle calculations"
echo
echo "Test scenarios:"
echo "1. Normal laptop mode - should detect orientation changes normally"
echo "2. Tablet mode flat - should detect orientation changes normally"
echo "3. Tablet mode tilted >45° - should maintain last orientation (PROTECTED)"
echo
echo "Press Ctrl+C to stop the test"
echo

# Run the daemon with debug output
exec ./chuwi-minibook-x-daemon