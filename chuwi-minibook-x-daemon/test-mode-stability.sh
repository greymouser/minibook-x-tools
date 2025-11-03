#!/bin/bash

# Test script for Chuwi MiniBook X mode and orientation stability
# 
# ENHANCED MODE STABILITY SYSTEM
# ==============================
# Current implementation provides:
# - Configurable mode boundaries (45°, 135°, 225°, 315°) and hysteresis (10°)
# - Sequential mode validation (only adjacent mode transitions allowed)
# - Spurious data filtering (requires 3 consistent readings before mode change)
# - Tablet mode reading protection (prevents orientation changes when tilted > 45°)
# 
# Mode boundaries with hysteresis:
# - Closing: 0°-45° (exit at 55°)
# - Laptop: 45°-135° (enter at 35°, exit at 145°)
# - Flat: 135°-225° (enter at 125°, exit at 235°)
# - Tent: 225°-315° (enter at 215°, exit at 325°)
# - Tablet: 315°-360° (enter at 305°)
# 
# Spurious data protection:
# - Requires 3 consecutive readings of new mode before switching
# - Prevents single spurious sensor readings from causing mode changes
# - Maintains responsiveness for legitimate mode changes
# 
# Reading scenarios protected in tablet mode:
# - Holding tablet above you in bed (>45° upward tilt)
# - Laying tablet flat on table and tilting up (>45° tilt)
# - Any tablet mode position beyond 45° tilt angle

echo "=== Chuwi MiniBook X Enhanced Mode Stability Test ==="
echo "Testing spurious data filtering and tablet reading protection"
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
echo "- Mode detection with stability filtering (closing/laptop/flat/tent/tablet)"
echo "- Mode candidate tracking (requires 3 consistent readings)"
echo "- Orientation changes (landscape/portrait/inverted/etc)"
echo "- Tablet mode reading protection messages"
echo "- Hysteresis and boundary management"
echo
echo "Test scenarios:"
echo "1. Normal laptop mode - should detect orientation changes normally"
echo "2. Mode transitions - should require 3 consistent readings"
echo "3. Tablet mode flat - should detect orientation changes normally"
echo "4. Tablet mode tilted >45° - should maintain last orientation (PROTECTED)"
echo "5. Spurious sensor spikes - should be filtered out by stability requirement"
echo
echo "Press Ctrl+C to stop the test"
echo

# Run the daemon with debug output
exec ./chuwi-minibook-x-daemon