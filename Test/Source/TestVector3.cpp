/**
 * Unit tests for Vector3 operations.
 * Vector3操作の単体テストね。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include <TohruPhysics/Vector3.h>
#include <stdio.h>
#include <stdlib.h>

#define TEST(cond, msg) do { \
	if (!(cond)) { \
		fprintf(stderr, "FAIL: %s (%s:%d)\n", msg, __FILE__, __LINE__); \
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

// Helper: check vector component-wise.
static int VecApprox(Real X, Real Y, Real Z, const Vector3 *V) {
	return NagisaApproxEqual(V->Data[0], X, 1e-9)
		&& NagisaApproxEqual(V->Data[1], Y, 1e-9)
		&& NagisaApproxEqual(V->Data[2], Z, 1e-9);
}

static Vector3 VA, VB, VC;

static void Setup(void) {
	VA = KannaVector3Make(1.0, 2.0, 3.0);
	VB = KannaVector3Make(4.0, 5.0, 6.0);
	VC = KannaVector3Make(0.0, 0.0, 0.0);
}

static void TestVector3Make(void) {
	Setup();
	TEST(VecApprox(1.0, 2.0, 3.0, &VA), "make");
	TEST(VecApprox(0.0, 0.0, 0.0, &VC), "zero make");
}

static void TestVector3Add(void) {
	Setup();
	Vector3 R = KannaVector3Add(&VA, &VB);
	TEST(VecApprox(5.0, 7.0, 9.0, &R), "add");
	R = KannaVector3Add(&VA, &VC);
	TEST(VecApprox(1.0, 2.0, 3.0, &R), "add zero");
}

static void TestVector3Sub(void) {
	Setup();
	Vector3 R = KannaVector3Sub(&VB, &VA);
	TEST(VecApprox(3.0, 3.0, 3.0, &R), "sub");
	R = KannaVector3Sub(&VA, &VA);
	TEST(VecApprox(0.0, 0.0, 0.0, &R), "sub self");
}

static void TestVector3Scale(void) {
	Setup();
	Vector3 R = KannaVector3Scale(&VA, 2.0);
	TEST(VecApprox(2.0, 4.0, 6.0, &R), "scale");
	R = KannaVector3Scale(&VA, 0.0);
	TEST(VecApprox(0.0, 0.0, 0.0, &R), "scale zero");
}

static void TestVector3Dot(void) {
	Setup();
	Real D = KannaVector3Dot(&VA, &VB);
	// 1*4 + 2*5 + 3*6 = 4 + 10 + 18 = 32
	TEST(NagisaEqual(D, 32.0), "dot");
	D = KannaVector3Dot(&VA, &VC);
	TEST(NagisaEqual(D, 0.0), "dot zero");
}

static void TestVector3Cross(void) {
	Setup();
	Vector3 R = KannaVector3Cross(&VA, &VB);
	// (2*6-3*5, 3*4-1*6, 1*5-2*4) = (12-15, 12-6, 5-8) = (-3, 6, -3)
	TEST(VecApprox(-3.0, 6.0, -3.0, &R), "cross");
	R = KannaVector3Cross(&VA, &VA);
	TEST(VecApprox(0.0, 0.0, 0.0, &R), "cross self");
	R = KannaVector3Cross(&VA, &VC);
	TEST(VecApprox(0.0, 0.0, 0.0, &R), "cross zero");
}

static void TestVector3LengthSq(void) {
	Setup();
	Real L = KannaVector3LengthSq(&VA);
	// 1^2 + 2^2 + 3^2 = 14
	TEST(NagisaEqual(L, 14.0), "length sq");
	L = KannaVector3LengthSq(&VC);
	TEST(NagisaEqual(L, 0.0), "length sq zero");
}

static void TestVector3Normalize(void) {
	Setup();
	Vector3 N = KannaVector3Normalize(&VA);
	Real Len = SulettaSqrt(KannaVector3LengthSq(&N));
	TEST(NagisaApproxEqual(Len, 1.0, 1e-6), "normalize unit length");
	// Normalize a zero vector → should stay zero vector (ZII).
	Vector3 Z = KannaVector3Normalize(&VC);
	TEST(VecApprox(0.0, 0.0, 0.0, &Z), "normalize zero");
}

static void TestVector3Dist(void) {
	Setup();
	Real D = KannaVector3Distance(&VA, &VA);
	TEST(NagisaApproxZero(D, 1e-9), "dist self");
	// |VB - VA| = sqrt(3^2 + 3^2 + 3^2) = sqrt(27) = 5.196...
	Real D2 = KannaVector3Distance(&VA, &VB);
	TEST(NagisaApproxEqual(D2, 5.196152422706632, 1e-6), "dist");
}

static void TestVector3Equal(void) {
	Setup();
	TEST(KannaVector3Equal(&VA, &VA) == 1, "equal self");
	TEST(KannaVector3Equal(&VA, &VB) == 0, "not equal");
	TEST(KannaVector3Equal(&VC, &VC) == 1, "zero equal");
	Vector3 Small = KannaVector3Make(REAL_EPSILON * 0.5, 0.0, 0.0);
	TEST(KannaVector3Equal(&Small, &VC) == 1, "within epsilon");
}

static void TestVector3Length(void) {
	Setup();
	Real L = KannaVector3Length(&VA);
	TEST(NagisaApproxEqual(L, 3.74165738677, 1e-6), "length");
	L = KannaVector3Length(&VC);
	TEST(NagisaApproxZero(L, 1e-9), "length zero");
}

static void TestVector3DistanceSq(void) {
	Setup();
	Real D = KannaVector3DistanceSq(&VA, &VB);
	// (3^2 + 3^2 + 3^2) = 27
	TEST(NagisaEqual(D, 27.0), "dist sq");
}

static void TestVector3MinMax(void) {
	Setup();
	Vector3 M = KannaVector3Min(&VA, &VB);
	TEST(VecApprox(1.0, 2.0, 3.0, &M), "min");
	M = KannaVector3Max(&VA, &VB);
	TEST(VecApprox(4.0, 5.0, 6.0, &M), "max");
}

static void TestVector3AbsNegate(void) {
	Vector3 Neg = KannaVector3Make(-1.0, -2.0, 3.0);
	Vector3 A = KannaVector3Abs(&Neg);
	TEST(VecApprox(1.0, 2.0, 3.0, &A), "abs");
	Vector3 N = KannaVector3Negate(&VA);
	TEST(VecApprox(-1.0, -2.0, -3.0, &N), "negate");
}

static void TestVector3LerpReflect(void) {
	Vector3 A = KannaVector3Make(0.0, 0.0, 0.0);
	Vector3 B = KannaVector3Make(10.0, 10.0, 10.0);
	Vector3 L = KannaVector3Lerp(&A, &B, 0.5);
	TEST(VecApprox(5.0, 5.0, 5.0, &L), "lerp");

	Vector3 V = KannaVector3Make(1.0, -1.0, 0.0);
	Vector3 N = KannaVector3Make(0.0, 1.0, 0.0);
	Vector3 R = KannaVector3Reflect(&V, &N);
	TEST(VecApprox(1.0, 1.0, 0.0, &R), "reflect");

	Vector3 P = KannaVector3Project(&V, &N);
	TEST(VecApprox(0.0, -1.0, 0.0, &P), "project");

	Vector3 V2 = KannaVector3Make(3.0, 4.0, 0.0);
	Vector3 Clamped = KannaVector3ClampLength(&V2, 2.0);
	Real Len = KannaVector3Length(&Clamped);
	TEST(NagisaApproxEqual(Len, 2.0, 1e-6), "clamp length");
}

int main(void) {
	fprintf(stderr, "=== TestVector3 ===\n");

	RUN_TEST(TestVector3Make);
	RUN_TEST(TestVector3Add);
	RUN_TEST(TestVector3Sub);
	RUN_TEST(TestVector3Scale);
	RUN_TEST(TestVector3Dot);
	RUN_TEST(TestVector3Cross);
	RUN_TEST(TestVector3LengthSq);
	RUN_TEST(TestVector3Normalize);
	RUN_TEST(TestVector3Dist);
	RUN_TEST(TestVector3Equal);
	RUN_TEST(TestVector3Length);
	RUN_TEST(TestVector3DistanceSq);
	RUN_TEST(TestVector3MinMax);
	RUN_TEST(TestVector3AbsNegate);
	RUN_TEST(TestVector3LerpReflect);

	fprintf(stderr, "\n=== %d passed, %d failed ===\n", Passed, Failed);
	return Failed > 0 ? 1 : 0;
}
