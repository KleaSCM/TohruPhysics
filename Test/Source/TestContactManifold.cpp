/**
 * Unit tests for ContactManifold generation.
 * 接触マニフォールド生成の単体テストね。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include <TohruPhysics/ContactManifold.h>
#include <TohruPhysics/Math.h>
#include <stdio.h>
#include <stdlib.h>
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

// ===========================================================================
//  Support function wrappers (for GJK-based manifold)
// ===========================================================================

static Vector3 SupportSphereWrapper(const void *Shape, const Vector3 *Dir) {
	return SupportSphere((const Sphere *)Shape, Dir);
}

// ===========================================================================
//  Tests
// ===========================================================================

static void TestManifoldSphereSphere(void) {
	Sphere A = { {0,0,0}, 2.0f };
	Sphere B = { {4,0,0}, 2.0f };
	Transform TxA = KaedeTransformIdentity();
	Transform TxB = KaedeTransformIdentity();
	B.Center.Data[0] = 3.5f;

	ContactManifold M;
	ManifoldSphereSphere(&A, &TxA, &B, &TxB, &M);
	TEST(M.PointCount > 0, "sphere-sphere has contact");
	TEST(M.Points[0].Penetration > 0, "sphere-sphere penetration > 0");
	TEST(M.Points[0].Normal.Data[0] > 0, "sphere-sphere normal along +X");
}

static void TestManifoldSphereSphereSeparated(void) {
	Sphere A = { {0,0,0}, 1.0f };
	Sphere B = { {10,0,0}, 1.0f };
	Transform TxA = KaedeTransformIdentity();
	Transform TxB = KaedeTransformIdentity();

	ContactManifold M;
	ManifoldSphereSphere(&A, &TxA, &B, &TxB, &M);
	TEST(M.PointCount == 0, "sphere-sphere separated has no contact");
}

static void TestManifoldSphereAABB(void) {
	Sphere S = { {0,0,0}, 2.0f };
	Vector3 Min = { {-3,-3,-3} };
	Vector3 Max = { {3,3,3} };
	AABB Box = SabinaAABBMake(&Min, &Max);
	Transform TxS = KaedeTransformIdentity();
	Transform TxB = KaedeTransformIdentity();

	ContactManifold M;
	ManifoldSphereAABB(&S, &TxS, &Box, &TxB, &M);
	TEST(M.PointCount > 0, "sphere-aabb overlapping has contact");
	TEST(M.Points[0].Penetration > 0, "sphere-aabb penetration > 0");
}

static void TestManifoldSphereOBB(void) {
	Sphere S = { {0,0,0}, 2.0f };
	Vector3 HE = { {2,2,2} };
	Vector3 Zero = KannaVector3Zero();
	Quaternion QR = EuphylliaQuaternionIdentity();
	OBB Box = SabinaOBBMake(&Zero, &HE, &QR);
	Transform TxS = KaedeTransformIdentity();
	Transform TxO = KaedeTransformIdentity();

	ContactManifold M;
	ManifoldSphereOBB(&S, &TxS, &Box, &TxO, &M);
	TEST(M.PointCount > 0, "sphere-obb overlapping has contact");
	TEST(M.Points[0].Penetration > 0, "sphere-obb penetration > 0");
}

static void TestManifoldSphereCapsule(void) {
	Sphere S = { {0,0,0}, 2.0f };
	Vector3 Start = { {4,0,0} };
	Vector3 End   = { {4,4,0} };
	Capsule Cap = SabinaCapsuleMake(&Start, &End, 1.5f);
	Transform TxS = KaedeTransformIdentity();
	Transform TxC = KaedeTransformIdentity();

	ContactManifold M;
	ManifoldSphereCapsule(&S, &TxS, &Cap, &TxC, &M);
	TEST(M.PointCount > 0, "sphere-capsule has contact");
	TEST(M.Points[0].Penetration > 0, "sphere-capsule penetration > 0");
}

static void TestManifoldAABBAABB(void) {
	Vector3 MinA = { {-3,-3,-3} };
	Vector3 MaxA = { {3,3,3} };
	Vector3 MinB = { {2,2,2} };
	Vector3 MaxB = { {8,8,8} };
	AABB BoxA = SabinaAABBMake(&MinA, &MaxA);
	AABB BoxB = SabinaAABBMake(&MinB, &MaxB);
	Transform TxA = KaedeTransformIdentity();
	Transform TxB = KaedeTransformIdentity();

	ContactManifold M;
	ManifoldAABBAABB(&BoxA, &TxA, &BoxB, &TxB, &M);
	TEST(M.PointCount > 0, "aabb-aabb has contact");
	TEST(M.Points[0].Penetration > 0, "aabb-aabb penetration > 0");
}

static void TestManifoldOBBOBB(void) {
	Vector3 HE = { {2,2,2} };
	Quaternion QR = EuphylliaQuaternionIdentity();
	Vector3 Zero = KannaVector3Zero();
	OBB BoxA = SabinaOBBMake(&Zero, &HE, &QR);
	Vector3 Offset = { {2,0,0} };
	OBB BoxB = SabinaOBBMake(&Offset, &HE, &QR);
	Transform TxA = KaedeTransformIdentity();
	Transform TxB = KaedeTransformIdentity();

	ContactManifold M;
	ManifoldOBBOBB(&BoxA, &TxA, &BoxB, &TxB, &M);
	TEST(M.PointCount > 0, "obb-obb has contact");
	Real NLen = KannaVector3Length(&M.Normal);
	TEST(NLen > 0, "obb-obb normal is non-zero");
}

static void TestClipPolygonToPlane(void) {
	Vector3 Quad[4] = {
		{-1,-1,0}, {1,-1,0}, {1,1,0}, {-1,1,0}
	};
	Plane CP;
	CP.Normal = KannaVector3Make(0,0,1);
	CP.Distance = 0.5f;

	Vector3 Out[8];
	int Count = ClipPolygonToPlane(Quad, 4, &CP, Out, 8);
	TEST(Count == 0, "clip all behind");

	CP.Distance = -0.5f;
	Count = ClipPolygonToPlane(Quad, 4, &CP, Out, 8);
	TEST(Count == 4, "clip all in front");
}

static void TestManifoldCapsuleCapsule(void) {
	Vector3 StartA = { {0,0,0} };
	Vector3 EndA   = { {0,2,0} };
	Vector3 StartB = { {0,1,1.5f} };
	Vector3 EndB   = { {0,3,1.5f} };
	Capsule CapA = SabinaCapsuleMake(&StartA, &EndA, 1.0f);
	Capsule CapB = SabinaCapsuleMake(&StartB, &EndB, 1.0f);
	Transform TxA = KaedeTransformIdentity();
	Transform TxB = KaedeTransformIdentity();

	ContactManifold M;
	ManifoldCapsuleCapsule(&CapA, &TxA, &CapB, &TxB, &M);
	TEST(M.PointCount > 0, "capsule-capsule has contact");
	TEST(M.Points[0].Penetration > 0, "capsule-capsule penetration > 0");
}

static void TestManifoldCapsuleCapsuleNoContact(void) {
	Vector3 StartA = { {0,0,0} };
	Vector3 EndA   = { {0,2,0} };
	Vector3 StartB = { {10,0,0} };
	Vector3 EndB   = { {10,2,0} };
	Capsule CapA = SabinaCapsuleMake(&StartA, &EndA, 1.0f);
	Capsule CapB = SabinaCapsuleMake(&StartB, &EndB, 1.0f);
	Transform TxA = KaedeTransformIdentity();
	Transform TxB = KaedeTransformIdentity();

	ContactManifold M;
	ManifoldCapsuleCapsule(&CapA, &TxA, &CapB, &TxB, &M);
	TEST(M.PointCount == 0, "capsule-capsule separated has no contact");
}

static void TestManifoldConvexConvex(void) {
	Sphere A = { {0,0,0}, 3.0f };
	Sphere B = { {1,0,0}, 3.0f };

	ContactManifold M;
	ManifoldConvexConvex(&A, SupportSphereWrapper,
		&B, SupportSphereWrapper, 10.0f, &M);
	TEST(M.PointCount > 0, "convex-convex (sphere-sphere) has contact");
}

// ===========================================================================
//  Main
// ===========================================================================

int main(void) {
	fprintf(stderr, "=== TestContactManifold ===\n");

	RUN_TEST(TestManifoldSphereSphere, "Manifold: sphere-sphere");
	RUN_TEST(TestManifoldSphereSphereSeparated, "Manifold: sphere-sphere separated");
	RUN_TEST(TestManifoldSphereAABB, "Manifold: sphere-AABB");
	RUN_TEST(TestManifoldSphereOBB, "Manifold: sphere-OBB");
	RUN_TEST(TestManifoldSphereCapsule, "Manifold: sphere-capsule");
	RUN_TEST(TestManifoldAABBAABB, "Manifold: AABB-AABB");
	RUN_TEST(TestManifoldOBBOBB, "Manifold: OBB-OBB");
	RUN_TEST(TestClipPolygonToPlane, "Manifold: clip polygon to plane");
	RUN_TEST(TestManifoldCapsuleCapsule, "Manifold: capsule-capsule");
	RUN_TEST(TestManifoldCapsuleCapsuleNoContact, "Manifold: capsule-capsule separated");
	RUN_TEST(TestManifoldConvexConvex, "Manifold: convex-convex (GJK+EPA)");

	fprintf(stderr, "\n=== %d passed, 0 failed ===\n", Passed);
	return 0;
}
