# Chuwi Minibook X Module-v2 Build System

## Overview
Unified build system for the Chuwi Minibook X kernel module with modprobe-based dependency management.

## Features

### Build System
- **Unified Makefile**: Single build file supporting both in-tree and out-of-tree builds
- **Kernel 6.17+ Compatible**: Resolved all compilation issues for modern kernels
- **Dependency Management**: Automatic loading of required kernel modules

### Module Dependencies
The module declares soft dependencies on:
- `mxc4005`: Accelerometer driver for hardware detection
- `serial_multi_instantiate`: Multi-device instantiation support

### Build Targets

#### `make all` (default)
Compiles the kernel module using the current kernel's build system.

#### `make clean`  
Removes all build artifacts and temporary files.

#### `make install`
Installs the module to the system's module directory (requires root).

#### `make load`
- Compiles the module if needed
- Copies module to temporary system location
- Loads dependencies (mxc4005, serial_multi_instantiate)  
- Loads main module with modprobe
- Shows loading status

#### `make unload`
- Unloads main module and dependencies
- Cleans up temporary system files
- Shows unload status

#### `make help`
Displays available targets and usage information.

## Usage Examples

### Development Workflow
```bash
# Build and load for testing
make load

# Check module status
lsmod | grep chuwi_minibook_x

# View module messages
sudo dmesg | tail -20

# Unload when done
make unload
```

### Dependency Verification
```bash
# Check what modprobe will load
modprobe --show-depends chuwi_minibook_x

# Manually load just dependencies
sudo modprobe mxc4005
sudo modprobe serial_multi_instantiate
```

## Module Detection Results

The module successfully:
- ✅ Loads with all dependencies via modprobe
- ✅ Detects MXC4005 accelerometer devices (found 2 devices)
- ✅ Integrates with kernel's device detection system
- ✅ Provides proper dependency metadata for modprobe

## System Integration

### Files Created/Modified
- `module-v2/Makefile`: Unified build system
- `module-v2/chuwi-minibook-x-main.c`: Added MODULE_SOFTDEP declarations
- `module-v2/.gitignore`: Build artifact exclusions

### Temporary System Files
During `make load`, the module is temporarily copied to:
- `/lib/modules/$(uname -r)/extra/chuwi-minibook-x.ko`

These files are automatically cleaned up during `make unload`.

## Technical Notes

### Dependency Loading
- `MODULE_SOFTDEP` provides hints to modprobe
- `make load` explicitly loads dependencies for reliability
- `serial_multi_instantiate` is typically built-in to kernel
- `mxc4005` loads as separate kernel module

### Hardware Detection
The module successfully detects:
- 2 MXC4005 accelerometer devices (base + lid)
- IIO subsystem integration
- Device enumeration and identification

### Build Compatibility
- Works with KERNELRELEASE-based kernel build system
- Supports both development and production scenarios
- Compatible with kernel 6.17+ API changes