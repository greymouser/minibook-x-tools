#!/bin/bash
# Chuwi Minibook X Tablet Mode - Installation Script
# Installs the kernel module with optimized bidirectional mode switching

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
MODULE_DIR="$SCRIPT_DIR/module-v1/module"
MODULE_NAME="chuwi-minibook-x-tablet-mode"

echo "=== Chuwi Minibook X Tablet Mode Installation ==="
echo ""

# Check if running as root for module operations
if [[ $EUID -eq 0 ]]; then
    SUDO=""
else
    SUDO="sudo"
fi

# Build the module
echo "Building kernel module..."
cd "$MODULE_DIR"
make clean >/dev/null 2>&1 || true
make

if [ ! -f "${MODULE_NAME}.ko" ]; then
    echo "ERROR: Module build failed"
    exit 1
fi

# Remove existing module if loaded
echo "Removing existing module (if loaded)..."
$SUDO rmmod chuwi_minibook_x_tablet_mode 2>/dev/null || true

# Install the new module
echo "Installing kernel module..."
$SUDO insmod "${MODULE_NAME}.ko"

# Wait for module to initialize
echo "Waiting for module initialization..."
sleep 2

# Check module status
echo ""
echo "=== Module Status ==="
enter_deg=$(cat /sys/kernel/chuwi-minibook-x-tablet-mode/enter_deg)
exit_deg=$(cat /sys/kernel/chuwi-minibook-x-tablet-mode/exit_deg)
angle=$(cat /sys/kernel/chuwi-minibook-x-tablet-mode/angle)
state=$(cat /sys/kernel/chuwi-minibook-x-tablet-mode/state)
force=$(cat /sys/kernel/chuwi-minibook-x-tablet-mode/force)

echo "✓ Module loaded successfully"
echo "✓ Tablet enter threshold: ${enter_deg}°"
echo "✓ Laptop exit threshold: ${exit_deg}°" 
echo "✓ Current hinge angle: ${angle}°"
echo "✓ Current state: ${state} (0=laptop, 1=tablet)"
echo "✓ Force mode: ${force}"
echo ""

# Show recent kernel messages
echo "=== Recent kernel messages ==="
$SUDO dmesg | tail -3 | grep "${MODULE_NAME}" || echo "No recent kernel messages"

echo ""
echo "=== Installation Complete ==="
echo ""
echo "Bidirectional laptop/tablet mode switching is now active:"
echo "• Enters tablet mode when hinge angle > 200°"
echo "• Returns to laptop mode when hinge angle < 170°"
echo "• 30° hysteresis prevents mode oscillation"
echo "• True 360° angle detection without dead zones"
echo ""
echo "Module will auto-calibrate hinge axis from accelerometer data."
echo "No userspace tool updates required - system is ready for use!"