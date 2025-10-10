#!/bin/bash

# Test the improved logic with user's actual data
echo "=== Testing Improved Axis Detection Logic ==="
echo "Using your actual test data:"
echo ""

# Your actual data:
right_x_change=-14
right_y_change=1088  
right_z_change=1144

left_x_change=-14
left_y_change=-966
left_z_change=901

# Calculate absolute values
right_x_abs=14
right_y_abs=1088
right_z_abs=1144
left_x_abs=14
left_y_abs=966
left_z_abs=901

echo "X-axis: Right=$right_x_change, Left=$left_x_change"
echo "Y-axis: Right=$right_y_change, Left=$left_y_change" 
echo "Z-axis: Right=$right_z_change, Left=$left_z_change"
echo ""

# Calculate total magnitude and consistency for each axis
max_lr_x_change=$((right_x_abs + left_x_abs))
max_lr_y_change=$((right_y_abs + left_y_abs))
max_lr_z_change=$((right_z_abs + left_z_abs))

# Calculate consistency score (opposite directions should have opposite signs)
x_consistent=0
y_consistent=0
z_consistent=0

# Check if changes are in opposite directions (good for left/right axis)
if [ $right_x_change -ne 0 ] && [ $left_x_change -ne 0 ]; then
    if [ $((right_x_change * left_x_change)) -lt 0 ]; then
        x_consistent=1
    fi
fi

if [ $right_y_change -ne 0 ] && [ $left_y_change -ne 0 ]; then
    if [ $((right_y_change * left_y_change)) -lt 0 ]; then
        y_consistent=1
    fi
fi

if [ $right_z_change -ne 0 ] && [ $left_z_change -ne 0 ]; then
    if [ $((right_z_change * left_z_change)) -lt 0 ]; then
        z_consistent=1
    fi
fi

echo "Total response magnitudes:"
echo "X: $max_lr_x_change (consistent: $x_consistent)"
echo "Y: $max_lr_y_change (consistent: $y_consistent)"
echo "Z: $max_lr_z_change (consistent: $z_consistent)"
echo ""

# Choose axis based on magnitude AND consistency
lr_axis=""

# Prefer axes with consistent opposite responses and high magnitude
if [ $y_consistent -eq 1 ] && [ $max_lr_y_change -gt 400 ]; then
    if [ $x_consistent -eq 0 ] && [ $z_consistent -eq 0 ]; then
        # Y is the only consistent axis
        lr_axis="Y"
        echo "✓ Y selected: Only consistent axis with strong response"
    elif [ $max_lr_y_change -gt $max_lr_x_change ] && [ $max_lr_y_change -gt $max_lr_z_change ]; then
        # Y has highest magnitude among consistent axes
        lr_axis="Y"
        echo "✓ Y selected: Highest magnitude among consistent axes"
    fi
fi

if [ -z "$lr_axis" ] && [ $x_consistent -eq 1 ] && [ $max_lr_x_change -gt 400 ]; then
    lr_axis="X"
    echo "✓ X selected as fallback"
fi

if [ -z "$lr_axis" ] && [ $z_consistent -eq 1 ] && [ $max_lr_z_change -gt 400 ]; then
    lr_axis="Z"
    echo "✓ Z selected as fallback"
fi

# Final result
if [ -n "$lr_axis" ]; then
    echo ""
    echo "🎯 RESULT: $lr_axis axis selected for left/right"
    if [ "$lr_axis" = "Y" ]; then
        if [ $right_y_change -lt 0 ]; then
            echo "   Y+ points toward RIGHT"
        else
            echo "   Y+ points toward LEFT"
        fi
    fi
else
    echo ""
    echo "❌ FAILED: No axis selected"
fi

echo ""
echo "This should fix your 'Y+=UNKNOWN' issue!"