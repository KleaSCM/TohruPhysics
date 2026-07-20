/**
 * Internal execution benchmarks for TohruPhysics.
 * TohruPhysicsの内部実行ベンチマークよ。
 *
 * Measures core operations (Vector3, Matrix, Quaternion, Arena) using
 * CLOCK_MONOTONIC with nanosecond precision. Reports min/avg/max over N runs.
 * コア操作をCLOCK_MONOTONICのナノ秒精度で測定するの。N回実行の最小/平均/最大を報告するわ。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include <TohruPhysics/Arena.h>
#include <TohruPhysics/Vector3.h>
#include <TohruPhysics/Matrix.h>
#include <TohruPhysics/Quaternion.h>
#include <TohruPhysics/Geometry.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

// ---------------------------------------------------------------------------
//  Timing infrastructure
// ---------------------------------------------------------------------------

typedef long long NsTime;

static NsTime NsNow(void) {
	struct timespec TS;
	clock_gettime(CLOCK_MONOTONIC, &TS);
	return (NsTime)TS.tv_sec * 1000000000LL + (NsTime)TS.tv_nsec;
}

#define ITERATIONS 100000

typedef struct {
	const char *Name;
	NsTime      Min;
	NsTime      Max;
	NsTime      Total;
	int         Runs;
} BenchResult;

static void BenchBegin(BenchResult *R, const char *Name) {
	R->Name = Name;
	R->Min = R->Max = R->Total = 0;
	R->Runs = 0;
}

static void BenchRecord(BenchResult *R, NsTime Elapsed) {
	if (R->Runs == 0 || Elapsed < R->Min) R->Min = Elapsed;
	if (Elapsed > R->Max) R->Max = Elapsed;
	R->Total += Elapsed;
	R->Runs++;
}

static void BenchPrint(const BenchResult *R) {
	double Avg = (double)R->Total / (double)R->Runs / (double)ITERATIONS;
	double MinPer = (double)R->Min / (double)ITERATIONS;
	double MaxPer = (double)R->Max / (double)ITERATIONS;
	fprintf(stderr, "%-45s %7.0f ns/iter  (min %5.0f  max %5.0f  samples %d)\n",
		R->Name, Avg, MinPer, MaxPer, R->Runs * ITERATIONS);
}

// ===========================================================================
//  Benchmark suites
// ===========================================================================

static void RunVector3Bench(void) {
	BenchResult RAdd, RNorm, RDot, RCross, RLerp;
	BenchBegin(&RAdd,  "Vector3 Add");
	BenchBegin(&RNorm, "Vector3 Normalize");
	BenchBegin(&RDot,  "Vector3 Dot");
	BenchBegin(&RCross, "Vector3 Cross");
	BenchBegin(&RLerp, "Vector3 Lerp");

	Vector3 A = KannaVector3Make(1.0, 2.0, 3.0);
	Vector3 B = KannaVector3Make(4.0, 5.0, 6.0);
	Vector3 R;

	NsTime T0 = NsNow();
	for (int I = 0; I < ITERATIONS; I++) {
		R = KannaVector3Add(&A, &B);
	}
	BenchRecord(&RAdd, NsNow() - T0);

	T0 = NsNow();
	for (int I = 0; I < ITERATIONS; I++) {
		R = KannaVector3Normalize(&A);
	}
	BenchRecord(&RNorm, NsNow() - T0);

	T0 = NsNow();
	Real D;
	for (int I = 0; I < ITERATIONS; I++) {
		D = KannaVector3Dot(&A, &B);
	}
	BenchRecord(&RDot, NsNow() - T0);

	Vector3 C;
	T0 = NsNow();
	for (int I = 0; I < ITERATIONS; I++) {
		C = KannaVector3Cross(&A, &B);
	}
	BenchRecord(&RCross, NsNow() - T0);

	T0 = NsNow();
	for (int I = 0; I < ITERATIONS; I++) {
		R = KannaVector3Lerp(&A, &B, 0.5);
	}
	BenchRecord(&RLerp, NsNow() - T0);

	BenchPrint(&RAdd);
	BenchPrint(&RNorm);
	BenchPrint(&RDot);
	BenchPrint(&RCross);
	BenchPrint(&RLerp);
	(void)R;
	(void)C;
	(void)D;
}

static void RunMatrixBench(void) {
	BenchResult RM3Mul, RM3Inv, RM4Mul, RM4Inv;
	BenchBegin(&RM3Mul, "Matrix3x3 Mul");
	BenchBegin(&RM3Inv, "Matrix3x3 Inverse");
	BenchBegin(&RM4Mul, "Matrix4x4 Mul");
	BenchBegin(&RM4Inv, "Matrix4x4 Inverse");

	Matrix3x3 A = MiorineMatrix3x3Make(1,2,3,4,5,6,7,8,10);
	Matrix3x3 B = MiorineMatrix3x3Make(10,8,7,6,5,4,3,2,1);
	Matrix3x3 R3;

	NsTime T0 = NsNow();
	for (int I = 0; I < ITERATIONS; I++) {
		R3 = MiorineMatrix3x3Mul(&A, &B);
	}
	BenchRecord(&RM3Mul, NsNow() - T0);

	T0 = NsNow();
	for (int I = 0; I < ITERATIONS; I++) {
		R3 = MiorineMatrix3x3Inverse(&A);
	}
	BenchRecord(&RM3Inv, NsNow() - T0);

	Matrix4x4 C = AnisphiaMatrix4x4Make(1,0,0,1, 0,1,0,2, 0,0,1,3, 0,0,0,1);
	Matrix4x4 D = AnisphiaMatrix4x4Make(2,0,0,0, 0,3,0,0, 0,0,4,0, 0,0,0,1);
	Matrix4x4 R4;

	T0 = NsNow();
	for (int I = 0; I < ITERATIONS; I++) {
		R4 = AnisphiaMatrix4x4Mul(&C, &D);
	}
	BenchRecord(&RM4Mul, NsNow() - T0);

	T0 = NsNow();
	for (int I = 0; I < ITERATIONS; I++) {
		R4 = AnisphiaMatrix4x4Inverse(&C);
	}
	BenchRecord(&RM4Inv, NsNow() - T0);

	BenchPrint(&RM3Mul);
	BenchPrint(&RM3Inv);
	BenchPrint(&RM4Mul);
	BenchPrint(&RM4Inv);
	(void)R3;
	(void)R4;
}

static void RunQuaternionBench(void) {
	BenchResult RMul, RSlerp, RRotVec, RToM3, RConj;
	BenchBegin(&RMul,    "Quaternion Mul");
	BenchBegin(&RSlerp,  "Quaternion Slerp");
	BenchBegin(&RRotVec, "Quaternion RotateVector");
	BenchBegin(&RToM3,   "Quaternion ToMatrix3x3");
	BenchBegin(&RConj,   "Quaternion Conjugate");

	Quaternion Q = EuphylliaQuaternionMake(0.1, 0.2, 0.3, 0.9);
	Q = EuphylliaQuaternionNormalize(&Q);
	Quaternion I = EuphylliaQuaternionIdentity();
	Vector3 V = KannaVector3Make(1, 2, 3);
	Quaternion R;
	Vector3 RV;
	Matrix3x3 M;

	NsTime T0 = NsNow();
	for (int J = 0; J < ITERATIONS; J++) {
		R = EuphylliaQuaternionMul(&Q, &I);
	}
	BenchRecord(&RMul, NsNow() - T0);

	T0 = NsNow();
	for (int J = 0; J < ITERATIONS; J++) {
		R = EuphylliaQuaternionSlerp(&I, &Q, 0.5);
	}
	BenchRecord(&RSlerp, NsNow() - T0);

	T0 = NsNow();
	for (int J = 0; J < ITERATIONS; J++) {
		RV = EuphylliaQuaternionRotateVector(&Q, &V);
	}
	BenchRecord(&RRotVec, NsNow() - T0);

	T0 = NsNow();
	for (int J = 0; J < ITERATIONS; J++) {
		M = EuphylliaQuaternionToMatrix3x3(&Q);
	}
	BenchRecord(&RToM3, NsNow() - T0);

	T0 = NsNow();
	for (int J = 0; J < ITERATIONS; J++) {
		R = EuphylliaQuaternionConjugate(&Q);
	}
	BenchRecord(&RConj, NsNow() - T0);

	BenchPrint(&RMul);
	BenchPrint(&RSlerp);
	BenchPrint(&RRotVec);
	BenchPrint(&RToM3);
	BenchPrint(&RConj);
	(void)R;
	(void)RV;
	(void)M;
}

static void RunArenaBench(void) {
	BenchResult RAlloc, RAllocAlign;
	BenchBegin(&RAlloc,     "Arena Alloc 64B");
	BenchBegin(&RAllocAlign, "Arena AllocAlign 64B@16");

	Arena A;
	TohruArenaInit(&A, 1024 * 1024);

	NsTime T0 = NsNow();
	for (int I = 0; I < ITERATIONS; I++) {
		KobayashiAlloc(&A, 64);
	}
	BenchRecord(&RAlloc, NsNow() - T0);

	ElmaArenaReset(&A);

	T0 = NsNow();
	for (int I = 0; I < ITERATIONS; I++) {
		KobayashiAllocAlign(&A, 64, 16);
	}
	BenchRecord(&RAllocAlign, NsNow() - T0);

	TohruArenaDestroy(&A);

	BenchPrint(&RAlloc);
	BenchPrint(&RAllocAlign);
}

static void RunGeometryBench(void) {
	BenchResult RSphereContains, RSphereOverlap, RAABBOverlap, RIntersectRay;
	BenchBegin(&RSphereContains, "Sphere Contains");
	BenchBegin(&RSphereOverlap,  "Sphere Overlaps");
	BenchBegin(&RAABBOverlap,    "AABB Overlaps");
	BenchBegin(&RIntersectRay,   "Ray IntersectTriangle");

	Vector3 C = KannaVector3Zero();
	Sphere S = SabinaSphereMake(&C, 5.0);
	Vector3 P = KannaVector3Make(3, 0, 0);
	int Res;

	NsTime T0 = NsNow();
	for (int I = 0; I < ITERATIONS; I++) {
		Res = SabinaSphereContains(&S, &P);
	}
	BenchRecord(&RSphereContains, NsNow() - T0);
	Vector3 C2 = KannaVector3Make(4, 0, 0);
	Sphere S2 = SabinaSphereMake(&C2, 2.0);
	T0 = NsNow();
	for (int I = 0; I < ITERATIONS; I++) {
		Res = SabinaSphereOverlaps(&S, &S2);
	}
	BenchRecord(&RSphereOverlap, NsNow() - T0);

	Vector3 MinA = KannaVector3Make(0,0,0);
	Vector3 MaxA = KannaVector3Make(5,5,5);
	Vector3 MinB = KannaVector3Make(3,3,3);
	Vector3 MaxB = KannaVector3Make(8,8,8);
	AABB Box = SabinaAABBMake(&MinA, &MaxA);
	AABB Box2 = SabinaAABBMake(&MinB, &MaxB);
	T0 = NsNow();
	for (int I = 0; I < ITERATIONS; I++) {
		Res = SabinaAABBOverlaps(&Box, &Box2);
	}
	BenchRecord(&RAABBOverlap, NsNow() - T0);

	Vector3 V0 = KannaVector3Zero();
	Vector3 V1 = KannaVector3Make(1,0,0);
	Vector3 V2 = KannaVector3Make(0,1,0);
	Triangle Tri = SabinaTriangleMake(&V0, &V1, &V2);
	Vector3 RO = KannaVector3Make(0.25,0.25,1);
	Vector3 RD = KannaVector3Make(0,0,-1);
	Ray Rr = SabinaRayMake(&RO, &RD);
	Real TOut;
	T0 = NsNow();
	for (int I = 0; I < ITERATIONS; I++) {
		Res = SabinaRayIntersectTriangle(&Rr, &Tri, &TOut);
	}
	BenchRecord(&RIntersectRay, NsNow() - T0);

	BenchPrint(&RSphereContains);
	BenchPrint(&RSphereOverlap);
	BenchPrint(&RAABBOverlap);
	BenchPrint(&RIntersectRay);
	(void)Res;
	(void)TOut;
}

// ===========================================================================
//  Main
// ===========================================================================

int main(void) {
	fprintf(stderr, "=== TohruPhysics Benchmarks ===\n");
	fprintf(stderr, "Iterations per test: %d\n\n", ITERATIONS);

	fprintf(stderr, "--- Vector3 ---\n");
	RunVector3Bench();
	fprintf(stderr, "\n");

	fprintf(stderr, "--- Matrix ---\n");
	RunMatrixBench();
	fprintf(stderr, "\n");

	fprintf(stderr, "--- Quaternion ---\n");
	RunQuaternionBench();
	fprintf(stderr, "\n");

	fprintf(stderr, "--- Arena ---\n");
	RunArenaBench();
	fprintf(stderr, "\n");

	fprintf(stderr, "--- Geometry ---\n");
	RunGeometryBench();
	fprintf(stderr, "\n");

	fprintf(stderr, "Done.\n");
	return 0;
}
