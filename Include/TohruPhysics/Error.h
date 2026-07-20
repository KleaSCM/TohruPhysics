/**
 * Error — startup-only result type.
 * TohruPhysics用のエラー型 — 初期化専用ね。
 *
 * ErrorCode + fixed-size message. Used only for init/startup paths where
 * failure is possible (mmap OOM, file not found). Runtime functions never
 * return Error — they return zero values (ZII).
 *
 * DESIGN PHILOSOPHY:
 * Physics engines must not branch on every operation. By confining Error
 * to startup paths only, runtime hot paths stay branch-free. The Result<T>
 * template pairs a value with Error for init pipelines that need to report
 * which subsystem failed without exceptions.
 *
 * ERROR CODE TABLE:
 * ┌──────────────────────┬──────┬────────────────────────────────────┐
 * │ Code                 │ Val  │ Meaning                            │
 * ├──────────────────────┼──────┼────────────────────────────────────┤
 * │ ErrorCode_Ok         │ 0    │ No error                           │
 * │ ErrorCode_OutOfMemory│ 1    │ mmap/mremap failed                 │
 * │ ErrorCode_InvalidParam│ 2    │ Null pointer or bad argument       │
 * │ ErrorCode_InitFailed │ 3    │ General initialisation failure      │
 * └──────────────────────┴──────┴────────────────────────────────────┘
 *
 * References:
 * - ZII pattern (see §3 of style guide)
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#pragma once

#include <stddef.h>

typedef enum {
	ErrorCode_Ok               = 0,
	ErrorCode_OutOfMemory      = 1,
	ErrorCode_InvalidParameter = 2,
	ErrorCode_InitFailed       = 3,
	ErrorCode_Count
} ErrorCode;

typedef struct {
	ErrorCode Code;
	char      Message[256];
} Error;

static inline Error ErrOk(void) {
	Error E = {ErrorCode_Ok, {0}};
	return E;
}

static inline Error ErrMake(ErrorCode Code) {
	Error E = {Code, {0}};
	return E;
}

#define ErrIsOk(E)   ((E).Code == ErrorCode_Ok)
#define ErrIsFail(E) ((E).Code != ErrorCode_Ok)

template<typename T>
struct Result {
	T     Value;
	Error Err;

	bool Ok(void)  const { return ErrIsOk(Err); }
	bool Fail(void) const { return ErrIsFail(Err); }
};

const char *ErrorCodeToString(ErrorCode Code);
