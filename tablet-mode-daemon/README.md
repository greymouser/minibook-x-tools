# Hyprland Tablet Mode Integration

This directory contains a complete tablet mode integration solution for Hyprland that bridges SW_TABLET_MODE kernel events to desktop environment changes.

## Overview

The **tablet-mode-daemon** monitors hardware tablet mode events from your Chuwi Minibook X (or other 2-in-1 device) and automatically executes configurable scripts for optimal tablet or laptop usage.

### Features

- **Automatic Detection**: Monitors SW_TABLET_MODE kernel events from input devices
- **Script-Based Actions**: Execute user-customizable scripts for tablet/laptop mode transitions
- **Default Hyprland Integration**: Includes optimized Hyprland settings for touch interfaces
- **Virtual Keyboard Support**: Default scripts support onboard, squeekboard, and wvkbd
- **Debounce Protection**: Prevents rapid mode switching from sensitive hardware
- **User Session Integration**: Runs per-user with systemd --user for proper isolation
- **Security Hardened**: Runs with restricted privileges and resource access
- **Flexible Configuration**: Command line options override user config files

## Quick Start

### 1. Build and Install

```bash
# Build the daemon
make

# Install system binary and examples
sudo make install

# Set up user configuration directory
make install-user
```

### 2. Add User to Input Group

**IMPORTANT**: The daemon needs access to input devices to detect tablet mode events.
Your user must be in the `input` group:

```bash
# Add your user to the input group
sudo usermod -a -G input $USER

# Log out and log back in for the group change to take effect
```

You can verify you're in the input group with:
```bash
groups | grep -q input && echo "✓ In input group" || echo "✗ Not in input group"
```

### 3. Configure Your Scripts

```bash
# Copy example scripts to active configuration
cp ~/.config/tablet-mode/tablet-on.sh.example ~/.config/tablet-mode/tablet-on.sh
cp ~/.config/tablet-mode/tablet-off.sh.example ~/.config/tablet-mode/tablet-off.sh

# Optionally customize the daemon configuration
cp ~/.config/tablet-mode/daemon.conf.example ~/.config/tablet-mode/daemon.conf
```

### 4. Enable the Service

```bash
systemctl --user daemon-reload
systemctl --user enable --now tablet-mode-daemon
```

## Configuration

The daemon loads configuration in this order of precedence:

The daemon loads configuration in this order of precedence:
1. **Command-line specified** (`-c /path/to/config`)
2. **User config**: `~/.config/tablet-mode/daemon.conf`  
3. **Default scripts**: Built-in example scripts if no config found

### Configuration File

Copy and customize the daemon configuration:

```bash
# Copy example to active config
cp ~/.config/tablet-mode/daemon.conf.example ~/.config/tablet-mode/daemon.conf
editor ~/.config/tablet-mode/daemon.conf
```

### Script Customization

The default scripts provide optimized Hyprland integration. You can customize them for your needs:

```bash
# View the default tablet mode script
cat ~/.config/tablet-mode/tablet-on.sh.example

# Customize for your setup
cp ~/.config/tablet-mode/tablet-on.sh.example ~/.config/tablet-mode/tablet-on.sh
editor ~/.config/tablet-mode/tablet-on.sh
```

## Virtual Keyboard Setup

The default scripts support multiple virtual keyboards. Install your preference:

```bash
# Option 1: onboard (GNOME virtual keyboard)
sudo pacman -S onboard        # Arch
sudo apt install onboard      # Debian/Ubuntu

# Option 2: squeekboard (mobile-optimized)
sudo pacman -S squeekboard    # Arch  
sudo apt install squeekboard  # Debian/Ubuntu

# Option 3: wvkbd (lightweight Wayland)
sudo pacman -S wvkbd-git      # Arch AUR
```

## Testing

Test the setup manually:

```bash
# Test the daemon in foreground mode
make test

# Check if SW_TABLET_MODE events are working
# Check service logs
journalctl --user -u tablet-mode-daemon -f
```

## Configuration Files

### Daemon Configuration

The main configuration file controls daemon behavior:

**`~/.config/tablet-mode/daemon.conf`**:
```ini
# Input device (auto-detected by default)
tablet_device=/dev/input/event20

# Scripts to execute on mode changes
on_tablet_script=~/.config/tablet-mode/tablet-on.sh
on_laptop_script=~/.config/tablet-mode/tablet-off.sh

# Prevent rapid switching (milliseconds)
debounce_ms=500

# Enable debug logging
verbose=1
```

### Default Scripts

The installation provides example scripts with optimized Hyprland integration:

- **`tablet-on.sh.example`**: Tablet mode activation with virtual keyboard and touch optimizations
- **`tablet-off.sh.example`**: Laptop mode restoration with desktop settings

Copy and customize these scripts for your needs:

```bash
# View the default tablet script
cat ~/.config/tablet-mode/tablet-on.sh.example

# Customize tablet mode behavior
cp ~/.config/tablet-mode/tablet-on.sh.example ~/.config/tablet-mode/tablet-on.sh
editor ~/.config/tablet-mode/tablet-on.sh
```

## Built-in Hyprland Optimizations

The default scripts provide these Hyprland adjustments:

### Tablet Mode Activations
- **Remove gaps**: `gaps_in=0`, `gaps_out=0` for more screen space
- **Disable rounding**: `rounding=0` for cleaner touch targets  
- **Enable natural scroll**: Touch-friendly scrolling direction
- **Virtual keyboard integration**: Automatic launching with proper window rules
- **Disable shadows**: `drop_shadow=false` for performance

### Laptop Mode Restorations
- **Restore gaps**: `gaps_in=4`, `gaps_out=8` for desktop aesthetics
- **Enable rounding**: `rounding=8` for visual polish
- **Disable natural scroll**: Traditional desktop scrolling
- **Close virtual keyboards**: Clean shutdown of touch input methods
- **Enable shadows**: `drop_shadow=true` for depth

## Device Detection

Find your tablet mode device:

```bash
# List all input devices
sudo libinput list-devices | grep -i tablet

# Check kernel input devices
cat /proc/bus/input/devices | grep -A5 -B5 -i tablet

# Monitor events (replace eventXX with your device)
sudo libinput debug-events --device=/dev/input/event20
```

## Troubleshooting

### Common Issues

**Permission Denied on Input Device / Service Failed at Step GROUP**
```bash
# Add user to input group
sudo usermod -a -G input $USER
# Log out and back in

# If you see "Failed at step GROUP" in journalctl:
# This means your user is not in the input group
# The service requires input group membership to access hardware events
```

**Daemon Not Starting**
```bash
# Check service status
systemctl --user status tablet-mode-daemon

# View logs
journalctl --user -u tablet-mode-daemon
```

**Hyprctl Commands Failing**
```bash
# Test hyprctl access
hyprctl version
echo $HYPRLAND_INSTANCE_SIGNATURE

# Check Hyprland socket
ls -la $XDG_RUNTIME_DIR/hypr/
```

**Virtual Keyboard Not Appearing**
```bash
# Test manual launch
onboard &

# Check window rules
hyprctl clients | grep -i onboard
```

### Debug Mode

Run the daemon in foreground with verbose logging:

```bash
# Stop the service first
systemctl --user stop tablet-mode-daemon

# Run manually with debugging
tablet-mode-daemon -f -v

# In another terminal, test mode switching
echo 1 | sudo tee /sys/kernel/chuwi-minibook-x-tablet-mode/force
echo 0 | sudo tee /sys/kernel/chuwi-minibook-x-tablet-mode/force
```

## Integration with Other Desktop Environments

While designed for Hyprland, the daemon can be adapted for other environments:

### GNOME/KDE
These usually have built-in tablet mode support, but you can still use the daemon for custom scripts:

```ini
# Disable built-in Hyprland commands, use only custom scripts
on_tablet_script=custom-tablet-script.sh
on_laptop_script=custom-laptop-script.sh
```

### Other Tiling WMs (Sway, i3, etc.)
Modify the built-in commands in the daemon source or use custom scripts:

```ini
on_tablet_script=swaymsg gaps inner all set 0; onboard &
on_laptop_script=swaymsg gaps inner all set 10; pkill onboard
```

## Hardware Requirements

- Linux kernel with SW_TABLET_MODE support
- Input device generating SW_TABLET_MODE events
- Wayland compositor (for virtual keyboards)
- Hyprland window manager (for built-in optimizations)

For the Chuwi Minibook X, ensure you have:
- The chuwi-minibook-x-tablet-mode kernel module loaded
- The userspace feeder daemon running
- Accelerometer devices properly configured

## Security

The daemon runs with restricted privileges:
- No new privileges allowed
- Private temporary directory
- Read-only home directory access
- System directories protected
- No access to kernel tunables or modules
- Restricted to input group permissions

## Development

### Building

```bash
# Debug build
make debug

# Clean build
make clean

# Check dependencies
make check

# Show build info
make info
```

### Testing

```bash
# Test build and run
make test

# System requirement check
make check
```

### Packaging

```bash
# Create distribution tarball
make dist
```

## Related Components

This tablet mode integration works with other components in the minibook-x-tools project:

- **accelerometers/**: IIO device detection and setup
- **module-v1/module/**: Kernel module for SW_TABLET_MODE generation  
- **module-v1/module-userspace/**: Userspace feeder for accelerometer data

## License

GPL-2.0 - See project root for license details.

## Support

- **GitHub Issues**: https://github.com/greymouser/minibook-x-tools
- **Documentation**: `man tablet-mode-daemon`
- **Examples**: `/usr/local/share/doc/tablet-mode-daemon/examples/`