# Chuwi Minibook X Tools & Documentation

This repository is a collection of tools, scripts, and information related to the **Chuwi Minibook X**.  
The goal is to document quirks of this platform and provide utilities to interact with system features that are otherwise undocumented.

> ⚠️ **Warning**  
> Many of these tools interact directly with low-level system components such as the Embedded Controller (EC).  
> **Use at your own risk.** Misuse can permanently damage your device.

---

## Tools

### `n150-ec-byte-bios`

A Bash script for reading and writing a single byte in the Embedded Controller (EC) I/O map via debugfs.  
On the Minibook X (N150 platform), the EC byte at **offset `0xF0` (decimal 240)** controls BIOS options that are otherwise inaccessible.

#### Features
- Read a byte at a given EC offset
- Write a new byte value to an EC offset
- Display the full EC map via `od`
- Automatic detection and setup of the `ec_sys` kernel module
  - Ensures `write_support` is enabled
  - If the module is not loaded, attempts to load it safely with `write_support=n`
  - Reloads with `write_support=y` only when necessary
- Safety features:
  - Requires root
  - Dry-run mode (`-n`) to preview without writing
  - Explicit `--i-understand-this-could-destroy-my-ec` flag required to confirm risk
  - If omitted, shows a 5-second countdown warning before performing a write
- Restores `ec_sys` module state on exit if it was reloaded by the script

#### Requirements
- Linux kernel with `CONFIG_ACPI_EC_DEBUGFS` enabled (built-in or as a module)
- Root privileges
- `debugfs` mounted at `/sys/kernel/debug`

#### Usage

```bash
# Show help
sudo ./n150-ec-byte-bios -h

# Show EC map
sudo ./n150-ec-byte-bios -m

# Read byte at default offset (0xF0 / 240)
sudo ./n150-ec-byte-bios -r

# Write 0xAA to offset 0xF0 (requires explicit acknowledgement)
sudo ./n150-ec-byte-bios -w -t 0xAA -i

# Dry-run a write (no actual change)
sudo ./n150-ec-byte-bios -w -t 0xAA -n

## Repository Layout
- tools/ — Scripts and utilities (currently: n150-ec-byte-bios)
- docs/ — Documentation, research notes, EC maps, BIOS information
README.md — This file

## Future Work
- Expand EC reverse-engineering notes
- Collect BIOS settings and quirks
- Add more utilities for power management, display scaling, and hardware toggles

## Disclaimer
These tools are experimental and intended for advanced users.
By using them, you acknowledge the risk of permanent hardware damage.
Neither the authors nor contributors are responsible for damage to your device.
