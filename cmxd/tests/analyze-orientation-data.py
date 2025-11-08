#!/usr/bin/env python3

"""
Post-processing analysis script for Chuwi Minibook X accelerometer data.

This script analyzes the collected orientation data to answer:
1. Does the biggest gravity dimension make sense for each sensor?
2. Is device0 definitely the lid, and device1 definitely the base?
3. Do their mount matrices make sense?
4. Are scale factors useful in calculations?
"""

import sys
import re
import numpy as np
import matplotlib.pyplot as plt
from collections import defaultdict
from datetime import datetime
import argparse

class AccelDataAnalyzer:
    def __init__(self, log_file):
        self.log_file = log_file
        self.device_info = {}
        self.position_data = {}
        self.mount_matrices = {}
        self.scales = {}
        self.scale_available = {}
        
    def parse_log_file(self):
        """Parse the log file and extract device info and position data."""
        print("Parsing log file...")
        
        current_position = None
        with open(self.log_file, 'r') as f:
            for line in f:
                line = line.strip()
                if not line:
                    continue
                
                # Remove timestamp prefix if present
                if line.startswith('[') and '] ' in line:
                    line = line.split('] ', 1)[1]
                    
                # Extract device information - updated to handle lid/base format
                if 'mount_matrix:' in line:
                    # Handle both old format "Device0 mount_matrix:" and new format "Lid device mount_matrix:"
                    device_match = re.search(r'(?:Device(\d+)|(\w+) device) mount_matrix: (.+)', line)
                    if device_match:
                        if device_match.group(1):
                            # Old format: Device0, Device1
                            device = int(device_match.group(1))
                        else:
                            # New format: Lid device, Base device
                            device_type = device_match.group(2).lower()
                            device = device_type  # Store as 'lid' or 'base'
                        
                        matrix_str = device_match.group(3)
                        # Parse mount matrix - handle comma and semicolon format
                        # Convert "1, 0, 0; 0, 1, 0; 0, 0, 1" to space-separated
                        matrix_str = matrix_str.replace(',', '').replace(';', '')
                        matrix_values = [float(x) for x in matrix_str.split()]
                        if len(matrix_values) == 9:
                            self.mount_matrices[device] = np.array(matrix_values).reshape(3, 3)
                
                elif 'scale:' in line and 'scale_available' not in line:
                    # Handle both old and new formats
                    device_match = re.search(r'(?:Device(\d+)|(\w+) device) scale: (.+)', line)
                    if device_match:
                        if device_match.group(1):
                            # Old format: Device0, Device1
                            device = int(device_match.group(1))
                        else:
                            # New format: Lid device, Base device
                            device_type = device_match.group(2).lower()
                            device = device_type  # Store as 'lid' or 'base'
                        
                        self.scales[device] = float(device_match.group(3))
                
                elif 'scale_available:' in line:
                    # Handle both old and new formats
                    device_match = re.search(r'(?:Device(\d+)|(\w+) device) scale_available: (.+)', line)
                    if device_match:
                        if device_match.group(1):
                            # Old format: Device0, Device1
                            device = int(device_match.group(1))
                        else:
                            # New format: Lid device, Base device
                            device_type = device_match.group(2).lower()
                            device = device_type  # Store as 'lid' or 'base'
                        
                        scales = [float(x) for x in device_match.group(3).split()]
                        self.scale_available[device] = scales
                
                # Extract position header
                elif line.startswith('=== POSITION:'):
                    position_match = re.search(r'=== POSITION: (.+) ===', line)
                    if position_match:
                        current_position = position_match.group(1)
                        self.position_data[current_position] = {
                            'lid': [],
                            'base': [],
                            'timestamps': []
                        }
                
                # Extract sensor data - updated to handle new format
                elif current_position and ',' in line and not line.startswith('#') and not line.startswith('Description:') and not line.startswith('Timestamp:') and not line.startswith('Duration:') and not line.startswith('Sample_rate:'):
                    # Handle both formats:
                    # New: timestamp,lid_x,lid_y,lid_z,base_x,base_y,base_z
                    # Old: timestamp,device0_x,device0_y,device0_z,device1_x,device1_y,device1_z
                    parts = line.split(',')
                    if len(parts) == 7:
                        try:
                            timestamp = parts[0]
                            first_device_data = [int(parts[1]), int(parts[2]), int(parts[3])]
                            second_device_data = [int(parts[4]), int(parts[5]), int(parts[6])]
                            
                            self.position_data[current_position]['timestamps'].append(timestamp)
                            
                            # Store data in both old and new formats for compatibility
                            # Assume new format by default, but maintain backwards compatibility
                            if 'lid' not in self.position_data[current_position]:
                                # Old format - initialize with device0/device1
                                self.position_data[current_position]['device0'] = []
                                self.position_data[current_position]['device1'] = []
                                
                            if 'device0' in self.position_data[current_position]:
                                # Old format
                                self.position_data[current_position]['device0'].append(first_device_data)
                                self.position_data[current_position]['device1'].append(second_device_data)
                            else:
                                # New format
                                self.position_data[current_position]['lid'].append(first_device_data)
                                self.position_data[current_position]['base'].append(second_device_data)
                                
                        except ValueError:
                            continue
        
        print(f"Parsed {len(self.position_data)} positions")
        print(f"Mount matrices found for devices: {list(self.mount_matrices.keys())}")
        print(f"Scales found for devices: {list(self.scales.keys())}")
        
    def apply_mount_matrix(self, raw_data, device):
        """Apply mount matrix transformation to raw accelerometer data."""
        if device not in self.mount_matrices:
            return raw_data
        
        matrix = self.mount_matrices[device]
        transformed = []
        
        for sample in raw_data:
            # Convert to numpy array and apply transformation
            raw_vector = np.array(sample)
            transformed_vector = matrix @ raw_vector
            transformed.append(transformed_vector.tolist())
        
        return transformed
    
    def apply_scale(self, data, device):
        """Apply scale factor to accelerometer data."""
        if device not in self.scales:
            return data
        
        scale = self.scales[device]
        return [[x * scale for x in sample] for sample in data]
    
    def calculate_gravity_stats(self):
        """Calculate gravity-related statistics for each device and position."""
        print("\nCalculating gravity statistics...")
        
        stats = {}
        
        for position, data in self.position_data.items():
            stats[position] = {}
            
            # Handle both old format (device0, device1) and new format (lid, base)
            if 'device0' in data and 'device1' in data:
                # Old format
                device_keys = [('device0', 0), ('device1', 1)]
            else:
                # New format
                device_keys = [('lid', 'lid'), ('base', 'base')]
            
            for device_key, device_id in device_keys:
                if device_key not in data:
                    continue
                    
                raw_data = np.array(data[device_key])
                
                if len(raw_data) == 0:
                    continue
                
                # Apply mount matrix transformation
                transformed_data = self.apply_mount_matrix(raw_data, device_id)
                transformed_data = np.array(transformed_data)
                
                # Apply scale factor
                scaled_data = self.apply_scale(transformed_data, device_id)
                scaled_data = np.array(scaled_data)
                
                # Calculate statistics
                mean_vals = np.mean(scaled_data, axis=0)
                std_vals = np.std(scaled_data, axis=0)
                magnitude = np.sqrt(np.sum(mean_vals**2))
                
                # Find dominant axis (largest absolute value)
                dominant_axis = np.argmax(np.abs(mean_vals))
                dominant_value = mean_vals[dominant_axis]
                
                stats[position][device_id] = {
                    'raw_mean': np.mean(raw_data, axis=0),
                    'raw_std': np.std(raw_data, axis=0),
                    'transformed_mean': mean_vals,
                    'transformed_std': std_vals,
                    'magnitude': magnitude,
                    'dominant_axis': dominant_axis,  # 0=X, 1=Y, 2=Z
                    'dominant_value': dominant_value
                }
        
        return stats
    
    def analyze_sensor_identification(self, stats):
        """Analyze which device is lid vs base based on gravity patterns."""
        print("\n=== SENSOR IDENTIFICATION ANALYSIS ===")
        
        # Expected behavior:
        # - In laptop_normal_135deg: base should show Z pointing up, lid should show complex orientation
        # - In flat positions: both should show Z pointing up/down
        # - In tent/tablet modes: orientations should differ predictably
        
        device_patterns = {0: [], 1: []}
        
        key_positions = ['laptop_normal_135deg', 'flat_screen_up', 'flat_screen_down', 'tent_mode']
        
        for position in key_positions:
            if position not in stats:
                continue
                
            print(f"\nPosition: {position}")
            for device in [0, 1]:
                if device not in stats[position]:
                    continue
                    
                data = stats[position][device]
                mean = data['transformed_mean']
                magnitude = data['magnitude']
                dominant_axis = data['dominant_axis']
                
                axis_names = ['X', 'Y', 'Z']
                print(f"  Device{device}: dominant={axis_names[dominant_axis]} "
                      f"({mean[dominant_axis]:.1f}), magnitude={magnitude:.1f}")
                
                device_patterns[device].append({
                    'position': position,
                    'dominant_axis': dominant_axis,
                    'dominant_value': data['dominant_value'],
                    'magnitude': magnitude
                })
        
        # Analysis logic
        print(f"\n=== IDENTIFICATION CONCLUSION ===")
        
        # In normal laptop position, base should be more stable (closer to pure gravity)
        normal_pos_stats = stats.get('laptop_normal_135deg', {})
        if 0 in normal_pos_stats and 1 in normal_pos_stats:
            dev0_mag = normal_pos_stats[0]['magnitude']
            dev1_mag = normal_pos_stats[1]['magnitude']
            
            print(f"In normal laptop position:")
            print(f"  Device0 gravity magnitude: {dev0_mag:.1f}")
            print(f"  Device1 gravity magnitude: {dev1_mag:.1f}")
            
            # The base should be closer to 1g in normal position
            if abs(dev0_mag - 1.0) < abs(dev1_mag - 1.0):
                print(f"  → Device0 appears to be the BASE (closer to 1g)")
                print(f"  → Device1 appears to be the LID")
            else:
                print(f"  → Device1 appears to be the BASE (closer to 1g)")
                print(f"  → Device0 appears to be the LID")
        
        return device_patterns
    
    def analyze_mount_matrices(self, stats):
        """Analyze if the current mount matrices make sense."""
        print("\n=== MOUNT MATRIX ANALYSIS ===")
        
        for device in [0, 1]:
            if device in self.mount_matrices:
                matrix = self.mount_matrices[device]
                print(f"\nDevice{device} mount matrix:")
                print(matrix)
                
                # Check if it's a rotation matrix (determinant should be ±1)
                det = np.linalg.det(matrix)
                print(f"Determinant: {det:.3f} (should be ±1 for pure rotation)")
                
                # Check orthogonality
                identity_check = matrix @ matrix.T
                is_orthogonal = np.allclose(identity_check, np.eye(3), atol=1e-10)
                print(f"Orthogonal: {is_orthogonal}")
                
                # Identify the transformation
                if np.allclose(matrix, np.array([[0, 1, 0], [-1, 0, 0], [0, 0, 1]])):
                    print("This is a 90° counter-clockwise rotation (CCW)")
                elif np.allclose(matrix, np.array([[0, -1, 0], [1, 0, 0], [0, 0, 1]])):
                    print("This is a 90° clockwise rotation (CW)")
                elif np.allclose(matrix, np.eye(3)):
                    print("This is no rotation (identity matrix)")
                else:
                    print("This is a custom rotation matrix")
    
    def analyze_scale_factors(self):
        """Analyze scale factors and their utility."""
        print("\n=== SCALE FACTOR ANALYSIS ===")
        
        for device in [0, 1]:
            if device in self.scales:
                scale = self.scales[device]
                available = self.scale_available.get(device, [])
                
                print(f"\nDevice{device}:")
                print(f"  Current scale: {scale}")
                print(f"  Available scales: {available}")
                
                # Check if scale converts raw values to m/s²
                # Standard gravity is ~9.81 m/s²
                print(f"  Purpose: Convert raw ADC values to m/s²")
                print(f"  Standard gravity (1g) = 9.81 m/s²")
    
    def create_visualizations(self, stats):
        """Create visualizations of the data."""
        print("\nCreating visualizations...")
        
        # Set up the plotting style
        plt.style.use('default')
        
        # 1. Gravity magnitude comparison across positions
        fig, ((ax1, ax2), (ax3, ax4)) = plt.subplots(2, 2, figsize=(15, 12))
        
        positions = list(stats.keys())
        device0_magnitudes = []
        device1_magnitudes = []
        
        # Handle both old and new data formats
        for pos in positions:
            # Check if we have old format (0, 1) or new format ('lid', 'base')
            if 0 in stats[pos] and 1 in stats[pos]:
                # Old format
                device0_magnitudes.append(stats[pos][0]['magnitude'])
                device1_magnitudes.append(stats[pos][1]['magnitude'])
            elif 'lid' in stats[pos] and 'base' in stats[pos]:
                # New format - map to old visualization labels for consistency
                device0_magnitudes.append(stats[pos]['lid']['magnitude'])
                device1_magnitudes.append(stats[pos]['base']['magnitude'])
            else:
                device0_magnitudes.append(0)
                device1_magnitudes.append(0)
        
        x = range(len(positions))
        ax1.bar([i - 0.2 for i in x], device0_magnitudes, 0.4, label='Lid', alpha=0.7)
        ax1.bar([i + 0.2 for i in x], device1_magnitudes, 0.4, label='Base', alpha=0.7)
        ax1.set_xlabel('Position')
        ax1.set_ylabel('Gravity Magnitude')
        ax1.set_title('Gravity Magnitude by Position')
        ax1.set_xticks(x)
        ax1.set_xticklabels(positions, rotation=45, ha='right')
        ax1.legend()
        ax1.grid(True, alpha=0.3)
        ax1.axhline(y=1.0, color='r', linestyle='--', label='1g reference')
        
        # 2. Dominant axis distribution
        device0_axes = []
        device1_axes = []
        
        for pos in positions:
            if 0 in stats[pos] and 1 in stats[pos]:
                # Old format
                device0_axes.append(stats[pos][0]['dominant_axis'])
                device1_axes.append(stats[pos][1]['dominant_axis'])
            elif 'lid' in stats[pos] and 'base' in stats[pos]:
                # New format
                device0_axes.append(stats[pos]['lid']['dominant_axis'])
                device1_axes.append(stats[pos]['base']['dominant_axis'])
            else:
                device0_axes.append(-1)
                device1_axes.append(-1)
        
        axis_names = ['X', 'Y', 'Z']
        ax2.scatter(x, device0_axes, label='Lid', alpha=0.7, s=50)
        ax2.scatter(x, device1_axes, label='Base', alpha=0.7, s=50)
        ax2.set_xlabel('Position')
        ax2.set_ylabel('Dominant Axis')
        ax2.set_title('Dominant Gravity Axis by Position')
        ax2.set_xticks(x)
        ax2.set_xticklabels(positions, rotation=45, ha='right')
        ax2.set_yticks([0, 1, 2])
        ax2.set_yticklabels(axis_names)
        ax2.legend()
        ax2.grid(True, alpha=0.3)
        
        # 3. Raw vs transformed comparison for a specific position
        if 'laptop_normal_135deg' in stats:
            pos_data = stats['laptop_normal_135deg']
            
            # Handle both old and new formats
            if 0 in pos_data and 1 in pos_data:
                # Old format
                devices_info = [(0, 'Device0'), (1, 'Device1')]
            elif 'lid' in pos_data and 'base' in pos_data:
                # New format  
                devices_info = [('lid', 'Lid'), ('base', 'Base')]
            else:
                devices_info = []
            
            for i, (device_key, device_name) in enumerate(devices_info):
                if device_key in pos_data:
                    data = pos_data[device_key]
                    
                    # Check if raw_mean and transformed_mean exist
                    if 'raw_mean' in data and 'transformed_mean' in data:
                        raw = data['raw_mean']
                        transformed = data['transformed_mean']
                    else:
                        # Fall back to using mean values if available
                        raw = data.get('mean', [0, 0, 0])
                        transformed = data.get('mean', [0, 0, 0])
                    
                    ax = ax3 if i == 0 else ax4
                    
                    x_vals = ['X', 'Y', 'Z']
                    x_pos = range(len(x_vals))
                    
                    ax.bar([j - 0.2 for j in x_pos], raw, 0.4, label='Raw', alpha=0.7)
                    ax.bar([j + 0.2 for j in x_pos], transformed, 0.4, label='Transformed', alpha=0.7)
                    ax.set_xlabel('Axis')
                    ax.set_ylabel('Acceleration Value')
                    ax.set_title(f'{device_name}: Raw vs Transformed (Normal Position)')
                    ax.set_xticks(x_pos)
                    ax.set_xticklabels(x_vals)
                    ax.legend()
                    ax.grid(True, alpha=0.3)
        
        plt.tight_layout()
        
        # Save the plot
        output_file = self.log_file.replace('.log', '_analysis.png')
        plt.savefig(output_file, dpi=300, bbox_inches='tight')
        print(f"Visualization saved to: {output_file}")
        
        plt.show()
    
    def generate_report(self, stats, device_patterns):
        """Generate a comprehensive analysis report."""
        report_file = self.log_file.replace('.log', '_report.txt')
        
        with open(report_file, 'w') as f:
            f.write("CHUWI MINIBOOK X ACCELEROMETER ANALYSIS REPORT\n")
            f.write("=" * 50 + "\n\n")
            f.write(f"Analysis performed on: {datetime.now()}\n")
            f.write(f"Data source: {self.log_file}\n\n")
            
            # Device Information
            f.write("DEVICE CONFIGURATION\n")
            f.write("-" * 20 + "\n")
            
            # Handle both old format (0, 1) and new format ('lid', 'base')
            device_list = []
            if 0 in self.mount_matrices or 1 in self.mount_matrices:
                # Old format
                device_list = [(0, "Device0"), (1, "Device1")]
            else:
                # New format
                device_list = [('lid', "Lid Device"), ('base', "Base Device")]
                
            for device_id, device_name in device_list:
                f.write(f"\n{device_name}:\n")
                if device_id in self.mount_matrices:
                    f.write(f"  Mount Matrix:\n")
                    for row in self.mount_matrices[device_id]:
                        f.write(f"    {row}\n")
                if device_id in self.scales:
                    f.write(f"  Scale: {self.scales[device_id]}\n")
                if device_id in self.scale_available:
                    f.write(f"  Available Scales: {self.scale_available[device_id]}\n")
            
            # Analysis Summary
            f.write(f"\n\nANALYSIS SUMMARY\n")
            f.write("-" * 16 + "\n")
            
            f.write(f"\n1. SENSOR IDENTIFICATION:\n")
            normal_stats = stats.get('laptop_normal_135deg', {})
            
            # Handle both old and new formats
            if 0 in normal_stats and 1 in normal_stats:
                # Old format
                dev0_mag = normal_stats[0]['magnitude']
                dev1_mag = normal_stats[1]['magnitude']
                
                if abs(dev0_mag - 1.0) < abs(dev1_mag - 1.0):
                    f.write("   Device0 = BASE (more stable in normal position)\n")
                    f.write("   Device1 = LID\n")
                else:
                    f.write("   Device1 = BASE (more stable in normal position)\n")
                    f.write("   Device0 = LID\n")
            elif 'lid' in normal_stats and 'base' in normal_stats:
                # New format
                lid_mag = normal_stats['lid']['magnitude']
                base_mag = normal_stats['base']['magnitude']
                f.write(f"   Lid device: magnitude = {lid_mag:.1f}\n")
                f.write(f"   Base device: magnitude = {base_mag:.1f}\n")
                f.write("   (Assignment determined by cmx driver)\n")
            
            f.write(f"\n2. MOUNT MATRIX VALIDATION:\n")
            for device_id, device_name in device_list:
                if device_id in self.mount_matrices:
                    matrix = self.mount_matrices[device_id]
                    det = np.linalg.det(matrix)
                    is_rotation = abs(abs(det) - 1.0) < 0.1
                    f.write(f"   {device_name}: {'Valid rotation matrix' if is_rotation else 'Invalid matrix'} (det={det:.3f})\n")
            
            f.write(f"\n3. SCALE FACTOR UTILITY:\n")
            f.write("   Scale factors convert raw ADC values to m/s²\n")
            f.write("   Essential for meaningful gravity calculations\n")
            
            # Detailed position data
            f.write(f"\n\nDETAILED POSITION DATA\n")
            f.write("-" * 21 + "\n")
            
            for position, pos_stats in stats.items():
                f.write(f"\n{position}:\n")
                
                # Handle both old and new formats
                if 0 in pos_stats and 1 in pos_stats:
                    # Old format
                    for device in [0, 1]:
                        if device in pos_stats:
                            data = pos_stats[device]
                            f.write(f"  Device{device}: magnitude={data['magnitude']:.2f}, ")
                            axis_names = ['X', 'Y', 'Z']
                            f.write(f"dominant={axis_names[data['dominant_axis']]}")
                            f.write(f" ({data['dominant_value']:.2f})\n")
                else:
                    # New format
                    for device_type in ['lid', 'base']:
                        if device_type in pos_stats:
                            data = pos_stats[device_type]
                            f.write(f"  {device_type.capitalize()}: magnitude={data['magnitude']:.2f}, ")
                            axis_names = ['X', 'Y', 'Z']
                            f.write(f"dominant={axis_names[data['dominant_axis']]}")
                            f.write(f" ({data['dominant_value']:.2f})\n")
        
        print(f"Detailed report saved to: {report_file}")
    
    def run_analysis(self):
        """Run the complete analysis pipeline."""
        print("Starting accelerometer data analysis...")
        
        self.parse_log_file()
        
        if not self.position_data:
            print("No position data found in log file!")
            return
        
        stats = self.calculate_gravity_stats()
        device_patterns = self.analyze_sensor_identification(stats)
        self.analyze_mount_matrices(stats)
        self.analyze_scale_factors()
        
        # Generate visualizations (optional, requires matplotlib)
        try:
            self.create_visualizations(stats)
        except ImportError:
            print("matplotlib not available, skipping visualizations")
        except Exception as e:
            print(f"Error creating visualizations: {e}")
        
        self.generate_report(stats, device_patterns)
        
        print("\nAnalysis complete!")

def main():
    parser = argparse.ArgumentParser(description='Analyze Chuwi Minibook X accelerometer data')
    parser.add_argument('log_file', help='Path to the data log file')
    parser.add_argument('--no-plots', action='store_true', help='Skip generating plots')
    
    args = parser.parse_args()
    
    if not sys.argv[1:]:
        print("Usage: ./analyze-orientation-data.py <log_file>")
        sys.exit(1)
    
    analyzer = AccelDataAnalyzer(args.log_file)
    analyzer.run_analysis()

if __name__ == "__main__":
    main()