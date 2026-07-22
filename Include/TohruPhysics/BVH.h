/**
 * TazusaBVH — bounding volume hierarchy for broad-phase acceleration.
 * TohruPhysics用のBVH — BroadPhase加速のためね。
 *
 * Stores scene geometry in a binary tree of AABBs. Each leaf references a
 * contiguous range of body indices. The SAH (Surface Area Heuristic) top-down
 * builder produces high-quality trees that minimise expected intersection cost.
 * Incremental refitting supports dynamic scenes where bodies move between
 * frames without full rebuild.
 *
 * DESIGN PHILOSOPHY:
 * A BVH partitions space hierarchically rather than uniformly (cf. SpatialGrid).
 * This gives better culling efficiency for non-uniform distributions at the
 * cost of O(log N) traversal vs O(1) grid lookup. The SAH builder computes
 * split cost as C_split = C_trav + p_L * N_L * C_test + p_R * N_R * C_test
 * where p = surface area ratio. We use a binning approximation (32 bins) to
 * avoid O(N²) sweep cost while staying within 5% of optimal.
 *
 * DATA LAYOUT:
 * ┌──────────────┬──────────────────────────────────────────────┐
 * │ Nodes[]      │ Flat array of TazusaBVHNode, root at [0]    │
 * │ PrimIndices[]│ Body indices in BVH-optimal traversal order  │
 * └──────────────┴──────────────────────────────────────────────┘
 *
 * TazusaBVHNode layout (32 bytes, cache-line friendly):
 * ┌──────────┬──────────┬──────────┬──────────┬──────────┬──────┐
 * │ AABB.Min │ AABB.Max │ Left     │ Right    │ Parent   │ Prim │
 * │ (24B)    │ (24B)    │ Child(4B)│ Child(4B)│ Idx (4B) │ Info │
 * │          │          │          │          │          │ (4B) │
 * └──────────┴──────────┴──────────┴──────────┴──────────┴──────┘
 * Packed: PrimCount | SplitAxis in last int.
 *
 * References:
 * - Wald, "Realtime Ray Tracing and Interactive Global Illumination" (2004)
 * - Lauterbach et al., "Fast BVH Construction on GPUs" (2009)
 * - Intel Embree design (kernel-style packet traversal)
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#pragma once

#include <TohruPhysics/Geometry.h>

#define TAZUSA_MAX_DEPTH        64
#define TAZUSA_DEFAULT_LEAF_SIZE 4
#define TAZUSA_SAH_BINS         32
#define TAZUSA_MAX_NODES        262144  // 256K nodes × 64B ≈ 16 MB
#define TAZUSA_COST_TRAVERSAL   1.0
#define TAZUSA_COST_INTERSECT   1.0

// 0201: BVH node — 64-byte aligned for cache line.
typedef struct TazusaBVHNode {
	AABB Box;           // 24B × 2 = 48B
	int  LeftChild;     // -1 = leaf
	int  RightChild;    // -1 = leaf
	int  Parent;        // -1 = root
	int  PrimitiveIndex; // first primitive index (leaf) or -1 (internal)
	unsigned int Packed; // [0:15] PrimCount, [16:17] SplitAxis, [18:31] flags
} TazusaBVHNode;

// Accessors for Packed field
#define TazusaNodePrimCount(N)   ((int)((N).Packed & 0xFFFF))
#define TazusaNodeSplitAxis(N)   ((int)(((N).Packed >> 16) & 3))
#define TazusaNodeSetPrimCount(N, C)  do { (N).Packed = ((N).Packed & ~0xFFFF) | ((unsigned int)(C) & 0xFFFF); } while(0)
#define TazusaNodeSetSplitAxis(N, A)  do { (N).Packed = ((N).Packed & ~(3u<<16)) | (((unsigned int)(A) & 3) << 16); } while(0)

typedef enum {
	TAZUSA_BUILD_DEFAULT     = 0,
	TAZUSA_BUILD_FAST        = 1,  // fewer bins, coarser splits
	TAZUSA_BUILD_HIGH_QUALITY = 2   // full O(N²) sweep (slow)
} TazusaBuildQuality;

// 0201: BVH tree state.
typedef struct {
	TazusaBVHNode *Nodes;       // flat node array, root at [0]
	int           *PrimIndices; // body indices in BVH traversal order
	int            NodeCapacity;
	int            NodeCount;
	int            PrimCapacity;
	int            PrimCount;
	int            RootIndex;   // always 0 after build
	int            MaxDepthReached;
	int            MaxLeafSize; // max primitives per leaf (default 4)
	int            Dirty;       // non-zero if tree needs full rebuild
} TazusaBVHTree;

// 0206: Serialised BVH — portable flat layout for network/save-state.
typedef struct {
	int    NodeCount;
	int    PrimCount;
	int    MaxLeafSize;
	// Followed by Nodes[NodeCount], then PrimIndices[PrimCount]
} TazusaBVHSerializedHeader;

// ---------------------------------------------------------------------------
//  0201: Initialisation and node layout
// ---------------------------------------------------------------------------

// Init tree with pre-allocated arrays. Returns 0 on success, -1 on invalid.
int TazusaBVHInit(TazusaBVHTree *Tree,
                  TazusaBVHNode *NodeBuffer, int NodeCapacity,
                  int *PrimBuffer, int PrimCapacity);

// Reset tree to empty state (preserves buffers).
void TazusaBVHReset(TazusaBVHTree *Tree);

// 0201: Query node properties.
static inline int  TazusaBVHIsLeaf(const TazusaBVHNode *Node) {
	return Node->LeftChild < 0;
}
static inline int  TazusaBVHIsInternal(const TazusaBVHNode *Node) {
	return Node->LeftChild >= 0;
}

// ---------------------------------------------------------------------------
//  0202: SAH top-down build
// ---------------------------------------------------------------------------

// Build from AABBs. Quality controls bin count / sweep detail.
// Returns number of nodes created, or 0 on failure.
int TazusaBVHBuild(TazusaBVHTree *Tree,
                   const AABB *BodyAABBs,
                   int BodyCount,
                   TazusaBuildQuality Quality);

// ---------------------------------------------------------------------------
//  0203: Rebalance (tree rotations)
// ---------------------------------------------------------------------------

// Attempt tree rotations to improve SAH cost after dynamic updates.
// Returns number of rotations performed.
int TazusaBVHRebalance(TazusaBVHTree *Tree);

// ---------------------------------------------------------------------------
//  0204: Ray traversal
// ---------------------------------------------------------------------------

// Callback for ray-intersection at leaves.
typedef int (*TazusaRayLeafFunc)(int PrimitiveIndex,
                                 const Ray *Ray,
                                 Real *OutT,
                                 void *UserData);

// Traverse ray against BVH, calling LeafFunc for each leaf intersected.
// Returns number of leaf hits, or -1 on invalid input.
int TazusaBVHTraverseRay(TazusaBVHTree *Tree,
                         const Ray *Ray,
                         TazusaRayLeafFunc LeafFunc,
                         void *UserData);

// ---------------------------------------------------------------------------
//  0205: Volume (AABB) traversal
// ---------------------------------------------------------------------------

// Callback for volume-overlap at leaves.
typedef int (*TazusaVolumeLeafFunc)(int PrimitiveIndex,
                                     const AABB *QueryAABB,
                                     void *UserData);

// Traverse AABB against BVH, calling LeafFunc for each leaf that overlaps.
// Returns number of leaf hits, or -1 on invalid input.
int TazusaBVHTraverseVolume(TazusaBVHTree *Tree,
                            const AABB *QueryAABB,
                            TazusaVolumeLeafFunc LeafFunc,
                            void *UserData);

// ---------------------------------------------------------------------------
//  0206: Serialisation
// ---------------------------------------------------------------------------

// Serialise tree to a flat buffer. Returns total bytes written.
// Buffer can be NULL to query required size.
int TazusaBVHSerialize(const TazusaBVHTree *Tree, void *Buffer, int BufferSize);

// Deserialise from flat buffer. NodeBuffer/PrimBuffer must be pre-allocated.
// Returns 0 on success, -1 on version/checksum mismatch.
int TazusaBVHDeserialize(TazusaBVHTree *Tree,
                         const void *Buffer, int BufferSize,
                         TazusaBVHNode *NodeBuffer, int NodeCapacity,
                         int *PrimBuffer, int PrimCapacity);

// ---------------------------------------------------------------------------
//  0207: Refit (bottom-up AABB update)
// ---------------------------------------------------------------------------

// Refit AABBs bottom-up after primitive AABBs have been updated.
// Returns number of nodes whose AABB changed.
int TazusaBVHRefit(TazusaBVHTree *Tree,
                   const AABB *BodyAABBs);

// ---------------------------------------------------------------------------
//  0208: Incremental update for dynamic scenes
// ---------------------------------------------------------------------------

// Full update cycle: refit + rebalance + split leaves.
// Returns 0 on success, -1 on resource exhaustion.
int TazusaBVHUpdate(TazusaBVHTree *Tree,
                    const AABB *BodyAABBs,
                    int BodyCount);

// ---------------------------------------------------------------------------
//  0209: Depth validation
// ---------------------------------------------------------------------------

// Validate that subtree depth does not exceed limit.
// Returns current max depth, or -1 if exceeded.
int TazusaBVHValidateDepth(const TazusaBVHTree *Tree, int MaxDepth);

// ---------------------------------------------------------------------------
//  0210: Dynamic leaf splitting
// ---------------------------------------------------------------------------

// Split overfilled leaves using SAH. Created new internal nodes.
// Returns number of splits performed.
int TazusaBVHSplitLeaves(TazusaBVHTree *Tree,
                         const AABB *BodyAABBs,
                         int BodyCount);
