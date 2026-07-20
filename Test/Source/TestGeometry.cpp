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
	Real T = SabinaRayClosestT(&R, &P1);
	TEST(AE(T, 10.0, 1e-12), "ray closest t=10");

	Vector3 P2 = KannaVector3Make(-5, 0, 0);
	T = SabinaRayClosestT(&R, &P2);
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

static void TestAABBMergeExpand(void) {
	Vector3 MinA = KannaVector3Make(0, 0, 0);
	Vector3 MaxA = KannaVector3Make(2, 2, 2);
	AABB A = SabinaAABBMake(&MinA, &MaxA);
	Vector3 MinB = KannaVector3Make(3, 3, 3);
	Vector3 MaxB = KannaVector3Make(5, 5, 5);
	AABB B = SabinaAABBMake(&MinB, &MaxB);
	AABB M = SabinaAABBMerge(&A, &B);
	TEST(AE(M.Min.Data[0], 0.0, 1e-12), "merge min");
	TEST(AE(M.Max.Data[0], 5.0, 1e-12), "merge max");
	AABB E = SabinaAABBExpand(&A, 1.0);
	TEST(AE(E.Min.Data[0], -1.0, 1e-12), "expand min");
	TEST(AE(E.Max.Data[0], 3.0, 1e-12), "expand max");
}

static void TestAABBClosestPointRay(void) {
	Vector3 Min = KannaVector3Make(0, 0, 0);
	Vector3 Max = KannaVector3Make(10, 10, 10);
	AABB Box = SabinaAABBMake(&Min, &Max);
	Vector3 P = KannaVector3Make(-5, 5, 5);
	Vector3 CP = SabinaAABBClosestPoint(&Box, &P);
	TEST(AE(CP.Data[0], 0.0, 1e-12), "closest x=0");

	Vector3 O = KannaVector3Make(-1, 5, 5);
	Vector3 D = KannaVector3Make(1, 0, 0);
	Ray R = SabinaRayMake(&O, &D);
	Real T0, T1;
	int Hit = SabinaAABBIntersectRay(&Box, &R, &T0, &T1);
	TEST(Hit, "ray aabb hit");
}

static void TestSphereClosestPointRay(void) {
	Vector3 C = KannaVector3Zero();
	Sphere S = SabinaSphereMake(&C, 5.0);
	Vector3 P = KannaVector3Make(10, 0, 0);
	Vector3 CP = SabinaSphereClosestPoint(&S, &P);
	TEST(AE(CP.Data[0], 5.0, 1e-6), "sphere closest x");

	Vector3 O = KannaVector3Make(-10, 0, 0);
	Vector3 D = KannaVector3Make(1, 0, 0);
	Ray R = SabinaRayMake(&O, &D);
	Real T0, T1;
	int Hit = SabinaSphereIntersectRay(&S, &R, &T0, &T1);
	TEST(Hit, "sphere ray hit");
	TEST(AE(T0, 5.0, 1e-4), "sphere ray t0=5");
}

static void TestOBBClosestPointCorners(void) {
	Vector3 C = KannaVector3Zero();
	Vector3 H = KannaVector3Make(2, 2, 2);
	Quaternion Id = EuphylliaQuaternionIdentity();
	OBB Box = SabinaOBBMake(&C, &H, &Id);
	Vector3 P = KannaVector3Make(10, 0, 0);
	Vector3 CP = SabinaOBBClosestPoint(&Box, &P);
	TEST(AE(CP.Data[0], 2.0, 1e-12), "obb closest x=2");
	Vector3 Corners[8];
	SabinaOBBGetCorners(&Box, Corners);
	TEST(AE(Corners[0].Data[0], -2.0, 1e-12), "corner 0 x");
	TEST(AE(Corners[7].Data[0], 2.0, 1e-12), "corner 7 x");
}

static void TestRayIntersectPlaneTriangle(void) {
	Vector3 N = KannaVector3Make(0, 1, 0);
	Plane P = SabinaPlaneMake(&N, 5.0);
	Vector3 O = KannaVector3Make(0, 0, 0);
	Vector3 D = KannaVector3Make(0, 1, 0);
	Ray R = SabinaRayMake(&O, &D);
	Real T = SabinaRayIntersectPlane(&R, &P);
	TEST(AE(T, 5.0, 1e-6), "ray plane t=5");

	Vector3 V0 = KannaVector3Zero();
	Vector3 V1 = KannaVector3Make(1, 0, 0);
	Vector3 V2 = KannaVector3Make(0, 1, 0);
	Triangle Tri = SabinaTriangleMake(&V0, &V1, &V2);
	Vector3 O2 = KannaVector3Make(0.25, 0.25, 1);
	Vector3 D2 = KannaVector3Make(0, 0, -1);
	Ray R2 = SabinaRayMake(&O2, &D2);
	Real TTri;
	int Hit = SabinaRayIntersectTriangle(&R2, &Tri, &TTri);
	TEST(Hit, "ray triangle hit");
}

// ===========================================================================
//  1.12 Intersection tests
// ===========================================================================

static void TestIntersectSphereSphere(void) {
	Vector3 Z = KannaVector3Make(0,0,0);
	Vector3 P4 = KannaVector3Make(4,0,0);
	Vector3 P10 = KannaVector3Make(10,0,0);
	Sphere A = SabinaSphereMake(&Z, 3.0);
	Sphere B = SabinaSphereMake(&P4, 2.0);
	Sphere C = SabinaSphereMake(&P10, 1.0);
	TEST(IntersectSphereSphere(&A, &B), "sphere-sphere overlap");
	TEST(!IntersectSphereSphere(&A, &C), "sphere-sphere separated");
}

static void TestIntersectSphereAABB(void) {
	Vector3 Z = KannaVector3Make(0,0,0);
	Vector3 F5 = KannaVector3Make(5,5,5);
	Vector3 S3 = KannaVector3Make(3,3,3);
	Vector3 S10 = KannaVector3Make(10,10,10);
	AABB Box = SabinaAABBMake(&Z, &F5);
	Sphere S = SabinaSphereMake(&S3, 2.0);
	Sphere S2 = SabinaSphereMake(&S10, 1.0);
	TEST(IntersectSphereAABB(&S, &Box), "sphere-aabb inside");
	TEST(!IntersectSphereAABB(&S2, &Box), "sphere-aabb outside");
}

static void TestIntersectSpherePlane(void) {
	Vector3 Y1 = KannaVector3Make(0,1,0);
	Vector3 S2 = KannaVector3Make(0,2,0);
	Vector3 S10 = KannaVector3Make(0,10,0);
	Sphere S = SabinaSphereMake(&S2, 2.0);
	Sphere S2s = SabinaSphereMake(&S10, 1.0);
	Plane P = SabinaPlaneMake(&Y1, 0.0);
	TEST(IntersectSpherePlane(&S, &P), "sphere-plane touching");
	TEST(!IntersectSpherePlane(&S2s, &P), "sphere-plane far");
}

static void TestIntersectSphereCapsule(void) {
	Vector3 S5 = KannaVector3Make(5,0,0);
	Vector3 S20 = KannaVector3Make(20,0,0);
	Vector3 Z = KannaVector3Make(0,0,0);
	Vector3 Y10 = KannaVector3Make(0,10,0);
	Sphere S = SabinaSphereMake(&S5, 2.0);
	Sphere S2 = SabinaSphereMake(&S20, 1.0);
	Capsule C = SabinaCapsuleMake(&Z, &Y10, 2.0);
	TEST(IntersectSphereCapsule(&S, &C), "sphere-capsule overlap");
	TEST(!IntersectSphereCapsule(&S2, &C), "sphere-capsule far");
}

static void TestIntersectAABBPlane(void) {
	Vector3 Z = KannaVector3Make(0,0,0);
	Vector3 T2 = KannaVector3Make(2,2,2);
	Vector3 Y1 = KannaVector3Make(0,1,0);
	AABB Box = SabinaAABBMake(&Z, &T2);
	Plane P = SabinaPlaneMake(&Y1, 1.0);
	Plane P2 = SabinaPlaneMake(&Y1, 10.0);
	TEST(IntersectAABBPlane(&Box, &P), "aabb-plane cutting");
	TEST(!IntersectAABBPlane(&Box, &P2), "aabb-plane far");
}

static void TestIntersectAABBOBB(void) {
	Vector3 N2 = KannaVector3Make(-2,-2,-2);
	Vector3 P2 = KannaVector3Make(2,2,2);
	Vector3 O1 = KannaVector3Make(0,0,0);
	Vector3 O10 = KannaVector3Make(10,0,0);
	Vector3 H1 = KannaVector3Make(1,1,1);
	Quaternion Id = EuphylliaQuaternionIdentity();
	AABB Box = SabinaAABBMake(&N2, &P2);
	OBB O = SabinaOBBMake(&O1, &H1, &Id);
	OBB O2 = SabinaOBBMake(&O10, &H1, &Id);
	TEST(IntersectAABBOBB(&Box, &O), "aabb-obb overlap");
	TEST(!IntersectAABBOBB(&Box, &O2), "aabb-obb far");
}

static void TestIntersectOBBOBB(void) {
	Vector3 Z = KannaVector3Make(0,0,0);
	Vector3 O1 = KannaVector3Make(1,1,1);
	Vector3 O10 = KannaVector3Make(10,0,0);
	Vector3 H2 = KannaVector3Make(2,2,2);
	Quaternion Id = EuphylliaQuaternionIdentity();
	OBB A = SabinaOBBMake(&Z, &H2, &Id);
	OBB B = SabinaOBBMake(&O1, &H2, &Id);
	OBB C = SabinaOBBMake(&O10, &H2, &Id);
	TEST(IntersectOBBOBB(&A, &B), "obb-obb overlap");
	TEST(!IntersectOBBOBB(&A, &C), "obb-obb far");
}

static void TestIntersectOBBPlane(void) {
	Vector3 Z = KannaVector3Make(0,0,0);
	Vector3 H2 = KannaVector3Make(2,2,2);
	Vector3 Y1 = KannaVector3Make(0,1,0);
	Quaternion Id = EuphylliaQuaternionIdentity();
	OBB O = SabinaOBBMake(&Z, &H2, &Id);
	Plane P = SabinaPlaneMake(&Y1, 1.0);
	Plane P2 = SabinaPlaneMake(&Y1, 10.0);
	TEST(IntersectOBBPlane(&O, &P), "obb-plane cutting");
	TEST(!IntersectOBBPlane(&O, &P2), "obb-plane far");
}

static void TestIntersectCapsuleCapsule(void) {
	Vector3 Z = KannaVector3Make(0,0,0);
	Vector3 Y10 = KannaVector3Make(0,10,0);
	Vector3 C2 = KannaVector3Make(0,5,2);
	Vector3 C4 = KannaVector3Make(0,5,4);
	Vector3 CZ10 = KannaVector3Make(0,0,10);
	Vector3 CY10 = KannaVector3Make(0,10,10);
	Capsule A = SabinaCapsuleMake(&Z, &Y10, 1.0);
	Capsule B = SabinaCapsuleMake(&C2, &C4, 1.0);
	Capsule D = SabinaCapsuleMake(&CZ10, &CY10, 1.0);
	TEST(IntersectCapsuleCapsule(&A, &B), "capsule-capsule overlap");
	TEST(!IntersectCapsuleCapsule(&A, &D), "capsule-capsule far");
}

// ===========================================================================
//  1.13 Ray/segment intersection tests
// ===========================================================================

static void TestIntersectRaySphere(void) {
	Vector3 O = KannaVector3Make(-10,0,0);
	Vector3 D = KannaVector3Make(1,0,0);
	Vector3 SC = KannaVector3Zero();
	Ray R = SabinaRayMake(&O, &D);
	Sphere S = SabinaSphereMake(&SC, 5.0);
	Real T0, T1;
	int Hit = IntersectRaySphere(&R, &S, &T0, &T1);
	TEST(Hit, "ray-sphere hit");
	TEST(T0 >= 0, "ray-sphere t0>=0");
}

static void TestIntersectRayAABB(void) {
	Vector3 O = KannaVector3Make(-5,2,2);
	Vector3 D = KannaVector3Make(1,0,0);
	Vector3 Min = KannaVector3Make(0,0,0);
	Vector3 Max = KannaVector3Make(10,10,10);
	Ray R = SabinaRayMake(&O, &D);
	AABB Box = SabinaAABBMake(&Min, &Max);
	Real T0, T1;
	int Hit = IntersectRayAABB(&R, &Box, &T0, &T1);
	TEST(Hit, "ray-aabb hit");
}

static void TestIntersectRayOBB(void) {
	Vector3 O = KannaVector3Make(-5,0,0);
	Vector3 D = KannaVector3Make(1,0,0);
	Vector3 C = KannaVector3Zero();
	Vector3 HE = KannaVector3Make(2,2,2);
	Quaternion Id = EuphylliaQuaternionIdentity();
	Ray R = SabinaRayMake(&O, &D);
	OBB Box = SabinaOBBMake(&C, &HE, &Id);
	Real T0, T1;
	int Hit = IntersectRayOBB(&R, &Box, &T0, &T1);
	TEST(Hit, "ray-obb hit");
}

static void TestIntersectRayPlane(void) {
	Vector3 O = KannaVector3Make(0,-5,0);
	Vector3 D = KannaVector3Make(0,1,0);
	Vector3 N = KannaVector3Make(0,1,0);
	Ray R = SabinaRayMake(&O, &D);
	Plane P = SabinaPlaneMake(&N, 0.0);
	Real TOut;
	int Hit = IntersectRayPlane(&R, &P, &TOut);
	TEST(Hit, "ray-plane hit");
	TEST(TOut >= 0, "ray-plane t>=0");
}

static void TestIntersectRayCapsule(void) {
	Vector3 O = KannaVector3Make(-5,0,0);
	Vector3 D = KannaVector3Make(1,0,0);
	Vector3 CS = KannaVector3Make(-2,0,0);
	Vector3 CE = KannaVector3Make(2,0,0);
	Ray R = SabinaRayMake(&O, &D);
	Capsule Cap = SabinaCapsuleMake(&CS, &CE, 1.0);
	Real T0, T1;
	int Hit = IntersectRayCapsule(&R, &Cap, &T0, &T1);
	TEST(Hit, "ray-capsule hit");
}

static void TestIntersectRayTriangle(void) {
	Vector3 O = KannaVector3Make(0.25,0.25,1);
	Vector3 D = KannaVector3Make(0,0,-1);
	Vector3 V0 = KannaVector3Zero();
	Vector3 V1 = KannaVector3Make(1,0,0);
	Vector3 V2 = KannaVector3Make(0,1,0);
	Ray R = SabinaRayMake(&O, &D);
	Triangle T = SabinaTriangleMake(&V0, &V1, &V2);
	Real TOut;
	int Hit = IntersectRayTriangle(&R, &T, &TOut);
	TEST(Hit, "ray-triangle hit");
}

static void TestIntersectSegmentSphere(void) {
	Vector3 S0 = KannaVector3Make(-10,0,0);
	Vector3 S1 = KannaVector3Make(10,0,0);
	Vector3 SC = KannaVector3Zero();
	Segment Seg = SabinaSegmentMake(&S0, &S1);
	Sphere Sph = SabinaSphereMake(&SC, 3.0);
	Real T0, T1;
	int Hit = IntersectSegmentSphere(&Seg, &Sph, &T0, &T1);
	TEST(Hit, "seg-sphere hit");
}

static void TestIntersectSegmentAABB(void) {
	Vector3 S0 = KannaVector3Make(-5,0,0);
	Vector3 S1 = KannaVector3Make(15,0,0);
	Vector3 Min = KannaVector3Make(0,-2,-2);
	Vector3 Max = KannaVector3Make(10,2,2);
	Segment Seg = SabinaSegmentMake(&S0, &S1);
	AABB Box = SabinaAABBMake(&Min, &Max);
	Real T0, T1;
	int Hit = IntersectSegmentAABB(&Seg, &Box, &T0, &T1);
	TEST(Hit, "seg-aabb hit");
}

static void TestIntersectSegmentOBB(void) {
	Vector3 S0 = KannaVector3Make(-5,0,0);
	Vector3 S1 = KannaVector3Make(5,0,0);
	Vector3 OC = KannaVector3Zero();
	Vector3 HE = KannaVector3Make(2,2,2);
	Quaternion Id = EuphylliaQuaternionIdentity();
	Segment Seg = SabinaSegmentMake(&S0, &S1);
	OBB Box = SabinaOBBMake(&OC, &HE, &Id);
	Real T0, T1;
	int Hit = IntersectSegmentOBB(&Seg, &Box, &T0, &T1);
	TEST(Hit, "seg-obb hit");
}

static void TestIntersectSegmentPlane(void) {
	Vector3 S0 = KannaVector3Make(0,-5,0);
	Vector3 S1 = KannaVector3Make(0,5,0);
	Vector3 N = KannaVector3Make(0,1,0);
	Segment Seg = SabinaSegmentMake(&S0, &S1);
	Plane P = SabinaPlaneMake(&N, 0.0);
	Real TOut;
	int Hit = IntersectSegmentPlane(&Seg, &P, &TOut);
	TEST(Hit, "seg-plane hit");
	TEST(TOut >= 0 && TOut <= 1, "seg-plane t in [0,1]");
}

// ===========================================================================
//  1.14 ClosestPoint / Distance tests
// ===========================================================================

static void TestClosestPointPointSphere(void) {
	Vector3 C = KannaVector3Make(0,0,0);
	Sphere S = SabinaSphereMake(&C, 5.0);
	Vector3 Inside = KannaVector3Make(2,0,0);
	Vector3 CP = ClosestPointPointSphere(&S, &Inside);
	// Inside point → stays at position
	TEST(AE(CP.Data[0], 2.0, 1e-12), "cp sphere inside");

	Vector3 Outside = KannaVector3Make(10,0,0);
	CP = ClosestPointPointSphere(&S, &Outside);
	// Outside point → projected to surface at (5,0,0)
	TEST(AE(CP.Data[0], 5.0, 1e-6), "cp sphere surface");
}

static void TestClosestPointPointAABB(void) {
	Vector3 Min = KannaVector3Make(0,0,0);
	Vector3 Max = KannaVector3Make(10,10,10);
	AABB Box = SabinaAABBMake(&Min, &Max);
	Vector3 P = KannaVector3Make(-5, 5, 5);
	Vector3 CP = ClosestPointPointAABB(&Box, &P);
	TEST(AE(CP.Data[0], 0.0, 1e-12), "cp aabb clamped x=0");
}

static void TestClosestPointPointOBB(void) {
	Vector3 C = KannaVector3Zero();
	Vector3 HE = KannaVector3Make(2,2,2);
	Quaternion Id = EuphylliaQuaternionIdentity();
	OBB Box = SabinaOBBMake(&C, &HE, &Id);
	Vector3 P = KannaVector3Make(10,0,0);
	Vector3 CP = ClosestPointPointOBB(&Box, &P);
	TEST(AE(CP.Data[0], 2.0, 1e-12), "cp obb clamped x=2");
}

static void TestClosestPointPointCapsule(void) {
	Vector3 CS = KannaVector3Make(0,0,0);
	Vector3 CE = KannaVector3Make(0,10,0);
	Capsule Cap = SabinaCapsuleMake(&CS, &CE, 2.0);
	Vector3 P = KannaVector3Make(5,5,0);
	Vector3 CP = ClosestPointPointCapsule(&Cap, &P);
	Vector3 AxisPt = KannaVector3Make(0,5,0);
	Vector3 Diff = KannaVector3Sub(&CP, &AxisPt);
	Real D = KannaVector3Length(&Diff);
	TEST(AE(D, 2.0, 1e-4), "cp capsule dist = radius");
}

static void TestClosestPointPointPlane(void) {
	Vector3 N = KannaVector3Make(0,1,0);
	Plane P = SabinaPlaneMake(&N, 5.0);
	Vector3 Pt = KannaVector3Make(0,10,0);
	Vector3 CP = ClosestPointPointPlane(&P, &Pt);
	TEST(AE(CP.Data[1], 5.0, 1e-12), "cp plane y=5");
}

static void TestClosestPointPointTriangle(void) {
	Vector3 V0 = KannaVector3Zero();
	Vector3 V1 = KannaVector3Make(1,0,0);
	Vector3 V2 = KannaVector3Make(0,1,0);
	Triangle T = SabinaTriangleMake(&V0, &V1, &V2);
	Vector3 P = KannaVector3Make(0.25, 0.25, 0);
	Vector3 CP = ClosestPointPointTriangle(&T, &P);
	TEST(AE(CP.Data[0], 0.25, 1e-12), "cp triangle x=0.25");
	TEST(AE(CP.Data[1], 0.25, 1e-12), "cp triangle y=0.25");
}

static void TestDistanceSphereSphere(void) {
	Vector3 C0 = KannaVector3Zero();
	Vector3 C1 = KannaVector3Make(10,0,0);
	Sphere A = SabinaSphereMake(&C0, 3.0);
	Sphere B = SabinaSphereMake(&C1, 2.0);
	Real D = DistanceSphereSphere(&A, &B);
	// Centers 10 apart, radii 3+2=5 → distance = 10-5 = 5
	TEST(AE(D, 5.0, 1e-6), "dist sphere-sphere = 5");

	Sphere C = SabinaSphereMake(&C0, 10.0);
	D = DistanceSphereSphere(&A, &C);
	// Overlapping → 0
	TEST(AE(D, 0.0, 1e-12), "dist sphere-sphere overlap = 0");
}

static void TestDistanceAABBAABB(void) {
	Vector3 MinA = KannaVector3Make(0,0,0);
	Vector3 MaxA = KannaVector3Make(2,2,2);
	Vector3 MinB = KannaVector3Make(5,0,0);
	Vector3 MaxB = KannaVector3Make(7,2,2);
	AABB A = SabinaAABBMake(&MinA, &MaxA);
	AABB B = SabinaAABBMake(&MinB, &MaxB);
	Real D = DistanceAABBAABB(&A, &B);
	TEST(AE(D, 3.0, 1e-6), "dist aabb-aabb = 3");

	// Overlapping → 0
	Vector3 Tmp3 = KannaVector3Make(3,3,3);
	AABB C = SabinaAABBMake(&MinA, &Tmp3);
	D = DistanceAABBAABB(&A, &C);
	TEST(AE(D, 0.0, 1e-12), "dist aabb-aabb overlap = 0");
}

static void TestDistanceOBBOBB(void) {
	Quaternion Id = EuphylliaQuaternionIdentity();
	Vector3 C0 = KannaVector3Make(0,0,0);
	Vector3 C1 = KannaVector3Make(10,0,0);
	Vector3 HE = KannaVector3Make(1,1,1);
	OBB A = SabinaOBBMake(&C0, &HE, &Id);
	OBB B = SabinaOBBMake(&C1, &HE, &Id);
	Real D = DistanceOBBOBB(&A, &B);
	// Centers 10 apart, each half extends 1 → gap = 10-2 = 8
	TEST(AE(D, 8.0, 1e-6), "dist obb-obb = 8");

	// Overlapping → negative
	Vector3 H5 = KannaVector3Make(5,5,5);
	OBB C = SabinaOBBMake(&C0, &H5, &Id);
	D = DistanceOBBOBB(&A, &C);
	TEST(D < REAL_ZERO, "dist obb-obb overlap negative");
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
	RUN_TEST(TestAABBMergeExpand, "AABB: merge and expand");
	RUN_TEST(TestAABBClosestPointRay, "AABB: closest point and ray");
	RUN_TEST(TestSphereClosestPointRay, "Sphere: closest point and ray");
	RUN_TEST(TestOBBClosestPointCorners, "OBB: closest point and corners");
	RUN_TEST(TestRayIntersectPlaneTriangle, "Ray: intersect plane and triangle");
	RUN_TEST(TestIntersectSphereSphere, "Intersect: Sphere-Sphere");
	RUN_TEST(TestIntersectSphereAABB, "Intersect: Sphere-AABB");
	RUN_TEST(TestIntersectSpherePlane, "Intersect: Sphere-Plane");
	RUN_TEST(TestIntersectSphereCapsule, "Intersect: Sphere-Capsule");
	RUN_TEST(TestIntersectAABBPlane, "Intersect: AABB-Plane");
	RUN_TEST(TestIntersectAABBOBB, "Intersect: AABB-OBB");
	RUN_TEST(TestIntersectOBBOBB, "Intersect: OBB-OBB");
	RUN_TEST(TestIntersectOBBPlane, "Intersect: OBB-Plane");
	RUN_TEST(TestIntersectCapsuleCapsule, "Intersect: Capsule-Capsule");
	RUN_TEST(TestIntersectRaySphere, "IntersectRay: Sphere");
	RUN_TEST(TestIntersectRayAABB, "IntersectRay: AABB");
	RUN_TEST(TestIntersectRayOBB, "IntersectRay: OBB");
	RUN_TEST(TestIntersectRayPlane, "IntersectRay: Plane");
	RUN_TEST(TestIntersectRayCapsule, "IntersectRay: Capsule");
	RUN_TEST(TestIntersectRayTriangle, "IntersectRay: Triangle");
	RUN_TEST(TestIntersectSegmentSphere, "IntersectSegment: Sphere");
	RUN_TEST(TestIntersectSegmentAABB, "IntersectSegment: AABB");
	RUN_TEST(TestIntersectSegmentOBB, "IntersectSegment: OBB");
	RUN_TEST(TestIntersectSegmentPlane, "IntersectSegment: Plane");
	RUN_TEST(TestClosestPointPointSphere, "ClosestPoint: Point-Sphere");
	RUN_TEST(TestClosestPointPointAABB, "ClosestPoint: Point-AABB");
	RUN_TEST(TestClosestPointPointOBB, "ClosestPoint: Point-OBB");
	RUN_TEST(TestClosestPointPointCapsule, "ClosestPoint: Point-Capsule");
	RUN_TEST(TestClosestPointPointPlane, "ClosestPoint: Point-Plane");
	RUN_TEST(TestClosestPointPointTriangle, "ClosestPoint: Point-Triangle");
	RUN_TEST(TestDistanceSphereSphere, "Distance: Sphere-Sphere");
	RUN_TEST(TestDistanceAABBAABB, "Distance: AABB-AABB");
	RUN_TEST(TestDistanceOBBOBB, "Distance: OBB-OBB");

	fprintf(stderr, "\n=== %d passed, 0 failed ===\n", Passed);
	return 0;
}
