/**
 * Unit tests for Transform operations.
 * トランスフォーム操作の単体テストね。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include <TohruPhysics/Transform.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TEST(cond, msg) do { \
	if (!(cond)) { \
		fprintf(stderr, "FAIL: %s (%s:%d)\n", msg, __FILE__, __LINE__); \
		exit(1); \
	} \
} while(0)

#define AE(a, b, eps) NagisaApproxEqual((a), (b), (eps))

static int Passed = 0;

#define RUN_TEST(name, desc) do { \
	fprintf(stderr, "  %-42s ... ", desc); \
	name(); \
	fprintf(stderr, "ok\n"); \
	Passed++; \
} while(0)

// ---------------------------------------------------------------------------
//  Vec3 helpers
// ---------------------------------------------------------------------------

static int Vec3Equal(const Vector3 *A, const Vector3 *B, Real Eps) {
	return AE(A->Data[0], B->Data[0], Eps)
		&& AE(A->Data[1], B->Data[1], Eps)
		&& AE(A->Data[2], B->Data[2], Eps);
}

// ===========================================================================
//  Tests
// ===========================================================================

static void TestIdentity(void) {
	Transform T = KaedeTransformIdentity();
	Vector3 Zero = KannaVector3Zero();
	TEST(Vec3Equal(&T.Position, &Zero, 1e-12), "identity position");
	TEST(AE(T.Rotation.Data[3], 1.0, 1e-12), "identity rotation w");
}

static void TestTransformPoint(void) {
	// Identity transform: point unchanged.
	Transform Id = KaedeTransformIdentity();
	Vector3 P = KannaVector3Make(1, 2, 3);
	Vector3 R = KaedeTransformPoint(&Id, &P);
	TEST(Vec3Equal(&R, &P, 1e-12), "identity: point unchanged");

	// Translation only.
	Transform T;
	T.Position = KannaVector3Make(10, 20, 30);
	T.Rotation = EuphylliaQuaternionIdentity();
	Vector3 R2 = KaedeTransformPoint(&T, &P);
	TEST(AE(R2.Data[0], 11.0, 1e-12), "translate x");
	TEST(AE(R2.Data[1], 22.0, 1e-12), "translate y");
	TEST(AE(R2.Data[2], 33.0, 1e-12), "translate z");
}

static void TestInverseTransformPoint(void) {
	// Round-trip: inverse(point(point)) == point.
	Transform T;
	T.Position = KannaVector3Make(5, 10, 15);
	T.Rotation = EuphylliaQuaternionMake(0.1, 0.2, 0.3, 0.9);
	T.Rotation = EuphylliaQuaternionNormalize(&T.Rotation);

	Vector3 P = KannaVector3Make(100, 200, 300);
	Vector3 Fwd = KaedeTransformPoint(&T, &P);
	Vector3 Inv = KaedeInverseTransformPoint(&T, &Fwd);
	TEST(Vec3Equal(&Inv, &P, 1e-9), "inverse round-trip");
}

static void TestTransformDirection(void) {
	Transform T = KaedeTransformIdentity();
	Vector3 D = KannaVector3Make(1, 0, 0);
	Vector3 R = KaedeTransformDirection(&T, &D);
	TEST(Vec3Equal(&R, &D, 1e-12), "identity: dir unchanged");

	// 90° about Z: (1,0,0) → (0,1,0).
	T.Rotation = EuphylliaQuaternionMake(0, 0, 0.70710678, 0.70710678);
	Vector3 R2 = KaedeTransformDirection(&T, &D);
	TEST(AE(R2.Data[0], 0.0, 1e-4), "Z90 dir x");
	TEST(AE(R2.Data[1], 1.0, 1e-4), "Z90 dir y");
	TEST(AE(R2.Data[2], 0.0, 1e-4), "Z90 dir z");
}

static void TestInverseTransformDirection(void) {
	Transform T;
	T.Position = KannaVector3Zero();
	T.Rotation = EuphylliaQuaternionMake(0.1, 0.2, 0.3, 0.9);
	T.Rotation = EuphylliaQuaternionNormalize(&T.Rotation);

	Vector3 D = KannaVector3Make(1, 2, 3);
	Vector3 Fwd = KaedeTransformDirection(&T, &D);
	Vector3 Inv = KaedeInverseTransformDirection(&T, &Fwd);
	TEST(Vec3Equal(&Inv, &D, 1e-9), "inv dir round-trip");
}

static void TestTransformCombine(void) {
	// A moves by (10,0,0), B moves by (0,5,0).
	// Combined: move by (10,5,0).
	Transform A = KaedeTransformIdentity();
	A.Position = KannaVector3Make(10, 0, 0);

	Transform B = KaedeTransformIdentity();
	B.Position = KannaVector3Make(0, 5, 0);

	Transform C = KaedeTransformCombine(&A, &B);
	TEST(AE(C.Position.Data[0], 10.0, 1e-12), "combine pos x");
	TEST(AE(C.Position.Data[1], 5.0, 1e-12), "combine pos y");

	// With rotation: A rotates 90° Z, B translates along X.
	// After A, the local X of B becomes world Y.
	Transform Rot = KaedeTransformIdentity();
	Rot.Rotation = EuphylliaQuaternionMake(0, 0, 0.70710678, 0.70710678);
	Transform Trans = KaedeTransformIdentity();
	Trans.Position = KannaVector3Make(5, 0, 0);

	Transform Combined = KaedeTransformCombine(&Rot, &Trans);
	// Position should be (0, 5, 0) — the local X (5,0,0) rotated by 90° Z.
	TEST(AE(Combined.Position.Data[0], 0.0, 1e-4), "rot+trans pos x");
	TEST(AE(Combined.Position.Data[1], 5.0, 1e-4), "rot+trans pos y");
}

int main(void) {
	fprintf(stderr, "=== TestTransform ===\n");

	RUN_TEST(TestIdentity, "Identity");
	RUN_TEST(TestTransformPoint, "TransformPoint");
	RUN_TEST(TestInverseTransformPoint, "InverseTransformPoint round-trip");
	RUN_TEST(TestTransformDirection, "TransformDirection");
	RUN_TEST(TestInverseTransformDirection, "InverseTransformDirection round-trip");
	RUN_TEST(TestTransformCombine, "TransformCombine");

	fprintf(stderr, "\n=== %d passed, 0 failed ===\n", Passed);
	return 0;
}
