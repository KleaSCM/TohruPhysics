/**
 * Unit tests for SpatialGrid spatial acceleration structure.
 * SpatialGridの単体テストね。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include <TohruPhysics/SpatialGrid.h>
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
//  Tests
// ===========================================================================

// 0191: Init
static void TestInit(void) {
	Vector3 Min = KannaVector3Make(-50, -50, -50);
	Vector3 Max = KannaVector3Make(50, 50, 50);
	SuzuSpatialGrid G;
	SuzuSpatialGridInit(&G, &Min, &Max, 5.0, 512, 4096);

	TEST(G.BucketCount == 512, "bucket count power of 2");
	TEST(G.PoolSize == 4096, "pool size matches");
	TEST(G.GridWidth > 0, "grid width positive");
	TEST(G.FreeHead >= 0, "free list built");
	TEST(G.Stats.PoolCapacity == 4096, "pool capacity tracked");

	SuzuStats S = SuzuSpatialGridGetStats(&G);
	TEST(S.ActiveCells == 0, "no active cells after init");
}

// 0192: Insert + Query
static void TestInsertQuery(void) {
	Vector3 Min = KannaVector3Make(-50, -50, -50);
	Vector3 Max = KannaVector3Make(50, 50, 50);
	SuzuSpatialGrid G;
	SuzuSpatialGridInit(&G, &Min, &Max, 5.0, 512, 4096);

	// Insert a body at the origin
	Vector3 BMin = KannaVector3Make(-2, -2, -2);
	Vector3 BMax = KannaVector3Make(2, 2, 2);
	AABB Box = SabinaAABBMake(&BMin, &BMax);
	SuzuSpatialGridInsert(&G, 0, &Box);

	// Query the same region
	int Bodies[64];
	int Count = SuzuSpatialGridQuery(&G, &Box, Bodies, 64);
	TEST(Count >= 1, "query finds inserted body");

	int Found = 0;
	for (int I = 0; I < Count; I++) {
		if (Bodies[I] == 0) Found = 1;
	}
	TEST(Found, "query returns correct body index");
}

// 0192: Remove
static void TestRemove(void) {
	Vector3 Min = KannaVector3Make(-50, -50, -50);
	Vector3 Max = KannaVector3Make(50, 50, 50);
	SuzuSpatialGrid G;
	SuzuSpatialGridInit(&G, &Min, &Max, 5.0, 512, 4096);

	Vector3 BMin = KannaVector3Make(-2, -2, -2);
	Vector3 BMax = KannaVector3Make(2, 2, 2);
	AABB Box = SabinaAABBMake(&BMin, &BMax);

	SuzuSpatialGridInsert(&G, 0, &Box);

	// Query before remove
	int Bodies[64];
	int CountA = SuzuSpatialGridQuery(&G, &Box, Bodies, 64);
	TEST(CountA >= 1, "body found before remove");

	// Remove
	SuzuSpatialGridRemove(&G, 0, &Box);

	// Query after remove
	int CountB = SuzuSpatialGridQuery(&G, &Box, Bodies, 64);
	int Found = 0;
	for (int I = 0; I < CountB; I++) {
		if (Bodies[I] == 0) Found = 1;
	}
	TEST(!Found, "body not found after remove");
}

// 0192: Update
static void TestUpdate(void) {
	Vector3 Min = KannaVector3Make(-50, -50, -50);
	Vector3 Max = KannaVector3Make(50, 50, 50);
	SuzuSpatialGrid G;
	SuzuSpatialGridInit(&G, &Min, &Max, 5.0, 512, 4096);

	Vector3 OldMin = KannaVector3Make(-2, -2, -2);
	Vector3 OldMax = KannaVector3Make(2, 2, 2);
	AABB OldBox = SabinaAABBMake(&OldMin, &OldMax);
	SuzuSpatialGridInsert(&G, 0, &OldBox);

	// Move far away
	Vector3 NewMin = KannaVector3Make(40, 40, 40);
	Vector3 NewMax = KannaVector3Make(45, 45, 45);
	AABB NewBox = SabinaAABBMake(&NewMin, &NewMax);
	SuzuSpatialGridUpdate(&G, 0, &OldBox, &NewBox);

	// Query old position — should not find body
	int Bodies[64];
	int CountOld = SuzuSpatialGridQuery(&G, &OldBox, Bodies, 64);
	int FoundOld = 0;
	for (int I = 0; I < CountOld; I++) {
		if (Bodies[I] == 0) FoundOld = 1;
	}
	TEST(!FoundOld, "body not found at old position after update");

	// Query new position — should find body
	int CountNew = SuzuSpatialGridQuery(&G, &NewBox, Bodies, 64);
	int FoundNew = 0;
	for (int I = 0; I < CountNew; I++) {
		if (Bodies[I] == 0) FoundNew = 1;
	}
	TEST(FoundNew, "body found at new position after update");
}

// 0191: Hash function produces consistent results
static void TestHashConsistency(void) {
	int H1 = SuzuSpatialGridHash(3, 7, 11, 1023);
	int H2 = SuzuSpatialGridHash(3, 7, 11, 1023);
	TEST(H1 == H2, "hash consistent for same coords");
	TEST(H1 >= 0 && H1 <= 1023, "hash within bucket range");

	// Different coords should (almost always) hash differently
	int H3 = SuzuSpatialGridHash(4, 7, 11, 1023);
	int H4 = SuzuSpatialGridHash(3, 8, 11, 1023);
	TEST(H3 != H1 || H4 != H1, "different coords produce different hash (probabilistic)");
}

// 0194: Multiple bodies in the same cell
static void TestMultipleBodies(void) {
	Vector3 Min = KannaVector3Make(-50, -50, -50);
	Vector3 Max = KannaVector3Make(50, 50, 50);
	SuzuSpatialGrid G;
	SuzuSpatialGridInit(&G, &Min, &Max, 5.0, 64, 4096);

	// Insert 10 bodies all in the same small region
	Vector3 BMin = KannaVector3Make(-1, -1, -1);
	Vector3 BMax = KannaVector3Make(1, 1, 1);
	AABB Box = SabinaAABBMake(&BMin, &BMax);
	for (int I = 0; I < 10; I++) {
		SuzuSpatialGridInsert(&G, I, &Box);
	}

	// Query should find all 10
	int Bodies[128];
	int Count = SuzuSpatialGridQuery(&G, &Box, Bodies, 128);
	TEST(Count >= 10, "query finds all 10 bodies");

	int FoundSum = 0;
	for (int I = 0; I < Count; I++) {
		if (Bodies[I] >= 0 && Bodies[I] < 10) FoundSum++;
	}
	TEST(FoundSum == 10, "all 10 distinct body indices found");
}

// 0193: Pair generation
static void TestPairGeneration(void) {
	Vector3 Min = KannaVector3Make(-50, -50, -50);
	Vector3 Max = KannaVector3Make(50, 50, 50);
	SuzuSpatialGrid G;
	SuzuSpatialGridInit(&G, &Min, &Max, 5.0, 128, 4096);

	AABB Boxes[3];
	for (int I = 0; I < 3; I++) {
		Vector3 BMin = KannaVector3Make(-2, -2, -2);
		Vector3 BMax = KannaVector3Make(2, 2, 2);
		Boxes[I] = SabinaAABBMake(&BMin, &BMax);
		SuzuSpatialGridInsert(&G, I, &Boxes[I]);
	}

	// Verify each body is findable
	int Bodies[64];
	for (int I = 0; I < 3; I++) {
		int N = SuzuSpatialGridQuery(&G, &Boxes[I], Bodies, 64);
		TEST(N >= 3, "each body query finds all 3");
	}

	int OutPairs[32 * 2]; // each pair is 2 ints, max 32 pairs
	int OutCount = 0;
	SuzuSpatialGridGeneratePairs(&G, Boxes, 3, OutPairs, &OutCount, 32);
	TEST(OutCount == 3, "3 bodies generate 3 pairs");

	// Check all pairs are unique and ordered
	for (int I = 0; I < OutCount; I++) {
		int A = OutPairs[I * 2];
		int B = OutPairs[I * 2 + 1];
		TEST(A < B, "pair ordering A < B");
		for (int J = I + 1; J < OutCount; J++) {
			int JA = OutPairs[J * 2];
			int JB = OutPairs[J * 2 + 1];
			TEST(!(A == JA && B == JB), "no duplicate pairs");
		}
	}
}

// 0196: Stats tracking
static void TestStats(void) {
	Vector3 Min = KannaVector3Make(-50, -50, -50);
	Vector3 Max = KannaVector3Make(50, 50, 50);
	SuzuSpatialGrid G;
	SuzuSpatialGridInit(&G, &Min, &Max, 5.0, 128, 4096);

	Vector3 BMin = KannaVector3Make(-2, -2, -2);
	Vector3 BMax = KannaVector3Make(2, 2, 2);
	AABB Box = SabinaAABBMake(&BMin, &BMax);
	SuzuSpatialGridInsert(&G, 0, &Box);
	SuzuSpatialGridInsert(&G, 1, &Box);

	int Bodies[64];
	SuzuSpatialGridQuery(&G, &Box, Bodies, 64);

	SuzuStats S = SuzuSpatialGridGetStats(&G);
	TEST(S.TotalInsertions == 2, "stats: 2 insertions");
	TEST(S.TotalQueries == 1, "stats: 1 query");
	TEST(S.ActiveCells > 0, "stats: active cells > 0");

	// Reset
	SuzuSpatialGridResetStats(&G);
	S = SuzuSpatialGridGetStats(&G);
	TEST(S.TotalInsertions == 0, "stats reset: 0 insertions");
	TEST(S.PoolCapacity == 4096, "stats reset: pool capacity preserved");
}

// 0199: Pool alloc/free cycle
static void TestPoolAllocFree(void) {
	Vector3 Min = KannaVector3Make(-50, -50, -50);
	Vector3 Max = KannaVector3Make(50, 50, 50);
	SuzuSpatialGrid G;
	SuzuSpatialGridInit(&G, &Min, &Max, 5.0, 64, 16);

	// Allocate 10 cells, free them, verify reuse
	int Cells[10];
	for (int I = 0; I < 10; I++) {
		Cells[I] = SuzuSpatialGridAllocCell(&G);
		TEST(Cells[I] >= 0, "cell allocation succeeds");
	}
	TEST(G.FreeHead >= 0, "free list has remaining cells");

	for (int I = 0; I < 10; I++) {
		SuzuSpatialGridFreeCell(&G, Cells[I]);
	}
	TEST(G.FreeHead >= 0, "free list non-empty after free");

	// Should be able to allocate again
	int Reused = SuzuSpatialGridAllocCell(&G);
	TEST(Reused >= 0, "reuse freed cell");
}

// 0192: Clear
static void TestClear(void) {
	Vector3 Min = KannaVector3Make(-50, -50, -50);
	Vector3 Max = KannaVector3Make(50, 50, 50);
	SuzuSpatialGrid G;
	SuzuSpatialGridInit(&G, &Min, &Max, 5.0, 128, 4096);

	Vector3 BMin = KannaVector3Make(-2, -2, -2);
	Vector3 BMax = KannaVector3Make(2, 2, 2);
	AABB Box = SabinaAABBMake(&BMin, &BMax);
	SuzuSpatialGridInsert(&G, 0, &Box);
	SuzuSpatialGridInsert(&G, 1, &Box);

	SuzuSpatialGridClear(&G);

	// Query should find nothing
	int Bodies[64];
	int Count = SuzuSpatialGridQuery(&G, &Box, Bodies, 64);
	int Found = 0;
	for (int I = 0; I < Count; I++) {
		if (Bodies[I] == 0 || Bodies[I] == 1) Found = 1;
	}
	TEST(!Found, "no bodies found after clear");
	TEST(G.Stats.ActiveCells == 0, "no active cells after clear");
}

// 0194: Compute optimal cell size
static void TestComputeOptimalCellSize(void) {
	Vector3 Min = KannaVector3Make(-50, -50, -50);
	Vector3 Max = KannaVector3Make(50, 50, 50);
	SuzuSpatialGrid G;
	SuzuSpatialGridInit(&G, &Min, &Max, 5.0, 128, 4096);

	AABB Boxes[5];
	for (int I = 0; I < 5; I++) {
		Vector3 BMin = KannaVector3Make(-2, -2, -2);
		Vector3 BMax = KannaVector3Make(2, 2, 2);
		Boxes[I] = SabinaAABBMake(&BMin, &BMax);
	}

	Real Cell = SuzuSpatialGridComputeOptimalCellSize(&G, Boxes, 5);
	// Average extent = 4, cell = 4 * 2 = 8
	TEST(Cell > 0, "optimal cell size positive");
	TEST(Cell > 1 && Cell < 100, "optimal cell size in reasonable range");

	// Empty body list should return current cell size
	Real CellEmpty = SuzuSpatialGridComputeOptimalCellSize(&G, Boxes, 0);
	TEST(CellEmpty > 0, "cell size for empty list is positive");
}

// 0195: Lock/Unlock bucket
static void TestBucketLocks(void) {
	Vector3 Min = KannaVector3Make(-50, -50, -50);
	Vector3 Max = KannaVector3Make(50, 50, 50);
	SuzuSpatialGrid G;
	SuzuSpatialGridInit(&G, &Min, &Max, 5.0, 128, 4096);

	// Lock and unlock a valid bucket — should not crash
	int MidBucket = G.BucketCount / 2;
	SuzuSpatialGridLockBucket(&G, MidBucket);
	SuzuSpatialGridUnlockBucket(&G, MidBucket);

	// Out of range — should not crash
	SuzuSpatialGridLockBucket(&G, -1);
	SuzuSpatialGridUnlockBucket(&G, G.BucketCount + 100);

	// Sequential lock + unlock via insert/remove in threaded context:
	// With a single thread, verify lock/unlock round-trips cleanly.
	SuzuSpatialGridLockBucket(&G, MidBucket);
	// Already locked — second lock from same thread would spin (OK)
	SuzuSpatialGridUnlockBucket(&G, MidBucket);
	TEST(1, "bucket lock/unlock round-trip OK");
}

// 0197: Debug cells callback
typedef struct {
	int CallCount;
	int TotalBodies;
} DebugCallData;

static void TestDebugCallback(int CX, int CY, int CZ,
                              int BodyCount, void *UserData) {
	(void)CX; (void)CY; (void)CZ;
	DebugCallData *D = (DebugCallData *)UserData;
	D->CallCount++;
	D->TotalBodies += BodyCount;
}

static void TestDebugCells(void) {
	Vector3 Min = KannaVector3Make(-50, -50, -50);
	Vector3 Max = KannaVector3Make(50, 50, 50);
	SuzuSpatialGrid G;
	SuzuSpatialGridInit(&G, &Min, &Max, 5.0, 128, 4096);

	// Insert a few bodies
	Vector3 BMin = KannaVector3Make(-2, -2, -2);
	Vector3 BMax = KannaVector3Make(2, 2, 2);
	AABB Box = SabinaAABBMake(&BMin, &BMax);
	SuzuSpatialGridInsert(&G, 0, &Box);
	SuzuSpatialGridInsert(&G, 1, &Box);

	DebugCallData D;
	memset(&D, 0, sizeof(D));
	SuzuSpatialGridDebugCells(&G, TestDebugCallback, &D);

	TEST(D.CallCount > 0, "debug callback called at least once");
	TEST(D.TotalBodies >= 2, "debug callback counted all bodies");

	// NULL callback should not crash
	SuzuSpatialGridDebugCells(&G, NULL, NULL);
	TEST(1, "NULL debug callback OK");
}

// 0198: Resize grid
static void TestResize(void) {
	Vector3 Min = KannaVector3Make(-50, -50, -50);
	Vector3 Max = KannaVector3Make(50, 50, 50);
	SuzuSpatialGrid G;
	SuzuSpatialGridInit(&G, &Min, &Max, 5.0, 128, 4096);

	// Insert bodies
	Vector3 BMin = KannaVector3Make(-2, -2, -2);
	Vector3 BMax = KannaVector3Make(2, 2, 2);
	AABB Box = SabinaAABBMake(&BMin, &BMax);
	SuzuSpatialGridInsert(&G, 0, &Box);
	SuzuSpatialGridInsert(&G, 1, &Box);

	// Resize to different cell size
	SuzuSpatialGridResize(&G, 10.0);
	TEST(G.CellSize == 10.0, "cell size updated after resize");
	TEST(G.BucketCount > 0, "bucket count positive after resize");
	TEST(G.GridWidth > 0, "grid width positive after resize");
	TEST(G.Cells != NULL, "cells allocated after resize");

	// Bodies should still be queryable (reinserted at center cell)
	int Bodies[64];
	int Count = SuzuSpatialGridQuery(&G, &Box, Bodies, 64);
	TEST(Count >= 2, "bodies queryable after resize");

	// Resize with zero cell size should be a no-op
	SuzuSpatialGridResize(&G, 0.0);
	TEST(G.CellSize == 10.0, "cell size unchanged after zero resize");
}

// 0198: Resize empty grid (no bodies)
static void TestResizeEmpty(void) {
	Vector3 Min = KannaVector3Make(-50, -50, -50);
	Vector3 Max = KannaVector3Make(50, 50, 50);
	SuzuSpatialGrid G;
	SuzuSpatialGridInit(&G, &Min, &Max, 5.0, 128, 4096);

	SuzuSpatialGridResize(&G, 10.0);
	TEST(G.CellSize == 10.0, "cell size updated (empty resize)");
	TEST(G.BucketCount > 0, "bucket count positive (empty resize)");
	TEST(G.FreeHead >= 0, "free list intact (empty resize)");
}

// ===========================================================================
//  Main
// ===========================================================================

int main(void) {
	fprintf(stderr, "=== TestSpatialGrid ===\n");

	RUN_TEST(TestInit, "SpatialGrid: init");
	RUN_TEST(TestHashConsistency, "SpatialGrid: hash consistency");
	RUN_TEST(TestInsertQuery, "SpatialGrid: insert + query");
	RUN_TEST(TestRemove, "SpatialGrid: remove");
	RUN_TEST(TestUpdate, "SpatialGrid: update");
	RUN_TEST(TestMultipleBodies, "SpatialGrid: multiple bodies");
	RUN_TEST(TestPairGeneration, "SpatialGrid: pair generation");
	RUN_TEST(TestStats, "SpatialGrid: stats");
	RUN_TEST(TestPoolAllocFree, "SpatialGrid: pool alloc/free");
	RUN_TEST(TestClear, "SpatialGrid: clear");
	RUN_TEST(TestComputeOptimalCellSize, "SpatialGrid: compute optimal cell size");
	RUN_TEST(TestBucketLocks, "SpatialGrid: bucket lock/unlock");
	RUN_TEST(TestDebugCells, "SpatialGrid: debug cells callback");
	RUN_TEST(TestResize, "SpatialGrid: resize");
	RUN_TEST(TestResizeEmpty, "SpatialGrid: resize (empty)");

	fprintf(stderr, "\n=== %d passed, 0 failed ===\n", Passed);
	return 0;
}
