# Chuwi Minibook X Tablet Mode Daemon - Core Binary Documentation

This document describes the core binary `chuwi-minibook-x-tablet-mode` that is installed in `/usr/libexec/chuwi-minibook-x-tablet-mode/`. This binary is not meant to be called directly by users - instead, use the main command which is a wrapper script.

## Name

chuwi-minibook-x-tablet-mode - userspace feeder for Chuwi Minibook X tablet mode detection

## Synopsis

```
chuwi-minibook-x-tablet-mode [OPTIONS]
```

## Description

`chuwi-minibook-x-tablet-mode` is a userspace daemon that reads accelerometer data from IIO (Industrial I/O) devices and feeds it to the Chuwi Minibook X tablet mode detection kernel module. The program continuously polls two accelerometer devices (base and lid) and writes the scaled vector data to the kernel module's sysfs interface for hinge angle calculation and tablet mode detection.

The daemon is designed to run as a system service and automatically detects MXC4005 accelerometer devices. It provides robust error handling, signal management, and extensive configuration options for production deployment.

**Note:** This binary is typically invoked by the wrapper script installed at `/usr/sbin/chuwi-minibook-x-tablet-mode`, not directly.

## Options

### Device Configuration
- `-b, --base-device=DEV` - Specify the base (keyboard) accelerometer device name. Default: `iio:device0`. Example: `-b iio:device0`
- `-l, --lid-device=DEV` - Specify the lid (screen) accelerometer device name. Default: `iio:device1`. Example: `-l iio:device1`
- `-s, --sysfs-path=PATH` - Specify the kernel module sysfs base path. Default: `/sys/kernel/chuwi-minibook-x-tablet-mode`. Example: `-s /sys/kernel/custom-tablet-mode`

### Timing Configuration
- `-p, --poll-ms=MS` - Set the polling interval in milliseconds. Must be between 1 and 10000. Default: `100` ms. Example: `-p 50`

### Operation Modes
- `-d, --daemon` - Run in daemon mode (background operation)
- `-v, --verbose` - Enable verbose logging with detailed debug information
- `-t, --test` - Run in test mode (single poll cycle, then exit)
- `-h, --help` - Display help message and exit
- `-V, --version` - Display version information and exit

## Configuration Files

The binary reads configuration from:
1. Command line arguments (highest priority)
2. Environment variables:
   - `BASE_IIO` - Base device name
   - `LID_IIO` - Lid device name  
   - `POLL_MS` - Polling interval
   - `SYSFS_PATH` - Kernel module sysfs path

## Signal Handling

The daemon responds to the following signals:
- `SIGTERM`, `SIGINT` - Graceful shutdown
- `SIGUSR1` - Toggle verbose logging
- `SIGUSR2` - Reload configuration

## Exit Codes

- `0` - Success
- `1` - General error
- `2` - Invalid arguments
- `3` - Device access error
- `4` - Kernel module communication error
- `5` - System resource error

## Implementation Details

### Data Processing
The daemon performs the following operations:
1. Opens IIO device files for accelerometer access
2. Reads raw accelerometer data from both devices
3. Applies scaling factors to convert to proper units
4. Writes scaled vector data to kernel module sysfs files
5. Sleeps for the configured polling interval
6. Repeats until shutdown signal received

### Error Handling
- Automatic retry on transient IIO device errors
- Graceful degradation when one accelerometer fails
- Detailed error logging with context information
- Clean resource cleanup on exit

### Performance Characteristics
- Low CPU usage (typically <1% on modern systems)
- Minimal memory footprint (~1-2MB RSS)
- Configurable polling rate for power vs. responsiveness trade-offs
- No dynamic memory allocation in main loop

## Files

- `/usr/libexec/chuwi-minibook-x-tablet-mode/chuwi-minibook-x-tablet-mode` - Main binary
- `/sys/kernel/chuwi-minibook-x-tablet-mode/` - Kernel module sysfs interface
- `/sys/bus/iio/devices/iio:deviceN/` - IIO accelerometer device files

## See Also

- `chuwi-minibook-x-tablet-mode(8)` - Main wrapper command
- `systemd.service(5)` - For service configuration
- `iio(4)` - Industrial I/O subsystem

## Author

Armando DiCianno <armando@noonshy.com>

## Copyright

Copyright (c) 2025 Armando DiCianno. Licensed under GPL-2.0.