/**
 * Unit tests for geometric primitives.
 * 基本幾何形状の単体テストね。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include <TohruPhysics/Geometry.h>
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

// ===========================================================================
//  AABB
// ===========================================================================

static void TestAABBMake(void) {
	Vector3 Min = KannaVector3Make(1, 2, 3);
	Vector3 Max = KannaVector3Make(4, 5, 6);
	AABB Box = SabinaAABBMake(&Min, &Max);
	TEST(AE(Box.Min.Data[0], 1, 1e-12), "min x");
	TEST(AE(Box.Max.Data[0], 4, 1e-12), "max x");
}

static void TestAABBMakeCenterExtents(void) {
	Vector3 C = KannaVector3Make(0, 0, 0);
	Vector3 H = KannaVector3Make(2, 3, 4);
	AABB Box = SabinaAABBMakeCenterExtents(&C, &H);
	TEST(AE(Box.Min.Data[0], -2, 1e-12), "ce min x");
	TEST(AE(Box.Max.Data[2],  4, 1e-12), "ce max z");
}

static void TestAABBContains(void) {
	Vector3 Min = KannaVector3Make(0, 0, 0);
	Vector3 Max = KannaVector3Make(10, 10, 10);
	AABB Box = SabinaAABBMake(&Min, &Max);

	Vector3 Inside = KannaVector3Make(5, 5, 5);
	TEST(SabinaAABBContains(&Box, &Inside), "contains inside");

	Vector3 Outside = KannaVector3Make(15, 5, 5);
	TEST(!SabinaAABBContains(&Box, &Outside), "not contains outside");

	Vector3 OnEdge = KannaVector3Make(0, 5, 5);
	TEST(SabinaAABBContains(&Box, &OnEdge), "contains on edge");
}

static void TestAABBOverlaps(void) {
	Vector3 MinA = KannaVector3Make(0, 0, 0);
	Vector3 MaxA = KannaVector3Make(5, 5, 5);
	AABB A = SabinaAABBMake(&MinA, &MaxA);

	Vector3 MinB = KannaVector3Make(3, 3, 3);
	Vector3 MaxB = KannaVector3Make(8, 8, 8);
	AABB B = SabinaAABBMake(&MinB, &MaxB);
	TEST(SabinaAABBOverlaps(&A, &B), "overlap intersecting");

	Vector3 MinC = KannaVector3Make(10, 10, 10);
	Vector3 MaxC = KannaVector3Make(15, 15, 15);
	AABB C = SabinaAABBMake(&MinC, &MaxC);
	TEST(!SabinaAABBOverlaps(&A, &C), "not overlap separated");
}

// ===========================================================================
//  Sphere
// ===========================================================================

static void TestSphereMake(void) {
	Vector3 C = KannaVector3Make(1, 2, 3);
	Sphere S = SabinaSphereMake(&C, 5.0);
	TEST(AE(S.Radius, 5.0, 1e-12), "sphere radius");
	TEST(AE(S.Center.Data[0], 1, 1e-12), "sphere center");
}

static void TestSphereContains(void) {
	Vector3 C = KannaVector3Zero();
	Sphere S = SabinaSphereMake(&C, 5.0);

	Vector3 Inside = KannaVector3Make(3, 0, 0);
	TEST(SabinaSphereContains(&S, &Inside), "sphere contains inside");

	Vector3 OnSurface = KannaVector3Make(5, 0, 0);
	TEST(SabinaSphereContains(&S, &OnSurface), "sphere contains on surface");

	Vector3 Outside = KannaVector3Make(6, 0, 0);
	TEST(!SabinaSphereContains(&S, &Outside), "sphere not contains outside");
}

static void TestSphereOverlaps(void) {
	Vector3 C0 = KannaVector3Zero();
	Sphere S0 = SabinaSphereMake(&C0, 3.0);

	Vector3 C1 = KannaVector3Make(4, 0, 0);
	Sphere S1 = SabinaSphereMake(&C1, 2.0);
	TEST(SabinaSphereOverlaps(&S0, &S1), "sphere overlap touching");

	Vector3 C2 = KannaVector3Make(10, 0, 0);
	Sphere S2 = SabinaSphereMake(&C2, 2.0);
	TEST(!SabinaSphereOverlaps(&S0, &S2), "sphere not overlap");
}

// ===========================================================================
//  OBB
// ===========================================================================

static void TestOBBMake(void) {
	Vector3 C = KannaVector3Zero();
	Vector3 H = KannaVector3Make(2, 3, 4);
	Quaternion Id = EuphylliaQuaternionIdentity();
	OBB Box = SabinaOBBMake(&C, &H, &Id);
	TEST(AE(Box.HalfExtents.Data[1], 3, 1e-12), "obb half y");
}

static void TestOBBContains(void) {
	Vector3 C = KannaVector3Zero();
	Vector3 H = KannaVector3Make(2, 2, 2);
	Quaternion Id = EuphylliaQuaternionIdentity();
	OBB Box = SabinaOBBMake(&C, &H, &Id);

	Vector3 Inside = KannaVector3Make(1, 1, 1);
	TEST(SabinaOBBContains(&Box, &Inside), "obb contains inside");

	Vector3 Outside = KannaVector3Make(3, 0, 0);
	TEST(!SabinaOBBContains(&Box, &Outside), "obb not contains outside");

	// Rotated OBB: 45° about Z, check a point that should be inside local space.
	// 45°Z回転OBB: ローカル空間で内側になる点をチェック。
	// q = cos(22.5°) + sin(22.5°)*k
	Quaternion Rot = EuphylliaQuaternionMake(0, 0, 0.382683, 0.92388);
	Vector3 HE = KannaVector3Make(2, 2, 2);
	OBB RotBox = SabinaOBBMake(&C, &HE, &Rot);
	Vector3 PInside = KannaVector3Make(1.0, 1.0, 0);
	TEST(SabinaOBBContains(&RotBox, &PInside), "obb rotated contains");
	Vector3 POutside = KannaVector3Make(3.0, 0.0, 0);
	TEST(!SabinaOBBContains(&RotBox, &POutside), "obb rotated not contains");
}

// ===========================================================================
//  Capsule
// ===========================================================================

static void TestCapsuleMake(void) {
	Vector3 S = KannaVector3Zero();
	Vector3 E = KannaVector3Make(0, 10, 0);
	Capsule Cap = SabinaCapsuleMake(&S, &E, 2.0);
	TEST(AE(Cap.Radius, 2.0, 1e-12), "capsule radius");
}

static void TestCapsuleClosestPoint(void) {
	Vector3 S = KannaVector3Make(0, 0, 0);
	Vector3 E = KannaVector3Make(0, 10, 0);
	Capsule Cap = SabinaCapsuleMake(&S, &E, 2.0);

	// Point at (5, 5, 0) — closest on axis is (0, 5, 0), push by 2 → (≈2, 5, 0).
	Vector3 P = KannaVector3Make(5, 5, 0);
	Vector3 CP = SabinaCapsuleClosestPoint(&Cap, &P);
	Vector3 AxisPt = KannaVector3Make(0, 5, 0);
	Vector3 Diff = KannaVector3Sub(&CP, &AxisPt);
	Real DistSq = KannaVector3Dot(&Diff, &Diff);
	TEST(AE(DistSq, 4.0, 1e-4), "capsule closest dist = radius²");
}

// ===========================================================================
//  Plane
// ===========================================================================

static void TestPlaneMake(void) {
	Vector3 N = KannaVector3Make(0, 1, 0);
	Plane P = SabinaPlaneMake(&N, 5.0);
	TEST(AE(P.Distance, 5.0, 1e-12), "plane distance");
}

static void TestPlaneFromPoints(void) {
	Vector3 A = KannaVector3Make(1, 0, 0);
	Vector3 B = KannaVector3Make(0, 1, 0);
	Vector3 C = KannaVector3Make(0, 0, 1);
	Plane P = SabinaPlaneMakeFromPoints(&A, &B, &C);
	// Normal should be (1,1,1)/sqrt(3).
	Real Len = SulettaSqrt(KannaVector3LengthSq(&P.Normal));
	TEST(AE(Len, 1.0, 1e-6), "plane normal unit");
}

static void TestPlaneSignedDistance(void) {
	Vector3 N = KannaVector3Make(0, 1, 0);
	Plane P = SabinaPlaneMake(&N, 5.0);

	Vector3 Above = KannaVector3Make(0, 10, 0);
	TEST(AE(SabinaPlaneSignedDistance(&P, &Above), 5.0, 1e-12), "plane dist above");

	Vector3 Below = KannaVector3Make(0, 0, 0);
	TEST(AE(SabinaPlaneSignedDistance(&P, &Below), -5.0, 1e-12), "plane dist below");
}

// ===========================================================================
//  Ray
// ===========================================================================

static void TestRayMake(void) {
	Vector3 O = KannaVector3Make(1, 2, 3);
	Vector3 D = KannaVector3Make(0, 1, 0);
	Ray R = SabinaRayMake(&O, &D);
	TEST(AE(R.Origin.Data[0], 1, 1e-12), "ray origin");
}

static void TestRayPointAt(void) {
	Vector3 O = KannaVector3Zero();
	Vector3 D = KannaVector3Make(1, 0, 0);
	Ray R = SabinaRayMake(&O, &D);

	Vector3 P = SabinaRayPointAt(&R, 5.0);
	TEST(AE(P.Data[0], 5.0, 1e-12), "ray point t=5");
}

static void TestRayClosestPoint(void) {
	Vector3 O = KannaVector3Make(0, 0, 0);
	Vector3 D = KannaVector3Make(1, 0, 0);
	Ray R = SabinaRayMake(&O, &D);

	Vector3 P1 = KannaVector3Make(10, 5, 0);
	Real T = SabinaRayClosestPoint(&R, &P1);
	TEST(AE(T, 10.0, 1e-12), "ray closest t=10");

	Vector3 P2 = KannaVector3Make(-5, 0, 0);
	T = SabinaRayClosestPoint(&R, &P2);
	TEST(AE(T, 0.0, 1e-12), "ray behind origin clamped to 0");
}

// ===========================================================================
//  Segment
// ===========================================================================

static void TestSegmentMake(void) {
	Vector3 S = KannaVector3Make(0, 0, 0);
	Vector3 E = KannaVector3Make(10, 0, 0);
	Segment Seg = SabinaSegmentMake(&S, &E);
	TEST(AE(Seg.End.Data[0], 10, 1e-12), "segment end");
}

static void TestSegmentLength(void) {
	Vector3 S = KannaVector3Make(0, 0, 0);
	Vector3 E = KannaVector3Make(3, 4, 0);
	Segment Seg = SabinaSegmentMake(&S, &E);
	TEST(AE(SabinaSegmentLength(&Seg), 5.0, 1e-6), "segment length 5");
}

static void TestSegmentClosestPoint(void) {
	Vector3 S = KannaVector3Make(0, 0, 0);
	Vector3 E = KannaVector3Make(10, 0, 0);
	Segment Seg = SabinaSegmentMake(&S, &E);

	Vector3 Q1 = KannaVector3Make(5, 5, 0);
	Vector3 CP = SabinaSegmentClosestPoint(&Seg, &Q1);
	TEST(AE(CP.Data[0], 5.0, 1e-12), "segment closest x=5");
	TEST(AE(CP.Data[1], 0.0, 1e-12), "segment closest y=0");

	// Beyond endpoint → clamped.
	Vector3 Q2 = KannaVector3Make(20, 0, 0);
	CP = SabinaSegmentClosestPoint(&Seg, &Q2);
	TEST(AE(CP.Data[0], 10.0, 1e-12), "segment clamp end");

	// Before start → clamped.
	Vector3 Q3 = KannaVector3Make(-5, 0, 0);
	CP = SabinaSegmentClosestPoint(&Seg, &Q3);
	TEST(AE(CP.Data[0], 0.0, 1e-12), "segment clamp start");
}

// ===========================================================================
//  Triangle
// ===========================================================================

static void TestTriangleMake(void) {
	Vector3 V0 = KannaVector3Zero();
	Vector3 V1 = KannaVector3Make(1, 0, 0);
	Vector3 V2 = KannaVector3Make(0, 1, 0);
	Triangle T = SabinaTriangleMake(&V0, &V1, &V2);
	TEST(AE(T.V1.Data[0], 1, 1e-12), "triangle v1");
}

static void TestTriangleNormal(void) {
	Vector3 V0 = KannaVector3Zero();
	Vector3 V1 = KannaVector3Make(1, 0, 0);
	Vector3 V2 = KannaVector3Make(0, 1, 0);
	Triangle T = SabinaTriangleMake(&V0, &V1, &V2);
	Vector3 N = SabinaTriangleNormal(&T);
	TEST(AE(N.Data[0], 0.0, 1e-6), "tri normal x");
	TEST(AE(N.Data[1], 0.0, 1e-6), "tri normal y");
	TEST(AE(N.Data[2], 1.0, 1e-6), "tri normal z");
}

static void TestTriangleArea(void) {
	Vector3 V0 = KannaVector3Zero();
	Vector3 V1 = KannaVector3Make(2, 0, 0);
	Vector3 V2 = KannaVector3Make(0, 3, 0);
	Triangle T = SabinaTriangleMake(&V0, &V1, &V2);
	TEST(AE(SabinaTriangleArea(&T), 3.0, 1e-6), "tri area = 3");
}

int main(void) {
	fprintf(stderr, "=== TestGeometry ===\n");

	RUN_TEST(TestAABBMake, "AABB: make");
	RUN_TEST(TestAABBMakeCenterExtents, "AABB: make center-extents");
	RUN_TEST(TestAABBContains, "AABB: contains");
	RUN_TEST(TestAABBOverlaps, "AABB: overlaps");
	RUN_TEST(TestSphereMake, "Sphere: make");
	RUN_TEST(TestSphereContains, "Sphere: contains");
	RUN_TEST(TestSphereOverlaps, "Sphere: overlaps");
	RUN_TEST(TestOBBMake, "OBB: make");
	RUN_TEST(TestOBBContains, "OBB: contains");
	RUN_TEST(TestCapsuleMake, "Capsule: make");
	RUN_TEST(TestCapsuleClosestPoint, "Capsule: closest point");
	RUN_TEST(TestPlaneMake, "Plane: make");
	RUN_TEST(TestPlaneFromPoints, "Plane: from points");
	RUN_TEST(TestPlaneSignedDistance, "Plane: signed distance");
	RUN_TEST(TestRayMake, "Ray: make");
	RUN_TEST(TestRayPointAt, "Ray: point at t");
	RUN_TEST(TestRayClosestPoint, "Ray: closest point");
	RUN_TEST(TestSegmentMake, "Segment: make");
	RUN_TEST(TestSegmentLength, "Segment: length");
	RUN_TEST(TestSegmentClosestPoint, "Segment: closest point");
	RUN_TEST(TestTriangleMake, "Triangle: make");
	RUN_TEST(TestTriangleNormal, "Triangle: normal");
	RUN_TEST(TestTriangleArea, "Triangle: area");

	fprintf(stderr, "\n=== %d passed, 0 failed ===\n", Passed);
	return 0;
}
