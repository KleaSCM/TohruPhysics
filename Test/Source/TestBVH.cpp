/**
 * Unit tests for TazusaBVH bounding volume hierarchy.
 * TazusaBVHの単体テストね。
 *
 * Tests SAH build, ray/volume traversal, refit, rebalance,
 * serialisation, leaf splitting, and depth validation.
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include <TohruPhysics/BVH.h>
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

// Create an AABB centered at (X, Y, Z) with given half-extents.
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

// Test buffers
#define TEST_NODE_CAP 4096
#define TEST_PRIM_CAP 1024

static TazusaBVHNode G_Nodes[TEST_NODE_CAP];
static int G_PrimIndices[TEST_PRIM_CAP];
static AABB G_AABBs[TEST_PRIM_CAP];

// ===========================================================================
//  Tests
// ===========================================================================

// 0201: Init
static void TestInit(void) {
	TazusaBVHTree T;
	int R = TazusaBVHInit(&T, G_Nodes, TEST_NODE_CAP,
	                       G_PrimIndices, TEST_PRIM_CAP);
	TEST(R == 0, "init succeeds");
	TEST(T.NodeCapacity == TEST_NODE_CAP, "node capacity");
	TEST(T.PrimCapacity == TEST_PRIM_CAP, "prim capacity");
	TEST(T.NodeCount == 0, "node count zero after init");
	TEST(T.PrimCount == 0, "prim count zero after init");
	TEST(T.MaxLeafSize == TAZUSA_DEFAULT_LEAF_SIZE, "default leaf size");

	// Null pointers
	R = TazusaBVHInit(NULL, G_Nodes, TEST_NODE_CAP, G_PrimIndices, TEST_PRIM_CAP);
	TEST(R == -1, "init with null tree fails");

	// Reset
	TazusaBVHReset(&T);
	TEST(T.NodeCount == 0, "reset clears node count");

	// IsLeaf / IsInternal
	TazusaBVHNode LN;
	memset(&LN, 0, sizeof(LN));
	LN.LeftChild = -1;
	TEST(TazusaBVHIsLeaf(&LN), "leaf when LeftChild < 0");
	TEST(!TazusaBVHIsInternal(&LN), "not internal when leaf");

	LN.LeftChild = 1;
	LN.RightChild = 2;
	TEST(!TazusaBVHIsLeaf(&LN), "not leaf when children exist");
	TEST(TazusaBVHIsInternal(&LN), "internal when children exist");
}

// 0202: Build simple BVH
static void TestBuild(void) {
	TazusaBVHTree T;
	TazusaBVHInit(&T, G_Nodes, TEST_NODE_CAP, G_PrimIndices, TEST_PRIM_CAP);

	// Build with 8 primitives in two clusters
	G_AABBs[0] = MakeBox(-10, 0, 0, 2, 2, 2);
	G_AABBs[1] = MakeBox(-8,  0, 0, 2, 2, 2);
	G_AABBs[2] = MakeBox(8,  0, 0, 2, 2, 2);
	G_AABBs[3] = MakeBox(10, 0, 0, 2, 2, 2);
	G_AABBs[4] = MakeBox(-10, 5, 0, 1, 1, 1);
	G_AABBs[5] = MakeBox(-8,  5, 0, 1, 1, 1);
	G_AABBs[6] = MakeBox(8,  5, 0, 1, 1, 1);
	G_AABBs[7] = MakeBox(10, 5, 0, 1, 1, 1);

	int Nodes = TazusaBVHBuild(&T, G_AABBs, 8, TAZUSA_BUILD_DEFAULT);
	TEST(Nodes > 0, "build produces nodes");
	TEST(T.NodeCount == Nodes, "build returns correct node count");
	TEST(T.PrimCount == 8, "build copies all primitives");
	TEST(T.RootIndex == 0, "root at index 0");
	TEST(T.NodeCount <= T.NodeCapacity, "node count within capacity");

	// Root should be internal (8 > MaxLeafSize)
	TEST(!TazusaBVHIsLeaf(&T.Nodes[T.RootIndex]),
		"root is internal for 8 primitives");

	// All leaves should have PrimCount <= MaxLeafSize
	int Stack[TAZUSA_MAX_DEPTH];
	int SP = 0;
	Stack[SP++] = T.RootIndex;
	while (SP > 0) {
		SP--;
		int NIdx = Stack[SP];
		TazusaBVHNode *N = &T.Nodes[NIdx];
		if (TazusaBVHIsLeaf(N)) {
			TEST(TazusaNodePrimCount(*N) <= T.MaxLeafSize,
				"leaf prim count <= MaxLeafSize");
		} else {
			Stack[SP++] = N->RightChild;
			Stack[SP++] = N->LeftChild;
		}
	}

	// Build with fast quality
	TazusaBVHReset(&T);
	Nodes = TazusaBVHBuild(&T, G_AABBs, 8, TAZUSA_BUILD_FAST);
	TEST(Nodes > 0, "fast build succeeds");

	// Build with high quality
	TazusaBVHReset(&T);
	Nodes = TazusaBVHBuild(&T, G_AABBs, 8, TAZUSA_BUILD_HIGH_QUALITY);
	TEST(Nodes > 0, "high quality build succeeds");
}

// 0202: Build with empty input
static void TestBuildEmpty(void) {
	TazusaBVHTree T;
	TazusaBVHInit(&T, G_Nodes, TEST_NODE_CAP, G_PrimIndices, TEST_PRIM_CAP);
	int Nodes = TazusaBVHBuild(&T, G_AABBs, 0, TAZUSA_BUILD_DEFAULT);
	TEST(Nodes == 0, "build with 0 bodies returns 0");
}

// 0204: Ray traversal against BVH
static int RayHitCounter(int PrimIdx, const Ray *R, Real *OutT, void *UD) {
	(void)R; (void)OutT;
	int *Count = (int *)UD;
	(void)PrimIdx;
	(*Count)++;
	return 1;
}

static void TestRayTraversal(void) {
	TazusaBVHTree T;
	TazusaBVHInit(&T, G_Nodes, TEST_NODE_CAP, G_PrimIndices, TEST_PRIM_CAP);

	G_AABBs[0] = MakeBox(0, 0, 0, 2, 2, 2);

	TazusaBVHBuild(&T, G_AABBs, 1, TAZUSA_BUILD_DEFAULT);

	// Ray through the box
	Vector3 ROrg = KannaVector3Make(-10, 0, 0);
	Vector3 RDir = KannaVector3Make(1, 0, 0);
	Ray R = SabinaRayMake(&ROrg, &RDir);
	int Hits = 0;
	TEST(TazusaBVHTraverseRay(&T, &R, RayHitCounter, &Hits) >= 0,
		"ray traversal returns non-negative");
	TEST(Hits > 0, "ray hits the box");

	// Ray that misses
	Vector3 MOrg = KannaVector3Make(-10, 100, 0);
	Vector3 MDir = KannaVector3Make(1, 0, 0);
	Ray MissR = SabinaRayMake(&MOrg, &MDir);
	Hits = 0;
	TazusaBVHTraverseRay(&T, &MissR, RayHitCounter, &Hits);
	TEST(Hits == 0, "ray misses box");

	// Null callback
	TEST(TazusaBVHTraverseRay(&T, &R, NULL, NULL) == -1,
		"null callback returns -1");

	// Empty tree
	TazusaBVHTree Empty;
	TazusaBVHInit(&Empty, G_Nodes, TEST_NODE_CAP, G_PrimIndices, TEST_PRIM_CAP);
	Hits = 0;
	TEST(TazusaBVHTraverseRay(&Empty, &R, RayHitCounter, &Hits) == 0,
		"empty tree ray traversal returns 0");
}

// 0205: Volume (AABB) traversal
static int VolHitCounter(int PrimIdx, const AABB *Q, void *UD) {
	(void)Q;
	int *Count = (int *)UD;
	(void)PrimIdx;
	(*Count)++;
	return 1;
}

static void TestVolumeTraversal(void) {
	TazusaBVHTree T;
	TazusaBVHInit(&T, G_Nodes, TEST_NODE_CAP, G_PrimIndices, TEST_PRIM_CAP);

	G_AABBs[0] = MakeBox(0, 0, 0, 2, 2, 2);
	G_AABBs[1] = MakeBox(5, 5, 5, 1, 1, 1);

	TazusaBVHBuild(&T, G_AABBs, 2, TAZUSA_BUILD_DEFAULT);

	// Query overlapping box
	AABB Query = MakeBox(0, 0, 0, 3, 3, 3);
	int Hits = 0;
	int Count = TazusaBVHTraverseVolume(&T, &Query, VolHitCounter, &Hits);
	TEST(Count >= 1, "volume traversal returns hits");

	// Query empty space
	AABB MissQ = MakeBox(100, 100, 100, 1, 1, 1);
	Hits = 0;
	Count = TazusaBVHTraverseVolume(&T, &MissQ, VolHitCounter, &Hits);
	TEST(Hits == 0, "volume traversal misses empty space");

	// Null callback
	Count = TazusaBVHTraverseVolume(&T, &Query, NULL, NULL);
	TEST(Count == -1, "null volume callback returns -1");
}

// 0206: Serialisation round-trip
static void TestSerialisation(void) {
	TazusaBVHTree T;
	TazusaBVHInit(&T, G_Nodes, TEST_NODE_CAP, G_PrimIndices, TEST_PRIM_CAP);

	for (int I = 0; I < 4; I++)
		G_AABBs[I] = MakeBox((Real)(I * 5), 0, 0, 2, 2, 2);

	TazusaBVHBuild(&T, G_AABBs, 4, TAZUSA_BUILD_DEFAULT);

	// Query required size
	int ReqSize = TazusaBVHSerialize(&T, NULL, 0);
	TEST(ReqSize > 0, "serialize returns required size");

	// Serialize to buffer
	char *Buf = (char *)malloc((size_t)ReqSize);
	int Written = TazusaBVHSerialize(&T, Buf, ReqSize);
	TEST(Written == ReqSize, "serialize writes correct size");

	// Deserialize into new buffers
	TazusaBVHNode *DstNodes = (TazusaBVHNode *)malloc(
		(size_t)TEST_NODE_CAP * sizeof(TazusaBVHNode));
	int *DstPrims = (int *)malloc(
		(size_t)TEST_PRIM_CAP * sizeof(int));

	TazusaBVHTree Dst;
	int R = TazusaBVHDeserialize(&Dst, Buf, ReqSize,
	                             DstNodes, TEST_NODE_CAP,
	                             DstPrims, TEST_PRIM_CAP);
	TEST(R == 0, "deserialize succeeds");
	TEST(Dst.NodeCount == T.NodeCount, "deserialized node count matches");
	TEST(Dst.PrimCount == T.PrimCount, "deserialized prim count matches");

	// Ray traversal on deserialized tree should still work
	Vector3 DRayOrg = KannaVector3Make(-10, 0, 0);
	Vector3 DRayDir = KannaVector3Make(1, 0, 0);
	Ray DRay = SabinaRayMake(&DRayOrg, &DRayDir);
	int DHits = 0;
	TazusaBVHTraverseRay(&Dst, &DRay, RayHitCounter, &DHits);
	TEST(DHits > 0, "deserialized tree ray hits");

	free(Buf);
	free(DstNodes);
	free(DstPrims);
}

// 0207: Refit
static void TestRefit(void) {
	TazusaBVHTree T;
	TazusaBVHInit(&T, G_Nodes, TEST_NODE_CAP, G_PrimIndices, TEST_PRIM_CAP);

	G_AABBs[0] = MakeBox(0, 0, 0, 2, 2, 2);
	G_AABBs[1] = MakeBox(10, 10, 10, 2, 2, 2);

	TazusaBVHBuild(&T, G_AABBs, 2, TAZUSA_BUILD_DEFAULT);

	// Move primitives
	AABB OldBox0 = G_AABBs[0];
	G_AABBs[0] = MakeBox(0, 0, 0, 5, 5, 5);  // expanded

	int Changed = TazusaBVHRefit(&T, G_AABBs);
	TEST(Changed > 0, "refit detects AABB changes");

	// Refit back to original (tree shrinks -> changes detected)
	G_AABBs[0] = OldBox0;
	Changed = TazusaBVHRefit(&T, G_AABBs);
	TEST(Changed > 0, "refit detects AABB shrinking");

	// Third refit: no changes expected
	Changed = TazusaBVHRefit(&T, G_AABBs);
	TEST(Changed == 0, "refit reports 0 when tree already matches");

	// Empty tree refit
	TazusaBVHTree Empty;
	TazusaBVHInit(&Empty, G_Nodes, TEST_NODE_CAP, G_PrimIndices, TEST_PRIM_CAP);
	Changed = TazusaBVHRefit(&Empty, G_AABBs);
	TEST(Changed == 0, "empty tree refit returns 0");
}

// 0208: Update cycle
static void TestUpdate(void) {
	TazusaBVHTree T;
	TazusaBVHInit(&T, G_Nodes, TEST_NODE_CAP, G_PrimIndices, TEST_PRIM_CAP);

	for (int I = 0; I < 8; I++)
		G_AABBs[I] = MakeBox((Real)(I * 4), 0, 0, 2, 2, 2);

	TazusaBVHBuild(&T, G_AABBs, 8, TAZUSA_BUILD_DEFAULT);

	// Move bodies
	for (int I = 0; I < 8; I++)
		G_AABBs[I] = MakeBox((Real)(I * 4), 10, 10, 2, 2, 2);

	int R = TazusaBVHUpdate(&T, G_AABBs, 8);
	TEST(R == 0, "update succeeds");

	// Tree should still be traversable
	Vector3 UOrg = KannaVector3Make(-10, 10, 10);
	Vector3 UDir = KannaVector3Make(1, 0, 0);
	Ray TestR = SabinaRayMake(&UOrg, &UDir);
	int Hits = 0;
	TazusaBVHTraverseRay(&T, &TestR, RayHitCounter, &Hits);
	TEST(Hits > 0, "updated tree is traversable");
}

// 0209: Depth validation
static void TestDepthValidation(void) {
	TazusaBVHTree T;
	TazusaBVHInit(&T, G_Nodes, TEST_NODE_CAP, G_PrimIndices, TEST_PRIM_CAP);

	for (int I = 0; I < 20; I++)
		G_AABBs[I] = MakeBox((Real)(I * 3), 0, 0, 1, 1, 1);

	TazusaBVHBuild(&T, G_AABBs, 20, TAZUSA_BUILD_DEFAULT);

	int Depth = TazusaBVHValidateDepth(&T, TAZUSA_MAX_DEPTH);
	TEST(Depth >= 0, "depth validation passes");
	TEST(Depth <= TAZUSA_MAX_DEPTH, "depth within limit");

	// Empty tree
	TazusaBVHTree Empty;
	TazusaBVHInit(&Empty, G_Nodes, TEST_NODE_CAP, G_PrimIndices, TEST_PRIM_CAP);
	Depth = TazusaBVHValidateDepth(&Empty, TAZUSA_MAX_DEPTH);
	TEST(Depth == 0, "empty tree depth is 0");
}

// 0210: Leaf splitting
static void TestLeafSplit(void) {
	TazusaBVHTree T;
	TazusaBVHInit(&T, G_Nodes, TEST_NODE_CAP, G_PrimIndices, TEST_PRIM_CAP);

	// Insert many primitives into the same region to force large leaves
	for (int I = 0; I < 50; I++)
		G_AABBs[I] = MakeBox((Real)((I % 5) * 3),
		                     (Real)((I / 5) * 3), 0, 1, 1, 1);

	T.MaxLeafSize = 50;  // all will be in one leaf
	TazusaBVHBuild(&T, G_AABBs, 50, TAZUSA_BUILD_DEFAULT);

	// Verify the leaf has too many primitives
	int HadLargeLeaf = 0;
	for (int I = 0; I < T.NodeCount; I++) {
		if (TazusaBVHIsLeaf(&T.Nodes[I])) {
			if (TazusaNodePrimCount(T.Nodes[I]) > TAZUSA_DEFAULT_LEAF_SIZE)
				HadLargeLeaf = 1;
		}
	}
	TEST(HadLargeLeaf, "leaf with many primitives exists");

	// Now reduce MaxLeafSize and split
	T.MaxLeafSize = TAZUSA_DEFAULT_LEAF_SIZE;
	int Splits = TazusaBVHSplitLeaves(&T, G_AABBs, 50);
	TEST(Splits > 0, "leaf splitting produces splits");

	// After split, no leaf should have > MaxLeafSize (traverse tree from root)
	int CheckStack[TAZUSA_MAX_DEPTH];
	int CheckSP = 0;
	CheckStack[CheckSP++] = T.RootIndex;
	while (CheckSP > 0) {
		CheckSP--;
		int NIdx = CheckStack[CheckSP];
		TazusaBVHNode *N = &T.Nodes[NIdx];
		if (TazusaBVHIsLeaf(N)) {
			TEST(TazusaNodePrimCount(*N) <= T.MaxLeafSize,
				"no overfull leaf after split");
		} else {
			CheckStack[CheckSP++] = N->RightChild;
			CheckStack[CheckSP++] = N->LeftChild;
		}
	}
}

// 0203: Rebalance
static void TestRebalance(void) {
	TazusaBVHTree T;
	TazusaBVHInit(&T, G_Nodes, TEST_NODE_CAP, G_PrimIndices, TEST_PRIM_CAP);

	// Build with known structure
	for (int I = 0; I < 16; I++)
		G_AABBs[I] = MakeBox((Real)(I * 2), 0, 0, 1, 1, 1);

	TazusaBVHBuild(&T, G_AABBs, 16, TAZUSA_BUILD_DEFAULT);

	int Rotations = TazusaBVHRebalance(&T);
	TEST(Rotations >= 0, "rebalance returns non-negative");

	// Tree should still be valid after rebalance
	int Stack[TAZUSA_MAX_DEPTH];
	int SP = 0;
	Stack[SP++] = T.RootIndex;
	while (SP > 0) {
		SP--;
		int NIdx = Stack[SP];
		if (NIdx < 0) continue;
		TazusaBVHNode *N = &T.Nodes[NIdx];
		if (!TazusaBVHIsLeaf(N)) {
			TEST(N->LeftChild >= 0, "internal node has left child");
			TEST(N->RightChild >= 0, "internal node has right child");
			Stack[SP++] = N->RightChild;
			Stack[SP++] = N->LeftChild;
		}
	}
}

// ===========================================================================
//  Main
// ===========================================================================

int main(void) {
	fprintf(stderr, "=== TestBVH ===\n");

	RUN_TEST(TestInit, "BVH: init");
	RUN_TEST(TestBuild, "BVH: build");
	RUN_TEST(TestBuildEmpty, "BVH: build empty");
	RUN_TEST(TestRayTraversal, "BVH: ray traversal");
	RUN_TEST(TestVolumeTraversal, "BVH: volume traversal");
	RUN_TEST(TestSerialisation, "BVH: serialisation");
	RUN_TEST(TestRefit, "BVH: refit");
	RUN_TEST(TestUpdate, "BVH: update");
	RUN_TEST(TestDepthValidation, "BVH: depth validation");
	RUN_TEST(TestLeafSplit, "BVH: leaf splitting");
	RUN_TEST(TestRebalance, "BVH: rebalance");

	fprintf(stderr, "\n=== %d passed, 0 failed ===\n", Passed);
	return 0;
}
