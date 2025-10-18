# Chuwi Minibook X Daemon

This userspace daemon provides accelerometer data to the tablet mode detection kernel module. It is a unified daemon that includes auto-detection, configuration loading, and device management capabilities.

## Purpose

The daemon bridges the gap between:
- **IIO accelerometer devices** with mount matrix transformations
- **Kernel module** expecting pre-transformed gravity vector data

## Features

1. **Unified Design**: Single executable with integrated functionality (no wrapper scripts)
2. **Auto-Detection**: Automatically detects MXC4005 accelerometer devices
3. **Configuration File Support**: Loads settings from `/etc/default/chuwi-minibook-x-daemon`
4. **Device Waiting**: Waits for kernel module and IIO devices to be ready
5. **Mount Matrix Support**: Works with mount matrix coordinate transformations
6. **Event-Driven Communication**: Writes to kernel sysfs trigger immediate processing

## Operation

```
IIO Devices → Mount Matrix Transform → Standard Coordinates → Daemon → Kernel Module
                                                             ↓
                                                    Tablet Mode Events
```

1. **Reads** accelerometer data from `/sys/bus/iio/devices/iio:device*`
2. **Benefits** from automatic mount matrix coordinate transformation
3. **Writes** transformed gravity vectors to kernel sysfs interface
4. **Triggers** immediate tablet mode evaluation in kernel (event-driven)

## Configuration

The daemon will be configured to work with the standard IIO device paths:
- `iio:device0` - First accelerometer (typically lid)
- `iio:device1` - Second accelerometer (typically base)

Mount matrices are automatically applied by the kernel IIO subsystem, so the daemon receives pre-corrected coordinates.

## Build and Installation

```bash
make
sudo make install
sudo systemctl enable chuwi-minibook-x-tablet-mode
sudo systemctl start chuwi-minibook-x-tablet-mode
```

## Integration with v3 Module

This daemon is specifically designed to work with the integrated v3 kernel module (`chuwi-minibook-x-integrated.ko`). The kernel module:

1. **Instantiates** I2C devices with proper mount matrices
2. **Provides** sysfs interface for receiving gravity data
3. **Performs** tablet mode detection using event-driven delayed work
4. **Reports** tablet mode changes via Linux input events

## Event-Driven Advantages

- **Immediate Response**: Kernel processing triggered instantly when new data arrives
- **Reduced CPU Usage**: No unnecessary polling in either userspace or kernel
- **Better Battery Life**: Only processes data when accelerometer readings change
- **Lower Latency**: Direct sysfs write → delayed work → input events

## Status Monitoring

Check daemon status:
```bash
systemctl status chuwi-minibook-x-tablet-mode
```

Monitor tablet mode events:
```bash
# Watch tablet mode mode attribute
cat /sys/devices/platform/chuwi-minibook-x/mode

# Monitor input events
evtest /dev/input/eventX  # Find correct event device
```

## Troubleshooting

1. **Verify IIO devices**: `ls /sys/bus/iio/devices/`
2. **Check mount matrices**: `cat /sys/bus/iio/devices/iio:device*/in_accel_mount_matrix`
3. **Verify kernel module**: `lsmod | grep chuwi`
4. **Check sysfs interface**: `ls /sys/devices/platform/chuwi-minibook-x/`

## Development Notes

This daemon represents the userspace component of a complete tablet mode detection system. The integration with mount matrix support means it handles significantly cleaner, pre-transformed coordinate data compared to the original v1 approach.

The event-driven architecture ensures optimal performance while maintaining full compatibility with Linux desktop environments expecting standard SW_TABLET_MODE input events.