/**
 * Log — custom logging without stdio locking.
 * TohruPhysics用のカスタムロギング — stdioのI/Oロックを使わないの。
 *
 * Formats messages via vsnprintf, writes to an fd via write(2).
 * Bypasses stdio's internal locking (fprintf, fwrite) for signal-safe
 * and lock-free logging from any thread.
 *
 * DESIGN PHILOSOPHY:
 * stdio (fprintf/fwrite) acquires internal locks that can deadlock if a
 * signal handler or interrupt callback tries to log. By writing directly
 * to the fd with write(2) + vsnprintf, we get atomic writes with no
 * lock acquisition. This is the same approach used by production
 * game engines (internal crash loggers, telemetry pipelines).
 *
 * LOG LEVELS (ascending verbosity):
 * ┌──────────┬──────┬──────────────────────────────────────────┐
 * │ Level    │ Val  │ Purpose                                  │
 * ├──────────┼──────┼──────────────────────────────────────────┤
 * │ Error    │ 0    │ Fatal: engine cannot continue             │
 * │ Warn     │ 1    │ Recoverable: degraded behaviour           │
 * │ Info     │ 2    │ Normal: frame rate, body count            │
 * │ Debug    │ 3    │ Verbose: per-contact/constraint           │
 * └──────────┴──────┴──────────────────────────────────────────┘
 *
 * Messages above the configured level are discarded (no format cost).
 *
 * References:
 * - write(2) vs fprintf(3) locking behaviour
 * - vsnprintf(3) for bounded formatting
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#pragma once

#include <stddef.h>

typedef enum {
	LogLevel_Error = 0,
	LogLevel_Warn,
	LogLevel_Info,
	LogLevel_Debug,
	LogLevel_Count
} LogLevel;

void     TohruLogSetLevel(LogLevel Level);
LogLevel TohruLogGetLevel(void);
void     TohruLogSetOutputFD(int FD);

void TohruLogWrite(LogLevel Level, const char *File, int Line, const char *Fmt, ...);

#define TohruLogError(Fmt, ...)   TohruLogWrite(LogLevel_Error, __FILE__, __LINE__, Fmt, ##__VA_ARGS__)
#define TohruLogWarn(Fmt, ...)    TohruLogWrite(LogLevel_Warn,  __FILE__, __LINE__, Fmt, ##__VA_ARGS__)
#define TohruLogInfo(Fmt, ...)    TohruLogWrite(LogLevel_Info,  __FILE__, __LINE__, Fmt, ##__VA_ARGS__)
#define TohruLogDebug(Fmt, ...)   TohruLogWrite(LogLevel_Debug, __FILE__, __LINE__, Fmt, ##__VA_ARGS__)
