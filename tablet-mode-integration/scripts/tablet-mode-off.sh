#!/bin/bash
# 
# Hyprland tablet mode deactivation script
# This script is executed when the device exits tablet mode
#
# Default actions for Hyprland desktop mode restoration

set -euo pipefail

# Log tablet mode deactivation
logger "Tablet mode deactivated"

# Restore Hyprland desktop settings
# Restore gaps for desktop experience
hyprctl keyword general:gaps_in 4 2>/dev/null || true
hyprctl keyword general:gaps_out 8 2>/dev/null || true

# Restore window decorations
hyprctl keyword decoration:rounding 8 2>/dev/null || true
hyprctl keyword decoration:drop_shadow true 2>/dev/null || true

# Disable natural scrolling for traditional mouse/trackpad use
hyprctl keyword input:touchpad:natural_scroll false 2>/dev/null || true
hyprctl keyword input:touchscreen:natural_scroll false 2>/dev/null || true

# Clean up window rules for virtual keyboards
hyprctl keyword windowrulev2 'unset,class:^(onboard|squeekboard|wvkbd)$' 2>/dev/null || true

# Close virtual keyboards
pkill onboard 2>/dev/null || true
pkill squeekboard 2>/dev/null || true
pkill wvkbd-mobintl 2>/dev/null || true

# Optional: Return to default workspace if needed
# hyprctl dispatch workspace 1

# Optional: Send notification
if command -v notify-send >/dev/null 2>&1; then
    notify-send "Laptop Mode" "Device switched to laptop mode" --icon=input-keyboard 2>/dev/null || true
fi

logger "Laptop mode script completed"