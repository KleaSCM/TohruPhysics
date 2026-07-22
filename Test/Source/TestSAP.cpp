/**
 * Unit tests for IliaSAP sweep-and-prune broad phase.
 * IliaSAPの単体テストね。
 *
 * Tests all 10 SAP functions: axis list, insert, remove, sort,
 * pair generation, multi-axis test, pair marker, dimension fallback,
 * batch verification, and boundary limits.
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include <TohruPhysics/SAP.h>
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

static int Passed = 0;

#define RUN_TEST(name, desc) do { \
	fprintf(stderr, "  %-45s ... ", desc); \
	name(); \
	fprintf(stderr, "ok\n"); \
	Passed++; \
} while(0)

static AABB MakeBox(Real X, Real Y, Real Z, Real HX, Real HY, Real HZ) {
	AABB B;
	B.Min.Data[0] = X - HX;
	B.Min.Data[1] = Y - HY;
	B.Min.Data[2] = Z - HZ;
	B.Max.Data[0] = X + HX;
	B.Max.Data[1] = Y + HY;
	B.Max.Data[2] = Z + HZ;
	return B;
}

// ===========================================================================
//  0211: Axis list init/destroy/reset
// ===========================================================================

static void TestAxisList(void) {
	IliaSapAxisList L;
	int R = IliaSapAxisListInit(&L, 32);
	TEST(R == 0, "axis list init");
	TEST(L.BodyCapacity == 32, "axis list body capacity");
	TEST(L.Capacity >= 64, "axis list endpoint capacity (2 * body capacity)");
	TEST(L.Count == 0, "axis list initially empty");
	TEST(L.BodyCount == 0, "axis list body count zero");
	TEST(L.Dirty == 0, "axis list not dirty initially");

	// BodyToIndex should be -1 for all
	for (int I = 0; I < 5; I++)
		TEST(L.BodyToIndex[I] == -1, "body index init to -1");

	IliaSapAxisListReset(&L);
	TEST(L.Count == 0, "reset preserves buffer, clears count");

	IliaSapAxisListDestroy(&L);
	TEST(L.Endpoints == NULL, "destroy frees endpoints");
	TEST(L.BodyToIndex == NULL, "destroy frees body map");

	// Null/zero init
	R = IliaSapAxisListInit(NULL, 32);
	TEST(R == -1, "null init fails");
	R = IliaSapAxisListInit(&L, 0);
	TEST(R == -1, "zero capacity init fails");
	IliaSapAxisListDestroy(&L);
}

// ===========================================================================
//  0212: Insert
// ===========================================================================

static void TestInsert(void) {
	IliaSapManager Mgr;
	IliaSapInit(&Mgr, 64);

	AABB Box0 = MakeBox(0, 0, 0, 2, 2, 2);
	int R = IliaSapInsertBody(&Mgr, 0, &Box0);
	TEST(R == 0, "insert body 0");
	TEST(Mgr.Axes[0].BodyCount == 1, "axis 0 body count = 1 after insert");
	TEST(Mgr.Axes[0].Count == 2, "axis 0 endpoint count = 2 after insert");

	// Insert second body
	AABB Box1 = MakeBox(10, 10, 10, 1, 1, 1);
	R = IliaSapInsertBody(&Mgr, 1, &Box1);
	TEST(R == 0, "insert body 1");
	TEST(Mgr.Axes[0].BodyCount == 2, "body count = 2");

	// Duplicate insert should fail
	R = IliaSapInsertBody(&Mgr, 0, &Box0);
	TEST(R == -1, "duplicate insert fails");

	// Insert with bad index
	R = IliaSapInsertBody(&Mgr, -1, &Box0);
	TEST(R == -1, "negative index insert fails");

	// Null manager
	R = IliaSapInsertBody(NULL, 0, &Box0);
	TEST(R == -1, "null manager insert fails");

	IliaSapDestroy(&Mgr);
}

// ===========================================================================
//  0213: Remove
// ===========================================================================

static void TestRemove(void) {
	IliaSapManager Mgr;
	IliaSapInit(&Mgr, 64);

	AABB Box0 = MakeBox(0, 0, 0, 2, 2, 2);
	AABB Box1 = MakeBox(10, 10, 10, 1, 1, 1);
	AABB Box2 = MakeBox(20, 20, 20, 3, 3, 3);

	IliaSapInsertBody(&Mgr, 0, &Box0);
	IliaSapInsertBody(&Mgr, 1, &Box1);
	IliaSapInsertBody(&Mgr, 2, &Box2);
	TEST(Mgr.Axes[0].BodyCount == 3, "three bodies inserted");

	// Remove middle body
	int R = IliaSapRemoveBody(&Mgr, 1);
	TEST(R == 0, "remove body 1");
	TEST(Mgr.Axes[0].BodyCount == 2, "body count = 2 after remove");

	// Remove non-existent body
	R = IliaSapRemoveBody(&Mgr, 99);
	TEST(R == -1, "remove non-existent body fails");

	// Remove remaining bodies
	IliaSapRemoveBody(&Mgr, 0);
	IliaSapRemoveBody(&Mgr, 2);
	TEST(Mgr.Axes[0].BodyCount == 0, "all bodies removed");

	// Null manager
	R = IliaSapRemoveBody(NULL, 0);
	TEST(R == -1, "null manager remove fails");

	IliaSapDestroy(&Mgr);
}

// ===========================================================================
//  0214: Incremental sort
// ===========================================================================

static void TestSort(void) {
	IliaSapManager Mgr;
	IliaSapInit(&Mgr, 64);

	// Insert bodies in non-sorted order spatially
	AABB Box0 = MakeBox(100, 0, 0, 1, 1, 1);
	AABB Box1 = MakeBox(0, 0, 0, 1, 1, 1);
	AABB Box2 = MakeBox(50, 0, 0, 1, 1, 1);

	IliaSapInsertBody(&Mgr, 0, &Box0);
	IliaSapInsertBody(&Mgr, 1, &Box1);
	IliaSapInsertBody(&Mgr, 2, &Box2);

	// After insertion, endpoints should be bubble-sorted
	// Check X-axis endpoints are in order
	int Swaps = IliaSapUpdateSort(&Mgr);
	TEST(Swaps >= 0, "sort returns non-negative swaps");

	// Verify sorted order on X axis
	IliaSapAxisList *XL = &Mgr.Axes[0];
	for (int I = 1; I < XL->Count; I++) {
		TEST(XL->Endpoints[I].Value >= XL->Endpoints[I - 1].Value,
			"X axis endpoints sorted");
	}

	// After sorting, running the sort again should have 0 swaps
	int Swaps2 = IliaSapUpdateSort(&Mgr);
	TEST(Swaps2 >= 0, "second sort runs");

	IliaSapDestroy(&Mgr);
}

// ===========================================================================
//  0215: Pair generation
// ===========================================================================

static void TestPairGeneration(void) {
	IliaSapManager Mgr;
	IliaSapInit(&Mgr, 64);

	// Two overlapping boxes
	AABB Box0 = MakeBox(0, 0, 0, 5, 5, 5);
	AABB Box1 = MakeBox(3, 3, 3, 5, 5, 5);  // overlaps with 0

	IliaSapInsertBody(&Mgr, 0, &Box0);
	IliaSapInsertBody(&Mgr, 1, &Box1);
	IliaSapUpdateSort(&Mgr);

	int Pairs = IliaSapGeneratePairs(&Mgr);
	TEST(Pairs >= 1, "overlapping boxes generate at least 1 pair");

	// Clear and test separated boxes
	IliaSapReset(&Mgr);

	AABB Box2 = MakeBox(0, 0, 0, 1, 1, 1);
	AABB Box3 = MakeBox(100, 100, 100, 1, 1, 1);  // far away

	IliaSapInsertBody(&Mgr, 0, &Box2);
	IliaSapInsertBody(&Mgr, 1, &Box3);
	IliaSapUpdateSort(&Mgr);

	Pairs = IliaSapGeneratePairs(&Mgr);
	TEST(Pairs == 0, "separated boxes generate 0 pairs");

	// Test with 3 overlapping boxes (should produce 3 pairs)
	IliaSapReset(&Mgr);

	AABB Box4 = MakeBox(0, 0, 0, 3, 3, 3);
	AABB Box5 = MakeBox(2, 2, 2, 3, 3, 3);
	AABB Box6 = MakeBox(-2, -2, -2, 3, 3, 3);

	IliaSapInsertBody(&Mgr, 0, &Box4);
	IliaSapInsertBody(&Mgr, 1, &Box5);
	IliaSapInsertBody(&Mgr, 2, &Box6);
	IliaSapUpdateSort(&Mgr);

	Pairs = IliaSapGeneratePairs(&Mgr);
	TEST(Pairs == 3, "3 overlapping boxes produce exactly 3 pairs");

	// Null manager
	Pairs = IliaSapGeneratePairs(NULL);
	TEST(Pairs == -1, "null manager generate pairs fails");

	IliaSapDestroy(&Mgr);
}

// ===========================================================================
//  0216: Multi-axis overlap test
// ===========================================================================

static void TestMultiAxisOverlap(void) {
	IliaSapManager Mgr;
	IliaSapInit(&Mgr, 64);

	AABB Box0 = MakeBox(0, 0, 0, 2, 2, 2);
	AABB Box1 = MakeBox(1, 1, 1, 2, 2, 2);  // overlaps on all axes
	AABB Box2 = MakeBox(100, 1, 1, 2, 2, 2); // X separated, Y/Z overlap

	IliaSapInsertBody(&Mgr, 0, &Box0);
	IliaSapInsertBody(&Mgr, 1, &Box1);
	IliaSapInsertBody(&Mgr, 2, &Box2);
	IliaSapUpdateSort(&Mgr);

	TEST(IliaSapTestOverlap(&Mgr, 0, 1) == 1, "overlapping bodies on all axes");
	TEST(IliaSapTestOverlap(&Mgr, 0, 2) == 0, "separated on X axis");
	TEST(IliaSapTestOverlap(&Mgr, 1, 2) == 0, "separated on X axis (body 1 & 2)");

	// Self test
	TEST(IliaSapTestOverlap(&Mgr, 0, 0) == 1, "self overlap is true");

	IliaSapDestroy(&Mgr);
}

// ===========================================================================
//  0217: Pair marker
// ===========================================================================

static void TestPairMarker(void) {
	IliaSapPairMarker Mkr;
	int R = IliaSapPairMarkerInit(&Mkr, 64);
	TEST(R == 0, "pair marker init");

	// First test — pair unseen
	R = IliaSapPairMarkerTest(&Mkr, 0, 1);
	TEST(R == 0, "first test of pair returns 0 (new)");

	// Second test — should be seen
	R = IliaSapPairMarkerTest(&Mkr, 0, 1);
	TEST(R == 1, "second test returns 1 (seen)");

	// Canonical order: (1, 0) should hit the same entry
	R = IliaSapPairMarkerTest(&Mkr, 1, 0);
	TEST(R == 1, "reversed order also returns 1");

	// New frame
	IliaSapPairMarkerNextFrame(&Mkr);
	R = IliaSapPairMarkerTest(&Mkr, 0, 1);
	TEST(R == 0, "after next frame, pair is new again");

	// Destroy
	IliaSapPairMarkerDestroy(&Mkr);
	TEST(Mkr.Data == NULL, "marker destroy clears data");

	// Init with zero capacity
	R = IliaSapPairMarkerInit(&Mkr, 0);
	TEST(R == -1, "zero capacity init fails");
}

// ===========================================================================
//  0218: Dimension fallback
// ===========================================================================

static void TestDimensionTest(void) {
	IliaSapManager Mgr;
	IliaSapInit(&Mgr, 64);

	// Two boxes overlapping on X only
	AABB Box0 = MakeBox(0, 0, 0, 2, 0.5, 0.5);
	AABB Box1 = MakeBox(1, 100, 100, 2, 0.5, 0.5); // X overlaps, Y/Z separate

	IliaSapInsertBody(&Mgr, 0, &Box0);
	IliaSapInsertBody(&Mgr, 1, &Box1);
	IliaSapUpdateSort(&Mgr);

	// X axis should overlap
	TEST(IliaSapDimensionTest(&Mgr.Axes[0], 0, 1) == 1,
		"dimension test X overlaps");
	// Y axis should not
	TEST(IliaSapDimensionTest(&Mgr.Axes[1], 0, 1) == 0,
		"dimension test Y separated");

	// Two boxes with identical intervals on X (axis-aligned)
	IliaSapReset(&Mgr);

	AABB Box2 = MakeBox(0, 0, 0, 2, 1, 1);
	AABB Box3 = MakeBox(0, 5, 5, 2, 1, 1); // identical X intervals, different Y/Z

	IliaSapInsertBody(&Mgr, 0, &Box2);
	IliaSapInsertBody(&Mgr, 1, &Box3);
	IliaSapUpdateSort(&Mgr);

	// Identical X intervals should report overlap (fallback)
	TEST(IliaSapDimensionTest(&Mgr.Axes[0], 0, 1) == 1,
		"dimension fallback: identical intervals overlap");

	// But multi-axis test should still see they don't overlap
	TEST(IliaSapTestOverlap(&Mgr, 0, 1) == 0,
		"multi-axis: separated on Y/Z despite X fallback");

	IliaSapDestroy(&Mgr);
}

// ===========================================================================
//  0219: Batch pair verification
// ===========================================================================

static void TestVerifyPairs(void) {
	IliaSapManager Mgr;
	IliaSapInit(&Mgr, 64);

	// Create a scenario with 3 bodies
	AABB Boxes[3];
	Boxes[0] = MakeBox(0, 0, 0, 2, 2, 2);
	Boxes[1] = MakeBox(3, 3, 3, 2, 2, 2);  // overlaps with 0
	Boxes[2] = MakeBox(100, 100, 100, 1, 1, 1);  // separate

	for (int I = 0; I < 3; I++)
		IliaSapInsertBody(&Mgr, I, &Boxes[I]);
	IliaSapUpdateSort(&Mgr);

	// Generate pairs
	int Pairs = IliaSapGeneratePairs(&Mgr);
	TEST(Pairs >= 1, "pairs generated before verification");

	// Verify — should keep only actual AABB overlaps
	int Verified = IliaSapVerifyPairs(&Mgr, Boxes);
	TEST(Verified >= 1, "some pairs survive verification");
	TEST(Mgr.OverlapCount == Verified, "overlap count matches verified");

	// Move body 2 to overlap with body 0 (remove and re-insert)
	IliaSapRemoveBody(&Mgr, 2);
	Boxes[2] = MakeBox(1, 1, 1, 2, 2, 2);
	IliaSapInsertBody(&Mgr, 2, &Boxes[2]);
	IliaSapUpdateSort(&Mgr);

	Pairs = IliaSapGeneratePairs(&Mgr);
	Verified = IliaSapVerifyPairs(&Mgr, Boxes);
	TEST(Verified > 1, "more pairs after moving body");

	// Null boxes
	Verified = IliaSapVerifyPairs(&Mgr, NULL);
	TEST(Verified == 0, "null boxes returns 0");

	IliaSapDestroy(&Mgr);
}

// ===========================================================================
//  0220: Boundary limits
// ===========================================================================

static void TestBoundaryCheck(void) {
	IliaSapManager Mgr;
	IliaSapInit(&Mgr, 64);

	// Insert a normal box
	AABB Box0 = MakeBox(0, 0, 0, 2, 2, 2);
	IliaSapInsertBody(&Mgr, 0, &Box0);

	int Clamped = IliaSapBoundaryCheck(&Mgr);
	TEST(Clamped == 0, "no clamping for normal values");

	// Now directly inject inf/nan into endpoints
	Mgr.Axes[0].Endpoints[0].Value = 1e20;  // extreme but ≤ limit
	Mgr.Axes[0].Endpoints[1].Value = -1e15; // extreme but ok

	Clamped = IliaSapBoundaryCheck(&Mgr);
	// 1e20 is > 1e12, -1e15 is < -1e12
	TEST(Clamped == 2, "extreme values clamped: 2 endpoints");

	// Verify clamping worked
	TEST(Mgr.Axes[0].Endpoints[0].Value == 1e12, "max clamped to 1e12");
	TEST(Mgr.Axes[0].Endpoints[1].Value == -1e12, "min clamped to -1e12");

	IliaSapDestroy(&Mgr);
}

// ===========================================================================
//  Manager lifecycle
// ===========================================================================

static void TestManagerLifecycle(void) {
	IliaSapManager Mgr;
	int R = IliaSapInit(&Mgr, ILIA_DEFAULT_BODY_CAPACITY);
	TEST(R == 0, "manager init with default capacity");
	TEST(Mgr.BodyCapacity == ILIA_DEFAULT_BODY_CAPACITY, "default body cap");

	// Reset
	IliaSapReset(&Mgr);
	for (int A = 0; A < 3; A++)
		TEST(Mgr.Axes[A].BodyCount == 0, "reset clears all axes");

	// Destroy
	IliaSapDestroy(&Mgr);
	for (int A = 0; A < 3; A++)
		TEST(Mgr.Axes[A].Endpoints == NULL, "destroy clears endpoints");

	// Null init
	R = IliaSapInit(NULL, 64);
	TEST(R == -1, "null manager init fails");
}

// ===========================================================================
//  Stress: large number of bodies
// ===========================================================================

static void TestStress(void) {
	const int NB = 200;
	IliaSapManager Mgr;
	IliaSapInit(&Mgr, NB);

	// Insert NB bodies in a grid (some overlap, some don't)
	for (int I = 0; I < NB; I++) {
		Real X = (Real)((I % 20) * 3);
		Real Y = (Real)((I / 20) * 3);
		AABB Box = MakeBox(X, Y, 0, 1.5, 1.5, 1.5);
		IliaSapInsertBody(&Mgr, I, &Box);
	}

	int Swaps = IliaSapUpdateSort(&Mgr);
	TEST(Swaps >= 0, "stress sort completes");

	int Pairs = IliaSapGeneratePairs(&Mgr);
	TEST(Pairs >= 0, "stress pair generation completes");
	TEST(Mgr.Marker.Generation > 0, "marker generation advanced");

	IliaSapDestroy(&Mgr);
}

// ===========================================================================
//  Motion: update, re-sort, re-generate
// ===========================================================================

static void TestMotion(void) {
	IliaSapManager Mgr;
	IliaSapInit(&Mgr, 64);

	// Start with overlapping boxes
	AABB Box0 = MakeBox(0, 0, 0, 5, 5, 5);
	AABB Box1 = MakeBox(2, 2, 2, 5, 5, 5);
	IliaSapInsertBody(&Mgr, 0, &Box0);
	IliaSapInsertBody(&Mgr, 1, &Box1);
	IliaSapUpdateSort(&Mgr);

	int Pairs = IliaSapGeneratePairs(&Mgr);
	TEST(Pairs == 1, "initial overlap produces 1 pair");

	// Move body 1 away — can't change endpoints directly in SAP,
	// so we remove and re-insert (simulating a new frame)
	IliaSapRemoveBody(&Mgr, 1);
	AABB Box1Moved = MakeBox(100, 100, 100, 5, 5, 5);
	IliaSapInsertBody(&Mgr, 1, &Box1Moved);
	IliaSapUpdateSort(&Mgr);

	Pairs = IliaSapGeneratePairs(&Mgr);
	TEST(Pairs == 0, "after separation, no pairs");

	IliaSapDestroy(&Mgr);
}

// ===========================================================================
//  Main
// ===========================================================================

int main(void) {
	fprintf(stderr, "=== TestSAP ===\n");

	RUN_TEST(TestAxisList, "SAP: axis list");
	RUN_TEST(TestInsert, "SAP: insert");
	RUN_TEST(TestRemove, "SAP: remove");
	RUN_TEST(TestSort, "SAP: incremental sort");
	RUN_TEST(TestPairGeneration, "SAP: pair generation");
	RUN_TEST(TestMultiAxisOverlap, "SAP: multi-axis overlap");
	RUN_TEST(TestPairMarker, "SAP: pair marker");
	RUN_TEST(TestDimensionTest, "SAP: dimension fallback");
	RUN_TEST(TestVerifyPairs, "SAP: batch verification");
	RUN_TEST(TestBoundaryCheck, "SAP: boundary check");
	RUN_TEST(TestManagerLifecycle, "SAP: manager lifecycle");
	RUN_TEST(TestStress, "SAP: stress");
	RUN_TEST(TestMotion, "SAP: motion");

	fprintf(stderr, "\n=== %d passed, 0 failed ===\n", Passed);
	return 0;
}
