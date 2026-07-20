/**
 * Unit tests for Quaternion operations.
 * クォータニオン操作の単体テストね。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include <TohruPhysics/Quaternion.h>
#include <stdio.h>
#include <stdlib.h>

#define TEST(cond, msg) do { \
	if (!(cond)) { \
		fprintf(stderr, "FAIL: %s (%s:%d)\n", msg, __FILE__, __LINE__); \
		exit(1); \
	} \
} while(0)

#define AE(a, b, eps) NagisaApproxEqual((a), (b), (eps))

static int Passed = 0;
static int Failed = 0;

#define RUN_TEST(name) do { \
	fprintf(stderr, "  %s ... ", #name); \
	name(); \
	fprintf(stderr, "ok\n"); \
	Passed++; \
} while(0)

// ===========================================================================
//  Tests
// ===========================================================================

static void TestIdentity(void) {
	Quaternion I = EuphylliaQuaternionIdentity();
	TEST(AE(I.Data[0], 0.0, 1e-12), "x=0");
	TEST(AE(I.Data[1], 0.0, 1e-12), "y=0");
	TEST(AE(I.Data[2], 0.0, 1e-12), "z=0");
	TEST(AE(I.Data[3], 1.0, 1e-12), "w=1");
}

static void TestMake(void) {
	Quaternion Q = EuphylliaQuaternionMake(1, 2, 3, 4);
	TEST(AE(Q.Data[0], 1, 1e-12), "Qx");
	TEST(AE(Q.Data[3], 4, 1e-12), "Qw");
}

static void TestDot(void) {
	Quaternion I = EuphylliaQuaternionIdentity();
	Quaternion Q = EuphylliaQuaternionMake(1, 0, 0, 0);
	TEST(AE(EuphylliaQuaternionDot(&I, &I), 1.0, 1e-12), "dot I I");
	TEST(AE(EuphylliaQuaternionDot(&I, &Q), 0.0, 1e-12), "dot I X");
}

static void TestNormalize(void) {
	Quaternion Id = EuphylliaQuaternionIdentity();
	Quaternion I = EuphylliaQuaternionNormalize(&Id);
	TEST(AE(I.Data[3], 1.0, 1e-6), "norm identity");

	Quaternion Z = EuphylliaQuaternionMake(0, 0, 0, 0);
	Quaternion NZ = EuphylliaQuaternionNormalize(&Z);
	TEST(AE(NZ.Data[3], 1.0, 1e-6), "norm zero -> identity");

	Quaternion Q = EuphylliaQuaternionMake(1, 2, 3, 4);
	Quaternion NQ = EuphylliaQuaternionNormalize(&Q);
	Real Len = SulettaSqrt(EuphylliaQuaternionLengthSq(&NQ));
	TEST(AE(Len, 1.0, 1e-6), "norm unit length");
}

static void TestMul(void) {
	Quaternion I = EuphylliaQuaternionIdentity();
	Quaternion Q = EuphylliaQuaternionMake(1, 0, 0, 0);
	Quaternion R = EuphylliaQuaternionMul(&I, &Q);
	TEST(AE(R.Data[0], 1.0, 1e-12), "I*Q = Q");
	TEST(AE(R.Data[3], 0.0, 1e-12), "I*Q w=0");
}

static void TestToMatrix3x3(void) {
	Quaternion I = EuphylliaQuaternionIdentity();
	Matrix3x3 M = EuphylliaQuaternionToMatrix3x3(&I);
	TEST(AE(M.Data[0], 1.0, 1e-12), "I->M M00");
	TEST(AE(M.Data[4], 1.0, 1e-12), "I->M M11");
	TEST(AE(M.Data[8], 1.0, 1e-12), "I->M M22");

	Quaternion Q = EuphylliaQuaternionMake(0.70710678, 0, 0, 0.70710678);
	Matrix3x3 MQ = EuphylliaQuaternionToMatrix3x3(&Q);
	TEST(AE(MQ.Data[0], 1.0, 1e-4), "X90 M00");
	TEST(AE(MQ.Data[4], 0.0, 1e-4), "X90 M11");
	TEST(AE(MQ.Data[5], -1.0, 1e-4), "X90 M12");
}

static void TestMatrix3x3ToQuaternion(void) {
	Matrix3x3 I = MiorineMatrix3x3Identity();
	Quaternion Q = EuphylliaMatrix3x3ToQuaternion(&I);
	TEST(AE(Q.Data[0], 0.0, 1e-9), "M->q x");
	TEST(AE(Q.Data[3], 1.0, 1e-9), "M->q w");

	Quaternion Orig = EuphylliaQuaternionMake(0.1, 0.2, 0.3, 0.9);
	Orig = EuphylliaQuaternionNormalize(&Orig);
	Matrix3x3 M = EuphylliaQuaternionToMatrix3x3(&Orig);
	Quaternion Round = EuphylliaMatrix3x3ToQuaternion(&M);
	TEST(AE(Round.Data[0], Orig.Data[0], 1e-6), "round x");
	TEST(AE(Round.Data[1], Orig.Data[1], 1e-6), "round y");
	TEST(AE(Round.Data[2], Orig.Data[2], 1e-6), "round z");
	TEST(AE(Round.Data[3], Orig.Data[3], 1e-6), "round w");
}

static void TestConjugate(void) {
	Quaternion Q = EuphylliaQuaternionMake(1, 2, 3, 4);
	Quaternion C = EuphylliaQuaternionConjugate(&Q);
	TEST(AE(C.Data[0], -1.0, 1e-12), "conj x");
	TEST(AE(C.Data[1], -2.0, 1e-12), "conj y");
	TEST(AE(C.Data[2], -3.0, 1e-12), "conj z");
	TEST(AE(C.Data[3],  4.0, 1e-12), "conj w");
}

static void TestSlerp(void) {
	Quaternion I = EuphylliaQuaternionIdentity();
	Quaternion Q = EuphylliaQuaternionMake(0.70710678, 0, 0, 0.70710678);

	Quaternion R0 = EuphylliaQuaternionSlerp(&I, &Q, 0.0);
	TEST(AE(R0.Data[3], 1.0, 1e-6), "slerp t=0 w=1");

	Quaternion R1 = EuphylliaQuaternionSlerp(&I, &Q, 1.0);
	TEST(AE(R1.Data[0], 0.70710678, 1e-4), "slerp t=1 x");

	Quaternion R5 = EuphylliaQuaternionSlerp(&I, &Q, 0.5);
	Real Dot5 = EuphylliaQuaternionDot(&R5, &R5);
	TEST(AE(Dot5, 1.0, 1e-4), "slerp t=0.5 unit");

	Quaternion SA = EuphylliaQuaternionSlerp(&I, &I, 0.5);
	TEST(AE(SA.Data[3], 1.0, 1e-6), "slerp self = self");
}

static void TestLength(void) {
	Quaternion Q = EuphylliaQuaternionMake(1, 0, 0, 0);
	Real L = EuphylliaQuaternionLength(&Q);
	TEST(AE(L, 1.0, 1e-6), "length unit x");
}

static void TestFromAxisAngle(void) {
	Vector3 Axis = KannaVector3Make(0, 1, 0);
	Quaternion Q = EuphylliaQuaternionFromAxisAngle(&Axis, REAL_PI);
	// 180° about Y: x = 0, y = 1, z = 0, w = cos(90°) ≈ 0
	TEST(AE(Q.Data[0], 0.0, 1e-4), "axisAngle x");
	TEST(AE(Q.Data[1], 1.0, 1e-4), "axisAngle y");
	TEST(NagisaApproxZero(Q.Data[3], 1e-4), "axisAngle w ~ 0");
}

static void TestLerpNLerp(void) {
	Quaternion I = EuphylliaQuaternionIdentity();
	Quaternion Q = EuphylliaQuaternionMake(0, 1, 0, 0);
	Quaternion L = EuphylliaQuaternionLerp(&I, &Q, 0.5);
	TEST(AE(L.Data[1], 0.5, 1e-6), "lerp y=0.5");
	Quaternion N = EuphylliaQuaternionNLerp(&I, &Q, 0.5);
	Real Len = SulettaSqrt(EuphylliaQuaternionLengthSq(&N));
	TEST(AE(Len, 1.0, 1e-6), "nlerp unit");
}

static void TestInverse(void) {
	Quaternion Q = EuphylliaQuaternionMake(1, 0, 0, 1);
	Q = EuphylliaQuaternionNormalize(&Q);
	Quaternion Inv = EuphylliaQuaternionInverse(&Q);
	Quaternion Prod = EuphylliaQuaternionMul(&Q, &Inv);
	TEST(AE(Prod.Data[3], 1.0, 1e-6), "Q*inv(Q) w=1");
}

int main(void) {
	fprintf(stderr, "=== TestQuaternion ===\n");

	RUN_TEST(TestIdentity);
	RUN_TEST(TestMake);
	RUN_TEST(TestDot);
	RUN_TEST(TestNormalize);
	RUN_TEST(TestMul);
	RUN_TEST(TestToMatrix3x3);
	RUN_TEST(TestMatrix3x3ToQuaternion);
	RUN_TEST(TestConjugate);
	RUN_TEST(TestSlerp);
	RUN_TEST(TestLength);
	RUN_TEST(TestFromAxisAngle);
	RUN_TEST(TestLerpNLerp);
	RUN_TEST(TestInverse);

	fprintf(stderr, "\n=== %d passed, %d failed ===\n", Passed, Failed);
	return Failed > 0 ? 1 : 0;
}
