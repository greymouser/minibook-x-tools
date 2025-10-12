#!/usr/bin/env python3

def decode_gmtr_buffer(name, buffer_hex):
    """Decode GMTR buffer to understand the matrix format"""
    print(f"\n{name} buffer analysis:")
    
    # First 9 bytes as potential 3x3 matrix
    matrix_bytes = buffer_hex[:9]
    print(f"First 9 bytes (potential matrix): {[hex(b) for b in matrix_bytes]}")
    
    # Convert 0xFF to -1, others as-is
    converted = []
    for b in matrix_bytes:
        if b == 0xFF:
            converted.append(-1)
        else:
            converted.append(b)
    
    print(f"Converted values: {converted}")
    
    # Arrange as 3x3 matrix
    matrix = [converted[i:i+3] for i in range(0, 9, 3)]
    print(f"As 3x3 matrix:")
    for row in matrix:
        print(f"  {row}")
    
    # Convert to ROTM-compatible strings
    rotm_strings = []
    for row in matrix:
        rotm_strings.append(f'"{row[0]} {row[1]} {row[2]}"')
    
    print(f"ROTM format: {', '.join(rotm_strings)}")
    
    # Show remaining bytes
    remaining = buffer_hex[9:]
    print(f"Remaining bytes: {[hex(b) for b in remaining]}")

# PARA buffer (lid accelerometer)
para = [0x00, 0xFF, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xB9, 0xAF, 0x1E, 0x05, 0x14, 0x10]

# PARB buffer (base accelerometer)  
parb = [0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0xB9, 0xAF, 0x1E, 0x05, 0x14, 0x13]

decode_gmtr_buffer("PARA", para)
decode_gmtr_buffer("PARB", parb)