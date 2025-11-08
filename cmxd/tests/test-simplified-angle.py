#!/usr/bin/env python3
"""
Simple test of the simplified hinge angle calculation.
Reads accelerometer data directly and calculates angles using the same 
simplified method as the updated cmxd.
"""

import sys
import time
import math

def read_accel_data(device_path):
    """Read raw accelerometer data from IIO device."""
    try:
        with open(f"{device_path}/in_accel_x_raw", "r") as f:
            x = int(f.read().strip())
        with open(f"{device_path}/in_accel_y_raw", "r") as f:
            y = int(f.read().strip())
        with open(f"{device_path}/in_accel_z_raw", "r") as f:
            z = int(f.read().strip())
        return x, y, z
    except (IOError, ValueError) as e:
        print(f"Error reading accelerometer data: {e}")
        return None, None, None

def calculate_magnitude(x, y, z):
    """Calculate magnitude of 3D vector."""
    return math.sqrt(x*x + y*y + z*z)

def calculate_hinge_angle_360(base_raw, lid_raw, scale=0.009582):
    """Calculate full 0-360° hinge angle using orientation analysis."""
    # Convert raw to m/s²
    base_x = base_raw[0] * scale
    base_y = base_raw[1] * scale
    base_z = base_raw[2] * scale
    
    lid_x = lid_raw[0] * scale
    lid_y = lid_raw[1] * scale
    lid_z = lid_raw[2] * scale
    
    # Calculate magnitudes
    base_mag = calculate_magnitude(base_x, base_y, base_z)
    lid_mag = calculate_magnitude(lid_x, lid_y, lid_z)
    
    if base_mag < 1.0 or lid_mag < 1.0:
        return -1.0
    
    # Dot product for basic 0-180° angle
    dot_product = base_x * lid_x + base_y * lid_y + base_z * lid_z
    cos_angle = dot_product / (base_mag * lid_mag)
    cos_angle = max(-1.0, min(1.0, cos_angle))
    base_angle = math.acos(cos_angle) * 180.0 / math.pi
    
    # Check if we're in "folded back" region (tent/tablet modes)
    # This occurs when both Z components have the same sign
    both_z_same_sign = (base_z > 0 and lid_z > 0) or (base_z < 0 and lid_z < 0)
    
    if both_z_same_sign and base_angle > 90.0:
        # Folded back region - convert to 180-360° range
        angle_360 = 360.0 - base_angle
    else:
        # Normal opening range 0-180°
        angle_360 = base_angle
    
    return angle_360

def calculate_hinge_angle(base_raw, lid_raw, scale=0.009582):
    """Calculate basic 0-180° hinge angle using simplified dot product method."""
    # Convert raw to m/s²
    base_x = base_raw[0] * scale
    base_y = base_raw[1] * scale
    base_z = base_raw[2] * scale
    
    lid_x = lid_raw[0] * scale
    lid_y = lid_raw[1] * scale
    lid_z = lid_raw[2] * scale
    
    # Calculate magnitudes
    base_mag = calculate_magnitude(base_x, base_y, base_z)
    lid_mag = calculate_magnitude(lid_x, lid_y, lid_z)
    
    if base_mag < 1.0 or lid_mag < 1.0:
        return -1.0
    
    # Dot product
    dot_product = base_x * lid_x + base_y * lid_y + base_z * lid_z
    
    # Cosine of angle
    cos_angle = dot_product / (base_mag * lid_mag)
    
    # Clamp to valid range
    cos_angle = max(-1.0, min(1.0, cos_angle))
    
    # Convert to degrees
    angle = math.acos(cos_angle) * 180.0 / math.pi
    
    return angle

def get_device_mode(angle):
    """Determine device mode based on hinge angle (0-360°)."""
    if angle < 0:
        return "invalid"
    elif angle < 60:
        return "closing"
    elif angle < 120:
        return "laptop"
    elif angle < 180:
        return "flat"
    elif angle < 240:
        return "tent"
    else:
        return "tablet"

def main():
    lid_path = "/sys/bus/iio/devices/iio:device0"
    base_path = "/sys/bus/iio/devices/iio:device1"
    
    print("Simplified Hinge Angle Test")
    print("===========================")
    print("Reading accelerometer data and calculating hinge angles...")
    print("Press Ctrl+C to stop")
    print()
    
    try:
        while True:
            # Read raw data
            lid_raw = read_accel_data(lid_path)
            base_raw = read_accel_data(base_path)
            
            if None in lid_raw or None in base_raw:
                print("Failed to read accelerometer data")
                time.sleep(1)
                continue
            
            # Calculate angle
            angle = calculate_hinge_angle(base_raw, lid_raw)
            mode = get_device_mode(angle)
            
            # Convert to m/s² for display
            scale = 0.009582
            lid_ms2 = [x * scale for x in lid_raw]
            base_ms2 = [x * scale for x in base_raw]
            
            print(f"Lid:  [{lid_ms2[0]:6.2f}, {lid_ms2[1]:6.2f}, {lid_ms2[2]:6.2f}] m/s²")
            print(f"Base: [{base_ms2[0]:6.2f}, {base_ms2[1]:6.2f}, {base_ms2[2]:6.2f}] m/s²")
            print(f"Hinge Angle: {angle:6.1f}° -> Mode: {mode}")
            print("-" * 50)
            
            time.sleep(2)
            
    except KeyboardInterrupt:
        print("\nTest stopped.")
        print("\nExpected results:")
        print("- Laptop normal position: ~90-110°, mode=laptop")  
        print("- Flat open (180°): ~180°, mode=flat")
        print("- Nearly closed: ~30-50°, mode=closing")
        print("- Values should be stable and reasonable")

if __name__ == "__main__":
    main()