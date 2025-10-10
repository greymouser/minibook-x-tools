# Base Sensor Axis Orientation Test

## Overview

I've created a comprehensive interactive test (`base_axis_test.sh`) that will help you definitively determine the orientation of the base sensor's X and Y axes. This test addresses your question about whether there's any rotation of the base sensor in "laptop mode".

## What the Test Does

The script guides you through a series of physical rotations of the laptop base:

1. **Baseline** - Laptop flat on table (reference position)
2. **Right side up** - Base rotated so right edge points up 
3. **Left side up** - Base rotated so left edge points up
4. **Front edge up** - Base standing on back edge (hinge down)
5. **Back edge up** - Base standing on front edge

For each position, it takes accelerometer readings and analyzes which axis changes most significantly.

## Analysis Features

The script automatically:
- **Maps sensor axes to physical directions** (X+ = right/left/front/back)
- **Determines rotation** (none, 90° CCW, 90° CW, 180°)
- **Compares with lid sensor** (known 90° CCW rotation)
- **Provides tablet mode implications** based on findings

## Integration

I've integrated this into your existing diagnostic suite:

### Access Methods:
```bash
# Option 1: Through main sensor test menu
./sensor_test.sh
# Select option 2

# Option 2: Direct execution  
./base_axis_test.sh

# Option 3: Command line flag
./sensor_test.sh --base-axis-test
```

### Expected Results:
Based on our earlier analysis showing the base sensor has Z-axis dominance (normal orientation), the test should confirm:

- **X+ = Right side** (standard laptop coordinates)
- **Y+ = Back edge** (away from user)
- **Z+ = Up** (already confirmed)
- **Result: No rotation** for base sensor

This would confirm that:
- **Lid sensor**: 90° CCW rotation (confirmed)
- **Base sensor**: Normal orientation (to be confirmed)
- **Difference**: 90° offset between sensors

## Practical Value

This test will definitively answer:
1. **Is the base sensor rotated in laptop mode?** (Expected: No)
2. **What coordinate transformations are needed?** (Base: none, Lid: 90° CCW)
3. **How should tablet mode detection work?** (Different logic for each sensor)

The comprehensive analysis will give you absolute certainty about the base sensor's orientation, removing any guesswork from your driver development.

## Documentation

I've updated the README.md with full documentation of this new test, including:
- Purpose and methodology
- Integration with existing diagnostic tools  
- Expected results and implications
- Step-by-step test process

When you run this test, it should provide definitive confirmation of whether the base sensor has any rotation applied, giving you the complete picture of both sensors' orientations.