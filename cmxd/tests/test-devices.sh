#!/bin/bash

# Quick test script to verify accelerometer devices are accessible
# Run this before the full data collection to ensure everything is working

set -euo pipefail

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}=== Chuwi Minibook X Accelerometer Device Test ===${NC}"
echo ""

# Read device assignments from cmx driver
echo -e "${BLUE}Reading device assignments from cmx driver...${NC}"

if [[ -r "/sys/devices/platform/cmx/iio_lid_device" ]]; then
    LID_DEVICE=$(cat /sys/devices/platform/cmx/iio_lid_device)
    echo -e "${GREEN}✓ Lid device: $LID_DEVICE${NC}"
else
    echo -e "${RED}✗ Cannot read lid device assignment from /sys/devices/platform/cmx/iio_lid_device${NC}"
    echo -e "${YELLOW}  Falling back to device0 as lid${NC}"
    LID_DEVICE="iio:device0"
fi

if [[ -r "/sys/devices/platform/cmx/iio_base_device" ]]; then
    BASE_DEVICE=$(cat /sys/devices/platform/cmx/iio_base_device)
    echo -e "${GREEN}✓ Base device: $BASE_DEVICE${NC}"
else
    echo -e "${RED}✗ Cannot read base device assignment from /sys/devices/platform/cmx/iio_base_device${NC}"
    echo -e "${YELLOW}  Falling back to device1 as base${NC}"
    BASE_DEVICE="iio:device1"
fi

# Extract device numbers for path construction
LID_NUM=$(echo "$LID_DEVICE" | sed 's/iio:device//')
BASE_NUM=$(echo "$BASE_DEVICE" | sed 's/iio:device//')

echo -e "${BLUE}Device mapping: lid/$LID_NUM, base/$BASE_NUM${NC}"
echo ""

# Check device directories
echo -e "${BLUE}Checking device directories...${NC}"

# Check lid device
lid_path="/sys/bus/iio/devices/$LID_DEVICE"
if [[ -d "$lid_path" ]]; then
    echo -e "${GREEN}✓ Lid device (lid/$LID_NUM) found at $lid_path${NC}"
else
    echo -e "${RED}✗ Lid device (lid/$LID_NUM) NOT found at $lid_path${NC}"
    exit 1
fi

# Check base device  
base_path="/sys/bus/iio/devices/$BASE_DEVICE"
if [[ -d "$base_path" ]]; then
    echo -e "${GREEN}✓ Base device (base/$BASE_NUM) found at $base_path${NC}"
else
    echo -e "${RED}✗ Base device (base/$BASE_NUM) NOT found at $base_path${NC}"
    exit 1
fi

echo ""

# Check required files
echo -e "${BLUE}Checking required files...${NC}"

# Define devices with their labels
declare -A DEVICES=(
    ["$lid_path"]="lid/$LID_NUM"
    ["$base_path"]="base/$BASE_NUM"
)

for device_path in "${!DEVICES[@]}"; do
    device_label="${DEVICES[$device_path]}"
    
    # Check accelerometer raw data files
    for axis in x y z; do
        file_path="${device_path}/in_accel_${axis}_raw"
        if [[ -r "$file_path" ]]; then
            echo -e "${GREEN}✓ $device_label ${axis}-axis readable${NC}"
        else
            echo -e "${RED}✗ $device_label ${axis}-axis NOT readable${NC}"
            exit 1
        fi
    done
    
    # Check mount matrix
    mount_matrix_path="${device_path}/in_accel_mount_matrix"
    if [[ -r "$mount_matrix_path" ]]; then
        echo -e "${GREEN}✓ $device_label mount matrix readable${NC}"
    else
        echo -e "${YELLOW}⚠ $device_label mount matrix not found (may not be set)${NC}"
    fi
    
    # Check scale
    scale_path="${device_path}/in_accel_scale"
    if [[ -r "$scale_path" ]]; then
        echo -e "${GREEN}✓ $device_label scale readable${NC}"
    else
        echo -e "${YELLOW}⚠ $device_label scale not found${NC}"
    fi
done

echo ""

# Sample current data
echo -e "${BLUE}Sampling current accelerometer data...${NC}"

# Sample lid device
echo -e "${YELLOW}Lid device (lid/$LID_NUM):${NC}"
x=$(cat "${lid_path}/in_accel_x_raw" 2>/dev/null || echo "N/A")
y=$(cat "${lid_path}/in_accel_y_raw" 2>/dev/null || echo "N/A")
z=$(cat "${lid_path}/in_accel_z_raw" 2>/dev/null || echo "N/A")

echo "  Raw values: X=$x, Y=$y, Z=$z"

# Mount matrix if available
if [[ -r "${lid_path}/in_accel_mount_matrix" ]]; then
    mount_matrix=$(cat "${lid_path}/in_accel_mount_matrix")
    echo "  Mount matrix: $mount_matrix"
fi

# Scale if available
if [[ -r "${lid_path}/in_accel_scale" ]]; then
    scale=$(cat "${lid_path}/in_accel_scale")
    echo "  Scale factor: $scale"
fi

echo ""

# Sample base device
echo -e "${YELLOW}Base device (base/$BASE_NUM):${NC}"
x=$(cat "${base_path}/in_accel_x_raw" 2>/dev/null || echo "N/A")
y=$(cat "${base_path}/in_accel_y_raw" 2>/dev/null || echo "N/A")
z=$(cat "${base_path}/in_accel_z_raw" 2>/dev/null || echo "N/A")

echo "  Raw values: X=$x, Y=$y, Z=$z"

# Mount matrix if available
if [[ -r "${base_path}/in_accel_mount_matrix" ]]; then
    mount_matrix=$(cat "${base_path}/in_accel_mount_matrix")
    echo "  Mount matrix: $mount_matrix"
fi

# Scale if available
if [[ -r "${base_path}/in_accel_scale" ]]; then
    scale=$(cat "${base_path}/in_accel_scale")
    echo "  Scale factor: $scale"
fi

echo ""

# Test data collection for a few samples
echo -e "${BLUE}Testing data collection (5 samples)...${NC}"
for i in {1..5}; do
    timestamp=$(date '+%H:%M:%S.%3N')
    
    lid_x=$(cat "${lid_path}/in_accel_x_raw")
    lid_y=$(cat "${lid_path}/in_accel_y_raw")
    lid_z=$(cat "${lid_path}/in_accel_z_raw")
    
    base_x=$(cat "${base_path}/in_accel_x_raw")
    base_y=$(cat "${base_path}/in_accel_y_raw")
    base_z=$(cat "${base_path}/in_accel_z_raw")
    
    echo "$timestamp: lid/$LID_NUM($lid_x,$lid_y,$lid_z) base/$BASE_NUM($base_x,$base_y,$base_z)"
    
    sleep 0.2
done

echo ""
echo -e "${GREEN}=== Device Test Complete! ===${NC}"
echo -e "${BLUE}All devices are accessible and providing data.${NC}"
echo -e "${BLUE}Device assignments: lid/$LID_NUM, base/$BASE_NUM${NC}"
echo -e "${BLUE}You can now run the full data collection:${NC}"
echo -e "${YELLOW}  ./collect-orientation-data.sh${NC}"