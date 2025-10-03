# MDA6655 Split Tool

This tool automatically creates the second MXC4005 accelerometer device on Chuwi Minibook X systems where the ACPI/BIOS only exposes one accelerometer device instead of the required two for tablet mode detection.

## Overview

The Chuwi Minibook X has two MXC4005 accelerometer devices (one in the base, one in the lid) that are needed for proper hinge angle calculation and tablet mode detection. However, the system's ACPI/BIOS only creates one device automatically. This tool creates the missing second device by directly interfacing with the I2C subsystem.

## Components

- **`mda6655-split`** - Main script that creates missing accelerometer devices
- **`99-mxc4005-second.rules`** - Udev rules for automatic device creation
- **`Makefile`** - Build and installation system

## Installation

### Quick Install

```bash
# Build and install
make install

# Reload udev rules (requires root)
sudo make reload-udev
```

### Manual Installation

```bash
# Install to /usr (default)
sudo make install

# Install to custom prefix
sudo make install PREFIX=/usr/local

# Install to staging directory for packaging
make install DESTDIR=/tmp/staging PREFIX=/usr
```

## Usage

### Automatic Operation

The tool is designed to run automatically via udev rules when:
- The ACPI device `MDA6655:00` is detected
- I2C adapters are added to the system

### Manual Operation

```bash
# Run normally
sudo mda6655-split

# Run with verbose output
sudo mda6655-split --verbose

# Test what would happen without making changes
sudo mda6655-split --dry-run

# Show help
mda6655-split --help
```

## How It Works

1. **ACPI Path Detection**: Scans for I2C controllers at known ACPI paths:
   - `\_SB_.PC00.I2C0`
   - `\_SB_.PC00.I2C2`

2. **Device Detection**: Checks if MXC4005 devices already exist on each I2C bus

3. **Device Creation**: Creates missing devices by writing to the I2C subsystem:
   ```
   echo "mxc4005 0x15" > /sys/bus/i2c/devices/i2c-N/new_device
   ```

## Configuration

The tool can be configured through `/etc/default/mda6655-split`. This file is installed automatically and contains commented examples of all available options.

### Configuration File Options

- **`ADDR_HEX`**: I2C address for the MXC4005 device (default: `0x15`)
- **`CTRL_ACPI_PATHS`**: Array of ACPI paths for I2C controllers (default: `\_SB_.PC00.I2C0` `\_SB_.PC00.I2C2`)
- **`DRIVER`**: Kernel driver name (default: `mxc4005`)

### Customizing for Your System

1. **Check I2C address**: Use `i2cdetect` to verify the device address on your system
2. **Find ACPI paths**: Check your system's I2C controller paths:
   ```bash
   find /sys/bus/i2c/devices/i2c-*/firmware_node/path -exec cat {} \;
   ```
3. **Edit configuration**: Uncomment and modify values in `/etc/default/mda6655-split`:
   ```bash
   # Example: Different ACPI paths
   CTRL_ACPI_PATHS=( "\_SB_.PCI0.I2C0" "\_SB_.PCI0.I2C2" )
   
   # Example: Different I2C address  
   ADDR_HEX="0x16"
   ```

### Legacy Configuration

For systems where editing the configuration file is not practical, the script can still be modified directly. The default values in the script will be used if no configuration file exists.

## Troubleshooting

### Check Current Devices

```bash
# List all IIO devices
for n in /sys/bus/iio/devices/iio:device*/name; do 
    printf "%s: " "$n"; cat "$n"; 
done

# Check for MXC4005 devices specifically
find /sys/bus/iio/devices -name name -exec grep -l mxc4005 {} \;
```

### Manual Testing

```bash
# Run with verbose output to see what's happening
sudo mda6655-split --verbose

# Check kernel messages
dmesg | grep -i mxc4005

# Verify udev rules are active
udevadm control --reload-rules
udevadm test /sys/devices/LNXSYSTM:00/LNXSYSTM:01/MDA6655:00
```

### Common Issues

1. **Permission Denied**: Ensure you're running as root when creating devices
2. **Device Already Exists**: The script safely skips existing devices
3. **No I2C Controllers Found**: Verify ACPI paths match your system
4. **Udev Rules Not Working**: Check that the rules file is properly installed and permissions are correct

## Integration

This tool integrates with the tablet mode detection system:

1. **Accelerometer Detection**: Creates the missing accelerometer devices
2. **Tablet Mode Module**: The kernel module reads from both accelerometers
3. **Userspace Feeder**: The userspace component feeds data to the kernel module

## Development

### Building

```bash
# Process files for current prefix
make

# Clean build artifacts
make clean

# Show tool information
make info
```

### Testing

```bash
# Test installation
make test

# Create distribution package
make dist
```

## License

GPL-2.0 - See project root for license details.

## See Also

- Chuwi Minibook X tablet mode kernel module
- Userspace feeder component
- IIO (Industrial I/O) subsystem documentation