/**
 * Custom logging for TohruPhysics — no stdio I/O locking.
 * TohruPhysics用のカスタムロギング — stdioのI/Oロックを使わないの。
 *
 * Uses write() syscall + vsnprintf for formatting. No fprintf, no stdio locks.
 * write()システムコール＋vsnprintfを使うの。fprintfは使わず、stdioロックもなしよ。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#pragma once

#include <stddef.h>

// ---------------------------------------------------------------------------
//  0072: LogLevel — severity.
//  ログレベル — 重大度。
// ---------------------------------------------------------------------------
typedef enum {
	LogLevel_Error = 0,
	LogLevel_Warn,
	LogLevel_Info,
	LogLevel_Debug,
	LogLevel_Count
} LogLevel;

// ---------------------------------------------------------------------------
//  Configuration
// ---------------------------------------------------------------------------
void     TohruLogSetLevel(LogLevel Level);
LogLevel TohruLogGetLevel(void);
void     TohruLogSetOutputFD(int FD);   // default: 2 (stderr)

// ---------------------------------------------------------------------------
//  0072: Core write — formats and writes to the output FD.
//  書式化して出力FDに書き込むの。
// ---------------------------------------------------------------------------
void TohruLogWrite(LogLevel Level, const char *File, int Line, const char *Fmt, ...);

// ---------------------------------------------------------------------------
//  Convenience macros
// ---------------------------------------------------------------------------
#define TohruLogError(Fmt, ...)   TohruLogWrite(LogLevel_Error, __FILE__, __LINE__, Fmt, ##__VA_ARGS__)
#define TohruLogWarn(Fmt, ...)    TohruLogWrite(LogLevel_Warn,  __FILE__, __LINE__, Fmt, ##__VA_ARGS__)
#define TohruLogInfo(Fmt, ...)    TohruLogWrite(LogLevel_Info,  __FILE__, __LINE__, Fmt, ##__VA_ARGS__)
#define TohruLogDebug(Fmt, ...)   TohruLogWrite(LogLevel_Debug, __FILE__, __LINE__, Fmt, ##__VA_ARGS__)
