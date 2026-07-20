/**
 * TohruPhysics terminal demo — full subsystem showcase.
 * TohruPhysicsのターミナルデモ — 全サブシステムのデモンストレーションね。
 *
 * Demonstrates: Arena, Vector3, Matrix, Quaternion, Transform,
 * BodyState, Geometry (primitives + intersections), Math (trig + bench).
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
//  6. Geometry demo
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
}

// ===========================================================================
//  7. BodyState demo
// ===========================================================================

static void DemoBodyState(void) {
	HEADER("7. BodyState — Physics Body Components");

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
//  8. Math demo
// ===========================================================================

static void DemoMath(void) {
	HEADER("8. Math — Trigonometry & Numerical Functions");

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
//  9. Performance snapshot
// ===========================================================================

static void DemoPerformance(void) {
	HEADER("9. Performance — Quick Benchmarks");

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
	DemoBodyState();
	DemoMath();
	DemoPerformance();

	SEP();
	fprintf(stderr, "  ✅  All subsystems operational.\n");
	fprintf(stderr, "  👧  Hanako says: TohruPhysics is ready!\n\n");
	return 0;
}
