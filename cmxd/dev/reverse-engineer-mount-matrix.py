#!/usr/bin/env python3
"""
Reverse-engineer mount matrices from accelerometer data.

This script analyzes gravity vectors from different orientations to determine
what the correct mount matrices should be for the lid and base sensors.
"""

import sys
import numpy as np
import re
from collections import defaultdict

class MountMatrixAnalyzer:
    def __init__(self, log_file):
        self.log_file = log_file
        self.scale_factor = 0.009582  # Convert from raw to m/s²
        self.gravity_nominal = 9.81   # Expected gravity magnitude
        
        self.positions = {}
        self.device_info = {}
        
    def parse_log_file(self):
        """Parse the log file and extract position data."""
        print("Parsing log file...")
        
        current_position = None
        current_data = []
        
        with open(self.log_file, 'r') as f:
            for line in f:
                line = line.strip()
                
                # Extract device info
                if "device mount_matrix:" in line:
                    if "Lid device" in line or "lid/0" in line:
                        matrix_str = line.split("mount_matrix: ")[1]
                        self.device_info['lid'] = self._parse_mount_matrix(matrix_str)
                    elif "Base device" in line or "base/1" in line:
                        matrix_str = line.split("mount_matrix: ")[1]
                        self.device_info['base'] = self._parse_mount_matrix(matrix_str)
                
                # Extract scale factor
                if "device scale:" in line:
                    scale_match = re.search(r'scale: ([0-9.]+)', line)
                    if scale_match:
                        self.scale_factor = float(scale_match.group(1))
                
                # Position headers (with timestamp prefix)
                if "=== POSITION:" in line:
                    if current_position and current_data:
                        self.positions[current_position] = current_data
                    # Extract position name from line like: [2025-11-07 21:45:40] === POSITION: laptop_normal_135deg ===
                    if "POSITION:" in line and "===" in line:
                        position_part = line.split("POSITION: ")[1].split(" ===")[0]
                        current_position = position_part
                        current_data = []
                
                # Data lines (timestamp,lid_x,lid_y,lid_z,base_x,base_y,base_z)
                # Handle lines with timestamp prefix like: [2025-11-07 21:45:40] 2025-11-07 21:45:40.544,-842,-4,348,20,-3,-1333
                elif current_position and '] 2025-' in line:
                    # Extract data part after the "] " 
                    data_part = line.split('] ', 1)[1]
                    parts = data_part.split(',')
                    if len(parts) == 7:
                        try:
                            timestamp = parts[0]
                            lid_raw = [int(parts[1]), int(parts[2]), int(parts[3])]
                            base_raw = [int(parts[4]), int(parts[5]), int(parts[6])]
                            
                            # Convert to m/s²
                            lid_ms2 = [x * self.scale_factor for x in lid_raw]
                            base_ms2 = [x * self.scale_factor for x in base_raw]
                            
                            current_data.append({
                                'timestamp': timestamp,
                                'lid_raw': lid_raw,
                                'base_raw': base_raw,
                                'lid_ms2': lid_ms2,
                                'base_ms2': base_ms2
                            })
                        except ValueError:
                            continue
        
        # Don't forget the last position
        if current_position and current_data:
            self.positions[current_position] = current_data
            
        print(f"Parsed {len(self.positions)} positions with data")
        
    def _parse_mount_matrix(self, matrix_str):
        """Parse mount matrix string into 3x3 numpy array."""
        # Replace semicolons with commas and split
        elements = re.findall(r'-?[0-9.]+', matrix_str)
        if len(elements) == 9:
            matrix = np.array([float(x) for x in elements]).reshape(3, 3)
            return matrix
        return None
    
    def calculate_position_stats(self):
        """Calculate average gravity vectors for each position."""
        print("\nCalculating position statistics...")
        
        stats = {}
        for position, data_points in self.positions.items():
            if not data_points:
                continue
                
            lid_vectors = np.array([dp['lid_ms2'] for dp in data_points])
            base_vectors = np.array([dp['base_ms2'] for dp in data_points])
            
            lid_mean = np.mean(lid_vectors, axis=0)
            base_mean = np.mean(base_vectors, axis=0)
            
            lid_magnitude = np.linalg.norm(lid_mean)
            base_magnitude = np.linalg.norm(base_mean)
            
            stats[position] = {
                'lid': {
                    'mean': lid_mean,
                    'magnitude': lid_magnitude,
                    'normalized': lid_mean / lid_magnitude if lid_magnitude > 0 else lid_mean
                },
                'base': {
                    'mean': base_mean,
                    'magnitude': base_magnitude,
                    'normalized': base_mean / base_magnitude if base_magnitude > 0 else base_mean
                },
                'sample_count': len(data_points)
            }
            
        return stats
    
    def analyze_key_positions(self, stats):
        """Analyze key positions to determine sensor orientations."""
        print("\n" + "="*80)
        print("MOUNT MATRIX REVERSE ENGINEERING ANALYSIS")
        print("="*80)
        
        print(f"\nCurrent mount matrices from log:")
        if 'lid' in self.device_info and self.device_info['lid'] is not None:
            print("Lid:")
            print(self.device_info['lid'])
        if 'base' in self.device_info and self.device_info['base'] is not None:
            print("Base:")
            print(self.device_info['base'])
        
        # Analyze flat positions - these should give us the Z-axis alignment
        print(f"\n1. FLAT POSITIONS ANALYSIS (Z-axis alignment)")
        print("-" * 50)
        
        flat_positions = ['flat_screen_up', 'flat_screen_down']
        for pos in flat_positions:
            if pos in stats:
                lid = stats[pos]['lid']
                base = stats[pos]['base']
                
                print(f"\n{pos}:")
                print(f"  Lid:  [{lid['mean'][0]:6.2f}, {lid['mean'][1]:6.2f}, {lid['mean'][2]:6.2f}] | mag={lid['magnitude']:5.2f}")
                print(f"  Base: [{base['mean'][0]:6.2f}, {base['mean'][1]:6.2f}, {base['mean'][2]:6.2f}] | mag={base['magnitude']:5.2f}")
                
                # In flat positions, Z should dominate and both sensors should agree on direction
                lid_z_dominant = abs(lid['mean'][2]) > max(abs(lid['mean'][0]), abs(lid['mean'][1]))
                base_z_dominant = abs(base['mean'][2]) > max(abs(base['mean'][0]), abs(base['mean'][1]))
                
                print(f"  Lid Z-dominant: {lid_z_dominant}, Base Z-dominant: {base_z_dominant}")
                
                if 'screen_up' in pos:
                    expected_z = self.gravity_nominal
                    print(f"  Expected: +Z gravity (~+{expected_z:.1f})")
                else:
                    expected_z = -self.gravity_nominal
                    print(f"  Expected: -Z gravity (~{expected_z:.1f})")
                    
                lid_z_correct = abs(lid['mean'][2] - expected_z) < abs(lid['mean'][2] + expected_z)
                base_z_correct = abs(base['mean'][2] - expected_z) < abs(base['mean'][2] + expected_z)
                print(f"  Z-direction correct: Lid={lid_z_correct}, Base={base_z_correct}")
        
        # Analyze laptop positions for X/Y alignment  
        print(f"\n2. LAPTOP POSITIONS ANALYSIS (X/Y alignment)")
        print("-" * 50)
        
        if 'laptop_normal_135deg' in stats:
            pos = 'laptop_normal_135deg'
            lid = stats[pos]['lid']
            base = stats[pos]['base']
            
            print(f"\n{pos}:")
            print(f"  Lid:  [{lid['mean'][0]:6.2f}, {lid['mean'][1]:6.2f}, {lid['mean'][2]:6.2f}] | mag={lid['magnitude']:5.2f}")
            print(f"  Base: [{base['mean'][0]:6.2f}, {base['mean'][1]:6.2f}, {base['mean'][2]:6.2f}] | mag={base['magnitude']:5.2f}")
            
            # In normal laptop position:
            # - Lid should be tilted back (negative X component likely)
            # - Base should be flat-ish (Z component should dominate)
            lid_dominant_axis = np.argmax(np.abs(lid['mean']))
            base_dominant_axis = np.argmax(np.abs(base['mean']))
            axis_names = ['X', 'Y', 'Z']
            
            print(f"  Lid dominant axis: {axis_names[lid_dominant_axis]} ({lid['mean'][lid_dominant_axis]:+.2f})")
            print(f"  Base dominant axis: {axis_names[base_dominant_axis]} ({base['mean'][base_dominant_axis]:+.2f})")
        
        # Analyze side positions for Y-axis
        print(f"\n3. SIDE POSITIONS ANALYSIS (Y-axis alignment)")
        print("-" * 50)
        
        side_positions = ['laptop_left_side_135deg', 'laptop_right_side_135deg']
        for pos in side_positions:
            if pos in stats:
                lid = stats[pos]['lid']
                base = stats[pos]['base']
                
                print(f"\n{pos}:")
                print(f"  Lid:  [{lid['mean'][0]:6.2f}, {lid['mean'][1]:6.2f}, {lid['mean'][2]:6.2f}] | mag={lid['magnitude']:5.2f}")
                print(f"  Base: [{base['mean'][0]:6.2f}, {base['mean'][1]:6.2f}, {base['mean'][2]:6.2f}] | mag={base['magnitude']:5.2f}")
                
                # When on side, Y should dominate for both sensors
                # Direction should be opposite for left vs right
                lid_dominant_axis = np.argmax(np.abs(lid['mean']))
                base_dominant_axis = np.argmax(np.abs(base['mean']))
                axis_names = ['X', 'Y', 'Z']
                
                print(f"  Lid dominant axis: {axis_names[lid_dominant_axis]} ({lid['mean'][lid_dominant_axis]:+.2f})")
                print(f"  Base dominant axis: {axis_names[base_dominant_axis]} ({base['mean'][base_dominant_axis]:+.2f})")
    
    def determine_mount_matrices(self, stats):
        """Attempt to determine correct mount matrices from the data."""
        print(f"\n4. MOUNT MATRIX DETERMINATION")
        print("-" * 50)
        
        # We'll use flat positions to determine the basic orientation
        if 'flat_screen_up' not in stats or 'flat_screen_down' not in stats:
            print("ERROR: Need flat_screen_up and flat_screen_down positions for analysis")
            return
        
        screen_up = stats['flat_screen_up']
        screen_down = stats['flat_screen_down']
        
        print("\nAnalyzing raw sensor behavior in flat positions...")
        
        # Expected: In flat positions, both sensors should read gravity in Z
        # If screen up: +Z gravity, if screen down: -Z gravity
        
        for sensor in ['lid', 'base']:
            print(f"\n{sensor.upper()} SENSOR:")
            
            up_vector = screen_up[sensor]['mean']
            down_vector = screen_down[sensor]['mean']
            
            print(f"  Screen up:   [{up_vector[0]:7.2f}, {up_vector[1]:7.2f}, {up_vector[2]:7.2f}]")
            print(f"  Screen down: [{down_vector[0]:7.2f}, {down_vector[1]:7.2f}, {down_vector[2]:7.2f}]")
            
            # Determine which axis shows the gravity flip
            axis_names = ['X', 'Y', 'Z']
            max_flip = 0
            gravity_axis = -1
            
            for i in range(3):
                flip_magnitude = abs(up_vector[i] - down_vector[i])
                if flip_magnitude > max_flip:
                    max_flip = flip_magnitude
                    gravity_axis = i
            
            if gravity_axis >= 0:
                print(f"  Gravity axis appears to be: {axis_names[gravity_axis]} (flip magnitude: {max_flip:.2f})")
                
                # Check if the gravity direction is correct
                expected_up = self.gravity_nominal
                expected_down = -self.gravity_nominal
                
                actual_up = up_vector[gravity_axis]
                actual_down = down_vector[gravity_axis]
                
                print(f"  Expected up/down: {expected_up:+.1f}/{expected_down:+.1f}")
                print(f"  Actual up/down: {actual_up:+.2f}/{actual_down:+.2f}")
                
                # Determine if axis is inverted
                up_correct = (actual_up > 0 and expected_up > 0)
                down_correct = (actual_down < 0 and expected_down < 0)
                
                if up_correct and down_correct:
                    print(f"  ✓ {axis_names[gravity_axis]} axis orientation is CORRECT")
                elif not up_correct and not down_correct:
                    print(f"  ✗ {axis_names[gravity_axis]} axis orientation is INVERTED")
                else:
                    print(f"  ? {axis_names[gravity_axis]} axis orientation is UNCLEAR")
                    
                # Suggest mount matrix correction
                if gravity_axis != 2:  # Not Z
                    print(f"  → Mount matrix needed: map sensor {axis_names[gravity_axis]} to device Z")
                    if gravity_axis == 0:  # X -> Z
                        if up_correct:
                            suggested_matrix = [[0, 0, 1], [0, 1, 0], [-1, 0, 0]]  # X->Z, Y->Y, Z->-X
                        else:
                            suggested_matrix = [[0, 0, -1], [0, 1, 0], [1, 0, 0]]   # X->-Z, Y->Y, Z->X
                    elif gravity_axis == 1:  # Y -> Z
                        if up_correct:
                            suggested_matrix = [[1, 0, 0], [0, 0, 1], [0, -1, 0]]   # X->X, Y->Z, Z->-Y
                        else:
                            suggested_matrix = [[1, 0, 0], [0, 0, -1], [0, 1, 0]]   # X->X, Y->-Z, Z->Y
                    
                    print(f"  Suggested mount matrix:")
                    for row in suggested_matrix:
                        print(f"    {row}")
                elif not (up_correct and down_correct):  # Z but inverted
                    print(f"  → Mount matrix needed: invert Z axis")
                    suggested_matrix = [[1, 0, 0], [0, 1, 0], [0, 0, -1]]
                    print(f"  Suggested mount matrix:")
                    for row in suggested_matrix:
                        print(f"    {row}")
                else:
                    print(f"  → No mount matrix correction needed for this axis")

    def run_analysis(self):
        """Run the complete analysis."""
        self.parse_log_file()
        stats = self.calculate_position_stats()
        self.analyze_key_positions(stats)
        self.determine_mount_matrices(stats)
        
        print(f"\n" + "="*80)
        print("ANALYSIS COMPLETE")
        print("="*80)

def main():
    if len(sys.argv) != 2:
        print("Usage: python3 reverse-engineer-mount-matrix.py <log_file>")
        sys.exit(1)
    
    log_file = sys.argv[1]
    analyzer = MountMatrixAnalyzer(log_file)
    analyzer.run_analysis()

if __name__ == "__main__":
    main()