# ACPI Device Table Example for Chuwi Minibook X
# This shows what the ACPI DSDT (Differentiated System Description Table) 
# would contain for proper device enumeration.

# Example ACPI Device Definition (ASL - ACPI Source Language)

Device (TBMD) {  // Tablet Mode Device
    Name (_HID, "CHUW0001")  // Hardware ID for Chuwi Minibook X
    Name (_CID, "PNP0C60")   // Compatible ID for tablet mode switch
    Name (_UID, 1)           // Unique ID
    
    // Current Resource Settings - defines I2C connections
    Name (_CRS, ResourceTemplate () {
        // Base accelerometer I2C connection
        I2cSerialBusV2 (0x15,          // Slave address
                       ControllerInitiated,
                       400000,          // Connection speed (400kHz)
                       AddressingMode7Bit,
                       "\\_SB.PCI0.I2C2",  // I2C controller path (i2c-12)
                       0x00,
                       ResourceConsumer,
                       ,
                       Exclusive,
                       ,)
        
        // Lid accelerometer I2C connection  
        I2cSerialBusV2 (0x15,          // Slave address
                       ControllerInitiated,
                       400000,          // Connection speed (400kHz)
                       AddressingMode7Bit,
                       "\\_SB.PCI0.I2C3",  // I2C controller path (i2c-13)
                       0x00,
                       ResourceConsumer,
                       ,
                       Exclusive,
                       ,)
    })
    
    // Device Specific Data (_DSD) - modern ACPI configuration method
    Name (_DSD, Package () {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"), // Device Properties UUID
        Package () {
            // Tablet mode thresholds
            Package () {"enter-threshold-degrees", 200},
            Package () {"exit-threshold-degrees", 170},
            Package () {"hysteresis-degrees", 10},
            
            // Orientation detection settings
            Package () {"orientation-method", "accelerometer"},
            Package () {"orientation-stability-ms", 2000},
            Package () {"orientation-confidence-threshold", 50},
            
            // Accelerometer configuration
            Package () {"accelerometer-mounting", "lid-base-dual"},
            Package () {"auto-calibration-enabled", 1},
            
            // Hinge axis default (will be auto-calibrated)
            Package () {"hinge-axis-x", 0},
            Package () {"hinge-axis-y", 1000000},
            Package () {"hinge-axis-z", 0},
            
            // Device-specific properties
            Package () {"polling-interval-ms", 200},
            Package () {"device-model", "chuwi-minibook-x"},
        }
    })
    
    // Device status - indicates device is present and functional
    Method (_STA, 0, NotSerialized) {
        Return (0x0F)  // Present, enabled, shown in UI, functioning
    }
    
    // Power management
    Name (_S0W, 3)  // Supports D3hot in S0
    
    // Custom methods for calibration data (optional)
    Method (GCAL, 0, Serialized) {
        // Get Calibration data
        // Could return factory calibration values
        Return (Package () {
            0,        // Base accel mount matrix row 1
            1000000,  // Base accel mount matrix row 2  
            0,        // Base accel mount matrix row 3
            // ... etc
        })
    }
}

# The above ACPI definition would be compiled into the system DSDT
# and would cause the kernel ACPI bus to automatically instantiate
# the platform device when the system boots.

# Key benefits of ACPI enumeration:
# 1. Automatic device discovery
# 2. Proper resource allocation  
# 3. Hardware abstraction
# 4. Power management integration
# 5. Configuration data storage
# 6. Platform independence

# In contrast to current manual instantiation:
# - No need for hardcoded I2C bus numbers
# - No manual platform_device_register_simple()
# - Proper integration with ACPI power management
# - Hardware vendor can provide configuration
# - Works across different hardware revisions