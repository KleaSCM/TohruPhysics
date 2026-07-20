/**
 * Unit tests for Error code and Result types.
 * エラーコードとResult型の単体テストね。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include <TohruPhysics/Error.h>
#include <stdio.h>
#include <stdlib.h>

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

static void TestErrOk(void) {
	Error E = ErrOk();
	TEST(ErrIsOk(E), "ok is ok");
	TEST(!ErrIsFail(E), "ok is not fail");
	TEST(E.Code == ErrorCode_Ok, "code == Ok");
}

static void TestErrMake(void) {
	Error E = ErrMake(ErrorCode_OutOfMemory);
	TEST(!ErrIsOk(E), "OOM is not ok");
	TEST(ErrIsFail(E), "OOM is fail");
	TEST(E.Code == ErrorCode_OutOfMemory, "code == OOM");

	E = ErrMake(ErrorCode_InvalidParameter);
	TEST(E.Code == ErrorCode_InvalidParameter, "code == InvalidParameter");

	E = ErrMake(ErrorCode_InitFailed);
	TEST(E.Code == ErrorCode_InitFailed, "code == InitFailed");
}

static void TestErrorCodeToString(void) {
	TEST(ErrorCodeToString(ErrorCode_Ok) != NULL, "Ok string");
	TEST(ErrorCodeToString(ErrorCode_OutOfMemory) != NULL, "OOM string");
	TEST(ErrorCodeToString((ErrorCode)999) != NULL, "unknown string");
}

struct Dummy {
	int X;
	int Y;
};

static void TestResult(void) {
	Result<int> R;
	R.Value = 42;
	R.Err = ErrOk();
	TEST(R.Ok(), "result ok");
	TEST(R.Value == 42, "result value");

	Result<struct Dummy> RD;
	RD.Value.X = 10;
	RD.Value.Y = 20;
	RD.Err = ErrMake(ErrorCode_InitFailed);
	TEST(RD.Fail(), "result fail");
	TEST(RD.Err.Code == ErrorCode_InitFailed, "result fail code");
}

int main(void) {
	fprintf(stderr, "=== TestError ===\n");

	RUN_TEST(TestErrOk, "ErrOk / ErrIsOk / ErrIsFail");
	RUN_TEST(TestErrMake, "ErrMake with ErrorCode values");
	RUN_TEST(TestErrorCodeToString, "ErrorCodeToString");
	RUN_TEST(TestResult, "Result<T> template");

	fprintf(stderr, "\n=== %d passed, 0 failed ===\n", Passed);
	return 0;
}
