#!/usr/bin/env python3
"""
Analysis script to identify "UU" devices from I2C scan results.

Based on I2C scan results:
- i2c-13: UU at 0x5D
- i2c-14: UU at 0x2C, device at 0x33

"UU" means the address is in use by a kernel driver.
"""

def analyze_i2c_devices():
    print("I2C Device Analysis for CHUWI MiniBook X")
    print("=" * 50)
    
    # Known I2C scan results
    scan_results = {
        13: {
            0x5D: "UU"  # Device in use by kernel driver
        },
        14: {
            0x2C: "UU",  # Device in use by kernel driver  
            0x33: "33"   # Device detected but no driver bound
        }
    }
    
    print("\nI2C Scan Results:")
    for bus, devices in scan_results.items():
        print(f"\nBus i2c-{bus}:")
        for addr, status in devices.items():
            print(f"  0x{addr:02X}: {status}")
    
    print("\nDevice Analysis:")
    print("-" * 30)
    
    # Analyze Bus 13
    print("\n🔍 I2C Bus 13 - Address 0x5D (UU)")
    print("Likely candidates:")
    print("• MXC4005 Accelerometer (common address)")
    print("• Motion sensor/accelerometer")
    print("• Orientation sensor")
    print("• Known usage: Lid accelerometer in convertible laptops")
    
    # Analyze Bus 14
    print("\n🔍 I2C Bus 14 - Address 0x2C (UU)")
    print("Likely candidates:")
    print("• Touchpad controller (ELAN, Synaptics)")
    print("• I2C HID device (PNP0C50)")
    print("• Precision touchpad")
    print("• Common touchpad I2C address")
    
    print("\n🔍 I2C Bus 14 - Address 0x33 (33)")
    print("Likely candidates:")
    print("• MXC4005 Accelerometer (alternative address)")
    print("• Base accelerometer in convertible laptop")
    print("• Secondary motion sensor")
    print("• Currently no driver bound (shows as '33' not 'UU')")
    
    print("\nACPI Device Mapping:")
    print("-" * 30)
    print("Based on DSDT analysis:")
    print("• Multiple PNP0C50 HID devices (I2C HID Protocol)")
    print("• TPD0/TPL1 touchpad devices in ACPI")
    print("• I2C device address assignment via ACPI methods")
    print("• Dynamic address assignment using I2CM() method")
    
    print("\nConclusions:")
    print("-" * 30)
    print("🎯 i2c-13:0x5D → MXC4005 Accelerometer (Lid sensor)")
    print("   - Already bound to mxc4005 driver")
    print("   - Higher bus number = lid sensor (per our driver logic)")
    
    print("🎯 i2c-14:0x2C → Touchpad Controller")
    print("   - Likely ELAN or Synaptics I2C touchpad")
    print("   - Bound to i2c-hid or specific touchpad driver")
    
    print("🎯 i2c-14:0x33 → MXC4005 Accelerometer (Base sensor)")
    print("   - Detected but no driver currently bound")
    print("   - Lower bus number = base sensor (per our driver logic)")
    print("   - Available for our driver to instantiate")
    
    print("\nRecommended Actions:")
    print("-" * 30)
    print("1. Verify accelerometer assignment:")
    print("   • i2c-13:0x5D should be lid accelerometer")
    print("   • i2c-14:0x33 should be base accelerometer")
    
    print("2. Check existing drivers:")
    print("   • lsmod | grep mxc4005")
    print("   • ls /sys/bus/i2c/devices/")
    
    print("3. Test device detection:")
    print("   • i2cget -y 13 0x5D 0x01  # Should return device ID")
    print("   • i2cget -y 14 0x33 0x01  # Should return device ID")

if __name__ == "__main__":
    analyze_i2c_devices()