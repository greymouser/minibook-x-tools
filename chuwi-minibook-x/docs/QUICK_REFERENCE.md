# Quick Reference: Chuwi Minibook X Sensor Orientations

## 🎯 **Key Results**

| Component | Rotation | Coordinate Mapping |
|-----------|----------|-------------------|
| **Lid Sensor** | 90° CCW | X+→Y+, Y+→X-, Z+→Z+ |
| **Base Sensor** | 90° CW | X+→Y-, Y+→X+, Z+→Z+ |
| **Display** | 90° CCW | (Hardware rotated) |

## 🔧 **Critical for Implementation**

### Base Sensor Coordinate System
```
X+ = BACK of laptop  (opposite of standard)
Y+ = LEFT of laptop  (opposite of standard)  
Z+ = UP              (same as standard)
```

### Lid Sensor Coordinate System  
```
90° counter-clockwise rotation from standard
(Same as previously determined)
```

## 📋 **Tablet Mode Detection**

**Complex configuration required** - each sensor needs different coordinate transformation:

- **Base**: 90° clockwise transformation
- **Lid**: 90° counter-clockwise transformation  
- **Cannot use unified logic** - must handle separately

## ✅ **Verification Status**

- ✅ Base sensor orientation definitively determined
- ✅ Opposite rotations confirmed (Base CW, Lid CCW)
- ✅ Coordinate transformations calculated
- ✅ Test data validates all findings

---
*For complete details see: [SENSOR_CONFIGURATION.md](./SENSOR_CONFIGURATION.md)*