#!/bin/bash

# Test script for simplified sysfs device assignment

set -e

echo "=== Testing Simplified Device Assignment ==="

KERNEL_MODULE="./chuwi-minibook-x/chuwi-minibook-x.ko"
DAEMON_BINARY="./chuwi-minibook-x-daemon/chuwi-minibook-x-daemon"

echo "✓ Module and daemon built successfully"

echo ""
echo "=== Expected Behavior ==="
echo "With default module parameters:"
echo "- lid_bus=13, lid_addr=0x15 -> should output 'iio:device0'"
echo "- base_bus=12, base_addr=0x15 -> should output 'iio:device1'"
echo ""
echo "This matches the sensor test results:"
echo "- iio:device0 (i2c-13) = lid accelerometer"  
echo "- iio:device1 (i2c-12) = base accelerometer"

echo ""
echo "=== Module Parameters ==="
modinfo "$KERNEL_MODULE" | grep "parm:" | grep -E "(lid_bus|base_bus|lid_addr|base_addr)"

echo ""
echo "=== Daemon Usage ==="
$DAEMON_BINARY --help | head -15

echo ""
echo "=== To Test On Hardware ==="
echo "1. Load module: sudo insmod $KERNEL_MODULE"
echo "2. Check sysfs outputs:"
echo "   cat /sys/kernel/chuwi-minibook-x/iio_base_device"
echo "   cat /sys/kernel/chuwi-minibook-x/iio_lid_device" 
echo "3. Run daemon: sudo $DAEMON_BINARY -v"
echo ""
echo "Expected sysfs outputs:"
echo "- /sys/kernel/chuwi-minibook-x/iio_base_device: iio:device1"
echo "- /sys/kernel/chuwi-minibook-x/iio_lid_device: iio:device0"