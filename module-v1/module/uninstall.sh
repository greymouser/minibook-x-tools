#!/bin/bash
# Uninstallation script for chuwi-minibook-x-tablet-mode kernel module
#
# Copyright (c) 2025 Armando DiCianno <armando@noonshy.com>
# Licensed under GPL-2.0

set -e

MODULE_NAME="chuwi-minibook-x-tablet-mode"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo "Uninstalling $MODULE_NAME kernel module..."

# Check if running as root
if [[ $EUID -ne 0 ]]; then
    echo "This script must be run as root (use sudo)"
    exit 1
fi

# Unload the module if it's loaded
if lsmod | grep -q "^$MODULE_NAME "; then
    echo "Unloading module..."
    rmmod "$MODULE_NAME" || echo "Warning: Could not unload module (may not be loaded)"
fi

# Uninstall the module
echo "Uninstalling module..."
make -C "$SCRIPT_DIR" uninstall

echo "Module $MODULE_NAME uninstalled successfully!"