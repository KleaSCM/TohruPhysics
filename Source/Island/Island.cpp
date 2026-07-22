/**
 * KuyuIsland — simulation island implementation.
 * TohruPhysics用のシミュレーションアイランド実装ね。
 *
 * Implements 0221-0230: island lifecycle, adjacency graph, connected
 * components, activation, deactivation, deep sleep, connection queries,
 * island split/merge, and load balancing.
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include <TohruPhysics/Island.h>
#include <TohruPhysics/Math.h>
#include <stdlib.h>
#include <string.h>

// ===========================================================================
//  Internal helpers
// ===========================================================================

// Count edges in the pair list and degree per body.
static int CountDegrees(int *Degrees, const int *Pairs, int PairCount,
                        int BodyCount) {
	memset(Degrees, 0, (size_t)BodyCount * sizeof(int));
	int EdgeCount = 0;
	for (int I = 0; I < PairCount; I++) {
		int A = Pairs[I * 2];
		int B = Pairs[I * 2 + 1];
		if (A >= 0 && A < BodyCount && B >= 0 && B < BodyCount && A != B) {
			Degrees[A]++;
			Degrees[B]++;
			EdgeCount++;
		}
	}
	return EdgeCount;
}

// ===========================================================================
//  0221: Island lifecycle
// ===========================================================================

int KuyuIslandInit(KuyuIslandManager *Mgr,
                   int BodyCapacity, int IslandCapacity) {
	if (!Mgr) return -1;
	memset(Mgr, 0, sizeof(*Mgr));

	if (BodyCapacity <= 0) BodyCapacity = 256;
	if (IslandCapacity <= 0) IslandCapacity = 64;

	Mgr->BodyInfo = (KuyuBodyIslandInfo *)calloc(
		(size_t)BodyCapacity, sizeof(KuyuBodyIslandInfo));
	if (!Mgr->BodyInfo) return -1;

	Mgr->Islands = (KuyuIslandGroup *)calloc(
		(size_t)IslandCapacity, sizeof(KuyuIslandGroup));
	if (!Mgr->Islands) {
		free(Mgr->BodyInfo);
		Mgr->BodyInfo = NULL;
		return -1;
	}

	// Init empty islands
	for (int I = 0; I < IslandCapacity; I++) {
		Mgr->Islands[I].FirstBody = -1;
		Mgr->Islands[I].IsActive = 1;
	}

	Mgr->WorkStack = (int *)calloc((size_t)BodyCapacity, sizeof(int));
	Mgr->WorkQueue = (int *)calloc((size_t)BodyCapacity, sizeof(int));
	if (!Mgr->WorkStack || !Mgr->WorkQueue) {
		free(Mgr->WorkStack);
		free(Mgr->WorkQueue);
		free(Mgr->Islands);
		free(Mgr->BodyInfo);
		return -1;
	}

	Mgr->BodyCapacity = BodyCapacity;
	Mgr->IslandCapacity = IslandCapacity;
	Mgr->IslandCount = 0;
	Mgr->SleepThresholdLinear = KUYU_SLEEP_LINEAR;
	Mgr->SleepThresholdAngular = KUYU_SLEEP_ANGULAR;
	Mgr->SleepFramesThreshold = KUYU_SLEEP_FRAMES;

	// Init graph
	Mgr->Graph.RowStarts = NULL;
	Mgr->Graph.Neighbors = NULL;
	Mgr->Graph.BodyCount = 0;
	Mgr->Graph.EdgeCount = 0;

	return 0;
}

void KuyuIslandDestroy(KuyuIslandManager *Mgr) {
	if (!Mgr) return;
	free(Mgr->BodyInfo);
	free(Mgr->Islands);
	free(Mgr->WorkStack);
	free(Mgr->WorkQueue);
	free(Mgr->Graph.RowStarts);
	free(Mgr->Graph.Neighbors);
	memset(Mgr, 0, sizeof(*Mgr));
}

void KuyuIslandReset(KuyuIslandManager *Mgr) {
	if (!Mgr) return;
	memset(Mgr->BodyInfo, 0,
		(size_t)Mgr->BodyCapacity * sizeof(KuyuBodyIslandInfo));
	for (int I = 0; I < Mgr->IslandCapacity; I++) {
		Mgr->Islands[I].FirstBody = -1;
		Mgr->Islands[I].BodyCount = 0;
		Mgr->Islands[I].IsActive = 1;
		Mgr->Islands[I].IsDeepSleep = 0;
		Mgr->Islands[I].SleepTimer = 0;
		Mgr->Islands[I].Energy = 0.0;
	}
	Mgr->IslandCount = 0;
	Mgr->Graph.BodyCount = 0;
	Mgr->Graph.EdgeCount = 0;
}

// ===========================================================================
//  0222: Build adjacency graph
// ===========================================================================

int KuyuIslandBuildGraph(KuyuIslandManager *Mgr,
                         const int *Pairs, int PairCount) {
	if (!Mgr || !Pairs) return -1;
	if (PairCount == 0) {
		// Clear graph
		Mgr->Graph.EdgeCount = 0;
		Mgr->Graph.BodyCount = Mgr->BodyCapacity;
		return 0;
	}

	// First pass: count degrees
	int *Degrees = (int *)calloc((size_t)Mgr->BodyCapacity, sizeof(int));
	if (!Degrees) return -1;

	int EdgeCount = CountDegrees(Degrees, Pairs, PairCount, Mgr->BodyCapacity);

	// Allocate CSR arrays
	int NewRowCap = Mgr->BodyCapacity + 1;
	int *NewRows = (int *)realloc(Mgr->Graph.RowStarts,
		(size_t)NewRowCap * sizeof(int));
	if (!NewRows) { free(Degrees); return -1; }
	Mgr->Graph.RowStarts = NewRows;

	int *NewNbrs = (int *)realloc(Mgr->Graph.Neighbors,
		(size_t)(EdgeCount * 2) * sizeof(int));
	if (!NewNbrs) { free(Degrees); return -1; }
	Mgr->Graph.Neighbors = NewNbrs;

	// Build row pointers (prefix sum of degrees)
	Mgr->Graph.RowStarts[0] = 0;
	for (int I = 0; I < Mgr->BodyCapacity; I++) {
		Mgr->Graph.RowStarts[I + 1] = Mgr->Graph.RowStarts[I] + Degrees[I];
	}

	// Reset degrees to use as fill counters
	int *Fill = (int *)calloc((size_t)Mgr->BodyCapacity, sizeof(int));
	if (!Fill) { free(Degrees); return -1; }

	// Second pass: fill neighbors
	for (int I = 0; I < PairCount; I++) {
		int A = Pairs[I * 2];
		int B = Pairs[I * 2 + 1];
		if (A < 0 || A >= Mgr->BodyCapacity ||
		    B < 0 || B >= Mgr->BodyCapacity || A == B) continue;

		int PosA = Mgr->Graph.RowStarts[A] + Fill[A];
		int PosB = Mgr->Graph.RowStarts[B] + Fill[B];

		Mgr->Graph.Neighbors[PosA] = B;
		Mgr->Graph.Neighbors[PosB] = A;

		Fill[A]++;
		Fill[B]++;
	}

	Mgr->Graph.BodyCount = Mgr->BodyCapacity;
	Mgr->Graph.EdgeCount = EdgeCount;

	free(Degrees);
	free(Fill);
	return 0;
}

// ===========================================================================
//  0223: Connected components
// ===========================================================================

static int FindIslandForBody(KuyuIslandManager *Mgr, int StartBody,
                             int IslandIndex) {
	// BFS from StartBody
	int *Q = Mgr->WorkQueue;
	int QHead = 0, QTail = 0;

	Q[QTail++] = StartBody;
	Mgr->BodyInfo[StartBody].Visited = 1;

	while (QHead < QTail) {
		int BIdx = Q[QHead++];
		KuyuBodyIslandInfo *BI = &Mgr->BodyInfo[BIdx];

		BI->IslandIndex = IslandIndex;

		// Link into island linked list
		KuyuIslandGroup *Isl = &Mgr->Islands[IslandIndex];
		if (Isl->FirstBody < 0) {
			Isl->FirstBody = BIdx;
			BI->IslandPrev = -1;
			BI->IslandNext = -1;
		} else {
			// Prepend to head
			BI->IslandPrev = -1;
			BI->IslandNext = Isl->FirstBody;
			Mgr->BodyInfo[Isl->FirstBody].IslandPrev = BIdx;
			Isl->FirstBody = BIdx;
		}
		Isl->BodyCount++;

		// Visit neighbors
		int Start = Mgr->Graph.RowStarts[BIdx];
		int End = Mgr->Graph.RowStarts[BIdx + 1];
		for (int J = Start; J < End; J++) {
			int Nbr = Mgr->Graph.Neighbors[J];
			if (!Mgr->BodyInfo[Nbr].Visited) {
				Mgr->BodyInfo[Nbr].Visited = 1;
				Q[QTail++] = Nbr;
			}
		}
	}

	return QTail;  // number of bodies found
}

int KuyuIslandFindComponents(KuyuIslandManager *Mgr) {
	if (!Mgr) return -1;
	if (Mgr->Graph.BodyCount == 0) return 0;

	// Reset body info and islands
	memset(Mgr->BodyInfo, 0,
		(size_t)Mgr->BodyCapacity * sizeof(KuyuBodyIslandInfo));
	for (int I = 0; I < Mgr->IslandCapacity; I++) {
		Mgr->Islands[I].FirstBody = -1;
		Mgr->Islands[I].BodyCount = 0;
	}

	Mgr->IslandCount = 0;

	int ActiveCount = 0;
	for (int I = 0; I < Mgr->BodyCapacity; I++) {
		if (Mgr->Graph.RowStarts[I + 1] > Mgr->Graph.RowStarts[I] ||
		    Mgr->BodyInfo[I].Visited) {
			ActiveCount++;
		}
	}

	for (int I = 0; I < Mgr->BodyCapacity; I++) {
		if (Mgr->BodyInfo[I].Visited) continue;

		int Degree = Mgr->Graph.RowStarts[I + 1] - Mgr->Graph.RowStarts[I];
		if (Degree == 0 && I >= ActiveCount) continue;
		if (Degree == 0) {
			// Isolated body — its own island of size 1
			if (Mgr->IslandCount >= Mgr->IslandCapacity) break;
			int IslIdx = Mgr->IslandCount++;
			KuyuIslandGroup *Isl = &Mgr->Islands[IslIdx];
			Isl->FirstBody = I;
			Isl->BodyCount = 1;
			Isl->IsActive = 1;
			Mgr->BodyInfo[I].IslandIndex = IslIdx;
			Mgr->BodyInfo[I].Visited = 1;
			Mgr->BodyInfo[I].IslandPrev = -1;
			Mgr->BodyInfo[I].IslandNext = -1;
			continue;
		}

		if (Mgr->IslandCount >= Mgr->IslandCapacity) break;

		int IslIdx = Mgr->IslandCount;
		// Activate island slot
		Mgr->Islands[IslIdx].IsActive = 1;
		Mgr->Islands[IslIdx].IsDeepSleep = 0;

		int Found = FindIslandForBody(Mgr, I, IslIdx);
		if (Found > 0) {
			Mgr->IslandCount++;
		}
	}

	return Mgr->IslandCount;
}

// ===========================================================================
//  0224: Activation
// ===========================================================================

int KuyuIslandActivateBody(KuyuIslandManager *Mgr, int BodyIndex) {
	if (!Mgr) return -1;
	if (BodyIndex < 0 || BodyIndex >= Mgr->BodyCapacity) return -1;

	int IslIdx = Mgr->BodyInfo[BodyIndex].IslandIndex;
	if (IslIdx < 0) return -1;

	return KuyuIslandActivateGroup(Mgr, IslIdx);
}

int KuyuIslandActivateGroup(KuyuIslandManager *Mgr, int IslandIndex) {
	if (!Mgr) return -1;
	if (IslandIndex < 0 || IslandIndex >= Mgr->IslandCount) return -1;

	KuyuIslandGroup *Isl = &Mgr->Islands[IslandIndex];
	Isl->IsActive = 1;
	Isl->IsDeepSleep = 0;
	Isl->SleepTimer = 0;

	// Walk the linked list and clear deep sleep on each body
	int BIdx = Isl->FirstBody;
	while (BIdx >= 0) {
		BIdx = Mgr->BodyInfo[BIdx].IslandNext;
	}

	return 0;
}

// ===========================================================================
//  0225: Deactivation
// ===========================================================================

int KuyuIslandDeactivateCheck(KuyuIslandManager *Mgr,
                              const RigidBodyState *States,
                              int BodyCount) {
	if (!Mgr || !States) return -1;

	int Deactivated = 0;

	for (int I = 0; I < Mgr->IslandCount; I++) {
		KuyuIslandGroup *Isl = &Mgr->Islands[I];
		if (!Isl->IsActive) continue;  // already asleep
		if (Isl->BodyCount == 0) continue;

		// Check all bodies in this island
		int AnyMoving = 0;
		int BIdx = Isl->FirstBody;
		while (BIdx >= 0) {
			if (BIdx < BodyCount) {
				const RigidBodyState *S = &States[BIdx];
				Real LinSpeed = KannaVector3Length(&S->LinearVelocity);
				Real AngSpeed = KannaVector3Length(&S->AngularVelocity);
				if (LinSpeed > Mgr->SleepThresholdLinear ||
				    AngSpeed > Mgr->SleepThresholdAngular) {
					AnyMoving = 1;
				}
			}
			BIdx = Mgr->BodyInfo[BIdx].IslandNext;
		}

		if (AnyMoving) {
			Isl->SleepTimer = 0;
		} else {
			Isl->SleepTimer++;
			if (Isl->SleepTimer >= Mgr->SleepFramesThreshold) {
				Isl->IsActive = 0;
				Isl->IsDeepSleep = 0;
				Deactivated++;
			}
		}
	}

	return Deactivated;
}

// ===========================================================================
//  0226: Deep sleep qualification
// ===========================================================================

int KuyuIslandDeepSleepCheck(KuyuIslandManager *Mgr,
                             int IslandIndex,
                             const Real *JointImpulses, int JointCount,
                             Real Threshold) {
	if (!Mgr) return -1;
	if (IslandIndex < 0 || IslandIndex >= Mgr->IslandCount) return -1;

	KuyuIslandGroup *Isl = &Mgr->Islands[IslandIndex];
	if (Isl->IsActive) return 0;   // only sleeping islands can deep sleep
	if (Isl->BodyCount == 0) return 0;

	if (!JointImpulses || JointCount <= 0) {
		// No joints — can deep sleep if already inactive
		Isl->IsDeepSleep = 1;
		return 1;
	}

	// Check average joint impulse (proxy for joint tension)
	Real Sum = 0.0;
	for (int I = 0; I < JointCount; I++) {
		Sum += JointImpulses[I];
	}
	Real Avg = Sum / (Real)JointCount;

	if (Avg < Threshold) {
		Isl->IsDeepSleep = 1;
		return 1;
	}

	return 0;
}

// ===========================================================================
//  0227: Query connections
// ===========================================================================

int KuyuIslandGetConnections(const KuyuIslandManager *Mgr,
                             int BodyIndex,
                             const int **OutNeighbors) {
	if (!Mgr || !OutNeighbors) return -1;
	if (BodyIndex < 0 || BodyIndex >= Mgr->Graph.BodyCount) return -1;

	int Start = Mgr->Graph.RowStarts[BodyIndex];
	int End = Mgr->Graph.RowStarts[BodyIndex + 1];
	int Degree = End - Start;

	if (Degree > 0) {
		*OutNeighbors = &Mgr->Graph.Neighbors[Start];
	} else {
		*OutNeighbors = NULL;
	}
	return Degree;
}

// ===========================================================================
//  0228: Island split
// ===========================================================================

int KuyuIslandSplit(KuyuIslandManager *Mgr, int OldIslandIndex) {
	if (!Mgr) return -1;
	if (OldIslandIndex < 0 || OldIslandIndex >= Mgr->IslandCount) return -1;

	KuyuIslandGroup *OldIsl = &Mgr->Islands[OldIslandIndex];
	if (OldIsl->BodyCount <= 1) return 0;

	// Collect all body indices in this island
	int *Bodies = (int *)calloc((size_t)OldIsl->BodyCount, sizeof(int));
	if (!Bodies) return -1;
	int BC = 0;
	int BIdx = OldIsl->FirstBody;
	while (BIdx >= 0 && BC < OldIsl->BodyCount) {
		Bodies[BC++] = BIdx;
		BIdx = Mgr->BodyInfo[BIdx].IslandNext;
	}

	// Reset visited for all bodies in the old island
	for (int I = 0; I < BC; I++) {
		Mgr->BodyInfo[Bodies[I]].Visited = 0;
	}

	int NewIslands = 0;

	// BFS within this island's graph to find components
	for (int I = 0; I < BC; I++) {
		int Start = Bodies[I];
		if (Mgr->BodyInfo[Start].Visited) continue;

		// Found a new component
		int NewIslIdx;
		if (NewIslands == 0) {
			// Reuse the old island index
			NewIslIdx = OldIslandIndex;
			OldIsl->FirstBody = -1;
			OldIsl->BodyCount = 0;
		} else {
			// Need a new island slot
			if (Mgr->IslandCount >= Mgr->IslandCapacity) {
				free(Bodies);
				return NewIslands;
			}
			NewIslIdx = Mgr->IslandCount++;
			Mgr->Islands[NewIslIdx].IsActive = 1;
			Mgr->Islands[NewIslIdx].IsDeepSleep = 0;
			Mgr->Islands[NewIslIdx].SleepTimer = 0;
			Mgr->Islands[NewIslIdx].FirstBody = -1;
			Mgr->Islands[NewIslIdx].BodyCount = 0;
		}

		// BFS
		int *Q = Mgr->WorkQueue;
		int QHead = 0, QTail = 0;
		Q[QTail++] = Start;
		Mgr->BodyInfo[Start].Visited = 1;

		while (QHead < QTail) {
			int B = Q[QHead++];
			KuyuBodyIslandInfo *BI = &Mgr->BodyInfo[B];
			BI->IslandIndex = NewIslIdx;

			KuyuIslandGroup *NewIsl = &Mgr->Islands[NewIslIdx];
			if (NewIsl->FirstBody < 0) {
				NewIsl->FirstBody = B;
				BI->IslandPrev = -1;
				BI->IslandNext = -1;
			} else {
				BI->IslandPrev = -1;
				BI->IslandNext = NewIsl->FirstBody;
				Mgr->BodyInfo[NewIsl->FirstBody].IslandPrev = B;
				NewIsl->FirstBody = B;
			}
			NewIsl->BodyCount++;

			int RowStart = Mgr->Graph.RowStarts[B];
			int RowEnd = Mgr->Graph.RowStarts[B + 1];
			for (int J = RowStart; J < RowEnd; J++) {
				int Nbr = Mgr->Graph.Neighbors[J];
				if (!Mgr->BodyInfo[Nbr].Visited) {
					Mgr->BodyInfo[Nbr].Visited = 1;
					Q[QTail++] = Nbr;
				}
			}
		}

		NewIslands++;
	}

	free(Bodies);
	return NewIslands - 1;  // -1 because one is the original
}

// ===========================================================================
//  0229: Island merge
// ===========================================================================

int KuyuIslandMerge(KuyuIslandManager *Mgr,
                    int IslandAIndex, int IslandBIndex) {
	if (!Mgr) return -1;
	if (IslandAIndex < 0 || IslandAIndex >= Mgr->IslandCount ||
	    IslandBIndex < 0 || IslandBIndex >= Mgr->IslandCount) return -1;
	if (IslandAIndex == IslandBIndex) return 0;

	KuyuIslandGroup *A = &Mgr->Islands[IslandAIndex];
	KuyuIslandGroup *B = &Mgr->Islands[IslandBIndex];
	if (B->BodyCount == 0) return 0;

	// Find tail of A or head of B
	if (A->FirstBody < 0) {
		// A is empty — just copy B's bodies
		A->FirstBody = B->FirstBody;
		A->BodyCount = B->BodyCount;
		// Update island index for B's bodies
		int BIdx = B->FirstBody;
		while (BIdx >= 0) {
			Mgr->BodyInfo[BIdx].IslandIndex = IslandAIndex;
			BIdx = Mgr->BodyInfo[BIdx].IslandNext;
		}
	} else {
		// Find tail of A
		int Tail = A->FirstBody;
		while (Mgr->BodyInfo[Tail].IslandNext >= 0) {
			Tail = Mgr->BodyInfo[Tail].IslandNext;
		}

		// Link B's head after A's tail
		Mgr->BodyInfo[Tail].IslandNext = B->FirstBody;
		Mgr->BodyInfo[B->FirstBody].IslandPrev = Tail;

		// Update island index for B's bodies
		int BIdx = B->FirstBody;
		while (BIdx >= 0) {
			Mgr->BodyInfo[BIdx].IslandIndex = IslandAIndex;
			BIdx = Mgr->BodyInfo[BIdx].IslandNext;
		}

		A->BodyCount += B->BodyCount;
	}

	// Activate merged island
	A->IsActive = 1;
	A->SleepTimer = 0;

	// Clear B
	B->FirstBody = -1;
	B->BodyCount = 0;
	B->IsActive = 0;

	return 0;
}

// ===========================================================================
//  0230: Load balancing
// ===========================================================================

int KuyuIslandLoadBalance(KuyuIslandManager *Mgr,
                          int *IslandToThread,
                          int ThreadCount) {
	if (!Mgr || !IslandToThread || ThreadCount <= 0) return -1;

	int IC = Mgr->IslandCount;
	if (IC == 0) return 0;

	// Initialise assignment to -1
	memset(IslandToThread, -1, (size_t)IC * sizeof(int));

	// Compute total bodies
	int TotalBodies = 0;
	for (int I = 0; I < IC; I++)
		TotalBodies += Mgr->Islands[I].BodyCount;

	if (TotalBodies == 0) {
		for (int I = 0; I < IC; I++)
			IslandToThread[I] = 0;
		return IC;
	}

	int *ThreadLoad = (int *)calloc((size_t)ThreadCount, sizeof(int));
	if (!ThreadLoad) return -1;

	int PerThreadLimit = (TotalBodies + ThreadCount - 1) / ThreadCount;

	// First pass: big islands (size > per-thread limit)
	for (int I = 0; I < IC; I++) {
		int BC = Mgr->Islands[I].BodyCount;
		if (BC <= PerThreadLimit) continue;

		int MinT = 0;
		for (int T = 1; T < ThreadCount; T++) {
			if (ThreadLoad[T] < ThreadLoad[MinT])
				MinT = T;
		}
		IslandToThread[I] = MinT;
		ThreadLoad[MinT] += BC;
	}

	// Second pass: small islands
	for (int I = 0; I < IC; I++) {
		if (IslandToThread[I] >= 0) continue;

		int MinT = 0;
		for (int T = 1; T < ThreadCount; T++) {
			if (ThreadLoad[T] < ThreadLoad[MinT])
				MinT = T;
		}
		IslandToThread[I] = MinT;
		ThreadLoad[MinT] += Mgr->Islands[I].BodyCount;
	}

	free(ThreadLoad);
	return IC;
}

// ===========================================================================
//  Convenience: full rebuild
// ===========================================================================

int KuyuIslandRebuild(KuyuIslandManager *Mgr,
                      const int *Pairs, int PairCount) {
	if (!Mgr) return -1;

	// Clear existing island state
	for (int I = 0; I < Mgr->IslandCapacity; I++) {
		Mgr->Islands[I].FirstBody = -1;
		Mgr->Islands[I].BodyCount = 0;
		Mgr->Islands[I].IsActive = 1;
		Mgr->Islands[I].IsDeepSleep = 0;
		Mgr->Islands[I].SleepTimer = 0;
	}
	Mgr->IslandCount = 0;

	int R = KuyuIslandBuildGraph(Mgr, Pairs, PairCount);
	if (R != 0) return R;

	return KuyuIslandFindComponents(Mgr);
}
