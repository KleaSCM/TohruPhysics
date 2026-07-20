# Section 1.3 — Linear Algebra & Vector Mathematics

**File:** `Include/TohruPhysics/Vector3.h`, `Source/Vector3/Vector3.cpp`

## Overview

Defines the `Vector3` type and all basic vector operations for TohruPhysics. All functions are ZII-compliant: NaN/Inf/degenerate inputs produce zero vectors or zero scalars.

## Type

```c
typedef struct {
    Real Data[3];
} Vector3;
```

Pure array layout — no constructors, no destructors, no padding guarantees beyond the ABI default. Indexable as `V.Data[0]`, `V.Data[1]`, `V.Data[2]`.

## API Reference

All operations prefixed with **Kanna** (Kanna Kamui).

| Task | Function | Description |
|------|----------|-------------|
| — | `KannaVector3Make(X, Y, Z)` | Constructor returning a new Vector3 |
| 0024 | `KannaVector3Add(A, B)` | Component-wise addition |
| 0025 | `KannaVector3Sub(A, B)` | Component-wise subtraction |
| 0026 | `KannaVector3Scale(V, S)` | Scalar multiplication |
| 0027 | `KannaVector3Dot(A, B)` | Dot product |
| 0028 | `KannaVector3Cross(A, B)` | Cross product |
| 0029 | `KannaVector3LengthSq(V)` | Squared length (avoids sqrt) |
| 0030 | `KannaVector3Normalize(V)` | Unit vector. Zero input → zero output (ZII) |
| 0031 | `KannaVector3Dist(A, B)` | Euclidean distance |
| 0032 | `KannaVector3Equal(A, B)` | Epsilon-bounded component-wise equality |

## ZII Edge Cases

- `KannaVector3Normalize(zero)` → zero vector
- `KannaVector3Cross(A, A)` → zero vector
- `KannaVector3Dist(A, A)` → 0
- `KannaVector3Equal(A, A)` → 1 (within epsilon)
- All functions accept NaN inputs: NaN propagates only through `KannaVector3Add`/`KannaVector3Sub`/`KannaVector3Scale` — other functions guard via `NagisaIsZero`/`SulettaInvSqrt`.
