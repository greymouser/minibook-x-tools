# IIO Event-Based Accelerometer Test

This directory contains a test program demonstrating IIO buffered reading with triggers for the mxc4005 accelerometers in the Chuwi Minibook X.

## Files
- `iio_event_test.c` - Main test program showing real-time accelerometer data
- `Makefile` - Build configuration
- `README.md` - This file

## Features
- Uses IIO triggers for event-driven data collection
- Buffered reading for efficient data acquisition  
- Real-time display with values updating in place
- Configurable sample rates
- Demonstrates alternative to polling approach

## Usage
```bash
make
sudo ./iio_event_test
```

## Requirements
- Root privileges (for IIO device access)
- IIO subsystem with trigger support
- mxc4005 accelerometer driver with buffer support