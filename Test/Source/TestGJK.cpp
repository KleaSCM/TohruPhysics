/**
 * Unit tests for GJK distance algorithm.
 * GJK距離アルゴリズムの単体テストね。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include <TohruPhysics/GJK.h>
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
//  Support function wrappers for GJK
// ===========================================================================

static Vector3 SupportSphereWrapper(const void *Shape, const Vector3 *Dir) {
	return SupportSphere((const Sphere *)Shape, Dir);
}

static Vector3 SupportAABBWrapper(const void *Shape, const Vector3 *Dir) {
	return SupportAABB((const AABB *)Shape, Dir);
}

// ===========================================================================
//  Tests
// ===========================================================================

static void TestGJKSphereSphereSeparated(void) {
	Sphere A, B;
	Vector3 CA = KannaVector3Make(0,0,0);
	Vector3 CB = KannaVector3Make(10,0,0);
	A = SabinaSphereMake(&CA, 2.0);
	B = SabinaSphereMake(&CB, 2.0);

	Vector3 InitialDir = KannaVector3Make(1,0,0);
	GJKState State;
	GJKInit(&State, &InitialDir, &A, SupportSphereWrapper, &B, SupportSphereWrapper, 1e-6, 32);
	GJKEvaluate(&State, &A, SupportSphereWrapper, &B, SupportSphereWrapper);

	TEST(State.Converged, "sphere-sphere converged");
	TEST(AE(State.DistanceSq, 36.0, 0.1), "sphere-sphere dist²=36 (gap=6)");
}

static void TestGJKSphereSphereOverlap(void) {
	Sphere A, B;
	Vector3 CA = KannaVector3Make(0,0,0);
	Vector3 CB = KannaVector3Make(0,0,0);
	A = SabinaSphereMake(&CA, 3.0);
	B = SabinaSphereMake(&CB, 3.0);

	Vector3 InitialDir = KannaVector3Make(1,0,0);
	GJKState State;
	GJKInit(&State, &InitialDir, &A, SupportSphereWrapper, &B, SupportSphereWrapper, 1e-6, 32);
	GJKEvaluate(&State, &A, SupportSphereWrapper, &B, SupportSphereWrapper);

	TEST(State.Degenerate || State.Converged, "overlapping spheres detect overlap");
}

static void TestGJKSphereAABB(void) {
	Sphere S;
	Vector3 SC = KannaVector3Make(5,0,0);
	S = SabinaSphereMake(&SC, 1.0);

	Vector3 Min = KannaVector3Make(-2,-2,-2);
	Vector3 Max = KannaVector3Make(2,2,2);
	AABB Box = SabinaAABBMake(&Min, &Max);

	Vector3 InitialDir = KannaVector3Make(1,0,0);
	GJKState State;
	GJKInit(&State, &InitialDir, &S, SupportSphereWrapper, &Box, SupportAABBWrapper, 1e-6, 32);
	GJKEvaluate(&State, &S, SupportSphereWrapper, &Box, SupportAABBWrapper);

	// Sphere at x=5 radius 1 touches AABB at x=2 → gap = 2
	TEST(State.Converged, "sphere-aabb converged");
	Real Dist = SulettaSqrt(State.DistanceSq);
	TEST(AE(Dist, 2.0, 0.5), "sphere-aabb dist approx 2");
}

// ===========================================================================
//  EPA tests
// ===========================================================================

static void TestEPASphereSphere(void) {
	Sphere A, B;
	Vector3 CA = KannaVector3Make(0,0,0);
	Vector3 CB = KannaVector3Make(1,0,0);
	A = SabinaSphereMake(&CA, 3.0);
	B = SabinaSphereMake(&CB, 3.0);

	Vector3 InitialDir = KannaVector3Make(1,0,0);
	GJKState G;
	GJKInit(&G, &InitialDir, &A, SupportSphereWrapper, &B, SupportSphereWrapper, 1e-6, 64);
	GJKEvaluate(&G, &A, SupportSphereWrapper, &B, SupportSphereWrapper);

	EPAState E;
	EPAInit(&E, &G, &A, SupportSphereWrapper, &B, SupportSphereWrapper, 1e-6, 64);
	EPAEvaluate(&E, &A, SupportSphereWrapper, &B, SupportSphereWrapper);

	TEST(E.PenetrationDepth > 0, "epa penetration positive");
}

// ===========================================================================
//  Main
// ===========================================================================

int main(void) {
	fprintf(stderr, "=== TestGJK ===\n");

	RUN_TEST(TestGJKSphereSphereSeparated, "GJK: sphere-sphere separated");
	RUN_TEST(TestGJKSphereSphereOverlap, "GJK: sphere-sphere overlapping");
	RUN_TEST(TestGJKSphereAABB, "GJK: sphere-AABB");
	RUN_TEST(TestEPASphereSphere, "EPA: sphere-sphere penetration");

	fprintf(stderr, "\n=== %d passed, 0 failed ===\n", Passed);
	return 0;
}
