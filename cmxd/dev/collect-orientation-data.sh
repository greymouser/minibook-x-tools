#!/bin/bash

# Data collection script for Chuwi Minibook X accelerometer analysis
# This script collects accelerometer data in various orientations to analyze:
# 1. Sensor identification (lid vs base)
# 2. Mount matrix correctness
# 3. Scale factor usage

set -euo pipefail

# Read device assignments from cmx driver
if [[ -r "/sys/devices/platform/cmx/iio_lid_device" ]]; then
    LID_DEVICE=$(cat /sys/devices/platform/cmx/iio_lid_device)
else
    echo "Warning: Cannot read lid device assignment, falling back to device0"
    LID_DEVICE="iio:device0"
fi

if [[ -r "/sys/devices/platform/cmx/iio_base_device" ]]; then
    BASE_DEVICE=$(cat /sys/devices/platform/cmx/iio_base_device)
else
    echo "Warning: Cannot read base device assignment, falling back to device1"
    BASE_DEVICE="iio:device1"
fi

# Extract device numbers and construct paths
LID_NUM=$(echo "$LID_DEVICE" | sed 's/iio:device//')
BASE_NUM=$(echo "$BASE_DEVICE" | sed 's/iio:device//')
LID_DEVICE_PATH="/sys/bus/iio/devices/$LID_DEVICE"
BASE_DEVICE_PATH="/sys/bus/iio/devices/$BASE_DEVICE"

# Configuration
SAMPLE_DURATION=5
SAMPLE_RATE=10  # samples per second
LOG_FILE="orientation-data-$(date +%Y%m%d-%H%M%S).log"

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Test positions - focusing on 135° lid opening with various 3D rotations
declare -a POSITIONS=(
    "laptop_normal_135deg"
    "laptop_upside_down_135deg"
    "laptop_left_side_135deg"
    "laptop_right_side_135deg"
    "laptop_tilted_forward_135deg"
    "laptop_tilted_backward_135deg"
    "laptop_rotated_45deg_left_135deg"
    "laptop_rotated_45deg_right_135deg"
    "flat_screen_up"
    "flat_screen_down"
    "tent_mode"
    "tablet_mode_screen_up"
    "tablet_mode_screen_down"
)

# Position descriptions for user guidance
declare -A POSITION_DESCRIPTIONS=(
    ["laptop_normal_135deg"]="Normal laptop position, lid open at 135°, on flat surface"
    ["laptop_upside_down_135deg"]="Laptop upside down (base on top), lid open at 135°"
    ["laptop_left_side_135deg"]="Laptop on left side, lid open at 135°, screen facing right"
    ["laptop_right_side_135deg"]="Laptop on right side, lid open at 135°, screen facing left"
    ["laptop_tilted_forward_135deg"]="Laptop tilted forward (screen angled down), lid at 135°"
    ["laptop_tilted_backward_135deg"]="Laptop tilted backward (screen angled up), lid at 135°"
    ["laptop_rotated_45deg_left_135deg"]="Laptop rotated 45° counter-clockwise, lid at 135°"
    ["laptop_rotated_45deg_right_135deg"]="Laptop rotated 45° clockwise, lid at 135°"
    ["flat_screen_up"]="Completely flat, screen facing up (180° open)"
    ["flat_screen_down"]="Completely flat, screen facing down (180° open)"
    ["tent_mode"]="Tent mode (inverted V shape, both parts touching surface)"
    ["tablet_mode_screen_up"]="Tablet mode, screen facing up"
    ["tablet_mode_screen_down"]="Tablet mode, screen facing down"
)

log_message() {
    local message="$1"
    local timestamp=$(date '+%Y-%m-%d %H:%M:%S')
    echo "[$timestamp] $message" | tee -a "$LOG_FILE"
}

check_devices() {
    echo -e "${BLUE}Checking accelerometer devices...${NC}"
    echo -e "${BLUE}Device assignments: lid/$LID_NUM, base/$BASE_NUM${NC}"
    
    if [[ ! -d "$LID_DEVICE_PATH" ]]; then
        echo -e "${RED}Error: Lid device $LID_DEVICE_PATH not found${NC}"
        exit 1
    fi
    
    if [[ ! -d "$BASE_DEVICE_PATH" ]]; then
        echo -e "${RED}Error: Base device $BASE_DEVICE_PATH not found${NC}"
        exit 1
    fi
    
    echo -e "${GREEN}Both devices found${NC}"
}

collect_device_info() {
    echo -e "${BLUE}Collecting device information...${NC}"
    
    log_message "=== DEVICE INFORMATION ==="
    log_message "Collection started at: $(date)"
    log_message "Sample duration: ${SAMPLE_DURATION}s per position"
    log_message "Sample rate: ${SAMPLE_RATE} Hz"
    log_message "Device assignments: lid/$LID_NUM, base/$BASE_NUM"
    log_message ""
    
    # Lid device information
    log_message "--- Lid Device (lid/$LID_NUM) Information ---"
    
    # Device name/type
    if [[ -f "${LID_DEVICE_PATH}/name" ]]; then
        log_message "Lid device name: $(cat ${LID_DEVICE_PATH}/name)"
    fi
    
    # Mount matrix
    if [[ -f "${LID_DEVICE_PATH}/in_accel_mount_matrix" ]]; then
        log_message "Lid device mount_matrix: $(cat ${LID_DEVICE_PATH}/in_accel_mount_matrix)"
    fi
    
    # Scale factor
    if [[ -f "${LID_DEVICE_PATH}/in_accel_scale" ]]; then
        log_message "Lid device scale: $(cat ${LID_DEVICE_PATH}/in_accel_scale)"
    fi
    
    # Available scale factors
    if [[ -f "${LID_DEVICE_PATH}/in_accel_scale_available" ]]; then
        log_message "Lid device scale_available: $(cat ${LID_DEVICE_PATH}/in_accel_scale_available)"
    fi
    
    log_message ""
    
    # Base device information
    log_message "--- Base Device (base/$BASE_NUM) Information ---"
    
    # Device name/type
    if [[ -f "${BASE_DEVICE_PATH}/name" ]]; then
        log_message "Base device name: $(cat ${BASE_DEVICE_PATH}/name)"
    fi
    
    # Mount matrix
    if [[ -f "${BASE_DEVICE_PATH}/in_accel_mount_matrix" ]]; then
        log_message "Base device mount_matrix: $(cat ${BASE_DEVICE_PATH}/in_accel_mount_matrix)"
    fi
    
    # Scale factor
    if [[ -f "${BASE_DEVICE_PATH}/in_accel_scale" ]]; then
        log_message "Base device scale: $(cat ${BASE_DEVICE_PATH}/in_accel_scale)"
    fi
    
    # Available scale factors
    if [[ -f "${BASE_DEVICE_PATH}/in_accel_scale_available" ]]; then
        log_message "Base device scale_available: $(cat ${BASE_DEVICE_PATH}/in_accel_scale_available)"
    fi
    
    log_message ""
}

read_sensor_data() {
    local device_type="$1"  # "lid" or "base"
    
    if [[ "$device_type" == "lid" ]]; then
        local device_path="$LID_DEVICE_PATH"
    else
        local device_path="$BASE_DEVICE_PATH"
    fi
    
    # Read raw accelerometer values
    local x=$(cat "${device_path}/in_accel_x_raw" 2>/dev/null || echo "N/A")
    local y=$(cat "${device_path}/in_accel_y_raw" 2>/dev/null || echo "N/A")
    local z=$(cat "${device_path}/in_accel_z_raw" 2>/dev/null || echo "N/A")
    
    echo "${x},${y},${z}"
}

collect_position_data() {
    local position="$1"
    local description="$2"
    
    echo -e "${YELLOW}Position: $position${NC}"
    echo -e "${BLUE}Description: $description${NC}"
    echo -e "${GREEN}Please position the laptop as described above.${NC}"
    echo -e "${GREEN}Press ENTER when ready to start ${SAMPLE_DURATION}s data collection...${NC}"
    read -r
    
    log_message "=== POSITION: $position ==="
    log_message "Description: $description"
    log_message "Timestamp: $(date '+%Y-%m-%d %H:%M:%S')"
    log_message "Duration: ${SAMPLE_DURATION}s"
    log_message "Sample_rate: ${SAMPLE_RATE}Hz"
    log_message ""
    log_message "# Format: timestamp,lid_x,lid_y,lid_z,base_x,base_y,base_z (lid/$LID_NUM, base/$BASE_NUM)"
    
    echo -e "${GREEN}Collecting data for ${SAMPLE_DURATION} seconds...${NC}"
    
    local sample_interval=$(echo "scale=3; 1.0 / $SAMPLE_RATE" | bc -l)
    local total_samples=$((SAMPLE_DURATION * SAMPLE_RATE))
    
    for ((i=1; i<=total_samples; i++)); do
        local timestamp=$(date '+%Y-%m-%d %H:%M:%S.%3N')
        local lid_data=$(read_sensor_data "lid")
        local base_data=$(read_sensor_data "base")
        
        log_message "${timestamp},${lid_data},${base_data}"
        
        # Progress indicator
        if ((i % SAMPLE_RATE == 0)); then
            local seconds_elapsed=$((i / SAMPLE_RATE))
            echo -e "${BLUE}  Progress: ${seconds_elapsed}/${SAMPLE_DURATION}s${NC}"
        fi
        
        sleep "$sample_interval"
    done
    
    log_message ""
    echo -e "${GREEN}Data collection complete for $position${NC}"
    echo ""
}

main() {
    echo -e "${BLUE}=== Chuwi Minibook X Accelerometer Data Collection ===${NC}"
    echo -e "${BLUE}This script will collect accelerometer data in various orientations${NC}"
    echo -e "${BLUE}to help analyze sensor identification and mount matrix correctness.${NC}"
    echo ""
    
    # Check if bc is available for floating point calculations
    if ! command -v bc &> /dev/null; then
        echo -e "${RED}Error: 'bc' calculator is required but not installed.${NC}"
        echo "Please install it with: sudo apt-get install bc"
        exit 1
    fi
    
    check_devices
    
    echo -e "${YELLOW}Data will be saved to: $LOG_FILE${NC}"
    echo -e "${GREEN}Press ENTER to start data collection...${NC}"
    read -r
    
    collect_device_info
    
    # Collect data for each position
    for position in "${POSITIONS[@]}"; do
        collect_position_data "$position" "${POSITION_DESCRIPTIONS[$position]}"
    done
    
    log_message "=== COLLECTION COMPLETE ==="
    log_message "Total positions tested: ${#POSITIONS[@]}"
    log_message "Collection finished at: $(date)"
    
    echo -e "${GREEN}=== Data Collection Complete! ===${NC}"
    echo -e "${BLUE}Data saved to: $LOG_FILE${NC}"
    echo -e "${BLUE}You can now run the post-processing analysis script.${NC}"
    echo ""
    echo -e "${YELLOW}Next steps:${NC}"
    echo "1. Review the log file: less $LOG_FILE"
    echo "2. Run analysis script: ./analyze-orientation-data.py $LOG_FILE"
}

# Handle Ctrl+C gracefully
trap 'echo -e "\n${RED}Data collection interrupted${NC}"; log_message "Collection interrupted at $(date)"; exit 1' INT

main "$@"