# Section 1.7 — Spatial Reference Frames & Transform Components

## Overview
`Transform` struct combining a `Vector3` position and `Quaternion` rotation, with point/direction transformation and compose/decompose operations. All functions use the **Kaede** prefix.

## Files
- `Include/TohruPhysics/Transform.h` — `Transform` type + 6 function declarations
- `Source/Transform/Transform.cpp` — implementation
- `Test/Source/TestTransform.cpp` — 6 unit tests

## Functions (Kaede)

| # | Function | Purpose |
|---|----------|---------|
| — | `KaedeTransformIdentity` | Zero position + identity rotation |
| 0063 | `KaedeTransformPoint` | Local→world: P' = R·P + T |
| 0064 | `KaedeInverseTransformPoint` | World→local: P = R⁻¹·(P' − T) |
| 0065 | `KaedeTransformDirection` | Rotate direction (no translation) |
| 0066 | `KaedeInverseTransformDirection` | Inverse rotate direction |
| 0067 | `KaedeTransformCombine` | Compose: C = A·B |

## Dependencies
- `Vector3.h` — position, add/sub/scale
- `Quaternion.h` — rotation, conjugate, rotate vector

## Implementation Details

### TransformPoint
P' = R·P + T using `EuphylliaQuaternionRotateVector` + `KannaVector3Add`.

### InverseTransformPoint
P = R⁻¹·(P' − T) using conjugate (inverse for unit quaternions): `KannaVector3Sub` then `EuphylliaQuaternionRotateVector` with conjugate.

### TransformDirection / InverseTransformDirection
Pure rotation via `EuphylliaQuaternionRotateVector` / conjugate variant.

### TransformCombine
C.P = A.P + A.R · B.P, C.R = A.R · B.R — matches standard hierarchical transform composition.

### QuaternionRotateVector (Euphyllia)
Added as a public API: v' = v + 2·w·cross(q.xyz, v) + 2·cross(q.xyz, cross(q.xyz, v)). Avoids matrix construction.

## Naming
**Kaede Johan Nouvelle** — 6 Transform operations + 1 Quaternion helper.
