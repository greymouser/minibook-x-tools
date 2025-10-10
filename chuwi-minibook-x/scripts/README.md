# Scripts Directory

This directory contains supporting scripts for testing, analysis, and debugging the Chuwi MiniBook X tablet mode detection module.

## Testing Scripts

### `sensor_test.sh`
Interactive sensor identification test that helps determine which sensor is in the lid vs base through guided movements.

### `base_axis_test.sh`
Comprehensive base sensor axis orientation testing script with detailed analysis.

### `test_logic.sh`
Logic testing script for validating tablet mode detection algorithms.

## Analysis Scripts

### `sensor_analysis.sh`
General sensor data analysis script for examining accelerometer readings.

### `base_orientation_analysis.sh`
Specialized analysis for base sensor orientation and coordinate system verification.

### `sensor_status.sh`
Quick status check script that shows current sensor readings and device information.

## Demo Scripts

### `mount_matrix_demo.sh`
Demonstration script showing mount matrix coordinate transformations in action.

## Usage

All scripts are executable and can be run directly:

```bash
# Run the main sensor identification test
./sensor_test.sh

# Check current sensor status
./sensor_status.sh

# Analyze sensor data
./sensor_analysis.sh

# Demo mount matrix functionality
./mount_matrix_demo.sh
```

Most scripts provide interactive guidance and detailed output to help with development and debugging.