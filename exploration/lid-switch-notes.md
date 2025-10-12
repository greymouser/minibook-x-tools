
Create a device entry for the second firmware node entry.
```bash
echo mxc4005 0x15 | tee /sys/bus/i2c/devices/i2c-12/new_device
```
TODO: figure out how to do this at boot time?