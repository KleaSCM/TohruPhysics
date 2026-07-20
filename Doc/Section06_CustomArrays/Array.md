# Section 1.6 — Custom Arrays & Contiguous Data Containers

## Overview
Arena-backed `Array` type with bounds-checked access, search, sort, copy, and a flat `Grid2D` wrapper. All functions use the **Tilty** prefix.

## Files
- `Include/TohruPhysics/Array.h` — `Array` + `Grid2D` type definitions, function declarations
- `Source/Array/Array.cpp` — implementation
- `Test/Source/TestArray.cpp` — 12 unit tests

## Types

### `Array`
| Field | Description |
|-------|-------------|
| `MemArena *` | Backing arena (may be NULL after destroy) |
| `Data *` | Raw element storage |
| `Length` | Number of live elements |
| `Capacity` | Allocated element slots |
| `ElemSize` | Bytes per element |

### `Grid2D`
Wraps `Array` with `Width` and `Height`; elements stored row-major at `Index = Y * Width + X`.

## Functions (Tilty)

| # | Function | Purpose | ZII Notes |
|---|----------|---------|-----------|
| 0053 | `TiltyArrayInit` | Init with pre-allocation | Zero cap → default 4 |
| 0053 | `TiltyArrayDestroy` | Teardown | NULLs fields |
| 0055 | `TiltyArrayGet` | Indexed read | OOB → `TohruZeroBlock` |
| 0055 | `TiltyArraySet` | Indexed write | OOB → no-op |
| 0055 | `TiltyArrayPush` | Append element | Auto-grows; OOM → ZeroBlock |
| 0055 | `TiltyArrayPop` | Remove last | Empty → ZeroBlock |
| 0061 | `TiltyArrayReserve` | Pre-allocate capacity | OOM → no-op |
| 0056 | `TiltyArraySearch` | Linear search (O(n)) | Not found → -1 |
| 0056 | `TiltyArrayBinarySearch` | Binary search (O(log n)) | Not found → -1 |
| 0057 | `TiltyArrayCopy` | Indexed block copy | Grows Dst if needed |
| 0058 | `TiltyArrayContains` | Membership test | Returns 0/1 |
| 0059 | `TiltyArraySort` | In-place quick sort | Empty/single → no-op |
| 0060 | `TiltyGrid2DInit` | Init flat grid | Allocates W×H×ElemSize |
| 0060 | `TiltyGrid2DGet` | (X,Y) access | OOB → ZeroBlock |
| 0060 | `TiltyGrid2DGetFlat` | Flat index access | OOB → ZeroBlock |

## Implementation Details

### Growth strategy
Double capacity on each resize (starting from 4). Allocation comes from the backing arena via `KobayashiAlloc` — the arena handles `mremap`.

### Sort algorithm
Quick sort with median-of-3 pivot selection and Lomuto partition. Byteswaps via `SwapElems` (uint64_t fast path for aligned data, byte loop fallback). Recursion depth is stack-bound — no protection against degenerate inputs (production TODO: introspective sort or iterative quick sort).

### Grid2D
Thin wrapper: `TiltyGrid2DInit` creates an `Array` with `Length = Width * Height`. `TiltyGrid2DGet` computes `Index = Y * Width + X`. Grid2D does not support resize.

## Dependencies
- `Arena.h` — `KobayashiAlloc`, `TohruZeroBlock`

## Naming
**Tilty Claret** — 7 Array + 3 Grid2D operations.
