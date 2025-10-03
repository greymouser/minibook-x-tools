#!/bin/bash
# Installation script for chuwi-minibook-x-tablet-mode kernel module
#
# Copyright (c) 2025 Armando DiCianno <armando@noonshy.com>
# Licensed under GPL-2.0

set -e

MODULE_NAME="chuwi-minibook-x-tablet-mode"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo "Installing $MODULE_NAME kernel module..."

# Check if running as root
if [[ $EUID -ne 0 ]]; then
    echo "This script must be run as root (use sudo)"
    exit 1
fi

# Build the module
echo "Building module..."
make -C "$SCRIPT_DIR" all

# Install the module
echo "Installing module..."
make -C "$SCRIPT_DIR" install

# Load the module
echo "Loading module..."
modprobe "$MODULE_NAME"

echo "Module $MODULE_NAME installed and loaded successfully!"
echo "Use 'modinfo $MODULE_NAME' to see module information"
echo "Use 'lsmod | grep $MODULE_NAME' to verify it's loaded"