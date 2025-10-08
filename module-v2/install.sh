#!/bin/bash
# Install script for Chuwi Minibook X module
# This demonstrates automatic dependency loading with modprobe

set -e

MODULE_PATH="$(dirname "$0")/chuwi-minibook-x.ko"
MODULE_NAME="chuwi_minibook_x"

echo "Chuwi Minibook X Module Installer"
echo "================================="

# Check if module file exists
if [[ ! -f "$MODULE_PATH" ]]; then
    echo "Error: Module file not found: $MODULE_PATH"
    echo "Please run 'make' first to build the module."
    exit 1
fi

# Show module dependencies
echo "Module dependencies:"
modinfo "$MODULE_PATH" | grep softdep || echo "  No soft dependencies found"
echo

# Install module to temporary location for modprobe
TEMP_DIR="/tmp/chuwi_minibook_x_module"
mkdir -p "$TEMP_DIR"
cp "$MODULE_PATH" "$TEMP_DIR/"

# Add to module search path and install
echo "Installing module with dependency resolution..."
if command -v modprobe >/dev/null 2>&1; then
    # Unload if already loaded
    if lsmod | grep -q "$MODULE_NAME"; then
        echo "Unloading existing module..."
        sudo rmmod "$MODULE_NAME" || true
    fi
    
    # Try to use modprobe with our module
    # Note: For full modprobe support, module needs to be in /lib/modules/
    echo "Loading module with insmod (dependencies must be loaded manually)..."
    
    # Load dependencies manually since we're not in the module path
    echo "Loading MXC4005 driver..."
    sudo modprobe mxc4005 2>/dev/null || echo "  Warning: mxc4005 module not found or already loaded"
    
    echo "Loading serial multi-instantiate driver..."  
    sudo modprobe serial_multi_instantiate 2>/dev/null || echo "  Warning: serial_multi_instantiate module not found or already loaded"
    
    echo "Loading main module..."
    sudo insmod "$MODULE_PATH"
    
    echo "✓ Module loaded successfully!"
    
    # Show loaded modules
    echo
    echo "Currently loaded related modules:"
    lsmod | grep -E "(chuwi|mxc4005|serial_multi)" || echo "  No related modules found"
    
else
    echo "Error: modprobe not found. Please install module-init-tools or kmod."
    exit 1
fi

echo
echo "Module installation complete!"
echo "Check dmesg for module messages: sudo dmesg | tail -20"

# Cleanup
rm -rf "$TEMP_DIR"