#!/bin/bash
# Configuration validation and troubleshooting script
# for Chuwi Minibook X tablet mode detection
#
# Copyright (c) 2025 Armando DiCianno <armando@noonshy.com>
# Licensed under GPL-2.0

set -euo pipefail

SCRIPT_NAME="$(basename "$0")"
PROGRAM_NAME="chuwi-minibook-x-tablet-mode"
CONFIG_FILE="/etc/default/$PROGRAM_NAME"
SYSFS_BASE="/sys/kernel/chuwi-minibook-x-tablet-mode"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Status tracking
ISSUES_FOUND=0
WARNINGS_FOUND=0

log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[OK]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
    ((WARNINGS_FOUND++))
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
    ((ISSUES_FOUND++))
}

usage() {
    cat << EOF
Usage: $SCRIPT_NAME [OPTIONS]

Validate configuration and troubleshoot issues for $PROGRAM_NAME

Options:
  -f, --fix         Attempt to fix common issues automatically
  -v, --verbose     Show detailed information
  -h, --help        Show this help

Examples:
  $SCRIPT_NAME                    # Check configuration
  $SCRIPT_NAME --fix             # Check and attempt fixes
  $SCRIPT_NAME --verbose         # Detailed diagnostic info
EOF
}

# Check if running as root
check_root() {
    if [[ $EUID -eq 0 ]]; then
        log_info "Running with root privileges"
        return 0
    else
        log_warning "Not running as root - some checks may be limited"
        return 1
    fi
}

# Check kernel module
check_kernel_module() {
    log_info "Checking kernel module..."
    
    if lsmod | grep -q "chuwi_minibook_x_tablet_mode"; then
        log_success "Kernel module is loaded"
    else
        log_error "Kernel module not loaded"
        echo "  Try: sudo modprobe chuwi-minibook-x-tablet-mode"
        return 1
    fi
    
    if [[ -d "$SYSFS_BASE" ]]; then
        log_success "Kernel module sysfs is available: $SYSFS_BASE"
        
        # Check sysfs permissions
        if [[ -w "$SYSFS_BASE/base_vec" && -w "$SYSFS_BASE/lid_vec" ]]; then
            log_success "Sysfs is writable"
        else
            log_warning "Sysfs may not be writable by the service user"
        fi
    else
        log_error "Kernel module sysfs not found: $SYSFS_BASE"
        return 1
    fi
    
    return 0
}

# Check IIO devices
check_iio_devices() {
    log_info "Checking IIO devices..."
    
    local iio_devices=()
    local mxc4005_devices=()
    
    # Find all IIO devices
    for dev_path in /sys/bus/iio/devices/iio:device*; do
        if [[ -d "$dev_path" ]]; then
            local dev_name=$(basename "$dev_path")
            iio_devices+=("$dev_name")
            
            # Check if it's an MXC4005
            local name_file="$dev_path/name"
            if [[ -r "$name_file" ]] && grep -qi "^mxc4005$" "$name_file"; then
                mxc4005_devices+=("$dev_name")
            fi
        fi
    done
    
    log_info "Found ${#iio_devices[@]} IIO device(s): ${iio_devices[*]}"
    log_info "Found ${#mxc4005_devices[@]} MXC4005 device(s): ${mxc4005_devices[*]}"
    
    if [[ ${#mxc4005_devices[@]} -lt 2 ]]; then
        log_error "Need at least 2 MXC4005 devices, found ${#mxc4005_devices[@]}"
        if [[ ${#mxc4005_devices[@]} -eq 0 ]]; then
            echo "  No MXC4005 accelerometers found"
            echo "  Check if the mxc4005 driver is loaded: lsmod | grep mxc4005"
        else
            echo "  Only found 1 MXC4005 device: ${mxc4005_devices[0]}"
            echo "  The Chuwi Minibook X should have 2 accelerometers (base and lid)"
        fi
        return 1
    else
        log_success "Found sufficient MXC4005 devices"
    fi
    
    # Test accelerometer readings
    for dev in "${mxc4005_devices[@]:0:2}"; do
        check_accelerometer_data "$dev"
    done
    
    return 0
}

# Check individual accelerometer
check_accelerometer_data() {
    local dev="$1"
    local dev_path="/sys/bus/iio/devices/$dev"
    
    log_info "Testing accelerometer data for $dev..."
    
    # Check required files exist
    local required_files=("in_accel_x_raw" "in_accel_y_raw" "in_accel_z_raw")
    for file in "${required_files[@]}"; do
        local full_path="$dev_path/$file"
        if [[ ! -r "$full_path" ]]; then
            log_error "Cannot read $full_path"
            return 1
        fi
    done
    
    # Read current values
    local x y z
    x=$(cat "$dev_path/in_accel_x_raw" 2>/dev/null || echo "ERROR")
    y=$(cat "$dev_path/in_accel_y_raw" 2>/dev/null || echo "ERROR")
    z=$(cat "$dev_path/in_accel_z_raw" 2>/dev/null || echo "ERROR")
    
    if [[ "$x" == "ERROR" || "$y" == "ERROR" || "$z" == "ERROR" ]]; then
        log_error "Cannot read accelerometer values from $dev"
        return 1
    fi
    
    # Validate values are numeric
    if ! [[ "$x" =~ ^-?[0-9]+$ ]] || ! [[ "$y" =~ ^-?[0-9]+$ ]] || ! [[ "$z" =~ ^-?[0-9]+$ ]]; then
        log_error "Invalid accelerometer values from $dev: x=$x y=$y z=$z"
        return 1
    fi
    
    log_success "$dev: x=$x y=$y z=$z"
    
    # Check scale factor
    local scale_file="$dev_path/in_accel_scale"
    if [[ -r "$scale_file" ]]; then
        local scale=$(cat "$scale_file" 2>/dev/null || echo "0")
        log_info "$dev scale factor: $scale"
    else
        log_warning "$dev: no scale factor available"
    fi
    
    # Warn about zero values (device might be stuck)
    if [[ "$x" -eq 0 && "$y" -eq 0 && "$z" -eq 0 ]]; then
        log_warning "$dev reports all zeros - device may be stuck"
    fi
    
    return 0
}

# Check service status
check_service() {
    log_info "Checking service status..."
    
    if systemctl is-enabled "$PROGRAM_NAME" >/dev/null 2>&1; then
        log_success "Service is enabled"
    else
        log_warning "Service is not enabled"
        echo "  Enable with: sudo systemctl enable $PROGRAM_NAME"
    fi
    
    if systemctl is-active "$PROGRAM_NAME" >/dev/null 2>&1; then
        log_success "Service is running"
    else
        log_warning "Service is not running"
        echo "  Start with: sudo systemctl start $PROGRAM_NAME"
        
        # Show recent logs
        echo "  Recent logs:"
        systemctl status "$PROGRAM_NAME" --no-pager -l -n 5 2>/dev/null | sed 's/^/    /' || true
    fi
}

# Check configuration file
check_config() {
    log_info "Checking configuration..."
    
    if [[ -f "$CONFIG_FILE" ]]; then
        log_success "Configuration file exists: $CONFIG_FILE"
        
        # Source and validate config
        set +u  # Allow unset variables temporarily
        source "$CONFIG_FILE"
        set -u
        
        if [[ -n "${BASE_IIO:-}" ]]; then
            log_info "Base device configured: $BASE_IIO"
            if [[ ! -d "/sys/bus/iio/devices/$BASE_IIO" ]]; then
                log_error "Configured base device not found: $BASE_IIO"
            fi
        fi
        
        if [[ -n "${LID_IIO:-}" ]]; then
            log_info "Lid device configured: $LID_IIO"
            if [[ ! -d "/sys/bus/iio/devices/$LID_IIO" ]]; then
                log_error "Configured lid device not found: $LID_IIO"
            fi
        fi
        
        if [[ -n "${POLL_MS:-}" ]]; then
            log_info "Poll interval configured: ${POLL_MS}ms"
            if ! [[ "$POLL_MS" =~ ^[0-9]+$ ]] || [[ "$POLL_MS" -lt 10 ]] || [[ "$POLL_MS" -gt 10000 ]]; then
                log_warning "Poll interval seems unusual: ${POLL_MS}ms (recommended: 50-500ms)"
            fi
        fi
    else
        log_info "No configuration file found (using defaults)"
        echo "  Create $CONFIG_FILE to customize settings"
    fi
}

# Check binary and permissions
check_binary() {
    log_info "Checking program binary..."
    
    local binary_path="/usr/sbin/$PROGRAM_NAME"
    if [[ -x "$binary_path" ]]; then
        log_success "Binary is installed and executable: $binary_path"
    else
        log_error "Binary not found or not executable: $binary_path"
        echo "  Install with: sudo make install"
        return 1
    fi
    
    local wrapper_path="/usr/sbin/$PROGRAM_NAME-wrapper"
    if [[ -x "$wrapper_path" ]]; then
        log_success "Wrapper script is installed: $wrapper_path"
    else
        log_error "Wrapper script not found: $wrapper_path"
        return 1
    fi
    
    return 0
}

# Test data flow
test_data_flow() {
    log_info "Testing data flow..."
    
    if [[ ! -w "$SYSFS_BASE/base_vec" ]] || [[ ! -w "$SYSFS_BASE/lid_vec" ]]; then
        log_warning "Cannot write to sysfs - skipping data flow test"
        return 0
    fi
    
    # Read current state
    local before_angle=$(cat "$SYSFS_BASE/angle" 2>/dev/null || echo "unknown")
    
    # Write test data
    echo "1000 0 0" > "$SYSFS_BASE/base_vec"
    echo "0 1000 0" > "$SYSFS_BASE/lid_vec"
    
    sleep 0.1
    
    local after_angle=$(cat "$SYSFS_BASE/angle" 2>/dev/null || echo "unknown")
    
    if [[ "$after_angle" != "unknown" && "$after_angle" != "$before_angle" ]]; then
        log_success "Data flow test passed (angle changed from $before_angle to $after_angle)"
    else
        log_warning "Data flow test unclear (angle: $before_angle -> $after_angle)"
    fi
}

# Attempt automatic fixes
auto_fix() {
    log_info "Attempting automatic fixes..."
    
    local fixes_applied=0
    
    # Load kernel module if not loaded
    if ! lsmod | grep -q "chuwi_minibook_x_tablet_mode"; then
        log_info "Loading kernel module..."
        if modprobe chuwi-minibook-x-tablet-mode 2>/dev/null; then
            log_success "Kernel module loaded"
            ((fixes_applied++))
        else
            log_error "Failed to load kernel module"
        fi
    fi
    
    # Enable service if not enabled
    if ! systemctl is-enabled "$PROGRAM_NAME" >/dev/null 2>&1; then
        log_info "Enabling service..."
        if systemctl enable "$PROGRAM_NAME" 2>/dev/null; then
            log_success "Service enabled"
            ((fixes_applied++))
        else
            log_error "Failed to enable service"
        fi
    fi
    
    # Start service if not running
    if ! systemctl is-active "$PROGRAM_NAME" >/dev/null 2>&1; then
        log_info "Starting service..."
        if systemctl start "$PROGRAM_NAME" 2>/dev/null; then
            log_success "Service started"
            ((fixes_applied++))
        else
            log_error "Failed to start service"
        fi
    fi
    
    if [[ $fixes_applied -gt 0 ]]; then
        log_success "Applied $fixes_applied automatic fix(es)"
    else
        log_info "No automatic fixes needed"
    fi
}

# Main validation function
run_validation() {
    local auto_fix_mode="${1:-false}"
    local verbose_mode="${2:-false}"
    
    echo "=== $PROGRAM_NAME Configuration Validation ==="
    echo ""
    
    check_root
    echo ""
    
    check_kernel_module
    echo ""
    
    check_iio_devices
    echo ""
    
    check_binary
    echo ""
    
    check_config
    echo ""
    
    check_service
    echo ""
    
    if [[ "$verbose_mode" == "true" ]]; then
        test_data_flow
        echo ""
    fi
    
    if [[ "$auto_fix_mode" == "true" ]]; then
        auto_fix
        echo ""
    fi
    
    # Summary
    echo "=== Summary ==="
    if [[ $ISSUES_FOUND -eq 0 && $WARNINGS_FOUND -eq 0 ]]; then
        log_success "All checks passed - system appears to be working correctly"
    elif [[ $ISSUES_FOUND -eq 0 ]]; then
        log_warning "Found $WARNINGS_FOUND warning(s) but no critical issues"
    else
        log_error "Found $ISSUES_FOUND error(s) and $WARNINGS_FOUND warning(s)"
        echo ""
        echo "Common solutions:"
        echo "  1. Load kernel module: sudo modprobe chuwi-minibook-x-tablet-mode"
        echo "  2. Install userspace: sudo make install"
        echo "  3. Enable service: sudo systemctl enable --now $PROGRAM_NAME"
        echo "  4. Check hardware: ensure MXC4005 devices are detected"
        return 1
    fi
    
    return 0
}

# Parse command line arguments
AUTO_FIX=false
VERBOSE=false

while [[ $# -gt 0 ]]; do
    case $1 in
        -f|--fix)
            AUTO_FIX=true
            shift
            ;;
        -v|--verbose)
            VERBOSE=true
            shift
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            usage
            exit 1
            ;;
    esac
done

# Run validation
run_validation "$AUTO_FIX" "$VERBOSE"