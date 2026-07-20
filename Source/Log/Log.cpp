/**
 * Log implementation — write() syscall, no stdio I/O locks.
 * ログ実装 — write()システムコール、stdio I/Oロックなし。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include <TohruPhysics/Log.h>
#include <stdarg.h>
#include <unistd.h>
#include <time.h>
#include <cstdio>

// ---------------------------------------------------------------------------
//  Internal state
// ---------------------------------------------------------------------------

static LogLevel GlobalLevel = LogLevel_Info;
static int      OutputFD    = 2;  // stderr

// ---------------------------------------------------------------------------
//  Level tags
// ---------------------------------------------------------------------------

static const char *LevelTag(LogLevel L) {
	switch (L) {
	case LogLevel_Error: return "ERRO";
	case LogLevel_Warn:  return "WARN";
	case LogLevel_Info:  return "INFO";
	case LogLevel_Debug: return "DBUG";
	default:             return "????";
	}
}

// ---------------------------------------------------------------------------
//  Public API
// ---------------------------------------------------------------------------

void TohruLogSetLevel(LogLevel Level) {
	GlobalLevel = Level;
}

LogLevel TohruLogGetLevel(void) {
	return GlobalLevel;
}

void TohruLogSetOutputFD(int FD) {
	OutputFD = FD;
}

void TohruLogWrite(LogLevel Level, const char *File, int Line, const char *Fmt, ...) {
	if (Level > GlobalLevel) return;

	// Format message body into a static buffer.
	// メッセージ本文を静的バッファに書式化するの。
	char Buf[4096];
	int  Pos = 0;

	// Monotonic timestamp (seconds.nanos since boot).
	// 起動からの単調増加タイムスタンプ。
	struct timespec TS;
	clock_gettime(CLOCK_MONOTONIC, &TS);

	Pos += snprintf(Buf + Pos, (size_t)(sizeof(Buf) - Pos),
		"[%s] %ld.%06ld %s:%d: ",
		LevelTag(Level),
		(long)TS.tv_sec, (long)(TS.tv_nsec / 1000),
		File, Line);

	// Append formatted message.
	// 書式化メッセージを追加するの。
	va_list Args;
	va_start(Args, Fmt);
	Pos += vsnprintf(Buf + Pos, (size_t)(sizeof(Buf) - Pos), Fmt, Args);
	va_end(Args);

	// Ensure newline-terminated.
	// 改行で終わるようにするの。
	if (Pos > 0 && Pos < (int)sizeof(Buf) - 1) {
		if (Buf[Pos - 1] != '\n') {
			Buf[Pos] = '\n';
			Pos++;
		}
	}

	// Write to output FD — no stdio locks.
	// 出力FDに書き込む — stdioロックなし。
	write(OutputFD, Buf, (size_t)Pos);
}
