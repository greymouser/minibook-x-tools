#!/bin/bash

echo "=== Base Sensor Axis Orientation Test ==="
echo "========================================"
echo ""
echo "This test will help determine the exact orientation of the base sensor's"
echo "X and Y axes by having you rotate the base in different directions."
echo ""
echo "We'll take readings in different orientations to map the sensor axes"
echo "to physical laptop directions."
echo ""
echo "=== LAPTOP ORIENTATION REFERENCE ==="
echo "When you sit at the laptop in normal typing position:"
echo "• RIGHT SIDE  = Where your RIGHT HAND naturally rests (usually has ports/vents)"
echo "• LEFT SIDE   = Where your LEFT HAND naturally rests (usually has ports/vents)"  
echo "• FRONT EDGE  = Where your palms/wrists rest while typing (closest to you)"
echo "• BACK EDGE   = Where the screen hinge is located (farthest from you)"
echo ""

# Function to take a snapshot
take_reading() {
    local label="$1"
    if [ -f "/sys/bus/iio/devices/iio:device1/in_accel_x_raw" ]; then
        local x=$(cat /sys/bus/iio/devices/iio:device1/in_accel_x_raw)
        local y=$(cat /sys/bus/iio/devices/iio:device1/in_accel_y_raw)
        local z=$(cat /sys/bus/iio/devices/iio:device1/in_accel_z_raw)
        echo "$label: X=$x, Y=$y, Z=$z"
        
        # Store values for analysis
        case "$label" in
            "Flat") flat_x=$x; flat_y=$y; flat_z=$z ;;
            "Right side up") right_x=$x; right_y=$y; right_z=$z ;;
            "Left side up") left_x=$x; left_y=$y; left_z=$z ;;
            "Front edge up") front_x=$x; front_y=$y; front_z=$z ;;
            "Back edge up") back_x=$x; back_y=$y; back_z=$z ;;
            "Right side 90° rotation") right90_x=$x; right90_y=$y; right90_z=$z ;;
            "Left side 90° rotation") left90_x=$x; left90_y=$y; left90_z=$z ;;
        esac
    else
        echo "Error: Cannot read base sensor (iio:device1)"
        exit 1
    fi
}

echo "Step 1: Baseline reading"
echo "Place the laptop flat on the table in normal position."
echo "Press Enter when ready..."
read
take_reading "Flat"
echo ""

echo "Step 2: Right side up test"
echo "REFERENCE: When facing the laptop in normal laptop mode, your RIGHT HAND rests on the RIGHT SIDE"
echo ""
echo "Carefully rotate the base so the RIGHT SIDE is pointing up"
echo "(like a book standing on its spine, with the right side of the laptop up)"
echo "The keyboard should be facing you, screen tilted away."
echo "Your RIGHT HAND side of the laptop should be pointing toward the ceiling."
echo "Press Enter when in position..."
read
take_reading "Right side up"
echo ""

echo "Step 3: Left side up test"
echo "REFERENCE: When facing the laptop in normal laptop mode, your LEFT HAND rests on the LEFT SIDE"
echo ""  
echo "Carefully rotate the base so the LEFT SIDE is pointing up"
echo "(opposite of previous position)"
echo "Your LEFT HAND side of the laptop should be pointing toward the ceiling."
echo "Press Enter when in position..."
read
take_reading "Left side up"
echo ""

echo "Step 4: Front edge up test"
echo "REFERENCE: When facing the laptop in normal laptop mode, the FRONT EDGE is where your palms/wrists rest"
echo ""
echo "Carefully rotate the base so the FRONT EDGE (where your palms rest) is pointing up"
echo "The laptop should be standing on its back edge (hinge side down)"
echo "The edge closest to you when typing should be pointing toward the ceiling."
echo "Press Enter when in position..."
read
take_reading "Front edge up"
echo ""

echo "Step 5: Back edge up test"
echo "REFERENCE: When facing the laptop in normal laptop mode, the BACK EDGE is where the hinge/screen connects"
echo ""
echo "Carefully rotate the base so the BACK EDGE (hinge side) is pointing up"
echo "The laptop should be standing on its front edge"
echo "The edge with the hinge/screen connection should be pointing toward the ceiling."
echo "Press Enter when in position..."
read
take_reading "Back edge up"
echo ""

echo "Return laptop to flat position for analysis..."
echo "Press Enter to continue..."
read

echo ""
echo "=== VERIFICATION TESTS ==="
echo "========================="
echo "These additional positions help verify the primary analysis."
echo ""

echo "Verification 1: Right side up (90° clockwise from normal)"
echo "REFERENCE: Rotate the laptop 90° clockwise so the right side is up"
echo "The laptop should look like a book standing on its spine"
echo "Press Enter when in position..."
read
take_reading "Right side 90° rotation"
echo ""

echo "Verification 2: Left side up (90° counter-clockwise from normal)"  
echo "REFERENCE: Rotate the laptop 90° counter-clockwise so the left side is up"
echo "Opposite of the previous position"
echo "Press Enter when in position..."
read
take_reading "Left side 90° rotation"
echo ""

echo "Return laptop to flat position for final analysis..."
echo "Press Enter to continue..."
read

echo ""
echo "=== ANALYSIS ==="
echo "==============="
echo ""

# Analyze X-axis
echo "X-AXIS ANALYSIS:"
echo "Flat position: X=$flat_x"
echo "Right side up: X=$right_x (change: $((right_x - flat_x)))"
echo "Left side up:  X=$left_x (change: $((left_x - flat_x)))"

right_x_change=$((right_x - flat_x))
left_x_change=$((left_x - flat_x))
right_x_abs=${right_x_change#-}
left_x_abs=${left_x_change#-}

# Analyze Y-axis  
echo ""
echo "Y-AXIS ANALYSIS:"
echo "Flat position: Y=$flat_y"
echo "Right side up: Y=$right_y (change: $((right_y - flat_y)))"
echo "Left side up:  Y=$left_y (change: $((left_y - flat_y)))"

right_y_change=$((right_y - flat_y))
left_y_change=$((left_y - flat_y))
right_y_abs=${right_y_change#-}
left_y_abs=${left_y_change#-}

# Analyze Z-axis  
echo ""
echo "Z-AXIS ANALYSIS:"
echo "Flat position: Z=$flat_z"
echo "Right side up: Z=$right_z (change: $((right_z - flat_z)))"
echo "Left side up:  Z=$left_z (change: $((left_z - flat_z)))"

right_z_change=$((right_z - flat_z))
left_z_change=$((left_z - flat_z))
right_z_abs=${right_z_change#-}
left_z_abs=${left_z_change#-}

echo ""
echo "LEFT/RIGHT TILT ANALYSIS:"

# Find which axis changed most during left/right tilts
max_right_change=0
max_right_axis=""
max_left_change=0
max_left_axis=""

# Check right tilt
if [ $right_x_abs -gt $max_right_change ]; then
    max_right_change=$right_x_abs
    max_right_axis="X"
fi
if [ $right_y_abs -gt $max_right_change ]; then
    max_right_change=$right_y_abs
    max_right_axis="Y"
fi
if [ $right_z_abs -gt $max_right_change ]; then
    max_right_change=$right_z_abs
    max_right_axis="Z"
fi

# Check left tilt
if [ $left_x_abs -gt $max_left_change ]; then
    max_left_change=$left_x_abs
    max_left_axis="X"
fi
if [ $left_y_abs -gt $max_left_change ]; then
    max_left_change=$left_y_abs
    max_left_axis="Y"
fi
if [ $left_z_abs -gt $max_left_change ]; then
    max_left_change=$left_z_abs
    max_left_axis="Z"
fi

echo "Most responsive axis during right tilt: $max_right_axis (change: $max_right_change)"
echo "Most responsive axis during left tilt:  $max_left_axis (change: $max_left_change)"

# Determine left/right axis mapping
lr_axis=""
lr_direction=""

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

echo "Total left/right response: X=$max_lr_x_change, Y=$max_lr_y_change, Z=$max_lr_z_change"
echo "Direction consistency: X=$x_consistent, Y=$y_consistent, Z=$z_consistent"

# Choose axis based on magnitude AND consistency
lr_axis=""
lr_direction=""

# Prefer axes with consistent opposite responses and high magnitude
if [ $y_consistent -eq 1 ] && [ $max_lr_y_change -gt 400 ]; then
    if [ $x_consistent -eq 0 ] && [ $z_consistent -eq 0 ]; then
        # Y is the only consistent axis
        lr_axis="Y"
    elif [ $max_lr_y_change -gt $max_lr_x_change ] && [ $max_lr_y_change -gt $max_lr_z_change ]; then
        # Y has highest magnitude among consistent axes
        lr_axis="Y"
    fi
fi

if [ -z "$lr_axis" ] && [ $x_consistent -eq 1 ] && [ $max_lr_x_change -gt 400 ]; then
    if [ $z_consistent -eq 0 ]; then
        # X is the only other consistent axis
        lr_axis="X"
    elif [ $max_lr_x_change -gt $max_lr_z_change ]; then
        # X has higher magnitude than Z
        lr_axis="X"
    fi
fi

if [ -z "$lr_axis" ] && [ $z_consistent -eq 1 ] && [ $max_lr_z_change -gt 400 ]; then
    lr_axis="Z"
fi

# If no consistent axis found, fall back to magnitude only
if [ -z "$lr_axis" ]; then
    if [ $max_lr_y_change -gt $max_lr_x_change ] && [ $max_lr_y_change -gt $max_lr_z_change ] && [ $max_lr_y_change -gt 400 ]; then
        lr_axis="Y"
    elif [ $max_lr_x_change -gt $max_lr_z_change ] && [ $max_lr_x_change -gt 400 ]; then
        lr_axis="X"
    elif [ $max_lr_z_change -gt 400 ]; then
        lr_axis="Z"
    fi
fi

# Determine direction for the chosen axis
if [ "$lr_axis" = "Y" ]; then
    if [ $right_y_change -lt 0 ]; then
        echo "→ Y+ points toward RIGHT side of laptop"
        lr_direction="Y+=RIGHT"
    else
        echo "→ Y+ points toward LEFT side of laptop"
        lr_direction="Y+=LEFT"
    fi
elif [ "$lr_axis" = "X" ]; then
    if [ $right_x_change -lt 0 ]; then
        echo "→ X+ points toward RIGHT side of laptop"
        lr_direction="X+=RIGHT"
    else
        echo "→ X+ points toward LEFT side of laptop"
        lr_direction="X+=LEFT"
    fi
elif [ "$lr_axis" = "Z" ]; then
    if [ $right_z_change -lt 0 ]; then
        echo "→ Z+ points toward RIGHT side of laptop"
        lr_direction="Z+=RIGHT"
    else
        echo "→ Z+ points toward LEFT side of laptop"
        lr_direction="Z+=LEFT"
    fi
else
    echo "→ Left/right axis direction unclear - no axis shows consistent strong response"
    lr_direction="UNCLEAR"
fi

echo ""
echo "FRONT/BACK TILT ANALYSIS:"

# Analyze front/back tilts for all axes
front_x_change=$((front_x - flat_x))
front_y_change=$((front_y - flat_y))
front_z_change=$((front_z - flat_z))
back_x_change=$((back_x - flat_x))
back_y_change=$((back_y - flat_y))
back_z_change=$((back_z - flat_z))

front_x_abs=${front_x_change#-}
front_y_abs=${front_y_change#-}
front_z_abs=${front_z_change#-}
back_x_abs=${back_x_change#-}
back_y_abs=${back_y_change#-}
back_z_abs=${back_z_change#-}

echo "Front edge up: X=$front_x (change: $front_x_change), Y=$front_y (change: $front_y_change), Z=$front_z (change: $front_z_change)"
echo "Back edge up:  X=$back_x (change: $back_x_change), Y=$back_y (change: $back_y_change), Z=$back_z (change: $back_z_change)"

# Find which axis changed most during front/back tilts
max_front_change=0
max_front_axis=""
max_back_change=0
max_back_axis=""

# Check front tilt
if [ $front_x_abs -gt $max_front_change ]; then
    max_front_change=$front_x_abs
    max_front_axis="X"
fi
if [ $front_y_abs -gt $max_front_change ]; then
    max_front_change=$front_y_abs
    max_front_axis="Y"
fi
if [ $front_z_abs -gt $max_front_change ]; then
    max_front_change=$front_z_abs
    max_front_axis="Z"
fi

# Check back tilt
if [ $back_x_abs -gt $max_back_change ]; then
    max_back_change=$back_x_abs
    max_back_axis="X"
fi
if [ $back_y_abs -gt $max_back_change ]; then
    max_back_change=$back_y_abs
    max_back_axis="Y"
fi
if [ $back_z_abs -gt $max_back_change ]; then
    max_back_change=$back_z_abs
    max_back_axis="Z"
fi

echo "Most responsive axis during front tilt: $max_front_axis (change: $max_front_change)"
echo "Most responsive axis during back tilt:  $max_back_axis (change: $max_back_change)"

# Determine front/back axis mapping
fb_axis=""
fb_direction=""

# Calculate total magnitude and consistency for each axis
max_fb_x_change=$((front_x_abs + back_x_abs))
max_fb_y_change=$((front_y_abs + back_y_abs))
max_fb_z_change=$((front_z_abs + back_z_abs))

# Calculate consistency score (opposite directions should have opposite signs)
x_fb_consistent=0
y_fb_consistent=0
z_fb_consistent=0

# Check if changes are in opposite directions (good for front/back axis)
if [ $front_x_change -ne 0 ] && [ $back_x_change -ne 0 ]; then
    if [ $((front_x_change * back_x_change)) -lt 0 ]; then
        x_fb_consistent=1
    fi
fi

if [ $front_y_change -ne 0 ] && [ $back_y_change -ne 0 ]; then
    if [ $((front_y_change * back_y_change)) -lt 0 ]; then
        y_fb_consistent=1
    fi
fi

if [ $front_z_change -ne 0 ] && [ $back_z_change -ne 0 ]; then
    if [ $((front_z_change * back_z_change)) -lt 0 ]; then
        z_fb_consistent=1
    fi
fi

echo "Total front/back response: X=$max_fb_x_change, Y=$max_fb_y_change, Z=$max_fb_z_change"
echo "Direction consistency: X=$x_fb_consistent, Y=$y_fb_consistent, Z=$z_fb_consistent"

# Choose axis based on magnitude AND consistency, avoiding the left/right axis
fb_axis=""
fb_direction=""

# Prefer axes with consistent opposite responses and high magnitude
# Also avoid choosing the same axis as left/right if possible
if [ $x_fb_consistent -eq 1 ] && [ $max_fb_x_change -gt 400 ] && [ "$lr_axis" != "X" ]; then
    fb_axis="X"
elif [ $y_fb_consistent -eq 1 ] && [ $max_fb_y_change -gt 400 ] && [ "$lr_axis" != "Y" ]; then
    fb_axis="Y"
elif [ $z_fb_consistent -eq 1 ] && [ $max_fb_z_change -gt 400 ] && [ "$lr_axis" != "Z" ]; then
    fb_axis="Z"
fi

# If no non-conflicting consistent axis found, choose highest magnitude with consistency
if [ -z "$fb_axis" ]; then
    if [ $x_fb_consistent -eq 1 ] && [ $max_fb_x_change -gt 400 ]; then
        if [ $max_fb_x_change -gt $max_fb_y_change ] && [ $max_fb_x_change -gt $max_fb_z_change ]; then
            fb_axis="X"
        fi
    fi
    if [ -z "$fb_axis" ] && [ $y_fb_consistent -eq 1 ] && [ $max_fb_y_change -gt 400 ]; then
        if [ $max_fb_y_change -gt $max_fb_z_change ]; then
            fb_axis="Y"
        fi
    fi
    if [ -z "$fb_axis" ] && [ $z_fb_consistent -eq 1 ] && [ $max_fb_z_change -gt 400 ]; then
        fb_axis="Z"
    fi
fi

# If still no consistent axis, fall back to magnitude only
if [ -z "$fb_axis" ]; then
    if [ $max_fb_x_change -gt $max_fb_y_change ] && [ $max_fb_x_change -gt $max_fb_z_change ] && [ $max_fb_x_change -gt 400 ]; then
        fb_axis="X"
    elif [ $max_fb_y_change -gt $max_fb_z_change ] && [ $max_fb_y_change -gt 400 ]; then
        fb_axis="Y"
    elif [ $max_fb_z_change -gt 400 ]; then
        fb_axis="Z"
    fi
fi

# Determine direction for the chosen axis
if [ "$fb_axis" = "X" ]; then
    if [ $front_x_change -gt 0 ]; then
        echo "→ X+ points toward FRONT of laptop"
        fb_direction="X+=FRONT"
    else
        echo "→ X+ points toward BACK of laptop"
        fb_direction="X+=BACK"
    fi
elif [ "$fb_axis" = "Y" ]; then
    if [ $front_y_change -gt 0 ]; then
        echo "→ Y+ points toward FRONT of laptop"
        fb_direction="Y+=FRONT"
    else
        echo "→ Y+ points toward BACK of laptop"
        fb_direction="Y+=BACK"
    fi
elif [ "$fb_axis" = "Z" ]; then
    if [ $front_z_change -gt 0 ]; then
        echo "→ Z+ points toward FRONT of laptop"
        fb_direction="Z+=FRONT"
    else
        echo "→ Z+ points toward BACK of laptop"
        fb_direction="Z+=BACK"
    fi
else
    echo "→ Front/back axis direction unclear - no axis shows consistent strong response"
    fb_direction="UNCLEAR"
fi

echo ""
echo "=== VERIFICATION ANALYSIS ==="
echo "============================"

# Analyze the 90° rotation tests for consistency
if [ -n "$right90_x" ] && [ -n "$left90_x" ]; then
    right90_x_change=$((right90_x - flat_x))
    right90_y_change=$((right90_y - flat_y))
    right90_z_change=$((right90_z - flat_z))
    left90_x_change=$((left90_x - flat_x))
    left90_y_change=$((left90_y - flat_y))
    left90_z_change=$((left90_z - flat_z))
    
    right90_x_abs=${right90_x_change#-}
    right90_y_abs=${right90_y_change#-}
    right90_z_abs=${right90_z_change#-}
    left90_x_abs=${left90_x_change#-}
    left90_y_abs=${left90_y_change#-}
    left90_z_abs=${left90_z_change#-}
    
    echo "90° rotation verification:"
    echo "Right 90°: X=$right90_x (Δ$right90_x_change), Y=$right90_y (Δ$right90_y_change), Z=$right90_z (Δ$right90_z_change)"
    echo "Left 90°:  X=$left90_x (Δ$left90_x_change), Y=$left90_y (Δ$left90_y_change), Z=$left90_z (Δ$left90_z_change)"
    
    # Check if verification data matches primary analysis
    verification_consistent=true
    if [ "$lr_direction" != "UNCLEAR" ]; then
        lr_axis_name="${lr_direction%+=*}"
        case "$lr_axis_name" in
            "X")
                if [ $right90_x_abs -lt 500 ] || [ $left90_x_abs -lt 500 ]; then
                    verification_consistent=false
                fi
                ;;
            "Y")
                if [ $right90_y_abs -lt 500 ] || [ $left90_y_abs -lt 500 ]; then
                    verification_consistent=false
                fi
                ;;
            "Z")
                if [ $right90_z_abs -lt 500 ] || [ $left90_z_abs -lt 500 ]; then
                    verification_consistent=false
                fi
                ;;
        esac
    fi
    
    if [ "$verification_consistent" = "true" ]; then
        echo "✓ Verification data consistent with primary analysis"
    else
        echo "⚠ Verification data shows different pattern - may indicate measurement error"
    fi
else
    echo "Verification tests not completed"
fi

echo ""
echo "=== COMPREHENSIVE BASE SENSOR ANALYSIS ==="
echo "=========================================="
echo ""

# Determine coordinate mapping
echo "COORDINATE SYSTEM MAPPING:"
echo "Standard laptop coordinates: X+=Right, Y+=Back, Z+=Up"
echo ""

# Parse the direction mappings
x_points=""
y_points=""
z_points="UP"  # We know Z is up from baseline

# Extract info from lr_direction and fb_direction
case "$lr_direction" in
    "X+=RIGHT") x_points="RIGHT" ;;
    "X+=LEFT") x_points="LEFT" ;;
    "Y+=RIGHT") y_points="RIGHT" ;;
    "Y+=LEFT") y_points="LEFT" ;;
    "Z+=RIGHT") z_points="RIGHT" ;;
    "Z+=LEFT") z_points="LEFT" ;;
    "UNCLEAR") ;; # Don't set anything if unclear
esac

case "$fb_direction" in
    "X+=FRONT") 
        if [ -z "$x_points" ]; then
            x_points="FRONT"
        fi
        ;;
    "X+=BACK") 
        if [ -z "$x_points" ]; then
            x_points="BACK"
        fi
        ;;
    "Y+=FRONT") 
        if [ -z "$y_points" ]; then
            y_points="FRONT"
        fi
        ;;
    "Y+=BACK") 
        if [ -z "$y_points" ]; then
            y_points="BACK"
        fi
        ;;
    "Z+=FRONT") z_points="FRONT" ;;
    "Z+=BACK") z_points="BACK" ;;
    "UNCLEAR") ;; # Don't set anything if unclear
esac

# Handle cases where axis directions conflict
if [ "$lr_direction" != "UNCLEAR" ] && [ "$fb_direction" != "UNCLEAR" ]; then
    # Check if the same axis was identified for both directions (which would be wrong)
    lr_axis_name="${lr_direction%+=*}"  # Extract "X", "Y", or "Z" from "X+=RIGHT"
    fb_axis_name="${fb_direction%+=*}"  # Extract "X", "Y", or "Z" from "Y+=BACK"
    
    if [ "$lr_axis_name" = "$fb_axis_name" ]; then
        echo "WARNING: Same axis ($lr_axis_name) detected for both left/right and front/back!"
        echo "This suggests measurement error or insufficient movement."
        x_points="CONFLICT"
        y_points="CONFLICT"
    fi
fi

# Set defaults for empty values
if [ -z "$x_points" ]; then
    x_points="UNKNOWN"
fi
if [ -z "$y_points" ]; then
    y_points="UNKNOWN"
fi

echo "Base sensor coordinate mapping:"
echo "  X+ = $x_points"
echo "  Y+ = $y_points"  
echo "  Z+ = $z_points"
echo ""

# Determine rotation
echo "=== ROTATION ANALYSIS ==="
rotation=""
rotation_angle=""

if [ "$x_points" = "RIGHT" ] && [ "$y_points" = "BACK" ]; then
    rotation="NONE"
    rotation_angle="0°"
    echo "✓ NO ROTATION DETECTED"
    echo "  Base sensor follows standard laptop coordinate system"
elif [ "$x_points" = "FRONT" ] && [ "$y_points" = "RIGHT" ]; then
    rotation="90_CCW"
    rotation_angle="90° counter-clockwise"
    echo "⟲ 90° COUNTER-CLOCKWISE ROTATION"
    echo "  Same rotation as lid sensor"
elif [ "$x_points" = "LEFT" ] && [ "$y_points" = "FRONT" ]; then
    rotation="180"
    rotation_angle="180°"
    echo "⟳ 180° ROTATION"
    echo "  Sensor is flipped relative to standard orientation"
elif [ "$x_points" = "BACK" ] && [ "$y_points" = "LEFT" ]; then
    rotation="90_CW"
    rotation_angle="90° clockwise"
    echo "⟳ 90° CLOCKWISE ROTATION"
    echo "  Opposite rotation from lid sensor"
elif [ "$x_points" = "CONFLICT" ] || [ "$y_points" = "CONFLICT" ]; then
    rotation="CONFLICT"
    rotation_angle="conflicted"
    echo "⚠️ CONFLICTING MEASUREMENTS"
    echo "  Same sensor axis responded to both left/right and front/back tilts"
elif [ "$x_points" = "UNKNOWN" ] || [ "$y_points" = "UNKNOWN" ]; then
    rotation="INCOMPLETE"
    rotation_angle="incomplete"
    echo "❓ INCOMPLETE DATA"
    echo "  Unable to determine one or both axis directions"
    echo "  Detected: X+=$x_points, Y+=$y_points"
else
    rotation="UNCLEAR"
    rotation_angle="unclear"
    echo "? UNCLEAR ROTATION"
    echo "  Unable to determine definitive orientation"
    echo "  Detected: X+=$x_points, Y+=$y_points"
fi

echo ""
echo "=== SENSOR MOUNTING CONCLUSION ==="
echo "================================="
echo ""

if [ "$rotation" != "UNCLEAR" ] && [ "$rotation" != "INCOMPLETE" ] && [ "$rotation" != "CONFLICT" ]; then
    echo "📋 DEFINITIVE RESULT:"
    echo "   Base sensor rotation: $rotation_angle"
    echo "   Lid sensor rotation:  90° counter-clockwise (known)"
    echo ""
    
    if [ "$rotation" = "NONE" ]; then
        echo "✅ SIMPLE CONFIGURATION:"
        echo "   • Base sensor: Standard orientation (no coordinate transformation needed)"
        echo "   • Lid sensor:  90° CCW rotation (coordinate transformation required)"
        echo "   • Relative difference: 90° between sensors"
    elif [ "$rotation" = "90_CCW" ]; then
        echo "🔄 MATCHED ROTATION:"
        echo "   • Both sensors have same 90° CCW rotation"
        echo "   • Same coordinate transformation needed for both"
        echo "   • Simpler tablet mode logic possible"
    else
        echo "⚠️  COMPLEX CONFIGURATION:"
        echo "   • Base sensor: $rotation_angle rotation"
        echo "   • Lid sensor:  90° CCW rotation"
        echo "   • Different coordinate transformations needed for each sensor"
    fi
elif [ "$rotation" = "CONFLICT" ]; then
    echo "⚠️ CONFLICTING MEASUREMENTS:"
    echo "   The same sensor axis responded to both left/right and front/back tilts"
    echo "   This suggests:"
    echo "   • Insufficient or incorrect movements during test"
    echo "   • Sensor mounted at a 45° angle or other non-standard orientation"
    echo "   • Try the test again with more careful, pronounced movements"
elif [ "$rotation" = "INCOMPLETE" ]; then
    echo "❓ INCOMPLETE DATA:"
    echo "   Could not determine direction for one or both axes"
    echo "   Detected: X+=$x_points, Y+=$y_points"
    echo "   This suggests:"
    echo "   • Movements were too small to detect reliably"
    echo "   • One axis may be non-responsive or damaged"
    echo "   • Try the test again with larger, more deliberate movements"
else
    echo "❌ INCONCLUSIVE RESULT:"
    echo "   Unable to determine base sensor orientation definitively"
    echo "   Detected: X+=$x_points, Y+=$y_points"
    echo "   Possible causes:"
    echo "   • Insufficient movement during test"
    echo "   • Sensor noise or calibration issues"
    echo "   • Non-standard sensor mounting"
    echo ""
    echo "   Try running the test again with more pronounced movements"
fi

echo ""
echo "=== TABLET MODE DETECTION IMPLICATIONS ==="
echo "=========================================="
if [ "$rotation" = "NONE" ]; then
    echo "Simple tablet mode detection:"
    echo "• Base sensor: Monitor Z-axis (standard gravity detection)"
    echo "• Lid sensor:  Monitor X-axis (90° rotated gravity detection)"
    echo "• When Z≈0 and X≈0: Tablet mode (both sensors horizontal)"
elif [ "$rotation" = "90_CCW" ]; then
    echo "Unified tablet mode detection:"
    echo "• Both sensors: Monitor X-axis (both have 90° CCW rotation)"
    echo "• When both X≈0: Tablet mode (both sensors horizontal)"
    echo "• Same coordinate transformation for both sensors"
else
    echo "Complex tablet mode detection required:"
    echo "• Each sensor needs specific coordinate transformation"
    echo "• Custom logic based on detected rotations"
    echo "• Base: $rotation_angle, Lid: 90° CCW"
fi