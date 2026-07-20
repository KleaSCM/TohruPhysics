/**
 * Unit tests for SAT collision detection.
 * SAT衝突検出の単体テストね。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include <TohruPhysics/SAT.h>
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

#define RUN_TEST(name, desc) do { \
	fprintf(stderr, "  %-45s ... ", desc); \
	name(); \
	fprintf(stderr, "ok\n"); \
	Passed++; \
} while(0)

static void TestSAT_OBBOBB(void) {
	Quaternion Id = EuphylliaQuaternionIdentity();
	Vector3 C0 = {0,0,0}, C1 = {1,0,0}, HE = {1,1,1};
	OBB A = SabinaOBBMake(&C0, &HE, &Id);
	OBB B = SabinaOBBMake(&C1, &HE, &Id);
	SATResult R = SintoOBBOBB(&A, &B);
	TEST(R.Intersect, "obb-obb intersect");

	Vector3 C2 = {10,0,0};
	OBB C = SabinaOBBMake(&C2, &HE, &Id);
	R = SintoOBBOBB(&A, &C);
	TEST(!R.Intersect, "obb-obb separated");
	TEST(R.PenetrationDepth == 0, "obb-obb sep pen=0");
}

static void TestSAT_AABBOBB(void) {
	Vector3 Min = {-2,-2,-2}, Max = {2,2,2};
	AABB Box = SabinaAABBMake(&Min, &Max);
	Quaternion Id = EuphylliaQuaternionIdentity();
	Vector3 C0 = {0,0,0}, HE = {1,1,1};
	OBB O = SabinaOBBMake(&C0, &HE, &Id);
	SATResult R = SintoAABBOBB(&Box, &O);
	TEST(R.Intersect, "aabb-obb intersect");

	Vector3 C1 = {10,0,0};
	OBB O2 = SabinaOBBMake(&C1, &HE, &Id);
	R = SintoAABBOBB(&Box, &O2);
	TEST(!R.Intersect, "aabb-obb separated");
}

static void TestSAT_Poly2D(void) {
	// Two overlapping squares
	Vector2 SqA[4] = {{-1,-1},{1,-1},{1,1},{-1,1}};
	Vector2 SqB[4] = {{0,0},{2,0},{2,2},{0,2}};
	SATResult R = SintoPolyPoly2D(SqA, 4, SqB, 4);
	TEST(R.Intersect, "poly2d squares overlap");

	// Separated squares
	Vector2 SqC[4] = {{5,5},{7,5},{7,7},{5,7}};
	R = SintoPolyPoly2D(SqA, 4, SqC, 4);
	TEST(!R.Intersect, "poly2d squares separated");
}

static void TestSAT_Poly3D(void) {
	// Two overlapping tetrahedra
	Vector3 TetA[4] = {{0,0,0},{1,0,0},{0,1,0},{0,0,1}};
	Vector3 TetB[4] = {{0.5,0.5,0},{1.5,0.5,0},{0.5,1.5,0},{0.5,0.5,1}};
	SATResult R = SintoPolyPoly3D(TetA, 4, TetB, 4);
	TEST(R.Intersect, "poly3d tets overlap");

	// Separated
	Vector3 TetC[4] = {{10,0,0},{11,0,0},{10,1,0},{10,0,1}};
	R = SintoPolyPoly3D(TetA, 4, TetC, 4);
	TEST(!R.Intersect, "poly3d tets separated");
}

int main(void) {
	fprintf(stderr, "=== TestSAT ===\n");

	RUN_TEST(TestSAT_OBBOBB, "SAT: OBB-OBB");
	RUN_TEST(TestSAT_AABBOBB, "SAT: AABB-OBB");
	RUN_TEST(TestSAT_Poly2D, "SAT: 2D polygon");
	RUN_TEST(TestSAT_Poly3D, "SAT: 3D polyhedron");

	fprintf(stderr, "\n=== %d passed, 0 failed ===\n", Passed);
	return 0;
}
