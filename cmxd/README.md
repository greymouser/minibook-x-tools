# CMXD - Chuwi Minibook X Daemon (New Generation)

CMXD is the new generation userspace daemon for the Chuwi Minibook X tablet mode detection system. It reads accelerometer data from IIO devices and feeds it to the tablet mode detection kernel module.

## Features

- **Event-driven IIO processing**: Uses IIO buffer interface for efficient accelerometer data reading
- **Robust setup and cleanup**: Comprehensive initialization and safe shutdown procedures
- **Automatic device detection**: Reads device assignments from the kernel module
- **Safety-first approach**: Always restores laptop mode on exit to prevent lockout
- **Configurable**: Support for configuration files and command-line options
- **Verbose debugging**: Optional detailed logging for troubleshooting

## Building

```bash
make                    # Build the daemon
make debug             # Build with debug symbols
make install           # Install to system
make install-systemd   # Install with systemd service
```

## Usage

```bash
cmxd                   # Run with defaults
cmxd -v               # Run with verbose logging
cmxd -t 50 -v         # Custom timeout with verbose logging
cmxd -d               # Run as daemon
```

## Command Line Options

- `-t, --timeout-ms MS`: Buffer read timeout in milliseconds (default: 100)
- `-s, --sysfs-path PATH`: Kernel module sysfs path 
- `-d, --daemon`: Run as daemon
- `-v, --verbose`: Verbose logging (shows all debug information)
- `-h, --help`: Show help
- `-V, --version`: Show version

## Configuration

Configuration can be placed in `/etc/default/cmxd`:

```
# Buffer read timeout in milliseconds
BUFFER_TIMEOUT_MS=100

# Kernel module sysfs path
SYSFS_DIR=/sys/devices/platform/chuwi-minibook-x
```

## Architecture

This daemon is designed as a solid foundation with excellent setup/cleanup code and IIO event processing. The mode detection logic will be added in future iterations. Key components:

- **IIO Buffer Management**: Complete setup/teardown of IIO buffers and triggers
- **Device Discovery**: Automatic detection of accelerometer devices from kernel module
- **Signal Handling**: Proper signal handling for clean shutdown
- **Error Recovery**: Robust error handling and recovery mechanisms
- **Safety Mechanisms**: Failsafe cleanup to prevent getting locked in tablet mode

## Next Steps

Future development will focus on:
1. Improved hinge angle calculation algorithms
2. Enhanced mode detection logic (closing ↔ laptop ↔ flat ↔ tent ↔ tablet)
3. Better orientation detection with tablet mode reading protection
4. Adaptive filtering and hysteresis algorithms

## Copyright

Copyright (c) 2025 Armando DiCianno <armando@noonshy.com>
Licensed under GPL-2.0