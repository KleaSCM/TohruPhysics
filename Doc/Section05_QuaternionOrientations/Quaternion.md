# Section 1.5 — Quaternion Orientations

## Overview
Defines the `Quaternion` type (4-element `Real` array `Data[4] = (X, Y, Z, W)`) and 9 operations for representing and composing 3D rotations. All functions use the **Euphyllia** prefix.

## Files
- `Include/TohruPhysics/Quaternion.h` — type definition, function declarations
- `Source/Quaternion/Quaternion.cpp` — implementation
- `Test/Source/TestQuaternion.cpp` — 9 unit tests

## Functions

| # | Function | Purpose | ZII Notes |
|---|----------|---------|-----------|
| 0045 | `EuphylliaQuaternionIdentity` | Returns (0,0,0,1) | Always valid |
| 0045 | `EuphylliaQuaternionMake` | Construct from 4 scalars | Any input valid |
| 0052 | `EuphylliaQuaternionDot` | Dot product (A·B) | Works for any input |
| 0052 | `EuphylliaQuaternionLengthSq` | Squared length (Q·Q) | Works for any input |
| 0046 | `EuphylliaQuaternionNormalize` | Unit-length quaternion | Zero → identity |
| 0047 | `EuphylliaQuaternionMul` | Hamilton product (A*B) | Works for any input |
| 0048 | `EuphylliaQuaternionToMatrix3x3` | Quat → rotation matrix | Works for any input |
| 0049 | `EuphylliaMatrix3x3ToQuaternion` | Rotation matrix → quat | Handles all cases |
| 0050 | `EuphylliaQuaternionSlerp` | Spherical linear interpolation | A=B → A (no div-by-zero) |
| 0051 | `EuphylliaQuaternionConjugate` | Conjugate (−X,−Y,−Z,W) | Works for any input |

## Implementation Details

### Normalization
Uses `SulettaInvSqrt` for fast inverse square root. If length² is zero (via `NagisaIsZero`), returns identity quaternion (ZII).

### Matrix ↔ Quaternion
- `QuaternionToMatrix3x3`: Standard formula using doubled products. No special-case needed.
- `Matrix3x3ToQuaternion`: Four-branch trace-based extraction. Handles identity, singular, and general rotation matrices.

### Slerp
- Computes ω via an internal `AcosApprox` (5th-order asin expansion).
- Short-arc selection: negates B if dot < 0.
- Linear lerp fallback when cosΩ > 0.9999 (small-angle regime).
- ZII: A=B → returns A (t=0 → A, t=1 → B).

## Dependencies
- `Math.h` — `Real`, `NagisaIsZero`, `NagisaApproxZero`, `SulettaInvSqrt`, `SulettaSqrt`, `SulettaSin`, `REAL_ZERO`
- `Matrix.h` — `Matrix3x3`

## Naming
**Euphyllia Magenta** — 10 operations.
