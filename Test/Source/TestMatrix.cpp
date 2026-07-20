/**
 * Unit tests for Matrix3x3 and Matrix4x4 operations.
 * 行列操作の単体テストね。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include <TohruPhysics/Matrix.h>
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

// ===========================================================================
//  Matrix3x3 tests
// ===========================================================================

static void TestM3Identity(void) {
	Matrix3x3 I = MiorineMatrix3x3Identity();
	// Diagonal should be 1, rest 0.
	int R, C;
	for (R = 0; R < 3; R++) {
		for (C = 0; C < 3; C++) {
			Real Expected = (R == C) ? 1.0 : 0.0;
			if (!NagisaEqual(I.Data[R * 3 + C], Expected)) {
				fprintf(stderr, "  I[%d][%d] = %f, expected %f\n",
					R, C, I.Data[R * 3 + C], Expected);
				TEST(0, "identity");
			}
		}
	}
}

static void TestM3Make(void) {
	Matrix3x3 M = MiorineMatrix3x3Make(
		1, 2, 3,
		4, 5, 6,
		7, 8, 9
	);
	TEST(NagisaEqual(M.Data[0], 1), "M00");
	TEST(NagisaEqual(M.Data[4], 5), "M11");
	TEST(NagisaEqual(M.Data[8], 9), "M22");
}

static void TestM3Mul(void) {
	Matrix3x3 A = MiorineMatrix3x3Make(
		1, 0, 0,
		0, 1, 0,
		0, 0, 1
	);
	Matrix3x3 B = MiorineMatrix3x3Make(
		2, 0, 0,
		0, 3, 0,
		0, 0, 4
	);
	Matrix3x3 R = MiorineMatrix3x3Mul(&A, &B);
	TEST(NagisaEqual(R.Data[0], 2), "I*scale M00");
	TEST(NagisaEqual(R.Data[4], 3), "I*scale M11");
	TEST(NagisaEqual(R.Data[8], 4), "I*scale M22");
}

static void TestM3VecMul(void) {
	Matrix3x3 I = MiorineMatrix3x3Identity();
	Vector3 V = KannaVector3Make(1.0, 2.0, 3.0);
	Vector3 R = MiorineMatrix3x3VecMul(&I, &V);
	TEST(NagisaEqual(R.Data[0], 1.0), "I*x");
	TEST(NagisaEqual(R.Data[1], 2.0), "I*y");
	TEST(NagisaEqual(R.Data[2], 3.0), "I*z");
}

static void TestM3Transpose(void) {
	Matrix3x3 M = MiorineMatrix3x3Make(
		1, 2, 3,
		4, 5, 6,
		7, 8, 9
	);
	Matrix3x3 T = MiorineMatrix3x3Transpose(&M);
	TEST(NagisaEqual(T.Data[1], 4), "T01==M10");
	TEST(NagisaEqual(T.Data[3], 2), "T10==M01");
	TEST(NagisaEqual(T.Data[0], 1), "T00==M00");
}

static void TestM3Determinant(void) {
	Matrix3x3 I = MiorineMatrix3x3Identity();
	TEST(NagisaEqual(MiorineMatrix3x3Determinant(&I), 1.0), "det I = 1");

	Matrix3x3 Z;
	int Ix;
	for (Ix = 0; Ix < 9; Ix++) Z.Data[Ix] = 0.0;
	TEST(NagisaEqual(MiorineMatrix3x3Determinant(&Z), 0.0), "det zero = 0");

	Matrix3x3 M = MiorineMatrix3x3Make(
		1, 2, 3,
		4, 5, 6,
		7, 8, 10
	);
	// det = 1*(50-48) - 2*(40-42) + 3*(32-35) = 2 + 4 - 9 = -3
	TEST(NagisaEqual(MiorineMatrix3x3Determinant(&M), -3.0), "det M = -3");
}

static void TestM3Inverse(void) {
	Matrix3x3 I = MiorineMatrix3x3Identity();
	Matrix3x3 InvI = MiorineMatrix3x3Inverse(&I);
	TEST(NagisaEqual(InvI.Data[0], 1.0), "inv I = I");

	// Singlar → zero matrix (ZII).
	Matrix3x3 Z;
	int Ix;
	for (Ix = 0; Ix < 9; Ix++) Z.Data[Ix] = 0.0;
	Matrix3x3 InvZ = MiorineMatrix3x3Inverse(&Z);
	TEST(NagisaEqual(InvZ.Data[0], 0.0), "inv zero = zero (ZII)");
	TEST(NagisaEqual(InvZ.Data[4], 0.0), "inv zero M11=0");

	// M * inv(M) ≈ I
	Matrix3x3 M = MiorineMatrix3x3Make(
		2, 0, 0,
		0, 3, 0,
		0, 0, 4
	);
	Matrix3x3 InvM = MiorineMatrix3x3Inverse(&M);
	Matrix3x3 Prod = MiorineMatrix3x3Mul(&M, &InvM);
	TEST(NagisaApproxEqual(Prod.Data[0], 1.0, 1e-9), "M*inv M00");
	TEST(NagisaApproxEqual(Prod.Data[4], 1.0, 1e-9), "M*inv M11");
	TEST(NagisaApproxEqual(Prod.Data[8], 1.0, 1e-9), "M*inv M22");
}

// ===========================================================================
//  Matrix4x4 tests
// ===========================================================================

static void TestM4Identity(void) {
	Matrix4x4 I = AnisphiaMatrix4x4Identity();
	int R, C;
	for (R = 0; R < 4; R++) {
		for (C = 0; C < 4; C++) {
			Real Expected = (R == C) ? 1.0 : 0.0;
			if (!NagisaEqual(I.Data[R * 4 + C], Expected)) {
				fprintf(stderr, "  I[%d][%d] = %f, expected %f\n",
					R, C, I.Data[R * 4 + C], Expected);
				TEST(0, "identity 4x4");
			}
		}
	}
}

static void TestM4Mul(void) {
	Matrix4x4 A = AnisphiaMatrix4x4Identity();
	Matrix4x4 B = AnisphiaMatrix4x4Make(
		1, 0, 0, 0,
		0, 2, 0, 0,
		0, 0, 3, 0,
		0, 0, 0, 4
	);
	Matrix4x4 R = AnisphiaMatrix4x4Mul(&A, &B);
	TEST(NagisaEqual(R.Data[0], 1), "I*scale M00");
	TEST(NagisaEqual(R.Data[5], 2), "I*scale M11");
	TEST(NagisaEqual(R.Data[10], 3), "I*scale M22");
	TEST(NagisaEqual(R.Data[15], 4), "I*scale M33");
}

static void TestM4VecMul(void) {
	Matrix4x4 I = AnisphiaMatrix4x4Identity();
	Vector3 V = KannaVector3Make(1.0, 2.0, 3.0);
	Vector3 R = AnisphiaMatrix4x4VecMul(&I, &V);
	TEST(NagisaEqual(R.Data[0], 1.0), "I*x");
	TEST(NagisaEqual(R.Data[1], 2.0), "I*y");
	TEST(NagisaEqual(R.Data[2], 3.0), "I*z");
}

static void TestM4Determinant(void) {
	Matrix4x4 I = AnisphiaMatrix4x4Identity();
	TEST(NagisaEqual(AnisphiaMatrix4x4Determinant(&I), 1.0), "det I = 1");

	Matrix4x4 Z;
	int Ix;
	for (Ix = 0; Ix < 16; Ix++) Z.Data[Ix] = 0.0;
	TEST(NagisaEqual(AnisphiaMatrix4x4Determinant(&Z), 0.0), "det zero = 0");

	// Diagonal 1,2,3,4 → det = 24
	Matrix4x4 D = AnisphiaMatrix4x4Make(
		1, 0, 0, 0,
		0, 2, 0, 0,
		0, 0, 3, 0,
		0, 0, 0, 4
	);
	TEST(NagisaApproxEqual(AnisphiaMatrix4x4Determinant(&D), 24.0, 1e-9), "det diag = 24");
}

static void TestM4Inverse(void) {
	Matrix4x4 I = AnisphiaMatrix4x4Identity();
	Matrix4x4 InvI = AnisphiaMatrix4x4Inverse(&I);
	TEST(NagisaEqual(InvI.Data[0], 1.0), "inv I = I");

	// Singlar → zero matrix (ZII).
	Matrix4x4 Z;
	int Ix;
	for (Ix = 0; Ix < 16; Ix++) Z.Data[Ix] = 0.0;
	Matrix4x4 InvZ = AnisphiaMatrix4x4Inverse(&Z);
	TEST(NagisaEqual(InvZ.Data[0], 0.0), "inv zero = zero (ZII)");

	// M * inv(M) ≈ I
	Matrix4x4 M = AnisphiaMatrix4x4Make(
		2, 0, 0, 0,
		0, 3, 0, 0,
		0, 0, 4, 0,
		0, 0, 0, 5
	);
	Matrix4x4 InvM = AnisphiaMatrix4x4Inverse(&M);
	Matrix4x4 Prod = AnisphiaMatrix4x4Mul(&M, &InvM);
	TEST(NagisaApproxEqual(Prod.Data[0], 1.0, 1e-9), "M*inv M00");
	TEST(NagisaApproxEqual(Prod.Data[5], 1.0, 1e-9), "M*inv M11");
	TEST(NagisaApproxEqual(Prod.Data[10], 1.0, 1e-9), "M*inv M22");
	TEST(NagisaApproxEqual(Prod.Data[15], 1.0, 1e-9), "M*inv M33");
}

// ===========================================================================
//  Main
// ===========================================================================

int main(void) {
	fprintf(stderr, "=== TestMatrix ===\n");

	RUN_TEST(TestM3Identity);
	RUN_TEST(TestM3Make);
	RUN_TEST(TestM3Mul);
	RUN_TEST(TestM3VecMul);
	RUN_TEST(TestM3Transpose);
	RUN_TEST(TestM3Determinant);
	RUN_TEST(TestM3Inverse);

	RUN_TEST(TestM4Identity);
	RUN_TEST(TestM4Mul);
	RUN_TEST(TestM4VecMul);
	RUN_TEST(TestM4Determinant);
	RUN_TEST(TestM4Inverse);

	fprintf(stderr, "\n=== %d passed, %d failed ===\n", Passed, Failed);
	return Failed > 0 ? 1 : 0;
}
