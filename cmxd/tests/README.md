# Accelerometer Testing Suite

This directory contains tools for collecting and analyzing accelerometer data from the Chuwi Minibook X to validate sensor identification and mount matrix correctness.

## Files

- `collect-orientation-data.sh` - Interactive data collection script
- `analyze-orientation-data.py` - Post-processing analysis script  
- `reverse-engineer-mount-matrix.py` - Mount matrix analysis from collected data
- `test-devices.sh` - Quick device accessibility test
- `MOUNT_MATRIX_ANALYSIS_RESULTS.md` - Analysis findings and recommendations
- `README.md` - This file

## Mount Matrix Analysis Results 🎯

**IMPORTANT**: Our analysis revealed that the **base sensor Z-axis was inverted** in the original mount matrix. This has been corrected as of the latest version. The correct mount matrices are:

- **Lid sensor**: Identity matrix (no rotation needed)
- **Base sensor**: 90° CW rotation **with Z-axis inversion**

See `MOUNT_MATRIX_ANALYSIS_RESULTS.md` for detailed analysis.

## Data Collection

The collection script (`collect-orientation-data.sh`) will guide you through positioning the laptop in various orientations while collecting accelerometer data. It focuses on positions with the lid at 135° (the most interesting angle for analysis) and rotates the laptop through 3D space.

### Prerequisites

```bash
# Install bc calculator (required for timing)
sudo apt-get install bc

# Optional: Install matplotlib for visualizations
pip3 install matplotlib numpy
```

### Usage

```bash
cd /path/to/minibook-x-tools/cmxd/tests
./collect-orientation-data.sh
```

The script will:
1. Check that both accelerometer devices are available
2. Collect device configuration (mount matrices, scales)
3. Guide you through 13 different positions
4. Collect 5 seconds of data at 10Hz for each position
5. Save everything to a timestamped log file

### Positions Tested

1. **laptop_normal_135deg** - Normal laptop position, lid at 135°
2. **laptop_upside_down_135deg** - Laptop upside down, lid at 135°
3. **laptop_left_side_135deg** - On left side, screen facing right
4. **laptop_right_side_135deg** - On right side, screen facing left
5. **laptop_tilted_forward_135deg** - Tilted forward, screen angled down
6. **laptop_tilted_backward_135deg** - Tilted backward, screen angled up
7. **laptop_rotated_45deg_left_135deg** - Rotated 45° CCW
8. **laptop_rotated_45deg_right_135deg** - Rotated 45° CW
9. **flat_screen_up** - Completely flat, screen up (180° open)
10. **flat_screen_down** - Completely flat, screen down (180° open)
11. **tent_mode** - Tent mode (inverted V)
12. **tablet_mode_screen_up** - Tablet mode, screen up
13. **tablet_mode_screen_down** - Tablet mode, screen down

## Data Analysis

The analysis script (`analyze-orientation-data.py`) processes the collected data to answer key questions about the accelerometer setup.

### Usage

```bash
./analyze-orientation-data.py orientation-data-20231107-142030.log
```

### Analysis Output

The script provides:

1. **Sensor Identification**: Determines which device (0 or 1) is the lid vs base
2. **Mount Matrix Validation**: Checks if the rotation matrices are mathematically correct
3. **Scale Factor Analysis**: Explains the purpose of scale factors
4. **Detailed Report**: Text file with comprehensive analysis
5. **Visualizations**: Plots showing gravity patterns (requires matplotlib)

### Key Questions Answered

#### 1. Device Identification
- Analyzes gravity magnitude stability in normal laptop position
- The base should be more stable (closer to 1g) than the lid
- Cross-references with expected behavior in different orientations

#### 2. Mount Matrix Correctness
- Validates that matrices are proper rotation matrices (determinant = ±1)
- Identifies the type of rotation (90° CW/CCW, identity, etc.)
- Checks mathematical properties (orthogonality)

#### 3. Scale Factor Utility
- Explains that scales convert raw ADC values to m/s²
- Shows available scale options for each device
- Confirms importance for meaningful gravity calculations

## Data Format

The log file contains:
- Device configuration (mount matrices, scales)
- Position-labeled data sections
- CSV format: `timestamp,dev0_x,dev0_y,dev0_z,dev1_x,dev1_y,dev1_z`
- Raw accelerometer values (integer ADC counts)

## Expected Behavior

Based on the hardware analysis:
- **Device0**: Should be the lid sensor (90° CCW rotation)
- **Device1**: Should be the base sensor (90° CW rotation)
- Mount matrices should transform coordinates correctly
- In normal laptop position, base should show stable gravity vector

## Troubleshooting

### Common Issues

1. **Devices not found**: Check that accelerometers are detected:
   ```bash
   ls /sys/bus/iio/devices/
   ```

2. **Permission errors**: Run as root or check file permissions:
   ```bash
   sudo ./collect-orientation-data.sh
   ```

3. **bc not found**: Install calculator:
   ```bash
   sudo apt-get install bc
   ```

4. **Python dependencies**: Install analysis dependencies:
   ```bash
   pip3 install numpy matplotlib
   ```

### Verification

To verify the collection is working:
```bash
# Check device paths exist
ls /sys/bus/iio/devices/iio:device{0,1}/in_accel_*_raw

# Check mount matrices
cat /sys/bus/iio/devices/iio:device*/in_accel_mount_matrix

# Monitor real-time data
watch 'cat /sys/bus/iio/devices/iio:device*/in_accel_*_raw'
```

## Output Files

- `orientation-data-YYYYMMDD-HHMMSS.log` - Raw data collection
- `orientation-data-YYYYMMDD-HHMMSS_report.txt` - Analysis report
- `orientation-data-YYYYMMDD-HHMMSS_analysis.png` - Visualization plots

## Integration with cmxd

This testing validates the foundation for the `cmxd` daemon:
- Confirms correct device identification
- Validates mount matrix transformations
- Provides data for hinge angle algorithm development
- Ensures scale factors are properly applied