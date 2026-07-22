/**
 * TohruPhysics terminal demo — full subsystem showcase.
 * TohruPhysicsのターミナルデモ — 全サブシステムのデモンストレーションね。
 *
 * Demonstrates: Arena, Vector3, Matrix, Quaternion, Transform,
 * BodyState, Geometry (primitives + intersections + closest point + support),
 * GJK/EPA/SAT, ContactManifold, Array, Math, Error/Log, benchmarks.
 *
 * All output goes to stderr (clean pipe separation).
 * 全ての出力はstderrに出るの（パイプ分離に対応）。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include <TohruPhysics/Arena.h>
#include <TohruPhysics/Vector3.h>
#include <TohruPhysics/Matrix.h>
#include <TohruPhysics/Quaternion.h>
#include <TohruPhysics/Transform.h>
#include <TohruPhysics/BodyState.h>
#include <TohruPhysics/Geometry.h>
#include <TohruPhysics/GJK.h>
#include <TohruPhysics/SAT.h>
#include <TohruPhysics/ContactManifold.h>
#include <TohruPhysics/Array.h>
#include <TohruPhysics/Error.h>
#include <TohruPhysics/Log.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define SEP() fprintf(stderr, \
	"\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n")
#define HEADER(Title) fprintf(stderr, \
	"╔══════════════════════════════════════════════════════╗\n" \
	"║  %-49s║\n" \
	"╚══════════════════════════════════════════════════════╝\n\n", Title)

static long long NsNow(void) {
	struct timespec TS;
	clock_gettime(CLOCK_MONOTONIC, &TS);
	return (long long)TS.tv_sec * 1000000000LL + (long long)TS.tv_nsec;
}

// ===========================================================================
//  1. Arena demo
// ===========================================================================

static void DemoArena(void) {
	HEADER("1. Arena — Page-Aligned Bump Allocator");

	Arena A;
	TohruArenaInit(&A, 1024);
	fprintf(stderr, "  Arena: base=%p  capacity=%zu  offset=%zu\n",
		(void*)A.Base, A.Capacity, A.Offset);

	void *P1 = KobayashiAlloc(&A, 128);
	void *P2 = KobayashiAllocAlign(&A, 64, 32);
	fprintf(stderr, "  Alloc(128)     → %p\n", P1);
	fprintf(stderr, "  AllocAlign(64) → %p  (offset+128=%zu)\n",
		P2, IluluOffset(&A, P2));

	size_t Snap = ElmaArenaSnapshot(&A);
	KobayashiAlloc(&A, 32);
	fprintf(stderr, "  After alloc: used=%zu  remaining=%zu\n",
		ElmaArenaUsed(&A), ElmaArenaRemaining(&A));
	ElmaArenaRollback(&A, Snap);
	fprintf(stderr, "  After rollback: used=%zu  (snapshot=%zu)\n",
		ElmaArenaUsed(&A), Snap);

	void *ZDup = KobayashiDup(&A, "Hello Arena!", 13);
	fprintf(stderr, "  Dup string:    \"%s\"\n", (const char*)ZDup);

	void *Z = KobayashiAlloc(NULL, 64);
	fprintf(stderr, "  NULL-arena alloc → ZeroBlock=%s\n",
		Z == TohruZeroBlock ? "✅" : "❌");

	TohruArenaDestroy(&A);
}

// ===========================================================================
//  2. Vector3 demo
// ===========================================================================

static void DemoVector3(void) {
	HEADER("2. Vector3 — 3D Vector Operations");

	Vector3 A = KannaVector3Make(3.0, 4.0, 0.0);
	Vector3 B = KannaVector3Make(1.0, 0.0, 0.0);
	fprintf(stderr, "  A = (%.1f, %.1f, %.1f)\n", A.Data[0], A.Data[1], A.Data[2]);
	fprintf(stderr, "  B = (%.1f, %.1f, %.1f)\n", B.Data[0], B.Data[1], B.Data[2]);

	Vector3 Add = KannaVector3Add(&A, &B);
	Vector3 Cross = KannaVector3Cross(&A, &B);
	Real Dot = KannaVector3Dot(&A, &B);
	Real LnA = KannaVector3Length(&A);
	Real LnB = KannaVector3Length(&B);
	Real CosAng = NagisaIsZero(LnA) || NagisaIsZero(LnB) ? 0 : YuuClamp(Dot / (LnA * LnB), -1.0, 1.0);
	Real Ang = SulettaAcos(CosAng);

	fprintf(stderr, "  A + B   = (%.1f, %.1f, %.1f)\n", Add.Data[0], Add.Data[1], Add.Data[2]);
	fprintf(stderr, "  A × B   = (%.1f, %.1f, %.1f)\n", Cross.Data[0], Cross.Data[1], Cross.Data[2]);
	fprintf(stderr, "  A · B   = %.1f\n", Dot);
	fprintf(stderr, "  |A|     = %.4f\n", KannaVector3Length(&A));
	Vector3 N = KannaVector3Normalize(&A);
	fprintf(stderr, "  |Â|     = %.6f  (unit length)\n", KannaVector3Length(&N));
	fprintf(stderr, "  ∠(A,B)  = %.2f°\n", KannaRadToDeg(Ang));

	Vector3 R = KannaVector3Reflect(&A, &B);
	Vector3 P = KannaVector3Project(&A, &B);
	fprintf(stderr, "  reflect(A,B) = (%.1f, %.1f, %.1f)\n", R.Data[0], R.Data[1], R.Data[2]);
	fprintf(stderr, "  project(A,B) = (%.1f, %.1f, %.1f)\n", P.Data[0], P.Data[1], P.Data[2]);

	Vector3 Clamped = KannaVector3ClampLength(&A, 2.0);
	fprintf(stderr, "  clamp_len(A,2)= %.4f\n", KannaVector3Length(&Clamped));
}

// ===========================================================================
//  3. Matrix demo
// ===========================================================================

static void DemoMatrix(void) {
	HEADER("3. Matrix — Rotation & Transform");

	Real Angle = KannaDegToRad(45.0);
	Matrix3x3 RX = MiorineMatrix3x3RotationX(Angle);
	Matrix3x3 RY = MiorineMatrix3x3RotationY(Angle);
	Matrix3x3 RZ = MiorineMatrix3x3RotationZ(Angle);
	fprintf(stderr, "  RX(45°)  det=%.2f  trace=%.2f\n",
		MiorineMatrix3x3Determinant(&RX), MiorineMatrix3x3Trace(&RX));
	fprintf(stderr, "  RY(45°)  det=%.2f  trace=%.2f\n",
		MiorineMatrix3x3Determinant(&RY), MiorineMatrix3x3Trace(&RY));
	fprintf(stderr, "  RZ(45°)  det=%.2f  trace=%.2f\n",
		MiorineMatrix3x3Determinant(&RZ), MiorineMatrix3x3Trace(&RZ));

	Matrix3x3 R = MiorineMatrix3x3Mul(&RX, &RY);
	R = MiorineMatrix3x3Mul(&R, &RZ);
	Vector3 V = KannaVector3Make(1.0, 0.0, 0.0);
	Vector3 VR = MiorineMatrix3x3VecMul(&R, &V);
	fprintf(stderr, "  (Rx·Ry·Rz) × (1,0,0) = (%.4f, %.4f, %.4f)\n",
		VR.Data[0], VR.Data[1], VR.Data[2]);
	fprintf(stderr, "  |result| = %.6f  (unit preserved)\n",
		KannaVector3Length(&VR));

	Matrix3x3 InvR = MiorineMatrix3x3Inverse(&R);
	Matrix3x3 I = MiorineMatrix3x3Mul(&R, &InvR);
	fprintf(stderr, "  R·R⁻¹ ≈ I:  diag=(%.1f,%.1f,%.1f)\n",
		I.Data[0], I.Data[4], I.Data[8]);
}

// ===========================================================================
//  4. Quaternion demo
// ===========================================================================

static void DemoQuaternion(void) {
	HEADER("4. Quaternion — Orientation & Interpolation");

	Vector3 Axis = KannaVector3Make(0.0, 1.0, 0.0);
	Quaternion Q = EuphylliaQuaternionFromAxisAngle(&Axis, KannaDegToRad(90.0));
	fprintf(stderr, "  Y-90° q = (%.4f, %.4f, %.4f, %.4f)  |q|=%.6f\n",
		Q.Data[0], Q.Data[1], Q.Data[2], Q.Data[3],
		EuphylliaQuaternionLength(&Q));

	Vector3 V = KannaVector3Make(1.0, 0.0, 0.0);
	Vector3 RV = EuphylliaQuaternionRotateVector(&Q, &V);
	fprintf(stderr, "  q × (1,0,0) = (%.4f, %.4f, %.4f)\n",
		RV.Data[0], RV.Data[1], RV.Data[2]);

	Quaternion Q2 = EuphylliaQuaternionFromAxisAngle(&Axis, KannaDegToRad(180.0));
	Quaternion S = EuphylliaQuaternionSlerp(&Q, &Q2, 0.5);
	Vector3 RS = EuphylliaQuaternionRotateVector(&S, &V);
	fprintf(stderr, "  slerp(90°,180°,0.5) × (1,0,0) = (%.4f, %.4f, %.4f)\n",
		RS.Data[0], RS.Data[1], RS.Data[2]);

	Quaternion L = EuphylliaQuaternionLerp(&Q, &Q2, 0.5);
	Quaternion NL = EuphylliaQuaternionNLerp(&Q, &Q2, 0.5);
	fprintf(stderr, "  lerp  |q|=%.4f  nlerp |q|=%.4f  slerp |q|=%.4f\n",
		EuphylliaQuaternionLength(&L),
		EuphylliaQuaternionLength(&NL),
		EuphylliaQuaternionLength(&S));
}

// ===========================================================================
//  5. Transform demo
// ===========================================================================

static void DemoTransform(void) {
	HEADER("5. Transform — Position & Orientation");

	Transform Id = KaedeTransformIdentity();
	Vector3 P = KannaVector3Make(1.0, 2.0, 3.0);
	Vector3 RP = KaedeTransformPoint(&Id, &P);
	fprintf(stderr, "  Identity: P' = (%.1f, %.1f, %.1f)\n",
		RP.Data[0], RP.Data[1], RP.Data[2]);

	Transform Tfm;
	Tfm.Position = KannaVector3Make(10.0, 0.0, 0.0);
	Vector3 Axis = KannaVector3Make(0.0, 1.0, 0.0);
	Tfm.Rotation = EuphylliaQuaternionFromAxisAngle(&Axis, KannaDegToRad(90.0));
	Vector3 TP = KaedeTransformPoint(&Tfm, &P);
	fprintf(stderr, "  T(10,0,0)+Y90°: P' = (%.1f, %.1f, %.1f)\n",
		TP.Data[0], TP.Data[1], TP.Data[2]);

	Vector3 InvP = KaedeInverseTransformPoint(&Tfm, &TP);
	fprintf(stderr, "  Inverse round-trip: P = (%.1f, %.1f, %.1f)\n",
		InvP.Data[0], InvP.Data[1], InvP.Data[2]);

	Transform Inv = KaedeTransformInverse(&Tfm);
	Transform Combined = KaedeTransformCombine(&Tfm, &Inv);
	fprintf(stderr, "  T·T⁻¹: pos=(%.2f,%.2f,%.2f) rot.w=%.4f\n",
		Combined.Position.Data[0], Combined.Position.Data[1],
		Combined.Position.Data[2], Combined.Rotation.Data[3]);
}

// ===========================================================================
//  6. Geometry demo (primitives, intersections, closest point, support)
// ===========================================================================

static void DemoGeometry(void) {
	HEADER("6. Geometry — Collision Primitives");

	Vector3 Min = KannaVector3Make(-2,-2,-2);
	Vector3 Max = KannaVector3Make(2,2,2);
	AABB Box = SabinaAABBMake(&Min, &Max);
	fprintf(stderr, "  AABB: center=(%.1f,%.1f,%.1f)  SA=%.1f  V=%.1f\n",
		SabinaAABBCenter(&Box).Data[0],
		SabinaAABBCenter(&Box).Data[1],
		SabinaAABBCenter(&Box).Data[2],
		SabinaAABBSurfaceArea(&Box),
		SabinaAABBVolume(&Box));

	Vector3 C = KannaVector3Zero();
	Sphere S = SabinaSphereMake(&C, 3.0);
	fprintf(stderr, "  Sphere: r=%.1f  SA=%.2f  V=%.2f\n",
		S.Radius, SabinaSphereSurfaceArea(&S), SabinaSphereVolume(&S));

	// Intersections demo
	Vector3 S2C = KannaVector3Make(4,0,0);
	Vector3 S3C = KannaVector3Make(10,0,0);
	Sphere S2 = SabinaSphereMake(&S2C, 2.0);
	Sphere S3 = SabinaSphereMake(&S3C, 1.0);
	fprintf(stderr, "  Sphere-Sphere: overlap=%d  separated=%d\n",
		IntersectSphereSphere(&S, &S2),
		IntersectSphereSphere(&S, &S3));

	Quaternion Id = EuphylliaQuaternionIdentity();
	Vector3 HE = KannaVector3Make(1,2,3);
	OBB O = SabinaOBBMake(&C, &HE, &Id);
	Vector3 Corners[8];
	SabinaOBBGetCorners(&O, Corners);
	fprintf(stderr, "  OBB corners: (%.1f,%.1f,%.1f) .. (%.1f,%.1f,%.1f)\n",
		Corners[0].Data[0], Corners[0].Data[1], Corners[0].Data[2],
		Corners[7].Data[0], Corners[7].Data[1], Corners[7].Data[2]);

	// Intersections with pre-built objects (avoid rvalue temps)
	Vector3 AABBMin2 = KannaVector3Make(1,1,1);
	Vector3 AABBMax2 = KannaVector3Make(3,3,3);
	AABB Box2 = SabinaAABBMake(&AABBMin2, &AABBMax2);

	Vector3 OBB2C = KannaVector3Make(0,0,0);
	Vector3 OBB2HE = KannaVector3Make(1,1,1);
	OBB O2 = SabinaOBBMake(&OBB2C, &OBB2HE, &Id);

	fprintf(stderr, "  AABB-AABB: %d   AABB-OBB: %d   OBB-OBB: %d\n",
		IntersectAABBAABB(&Box, &Box2),
		IntersectAABBOBB(&Box, &O),
		IntersectOBBOBB(&O, &O2));

	// Ray intersections
	Vector3 RO = KannaVector3Make(-5, 0, 0);
	Vector3 RD = KannaVector3Make(1, 0, 0);
	Ray R = SabinaRayMake(&RO, &RD);
	Real RTMin, RTMax;
	int Hit = IntersectRayAABB(&R, &Box, &RTMin, &RTMax);
	fprintf(stderr, "  Ray-AABB: hit=%d  t=(%.2f,%.2f)\n", Hit, RTMin, RTMax);

	// Ray-Triangle intersection (build triangle from OBB corners)
	Triangle Tri;
	Tri.V0 = Corners[0];
	Tri.V1 = Corners[1];
	Tri.V2 = Corners[2];
	Real RTT;
	int RTH = IntersectRayTriangle(&R, &Tri, &RTT);
	fprintf(stderr, "  Ray-Tri: hit=%d  t=%.2f\n", RTH, RTT);

	// Segment intersections
	Vector3 SSt = KannaVector3Make(-3, 0, 0);
	Vector3 SEn = KannaVector3Make(3, 0, 0);
	Segment Seg2 = SabinaSegmentMake(&SSt, &SEn);
	Real SHT0, SHT1;
	int SHit = IntersectSegmentSphere(&Seg2, &S, &SHT0, &SHT1);
	fprintf(stderr, "  Segment-Sphere: hit=%d  t=(%.2f,%.2f)\n", SHit, SHT0, SHT1);

	// Closest point queries
	Vector3 QP = KannaVector3Make(5, 1, 1);
	Vector3 CPA = SabinaAABBClosestPoint(&Box, &QP);
	fprintf(stderr, "  ClosestPoint-AABB: (%.1f,%.1f,%.1f)\n",
		CPA.Data[0], CPA.Data[1], CPA.Data[2]);

	Vector3 CPO = SabinaOBBClosestPoint(&O, &QP);
	fprintf(stderr, "  ClosestPoint-OBB:  (%.1f,%.1f,%.1f)\n",
		CPO.Data[0], CPO.Data[1], CPO.Data[2]);

	// Support functions
	Vector3 SD = KannaVector3Make(1, 0, 0);
	Vector3 SPT = SupportAABB(&Box, &SD);
	fprintf(stderr, "  Support AABB along (1,0,0): (%.1f,%.1f,%.1f)\n",
		SPT.Data[0], SPT.Data[1], SPT.Data[2]);
	SPT = SupportSphere(&S, &SD);
	fprintf(stderr, "  Support Sphere along (1,0,0): (%.1f,%.1f,%.1f)\n",
		SPT.Data[0], SPT.Data[1], SPT.Data[2]);

	// Distance queries
	Real D = DistanceSphereSphere(&S, &S2);
	fprintf(stderr, "  Distance Sphere-Sphere: %.2f\n", D);
}

// ===========================================================================
//  7. GJK & SAT demo
// ===========================================================================

static Vector3 SupportSphereWrap(const void *Shape, const Vector3 *Dir) {
	return SupportSphere((const Sphere *)Shape, Dir);
}

static void DemoGJKSAT(void) {
	HEADER("7. GJK & SAT — Distance & Separating Axis");

	// GJK: separated spheres
	Vector3 CA = KannaVector3Make(0,0,0);
	Vector3 CB = KannaVector3Make(8,0,0);
	Sphere SA = SabinaSphereMake(&CA, 2.0);
	Sphere SB = SabinaSphereMake(&CB, 2.0);
	Vector3 InitDir = KannaVector3Make(1,0,0);
	GJKState G;
	GJKInit(&G, &InitDir, &SA, SupportSphereWrap, &SB, SupportSphereWrap, 1e-6, 32);
	GJKEvaluate(&G, &SA, SupportSphereWrap, &SB, SupportSphereWrap);
	Real Dist = SulettaSqrt(G.DistanceSq);
	fprintf(stderr, "  GJK: sphere dist=%.1f  (converged=%d)\n",
		Dist, G.Converged);

	// GJK: overlapping spheres → EPA (non-collinear centers to avoid degenerate GJK simplex)
	Vector3 CCO = KannaVector3Make(2.9, 0.1, 0.0);
	Sphere SC = SabinaSphereMake(&CCO, 2.0);
	GJKState G2;
	GJKInit(&G2, &InitDir, &SA, SupportSphereWrap, &SC, SupportSphereWrap, 1e-6, 32);
	GJKEvaluate(&G2, &SA, SupportSphereWrap, &SC, SupportSphereWrap);
	fprintf(stderr, "  GJK: overlapping: degenerate=%d simplexCount=%d distSq=%.6f\n",
		G2.Degenerate, G2.SimplexCount, G2.DistanceSq);

	EPAState E;
	EPAInit(&E, &G2, &SA, SupportSphereWrap, &SC, SupportSphereWrap, 1e-3, 64);
	EPAEvaluate(&E, &SA, SupportSphereWrap, &SC, SupportSphereWrap);
	fprintf(stderr, "  EPA: pen=%.3f  normal=(%.4f,%.4f,%.4f)  converged=%d\n",
		E.PenetrationDepth,
		E.ContactNormal.Data[0], E.ContactNormal.Data[1],
		E.ContactNormal.Data[2], E.Converged);
	if (E.VertexCount > 0) {
		fprintf(stderr, "  EPA: vertices=%d  faces=%d\n",
			E.VertexCount, E.FaceCount);
	}

	// SAT: OBB-OBB
	Quaternion Id = EuphylliaQuaternionIdentity();
	Vector3 HE1 = KannaVector3Make(2,1,1);
	Vector3 C1 = KannaVector3Zero();
	OBB O1 = SabinaOBBMake(&C1, &HE1, &Id);
	Vector3 HE2 = KannaVector3Make(2,1,1);
	Vector3 C2 = KannaVector3Make(2,0,0);
	OBB O2 = SabinaOBBMake(&C2, &HE2, &Id);
	SATResult SR = SintoOBBOBB(&O1, &O2);
	fprintf(stderr, "  SAT: OBB-OBB intersect=%d  pen=%.2f  norm=(%.2f,%.2f,%.2f)\n",
		SR.Intersect, SR.PenetrationDepth,
		SR.Normal.Data[0], SR.Normal.Data[1], SR.Normal.Data[2]);
}

// ===========================================================================
//  8. Contact Manifold demo
// ===========================================================================

static void DemoContactManifold(void) {
	HEADER("8. Contact Manifold — Clipping & Contact Points");

	// OBB-OBB: clipping manifold
	Quaternion Id = EuphylliaQuaternionIdentity();
	Vector3 HE = KannaVector3Make(2,2,2);
	Vector3 C = KannaVector3Zero();
	OBB BoxA = SabinaOBBMake(&C, &HE, &Id);
	Vector3 C2 = KannaVector3Make(2.5,0,0);
	OBB BoxB = SabinaOBBMake(&C2, &HE, &Id);
	Transform Tx = KaedeTransformIdentity();

	ContactManifold M;
	ManifoldOBBOBB(&BoxA, &Tx, &BoxB, &Tx, &M);
	fprintf(stderr, "  OBB-OBB manifold: %d points, pen=%.2f, norm=(%.2f,%.2f,%.2f)\n",
		M.PointCount, M.Penetration,
		M.Normal.Data[0], M.Normal.Data[1], M.Normal.Data[2]);
	for (int I = 0; I < M.PointCount && I < 4; I++) {
		fprintf(stderr, "    pt[%d] pos=(%.2f,%.2f,%.2f) pen=%.2f\n",
			I,
			M.Points[I].Position.Data[0],
			M.Points[I].Position.Data[1],
			M.Points[I].Position.Data[2],
			M.Points[I].Penetration);
	}

	// Sphere-Sphere manifold
	Sphere SphA = { {0,0,0}, 3.0f };
	Sphere SphB = { {4,0,0}, 3.0f };
	ContactManifold MS;
	ManifoldSphereSphere(&SphA, &Tx, &SphB, &Tx, &MS);
	fprintf(stderr, "  Sphere-Sphere manifold: %d pts pen=%.2f norm=(%.2f,%.2f,%.2f)\n",
		MS.PointCount, MS.Penetration,
		MS.Normal.Data[0], MS.Normal.Data[1], MS.Normal.Data[2]);

	// Capsule-Capsule manifold (overlapping: segments offset by 1 in Z, radii=1.5 each)
	Vector3 StA = KannaVector3Make(0,0,0);
	Vector3 EnA = KannaVector3Make(0,4,0);
	Vector3 StB = KannaVector3Make(0,0,0.5);
	Vector3 EnB = KannaVector3Make(0,4,0.5);
	Capsule CapA = SabinaCapsuleMake(&StA, &EnA, 1.5);
	Capsule CapB = SabinaCapsuleMake(&StB, &EnB, 1.5);
	ContactManifold MC;
	ManifoldCapsuleCapsule(&CapA, &Tx, &CapB, &Tx, &MC);
	fprintf(stderr, "  Capsule-Capsule manifold: %d pts pen=%.2f norm=(%.2f,%.2f,%.2f)\n",
		MC.PointCount, MC.Penetration,
		MC.Normal.Data[0], MC.Normal.Data[1], MC.Normal.Data[2]);

	// AABB-AABB manifold
	Vector3 MinA = KannaVector3Make(-3,-3,-3);
	Vector3 MaxA = KannaVector3Make(3,3,3);
	Vector3 MinB = KannaVector3Make(1,1,1);
	Vector3 MaxB = KannaVector3Make(7,7,7);
	AABB BoxAabb = SabinaAABBMake(&MinA, &MaxA);
	AABB BoxBbb = SabinaAABBMake(&MinB, &MaxB);
	ContactManifold MAB;
	ManifoldAABBAABB(&BoxAabb, &Tx, &BoxBbb, &Tx, &MAB);
	fprintf(stderr, "  AABB-AABB manifold: %d pts pen=%.2f\n",
		MAB.PointCount, MAB.Penetration);
}

static int CmpIntAsc(const void *A, const void *B) {
	int IA = *(const int*)A, IB = *(const int*)B;
	return (IA > IB) - (IA < IB);
}

// ===========================================================================
//  9. Array demo
// ===========================================================================

static void DemoArray(void) {
	HEADER("9. Array — Arena-Backed Dynamic Array");

	Arena Arena;
	TohruArenaInit(&Arena, 4096);

	Array Arr;
	TiltyArrayInit(&Arena, &Arr, sizeof(int), 4);
	fprintf(stderr, "  Init: cap=%zu  len=%zu\n", Arr.Capacity, Arr.Length);

	int Vals[] = {30, 10, 50, 20, 40};
	for (int I = 0; I < 5; I++) {
		TiltyArrayPush(&Arr, &Vals[I]);
	}
	fprintf(stderr, "  Push 5: len=%zu  cap=%zu  data[0]=%d  data[4]=%d\n",
		Arr.Length, Arr.Capacity,
		*(int*)TiltyArrayGet(&Arr, 0),
		*(int*)TiltyArrayGet(&Arr, 4));

	// Insert + remove
	int V = 15;
	TiltyArrayInsert(&Arr, 1, &V);
	fprintf(stderr, "  Insert 15 @1: len=%zu  data[1]=%d\n",
		Arr.Length, *(int*)TiltyArrayGet(&Arr, 1));

	TiltyArrayRemove(&Arr, 1);
	fprintf(stderr, "  Remove @1: len=%zu  data[1]=%d\n",
		Arr.Length, *(int*)TiltyArrayGet(&Arr, 1));

	// Sort
	TiltyArraySort(&Arr, CmpIntAsc);
	fprintf(stderr, "  Sort: ");
	for (size_t I = 0; I < Arr.Length; I++) {
		fprintf(stderr, "%s%d", I > 0 ? " " : "", *(int*)TiltyArrayGet(&Arr, I));
	}
	fprintf(stderr, "\n");

	// Pop + Front + Back
	int *F = (int*)TiltyArrayFront(&Arr);
	int *B = (int*)TiltyArrayBack(&Arr);
	int *P = (int*)TiltyArrayPop(&Arr);
	fprintf(stderr, "  Front=%d  Back=%d  Pop=%d  len=%zu\n",
		*F, *B, *P, Arr.Length);

	TohruArenaDestroy(&Arena);
}

// ===========================================================================
//  10. BodyState demo
// ===========================================================================

static void DemoBodyState(void) {
	HEADER("10. BodyState — Physics Body Components");

	Vector3 Pos = KannaVector3Make(0, 10, 0);
	Quaternion Rot = EuphylliaQuaternionIdentity();
	Vector3 HE = KannaVector3Make(1, 1, 1);
	BodyConfig C = YuiBodyConfigMakeDynamic(&Pos, &Rot, 10.0, &HE);
	fprintf(stderr, "  Dynamic body: mass=%.1f  invMass=%.4f\n",
		C.Mass.Mass, C.Mass.InverseMass);
	fprintf(stderr, "  Position: (%.1f,%.1f,%.1f)  Type=%d\n",
		C.State.Position.Data[0], C.State.Position.Data[1],
		C.State.Position.Data[2], (int)C.Kinematic.Type);

	// Inertia
	Vector3 BoxHE = KannaVector3Make(1,2,3);
	Matrix3x3 InvI = YuiInertiaComputeBox(10.0, &BoxHE);
	fprintf(stderr, "  Box inertia: Ixx⁻¹=%.6f  Iyy⁻¹=%.6f  Izz⁻¹=%.6f\n",
		InvI.Data[0], InvI.Data[4], InvI.Data[8]);

	InvI = YuiInertiaComputeSphere(10.0, 2.0);
	fprintf(stderr, "  Sphere(r=2) inertia⁻¹: %.6f (all axes)\n", InvI.Data[0]);

	// Gravity + damping
	GravityField Grav = YuiGravityFieldDefault();
	fprintf(stderr, "  Gravity: (%.2f,%.2f,%.2f) m/s²\n",
		Grav.Acceleration.Data[0], Grav.Acceleration.Data[1],
		Grav.Acceleration.Data[2]);

	RigidBodyState State = C.State;
	KinematicConfig Kin = C.Kinematic;
	YuiApplyGravity(&Grav, &Kin, &State, 1.0);
	fprintf(stderr, "  After 1s gravity: vy=%.2f m/s\n", State.LinearVelocity.Data[1]);

	DampingConfig Damp = YuiDampingConfigMake(0.5, 0.3);
	YuiDampingApply(&Damp, &State, 1.0);
	fprintf(stderr, "  After 1s damping: vy=%.2f m/s\n", State.LinearVelocity.Data[1]);

	// Sleep
	SleepConfig Sleep = YuiSleepConfigMake(0.1, 0.1, 2.0);
	State.LinearVelocity = KannaVector3Zero();
	int ShouldSleep = YuiSleepUpdate(&Sleep, &State, 2.5);
	fprintf(stderr, "  Sleep check: timer=%.1f  shouldSleep=%d\n",
		Sleep.Timer, ShouldSleep);
}

// ===========================================================================
//  11. Math demo
// ===========================================================================

static void DemoMath(void) {
	HEADER("11. Math — Trigonometry & Numerical Functions");

	fprintf(stderr, "  sin(0)=%.6f  sin(π/2)=%.6f  sin(π)=%.6f\n",
		SulettaSin(0), SulettaSin(REAL_PI_HALF), SulettaSin(REAL_PI));
	fprintf(stderr, "  cos(0)=%.6f  cos(π/2)=%.6f  cos(π)=%.6f\n",
		SulettaCos(0), SulettaCos(REAL_PI_HALF), SulettaCos(REAL_PI));
	fprintf(stderr, "  sqrt(2)=%.6f  invsqrt(2)=%.6f  acos(0.5)=%.4f°\n",
		SulettaSqrt(2.0), SulettaInvSqrt(2.0), KannaRadToDeg(SulettaAcos(0.5)));
	fprintf(stderr, "  atan2(1,1)=%.4f°  atan2(0,-1)=%.4f°\n",
		KannaRadToDeg(SulettaAtan2(1,1)), KannaRadToDeg(SulettaAtan2(0,-1)));
	fprintf(stderr, "  floor(3.7)=%.1f  ceil(3.2)=%.1f  round(3.5)=%.1f\n",
		SulettaFloor(3.7), SulettaCeil(3.2), SulettaRound(3.5));

	fprintf(stderr, "  lerp(0,100,0.25)=%.1f\n", KannaLerp(0, 100, 0.25));
	fprintf(stderr, "  deg2rad(180)=%.4f  rad2deg(%.4f)=%.1f\n",
		KannaDegToRad(180), REAL_PI, KannaRadToDeg(REAL_PI));
	fprintf(stderr, "  smoothstep(0,1,0.25)=%.6f\n",
		KannaSmoothstep(0, 1, 0.25));
}

// ===========================================================================
//  12. Performance snapshot
// ===========================================================================

static void DemoPerformance(void) {
	HEADER("12. Performance — Quick Benchmarks");

	long long T0, T1;
	int I;

	T0 = NsNow();
	Vector3 VA = KannaVector3Make(1,2,3);
	Vector3 VB = KannaVector3Make(4,5,6);
	Vector3 VR;
	for (I = 0; I < 100000; I++) {
		VR = KannaVector3Add(&VA, &VB);
	}
	T1 = NsNow();
	fprintf(stderr, "  Vector3 Add ×100k:  %lld ns  (%.1f ns/op)\n",
		T1 - T0, (double)(T1 - T0) / 100000.0);
	(void)VR;

	T0 = NsNow();
	Real RD;
	for (I = 0; I < 100000; I++) {
		RD = KannaVector3Dot(&VA, &VB);
	}
	T1 = NsNow();
	fprintf(stderr, "  Vector3 Dot ×100k:  %lld ns  (%.1f ns/op)\n",
		T1 - T0, (double)(T1 - T0) / 100000.0);
	(void)RD;

	T0 = NsNow();
	Quaternion QA = EuphylliaQuaternionMake(0.1, 0.2, 0.3, 0.9);
	QA = EuphylliaQuaternionNormalize(&QA);
	Quaternion QB = EuphylliaQuaternionIdentity();
	Quaternion QR;
	for (I = 0; I < 100000; I++) {
		QR = EuphylliaQuaternionMul(&QA, &QB);
	}
	T1 = NsNow();
	fprintf(stderr, "  Quaternion Mul ×100k: %lld ns  (%.1f ns/op)\n",
		T1 - T0, (double)(T1 - T0) / 100000.0);
	(void)QR;

	T0 = NsNow();
	Matrix3x3 MA = MiorineMatrix3x3Make(1,2,3,4,5,6,7,8,10);
	Matrix3x3 MB = MiorineMatrix3x3Make(10,8,7,6,5,4,3,2,1);
	Matrix3x3 MR;
	for (I = 0; I < 100000; I++) {
		MR = MiorineMatrix3x3Mul(&MA, &MB);
	}
	T1 = NsNow();
	fprintf(stderr, "  Matrix3x3 Mul ×100k: %lld ns  (%.1f ns/op)\n",
		T1 - T0, (double)(T1 - T0) / 100000.0);
	(void)MR;

	Arena Arena;
	TohruArenaInit(&Arena, 1024*1024);
	T0 = NsNow();
	for (I = 0; I < 100000; I++) {
		KobayashiAlloc(&Arena, 64);
	}
	T1 = NsNow();
	fprintf(stderr, "  Arena Alloc ×100k:   %lld ns  (%.1f ns/op)\n",
		T1 - T0, (double)(T1 - T0) / 100000.0);
	TohruArenaDestroy(&Arena);
}

// ===========================================================================
//  13. Error & Log demo
// ===========================================================================

static void DemoErrorLog(void) {
	HEADER("13. Error & Log — Startup Safety & Custom Logging");

	// Error type: zero-init is Ok
	Error E0 = {};
	fprintf(stderr, "  Error{}: code=%d  isOk=%d  isFail=%d\n",
		E0.Code, ErrIsOk(E0), ErrIsFail(E0));

	// ErrorCodeToString
	Error E1 = ErrMake(ErrorCode_OutOfMemory);
	fprintf(stderr, "  ErrMake(OOM): code=%d  str=\"%s\"\n",
		E1.Code, ErrorCodeToString(E1.Code));

	// Result<int>
	Result<int> Res = {42, ErrOk()};
	fprintf(stderr, "  Result<int>(42): value=%d  ok=%d\n",
		Res.Value, Res.Ok());

	// Log: redirect to stderr
	TohruLogSetLevel(LogLevel_Debug);
	TohruLogSetOutputFD(2);
	TohruLogInfo("Demo logging: info message — level=%d", LogLevel_Info);
	TohruLogDebug("Demo logging: debug message — level=%d", LogLevel_Debug);
	TohruLogWarn("Demo logging: warning — code=%d", ErrorCode_InvalidParameter);
	fprintf(stderr, "  Log: 3 messages written to stderr\n");
}

// ===========================================================================
//  Main
// ===========================================================================

int main(void) {
	fprintf(stderr,
		"\n"
		"╔══════════════════════════════════════════════════════╗\n"
		"║         TohruPhysics Engine — Terminal Demo         ║\n"
		"║    トールフィジックスエンジン — ターミナルデモ    ║\n"
		"╚══════════════════════════════════════════════════════╝\n"
		"\n");

	DemoArena();
	DemoVector3();
	DemoMatrix();
	DemoQuaternion();
	DemoTransform();
	DemoGeometry();
	DemoGJKSAT();
	DemoContactManifold();
	DemoArray();
	DemoBodyState();
	DemoMath();
	DemoErrorLog();
	DemoPerformance();

	SEP();
	fprintf(stderr, "  ✅  All subsystems operational.\n");
	fprintf(stderr, "  👧  Hanako says: TohruPhysics is ready!\n\n");
	return 0;
}
