#!/bin/bash

# Test script to verify kernel module sysfs integration

set -e

KERNEL_MODULE="./chuwi-minibook-x/chuwi-minibook-x.ko"
DAEMON_BINARY="./chuwi-minibook-x-daemon/chuwi-minibook-x-daemon"
SYSFS_BASE="/sys/kernel/chuwi-minibook-x"

echo "=== Testing Kernel Module and Daemon Integration ==="

# Check if module and daemon exist
if [ ! -f "$KERNEL_MODULE" ]; then
    echo "ERROR: Kernel module not found: $KERNEL_MODULE"
    exit 1
fi

if [ ! -x "$DAEMON_BINARY" ]; then
    echo "ERROR: Daemon binary not found or not executable: $DAEMON_BINARY"
    exit 1
fi

echo "✓ Both kernel module and daemon binaries exist"

# Test if we can load the module (as a dry run test)
echo "Testing kernel module info..."
modinfo "$KERNEL_MODULE" | head -10

echo ""
echo "=== Module Parameters ==="
modinfo "$KERNEL_MODULE" | grep "^parm:"

echo ""
echo "=== Module Dependencies ==="
modinfo "$KERNEL_MODULE" | grep "^depends:"

echo ""
echo "=== Testing Daemon Help ==="
$DAEMON_BINARY --help || true

echo ""
echo "=== Module Build Test Complete ==="
echo "To test with actual hardware:"
echo "1. Load module: sudo insmod $KERNEL_MODULE"
echo "2. Check sysfs: ls -la $SYSFS_BASE/"
echo "3. Run daemon: sudo $DAEMON_BINARY -v"
echo "4. Unload module: sudo rmmod chuwi-minibook-x"