# Chuwi Minibook X Tablet Mode Userspace Feeder

This directory contains the userspace components for the Chuwi Minibook X tablet mode detection system. The feeder program reads accelerometer data from IIO devices and provides it to the kernel module for tablet mode detection.

## Components

- **`chuwi-minibook-x-tablet-mode-improved.c`** - Enhanced feeder program with robust error handling
- **`chuwi-minibook-x-tablet-mode-wrapper`** - Shell wrapper script for device detection and initialization
- **`chuwi-minibook-x-tablet-mode.service`** - Systemd service file for automatic startup
- **`Makefile`** - Comprehensive build system
- **Legacy files** (for reference):
  - `chuwi-minibook-x-tablet-mode.c` - Original simple feeder
  - `build-and-test.sh` - Original build script

## Quick Start

### Building and Installation

```bash
# Build the program
make

# Install system-wide (requires root)
sudo make install

# Enable and start the service
sudo systemctl enable chuwi-minibook-x-tablet-mode
sudo systemctl start chuwi-minibook-x-tablet-mode
```

### Manual Testing

```bash
# Build and run with default settings
make run

# Run with custom devices
./chuwi-minibook-x-tablet-mode -b iio:device0 -l iio:device1 -v

# Show help
./chuwi-minibook-x-tablet-mode --help
```

## Documentation

Comprehensive manual pages are available for both components:

```bash
# View main program documentation
man chuwi-minibook-x-tablet-mode

# View wrapper script documentation  
man chuwi-minibook-x-tablet-mode-wrapper

# Preview man pages before installation
make man              # View main program man page
make man-wrapper      # View wrapper script man page
make man-all          # View both man pages
```

## Configuration

### Automatic Device Detection

The wrapper script automatically detects MXC4005 accelerometer devices. If you need to specify devices manually, create a configuration file:

```bash
sudo nano /etc/default/chuwi-minibook-x-tablet-mode
```

Add your configuration:
```bash
# Base (keyboard) accelerometer device
BASE_IIO=iio:device0

# Lid (screen) accelerometer device  
LID_IIO=iio:device1

# Polling interval in milliseconds
POLL_MS=100
```

### Command Line Options

The improved feeder supports extensive configuration:

```
Options:
  -b, --base-device DEV    Base accelerometer device (default: iio:device0)
  -l, --lid-device DEV     Lid accelerometer device (default: iio:device1)
  -p, --poll-ms MS         Polling interval in milliseconds (default: 100)
  -s, --sysfs-path PATH    Kernel module sysfs path
  -d, --daemon             Run as daemon
  -v, --verbose            Verbose logging
  -h, --help               Show help
  -V, --version            Show version
```

## System Integration

### Systemd Service

The service is designed to:
- Start automatically after the kernel module is loaded
- Wait for IIO devices to be available
- Run with minimal privileges (`nobody` user)
- Restart automatically on failure
- Apply security restrictions

Check service status:
```bash
# Status
sudo systemctl status chuwi-minibook-x-tablet-mode

# Logs
sudo journalctl -u chuwi-minibook-x-tablet-mode -f

# Restart
sudo systemctl restart chuwi-minibook-x-tablet-mode
```

### Security Features

The improved implementation includes:
- Input validation and bounds checking
- Proper resource management (no file handle leaks)
- Signal handling for graceful shutdown
- Error recovery with exponential backoff
- Systemd security hardening

## Troubleshooting

### Device Detection Issues

1. **Check available IIO devices:**
   ```bash
   ls /sys/bus/iio/devices/
   for dev in /sys/bus/iio/devices/iio:device*/name; do
       echo "$dev: $(cat "$dev" 2>/dev/null || echo 'unreadable')"
   done
   ```

2. **Verify accelerometer data:**
   ```bash
   cat /sys/bus/iio/devices/iio:device0/in_accel_x_raw
   cat /sys/bus/iio/devices/iio:device0/in_accel_y_raw
   cat /sys/bus/iio/devices/iio:device0/in_accel_z_raw
   ```

3. **Check kernel module sysfs:**
   ```bash
   ls -la /sys/kernel/chuwi-minibook-x-tablet-mode/
   ```

### Common Problems

**Problem:** Service fails to start
- **Solution:** Check that the kernel module is loaded and IIO devices exist

**Problem:** No tablet mode events
- **Solution:** Verify accelerometer data is changing and being written to kernel module

**Problem:** High CPU usage
- **Solution:** Increase polling interval with `-p` option or in config file

### Debug Mode

Run in foreground with verbose logging:
```bash
sudo systemctl stop chuwi-minibook-x-tablet-mode
sudo ./chuwi-minibook-x-tablet-mode -v
```

## Development

### Building Debug Version

```bash
# Debug build
make debug

# Run static analysis
make analyze

# Run tests and checks
make test
make check
```

### Code Quality

The improved code includes:
- Comprehensive error handling
- Memory safety (no buffer overflows)
- Resource leak prevention
- Signal handling for clean shutdown
- Detailed logging with timestamps
- Input validation and sanitization

### Performance Considerations

- Default 100ms polling provides good responsiveness
- File operations are optimized with proper buffering
- Error recovery prevents crash loops
- Scale factors are cached to avoid repeated file reads

## Comparison with Original

| Feature | Original | Improved |
|---------|----------|----------|
| Lines of code | 101 | 500+ |
| Error handling | Minimal | Comprehensive |
| Security | None | Hardened |
| Configuration | Command line only | Config file + CLI |
| Logging | printf only | Structured logging |
| Signal handling | None | Full support |
| Input validation | None | Complete |
| Memory safety | Risky | Safe |
| Build system | Simple script | Full Makefile |
| Service integration | None | Systemd service |

## Installation Paths

Default installation paths (with `PREFIX=/usr`):
- Binary: `/usr/sbin/chuwi-minibook-x-tablet-mode`
- Wrapper: `/usr/sbin/chuwi-minibook-x-tablet-mode-wrapper`
- Config: `/etc/default/chuwi-minibook-x-tablet-mode`
- Service: `/usr/lib/systemd/system/chuwi-minibook-x-tablet-mode.service`

## License

GPL-2.0 - See the main project LICENSE file.

## Author

Armando DiCianno <armando@noonshy.com>