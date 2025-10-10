#!/bin/bash
# Helper script to display module parameters in a more readable format

echo "Chuwi Minibook X Module Parameters:"
echo "=================================="

if [ ! -d "/sys/module/chuwi_minibook_x/parameters" ]; then
    echo "Module not loaded or parameters not available"
    exit 1
fi

for param in lid_bus base_bus; do
    value=$(cat /sys/module/chuwi_minibook_x/parameters/$param 2>/dev/null)
    if [ -n "$value" ]; then
        echo "$param: $value"
    fi
done

for param in lid_addr base_addr; do
    value=$(cat /sys/module/chuwi_minibook_x/parameters/$param 2>/dev/null)
    if [ -n "$value" ]; then
        hex_value=$(printf "0x%02x" "$value")
        echo "$param: $value (hex: $hex_value)"
    fi
done

debug_mode=$(cat /sys/module/chuwi_minibook_x/parameters/debug_mode 2>/dev/null)
if [ -n "$debug_mode" ]; then
    if [ "$debug_mode" = "Y" ]; then
        echo "debug_mode: enabled"
    else
        echo "debug_mode: disabled"
    fi
fi

echo ""
echo "Current Configuration (from kernel messages):"
echo "============================================="
sudo dmesg | grep "chuwi-minibook-x" | grep -E "(Lid accelerometer|Base accelerometer)" | tail -2