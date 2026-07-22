/**
 * SpatialGrid — uniform spatial subdivision implementation.
 * TohruPhysics用の一様空間分割実装ね。
 *
 * Flat-array cell pool with int-indexed free list. Hash table with
 * separate chaining. No per-body cell tracking — removal re-scans
 * cells from the AABB.
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include <TohruPhysics/SpatialGrid.h>
#include <TohruPhysics/Math.h>
#include <stdlib.h>
#include <string.h>

// ---------------------------------------------------------------------------
//  Internal helpers
// ---------------------------------------------------------------------------

static void PosToCell(SuzuSpatialGrid *Grid, const Vector3 *Pos,
                      int *CX, int *CY, int *CZ) {
	Real FX = (Pos->Data[0] - Grid->WorldMin.Data[0]) / Grid->CellSize;
	Real FY = (Pos->Data[1] - Grid->WorldMin.Data[1]) / Grid->CellSize;
	Real FZ = (Pos->Data[2] - Grid->WorldMin.Data[2]) / Grid->CellSize;
	*CX = (int)SulettaFloor(FX);
	*CY = (int)SulettaFloor(FY);
	*CZ = (int)SulettaFloor(FZ);
}

static int MaxCellOf(Real V, Real Origin, Real CellSize) {
	Real T = (V - Origin) / CellSize;
	// ceil(V/cellSize) - 1 handles boundary cases correctly
	return (int)SulettaCeil(T) - 1;
}

static void AABBToCellRange(SuzuSpatialGrid *Grid, const AABB *Box,
                            int *MinCX, int *MinCY, int *MinCZ,
                            int *MaxCX, int *MaxCY, int *MaxCZ) {
	PosToCell(Grid, &Box->Min, MinCX, MinCY, MinCZ);
	*MaxCX = MaxCellOf(Box->Max.Data[0], Grid->WorldMin.Data[0], Grid->CellSize);
	*MaxCY = MaxCellOf(Box->Max.Data[1], Grid->WorldMin.Data[1], Grid->CellSize);
	*MaxCZ = MaxCellOf(Box->Max.Data[2], Grid->WorldMin.Data[2], Grid->CellSize);

	// Clamp to grid bounds
	*MinCX = YuuClamp(*MinCX, 0, Grid->GridWidth - 1);
	*MinCY = YuuClamp(*MinCY, 0, Grid->GridHeight - 1);
	*MinCZ = YuuClamp(*MinCZ, 0, Grid->GridDepth - 1);
	*MaxCX = YuuClamp(*MaxCX, 0, Grid->GridWidth - 1);
	*MaxCY = YuuClamp(*MaxCY, 0, Grid->GridHeight - 1);
	*MaxCZ = YuuClamp(*MaxCZ, 0, Grid->GridDepth - 1);
}

// ---------------------------------------------------------------------------
//  0199: Hash function + bucket helpers
// ---------------------------------------------------------------------------

int SuzuSpatialGridHash(int CellX, int CellY, int CellZ, int BucketMask) {
	unsigned int H = (unsigned int)CellX * 73856093u
	               ^ (unsigned int)CellY * 19349663u
	               ^ (unsigned int)CellZ * 83492791u;
	return (int)(H & (unsigned int)BucketMask);
}

int SuzuSpatialGridAllocCell(SuzuSpatialGrid *Grid) {
	if (Grid->FreeHead == SUZU_INVALID_INDEX) {
		return SUZU_INVALID_INDEX;
	}
	int Idx = Grid->FreeHead;
	Grid->FreeHead = Grid->Cells[Idx].Next;
	Grid->Cells[Idx].Next = SUZU_INVALID_INDEX;
	Grid->Stats.PoolUsed++;
	Grid->Stats.ActiveCells++;
	return Idx;
}

void SuzuSpatialGridFreeCell(SuzuSpatialGrid *Grid, int CellIndex) {
	if (CellIndex < 0 || CellIndex >= Grid->PoolSize) return;
	Grid->Cells[CellIndex].BodyIndex = 0;
	Grid->Cells[CellIndex].Next = Grid->FreeHead;
	Grid->FreeHead = CellIndex;
	Grid->Stats.PoolUsed--;
	Grid->Stats.ActiveCells--;
}

// ---------------------------------------------------------------------------
//  0191: Init + Destroy
// ---------------------------------------------------------------------------

void SuzuSpatialGridDestroy(SuzuSpatialGrid *Grid) {
	free(Grid->Buckets);
	free(Grid->Cells);
	free(Grid->BucketLocks);
	Grid->Buckets = NULL;
	Grid->Cells = NULL;
	Grid->BucketLocks = NULL;
	Grid->PoolSize = 0;
	Grid->BucketCount = 0;
}

void SuzuSpatialGridInit(SuzuSpatialGrid *Grid,
                         const Vector3 *WorldMin,
                         const Vector3 *WorldMax,
                         Real CellSize,
                         int BucketCount,
                         int PoolSize) {
	memset(Grid, 0, sizeof(*Grid));

	Grid->WorldMin = *WorldMin;
	Grid->WorldMax = *WorldMax;
	Grid->CellSize = CellSize > REAL_ZERO ? CellSize : 1.0;

	Grid->GridWidth  = (int)SulettaCeil(
		(WorldMax->Data[0] - WorldMin->Data[0]) / Grid->CellSize);
	Grid->GridHeight = (int)SulettaCeil(
		(WorldMax->Data[1] - WorldMin->Data[1]) / Grid->CellSize);
	Grid->GridDepth  = (int)SulettaCeil(
		(WorldMax->Data[2] - WorldMin->Data[2]) / Grid->CellSize);

	if (Grid->GridWidth  < 1) Grid->GridWidth  = 1;
	if (Grid->GridHeight < 1) Grid->GridHeight = 1;
	if (Grid->GridDepth  < 1) Grid->GridDepth  = 1;

	// Bucket count: power of 2
	int BC = 1;
	while (BC < BucketCount) BC <<= 1;
	Grid->BucketCount = BC;
	Grid->BucketMask = BC - 1;

	// Allocate bucket array
	Grid->Buckets = (int *)calloc((size_t)BC, sizeof(int));
	for (int I = 0; I < BC; I++) {
		Grid->Buckets[I] = SUZU_INVALID_INDEX;
	}

	// 0195: Per-bucket spinlocks
	Grid->BucketLocks = (int *)calloc((size_t)BC, sizeof(int));

	if (PoolSize < 1) PoolSize = SUZU_DEFAULT_POOLSIZE;
	Grid->PoolSize = PoolSize;
	Grid->Cells = (SuzuCell *)calloc((size_t)PoolSize, sizeof(SuzuCell));

	// Build free list
	Grid->FreeHead = 0;
	for (int I = 0; I < PoolSize - 1; I++) {
		Grid->Cells[I].Next = I + 1;
	}
	Grid->Cells[PoolSize - 1].Next = SUZU_INVALID_INDEX;

	Grid->Stats.PoolCapacity = PoolSize;
}

// ---------------------------------------------------------------------------
//  0192: Insert
// ---------------------------------------------------------------------------

void SuzuSpatialGridInsert(SuzuSpatialGrid *Grid,
                           int BodyIndex,
                           const AABB *BodyAABB) {
	int MinCX, MinCY, MinCZ, MaxCX, MaxCY, MaxCZ;
	AABBToCellRange(Grid, BodyAABB, &MinCX, &MinCY, &MinCZ,
	                &MaxCX, &MaxCY, &MaxCZ);

	for (int CZ = MinCZ; CZ <= MaxCZ; CZ++) {
		for (int CY = MinCY; CY <= MaxCY; CY++) {
			for (int CX = MinCX; CX <= MaxCX; CX++) {
				int BucketIdx = SuzuSpatialGridHash(CX, CY, CZ, Grid->BucketMask);
				int CellIdx = SuzuSpatialGridAllocCell(Grid);
				if (CellIdx == SUZU_INVALID_INDEX) continue;

				Grid->Cells[CellIdx].BodyIndex = BodyIndex;
				Grid->Cells[CellIdx].Next = Grid->Buckets[BucketIdx];
				Grid->Buckets[BucketIdx] = CellIdx;
			}
		}
	}

	Grid->Stats.TotalBodies++;
	Grid->Stats.TotalInsertions++;
}

// ---------------------------------------------------------------------------
//  0192: Remove
// ---------------------------------------------------------------------------

void SuzuSpatialGridRemove(SuzuSpatialGrid *Grid,
                           int BodyIndex,
                           const AABB *BodyAABB) {
	int MinCX, MinCY, MinCZ, MaxCX, MaxCY, MaxCZ;
	AABBToCellRange(Grid, BodyAABB, &MinCX, &MinCY, &MinCZ,
	                &MaxCX, &MaxCY, &MaxCZ);

	for (int CZ = MinCZ; CZ <= MaxCZ; CZ++) {
		for (int CY = MinCY; CY <= MaxCY; CY++) {
			for (int CX = MinCX; CX <= MaxCX; CX++) {
				int BucketIdx = SuzuSpatialGridHash(CX, CY, CZ, Grid->BucketMask);
				int *Prev = &Grid->Buckets[BucketIdx];
				int Cur = *Prev;

				while (Cur != SUZU_INVALID_INDEX) {
					if (Grid->Cells[Cur].BodyIndex == BodyIndex) {
						// Unlink from bucket chain
						*Prev = Grid->Cells[Cur].Next;
						SuzuSpatialGridFreeCell(Grid, Cur);
						break;
					}
					Prev = &Grid->Cells[Cur].Next;
					Cur = *Prev;
				}
			}
		}
	}

	Grid->Stats.TotalRemovals++;
}

// ---------------------------------------------------------------------------
//  0192: Update
// ---------------------------------------------------------------------------

void SuzuSpatialGridUpdate(SuzuSpatialGrid *Grid,
                           int BodyIndex,
                           const AABB *OldAABB,
                           const AABB *NewAABB) {
	SuzuSpatialGridRemove(Grid, BodyIndex, OldAABB);
	SuzuSpatialGridInsert(Grid, BodyIndex, NewAABB);
}

// ---------------------------------------------------------------------------
//  0193: Query
// ---------------------------------------------------------------------------

int SuzuSpatialGridQuery(SuzuSpatialGrid *Grid,
                         const AABB *QueryAABB,
                         int *OutBodies,
                         int MaxOut) {
	int MinCX, MinCY, MinCZ, MaxCX, MaxCY, MaxCZ;
	AABBToCellRange(Grid, QueryAABB, &MinCX, &MinCY, &MinCZ,
	                &MaxCX, &MaxCY, &MaxCZ);

	// Visited marker to deduplicate bodies across multiple cells
	// (a body spanning multiple cells would be returned multiple times)
	int Visited[SUZU_MAX_BODIES];
	memset(Visited, 0, sizeof(Visited));

	int OutCount = 0;
	int CellsVisited = 0;

	for (int CZ = MinCZ; CZ <= MaxCZ; CZ++) {
		for (int CY = MinCY; CY <= MaxCY; CY++) {
			for (int CX = MinCX; CX <= MaxCX; CX++) {
				int BucketIdx = SuzuSpatialGridHash(CX, CY, CZ, Grid->BucketMask);
				int Cur = Grid->Buckets[BucketIdx];

				while (Cur != SUZU_INVALID_INDEX) {
					CellsVisited++;
					int BI = Grid->Cells[Cur].BodyIndex;
					if (BI >= 0 && BI < SUZU_MAX_BODIES && !Visited[BI]) {
						Visited[BI] = 1;
						if (OutCount < MaxOut && OutBodies) {
							OutBodies[OutCount] = BI;
						}
						OutCount++;
					}
					Cur = Grid->Cells[Cur].Next;
				}
			}
		}
	}

	// Update stats
	Grid->Stats.TotalQueries++;
	Grid->Stats.TotalCellsVisited += CellsVisited;
	if (CellsVisited > Grid->Stats.MaxCellsVisited) {
		Grid->Stats.MaxCellsVisited = CellsVisited;
	}
	if (Grid->Stats.TotalQueries > 0) {
		Grid->Stats.AvgCellsPerQuery =
			Grid->Stats.TotalCellsVisited / Grid->Stats.TotalQueries;
	}

	// Compute max chain length for stats
	int MaxChain = 0;
	for (int I = 0; I < Grid->BucketCount; I++) {
		int Len = 0;
		int Cur = Grid->Buckets[I];
		while (Cur != SUZU_INVALID_INDEX) {
			Len++;
			Cur = Grid->Cells[Cur].Next;
		}
		if (Len > MaxChain) MaxChain = Len;
	}
	Grid->Stats.MaxChainLength = MaxChain;

	return OutCount;
}

// ---------------------------------------------------------------------------
//  0193: Generate pairs for all bodies
// ---------------------------------------------------------------------------

void SuzuSpatialGridGeneratePairs(SuzuSpatialGrid *Grid,
                                  const AABB *BodyAABBs,
                                  int BodyCount,
                                  int *OutPairs,
                                  int *OutPairCount,
                                  int MaxPairs) {
	int Pairs = 0;
	memset(OutPairs, 0, (size_t)MaxPairs * 2 * sizeof(int));

	for (int I = 0; I < BodyCount && Pairs < MaxPairs; I++) {
		int Neighbors[SUZU_MAX_BODIES];
		int NumNbrs = SuzuSpatialGridQuery(Grid, &BodyAABBs[I],
		                                    Neighbors, SUZU_MAX_BODIES);

		for (int J = 0; J < NumNbrs && Pairs < MaxPairs; J++) {
			int N = Neighbors[J];
			// Only add pair if I < N to avoid duplicates
			if (I < N && N < BodyCount) {
				OutPairs[Pairs * 2]     = I;
				OutPairs[Pairs * 2 + 1] = N;
				Pairs++;
				Grid->Stats.TotalPairsGenerated++;
			}
		}
	}

	*OutPairCount = Pairs;
}

// ---------------------------------------------------------------------------
//  0192: Clear
// ---------------------------------------------------------------------------

void SuzuSpatialGridClear(SuzuSpatialGrid *Grid) {
	// Reset bucket heads
	for (int I = 0; I < Grid->BucketCount; I++) {
		Grid->Buckets[I] = SUZU_INVALID_INDEX;
	}

	// Rebuild free list from all cells
	Grid->FreeHead = 0;
	for (int I = 0; I < Grid->PoolSize - 1; I++) {
		Grid->Cells[I].BodyIndex = 0;
		Grid->Cells[I].Next = I + 1;
	}
	Grid->Cells[Grid->PoolSize - 1].BodyIndex = 0;
	Grid->Cells[Grid->PoolSize - 1].Next = SUZU_INVALID_INDEX;

	Grid->Stats.ActiveCells = 0;
	Grid->Stats.PoolUsed = 0;
}

// ---------------------------------------------------------------------------
//  0196: Stats
// ---------------------------------------------------------------------------

SuzuStats SuzuSpatialGridGetStats(SuzuSpatialGrid *Grid) {
	return Grid->Stats;
}

void SuzuSpatialGridResetStats(SuzuSpatialGrid *Grid) {
	memset(&Grid->Stats, 0, sizeof(SuzuStats));
	Grid->Stats.PoolCapacity = Grid->PoolSize;
}

// ===========================================================================
//  0194: Compute optimal cell size
// ===========================================================================

Real SuzuSpatialGridComputeOptimalCellSize(SuzuSpatialGrid *Grid,
                                           const AABB *BodyAABBs,
                                           int BodyCount) {
	(void)Grid;
	if (BodyCount <= 0) return Grid->CellSize;

	// Compute average AABB extents
	Real AvgExtentX = REAL_ZERO;
	Real AvgExtentY = REAL_ZERO;
	Real AvgExtentZ = REAL_ZERO;

	for (int I = 0; I < BodyCount; I++) {
		Vector3 HE = SabinaAABBHalfExtents(&BodyAABBs[I]);
		AvgExtentX += HE.Data[0] * 2.0;
		AvgExtentY += HE.Data[1] * 2.0;
		AvgExtentZ += HE.Data[2] * 2.0;
	}

	Real InvN = 1.0 / (Real)BodyCount;
	AvgExtentX *= InvN;
	AvgExtentY *= InvN;
	AvgExtentZ *= InvN;

	// Cell size = average extent × 2 (so most bodies span 2-4 cells)
	Real AvgMax = YuuMax(AvgExtentX, YuuMax(AvgExtentY, AvgExtentZ));
	Real CellSize = AvgMax * 2.0;

	// Clamp to reasonable range
	if (CellSize < 0.1) CellSize = 0.1;
	Real WorldSpanX = Grid->WorldMax.Data[0] - Grid->WorldMin.Data[0];
	if (WorldSpanX > REAL_ZERO && CellSize > WorldSpanX * 0.5)
		CellSize = WorldSpanX * 0.5;

	return CellSize;
}

// ===========================================================================
//  0195: Per-bucket spinlocks
// ===========================================================================

void SuzuSpatialGridLockBucket(SuzuSpatialGrid *Grid, int BucketIndex) {
	if (!Grid->BucketLocks) return;
	if (BucketIndex < 0 || BucketIndex >= Grid->BucketCount) return;
	// Simple test-and-set spinlock
	while (__atomic_test_and_set(&Grid->BucketLocks[BucketIndex],
	                             __ATOMIC_ACQUIRE)) {
		// Spin — yield to reduce contention
		__builtin_ia32_pause();
	}
}

void SuzuSpatialGridUnlockBucket(SuzuSpatialGrid *Grid, int BucketIndex) {
	if (!Grid->BucketLocks) return;
	if (BucketIndex < 0 || BucketIndex >= Grid->BucketCount) return;
	__atomic_clear(&Grid->BucketLocks[BucketIndex], __ATOMIC_RELEASE);
}

// ===========================================================================
//  0197: Debug cells
// ===========================================================================

void SuzuSpatialGridDebugCells(SuzuSpatialGrid *Grid,
                               SuzuGridDebugCallback Callback,
                               void *UserData) {
	if (!Callback) return;

	// Iterate all buckets; for each active cell, reconstruct its cell
	// coordinates from the bucket chain and call the callback.
	// Note: we don't have a reverse mapping from cell node → cell coords,
	// so we track visited buckets and report per-bucket body counts.
	for (int B = 0; B < Grid->BucketCount; B++) {
		int Cur = Grid->Buckets[B];
		if (Cur == SUZU_INVALID_INDEX) continue;

		// Count bodies in this bucket chain
		int BodyCount = 0;
		while (Cur != SUZU_INVALID_INDEX) {
			if (Grid->Cells[Cur].BodyIndex >= 0)
				BodyCount++;
			Cur = Grid->Cells[Cur].Next;
		}

		if (BodyCount == 0) continue;

		// We don't have cell coordinates stored per node, so use
		// the bucket index as a proxy and report (0,0,0) cell coords.
		// A full implementation would store cell coords in SuzuCell.
		Callback(0, 0, B, BodyCount, UserData);
	}
}

// ===========================================================================
//  0198: Resize grid with new cell size
// ===========================================================================

void SuzuSpatialGridResize(SuzuSpatialGrid *Grid, Real NewCellSize) {
	if (NewCellSize <= REAL_ZERO) return;
	if (Grid->BucketCount == 0) return;

	// Save old state for scanning
	int OldBucketCount = Grid->BucketCount;
	int *OldBuckets = Grid->Buckets;
	SuzuCell *OldCells = Grid->Cells;

	// Determine new dimensions
	Real NewGridW = (Grid->WorldMax.Data[0] - Grid->WorldMin.Data[0])
	                / NewCellSize;
	Real NewGridH = (Grid->WorldMax.Data[1] - Grid->WorldMin.Data[1])
	                / NewCellSize;
	Real NewGridD = (Grid->WorldMax.Data[2] - Grid->WorldMin.Data[2])
	                / NewCellSize;

	int EstCells = (int)(NewGridW * NewGridH * NewGridD);
	if (EstCells < 64) EstCells = 64;

	int NewBucketCount = 1;
	while (NewBucketCount < EstCells * 4)
		NewBucketCount <<= 1;
	if (NewBucketCount < 64) NewBucketCount = 64;
	if (NewBucketCount > 65536) NewBucketCount = 65536;

	int NewBucketMask = NewBucketCount - 1;

	// Allocate and swap in new bucket array
	int *NewBuckets = (int *)calloc((size_t)NewBucketCount, sizeof(int));
	for (int I = 0; I < NewBucketCount; I++)
		NewBuckets[I] = SUZU_INVALID_INDEX;

	int *NewLocks = (int *)calloc((size_t)NewBucketCount, sizeof(int));

	// Scan old data for body indices BEFORE freeing anything
	int BodySet[SUZU_MAX_BODIES];
	int BodyCount = 0;
	memset(BodySet, 0, sizeof(BodySet));
	for (int B = 0; B < OldBucketCount; B++) {
		int Cur = OldBuckets[B];
		while (Cur != SUZU_INVALID_INDEX) {
			int BI = OldCells[Cur].BodyIndex;
			if (BI >= 0 && BI < SUZU_MAX_BODIES && !BodySet[BI]) {
				BodySet[BI] = 1;
				BodyCount++;
			}
			Cur = OldCells[Cur].Next;
		}
	}

	// Build new cell pool
	int NewPoolSize = Grid->PoolSize > 0 ? Grid->PoolSize : SUZU_DEFAULT_POOLSIZE;
	SuzuCell *NewCells = (SuzuCell *)calloc((size_t)NewPoolSize,
	                                         sizeof(SuzuCell));
	int NewFreeHead = 0;
	for (int I = 0; I < NewPoolSize - 1; I++)
		NewCells[I].Next = I + 1;
	NewCells[NewPoolSize - 1].Next = SUZU_INVALID_INDEX;

	// Free old arrays and swap in new state
	free(OldBuckets);
	free(Grid->BucketLocks);
	free(OldCells);

	Grid->Buckets = NewBuckets;
	Grid->BucketLocks = NewLocks;
	Grid->Cells = NewCells;
	Grid->FreeHead = NewFreeHead;
	Grid->PoolSize = NewPoolSize;
	Grid->BucketCount = NewBucketCount;
	Grid->BucketMask = NewBucketMask;
	Grid->CellSize = NewCellSize;

	// Recompute grid dimensions
	Grid->GridWidth = (int)SulettaCeil(
		(Grid->WorldMax.Data[0] - Grid->WorldMin.Data[0]) / NewCellSize);
	Grid->GridHeight = (int)SulettaCeil(
		(Grid->WorldMax.Data[1] - Grid->WorldMin.Data[1]) / NewCellSize);
	Grid->GridDepth = (int)SulettaCeil(
		(Grid->WorldMax.Data[2] - Grid->WorldMin.Data[2]) / NewCellSize);
	if (Grid->GridWidth  < 1) Grid->GridWidth  = 1;
	if (Grid->GridHeight < 1) Grid->GridHeight = 1;
	if (Grid->GridDepth  < 1) Grid->GridDepth  = 1;

	// Reinsert body indices near the world center (caller should refresh positions)
	int CenterX = Grid->GridWidth / 2;
	int CenterY = Grid->GridHeight / 2;
	int CenterZ = Grid->GridDepth / 2;
	int NewActive = 0;
	for (int I = 0; I < SUZU_MAX_BODIES && BodyCount > 0; I++) {
		if (!BodySet[I]) continue;
		BodyCount--;

		int BIdx = SuzuSpatialGridHash(CenterX, CenterY, CenterZ, NewBucketMask);
		int CellIdx = SuzuSpatialGridAllocCell(Grid);
		if (CellIdx == SUZU_INVALID_INDEX) break;
		Grid->Cells[CellIdx].BodyIndex = I;
		Grid->Cells[CellIdx].Next = Grid->Buckets[BIdx];
		Grid->Buckets[BIdx] = CellIdx;
		NewActive++;
	}

	Grid->Stats.ActiveCells = NewActive;
	Grid->Stats.PoolCapacity = Grid->PoolSize;
}
