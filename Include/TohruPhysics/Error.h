/**
 * Error types for TohruPhysics — init/startup only, not runtime.
 * TohruPhysics用のエラー型 — 初期化/起動専用、実行パスでは使わないの。
 *
 * ZII: runtime functions never return Error. Only init/startup.
 * ZII: ランタイム関数はエラーを返さないの。初期化/起動のみよ。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#pragma once

#include <stddef.h>

// ---------------------------------------------------------------------------
//  0068: ErrorCode — covers all subsystem failure paths.
//  全てのサブシステム障害経路をカバーするの。
// ---------------------------------------------------------------------------
typedef enum {
	ErrorCode_Ok               = 0,
	ErrorCode_OutOfMemory      = 1,
	ErrorCode_InvalidParameter = 2,
	ErrorCode_InitFailed       = 3,
	ErrorCode_Count
} ErrorCode;

// ---------------------------------------------------------------------------
//  Error — code + message for init failures.
//  コード＋メッセージ。
// ---------------------------------------------------------------------------
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

// ---------------------------------------------------------------------------
//  0069: Result<T> — pairs a value with Error for init/startup pipelines.
//  値を Error とペアにする Result テンプレートよ。
// ---------------------------------------------------------------------------
template<typename T>
struct Result {
	T     Value;
	Error Err;

	bool Ok(void)  const { return ErrIsOk(Err); }
	bool Fail(void) const { return ErrIsFail(Err); }
};

// ---------------------------------------------------------------------------
//  Utility: convert ErrorCode to human-readable string.
//  エラーコードを人間可読な文字列に変換するの。
// ---------------------------------------------------------------------------
const char *ErrorCodeToString(ErrorCode Code);
