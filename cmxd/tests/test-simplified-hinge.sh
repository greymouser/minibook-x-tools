#!/bin/bash
# Test simplified cmxd hinge angle calculation

echo "=== Testing Simplified CMXD Hinge Angle Calculation ==="
echo

# Check if cmxd is running
if pgrep -x "cmxd" > /dev/null; then
    echo "Stopping existing cmxd..."
    sudo killall cmxd
    sleep 1
fi

# Start cmxd with verbose output in background
echo "Starting cmxd with verbose logging..."
cd /home/greymouser/Projects/noonshy/repos/minibook-x-tools/cmxd
sudo ./cmxd -v &
CMXD_PID=$!

# Give it time to start
sleep 2

echo "Monitoring hinge angle for 30 seconds..."
echo "Try different laptop positions: normal, flat, tent, tablet"
echo "Watch for mode changes and angle values."
echo

# Monitor the log output
timeout 30 tail -f /var/log/syslog | grep -E "(cmxd|hinge|mode|angle)" || true

echo
echo "Stopping cmxd..."
sudo kill $CMXD_PID 2>/dev/null || true
sudo killall cmxd 2>/dev/null || true

echo "Test complete!"
echo
echo "Expected behavior:"
echo "- Angles should be reasonable (0-180° for most positions)"
echo "- Mode transitions should be smooth and stable"
echo "- No complex gravity correction messages"