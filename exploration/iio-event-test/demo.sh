#!/bin/bash

# Demo script showing IIO event-based vs polling comparison

echo "=== IIO Event vs Polling Demonstration ==="
echo ""

if [ "$EUID" -ne 0 ]; then
    echo "This demo requires root privileges."
    echo "Please run: sudo $0"
    exit 1
fi

echo "This demo shows two approaches to reading accelerometer data:"
echo ""
echo "1. EVENT-DRIVEN: Uses IIO triggers and buffers (this program)"
echo "2. POLLING: Reads sysfs files periodically (current daemon approach)"
echo ""

read -p "Press Enter to start 10-second IIO event demo..."

echo ""
echo "=== Running IIO Event Test (10 seconds) ==="
echo "Move the laptop lid to see real-time updates!"
echo ""

timeout 10s ./iio_event_test

echo ""
echo "=== Comparison Summary ==="
echo ""
echo "EVENT-DRIVEN (what you just saw):"
echo "✓ Hardware triggers data collection"
echo "✓ CPU only wakes when data changes"  
echo "✓ Potentially lower latency"
echo "✗ More complex setup"
echo "✗ Driver dependency"
echo ""
echo "POLLING (current daemon):"
echo "✓ Simple and reliable"
echo "✓ Works universally"
echo "✓ Easy to debug"
echo "✗ Periodic CPU wake-ups"
echo ""
echo "For tablet mode detection, polling @ 100ms is actually optimal!"
echo "The complexity of event-driven isn't justified for this use case."