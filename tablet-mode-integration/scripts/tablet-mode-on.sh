#!/bin/bash
# 
# Hyprland tablet mode activation script
# This script is executed when the device enters tablet mode
#
# Default actions for Hyprland tablet mode optimization

set -euo pipefail

# Log tablet mode activation
logger "Tablet mode activated"

# Hyprland optimizations for tablet mode
# Remove gaps for more screen real estate
hyprctl keyword general:gaps_in 0 2>/dev/null || true
hyprctl keyword general:gaps_out 0 2>/dev/null || true

# Remove window decorations for cleaner look
hyprctl keyword decoration:rounding 0 2>/dev/null || true
hyprctl keyword decoration:drop_shadow false 2>/dev/null || true

# Enable natural scrolling for touch
hyprctl keyword input:touchpad:natural_scroll true 2>/dev/null || true
hyprctl keyword input:touchscreen:natural_scroll true 2>/dev/null || true

# Set up window rules for virtual keyboards
hyprctl keyword windowrulev2 'float,class:^(onboard|squeekboard|wvkbd)$' 2>/dev/null || true
hyprctl keyword windowrulev2 'size 100% 40%,class:^(onboard|squeekboard|wvkbd)$' 2>/dev/null || true
hyprctl keyword windowrulev2 'move 0 60%,class:^(onboard|squeekboard|wvkbd)$' 2>/dev/null || true
hyprctl keyword windowrulev2 'pin,class:^(onboard|squeekboard|wvkbd)$' 2>/dev/null || true

# Start virtual keyboard (choose one that works best for you)
# Option 1: Onboard (GNOME's virtual keyboard)
if command -v onboard >/dev/null 2>&1; then
    pkill onboard 2>/dev/null || true  # Kill any existing instance
    onboard &
    logger "Started onboard virtual keyboard"
fi

# Option 2: Squeekboard (designed for mobile/touch devices)
# if command -v squeekboard >/dev/null 2>&1; then
#     pkill squeekboard 2>/dev/null || true
#     squeekboard &
#     logger "Started squeekboard virtual keyboard"
# fi

# Option 3: wvkbd (lightweight wayland virtual keyboard)
# if command -v wvkbd-mobintl >/dev/null 2>&1; then
#     pkill wvkbd-mobintl 2>/dev/null || true
#     wvkbd-mobintl &
#     logger "Started wvkbd virtual keyboard"
# fi

# Optional: Send notification
if command -v notify-send >/dev/null 2>&1; then
    notify-send "Tablet Mode" "Device switched to tablet mode" --icon=input-tablet 2>/dev/null || true
fi

logger "Tablet mode script completed"