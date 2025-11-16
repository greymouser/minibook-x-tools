#!/bin/bash
# Screen Rotation Script for Hyprland
# 
# This script is called by cmxsd when the device orientation changes.
# It receives the orientation name as a parameter and rotates the screen accordingly.
#
# Usage: rotate-screen.sh <orientation>
#   orientation: portrait, portrait-flipped, landscape, landscape-flipped
#
# Configuration: Set LANDSCAPE_TRANSFORM to match your device's natural landscape orientation
# For DSI-1 on Chuwi Minibook X, the natural landscape is transform 3 (270 degrees)

set -e

# Configuration
LANDSCAPE_TRANSFORM=3  # Transform value for your device's natural landscape orientation
                       # Adjust this based on your device (0-3)

# Get orientation parameter
ORIENTATION="$1"

if [ -z "$ORIENTATION" ]; then
    echo "Error: No orientation specified" >&2
    echo "Usage: $0 <orientation>" >&2
    exit 1
fi

# Get current monitor configuration (first monitor)
MONITOR_INFO=$(hyprctl monitors -j | jq -r '.[0]')

if [ -z "$MONITOR_INFO" ]; then
    echo "Error: Could not get monitor information" >&2
    exit 1
fi

# Extract monitor name and current settings
MONITOR_NAME=$(echo "$MONITOR_INFO" | jq -r '.name')
WIDTH=$(echo "$MONITOR_INFO" | jq -r '.width')
HEIGHT=$(echo "$MONITOR_INFO" | jq -r '.height')
REFRESH=$(echo "$MONITOR_INFO" | jq -r '.refreshRate' | cut -d. -f1)
POS_X=$(echo "$MONITOR_INFO" | jq -r '.x')
POS_Y=$(echo "$MONITOR_INFO" | jq -r '.y')
SCALE=$(echo "$MONITOR_INFO" | jq -r '.scale')

# Map orientation to transform offset
# portrait         -> 90 degrees clockwise from landscape  -> offset +1
# landscape        -> natural landscape                     -> offset  0
# portrait-flipped -> 270 degrees clockwise from landscape -> offset +3 (or -1)
# landscape-flipped-> 180 degrees from landscape           -> offset +2

case "$ORIENTATION" in
    landscape)
        OFFSET=0
        ;;
    portrait)
        OFFSET=3
        ;;
    landscape-flipped)
        OFFSET=2
        ;;
    portrait-flipped)
        OFFSET=1
        ;;
    *)
        echo "Warning: Unknown orientation '$ORIENTATION', defaulting to landscape" >&2
        OFFSET=0
        ;;
esac

# Calculate final transform (wrap around at 4)
TRANSFORM=$(( (LANDSCAPE_TRANSFORM + OFFSET) % 4 ))

# Apply the new monitor configuration
echo "Rotating $MONITOR_NAME to $ORIENTATION (transform $TRANSFORM)"
hyprctl keyword monitor "$MONITOR_NAME,${WIDTH}x${HEIGHT}@${REFRESH},${POS_X}x${POS_Y},$SCALE,transform,$TRANSFORM"

exit 0
