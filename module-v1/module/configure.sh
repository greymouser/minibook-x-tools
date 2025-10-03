#!/bin/bash
# Configuration demonstration script for chuwi-minibook-x-tablet-mode module
#
# Copyright (c) 2025 Armando DiCianno <armando@noonshy.com>
# Licensed under GPL-2.0

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
MODULE_NAME="chuwi-minibook-x-tablet-mode"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

usage() {
    echo "Usage: $0 [release|debug]"
    echo ""
    echo "Build configurations:"
    echo "  release    Build without debugfs support (default, smaller size)"
    echo "  debug      Build with debugfs support (larger, includes debugging)"
    echo ""
    echo "Examples:"
    echo "  $0 release   # Build production version"
    echo "  $0 debug     # Build debug version with debugfs"
}

show_config_info() {
    local config="$1"
    
    echo ""
    log_info "Configuration: $config"
    
    case "$config" in
        "release")
            echo "  • Optimized for production use"
            echo "  • Smaller binary size"
            echo "  • No debugfs dependencies"
            echo "  • Suitable for embedded systems"
            ;;
        "debug")
            echo "  • Includes debugfs interface"
            echo "  • Advanced debugging capabilities"
            echo "  • Larger binary size (~29KB more)"
            echo "  • Requires CONFIG_DEBUG_FS=y in kernel"
            echo "  • Creates /sys/kernel/debug/$MODULE_NAME/ interface"
            ;;
    esac
    echo ""
}

build_config() {
    local config="$1"
    
    log_info "Building $MODULE_NAME in $config mode..."
    
    case "$config" in
        "release")
            make -C "$SCRIPT_DIR" clean
            make -C "$SCRIPT_DIR" no-debugfs
            ;;
        "debug")
            make -C "$SCRIPT_DIR" clean
            make -C "$SCRIPT_DIR" debugfs
            ;;
        *)
            log_warning "Unknown configuration: $config"
            return 1
            ;;
    esac
    
    if [[ $? -eq 0 ]]; then
        log_success "Build completed successfully"
        if [[ -f "$SCRIPT_DIR/$MODULE_NAME.ko" ]]; then
            local size=$(stat -c%s "$SCRIPT_DIR/$MODULE_NAME.ko")
            log_info "Module size: $size bytes"
            
            # Show whether debugfs symbols are present
            local debugfs_symbols=0
            if objdump -t "$SCRIPT_DIR/$MODULE_NAME.ko" >/dev/null 2>&1; then
                debugfs_symbols=$(objdump -t "$SCRIPT_DIR/$MODULE_NAME.ko" 2>/dev/null | grep -c "debugfs" 2>/dev/null || echo "0")
            fi
            # Ensure we have a valid number
            [[ "$debugfs_symbols" =~ ^[0-9]+$ ]] || debugfs_symbols=0
            if [[ $debugfs_symbols -gt 0 ]]; then
                log_info "Debugfs support: ENABLED ($debugfs_symbols symbols found)"
            else
                log_info "Debugfs support: DISABLED (no debugfs symbols)"
            fi
        fi
    else
        log_warning "Build failed"
        return 1
    fi
}

# Parse arguments
CONFIG="release"

if [[ $# -eq 0 ]]; then
    CONFIG="release"
elif [[ $# -eq 1 ]]; then
    case "$1" in
        release|debug)
            CONFIG="$1"
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            log_warning "Unknown option: $1"
            usage
            exit 1
            ;;
    esac
else
    log_warning "Too many arguments"
    usage
    exit 1
fi

# Show configuration information
show_config_info "$CONFIG"

# Build the configuration
build_config "$CONFIG"

# Show usage information for debug builds
if [[ "$CONFIG" == "debug" && -f "$SCRIPT_DIR/$MODULE_NAME.ko" ]]; then
    echo ""
    log_info "Debug interface usage (after loading module):"
    echo "  • View raw data: cat /sys/kernel/debug/$MODULE_NAME/raw_data"
    echo "  • View calculations: cat /sys/kernel/debug/$MODULE_NAME/calculations"
    echo "  • Install and load: sudo make install && sudo modprobe $MODULE_NAME"
fi