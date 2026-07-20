# Section 1.2 — Fixed-Width Foundations & Scalar Math

**File:** `Include/TohruPhysics/Math.h`, `Source/Math/Math.cpp`

## Overview

Defines the scalar mathematics layer for TohruPhysics: a `Real` precision type, NaN/Inf guards, epsilon comparisons, fast trig/inv-sqrt, and wrapping arithmetic — all with the ZII contract.

## API Reference

### Type

| Function | Description |
|----------|-------------|
| `Real` | `typedef double Real`. IEEE 754 64-bit. |

### Mai — NaN/Inf detection

Named after **Mai Thi Yoshimura**.

| Function | Returns | Description |
|----------|---------------|-------------|
| `MaiIsNaN(V)` | returns 0 or 1 | Bit-level NaN detection |
| `MaiIsInf(V)` | returns 0 or 1 | Bit-level ±Inf detection |
| `MaiIsFinite(V)` | returns 0 or 1 | True if not NaN and not Inf |
| `MaiSanitize(V)` | returns 0 on NaN/Inf | Maps non-finite to 0.0 |

Implements IEEE 754 bit-field inspection via `uint64_t` union — no `<cmath>` dependency.

### Nagisa — epsilon comparison

Named after **Nagisa Aoi**.

| Function | Description |
|----------|-------------|
| `NagisaApproxEqual(A, B, Eps)` | `\|A-B\| ≤ Eps`. NaN-safe (returns 0). |
| `NagisaApproxZero(V, Eps)` | `\|V\| ≤ Eps`. |
| `NagisaEqual(A, B)` | Uses `REAL_EPSILON` (2.22e-16). |
| `NagisaIsZero(V)` | Uses `REAL_EPSILON`. |

Negative epsilon is clamped to zero. NaN input returns 0 (not-equal).

### Yuu — clamp, abs, wrap

Named after **Yuu Koito**.

| Function | Description |
|----------|-------------|
| `YuuClamp(V, Lo, Hi)` | Clamp to `[Lo, Hi]`. If `Lo > Hi` → 0 (degenerate guard). NaN → 0. |
| `YuuClamp01(V)` | Clamp to `[0, 1]`. |
| `YuuAbs(V)` | Sign-bit mask — never calls `fabs`. NaN → 0. |
| `YuuMin(A, B)` | NaN → treated as 0. |
| `YuuMax(A, B)` | NaN → treated as 0. |
| `YuuSign(V)` | Returns -1.0, 0.0, or +1.0. NaN → 0. |
| `YuuClamp64(V, Lo, Hi)` | `int64_t` bounds clamp. |
| `YuuWrap64(V, Lo, Hi)` | Modular wrap into `[Lo, Hi]`. Handles negative. |

### Suletta — fast trig & special math

Named after **Suletta Mercury**.

| Function | Accuracy | Description |
|----------|----------|-------------|
| `SulettaSin(V)` | ~±3e-4 | 9th-order polynomial + quadrant reduction |
| `SulettaCos(V)` | ~±3e-4 | `sin(V + PI/2)` |
| `SulettaInvSqrt(V)` | ~±1e-6 | Double magic constant + 3 Newton iterations |
| `SulettaSqrt(V)` | ~±1e-6 | `V * invsqrt(V)` |

All use range reduction to `[0, 2π)` then quadrant mapping to `[0, π/2]`.

## ZII Contract

Every math function in this section guarantees:
- NaN input → returns 0 (or 0-equivalent like 0.0).
- Inf input → returns 0 if the result is undefined; otherwise clamped.
- Degenerate input (zero length, `Lo > Hi`, negative sqrt) → returns 0.
- No `isnan` / `isinf` / `fpclassify` calls — bit-level inspection only.
- Callers **must not** branch-check results.

## Edge Cases Documented

- `SulettaInvSqrt(0)` → 0 (not ∞).
- `YuuClamp(NaN, ...)` → 0.
- `YuuWrap64(V, Lo, Hi)` with `Lo > Hi` → 0.
- `NagisaApproxEqual(NaN, any)` → 0.
- `YuuAbs(NaN)` → 0.
- `SulettaSin(Inf)` → 0.
