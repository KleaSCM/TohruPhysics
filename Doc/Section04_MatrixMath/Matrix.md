# Section 1.4 — Matrix Operations

**File:** `Include/TohruPhysics/Matrix.h`, `Source/Matrix/Matrix.cpp`

## Overview

Defines `Matrix3x3` and `Matrix4x4` types with full row-major layout and standard linear algebra operations. All functions are ZII-compliant.

## Types

| Type | Layout | Elements |
|------|--------|----------|
| `Matrix3x3` | Row-major `Real Data[9]` | M[0..2]=row0, M[3..5]=row1, M[6..8]=row2 |
| `Matrix4x4` | Row-major `Real Data[16]` | M[0..3]=row0, ..., M[12..15]=row3 |

## API Reference

### Miorine — Matrix3x3

Named after **Miorine Rembran**.

| Task | Function | Description |
|------|----------|-------------|
| 0034 | `MiorineMatrix3x3Identity()` | Identity matrix |
| — | `MiorineMatrix3x3Make(m00..m22)` | Constructor from 9 components |
| 0035 | `MiorineMatrix3x3Mul(A, B)` | Matrix-matrix multiply |
| 0036 | `MiorineMatrix3x3VecMul(M, V)` | Matrix-vector multiply |
| 0037 | `MiorineMatrix3x3Transpose(M)` | Transpose (returns new matrix) |
| 0038 | `MiorineMatrix3x3Inverse(M)` | Inverse. Singular → zero matrix (ZII) |
| — | `MiorineMatrix3x3Determinant(M)` | Determinant |

### Anisphia — Matrix4x4

Named after **Anisphia Wynn Palettia**.

| Task | Function | Description |
|------|----------|-------------|
| 0040 | `AnisphiaMatrix4x4Identity()` | Identity matrix |
| — | `AnisphiaMatrix4x4Make(m00..m33)` | Constructor from 16 components |
| 0041 | `AnisphiaMatrix4x4Mul(A, B)` | Matrix-matrix multiply |
| 0042 | `AnisphiaMatrix4x4VecMul(M, V)` | Affine transform (V as (x,y,z,1)), perspective divide |
| 0043 | `AnisphiaMatrix4x4Inverse(M)` | Inverse via cofactors. Singular → zero matrix (ZII) |
| — | `AnisphiaMatrix4x4Determinant(M)` | Determinant via Laplace expansion |

## ZII Edge Cases

- `MiorineMatrix3x3Inverse(singular)` → zero matrix
- `AnisphiaMatrix4x4Inverse(singular)` → zero matrix
- `MiorineMatrix3x3Determinant(zero)` → 0
- `AnisphiaMatrix4x4VecMul` with w=0 → zero vector (perspective divide by zero guard)
