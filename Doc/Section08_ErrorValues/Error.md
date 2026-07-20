# Section 1.8 — Error Values & Return Code Pipelines

## Overview
Formal `ErrorCode` enum, `Error` struct, `Result<T>` template, and a custom logging system that bypasses stdio I/O locks. Init/startup only — runtime paths use ZII (never fail).

## Files
- `Include/TohruPhysics/Error.h` — ErrorCode, Error, Result<T>, ErrorCodeToString
- `Include/TohruPhysics/Log.h` — LogLevel, TohruLogSetLevel/Write/macros
- `Source/Error/Error.cpp` — ErrorCodeToString implementation
- `Source/Log/Log.cpp` — write() + vsnprintf logging
- `Test/Source/TestError.cpp` — 4 tests
- `Test/Source/TestLog.cpp` — 3 tests

## Types

### ErrorCode
| Code | Value | Meaning |
|------|-------|---------|
| `ErrorCode_Ok` | 0 | Success |
| `ErrorCode_OutOfMemory` | 1 | mmap/mremap allocation failure |
| `ErrorCode_InvalidParameter` | 2 | NULL arena, zero capacity, etc. |
| `ErrorCode_InitFailed` | 3 | Generic init failure |

### Error
| Field | Type | Description |
|-------|------|-------------|
| Code | ErrorCode | Error code |
| Message | char[256] | Human-readable message |

### Result<T>
Templates pairing `T Value` + `Error Err` with `.Ok()` / `.Fail()` helper methods.

## Logging

### Design
- `write()` syscall directly to output FD (default: stderr = FD 2)
- `vsnprintf()` for string formatting (pure computation, no I/O locking)
- Static 4KB per-message buffer
- Monotonic timestamp via `clock_gettime(CLOCK_MONOTONIC, ...)`
- Format: `[LEVEL] seconds.micros file:line: message\n`

### Level suppression
Messages above the configured level are dropped before formatting.

## Dependencies
- `Error.h` — standalone (only `<stddef.h>`)
- `Log.cpp` — `<unistd.h>` (write), `<time.h>` (clock_gettime), `<cstdio>` (vsnprintf)
- `Arena.h` — now includes `Error.h` instead of defining Error inline

## Naming
No cute-girl prefix — Tohru-prefixed for logging (TohruLog), generic for Error.
