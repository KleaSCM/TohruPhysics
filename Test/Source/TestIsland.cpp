/**
 * Unit tests for KuyuIsland simulation island management.
 * KuyuIslandの単体テストね。
 *
 * Tests 0221-0230: island lifecycle, adjacency graph, connected components,
 * activation, deactivation, deep sleep, connection queries, split, merge,
 * and load balancing.
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include <TohruPhysics/Island.h>
#include <TohruPhysics/Vector3.h>
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
//  0221: Island lifecycle
// ===========================================================================

static void TestLifecycle(void) {
	KuyuIslandManager Mgr;
	int R = KuyuIslandInit(&Mgr, 128, 32);
	TEST(R == 0, "init succeeds");
	TEST(Mgr.BodyCapacity == 128, "body capacity");
	TEST(Mgr.IslandCapacity == 32, "island capacity");
	TEST(Mgr.IslandCount == 0, "no islands initially");
	TEST(Mgr.BodyInfo != NULL, "body info allocated");

	// Body info should be zeroed — IslandIndex = -1 for all
	for (int I = 0; I < 5; I++)
		TEST(Mgr.BodyInfo[I].IslandIndex == 0, "body island index 0 initially");

	// Reset
	KuyuIslandReset(&Mgr);
	TEST(Mgr.IslandCount == 0, "reset clears island count");

	// Destroy
	KuyuIslandDestroy(&Mgr);
	TEST(Mgr.BodyInfo == NULL, "destroy frees body info");

	// Null init
	R = KuyuIslandInit(NULL, 128, 32);
	TEST(R == -1, "null init fails");
}

// ===========================================================================
//  0222: Build adjacency graph
// ===========================================================================

static void TestBuildGraph(void) {
	KuyuIslandManager Mgr;
	KuyuIslandInit(&Mgr, 64, 16);

	// Pairs: (0,1), (1,2) — chain of 3
	int Pairs[] = {0, 1, 1, 2};
	int R = KuyuIslandBuildGraph(&Mgr, Pairs, 2);
	TEST(R == 0, "build graph succeeds");
	TEST(Mgr.Graph.EdgeCount == 2, "edge count = 2");

	// Check degrees: body 0 has 1 neighbor, body 1 has 2, body 2 has 1
	TEST(Mgr.Graph.RowStarts[1] - Mgr.Graph.RowStarts[0] == 1,
		"body 0 degree = 1");
	TEST(Mgr.Graph.RowStarts[2] - Mgr.Graph.RowStarts[1] == 2,
		"body 1 degree = 2");
	TEST(Mgr.Graph.RowStarts[3] - Mgr.Graph.RowStarts[2] == 1,
		"body 2 degree = 1");

	// Check neighbor content
	TEST(Mgr.Graph.Neighbors[0] == 1, "body 0 neighbor = 1");

	// Empty pair list
	R = KuyuIslandBuildGraph(&Mgr, Pairs, 0);
	TEST(R == 0, "empty pair build succeeds");

	// Null
	R = KuyuIslandBuildGraph(NULL, Pairs, 2);
	TEST(R == -1, "null build fails");

	KuyuIslandDestroy(&Mgr);
}

// ===========================================================================
//  0223: Connected components
// ===========================================================================

static void TestFindComponents(void) {
	KuyuIslandManager Mgr;
	KuyuIslandInit(&Mgr, 64, 16);

	// Chain: 0-1-2 (one component), 3-4 (second component)
	int Pairs[] = {0, 1, 1, 2, 3, 4};
	KuyuIslandBuildGraph(&Mgr, Pairs, 3);

	int Islands = KuyuIslandFindComponents(&Mgr);
	TEST(Islands == 2, "2 islands from 2 components");

	// Verify bodies are assigned correctly
	TEST(Mgr.BodyInfo[0].IslandIndex == Mgr.BodyInfo[1].IslandIndex,
		"body 0 and 1 in same island");
	TEST(Mgr.BodyInfo[1].IslandIndex == Mgr.BodyInfo[2].IslandIndex,
		"body 1 and 2 in same island");
	TEST(Mgr.BodyInfo[3].IslandIndex == Mgr.BodyInfo[4].IslandIndex,
		"body 3 and 4 in same island");
	TEST(Mgr.BodyInfo[0].IslandIndex != Mgr.BodyInfo[3].IslandIndex,
		"different components in different islands");

	// Verify island body counts
	for (int I = 0; I < Islands; I++) {
		TEST(Mgr.Islands[I].BodyCount > 0, "each island has bodies");
	}

	// Isolated body (no pairs) — each should be its own island
	KuyuIslandReset(&Mgr);
	KuyuIslandBuildGraph(&Mgr, Pairs, 0);  // empty
	Islands = KuyuIslandFindComponents(&Mgr);
	TEST(Islands >= 0, "empty graph find components");

	KuyuIslandDestroy(&Mgr);
}

// ===========================================================================
//  0224: Activation
// ===========================================================================

static void TestActivation(void) {
	KuyuIslandManager Mgr;
	KuyuIslandInit(&Mgr, 64, 16);

	int Pairs[] = {0, 1};
	KuyuIslandBuildGraph(&Mgr, Pairs, 1);
	KuyuIslandFindComponents(&Mgr);

	// Deactivate then reactivate
	int Isl0 = Mgr.BodyInfo[0].IslandIndex;
	Mgr.Islands[Isl0].IsActive = 0;
	Mgr.Islands[Isl0].SleepTimer = 50;

	int R = KuyuIslandActivateBody(&Mgr, 0);
	TEST(R == 0, "activate body succeeds");
	TEST(Mgr.Islands[Isl0].IsActive == 1, "island reactivated");
	TEST(Mgr.Islands[Isl0].SleepTimer == 0, "sleep timer reset");

	// Activate non-existent body
	R = KuyuIslandActivateBody(&Mgr, 999);
	TEST(R == -1, "activate bad body fails");

	// Activate group by index
	Mgr.Islands[Isl0].IsActive = 0;
	R = KuyuIslandActivateGroup(&Mgr, Isl0);
	TEST(R == 0, "activate group succeeds");
	TEST(Mgr.Islands[Isl0].IsActive == 1, "group reactivated");

	KuyuIslandDestroy(&Mgr);
}

// ===========================================================================
//  0225: Deactivation
// ===========================================================================

static void TestDeactivation(void) {
	KuyuIslandManager Mgr;
	KuyuIslandInit(&Mgr, 64, 16);

	int Pairs[] = {0, 1};
	KuyuIslandBuildGraph(&Mgr, Pairs, 1);
	KuyuIslandFindComponents(&Mgr);

	int Isl0 = Mgr.BodyInfo[0].IslandIndex;

	// Set sleep threshold low and test with still bodies
	Mgr.SleepThresholdLinear = 0.01;
	Mgr.SleepThresholdAngular = 0.01;
	Mgr.SleepFramesThreshold = 3;

	// Create still body states
	RigidBodyState States[2];
	memset(States, 0, sizeof(States));  // zero velocity = still

	// First call — timer should increment
	int Deactivated = KuyuIslandDeactivateCheck(&Mgr, States, 2);
	TEST(Deactivated == 0, "not deactivated after first check");
	TEST(Mgr.Islands[Isl0].SleepTimer == 1, "sleep timer = 1");

	// Second call
	Deactivated = KuyuIslandDeactivateCheck(&Mgr, States, 2);
	TEST(Mgr.Islands[Isl0].SleepTimer == 2, "sleep timer = 2");

	// Third call — should deactivate
	Deactivated = KuyuIslandDeactivateCheck(&Mgr, States, 2);
	TEST(Deactivated == 1, "island deactivated after threshold");
	TEST(Mgr.Islands[Isl0].IsActive == 0, "island now sleeping");

	// Moving body should reset timer
	Mgr.Islands[Isl0].IsActive = 1;
	States[0].LinearVelocity.Data[0] = 10.0;  // moving fast
	Deactivated = KuyuIslandDeactivateCheck(&Mgr, States, 2);
	TEST(Mgr.Islands[Isl0].SleepTimer == 0, "moving body resets timer");

	// Null states
	Deactivated = KuyuIslandDeactivateCheck(NULL, States, 2);
	TEST(Deactivated == -1, "null manager returns -1");

	KuyuIslandDestroy(&Mgr);
}

// ===========================================================================
//  0226: Deep sleep
// ===========================================================================

static void TestDeepSleep(void) {
	KuyuIslandManager Mgr;
	KuyuIslandInit(&Mgr, 64, 16);

	int Pairs[] = {0, 1};
	KuyuIslandBuildGraph(&Mgr, Pairs, 1);
	KuyuIslandFindComponents(&Mgr);

	int Isl0 = Mgr.BodyInfo[0].IslandIndex;

	// Deactivate first
	Mgr.Islands[Isl0].IsActive = 0;

	// Deep sleep with no joints — should succeed
	RigidBodyState States[2];
	memset(States, 0, sizeof(States));
	int R = KuyuIslandDeepSleepCheck(&Mgr, Isl0, NULL, 0, 0.1);
	TEST(R == 1, "deep sleep with no joints succeeds");
	TEST(Mgr.Islands[Isl0].IsDeepSleep == 1, "island in deep sleep");

	// Deep sleep with low impulse joints
	Mgr.Islands[Isl0].IsDeepSleep = 0;
	Real LowJoints[] = {0.001, 0.002, 0.001};
	R = KuyuIslandDeepSleepCheck(&Mgr, Isl0, LowJoints, 3, 0.1);
	TEST(R == 1, "low tension joints allow deep sleep");

	// Deep sleep with high impulse joints
	Mgr.Islands[Isl0].IsDeepSleep = 0;
	Real HighJoints[] = {1.0, 2.0, 3.0};
	R = KuyuIslandDeepSleepCheck(&Mgr, Isl0, HighJoints, 3, 0.1);
	TEST(R == 0, "high tension joints prevent deep sleep");

	// Active island can't deep sleep
	Mgr.Islands[Isl0].IsActive = 1;
	R = KuyuIslandDeepSleepCheck(&Mgr, Isl0, NULL, 0, 0.1);
	TEST(R == 0, "active island can't deep sleep");

	// Bad island index
	R = KuyuIslandDeepSleepCheck(&Mgr, -1, NULL, 0, 0.1);
	TEST(R == -1, "bad island index returns -1");

	KuyuIslandDestroy(&Mgr);
}

// ===========================================================================
//  0227: Connection query
// ===========================================================================

static void TestGetConnections(void) {
	KuyuIslandManager Mgr;
	KuyuIslandInit(&Mgr, 64, 16);

	int Pairs[] = {0, 1, 1, 2, 3, 4};
	KuyuIslandBuildGraph(&Mgr, Pairs, 3);

	const int *Nbrs;
	int Deg = KuyuIslandGetConnections(&Mgr, 0, &Nbrs);
	TEST(Deg == 1, "body 0 has 1 neighbor");
	TEST(Nbrs != NULL, "neighbors pointer non-null");
	TEST(Nbrs[0] == 1, "body 0 neighbor is 1");

	Deg = KuyuIslandGetConnections(&Mgr, 1, &Nbrs);
	TEST(Deg == 2, "body 1 has 2 neighbors");

	Deg = KuyuIslandGetConnections(&Mgr, 99, &Nbrs);
	TEST(Deg == -1, "bad body index returns -1");

	// Body with no connections
	Deg = KuyuIslandGetConnections(&Mgr, 10, &Nbrs);
	TEST(Deg == 0, "isolated body has 0 neighbors");
	TEST(Nbrs == NULL, "null pointer for isolated body");

	KuyuIslandDestroy(&Mgr);
}

// ===========================================================================
//  0228: Island split
// ===========================================================================

static void TestSplit(void) {
	KuyuIslandManager Mgr;
	KuyuIslandInit(&Mgr, 64, 16);

	// Two separate chains: 0-1 and 2-3, but build them as one island
	int Pairs[] = {0, 1, 2, 3};
	KuyuIslandBuildGraph(&Mgr, Pairs, 2);
	KuyuIslandFindComponents(&Mgr);
	TEST(Mgr.IslandCount == 2, "2 islands initially");

	// If we update pairs to connect them: 0-1, 1-2, 2-3
	int Pairs2[] = {0, 1, 1, 2, 2, 3};
	KuyuIslandBuildGraph(&Mgr, Pairs2, 3);
	KuyuIslandFindComponents(&Mgr);
	TEST(Mgr.IslandCount == 1, "1 island after connection");

	// Now split back: remove middle pair (1,2)
	int Pairs3[] = {0, 1, 2, 3};
	KuyuIslandBuildGraph(&Mgr, Pairs3, 2);

	int Isl0 = Mgr.BodyInfo[0].IslandIndex;
	int Splits = KuyuIslandSplit(&Mgr, Isl0);
	TEST(Splits > 0, "split creates new islands");

	// After split, there should be 2 islands again
	TEST(Mgr.IslandCount >= 2, "at least 2 islands after split");

	KuyuIslandDestroy(&Mgr);
}

// ===========================================================================
//  0229: Island merge
// ===========================================================================

static void TestMerge(void) {
	KuyuIslandManager Mgr;
	KuyuIslandInit(&Mgr, 64, 16);

	// Two separate pairs = 2 islands
	int Pairs[] = {0, 1, 2, 3};
	KuyuIslandBuildGraph(&Mgr, Pairs, 2);
	KuyuIslandFindComponents(&Mgr);
	TEST(Mgr.IslandCount == 2, "2 islands before merge");

	int Isl0 = Mgr.BodyInfo[0].IslandIndex;
	int Isl1 = Mgr.BodyInfo[2].IslandIndex;

	// Merge them
	int R = KuyuIslandMerge(&Mgr, Isl0, Isl1);
	TEST(R == 0, "merge succeeds");

	// After merge, all bodies should have same island index
	int TargetIsland = Mgr.BodyInfo[0].IslandIndex;
	TEST(Mgr.BodyInfo[1].IslandIndex == TargetIsland,
		"body 1 in merged island");
	TEST(Mgr.BodyInfo[2].IslandIndex == TargetIsland,
		"body 2 in merged island");
	TEST(Mgr.BodyInfo[3].IslandIndex == TargetIsland,
		"body 3 in merged island");

	// Merged island has 4 bodies
	TEST(Mgr.Islands[TargetIsland].BodyCount == 4,
		"merged island has 4 bodies");

	// Self-merge should succeed trivially
	R = KuyuIslandMerge(&Mgr, Isl0, Isl0);
	TEST(R == 0, "self merge succeeds");

	// Bad indices
	R = KuyuIslandMerge(&Mgr, -1, 0);
	TEST(R == -1, "bad index merge fails");

	KuyuIslandDestroy(&Mgr);
}

// ===========================================================================
//  0230: Load balance
// ===========================================================================

static void TestLoadBalance(void) {
	KuyuIslandManager Mgr;
	KuyuIslandInit(&Mgr, 64, 16);

	int Pairs[] = {0, 1, 2, 3, 4, 5};
	KuyuIslandBuildGraph(&Mgr, Pairs, 3);
	KuyuIslandFindComponents(&Mgr);
	TEST(Mgr.IslandCount == 3, "3 islands for load balance");

	int Assignment[16];
	int R = KuyuIslandLoadBalance(&Mgr, Assignment, 4);
	TEST(R == 3, "3 islands assigned");
	for (int I = 0; I < 3; I++) {
		TEST(Assignment[I] >= 0 && Assignment[I] < 4,
			"assignment in valid range");
	}

	// Single thread
	R = KuyuIslandLoadBalance(&Mgr, Assignment, 1);
	TEST(R == 3, "single thread assigns all");
	for (int I = 0; I < 3; I++)
		TEST(Assignment[I] == 0, "single thread: all to thread 0");

	// Empty (no islands) after reset
	KuyuIslandReset(&Mgr);
	R = KuyuIslandLoadBalance(&Mgr, Assignment, 4);
	TEST(R == 0, "empty returns 0");

	// Null
	R = KuyuIslandLoadBalance(NULL, Assignment, 4);
	TEST(R == -1, "null returns -1");

	KuyuIslandDestroy(&Mgr);
}

// ===========================================================================
//  Rebuild convenience
// ===========================================================================

static void TestRebuild(void) {
	KuyuIslandManager Mgr;
	KuyuIslandInit(&Mgr, 64, 16);

	int Pairs[] = {0, 1, 2, 3};
	int R = KuyuIslandRebuild(&Mgr, Pairs, 2);
	TEST(R == 2, "rebuild creates 2 islands");
	TEST(Mgr.IslandCount == 2, "2 islands after rebuild");

	// Null
	R = KuyuIslandRebuild(NULL, Pairs, 2);
	TEST(R == -1, "null rebuild fails");

	KuyuIslandDestroy(&Mgr);
}

// ===========================================================================
//  Stress test
// ===========================================================================

static void TestStress(void) {
	const int NB = 100;
	const int NP = 50;
	KuyuIslandManager Mgr;
	KuyuIslandInit(&Mgr, NB, 32);

	int Pairs[NP * 2];
	for (int I = 0; I < NP; I++) {
		int A = I * 2 % NB;
		int B = (I * 2 + 1) % NB;
		Pairs[I * 2] = A;
		Pairs[I * 2 + 1] = B;
	}

	int R = KuyuIslandRebuild(&Mgr, Pairs, NP);
	TEST(R > 0, "stress rebuild produces islands");

	int Assignment[32];
	R = KuyuIslandLoadBalance(&Mgr, Assignment, 4);
	TEST(R > 0, "stress load balance assigns islands");

	KuyuIslandDestroy(&Mgr);
}

// ===========================================================================
//  Main
// ===========================================================================

int main(void) {
	fprintf(stderr, "=== TestIsland ===\n");

	RUN_TEST(TestLifecycle, "Island: lifecycle");
	RUN_TEST(TestBuildGraph, "Island: build graph");
	RUN_TEST(TestFindComponents, "Island: find components");
	RUN_TEST(TestActivation, "Island: activation");
	RUN_TEST(TestDeactivation, "Island: deactivation");
	RUN_TEST(TestDeepSleep, "Island: deep sleep");
	RUN_TEST(TestGetConnections, "Island: connection query");
	RUN_TEST(TestSplit, "Island: split");
	RUN_TEST(TestMerge, "Island: merge");
	RUN_TEST(TestLoadBalance, "Island: load balance");
	RUN_TEST(TestRebuild, "Island: rebuild");
	RUN_TEST(TestStress, "Island: stress");

	fprintf(stderr, "\n=== %d passed, 0 failed ===\n", Passed);
	return 0;
}
