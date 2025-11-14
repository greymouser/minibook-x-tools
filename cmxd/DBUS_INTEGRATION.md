# DBus Integration for cmxd

## Overview

cmxd now supports optional DBus integration alongside its existing Unix socket interface. This provides compatibility with desktop environments expecting standard tablet mode and orientation signals.

## Features

### Compile-time Configuration
- **Default**: DBus support enabled (`ENABLE_DBUS=1`)
- **Disable**: Build with `make ENABLE_DBUS=0` 
- **Dependencies**: Automatically detected via pkg-config

### Runtime Control
- **Default**: DBus enabled if compiled in
- **Disable**: Use `--no-dbus` command-line option
- **Strict mode**: Daemon exits if DBus initialization fails (security policies)

### DBus Interfaces

#### 1. iio-sensor-proxy Compatibility (`net.hadess.SensorProxy`)
Provides GNOME desktop environment compatibility:
- **Service**: `net.hadess.SensorProxy`
- **Object**: `/net/hadess/SensorProxy`
- **Properties**:
  - `AccelerometerOrientation` (string): `normal`, `left-up`, `right-up`, `bottom-up`
  - `HasAccelerometer` (boolean): Always `true`
- **Signals**:
  - `PropertiesChanged`: Emitted on orientation changes

#### 2. Custom Tablet Mode Interface (`com.noonshy.TabletMode1`)
Dedicated tablet mode detection:
- **Service**: `com.noonshy.cmxd`
- **Object**: `/com/noonshy/TabletMode1`  
- **Properties**:
  - `TabletMode` (string): `laptop`, `tablet`, `tent`, `flat`, `closing`, `indeterminate`
  - `HingeAngle` (double): Current hinge angle in degrees
- **Signals**:
  - `PropertiesChanged`: Emitted on mode/angle changes

## System Integration

### Bus Selection
- **Primary**: System bus (requires proper DBus policies)
- **Fallback**: Session bus (for user-level testing)
- **Security**: Respects DBus security policies

### Error Handling
- **Strict mode**: Exit on DBus registration failure (default)
- **User control**: `--no-dbus` continues without DBus
- **Graceful degradation**: Unix socket always available

## Usage Examples

### Standard Operation
```bash
# Enable DBus (default)
sudo ./cmxd -v

# Disable DBus
sudo ./cmxd --no-dbus -v
```

### Build Options
```bash
# Build with DBus (default)
make

# Build without DBus support  
make ENABLE_DBUS=0

# Show build options
make help
```

### Monitoring DBus Events
```bash
# Monitor orientation changes (GNOME compatible)
dbus-monitor --system "type='signal',interface='org.freedesktop.DBus.Properties',member='PropertiesChanged',path='/net/hadess/SensorProxy'"

# Monitor tablet mode changes (custom interface)
dbus-monitor --system "type='signal',interface='org.freedesktop.DBus.Properties',member='PropertiesChanged',path='/com/noonshy/TabletMode1'"
```

### Reading Properties
```bash
# Get current orientation (GNOME compatible)
dbus-send --system --print-reply --dest=net.hadess.SensorProxy \
  /net/hadess/SensorProxy org.freedesktop.DBus.Properties.Get \
  string:net.hadess.SensorProxy string:AccelerometerOrientation

# Get current tablet mode
dbus-send --system --print-reply --dest=com.noonshy.cmxd \
  /com/noonshy/TabletMode1 org.freedesktop.DBus.Properties.Get \
  string:com.noonshy.TabletMode1 string:TabletMode
```

## Security Requirements

### DBus Policies
For system bus operation, create `/etc/dbus-1/system.d/cmxd.conf`:
```xml
<!DOCTYPE busconfig PUBLIC "-//freedesktop//DTD D-BUS Bus Configuration 1.0//EN"
 "http://www.freedesktop.org/standards/dbus/1.0/busconfig.dtd">
<busconfig>
  <policy user="root">
    <allow own="com.noonshy.cmxd"/>
    <allow own="net.hadess.SensorProxy"/>
    <allow send_destination="com.noonshy.cmxd"/>
    <allow send_destination="net.hadess.SensorProxy"/>
  </policy>
  <policy context="default">
    <allow send_destination="com.noonshy.cmxd"/>
    <allow send_destination="net.hadess.SensorProxy"/>
  </policy>
</busconfig>
```

## Desktop Environment Integration

### GNOME
- Automatic rotation based on `AccelerometerOrientation`
- Shell UI adapts to tablet mode via orientation signals
- Compatible with existing sensor-proxy expectations

### KDE/Plasma
- Custom integration possible via `com.noonshy.TabletMode1`
- Direct access to hinge angle and mode information
- Scriptable via `qdbus` or similar tools

### Custom Integration
- Unix socket interface remains available
- JSON events: `{"type":"mode","value":"tablet"}` 
- Real-time hinge angle monitoring
- Socket path: `/run/cmxd/events.sock`

## Architecture

### Event Flow
```
Hardware → cmxd calculations → Dual publishing:
                              ├─ Unix socket (JSON)
                              └─ DBus signals (Properties)
```

### Module Integration
- `cmxd-dbus.c`: Core DBus implementation
- `cmxd-events.c`: Dual event publishing
- `cmxd.c`: Command-line integration
- Conditional compilation throughout

## Troubleshooting

### Common Issues

1. **Permission Denied on System Bus**
   - Install proper DBus policy file
   - Or use `--no-dbus` to continue without DBus

2. **Build Errors - DBus Not Found**
   - Install `libdbus-1-dev` (Debian/Ubuntu)
   - Or build with `ENABLE_DBUS=0`

3. **Services Not Visible**
   - Check DBus policy configuration
   - Verify daemon running as appropriate user
   - Try session bus for testing

### Debug Commands
```bash
# Check DBus policy
dbus-send --system --print-reply --dest=org.freedesktop.DBus \
  / org.freedesktop.DBus.ListNames

# Verify service registration
busctl --system list | grep -E "(cmxd|SensorProxy)"

# Test with session bus
sudo -u $USER ./cmxd --no-dbus -v  # Force session bus usage
```

## Development Notes

### Key Implementation Details
- Low-level libdbus API for minimal overhead
- Proper PropertiesChanged signal format
- Thread-safe event broadcasting
- System/session bus fallback logic
- Conditional compilation preserves no-DBus builds

### Future Enhancements
- Systemd service with DBus activation
- Additional desktop environment integrations
- Configuration file support for DBus settings