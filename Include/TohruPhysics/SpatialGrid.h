/**
 * SpatialGrid — uniform spatial subdivision for broad-phase acceleration.
 * TohruPhysics用の一様空間分割 — BroadPhase加速のためね。
 *
 * Maps world-space AABBs to grid cells using a hash table with separate
 * chaining. Provides fast neighbor queries for collision pair generation.
 *
 * DESIGN PHILOSOPHY:
 * Brute-force O(n²) AABB testing becomes prohibitive above ~100 bodies.
 * A uniform grid divides space into fixed-size cells; each body is inserted
 * into every cell its AABB overlaps. Queries only check neighboring cells
 * (the cell containing the query AABB plus its 26 adjacent cells in 3D).
 * Hash table with power-of-two bucket count maps cell coordinates to
 * linked lists of body indices. Cell nodes are pooled to avoid per-frame
 * allocation.
 *
 * DATA LAYOUT:
 * ┌────────────┬──────────────┬──────────────────────────────────┐
 * │ Buckets[]  │ Bucket heads │ Array of cell indices, -1 = empty│
 * │ CellPool[] │ Node storage │ Flat array indexed by int        │
 * │ FreeHead   │ Free list    │ Index of first reusable node     │
 * └────────────┴──────────────┴──────────────────────────────────┘
 *
 * BODY TRACKING:
 * ┌──────────────┬──────────────────────────────────────────────┐
 * │ BodyCells[]  │ Linked list of cells each body occupies      │
 * │ BodyCellNext │ "next" indices for per-body cell lists       │
 * └──────────────┴──────────────────────────────────────────────┘
 *
 * References:
 * - Real-Time Collision Detection (Ericson), Ch 7 — Spatial Partitioning
 * - M. Teschner et al., "Optimized Spatial Hashing for Collision Detection"
 *   (2003)
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#pragma once

#include <TohruPhysics/Geometry.h>

#define SUZU_DEFAULT_BUCKETS  1024
#define SUZU_DEFAULT_POOLSIZE 16384
#define SUZU_MAX_BODIES       2048
#define SUZU_INVALID_INDEX     (-1)

// 0191: Cell node in the hash chain (flat array, int-indexed)
typedef struct {
	int BodyIndex;
	int Next;  // index of next node in chain, SUZU_INVALID_INDEX for end
} SuzuCell;

// 0197: Debug visualisation callback — called per active cell.
typedef void (*SuzuGridDebugCallback)(int CellX, int CellY, int CellZ,
                                      int BodyCount,
                                      void *UserData);

// 0196: Statistics
typedef struct {
	int TotalBodies;
	int ActiveCells;
	int PoolCapacity;
	int PoolUsed;
	int TotalInsertions;
	int TotalRemovals;
	int TotalQueries;
	int TotalCellsVisited;
	int TotalPairsGenerated;
	int AvgCellsPerQuery;
	int MaxCellsVisited;
	int MaxChainLength;
} SuzuStats;

// 0191: Main spatial grid state
typedef struct {
	// Grid dimensions
	Vector3 WorldMin;
	Vector3 WorldMax;
	Real    CellSize;
	int     GridWidth;
	int     GridHeight;
	int     GridDepth;

	// Hash table
	int       BucketCount;
	int       BucketMask;
	int      *Buckets;  // array of bucket heads (cell indices)

	// Cell pool (flat array, int-indexed free list)
	SuzuCell *Cells;
	int       PoolSize;
	int       FreeHead;  // index of first free cell, SUZU_INVALID_INDEX = empty

	// 0195: Per-bucket spinlocks (simple volatile int flag, 0=unlocked, 1=locked)
	int      *BucketLocks;

	// Statistics
	SuzuStats Stats;
} SuzuSpatialGrid;

// 0191: Init grid with world bounds, cell size, bucket count, and pool size.
void SuzuSpatialGridInit(SuzuSpatialGrid *Grid,
                         const Vector3 *WorldMin,
                         const Vector3 *WorldMax,
                         Real CellSize,
                         int BucketCount,
                         int PoolSize);

// 0191: Free allocated memory (call when grid is no longer needed).
void SuzuSpatialGridDestroy(SuzuSpatialGrid *Grid);

// 0192: Insert a body into cells its AABB overlaps.
void SuzuSpatialGridInsert(SuzuSpatialGrid *Grid,
                           int BodyIndex,
                           const AABB *BodyAABB);

// 0192: Remove a body from all cells its AABB overlaps.
void SuzuSpatialGridRemove(SuzuSpatialGrid *Grid,
                           int BodyIndex,
                           const AABB *BodyAABB);

// 0192: Update body position — remove from old cells, insert into new.
void SuzuSpatialGridUpdate(SuzuSpatialGrid *Grid,
                           int BodyIndex,
                           const AABB *OldAABB,
                           const AABB *NewAABB);

// 0193: Query all bodies whose AABB overlaps QueryAABB.
// Returns count of bodies found (up to MaxOut).
int SuzuSpatialGridQuery(SuzuSpatialGrid *Grid,
                         const AABB *QueryAABB,
                         int *OutBodies,
                         int MaxOut);

// 0193: Generate collision pairs for all tracked bodies using grid queries.
// BodyA < BodyB in all output pairs to avoid duplicates.
void SuzuSpatialGridGeneratePairs(SuzuSpatialGrid *Grid,
                                  const AABB *BodyAABBs,
                                  int BodyCount,
                                  int *OutPairs,
                                  int *OutPairCount,
                                  int MaxPairs);

// 0192: Clear all cells and reset tracker state.
void SuzuSpatialGridClear(SuzuSpatialGrid *Grid);

// 0196: Get current statistics snapshot.
SuzuStats SuzuSpatialGridGetStats(SuzuSpatialGrid *Grid);

// 0196: Reset statistics counters.
void SuzuSpatialGridResetStats(SuzuSpatialGrid *Grid);

// 0199: Hash helper — maps cell coords to bucket index (power-of-2 fast path).
int  SuzuSpatialGridHash(int CellX, int CellY, int CellZ, int BucketMask);

// 0199: Allocate a cell node from the pool (free list pop).
int  SuzuSpatialGridAllocCell(SuzuSpatialGrid *Grid);

// 0199: Free a cell node back to the pool (free list push).
void SuzuSpatialGridFreeCell(SuzuSpatialGrid *Grid, int CellIndex);

// 0194: Compute optimal cell size based on body AABB distribution.
// Analyses average AABB extents and recommends a cell size.
Real SuzuSpatialGridComputeOptimalCellSize(SuzuSpatialGrid *Grid,
                                           const AABB *BodyAABBs,
                                           int BodyCount);

// 0195: Lock/unlock a bucket for thread-safe concurrent access.
void SuzuSpatialGridLockBucket(SuzuSpatialGrid *Grid, int BucketIndex);
void SuzuSpatialGridUnlockBucket(SuzuSpatialGrid *Grid, int BucketIndex);

// 0197: Debug visualisation — iterate all active cells and call callback.
void SuzuSpatialGridDebugCells(SuzuSpatialGrid *Grid,
                               SuzuGridDebugCallback Callback,
                               void *UserData);

// 0198: Resize grid with a new cell size. Rebuilds all internal data
// structures. All existing body indices are preserved.
void SuzuSpatialGridResize(SuzuSpatialGrid *Grid,
                           Real NewCellSize);
