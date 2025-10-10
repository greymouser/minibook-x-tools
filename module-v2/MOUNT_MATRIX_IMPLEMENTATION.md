# Mount Matrix Implementation - Complete Code Changes

## Overview

This document shows the complete implementation of mount matrix support for the Chuwi Minibook X platform driver, enabling automatic coordinate transformation for both accelerometer sensors.

## Code Changes Made

### 1. Added Required Includes

```c
#include <linux/property.h>  // Added for PROPERTY_ENTRY_STRING_ARRAY
```

### 2. Added Mount Matrix Definitions

```c
bool enable_mount_matrix = true;
module_param(enable_mount_matrix, bool, 0644);
MODULE_PARM_DESC(enable_mount_matrix, "Enable automatic mount matrix transformation (default: true)");

/* Mount Matrix Definitions */

/* Lid sensor mount matrix (90° CCW rotation) */
static const char * const lid_sensor_mount_matrix[] = {
    "0", "1", "0",    /* X' = Y  (laptop right = sensor back)  */
    "-1", "0", "0",   /* Y' = -X (laptop back = sensor left)   */
    "0", "0", "1"     /* Z' = Z  (laptop up = sensor up)       */
};

/* Base sensor mount matrix (90° CW rotation) */  
static const char * const base_sensor_mount_matrix[] = {
    "0", "-1", "0",   /* X' = -Y (laptop right = sensor front) */
    "1", "0", "0",    /* Y' = X  (laptop back = sensor right)  */
    "0", "0", "1"     /* Z' = Z  (laptop up = sensor up)       */
};

/* Property entries for sensors */
static const struct property_entry lid_sensor_props[] = {
    PROPERTY_ENTRY_STRING_ARRAY("mount-matrix", lid_sensor_mount_matrix),
    { }
};

static const struct property_entry base_sensor_props[] = {
    PROPERTY_ENTRY_STRING_ARRAY("mount-matrix", base_sensor_mount_matrix),
    { }
};

/* Software nodes for device property assignment */
static const struct software_node lid_sensor_node = {
    .properties = lid_sensor_props,
};

static const struct software_node base_sensor_node = {
    .properties = base_sensor_props,
};
```

### 3. Updated Function Signature

```c
// Old:
static int chuwi_minibook_x_instantiate_mxc4005(int bus_nr, int addr, bool is_second)

// New:
static int chuwi_minibook_x_instantiate_mxc4005(int bus_nr, int addr, bool is_second, bool is_lid)
```

### 4. Enhanced Device Instantiation Logic

```c
static int chuwi_minibook_x_instantiate_mxc4005(int bus_nr, int addr, bool is_second, bool is_lid)
{
    struct i2c_adapter *adapter;
    struct i2c_board_info info = {};
    struct i2c_client *client;
    
    adapter = i2c_get_adapter(bus_nr);
    if (!adapter) {
        pr_err("chuwi-minibook-x: I2C adapter %d not found\n", bus_nr);
        return -ENODEV;
    }
    
    snprintf(info.type, sizeof(info.type), "mxc4005");
    info.addr = addr;
    
    /* Apply mount matrix if enabled */
    if (enable_mount_matrix) {
        if (is_lid) {
            info.swnode = &lid_sensor_node;
            pr_info("chuwi-minibook-x: Applying lid sensor mount matrix (90° CCW)\n");
        } else {
            info.swnode = &base_sensor_node;
            pr_info("chuwi-minibook-x: Applying base sensor mount matrix (90° CW)\n");
        }
    } else {
        pr_info("chuwi-minibook-x: Mount matrix disabled, using identity transformation\n");
    }
    
    client = i2c_new_client_device(adapter, &info);
    // ... rest of function unchanged
}
```

### 5. Updated All Function Calls

```c
// Lid sensor calls (is_lid = true):
ret = chuwi_minibook_x_instantiate_mxc4005(lid_bus, lid_addr, false, true);
ret = chuwi_minibook_x_instantiate_mxc4005(lid_bus, lid_addr, true, true);

// Base sensor calls (is_lid = false):
ret = chuwi_minibook_x_instantiate_mxc4005(base_bus, base_addr, true, false);
ret = chuwi_minibook_x_instantiate_mxc4005(base_bus, base_addr, false, false);
```

## How It Works

### 1. **Initialization Phase**
- Driver loads with predefined mount matrices for lid (90° CCW) and base (90° CW) sensors
- Software nodes are created with property entries containing the mount matrix values

### 2. **Device Instantiation Phase**
- When creating I2C devices, the driver assigns the appropriate software node
- `info.swnode = &lid_sensor_node` or `info.swnode = &base_sensor_node`
- The MXC4005 driver receives the device with the mount matrix property

### 3. **MXC4005 Driver Integration**
- MXC4005 driver calls `iio_read_mount_matrix()` during probe
- The mount matrix is read from device properties and stored in `data->orientation`
- IIO framework applies transformation automatically to all readings

### 4. **Runtime Operation**
- All accelerometer readings are automatically transformed
- Applications see standard laptop coordinates (X=right, Y=back, Z=up)
- No manual coordinate transformation needed in userspace

## Expected Results

### Before Implementation
```bash
# Raw sensor values requiring complex transformation
cat /sys/bus/iio/devices/iio:device0/in_accel_mount_matrix
1, 0, 0; 0, 1, 0; 0, 0, 1

# Complex coordinate analysis needed
./sensor_test.sh  # Shows mixed axis responses
```

### After Implementation
```bash
# Proper mount matrices applied
cat /sys/bus/iio/devices/iio:device0/in_accel_mount_matrix
0, 1, 0; -1, 0, 0; 0, 0, 1

cat /sys/bus/iio/devices/iio:device1/in_accel_mount_matrix  
0, -1, 0; 1, 0, 0; 0, 0, 1

# Standard coordinate behavior
./sensor_test.sh  # Shows clean X+/X- for left/right tilts
```

## Benefits

1. **🎯 Automatic Transformation**: No manual coordinate conversion needed
2. **📱 Standard Compliance**: Uses official IIO mount matrix mechanism  
3. **🔧 Hardware Abstraction**: Applications see consistent coordinate system
4. **⚙️ Configurable**: Enable/disable via module parameter
5. **🚀 Simplified Logic**: Tablet mode detection becomes straightforward

## Testing Commands

```bash
# Load with mount matrix enabled (default)
sudo insmod chuwi-minibook-x.ko

# Load with mount matrix disabled
sudo insmod chuwi-minibook-x.ko enable_mount_matrix=false

# Check current setting
cat /sys/module/chuwi_minibook_x/parameters/enable_mount_matrix

# Verify mount matrices
cat /sys/bus/iio/devices/iio:device*/in_accel_mount_matrix

# Test coordinate transformation
./sensor_test.sh
```

This implementation provides a complete solution for automatic coordinate transformation, making tablet mode detection and orientation handling much simpler and more reliable.