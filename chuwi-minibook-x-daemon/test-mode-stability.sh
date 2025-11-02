#!/bin/bash
# Test script for orientation and mode stability system
# Tests both working orientation detection and sequential mode validation

echo "Testing orientation and mode stability system..."
echo "This will start the daemon with verbose logging for 30 seconds"
echo ""
echo "Current system features:"
echo "  ✓ Working orientation detection (from HEAD~1)"
echo "  ✓ Cleanup-on-exit to prevent tablet mode lockout"
echo "  ✓ Sequential mode validation (prevents mode jumping)"
echo ""
echo "Correct mode sequence: closing ↔ laptop ↔ flat ↔ tent ↔ tablet"
echo "  - closing and tablet are endpoints (no wrapping)"
echo "  - Each mode can only transition to adjacent modes"
echo ""
echo "Try different orientations and hinge positions"
echo "Press Enter to start test..."
read

# Start daemon with verbose debugging
echo "Starting daemon with debug logging..."
sudo timeout 30s ./chuwi-minibook-x-daemon -v -d

echo ""
echo "Test completed. Check the log output above for:"
echo "  - 'Mode transition:' for valid sequential mode changes"
echo "  - 'Invalid mode jump blocked:' for prevented erratic jumps"
echo "  - Normal orientation updates"
echo "  - Cleanup messages when daemon exits"