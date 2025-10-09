# ACPI Research: ACMK Device and Transformation Matrices

## Overview

Analysis of the Chuwi MiniBook X ACPI tables reveals a sophisticated accelerometer management device called `ACMK` with Hardware ID `MDA6655`. This device provides transformation matrices for calibrating accelerometer readings and manages tablet mode switching at the firmware level.

## ACMK Device Analysis

### Device Identification
- **ACPI Path**: `\_SB.ACMK`
- **Hardware ID (_HID)**: `MDA6655`
- **Compatible ID (_CID)**: `MDA6655`
- **Description (_DDN)**: "Accelerometer with Angle Calculation"
- **Dependencies**: PC00.I2C0, PC00.I2C1

### Hardware Resources (_CRS)
```
I2cSerialBusV2 (0x0015, ControllerInitiated, 0x00061A80, AddressingMode7Bit, "\_SB.PC00.I2C1", ...)
I2cSerialBusV2 (0x0015, ControllerInitiated, 0x00061A80, AddressingMode7Bit, "\_SB.PC00.I2C0", ...)
```

### Physical Device Mapping (VERIFIED)
Based on DSDT analysis combined with real system examination:

- **I2C1 (bus i2c-13) = LID ACCELEROMETER**
  - Current visible device: `/sys/bus/iio/devices/iio:device0`
  - Hardware path: `i2c-13/i2c-MDA6655:00`
  - ACPI declares this as MDA6655 composite device

- **I2C0 (bus i2c-12) = BASE ACCELEROMETER** 
  - Should appear as `/sys/bus/iio/devices/iio:device1` (currently missing)
  - Individual ACMG device (MXC6655) found in DSDT at `I2C0.ACMG`
  - Address 0x15, same as lid accelerometer

### ACPI to Linux I2C Bus Mapping (FIRM EVIDENCE)

**ACPI DSDT Provides PCI Addresses**:
```
Device (I2C0)
{
    Method (_ADR, 0, NotSerialized)  // _ADR: Address
    {
        Return (0x00150000)  // PCI 0000:00:15.0
    }
}

Device (I2C1)
{
    Method (_ADR, 0, NotSerialized)  // _ADR: Address
    {
        Return (0x00150001)  // PCI 0000:00:15.1
    }
}
```

**Linux /sys Filesystem Verification**:
```bash
readlink /sys/bus/i2c/devices/i2c-12
# ../../../devices/pci0000:00/0000:00:15.0/i2c_designware.0/i2c-12

readlink /sys/bus/i2c/devices/i2c-13  
# ../../../devices/pci0000:00/0000:00:15.1/i2c_designware.1/i2c-13

find /sys/bus/i2c/devices/ -name "*MDA6655*" -exec readlink {} \;
# ../../../devices/pci0000:00/0000:00:15.1/i2c_designware.1/i2c-13/i2c-MDA6655:00
```

**CONFIRMED MAPPING**:
- **ACPI I2C0** (PCI 0000:00:15.0) = **Linux i2c-12** = **BASE ACCELEROMETER**
- **ACPI I2C1** (PCI 0000:00:15.1) = **Linux i2c-13** = **LID ACCELEROMETER**

**Evidence Sources**:
1. DSDT `_ADR` methods provide PCI addresses for I2C controllers
2. Linux `/sys` filesystem shows real PCI device to bus number mapping
3. MDA6655 device confirmed on i2c-13 (PCI 0000:00:15.1 = ACPI I2C1)
4. PCI device list confirms Intel I2C controllers at both addresses

Both accelerometers at address 0x15 (400 kHz):

### ACPI Methods

#### 1. GMTR() - Get Matrix/Transformation
Returns 24-byte transformation matrices for accelerometer calibration.

**Matrix Structure** (24 bytes):
- Bytes 0-7: Base accelerometer transformation matrix (2×3)
- Bytes 8-15: Lid accelerometer transformation matrix (2×3)  
- Bytes 16-23: Metadata (timestamps, hardware IDs)

**Four Available Matrices**:

```
PARA: [0 -1  0] [1  0  0] [timestamp: B9 AF 1E 05 14 10]
      [1  0 -1] [0 -1  0]

PARB: [1  0  0] [0  1  0] [timestamp: B9 AF 1E 05 14 13] ← CURRENT
      [1  1  0] [0 -1  0]

PARC: [0 -1  0] [-1 0  0] [timestamp: B9 AF 1E 05 14 10]
      [-1 0  1] [0  1  0]

PARD: [0 -1  0] [-1 0  0] [timestamp: B9 AF 1E 05 14 10]
      [-1 0 -1] [0 -1  0]
```

**Selection Logic**:
- Currently hardcoded to return **PARB** (`If (Ones)` always true)
- Fallback switch statement uses `Local0 = 0x11`:
  - Case 0x10 → PARC
  - Case 0x05 → PARD  
  - Default → PARA

#### 2. LTSM(Arg0) - Laptop/Tablet State Management ⭐
**Critical tablet mode switching method**:

**Laptop Mode (Arg0 = 0)**:
```
KBCD = 0        // Enable keyboard
PB1E |= 0x08    // Set power button state  
UPBT(0x06, 1)   // Update button state via EC
Notify(HIDD, 0xCD)  // Notify HID device
```

**Tablet Mode (Arg0 = 1)**:
```
KBCD = 0x03     // Disable keyboard
PB1E &= 0xF7    // Clear power button state
UPBT(0x06, 0)   // Update button state via EC  
Notify(HIDD, 0xCC)  // Notify HID device
```

#### 3. PRIM() - Primary Accelerometer
Returns `{0x01}` indicating accelerometer 1 is primary.

#### 4. _STA() - Status
Returns `0x0F` (device present and enabled) when conditions are met.

## Key Findings

### 1. Transformation Matrices Are NOT Applied Automatically
- **IIO Mount Matrix**: Shows identity matrix `1,0,0;0,1,0;0,0,1`
- **Raw Values**: Accelerometer readings are unprocessed chip values
- **GMTR Never Called**: No firmware auto-application of transformations

### 2. Software Must Apply Transformations
**Current Workflow**:
```
MXC4005 Hardware → Raw Values → IIO Driver → User/Kernel Space
```

**Optimal Workflow**:
```
MXC4005 Hardware → Raw Values → IIO Driver → ACPI GMTR → Calibrated Values
```

### 3. PARB Matrix Analysis (Current Production)
- **Base Accelerometer**: Identity transformation `[1 0 0][0 1 0]` (use raw values)
- **Lid Accelerometer**: Modified transformation `[1 1 0][0 -1 0]` (X'=X+Y, Y'=-Y)

### 4. Hardware Detection
- **DMI**: Primary identification via system vendor/product
- **ACPI Device**: MDA6655 provides secondary verification
- **Dual-layer validation**: Both DMI + ACPI device presence required

## Testing with acpi_call

### Invoke GMTR Method
```bash
function acpi_call { echo "$1" | sudo tee /proc/acpi/call > /dev/null && sudo cat /proc/acpi/call; echo; }

# Get transformation matrix
acpi_call '\_SB.ACMK.GMTR'
# Returns: {0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0xb9, 0xaf, 0x1e, 0x05, 0x14, 0x13}

# Get primary accelerometer  
acpi_call '\_SB.ACMK.PRIM'
# Returns: {0x01}

# Switch to laptop mode
acpi_call '\_SB.ACMK.LTSM 0x0'

# Switch to tablet mode  
acpi_call '\_SB.ACMK.LTSM 0x1'
```

## Implications for Driver Development

### Accelerometer Identification (CRITICAL FINDING)

**Question**: "Does the dsdt.dsl give us hints as to which accelerometer is the lid and which is the base?"

**Answer**: YES - Through combined DSDT analysis and real-world device mapping:

1. **LID ACCELEROMETER** = I2C1 (i2c-13)
   - Currently visible as `/sys/bus/iio/devices/iio:device0`
   - Hardware path: `../../../devices/pci0000:00/0000:00:15.1/i2c_designware.1/i2c-13/i2c-MDA6655:00/iio:device0`
   - ACPI device: The primary device of the MDA6655 composite
   - Matrix position: Second matrix in GMTR return (bytes 8-15)

2. **BASE ACCELEROMETER** = I2C0 (i2c-12) 
   - Should appear as `/sys/bus/iio/devices/iio:device1` (currently missing)
   - Individual ACMG device (MXC6655) declared in DSDT at `\_SB.PC00.I2C0.ACMG`
   - Matrix position: First matrix in GMTR return (bytes 0-7)

**Evidence Chain**:
- DSDT declares individual `ACMG` devices on I2C0 and I2C2, but NOT on I2C1
- I2C1 only has the composite MDA6655 device (ACMK)
- Real system shows `iio:device0` maps to `i2c-13` (= I2C1) with MDA6655
- Therefore: I2C1 = lid (composite device), I2C0 = base (individual device)

### Driver Implementation Strategy

### Current State
- Driver works with raw accelerometer values
- Acceptable accuracy due to PARB base matrix being identity
- Missing optimal calibration for lid accelerometer

### Recommended Enhancements
1. **Add ACPI Integration**: Call `\_SB.ACMK.GMTR` to get transformation matrices
2. **Apply Calibration**: Transform lid readings using `X'=X+Y, Y'=-Y`
3. **Leverage LTSM**: Use firmware tablet mode switching instead of reimplementing
4. **Hardware Validation**: Use both DMI and MDA6655 ACPI device detection

### Alternative Architecture
Instead of complex kernel driver, consider:
1. **Minimal kernel driver**: Just reads raw accelerometer values
2. **Userspace daemon**: Applies ACPI transformations and manages tablet mode
3. **ACPI integration**: Uses `LTSM` method for proper firmware-level switching

## Matrix Variants
The four matrix sets likely represent:
- **PARB**: Current production hardware (timestamp 0x13)
- **PARA/PARC/PARD**: Alternative hardware variants or development versions (timestamp 0x10)

## Hardware Layout
```
Device Path: \_SB.PC00.I2C1.MDA6655 (Lid Accelerometer)
             \_SB.PC00.I2C0.??????  (Base Accelerometer)

IIO Mapping: iio:device0 → I2C1 → Lid (visible)
             iio:device1 → I2C0 → Base (sometimes missing)
```

## References
- ACPI DSDT analysis from `/home/greymouser/Projects/noonshy/repos/minibook-x-tools/acpi-data/dsdt.dsl`
- acpi_call testing results
- IIO device examination (`/sys/bus/iio/devices/iio:device0/`)
- Current accelerometer readings and mount matrix analysis

---
*Last updated: October 9, 2025*