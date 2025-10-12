# I2C Device Analysis for Chuwi MiniBook X

## Executive Summary

Analysis of I2C scan results cross-referenced with ACPI DSDT data reveals a comprehensive picture of I2C devices on the Chuwi MiniBook X. The system has 8 ACPI I2C controllers (I2C0-I2C7) which map to Linux i2c buses, with several accelerometer devices and other sensors detected.

## I2C Bus Mapping (ACPI to Linux)

- **ACPI I2C0** → **Linux i2c-12**
- **ACPI I2C1** → **Linux i2c-13**
- **ACPI I2C2** → **Linux i2c-??** (likely 14 or 15)
- **ACPI I2C3** → **Linux i2c-??** (with LTR-303 sensor)
- **ACPI I2C4-I2C7** → **Unknown mapping**

## Detected I2C Devices from Scan

### Bus 11 (i2c-11)
- **Device at 0x44**
  - Status: No driver bound
  - ACPI Match: Not found in DSDT
  - **Direct I2C Probe Results:**
    - Register 0x0F: **0x25** (device ID found!)
    - Memory pattern: `00 00 00 03 00 00 00 00 00 10 38 16 03 09 10 25`
    - Analysis: **Real sensor/controller with structured identification data**
    - **Note: This device responds to I2C but isn't defined in ACPI**

### Bus 12 (i2c-12) - ACPI I2C0
- **Device at 0x15**
  - Status: No driver bound
  - ACPI Match: **ACMG (MXC6655) individual accelerometer - BUT DISABLED**
  - Hardware ID: "MXC6655" 
  - Description: "Accelerometer"
  - **Key Finding: ACMG device _STA method returns Zero (DISABLED)**
  - **Analysis: Physical device present but ACPI device disabled**

### Bus 13 (i2c-13) - ACPI I2C1
- **Device at 0x15 UU**
  - Status: **Driver bound (UU = driver in use)**
  - ACPI Match: **ACMK (MDA6655) composite accelerometer**
  - Hardware ID: "MDA6655"
  - Description: "Accelerometer with Angle Calculation"
  - **Special: Uses BOTH I2C0 and I2C1 (dual-bus device)**

### Bus 14 (i2c-14)
- **Device at 0x5D UU**
  - Status: **Driver bound (UU = driver in use)**
  - ACPI Match: **Not found in DSDT with this address**
  - **Analysis: Likely GDIX1002 touchscreen (Goodix-TS driver) or XXXX0000 HID device**
  - **Note: Cannot probe due to driver binding, but matches ACPI device patterns**

### Bus 15 (i2c-15)
- **Device at 0x2C UU**
  - Status: **Driver bound (UU = driver in use)**
  - ACPI Match: Not found in DSDT with this address
  - **Analysis: Likely GDIX1002 touchscreen or XXXX0000 HID device (complement to 0x5D)**

- **Device at 0x33**
  - Status: No driver bound
  - ACPI Match: Not found in DSDT with this address
  - **Direct I2C Probe Results:**
    - Mostly zeros with pattern at 0x70: `c1 00 00 00 00 00 5e 00 00 00 00 00 00 01 00 06`
    - Analysis: **Sparse data pattern - likely test point, debug interface, or minimal device**

## ACPI Device Definitions

### Accelerometer Devices

#### ACMK (MDA6655) - Composite Accelerometer
- **Location**: I2C0 + I2C1 (dual-bus device)
- **Address**: 0x0015 on both buses
- **Hardware ID**: "MDA6655"
- **Description**: "Accelerometer with Angle Calculation"
- **Dependencies**: PC00.I2C0 and PC00.I2C1
- **Status**: **Active (driver bound to I2C1) - using mxc4005 driver**
- **Linux device**: i2c-MDA6655:00

#### ACMG (MXC6655) - Individual Accelerometers **[DISABLED]**
- **Location 1**: I2C0 (bus 12)
  - **Address**: 0x0015
  - **Hardware ID**: "MXC6655"
  - **Description**: "Accelerometer"
  - **Status**: **DISABLED (_STA returns Zero) - Physical device detected but ACPI disabled**

- **Location 2**: I2C2 (unmapped bus)
  - **Address**: 0x0015
  - **Hardware ID**: "MXC6655"
  - **Description**: "Accelerometer"
  - **Status**: **DISABLED (_STA returns Zero) - Not detected in scan**

### Other ACPI Devices (Found via Linux device enumeration)

#### GDIX1002 - Touchscreen
- **Hardware ID**: "GDIX1002"
- **Driver**: Goodix-TS
- **Linux device**: i2c-GDIX1002:00
- **Status**: Active with driver bound
- **Analysis**: Likely corresponds to 0x5D or 0x2C device

#### XXXX0000 - Multitouch Touchpad **[IDENTIFIED]**
- **Hardware ID**: "XXXX0000" (generic placeholder ID)
- **Compatible ID**: "PNP0C50" (HID Protocol Device for I2C bus)
- **Driver**: i2c_hid_acpi → hid-multitouch
- **Linux device**: i2c-XXXX0000:05
- **HID Product ID**: 0911:5288
- **Device Name**: "XXXX0000:05 0911:5288 Touchpad"
- **Function**: **Built-in multitouch touchpad of the MiniBook X**
- **Capabilities**: X/Y coordinates, pressure, multitouch gestures, touchpad buttons
- **Status**: Active with driver bound
- **Analysis**: This is the laptop's main touchpad - corresponds to one of the UU devices (0x5D or 0x2C)

### Other Sensors

#### LTR-303 Ambient Light Sensor
- **Location**: I2C3
- **Address**: 0x0029
- **Hardware ID**: "LTR303"
- **Description**: "LTR-303"
- **Status**: Disabled in ACPI (_STA returns 0)

## Analysis and Conclusions

### Identified Devices
1. **MDA6655 Composite Accelerometer**: Successfully detected and working (UU on i2c-13)
2. **MXC6655 Individual Accelerometer**: Physical device present on i2c-12 but ACPI disabled
3. **XXXX0000 Multitouch Touchpad**: Built-in touchpad, active with hid-multitouch driver
4. **GDIX1002 Touchscreen**: Goodix touchscreen device (if present)
5. **Unknown devices**: At addresses 0x44, 0x33 - not defined in ACPI tables

### Driver Status
- **Working**: MDA6655 on i2c-13 (the main accelerometer used by your driver)
- **Working**: XXXX0000 touchpad on i2c-14 or i2c-15 (your laptop's built-in touchpad)
- **Working**: GDIX1002 touchscreen (if present, likely external display touch)
- **Disabled**: MXC6655 devices are disabled in ACPI (_STA returns Zero)
- **Unknown active devices**: Still investigating 0x44, 0x33

### Key Findings
- **ACMG devices are disabled**: Both MXC6655 devices have _STA returning Zero
- **Physical vs ACPI**: Device at 0x15 on i2c-12 exists physically but is disabled in ACPI
- **0x5D cannot be ACMG**: ACMG uses address 0x15, not 0x5D
- **XXXX0000 identified**: Built-in multitouch touchpad using I2C-HID protocol
- **Mystery devices solved**: 0x5D and 0x2C are the touchpad (XXXX0000) and touchscreen (GDIX1002)
- **Remaining unknowns**: 0x44 (real sensor with ID 0x25) and 0x33 (sparse data pattern)

### Recommendations
1. **Bus mapping verification**: Determine exact mapping of ACPI I2C2-I2C7 to Linux buses
2. **Device identification**: Investigate devices at 0x44, 0x5D, 0x2C, 0x33
3. **Driver optimization**: Consider if additional accelerometer devices should be utilized
4. **Hardware validation**: Verify if all detected devices are intentionally present

### Key Findings
- The **MDA6655 at 0x15 on i2c-13** is your main working accelerometer
- The **XXXX0000 touchpad** (0x5D or 0x2C) is your laptop's built-in multitouch trackpad
- There's a **duplicate MXC6655 at 0x15 on i2c-12** that's unused (disabled in ACPI)
- **Device 0x44** is a real sensor with ID 0x25 but not defined in ACPI
- **Device 0x33** has minimal data, likely a test point or debug interface
- **I2C bus mapping** beyond I2C0/I2C1 needs verification

This analysis explains why your driver works with the MDA6655 device and provides insight into the additional I2C devices present on the system.