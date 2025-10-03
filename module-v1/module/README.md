# Chuwi Minibook X Tablet Mode Kernel Module

This is an out-of-tree Linux kernel module that provides tablet mode detection for the Chuwi Minibook X using hinge angle calculation from accelerometer data.

## Building and Installation

### Prerequisites

- Linux kernel headers for your running kernel
- GCC and make
- Root access for installation

### Quick Installation

```bash
# Build, install, and load the module
sudo ./install.sh
```

### Manual Build Process

```bash
# Build the module
make

# Install the module
sudo make install

# Load the module
sudo modprobe chuwi-minibook-x-tablet-mode
```

### Makefile Targets

- `make` or `make all` - Build the module (default, without debugfs)
- `make debugfs` - Build with debugfs interface enabled
- `make no-debugfs` - Build explicitly without debugfs support
- `make install` - Install the module to the system
- `make uninstall` - Remove the module from the system
- `make clean` - Clean build artifacts
- `make load` - Load the module into the kernel
- `make unload` - Unload the module from the kernel
- `make reload` - Reload the module (unload + load)
- `make info` - Show module information
- `make help` - Show all available targets

### Build Configuration

The module supports conditional compilation of the debugfs interface:

- **Release build (default)**: `make` or `make no-debugfs`
  - Smaller binary size (~29KB smaller)
  - No debugfs dependencies
  - Suitable for production use

- **Debug build**: `make debugfs` 
  - Includes debugfs interface for advanced debugging
  - Requires `CONFIG_DEBUG_FS=y` in kernel configuration
  - Creates files under `/sys/kernel/debug/chuwi-minibook-x-tablet-mode/`

You can also set the configuration explicitly:
```bash
# Build with debugfs support
make CONFIG_CHUWI_MINIBOOK_X_TABLET_MODE_DEBUGFS=y modules

# Build without debugfs support
make CONFIG_CHUWI_MINIBOOK_X_TABLET_MODE_DEBUGFS=n modules
```

### Installation Variables

- `DESTDIR` - Staging directory for package building
- `INSTALL_MOD_PATH` - Module installation prefix
- `KDIR` - Kernel build directory (default: `/lib/modules/$(uname -r)/build`)

### DKMS Support

This module includes DKMS configuration for automatic rebuilding across kernel updates:

```bash
# Install with DKMS
sudo dkms add .
sudo dkms build chuwi-minibook-x-tablet-mode/1.0
sudo dkms install chuwi-minibook-x-tablet-mode/1.0
```

### Usage

Once loaded, the module creates:
- `/sys/kernel/chuwi-minibook-x-tablet-mode/` - sysfs interface
- SW_TABLET_MODE input switch device

See the main project documentation for detailed usage instructions.

### Uninstallation

```bash
# Quick uninstall
sudo ./uninstall.sh

# Manual uninstall
sudo rmmod chuwi-minibook-x-tablet-mode
sudo make uninstall
```

## Development

### Testing Changes

```bash
# Rebuild and reload for testing
make clean && make && sudo make reload
```

### Module Information

```bash
# Show module details
make info

# Or after installation
modinfo chuwi-minibook-x-tablet-mode
```

### DebugFS

The debugfs interface is available when the module is built with `make debugfs`:

```bash
# View raw sensor data
cat /sys/kernel/debug/chuwi-minibook-x-tablet-mode/raw_data

# View calculation details
cat /sys/kernel/debug/chuwi-minibook-x-tablet-mode/calculations
```

**Note**: Debugfs support requires:
- Building with `make debugfs` or `CONFIG_CHUWI_MINIBOOK_X_TABLET_MODE_DEBUGFS=y`
- Kernel built with `CONFIG_DEBUG_FS=y`
- debugfs mounted (usually automatic)

## License

GPL-2.0 - See LICENSE file in the project root.

## Author

Armando DiCianno <armando@noonshy.com>