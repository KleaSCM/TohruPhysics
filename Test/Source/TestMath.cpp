/**
 * Unit tests for TohruPhysics scalar math.
 * TohruPhysicsのスカラー数学の単体テストね。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include <TohruPhysics/Math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define TEST(cond, msg) do { \
	if (!(cond)) { \
		fprintf(stderr, "FAIL: %s (%s:%d)\n", msg, __FILE__, __LINE__); \
		exit(1); \
	} \
} while(0)

#define TEST_NAMED(cond, name) do { \
	if (!(cond)) { \
		fprintf(stderr, "FAIL: %s\n", name); \
		exit(1); \
	} \
} while(0)

static int Passed = 0;
static int Failed = 0;

#define RUN_TEST(name) do { \
	fprintf(stderr, "  %s ... ", #name); \
	name(); \
	fprintf(stderr, "ok\n"); \
	Passed++; \
} while(0)

// Raw double bit-cast for NaN/Inf construction.
typedef union { uint64_t U; double F; } F64Bits;

static double MakeNaN(void) {
	F64Bits B;
	B.U = 0x7FF0000000000001ULL; // quiet NaN
	return B.F;
}

static double MakePosInf(void) {
	F64Bits B;
	B.U = 0x7FF0000000000000ULL;
	return B.F;
}

static double MakeNegInf(void) {
	F64Bits B;
	B.U = 0xFFF0000000000000ULL;
	return B.F;
}

// ===========================================================================
//  Mai — NaN/Inf detection
// ===========================================================================

static void TestMaiIsNaN(void) {
	TEST(MaiIsNaN(MakeNaN()) == 1, "NaN detection");
	TEST(MaiIsNaN(MakePosInf()) == 0, "Inf not NaN");
	TEST(MaiIsNaN(MakeNegInf()) == 0, "-Inf not NaN");
	TEST(MaiIsNaN(0.0) == 0, "0 not NaN");
	TEST(MaiIsNaN(1.0) == 0, "1 not NaN");
	TEST(MaiIsNaN(-1.0) == 0, "-1 not NaN");
}

static void TestMaiIsInf(void) {
	TEST(MaiIsInf(MakePosInf()) == 1, "+Inf detection");
	TEST(MaiIsInf(MakeNegInf()) == 1, "-Inf detection");
	TEST(MaiIsInf(MakeNaN()) == 0, "NaN not Inf");
	TEST(MaiIsInf(0.0) == 0, "0 not Inf");
}

static void TestMaiIsFinite(void) {
	TEST(MaiIsFinite(0.0) == 1, "0 finite");
	TEST(MaiIsFinite(1e308) == 1, "large finite");
	TEST(MaiIsFinite(MakePosInf()) == 0, "+Inf not finite");
	TEST(MaiIsFinite(MakeNegInf()) == 0, "-Inf not finite");
	TEST(MaiIsFinite(MakeNaN()) == 0, "NaN not finite");
}

static void TestMaiSanitize(void) {
	TEST(MaiSanitize(MakeNaN()) == REAL_ZERO, "NaN -> 0");
	TEST(MaiSanitize(MakePosInf()) == REAL_ZERO, "+Inf -> 0");
	TEST(MaiSanitize(MakeNegInf()) == REAL_ZERO, "-Inf -> 0");
	TEST(MaiSanitize(42.0) == 42.0, "42 unchanged");
	TEST(MaiSanitize(-3.14) == -3.14, "-3.14 unchanged");
}

// ===========================================================================
//  Nagisa — epsilon comparison
// ===========================================================================

static void TestNagisaApproxEqual(void) {
	TEST(NagisaApproxEqual(1.0, 1.0, 1e-9) == 1, "equal");
	TEST(NagisaApproxEqual(1.0, 1.0 + 1e-10, 1e-9) == 1, "within eps");
	TEST(NagisaApproxEqual(1.0, 2.0, 0.5) == 0, "outside eps");
	TEST(NagisaApproxEqual(MakeNaN(), 1.0, 1e-9) == 0, "NaN A -> 0");
	TEST(NagisaApproxEqual(1.0, MakeNaN(), 1e-9) == 0, "NaN B -> 0");
	TEST(NagisaApproxEqual(0.0, 0.0, -1.0) == 1, "negative eps clamped");
}

static void TestNagisaApproxZero(void) {
	TEST(NagisaApproxZero(0.0, 1e-9) == 1, "zero");
	TEST(NagisaApproxZero(1e-12, 1e-9) == 1, "tiny");
	TEST(NagisaApproxZero(1.0, 1e-9) == 0, "non-zero");
	TEST(NagisaApproxZero(MakeNaN(), 1e-9) == 0, "NaN -> 0");
}

static void TestNagisaEqualIsZero(void) {
	TEST(NagisaEqual(1.5, 1.5) == 1, "epsilon equal");
	TEST(NagisaEqual(1.0, 1.0 + REAL_EPSILON * 4.0) == 0, "boundary");
	TEST(NagisaIsZero(0.0) == 1, "is zero");
	TEST(NagisaIsZero(REAL_EPSILON * 0.5) == 1, "is tiny");
}

// ===========================================================================
//  Yuu — clamp, abs, wrap, bounds
// ===========================================================================

static void TestYuuClamp(void) {
	TEST(YuuClamp(5.0, 0.0, 10.0) == 5.0, "in range");
	TEST(YuuClamp(-1.0, 0.0, 10.0) == 0.0, "below");
	TEST(YuuClamp(15.0, 0.0, 10.0) == 10.0, "above");
	TEST(YuuClamp(MakeNaN(), 0.0, 10.0) == 0.0, "NaN -> 0");
	// Degenerate Lo>Hi
	TEST(YuuClamp(5.0, 10.0, 0.0) == 0.0, "Lo>Hi -> 0");
}

static void TestYuuClamp01(void) {
	TEST(YuuClamp01(0.5) == 0.5, "in range");
	TEST(YuuClamp01(-0.5) == 0.0, "below 0");
	TEST(YuuClamp01(1.5) == 1.0, "above 1");
}

static void TestYuuAbs(void) {
	TEST(YuuAbs(5.0) == 5.0, "positive");
	TEST(YuuAbs(-5.0) == 5.0, "negative");
	TEST(YuuAbs(0.0) == 0.0, "zero");
	TEST(YuuAbs(MakeNaN()) == 0.0, "NaN -> 0");
}

static void TestYuuMinMax(void) {
	TEST(YuuMin(3.0, 5.0) == 3.0, "min");
	TEST(YuuMin(-1.0, -5.0) == -5.0, "min neg");
	TEST(YuuMax(3.0, 5.0) == 5.0, "max");
	TEST(YuuMax(-1.0, -5.0) == -1.0, "max neg");
	TEST(YuuMin(MakeNaN(), 5.0) == 0.0, "min NaN -> 0");
	TEST(YuuMax(MakeNaN(), 5.0) == 5.0, "max NaN -> other");
}

static void TestYuuSign(void) {
	TEST(YuuSign(5.0) == 1.0, "positive sign");
	TEST(YuuSign(-5.0) == -1.0, "negative sign");
	TEST(YuuSign(0.0) == 0.0, "zero sign");
	TEST(YuuSign(MakeNaN()) == 0.0, "NaN sign -> 0");
}

static void TestYuuClamp64(void) {
	TEST(YuuClamp64(5, 0, 10) == 5, "i64 in range");
	TEST(YuuClamp64(-1, 0, 10) == 0, "i64 below");
	TEST(YuuClamp64(15, 0, 10) == 10, "i64 above");
	TEST(YuuClamp64(5, 10, 0) == 0, "i64 Lo>Hi -> 0");
}

static void TestYuuWrap64(void) {
	TEST(YuuWrap64(5, 0, 10) == 5, "wrap in range");
	TEST(YuuWrap64(12, 0, 10) == 2, "wrap above");
	TEST(YuuWrap64(-3, 0, 10) == 7, "wrap negative");
	TEST(YuuWrap64(5, 10, 0) == 0, "wrap Lo>Hi -> 0");
	TEST(YuuWrap64(0, 0, 0) == 0, "wrap zero range");
}

// ===========================================================================
//  Suletta — fast trig and math
// ===========================================================================

static void TestSulettaSin(void) {
	TEST(NagisaApproxZero(SulettaSin(0.0), 1e-6), "sin(0) ~ 0");
	TEST(NagisaApproxEqual(SulettaSin(REAL_PI_HALF), 1.0, 1e-4), "sin(PI/2) ~ 1");
	TEST(NagisaApproxZero(SulettaSin(REAL_PI), 1e-4), "sin(PI) ~ 0");
	TEST(NagisaApproxEqual(SulettaSin(3.0 * REAL_PI_HALF), -1.0, 1e-4), "sin(3PI/2) ~ -1");
	TEST(SulettaSin(MakeNaN()) == 0.0, "sin(NaN) -> 0");
	TEST(SulettaSin(MakePosInf()) == 0.0, "sin(Inf) -> 0");
}

static void TestSulettaCos(void) {
	TEST(NagisaApproxEqual(SulettaCos(0.0), 1.0, 1e-4), "cos(0) ~ 1");
	TEST(NagisaApproxZero(SulettaCos(REAL_PI_HALF), 1e-4), "cos(PI/2) ~ 0");
	TEST(NagisaApproxEqual(SulettaCos(REAL_PI), -1.0, 1e-4), "cos(PI) ~ -1");
	TEST(NagisaApproxZero(SulettaCos(3.0 * REAL_PI_HALF), 1e-4), "cos(3PI/2) ~ 0");
}

static void TestSulettaInvSqrt(void) {
	TEST(NagisaApproxEqual(SulettaInvSqrt(4.0), 0.5, 1e-6), "invsqrt(4) ~ 0.5");
	TEST(NagisaApproxEqual(SulettaInvSqrt(1.0), 1.0, 1e-6), "invsqrt(1) ~ 1");
	Real Ref = 1.0 / sqrt(2.0);
	TEST(NagisaApproxEqual(SulettaInvSqrt(2.0), Ref, 1e-6), "invsqrt(2)");
	TEST(SulettaInvSqrt(0.0) == 0.0, "invsqrt(0) -> 0");
	TEST(SulettaInvSqrt(-1.0) == 0.0, "invsqrt(-1) -> 0");
	TEST(SulettaInvSqrt(MakeNaN()) == 0.0, "invsqrt(NaN) -> 0");
}

static void TestSulettaSqrt(void) {
	TEST(NagisaApproxEqual(SulettaSqrt(4.0), 2.0, 1e-6), "sqrt(4) ~ 2");
	TEST(NagisaApproxEqual(SulettaSqrt(9.0), 3.0, 1e-6), "sqrt(9) ~ 3");
	TEST(SulettaSqrt(0.0) == 0.0, "sqrt(0) -> 0");
	TEST(SulettaSqrt(-1.0) == 0.0, "sqrt(-1) -> 0");
	TEST(SulettaSqrt(MakeNaN()) == 0.0, "sqrt(NaN) -> 0");
	TEST(SulettaSqrt(MakePosInf()) == 0.0, "sqrt(Inf) -> 0");
}

// ===========================================================================
//  Suletta — new trig and math (Tan, Atan2, Floor, Ceil, Round, Fmod, Pow)
// ===========================================================================

static void TestSulettaTan(void) {
	TEST(NagisaApproxZero(SulettaTan(0.0), 1e-6), "tan(0) ~ 0");
	TEST(NagisaApproxEqual(SulettaTan(REAL_PI / 4.0), 1.0, 1e-4), "tan(PI/4) ~ 1");
	TEST(NagisaApproxEqual(SulettaTan(-REAL_PI / 4.0), -1.0, 1e-4), "tan(-PI/4) ~ -1");
	TEST(NagisaApproxZero(SulettaTan(REAL_PI), 1e-4), "tan(PI) ~ 0");
	TEST(SulettaTan(MakeNaN()) == 0.0, "tan(NaN) -> 0");
	TEST(SulettaTan(MakePosInf()) == 0.0, "tan(Inf) -> 0");
	TEST(SulettaTan(MakeNegInf()) == 0.0, "tan(-Inf) -> 0");
}

static void TestSulettaAtan2(void) {
	TEST(NagisaApproxZero(SulettaAtan2(0.0, 1.0), 1e-6), "atan2(0,1) ~ 0");
	TEST(NagisaApproxEqual(SulettaAtan2(1.0, 0.0), REAL_PI_HALF, 1e-2), "atan2(1,0) ~ PI/2");
	TEST(NagisaApproxEqual(SulettaAtan2(0.0, -1.0), REAL_PI, 2e-2), "atan2(0,-1) ~ PI");
	TEST(NagisaApproxEqual(SulettaAtan2(-1.0, 0.0), -REAL_PI_HALF, 1e-2), "atan2(-1,0) ~ -PI/2");
	TEST(NagisaApproxEqual(SulettaAtan2(1.0, 1.0), REAL_PI / 4.0, 2e-2), "atan2(1,1) ~ PI/4");
	TEST(SulettaAtan2(MakeNaN(), 1.0) == 0.0, "atan2(NaN,1) -> 0");
	TEST(SulettaAtan2(1.0, MakeNaN()) == 0.0, "atan2(1,NaN) -> 0");
}

static void TestSulettaFloor(void) {
	TEST(NagisaApproxEqual(SulettaFloor(3.7), 3.0, 1e-9), "floor(3.7) ~ 3");
	TEST(NagisaApproxEqual(SulettaFloor(-3.7), -4.0, 1e-9), "floor(-3.7) ~ -4");
	TEST(NagisaApproxEqual(SulettaFloor(0.0), 0.0, 1e-9), "floor(0) ~ 0");
	TEST(NagisaApproxEqual(SulettaFloor(5.0), 5.0, 1e-9), "floor(5) ~ 5");
	TEST(SulettaFloor(MakeNaN()) == 0.0, "floor(NaN) -> 0");
}

static void TestSulettaCeil(void) {
	TEST(NagisaApproxEqual(SulettaCeil(3.2), 4.0, 1e-9), "ceil(3.2) ~ 4");
	TEST(NagisaApproxEqual(SulettaCeil(-3.2), -3.0, 1e-9), "ceil(-3.2) ~ -3");
	TEST(NagisaApproxEqual(SulettaCeil(0.0), 0.0, 1e-9), "ceil(0) ~ 0");
	TEST(NagisaApproxEqual(SulettaCeil(5.0), 5.0, 1e-9), "ceil(5) ~ 5");
	TEST(SulettaCeil(MakeNaN()) == 0.0, "ceil(NaN) -> 0");
}

static void TestSulettaRound(void) {
	TEST(NagisaApproxEqual(SulettaRound(3.4), 3.0, 1e-9), "round(3.4) ~ 3");
	TEST(NagisaApproxEqual(SulettaRound(3.6), 4.0, 1e-9), "round(3.6) ~ 4");
	TEST(NagisaApproxEqual(SulettaRound(-3.4), -3.0, 1e-9), "round(-3.4) ~ -3");
	TEST(NagisaApproxEqual(SulettaRound(-3.6), -4.0, 1e-9), "round(-3.6) ~ -4");
	TEST(NagisaApproxEqual(SulettaRound(0.0), 0.0, 1e-9), "round(0) ~ 0");
	TEST(NagisaApproxEqual(SulettaRound(0.5), 1.0, 1e-9), "round(0.5) ~ 1");
	TEST(SulettaRound(MakeNaN()) == 0.0, "round(NaN) -> 0");
}

static void TestSulettaFmod(void) {
	TEST(NagisaApproxEqual(SulettaFmod(10.0, 3.0), 1.0, 1e-9), "fmod(10,3) ~ 1");
	TEST(NagisaApproxEqual(SulettaFmod(10.5, 3.0), 1.5, 1e-9), "fmod(10.5,3) ~ 1.5");
	TEST(NagisaApproxEqual(SulettaFmod(-10.0, 3.0), -1.0, 1e-9), "fmod(-10,3) ~ -1");
	TEST(SulettaFmod(10.0, 0.0) == 0.0, "fmod(10,0) -> 0");
	TEST(SulettaFmod(MakeNaN(), 1.0) == 0.0, "fmod(NaN,1) -> 0");
	TEST(SulettaFmod(1.0, MakeNaN()) == 0.0, "fmod(1,NaN) -> 0");
}

static void TestSulettaPow(void) {
	TEST(NagisaApproxEqual(SulettaPow(2.0, 3.0), 8.0, 1e-9), "pow(2,3) ~ 8");
	TEST(NagisaApproxEqual(SulettaPow(10.0, 0.0), 1.0, 1e-9), "pow(10,0) ~ 1");
	TEST(NagisaApproxEqual(SulettaPow(0.0, 2.0), 0.0, 1e-9), "pow(0,2) ~ 0");
	TEST(NagisaApproxEqual(SulettaPow(2.0, -1.0), 0.5, 1e-9), "pow(2,-1) ~ 0.5");
	TEST(SulettaPow(MakeNaN(), 1.0) == 0.0, "pow(NaN,1) -> 0");
	TEST(SulettaPow(1.0, MakeNaN()) == 0.0, "pow(1,NaN) -> 0");
}

// ===========================================================================
//  Kanna — interpolation and mapping
// ===========================================================================

static void TestKannaLerp(void) {
	TEST(NagisaApproxEqual(KannaLerp(0.0, 10.0, 0.5), 5.0, 1e-9), "lerp(0,10,0.5) ~ 5");
	TEST(NagisaApproxEqual(KannaLerp(0.0, 10.0, 0.0), 0.0, 1e-9), "lerp(0,10,0) ~ 0");
	TEST(NagisaApproxEqual(KannaLerp(0.0, 10.0, 1.0), 10.0, 1e-9), "lerp(0,10,1) ~ 10");
	TEST(NagisaApproxEqual(KannaLerp(10.0, 20.0, 0.25), 12.5, 1e-9), "lerp(10,20,0.25) ~ 12.5");
	TEST(NagisaApproxEqual(KannaLerp(5.0, 5.0, 0.5), 5.0, 1e-9), "lerp(5,5,0.5) ~ 5");
	TEST(NagisaApproxEqual(KannaLerp(MakeNaN(), 10.0, 0.5), 5.0, 1e-9), "lerp(NaN,10,0.5) -> 5");
}

static void TestKannaSmoothstep(void) {
	TEST(NagisaApproxEqual(KannaSmoothstep(0.0, 1.0, 0.5), 0.5, 1e-9), "smoothstep(0,1,0.5) ~ 0.5");
	TEST(NagisaApproxEqual(KannaSmoothstep(0.0, 1.0, 0.0), 0.0, 1e-9), "smoothstep(0,1,0) ~ 0");
	TEST(NagisaApproxEqual(KannaSmoothstep(0.0, 1.0, 1.0), 1.0, 1e-9), "smoothstep(0,1,1) ~ 1");
	TEST(NagisaApproxEqual(KannaSmoothstep(0.0, 1.0, 0.25), 0.15625, 1e-9), "smoothstep(0,1,0.25) ~ 0.15625");
	TEST(KannaSmoothstep(MakeNaN(), 1.0, 0.5) == 0.0, "smoothstep(NaN,1,0.5) -> 0");
}

static void TestKannaDegToRad(void) {
	TEST(NagisaApproxEqual(KannaDegToRad(0.0), 0.0, 1e-9), "degToRad(0) ~ 0");
	TEST(NagisaApproxEqual(KannaDegToRad(180.0), REAL_PI, 1e-9), "degToRad(180) ~ PI");
	TEST(NagisaApproxEqual(KannaDegToRad(90.0), REAL_PI_HALF, 1e-9), "degToRad(90) ~ PI/2");
	TEST(NagisaApproxEqual(KannaDegToRad(-180.0), -REAL_PI, 1e-9), "degToRad(-180) ~ -PI");
	TEST(NagisaApproxEqual(KannaDegToRad(360.0), REAL_2PI, 1e-9), "degToRad(360) ~ 2PI");
	TEST(KannaDegToRad(MakeNaN()) == 0.0, "degToRad(NaN) -> 0");
}

static void TestKannaRadToDeg(void) {
	TEST(NagisaApproxEqual(KannaRadToDeg(0.0), 0.0, 1e-9), "radToDeg(0) ~ 0");
	TEST(NagisaApproxEqual(KannaRadToDeg(REAL_PI), 180.0, 1e-9), "radToDeg(PI) ~ 180");
	TEST(NagisaApproxEqual(KannaRadToDeg(REAL_PI_HALF), 90.0, 1e-9), "radToDeg(PI/2) ~ 90");
	TEST(NagisaApproxEqual(KannaRadToDeg(-REAL_PI), -180.0, 1e-9), "radToDeg(-PI) ~ -180");
	TEST(NagisaApproxEqual(KannaRadToDeg(REAL_2PI), 360.0, 1e-9), "radToDeg(2PI) ~ 360");
	TEST(KannaRadToDeg(MakeNaN()) == 0.0, "radToDeg(NaN) -> 0");
}

// ===========================================================================
//  Main
// ===========================================================================

int main(void) {
	fprintf(stderr, "=== TestMath ===\n");

	RUN_TEST(TestMaiIsNaN);
	RUN_TEST(TestMaiIsInf);
	RUN_TEST(TestMaiIsFinite);
	RUN_TEST(TestMaiSanitize);

	RUN_TEST(TestNagisaApproxEqual);
	RUN_TEST(TestNagisaApproxZero);
	RUN_TEST(TestNagisaEqualIsZero);

	RUN_TEST(TestYuuClamp);
	RUN_TEST(TestYuuClamp01);
	RUN_TEST(TestYuuAbs);
	RUN_TEST(TestYuuMinMax);
	RUN_TEST(TestYuuSign);
	RUN_TEST(TestYuuClamp64);
	RUN_TEST(TestYuuWrap64);

	RUN_TEST(TestSulettaSin);
	RUN_TEST(TestSulettaCos);
	RUN_TEST(TestSulettaInvSqrt);
	RUN_TEST(TestSulettaSqrt);

	RUN_TEST(TestSulettaTan);
	RUN_TEST(TestSulettaAtan2);
	RUN_TEST(TestSulettaFloor);
	RUN_TEST(TestSulettaCeil);
	RUN_TEST(TestSulettaRound);
	RUN_TEST(TestSulettaFmod);
	RUN_TEST(TestSulettaPow);

	RUN_TEST(TestKannaLerp);
	RUN_TEST(TestKannaSmoothstep);
	RUN_TEST(TestKannaDegToRad);
	RUN_TEST(TestKannaRadToDeg);

	fprintf(stderr, "\n=== %d passed, %d failed ===\n", Passed, Failed);
	return Failed > 0 ? 1 : 0;
}
