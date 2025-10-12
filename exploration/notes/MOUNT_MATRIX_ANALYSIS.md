# IIO Mount Matrix Implementation Analysis

## Executive Summary

**YES, we can pass different mount matrices for each sensor!** The IIO system supports this through Linux device properties, and our Chuwi Minibook X platform driver can set unique mount matrices for each MXC4005 accelerometer during device instantiation.

## How Mount Matrices Work in IIO

### 1. **Current IIO Mount Matrix System**

The MXC4005 driver already has full mount matrix support:

```c
// MXC4005 driver structure (drivers/iio/accel/mxc4005.c)
struct mxc4005_data {
    struct iio_mount_matrix orientation;  // ✅ Mount matrix support built-in
    // ... other fields
};

// Mount matrix initialization in probe()
if (!iio_read_acpi_mount_matrix(&client->dev, &data->orientation, "ROTM")) {
    ret = iio_read_mount_matrix(&client->dev, &data->orientation);
    if (ret)
        return ret;
}
```

### 2. **Mount Matrix Sources (Priority Order)**

1. **ACPI "ROTM" property** (highest priority)
2. **Device tree/Linux device properties** (`mount-matrix` property)
3. **Default identity matrix** (fallback)

### 3. **Current Mount Matrix Status**

```bash
# Current readings show identity matrices (no transformation)
$ cat /sys/bus/iio/devices/iio:device0/in_accel_mount_matrix
1, 0, 0; 0, 1, 0; 0, 0, 1

$ cat /sys/bus/iio/devices/iio:device1/in_accel_mount_matrix  
1, 0, 0; 0, 1, 0; 0, 0, 1
```

**This means both sensors are using raw values with no coordinate transformation.**

## Implementation Strategy

### **Method 1: Software Node Properties (Recommended)**

Based on successful patterns from `drivers/platform/x86/x86-android-tablets/`, we can set mount matrices when instantiating I2C devices:

```c
// Define mount matrices for each sensor orientation
static const char * const lid_sensor_mount_matrix[] = {
    "0", "1", "0",    // 90° CCW: X' = Y
    "-1", "0", "0",   //          Y' = -X  
    "0", "0", "1"     //          Z' = Z
};

static const char * const base_sensor_mount_matrix[] = {
    "0", "-1", "0",   // 90° CW:  X' = -Y
    "1", "0", "0",    //          Y' = X
    "0", "0", "1"     //          Z' = Z  
};

// Create property entries
static const struct property_entry lid_sensor_props[] = {
    PROPERTY_ENTRY_STRING_ARRAY("mount-matrix", lid_sensor_mount_matrix),
    { }
};

static const struct property_entry base_sensor_props[] = {
    PROPERTY_ENTRY_STRING_ARRAY("mount-matrix", base_sensor_mount_matrix),
    { }
};

// Create software nodes
static const struct software_node lid_sensor_node = {
    .properties = lid_sensor_props,
};

static const struct software_node base_sensor_node = {
    .properties = base_sensor_props,
};

// Apply during I2C device instantiation
static int instantiate_mxc4005_with_mount_matrix(int bus, int addr, bool is_lid) {
    struct i2c_adapter *adapter;
    struct i2c_board_info board_info = {};
    struct i2c_client *client;
    
    adapter = i2c_get_adapter(bus);
    if (!adapter)
        return -ENODEV;
        
    strscpy(board_info.type, "mxc4005", sizeof(board_info.type));
    board_info.addr = addr;
    
    // Set appropriate mount matrix
    if (is_lid)
        board_info.swnode = &lid_sensor_node;
    else
        board_info.swnode = &base_sensor_node;
    
    client = i2c_new_client_device(adapter, &board_info);
    i2c_put_adapter(adapter);
    
    return PTR_ERR_OR_ZERO(client);
}
```

### **Method 2: Direct Device Property Setting**

Alternative approach using device properties (if Method 1 doesn't work):

```c
#include <linux/property.h>

static int set_mount_matrix_post_instantiation(struct device *dev, bool is_lid) {
    const char **matrix;
    
    if (is_lid)
        matrix = lid_sensor_mount_matrix;
    else
        matrix = base_sensor_mount_matrix;
        
    // Note: This approach needs investigation - may require device property manipulation
    return device_property_add_string_array(dev, "mount-matrix", matrix, 9);
}
```

## Mount Matrix Calculations

### **Coordinate Transformation Theory**

Based on our sensor analysis:

| Sensor | Actual Orientation | Required Transformation |
|--------|-------------------|-------------------------|
| **Lid** | 90° CCW rotation | `[0,1,0; -1,0,0; 0,0,1]` |
| **Base** | 90° CW rotation | `[0,-1,0; 1,0,0; 0,0,1]` |

### **Matrix Verification**

```
Standard coordinates: X=right, Y=back, Z=up

Lid sensor (90° CCW):
- Physical X+ points BACK  → Standard Y+ 
- Physical Y+ points LEFT  → Standard X-
- Physical Z+ points UP    → Standard Z+
Matrix: [0,1,0; -1,0,0; 0,0,1]

Base sensor (90° CW):  
- Physical X+ points BACK  → Standard Y+
- Physical Y+ points LEFT  → Standard X-  
- Physical Z+ points UP    → Standard Z+
Matrix: [0,-1,0; 1,0,0; 0,0,1]
```

## Integration Points

### **1. Modify chuwi-minibook-x-main.c**

```c
// In chuwi_minibook_x_instantiate_mxc4005()
static int chuwi_minibook_x_instantiate_mxc4005(int bus, int addr, bool is_lid) {
    // ... existing code ...
    
    // Set mount matrix before instantiation
    if (is_lid)
        board_info.swnode = &lid_sensor_node;
    else  
        board_info.swnode = &base_sensor_node;
        
    // ... rest of instantiation ...
}
```

### **2. Update Module Parameters**

Add mount matrix control:

```c
static bool enable_mount_matrix = true;
module_param(enable_mount_matrix, bool, 0644);
MODULE_PARM_DESC(enable_mount_matrix, "Enable mount matrix transformation (default: true)");
```

### **3. Testing and Validation**

After implementation:

```bash
# Verify mount matrices are applied
cat /sys/bus/iio/devices/iio:device0/in_accel_mount_matrix
# Should show: 0, 1, 0; -1, 0, 0; 0, 0, 1

cat /sys/bus/iio/devices/iio:device1/in_accel_mount_matrix  
# Should show: 0, -1, 0; 1, 0, 0; 0, 0, 1

# Test coordinate transformation
./sensor_test.sh
# Readings should now show standard laptop coordinates
```

## Benefits of This Approach

1. **✅ Hardware Abstraction**: Applications see standard coordinates automatically
2. **✅ No Software Changes**: Existing tablet mode detection works without modification  
3. **✅ IIO Standard**: Uses official IIO mount matrix mechanism
4. **✅ Per-Device**: Each sensor gets appropriate transformation
5. **✅ Maintainable**: Clear, documented implementation

## Expected Results

After implementation:
- Lid sensor readings automatically transformed to standard orientation
- Base sensor readings automatically transformed to standard orientation  
- Tablet mode detection works with simple gravity vector analysis
- No complex coordinate transformation needed in userspace applications

---

**Conclusion**: The IIO mount matrix system provides exactly what we need. Implementation should be straightforward using the software node approach demonstrated in x86-android-tablets drivers.