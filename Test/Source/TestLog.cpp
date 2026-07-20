/**
 * Unit tests for Log subsystem.
 * ログサブシステムの単体テストね。
 *
 * Tests that logging writes to a pipe without crashing.
 * ログがパイプにクラッシュせず書き込めることをテストするの。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include <TohruPhysics/Log.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define TEST(cond, msg) do { \
	if (!(cond)) { \
		fprintf(stderr, "FAIL: %s (%s:%d)\n", msg, __FILE__, __LINE__); \
		exit(1); \
	} \
} while(0)

static int Passed = 0;

#define RUN_TEST(name, desc) do { \
	fprintf(stderr, "  %-45s ... ", desc); \
	name(); \
	fprintf(stderr, "ok\n"); \
	Passed++; \
} while(0)

static void TestLogLevel(void) {
	TohruLogSetLevel(LogLevel_Debug);
	TEST(TohruLogGetLevel() == LogLevel_Debug, "set/get Debug");

	TohruLogSetLevel(LogLevel_Error);
	TEST(TohruLogGetLevel() == LogLevel_Error, "set/get Error");

	// Reset to default for other tests.
	TohruLogSetLevel(LogLevel_Info);
}

static void TestLogWrite(void) {
	// Redirect to a pipe and verify output.
	// パイプにリダイレクトして出力を確認するの。
	int PipeFD[2];
	int Ret = pipe(PipeFD);
	TEST(Ret == 0, "pipe created");

	TohruLogSetOutputFD(PipeFD[1]);
	TohruLogSetLevel(LogLevel_Debug);

	// Write a log message.
	TohruLogWrite(LogLevel_Info, "test.c", 42, "hello %d", 123);
	close(PipeFD[1]);

	// Read back.
	char Buf[1024] = {0};
	ssize_t N = read(PipeFD[0], Buf, sizeof(Buf) - 1);
	close(PipeFD[0]);

	TEST(N > 0, "log output written");
	TEST(strstr(Buf, "hello 123") != NULL, "log contains message");
	TEST(strstr(Buf, "test.c") != NULL, "log contains filename");
	TEST(strstr(Buf, "INFO") != NULL || strstr(Buf, "ERRO") != NULL
		|| strstr(Buf, "WARN") != NULL || strstr(Buf, "DBUG") != NULL,
		"log contains level tag");

	// Reset output to stderr.
	TohruLogSetOutputFD(2);
}

static void TestLogMacros(void) {
	// Macros should compile and not crash.
	// マクロがコンパイルできてクラッシュしないこと。
	TohruLogSetLevel(LogLevel_Debug);

	TohruLogError("error test %d", 1);
	TohruLogWarn("warn test %d", 2);
	TohruLogInfo("info test %d", 3);
	TohruLogDebug("debug test %d", 4);

	// Lower level to suppress debug output.
	TohruLogSetLevel(LogLevel_Warn);
	TohruLogDebug("should not appear %d", 99);
	TohruLogInfo("should not appear %d", 99);

	TohruLogSetLevel(LogLevel_Info);
}

int main(void) {
	fprintf(stderr, "=== TestLog ===\n");

	RUN_TEST(TestLogLevel, "SetLevel / GetLevel");
	RUN_TEST(TestLogWrite, "LogWrite to pipe");
	RUN_TEST(TestLogMacros, "Log macros compile and run");

	fprintf(stderr, "\n=== %d passed, 0 failed ===\n", Passed);
	return 0;
}
