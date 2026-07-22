/**
 * Unit tests for BroadPhase collision pair generation.
 * BroadPhaseの単体テストね。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include <TohruPhysics/BroadPhase.h>
#include <TohruPhysics/Geometry.h>
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

#define AE(a, b, eps) NagisaApproxEqual((a), (b), (eps))

static int Passed = 0;

#define RUN_TEST(name, desc) do { \
	fprintf(stderr, "  %-45s ... ", desc); \
	name(); \
	fprintf(stderr, "ok\n"); \
	Passed++; \
} while(0)

// ===========================================================================
//  Helpers
// ===========================================================================

static BroadPhaseBody MakeBody(int BodyId, int BodyType,
                               const Vector3 *Min, const Vector3 *Max,
                               int Group, int Mask) {
	BroadPhaseBody B;
	memset(&B, 0, sizeof(B));
	B.BodyId = BodyId;
	B.BodyType = BodyType;
	B.AABBBox = SabinaAABBMake(Min, Max);
	B.PredictedAABB = B.AABBBox;
	B.CollisionGroup = Group;
	B.CollidesWith = Mask;
	return B;
}

// ===========================================================================
//  Tests
// ===========================================================================

// 0181: Init
static void TestInit(void) {
	BroadPhase BP;
	MiyabiBroadPhaseInit(&BP);
	TEST(BP.BodyCount == 0, "body count zero after init");
	TEST(BP.PairCount == 0, "pair count zero after init");
	TEST(BP.LastFramePairCount == 0, "no persisted pairs after init");
	// BodyMap should all be INVALID_INDEX
	for (int I = 0; I < BROADPHASE_MAX_BODIES; I++) {
		TEST(BP.BodyMap[I] == BROADPHASE_INVALID_INDEX, "body map invalid after init");
	}
}

// 0182: Add and remove bodies
static void TestAddRemove(void) {
	BroadPhase BP;
	MiyabiBroadPhaseInit(&BP);

	Vector3 MinA = KannaVector3Make(-2,-2,-2);
	Vector3 MaxA = KannaVector3Make(2,2,2);
	BroadPhaseBody BA = MakeBody(100, BPBodyType_Dynamic, &MinA, &MaxA,
		COLLISIONGROUP_DYNAMIC, COLLISIONGROUP_ALL);

	int IdxA = MiyabiBroadPhaseAddBody(&BP, &BA);
	TEST(IdxA >= 0, "add body A returns valid index");
	TEST(BP.BodyCount == 1, "body count 1 after add");
	TEST(BP.BodyMap[100] == IdxA, "body map tracks external ID");

	Vector3 MinB = KannaVector3Make(10,10,10);
	Vector3 MaxB = KannaVector3Make(12,12,12);
	BroadPhaseBody BB = MakeBody(200, BPBodyType_Static, &MinB, &MaxB,
		COLLISIONGROUP_STATIC, COLLISIONGROUP_DYNAMIC);
	int IdxB = MiyabiBroadPhaseAddBody(&BP, &BB);
	TEST(IdxB == 1, "add body B at index 1");
	TEST(BP.BodyCount == 2, "body count 2 after second add");

	// Remove body A (swap-remove)
	MiyabiBroadPhaseRemoveBody(&BP, IdxA);
	TEST(BP.BodyCount == 1, "body count 1 after remove");
	// Body B should now be at index 0
	TEST(BP.Bodies[0].BodyId == 200, "last body swapped into removed slot");
	TEST(BP.BodyMap[200] == 0, "body map updated for moved body");
}

// 0183: Motion prediction expands AABB
static void TestMotionPrediction(void) {
	BroadPhase BP;
	MiyabiBroadPhaseInit(&BP);

	Vector3 Min = KannaVector3Make(-1,-1,-1);
	Vector3 Max = KannaVector3Make(1,1,1);
	Vector3 Vel = KannaVector3Make(10, 0, 0);
	BroadPhaseBody B = MakeBody(0, BPBodyType_Dynamic, &Min, &Max,
		COLLISIONGROUP_DYNAMIC, COLLISIONGROUP_ALL);
	B.LinearVelocity = Vel;
	int Idx = MiyabiBroadPhaseAddBody(&BP, &B);

	// Expand with dt=0.5 → predicted AABB should be expanded by 5 in X
	MiyabiBroadPhaseExpandAABB(&BP, Idx, 0.5);
	Vector3 HE = SabinaAABBHalfExtents(&BP.Bodies[Idx].PredictedAABB);
	(void)HE;

	// Predicted AABB should be larger than original
	BroadPhaseBody *Body = &BP.Bodies[Idx];
	TEST(Body->PredictedAABB.Min.Data[0] < Body->AABBBox.Min.Data[0],
		"predicted AABB expands in negative X");
	TEST(Body->PredictedAABB.Max.Data[0] > Body->AABBBox.Max.Data[0],
		"predicted AABB expands in positive X");

	// Static body should not expand
	BroadPhaseBody B2 = MakeBody(1, BPBodyType_Static, &Min, &Max,
		COLLISIONGROUP_STATIC, COLLISIONGROUP_ALL);
	B2.LinearVelocity = Vel;
	int Idx2 = MiyabiBroadPhaseAddBody(&BP, &B2);
	MiyabiBroadPhaseExpandAABB(&BP, Idx2, 0.5);

	Body = &BP.Bodies[Idx2];
	TEST(AE(Body->PredictedAABB.Min.Data[0], -1.0, 0.01),
		"static body AABB unchanged");
	TEST(AE(Body->PredictedAABB.Max.Data[0], 1.0, 0.01),
		"static body AABB unchanged");
	TEST(AE(Body->PredictedAABB.Max.Data[0], 1.0, 0.01),
		"static body AABB unchanged");
}

// 0184: Collision group filtering
static void TestCollisionGroupFiltering(void) {
	BroadPhase BP;
	MiyabiBroadPhaseInit(&BP);

	Vector3 Min = KannaVector3Make(-1,-1,-1);
	Vector3 Max = KannaVector3Make(1,1,1);

	// Body A: group=DYNAMIC, collides with STATIC|DYNAMIC|SENSOR
	BroadPhaseBody BA = MakeBody(0, BPBodyType_Dynamic, &Min, &Max,
		COLLISIONGROUP_DYNAMIC,
		COLLISIONGROUP_STATIC | COLLISIONGROUP_DYNAMIC | COLLISIONGROUP_SENSOR);
	int IdxA = MiyabiBroadPhaseAddBody(&BP, &BA);

	// Body B: group=STATIC, collides with DYNAMIC
	BroadPhaseBody BB = MakeBody(1, BPBodyType_Static, &Min, &Max,
		COLLISIONGROUP_STATIC, COLLISIONGROUP_DYNAMIC);
	int IdxB = MiyabiBroadPhaseAddBody(&BP, &BB);

	// Dynamic and Static should collide (mutual)
	TEST(MiyabiBroadPhaseCanCollide(&BP, IdxA, IdxB) != 0,
		"dynamic-static can collide");

	// Body C: group=SENSOR, collides with DYNAMIC
	BroadPhaseBody BC = MakeBody(2, BPBodyType_Sensor, &Min, &Max,
		COLLISIONGROUP_SENSOR, COLLISIONGROUP_DYNAMIC);
	int IdxC = MiyabiBroadPhaseAddBody(&BP, &BC);

	// Both masks allow the pairing → should collide
	TEST(MiyabiBroadPhaseCanCollide(&BP, IdxA, IdxC) != 0,
		"dynamic-sensor can collide");
}

// 0184: Filter prevents collision when masks don't match
static void TestCollisionGroupFilter(void) {
	BroadPhase BP;
	MiyabiBroadPhaseInit(&BP);

	Vector3 Min = KannaVector3Make(-1,-1,-1);
	Vector3 Max = KannaVector3Make(1,1,1);

	// Body A: group=DYNAMIC, collides with NOTHING
	BroadPhaseBody BA = MakeBody(0, BPBodyType_Dynamic, &Min, &Max,
		COLLISIONGROUP_DYNAMIC, COLLISIONGROUP_NONE);
	int IdxA = MiyabiBroadPhaseAddBody(&BP, &BA);

	// Body B: group=STATIC, collides with DYNAMIC
	BroadPhaseBody BB = MakeBody(1, BPBodyType_Static, &Min, &Max,
		COLLISIONGROUP_STATIC, COLLISIONGROUP_DYNAMIC);
	int IdxB = MiyabiBroadPhaseAddBody(&BP, &BB);

	// A says it collides with nothing → no collision
	TEST(MiyabiBroadPhaseCanCollide(&BP, IdxA, IdxB) == 0,
		"filter prevents collision when A has empty mask");
}

// 0188: Pair persistence
static void TestPairPersistence(void) {
	BroadPhase BP;
	MiyabiBroadPhaseInit(&BP);

	Vector3 MinA = KannaVector3Make(-2,-2,-2);
	Vector3 MaxA = KannaVector3Make(2,2,2);
	BroadPhaseBody BA = MakeBody(0, BPBodyType_Dynamic, &MinA, &MaxA,
		COLLISIONGROUP_DYNAMIC, COLLISIONGROUP_ALL);
	MiyabiBroadPhaseAddBody(&BP, &BA);

	Vector3 MinB = KannaVector3Make(-1,-1,-1);
	Vector3 MaxB = KannaVector3Make(1,1,1);
	BroadPhaseBody BB = MakeBody(1, BPBodyType_Dynamic, &MinB, &MaxB,
		COLLISIONGROUP_DYNAMIC, COLLISIONGROUP_ALL);
	MiyabiBroadPhaseAddBody(&BP, &BB);

	// Evaluate frame 1
	MiyabiBroadPhaseEvaluate(&BP, 0.0);
	TEST(BP.PairCount == 1, "one pair generated for overlapping AABBs");
	TEST(BP.Pairs[0].Persistent == 0, "first frame pair is not persistent");

	// Evaluate frame 2
	MiyabiBroadPhaseEvaluate(&BP, 0.0);
	TEST(BP.PairCount == 1, "one pair still generated");
	TEST(BP.Pairs[0].Persistent == 1, "second frame pair is persistent");
}

// 0190: Stats tracking
static void TestStatsTracking(void) {
	BroadPhase BP;
	MiyabiBroadPhaseInit(&BP);

	// Add 3 overlapping bodies
	for (int I = 0; I < 3; I++) {
		Vector3 Min = KannaVector3Make(-1,-1,-1);
		Vector3 Max = KannaVector3Make(1,1,1);
		BroadPhaseBody B = MakeBody(I, BPBodyType_Dynamic, &Min, &Max,
			COLLISIONGROUP_DYNAMIC, COLLISIONGROUP_ALL);
		MiyabiBroadPhaseAddBody(&BP, &B);
	}

	MiyabiBroadPhaseEvaluate(&BP, 0.0);
	BroadPhaseStats S = MiyabiBroadPhaseGetStats(&BP);
	TEST(S.BodyCount == 3, "stats body count 3");
	TEST(S.PairCount == 3, "stats pair count 3 (C(3,2)=3)");
	TEST(S.AABBTestsPerFrame == 3, "stats AABB tests = 3");

	// Reset and check
	MiyabiBroadPhaseResetStats(&BP);
	S = MiyabiBroadPhaseGetStats(&BP);
	TEST(S.TotalPairsGenerated == 0, "stats reset zero total pairs");
}

// 0189: Validation detects duplicates
static void TestValidation(void) {
	BroadPhase BP;
	MiyabiBroadPhaseInit(&BP);

	Vector3 Min = KannaVector3Make(-1,-1,-1);
	Vector3 Max = KannaVector3Make(1,1,1);
	BroadPhaseBody B = MakeBody(0, BPBodyType_Dynamic, &Min, &Max,
		COLLISIONGROUP_DYNAMIC, COLLISIONGROUP_ALL);
	MiyabiBroadPhaseAddBody(&BP, &B);
	BroadPhaseBody B2 = MakeBody(1, BPBodyType_Dynamic, &Min, &Max,
		COLLISIONGROUP_DYNAMIC, COLLISIONGROUP_ALL);
	MiyabiBroadPhaseAddBody(&BP, &B2);

	MiyabiBroadPhaseEvaluate(&BP, 0.0);
	int Issues = MiyabiBroadPhaseValidate(&BP);
	TEST(Issues == 0, "validation clean for non-static body pair");
}

// 0183: Generate pairs between separated bodies
static void TestNoPairsWhenSeparated(void) {
	BroadPhase BP;
	MiyabiBroadPhaseInit(&BP);

	Vector3 MinA = KannaVector3Make(-2,-2,-2);
	Vector3 MaxA = KannaVector3Make(2,2,2);
	BroadPhaseBody BA = MakeBody(0, BPBodyType_Static, &MinA, &MaxA,
		COLLISIONGROUP_STATIC, COLLISIONGROUP_ALL);
	MiyabiBroadPhaseAddBody(&BP, &BA);

	// Far away
	Vector3 MinB = KannaVector3Make(100,100,100);
	Vector3 MaxB = KannaVector3Make(102,102,102);
	BroadPhaseBody BB = MakeBody(1, BPBodyType_Static, &MinB, &MaxB,
		COLLISIONGROUP_STATIC, COLLISIONGROUP_ALL);
	MiyabiBroadPhaseAddBody(&BP, &BB);

	MiyabiBroadPhaseEvaluate(&BP, 0.0);
	TEST(BP.PairCount == 0, "no pairs when bodies are far apart");
}

// ===========================================================================
//  Main
// ===========================================================================

int main(void) {
	fprintf(stderr, "=== TestBroadPhase ===\n");

	RUN_TEST(TestInit, "BroadPhase: init");
	RUN_TEST(TestAddRemove, "BroadPhase: add/remove bodies");
	RUN_TEST(TestMotionPrediction, "BroadPhase: motion prediction");
	RUN_TEST(TestCollisionGroupFiltering, "BroadPhase: collision group filtering");
	RUN_TEST(TestCollisionGroupFilter, "BroadPhase: collision group filter prevents");
	RUN_TEST(TestPairPersistence, "BroadPhase: pair persistence");
	RUN_TEST(TestStatsTracking, "BroadPhase: stats tracking");
	RUN_TEST(TestValidation, "BroadPhase: validation");
	RUN_TEST(TestNoPairsWhenSeparated, "BroadPhase: no pairs when separated");

	fprintf(stderr, "\n=== %d passed, 0 failed ===\n", Passed);
	return 0;
}
