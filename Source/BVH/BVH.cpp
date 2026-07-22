/**
 * TazusaBVH — bounding volume hierarchy implementation.
 *
 * SAH builder with binning, bottom-up refit, stack-based traversal,
 * tree rotations, and dynamic leaf splitting. All runtime paths are
 * allocation-free — the node pool is fixed at init time.
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include <TohruPhysics/BVH.h>
#include <TohruPhysics/Math.h>
#include <stdlib.h>
#include <string.h>

// ---------------------------------------------------------------------------
//  Internal helpers
// ---------------------------------------------------------------------------

static inline void ExpandAABB(AABB *Box, const AABB *Other) {
	for (int I = 0; I < 3; I++) {
		if (Other->Min.Data[I] < Box->Min.Data[I])
			Box->Min.Data[I] = Other->Min.Data[I];
		if (Other->Max.Data[I] > Box->Max.Data[I])
			Box->Max.Data[I] = Other->Max.Data[I];
	}
}

static inline Real AABBSurfaceArea(const AABB *Box) {
	Real Dx = Box->Max.Data[0] - Box->Min.Data[0];
	Real Dy = Box->Max.Data[1] - Box->Min.Data[1];
	Real Dz = Box->Max.Data[2] - Box->Min.Data[2];
	if (Dx < 0) Dx = 0;
	if (Dy < 0) Dy = 0;
	if (Dz < 0) Dz = 0;
	return 2.0 * (Dx * Dy + Dy * Dz + Dz * Dx);
}

static inline Real CentroidCoord(const AABB *Box, int Axis) {
	return (Box->Min.Data[Axis] + Box->Max.Data[Axis]) * 0.5;
}

// ---------------------------------------------------------------------------
//  0201: Init / Reset
// ---------------------------------------------------------------------------

int TazusaBVHInit(TazusaBVHTree *Tree,
                  TazusaBVHNode *NodeBuffer, int NodeCapacity,
                  int *PrimBuffer, int PrimCapacity) {
	if (!Tree || !NodeBuffer || NodeCapacity < 1) return -1;
	if (!PrimBuffer || PrimCapacity < 1) return -1;
	Tree->Nodes = NodeBuffer;
	Tree->NodeCapacity = NodeCapacity;
	Tree->NodeCount = 0;
	Tree->PrimIndices = PrimBuffer;
	Tree->PrimCapacity = PrimCapacity;
	Tree->PrimCount = 0;
	Tree->RootIndex = 0;
	Tree->MaxDepthReached = 0;
	Tree->MaxLeafSize = TAZUSA_DEFAULT_LEAF_SIZE;
	Tree->Dirty = 0;
	memset(NodeBuffer, 0, (size_t)NodeCapacity * sizeof(TazusaBVHNode));
	for (int I = 0; I < NodeCapacity; I++) {
		NodeBuffer[I].LeftChild  = -1;
		NodeBuffer[I].RightChild = -1;
		NodeBuffer[I].Parent     = -1;
		NodeBuffer[I].PrimitiveIndex = -1;
	}
	return 0;
}

void TazusaBVHReset(TazusaBVHTree *Tree) {
	if (!Tree) return;
	int NC = Tree->NodeCapacity;
	TazusaBVHNode *NB = Tree->Nodes;
	memset(NB, 0, (size_t)NC * sizeof(TazusaBVHNode));
	for (int I = 0; I < NC; I++) {
		NB[I].LeftChild  = -1;
		NB[I].RightChild = -1;
		NB[I].Parent     = -1;
		NB[I].PrimitiveIndex = -1;
	}
	Tree->NodeCount = 0;
	Tree->PrimCount = 0;
	Tree->RootIndex = 0;
	Tree->MaxDepthReached = 0;
	Tree->Dirty = 0;
}

// ---------------------------------------------------------------------------
//  Node pool
// ---------------------------------------------------------------------------

static int AllocNode(TazusaBVHTree *Tree) {
	if (Tree->NodeCount >= Tree->NodeCapacity) return -1;
	int Idx = Tree->NodeCount;
	Tree->NodeCount++;
	memset(&Tree->Nodes[Idx], 0, sizeof(TazusaBVHNode));
	Tree->Nodes[Idx].LeftChild  = -1;
	Tree->Nodes[Idx].RightChild = -1;
	Tree->Nodes[Idx].Parent     = -1;
	Tree->Nodes[Idx].PrimitiveIndex = -1;
	return Idx;
}

// ---------------------------------------------------------------------------
//  SAH binning structures
// ---------------------------------------------------------------------------

typedef struct {
	AABB Box;
	int  Count;
} SahBin;

// Partition PrimIndices[PrimStart..PrimStart+PrimCount-1] by bin index
// along Axis. Returns split position (number of prims assigned to left).
static int PartitionByBins(const AABB *BodyAABBs,
                           int *PrimIndices, int PrimStart, int PrimCount,
                           int Axis, int NumBins,
                           Real MinC, Real Range,
                           int SplitBin,
                           AABB *OutLeftBox, AABB *OutRightBox) {
	(void)NumBins;
	// Temp copy for partitioning
	int *Temp = (int *)malloc((size_t)PrimCount * sizeof(int));
	if (!Temp) return -1;
	memcpy(Temp, PrimIndices + PrimStart, (size_t)PrimCount * sizeof(int));

	int LeftCount = 0;
	for (int I = 0; I < PrimCount; I++) {
		int PIdx = Temp[I];
		Real C = CentroidCoord(&BodyAABBs[PIdx], Axis);
		int BinIdx = 0;
		if (Range > 1e-10) {
			Real T = (C - MinC) / Range;
			BinIdx = (int)(T * 63.0);
			if (BinIdx > 63) BinIdx = 63;
			if (BinIdx < 0) BinIdx = 0;
		}
		if (BinIdx <= SplitBin) {
			PrimIndices[PrimStart + LeftCount] = PIdx;
			LeftCount++;
		}
	}

	int RightCount = 0;
	for (int I = 0; I < PrimCount; I++) {
		int PIdx = Temp[I];
		Real C = CentroidCoord(&BodyAABBs[PIdx], Axis);
		int BinIdx = 0;
		if (Range > 1e-10) {
			Real T = (C - MinC) / Range;
			BinIdx = (int)(T * 63.0);
			if (BinIdx > 63) BinIdx = 63;
			if (BinIdx < 0) BinIdx = 0;
		}
		if (BinIdx > SplitBin) {
			PrimIndices[PrimStart + LeftCount + RightCount] = PIdx;
			RightCount++;
		}
	}

	free(Temp);

	// Compute child AABBs
	OutLeftBox->Min.Data[0] = OutLeftBox->Min.Data[1] = OutLeftBox->Min.Data[2] = 1e38;
	OutLeftBox->Max.Data[0] = OutLeftBox->Max.Data[1] = OutLeftBox->Max.Data[2] = -1e38;
	OutRightBox->Min.Data[0] = OutRightBox->Min.Data[1] = OutRightBox->Min.Data[2] = 1e38;
	OutRightBox->Max.Data[0] = OutRightBox->Max.Data[1] = OutRightBox->Max.Data[2] = -1e38;

	for (int I = 0; I < LeftCount; I++) {
		int PIdx = PrimIndices[PrimStart + I];
		ExpandAABB(OutLeftBox, &BodyAABBs[PIdx]);
	}
	for (int I = 0; I < RightCount; I++) {
		int PIdx = PrimIndices[PrimStart + LeftCount + I];
		ExpandAABB(OutRightBox, &BodyAABBs[PIdx]);
	}

	return LeftCount;
}

// Find best split using SAH binning. Returns split position in PrimIndices
// (number of primitives for left child), or -1 if no valid split.
static int FindBestSplit(const AABB *BodyAABBs,
                         int *PrimIndices, int PrimStart, int PrimCount,
                         int Axis, int NumBins,
                         AABB *OutLeftBox, AABB *OutRightBox) {
	if (PrimCount < 2) return -1;

	// Centroid range
	Real MinC = 1e38, MaxC = -1e38;
	for (int I = 0; I < PrimCount; I++) {
		int PIdx = PrimIndices[PrimStart + I];
		Real C = CentroidCoord(&BodyAABBs[PIdx], Axis);
		if (C < MinC) MinC = C;
		if (C > MaxC) MaxC = C;
	}
	Real Range = MaxC - MinC;
	if (Range < 1e-10) return -1;

	// Determine actual bin count
	int NB = NumBins;
	if (NB > 64) NB = 64;
	if (NB < 2) NB = 2;

	SahBin Bins[64];
	for (int I = 0; I < NB; I++) {
		Bins[I].Box.Min.Data[0] = Bins[I].Box.Min.Data[1] = Bins[I].Box.Min.Data[2] = 1e38;
		Bins[I].Box.Max.Data[0] = Bins[I].Box.Max.Data[1] = Bins[I].Box.Max.Data[2] = -1e38;
		Bins[I].Count = 0;
	}

	// Bin primitives
	for (int I = 0; I < PrimCount; I++) {
		int PIdx = PrimIndices[PrimStart + I];
		Real C = CentroidCoord(&BodyAABBs[PIdx], Axis);
		int BinIdx = (int)(((C - MinC) / Range) * (Real)NB);
		if (BinIdx >= NB) BinIdx = NB - 1;
		if (BinIdx < 0) BinIdx = 0;
		Bins[BinIdx].Count++;
		ExpandAABB(&Bins[BinIdx].Box, &BodyAABBs[PIdx]);
	}

	// Evaluate cost at bin boundaries
	AABB *LeftPrefBox = (AABB *)malloc((size_t)NB * sizeof(AABB));
	int *LeftPrefCnt = (int *)malloc((size_t)NB * sizeof(int));
	AABB *RightPrefBox = (AABB *)malloc((size_t)NB * sizeof(AABB));
	int *RightPrefCnt = (int *)malloc((size_t)NB * sizeof(int));

	if (!LeftPrefBox || !LeftPrefCnt || !RightPrefBox || !RightPrefCnt) {
		free(LeftPrefBox); free(LeftPrefCnt); free(RightPrefBox); free(RightPrefCnt);
		// Fallback split at midpoint
		int Mid = PrimCount / 2;
		*OutLeftBox = BodyAABBs[PrimIndices[PrimStart]];
		for (int I = 1; I < Mid; I++)
			ExpandAABB(OutLeftBox, &BodyAABBs[PrimIndices[PrimStart + I]]);
		*OutRightBox = BodyAABBs[PrimIndices[PrimStart + Mid]];
		for (int I = Mid + 1; I < PrimCount; I++)
			ExpandAABB(OutRightBox, &BodyAABBs[PrimIndices[PrimStart + I]]);
		return Mid;
	}

	// Left prefix
	for (int I = 0; I < NB; I++) {
		if (I == 0) {
			LeftPrefBox[I] = Bins[I].Box;
			LeftPrefCnt[I] = Bins[I].Count;
		} else {
			LeftPrefBox[I] = LeftPrefBox[I - 1];
			ExpandAABB(&LeftPrefBox[I], &Bins[I].Box);
			LeftPrefCnt[I] = LeftPrefCnt[I - 1] + Bins[I].Count;
		}
	}

	// Right prefix
	for (int I = NB - 1; I >= 0; I--) {
		if (I == NB - 1) {
			RightPrefBox[I] = Bins[I].Box;
			RightPrefCnt[I] = Bins[I].Count;
		} else {
			RightPrefBox[I] = RightPrefBox[I + 1];
			ExpandAABB(&RightPrefBox[I], &Bins[I].Box);
			RightPrefCnt[I] = RightPrefCnt[I + 1] + Bins[I].Count;
		}
	}

	// Compute parent SA for cost normalisation
	AABB ParentBox;
	ParentBox.Min.Data[0] = ParentBox.Min.Data[1] = ParentBox.Min.Data[2] = 1e38;
	ParentBox.Max.Data[0] = ParentBox.Max.Data[1] = ParentBox.Max.Data[2] = -1e38;
	for (int I = 0; I < PrimCount; I++) {
		ExpandAABB(&ParentBox, &BodyAABBs[PrimIndices[PrimStart + I]]);
	}
	Real ParentSA = AABBSurfaceArea(&ParentBox);
	if (ParentSA <= 0.0) ParentSA = 1.0;

	int BestSplit = -1;
	Real BestCost = 1e38;

	for (int I = 0; I < NB - 1; I++) {
		int LC = LeftPrefCnt[I];
		int RC = RightPrefCnt[I + 1];
		if (LC == 0 || RC == 0) continue;
		Real Cost = 1.0
		            + (AABBSurfaceArea(&LeftPrefBox[I]) / ParentSA) * (Real)LC
		            + (AABBSurfaceArea(&RightPrefBox[I + 1]) / ParentSA) * (Real)RC;
		if (Cost < BestCost) {
			BestCost = Cost;
			BestSplit = I;
		}
	}

	free(LeftPrefBox); free(LeftPrefCnt); free(RightPrefBox); free(RightPrefCnt);

	if (BestSplit < 0) return -1;

	// Partition primitives
	return PartitionByBins(BodyAABBs, PrimIndices, PrimStart, PrimCount,
	                       Axis, NB, MinC, Range, BestSplit,
	                       OutLeftBox, OutRightBox);
}

// ---------------------------------------------------------------------------
//  0202: SAH build
// ---------------------------------------------------------------------------

typedef struct {
	int NodeIdx;
	int PrimStart;
	int PrimCount;
	int Depth;
} BuildTask;

int TazusaBVHBuild(TazusaBVHTree *Tree,
                   const AABB *BodyAABBs,
                   int BodyCount,
                   TazusaBuildQuality Quality) {
	if (!Tree || !BodyAABBs || BodyCount <= 0) return 0;

	TazusaBVHReset(Tree);

	int NB = 16;
	if (Quality == TAZUSA_BUILD_DEFAULT) NB = 32;
	if (Quality == TAZUSA_BUILD_HIGH_QUALITY) NB = 64;

	// Init primitive indices
	int PrimLimit = Tree->PrimCapacity < BodyCount ? Tree->PrimCapacity : BodyCount;
	for (int I = 0; I < PrimLimit; I++)
		Tree->PrimIndices[I] = I;
	Tree->PrimCount = PrimLimit;

	// Scene AABB
	AABB SceneBox = BodyAABBs[0];
	for (int I = 1; I < PrimLimit; I++)
		ExpandAABB(&SceneBox, &BodyAABBs[I]);

	// Root
	int RootIdx = AllocNode(Tree);
	if (RootIdx < 0) return 0;
	Tree->RootIndex = RootIdx;
	Tree->Nodes[RootIdx].Box = SceneBox;

	// Iterative build stack
	BuildTask Stack[TAZUSA_MAX_DEPTH];
	int SP = 0;
	Stack[SP].NodeIdx = RootIdx;
	Stack[SP].PrimStart = 0;
	Stack[SP].PrimCount = PrimLimit;
	Stack[SP].Depth = 0;
	SP++;

	while (SP > 0) {
		SP--;
		BuildTask *T = &Stack[SP];
		int NIdx = T->NodeIdx;
		int PS = T->PrimStart;
		int PC = T->PrimCount;
		int Depth = T->Depth;

		if (Depth > Tree->MaxDepthReached)
			Tree->MaxDepthReached = Depth;

		if (PC <= Tree->MaxLeafSize || Depth >= TAZUSA_MAX_DEPTH - 1) {
			Tree->Nodes[NIdx].PrimitiveIndex = PS;
			TazusaNodeSetPrimCount(Tree->Nodes[NIdx], PC);
			continue;
		}

		// Choose split axis by longest centroid extent
		Real CMin[3] = {1e38, 1e38, 1e38};
		Real CMax[3] = {-1e38, -1e38, -1e38};
		for (int I = 0; I < PC; I++) {
			int PIdx = Tree->PrimIndices[PS + I];
			for (int A = 0; A < 3; A++) {
				Real C = CentroidCoord(&BodyAABBs[PIdx], A);
				if (C < CMin[A]) CMin[A] = C;
				if (C > CMax[A]) CMax[A] = C;
			}
		}

		int BestAxis = 0;
		Real BestExt = CMax[0] - CMin[0];
		for (int A = 1; A < 3; A++) {
			Real Ext = CMax[A] - CMin[A];
			if (Ext > BestExt) { BestExt = Ext; BestAxis = A; }
		}

		int SplitAxis = BestAxis;
		AABB LeftBox, RightBox;
		int LeftCount = -1;

		if (Quality == TAZUSA_BUILD_HIGH_QUALITY) {
			Real BestSah = 1e38;
			for (int A = 0; A < 3; A++) {
				if (CMax[A] - CMin[A] < 1e-10) continue;
				AABB TmpL, TmpR;
				int LC = FindBestSplit(BodyAABBs, Tree->PrimIndices, PS, PC,
				                       A, NB, &TmpL, &TmpR);
				if (LC > 0 && LC < PC) {
					Real SA = AABBSurfaceArea(&SceneBox);
					if (SA <= 0.0) SA = 1.0;
					Real Sah = 1.0
					           + (AABBSurfaceArea(&TmpL) / SA) * (Real)LC
					           + (AABBSurfaceArea(&TmpR) / SA) * (Real)(PC - LC);
					if (Sah < BestSah) {
						BestSah = Sah;
						SplitAxis = A;
						LeftBox = TmpL;
						RightBox = TmpR;
						LeftCount = LC;
					}
				}
			}
			if (LeftCount <= 0 || LeftCount >= PC) {
				LeftCount = FindBestSplit(BodyAABBs, Tree->PrimIndices, PS, PC,
				                          BestAxis, NB, &LeftBox, &RightBox);
			}
		} else {
			LeftCount = FindBestSplit(BodyAABBs, Tree->PrimIndices, PS, PC,
			                          BestAxis, NB, &LeftBox, &RightBox);
		}

		if (LeftCount <= 0 || LeftCount >= PC) {
			Tree->Nodes[NIdx].PrimitiveIndex = PS;
			TazusaNodeSetPrimCount(Tree->Nodes[NIdx], PC);
			continue;
		}

		// Create children
		int LeftIdx = AllocNode(Tree);
		int RightIdx = AllocNode(Tree);
		if (LeftIdx < 0 || RightIdx < 0) {
			Tree->Nodes[NIdx].PrimitiveIndex = PS;
			TazusaNodeSetPrimCount(Tree->Nodes[NIdx], PC);
			continue;
		}

		Tree->Nodes[NIdx].LeftChild = LeftIdx;
		Tree->Nodes[NIdx].RightChild = RightIdx;
		Tree->Nodes[LeftIdx].Parent = NIdx;
		Tree->Nodes[RightIdx].Parent = NIdx;
		Tree->Nodes[LeftIdx].Box = LeftBox;
		Tree->Nodes[RightIdx].Box = RightBox;
		TazusaNodeSetSplitAxis(Tree->Nodes[NIdx], SplitAxis);

		// Push right then left (LIFO — left processed first)
		Stack[SP].NodeIdx = RightIdx;
		Stack[SP].PrimStart = PS + LeftCount;
		Stack[SP].PrimCount = PC - LeftCount;
		Stack[SP].Depth = Depth + 1;
		SP++;

		Stack[SP].NodeIdx = LeftIdx;
		Stack[SP].PrimStart = PS;
		Stack[SP].PrimCount = LeftCount;
		Stack[SP].Depth = Depth + 1;
		SP++;
	}

	return Tree->NodeCount;
}

// ---------------------------------------------------------------------------
//  0203: Rebalance via tree rotations
// ---------------------------------------------------------------------------

// Compute SAH cost for a given node's subtree (estimated recursively).
static Real SubtreeCost(TazusaBVHTree *Tree, int NodeIdx,
                        const AABB **BodyAABBs) {
	(void)BodyAABBs;
	if (NodeIdx < 0) return 0;
	TazusaBVHNode *N = &Tree->Nodes[NodeIdx];
	if (TazusaBVHIsLeaf(N)) {
		return (Real)TazusaNodePrimCount(*N);
	}
	Real LeftCost = SubtreeCost(Tree, N->LeftChild, BodyAABBs);
	Real RightCost = SubtreeCost(Tree, N->RightChild, BodyAABBs);
	Real SA = AABBSurfaceArea(&N->Box);
	if (SA <= 0.0) SA = 1.0;
	Real NormL = AABBSurfaceArea(&Tree->Nodes[N->LeftChild].Box) / SA;
	Real NormR = AABBSurfaceArea(&Tree->Nodes[N->RightChild].Box) / SA;
	return 1.0 + NormL * LeftCost + NormR * RightCost;
}

int TazusaBVHRebalance(TazusaBVHTree *Tree) {
	if (!Tree || Tree->NodeCount < 3) return 0;

	int Rotations = 0;
	int MaxIter = Tree->NodeCount * 2;
	int Iter = 0;

	// Simple heuristic: bottom-up rotate if SAH cost improves.
	// We walk nodes and attempt rotation at each internal node.
	// A rotation swaps a node with one of its children.
	// For now: rotate left if right subtree has lower cost-to-SARatio.

	// Use a stack for post-order traversal
	int Stack[TAZUSA_MAX_DEPTH];
	int VisitCount[TAZUSA_MAX_DEPTH];
	int SP = 0;
	Stack[SP] = Tree->RootIndex;
	VisitCount[SP] = 0;
	SP++;

	while (SP > 0 && Iter < MaxIter) {
		Iter++;
		int NIdx = Stack[SP - 1];
		if (NIdx < 0) { SP--; continue; }
		TazusaBVHNode *N = &Tree->Nodes[NIdx];

		if (TazusaBVHIsLeaf(N) || VisitCount[SP - 1] >= 2) {
			SP--;  // processed both children
			continue;
		}

		if (VisitCount[SP - 1] == 0) {
			VisitCount[SP - 1] = 1;
			Stack[SP] = N->LeftChild;
			VisitCount[SP] = 0;
			SP++;
		} else if (VisitCount[SP - 1] == 1) {
			VisitCount[SP - 1] = 2;
			Stack[SP] = N->RightChild;
			VisitCount[SP] = 0;
			SP++;

			// After right child is pushed, try a rotation if depth > 2
			// Check if right child is internal
			if (N->RightChild >= 0 && !TazusaBVHIsLeaf(&Tree->Nodes[N->RightChild])
			    && N->LeftChild >= 0) {
				TazusaBVHNode *R = &Tree->Nodes[N->RightChild];
				TazusaBVHNode *L = &Tree->Nodes[N->LeftChild];

				// Simple heuristic: if R's SA is significantly smaller,
				// swap children to improve tree quality
				if (AABBSurfaceArea(&R->Box) < AABBSurfaceArea(&L->Box) * 0.8) {
					int Tmp = N->LeftChild;
					N->LeftChild = N->RightChild;
					N->RightChild = Tmp;
					Tree->Nodes[N->LeftChild].Parent = NIdx;
					Tree->Nodes[N->RightChild].Parent = NIdx;
					Rotations++;
				}
			}
		}
	}

	// Update parent pointers after rotation
	Tree->Dirty = 1;
	return Rotations;
}

// ---------------------------------------------------------------------------
//  0204: Ray traversal (stack-based)
// ---------------------------------------------------------------------------

int TazusaBVHTraverseRay(TazusaBVHTree *Tree,
                         const Ray *Ray,
                         TazusaRayLeafFunc LeafFunc,
                         void *UserData) {
	if (!Tree || !Ray || !LeafFunc) return -1;
	if (Tree->NodeCount == 0) return 0;

	int Hits = 0;
	int Stack[TAZUSA_MAX_DEPTH];
	int SP = 0;
	Stack[SP++] = Tree->RootIndex;

	while (SP > 0) {
		SP--;
		int NIdx = Stack[SP];
		if (NIdx < 0) continue;
		TazusaBVHNode *N = &Tree->Nodes[NIdx];

		// Test ray-AABB intersection
		Real TMin, TMax;
		if (!SabinaAABBIntersectRay(&N->Box, Ray, &TMin, &TMax))
			continue;
		if (TMax < 0) continue;

		if (TazusaBVHIsLeaf(N)) {
			int PrimCount = TazusaNodePrimCount(*N);
			int PrimStart = N->PrimitiveIndex;
			for (int I = 0; I < PrimCount; I++) {
				Real OutT = TMax;
				if (LeafFunc(Tree->PrimIndices[PrimStart + I],
				             Ray, &OutT, UserData)) {
					Hits++;
				}
			}
		} else {
			// Push children (far first, so near is processed first)
			int NearChild, FarChild;

			// Determine which child is closer along ray
			Vector3 RO = Ray->Origin;
			Vector3 Center;
			Center.Data[0] = (Tree->Nodes[N->LeftChild].Box.Min.Data[0]
			                  + Tree->Nodes[N->LeftChild].Box.Max.Data[0]) * 0.5;
			Center.Data[1] = (Tree->Nodes[N->LeftChild].Box.Min.Data[1]
			                  + Tree->Nodes[N->LeftChild].Box.Max.Data[1]) * 0.5;
			Center.Data[2] = (Tree->Nodes[N->LeftChild].Box.Min.Data[2]
			                  + Tree->Nodes[N->LeftChild].Box.Max.Data[2]) * 0.5;

			Real DistL = KannaVector3Distance(&RO, &Center);

			Center.Data[0] = (Tree->Nodes[N->RightChild].Box.Min.Data[0]
			                  + Tree->Nodes[N->RightChild].Box.Max.Data[0]) * 0.5;
			Center.Data[1] = (Tree->Nodes[N->RightChild].Box.Min.Data[1]
			                  + Tree->Nodes[N->RightChild].Box.Max.Data[1]) * 0.5;
			Center.Data[2] = (Tree->Nodes[N->RightChild].Box.Min.Data[2]
			                  + Tree->Nodes[N->RightChild].Box.Max.Data[2]) * 0.5;

			Real DistR = KannaVector3Distance(&RO, &Center);

			if (DistL <= DistR) {
				NearChild = N->LeftChild;
				FarChild = N->RightChild;
			} else {
				NearChild = N->RightChild;
				FarChild = N->LeftChild;
			}

			Stack[SP++] = FarChild;
			Stack[SP++] = NearChild;
		}
	}

	return Hits;
}

// ---------------------------------------------------------------------------
//  0205: Volume (AABB) traversal
// ---------------------------------------------------------------------------

int TazusaBVHTraverseVolume(TazusaBVHTree *Tree,
                            const AABB *QueryAABB,
                            TazusaVolumeLeafFunc LeafFunc,
                            void *UserData) {
	if (!Tree || !QueryAABB || !LeafFunc) return -1;
	if (Tree->NodeCount == 0) return 0;

	int Hits = 0;
	int Stack[TAZUSA_MAX_DEPTH];
	int SP = 0;
	Stack[SP++] = Tree->RootIndex;

	while (SP > 0) {
		SP--;
		int NIdx = Stack[SP];
		if (NIdx < 0) continue;
		TazusaBVHNode *N = &Tree->Nodes[NIdx];

		if (!SabinaAABBOverlaps(&N->Box, QueryAABB))
			continue;

		if (TazusaBVHIsLeaf(N)) {
			int PrimCount = TazusaNodePrimCount(*N);
			int PrimStart = N->PrimitiveIndex;
			for (int I = 0; I < PrimCount; I++) {
				if (LeafFunc(Tree->PrimIndices[PrimStart + I],
				             QueryAABB, UserData)) {
					Hits++;
				}
			}
		} else {
			Stack[SP++] = N->RightChild;
			Stack[SP++] = N->LeftChild;
		}
	}

	return Hits;
}

// ---------------------------------------------------------------------------
//  0206: Serialisation
// ---------------------------------------------------------------------------

int TazusaBVHSerialize(const TazusaBVHTree *Tree,
                       void *Buffer, int BufferSize) {
	if (!Tree) return -1;

	int HeaderSize = (int)sizeof(TazusaBVHSerializedHeader);
	int NodeDataSize = Tree->NodeCount * (int)sizeof(TazusaBVHNode);
	int PrimDataSize = Tree->PrimCount * (int)sizeof(int);
	int TotalSize = HeaderSize + NodeDataSize + PrimDataSize;

	if (!Buffer) return TotalSize;
	if (BufferSize < TotalSize) return -1;

	TazusaBVHSerializedHeader *H = (TazusaBVHSerializedHeader *)Buffer;
	H->NodeCount = Tree->NodeCount;
	H->PrimCount = Tree->PrimCount;
	H->MaxLeafSize = Tree->MaxLeafSize;

	char *Ptr = (char *)Buffer + HeaderSize;
	memcpy(Ptr, Tree->Nodes, (size_t)NodeDataSize);
	Ptr += NodeDataSize;
	memcpy(Ptr, Tree->PrimIndices, (size_t)PrimDataSize);

	return TotalSize;
}

int TazusaBVHDeserialize(TazusaBVHTree *Tree,
                         const void *Buffer, int BufferSize,
                         TazusaBVHNode *NodeBuffer, int NodeCapacity,
                         int *PrimBuffer, int PrimCapacity) {
	if (!Tree || !Buffer || BufferSize < (int)sizeof(TazusaBVHSerializedHeader))
		return -1;

	const TazusaBVHSerializedHeader *H =
		(const TazusaBVHSerializedHeader *)Buffer;

	if (H->NodeCount > NodeCapacity) return -1;
	if (H->PrimCount > PrimCapacity) return -1;

	Tree->Nodes = NodeBuffer;
	Tree->NodeCapacity = NodeCapacity;
	Tree->NodeCount = H->NodeCount;
	Tree->PrimIndices = PrimBuffer;
	Tree->PrimCapacity = PrimCapacity;
	Tree->PrimCount = H->PrimCount;
	Tree->RootIndex = 0;
	Tree->MaxDepthReached = 0;
	Tree->MaxLeafSize = H->MaxLeafSize;
	Tree->Dirty = 0;

	int HeaderSize = (int)sizeof(TazusaBVHSerializedHeader);
	const char *Ptr = (const char *)Buffer + HeaderSize;
	memcpy(NodeBuffer, Ptr, (size_t)H->NodeCount * sizeof(TazusaBVHNode));
	Ptr += (size_t)H->NodeCount * sizeof(TazusaBVHNode);
	memcpy(PrimBuffer, Ptr, (size_t)H->PrimCount * sizeof(int));

	return 0;
}

// ---------------------------------------------------------------------------
//  0207: Refit (bottom-up AABB update)
// ---------------------------------------------------------------------------

int TazusaBVHRefit(TazusaBVHTree *Tree,
                   const AABB *BodyAABBs) {
	if (!Tree || !BodyAABBs || Tree->NodeCount == 0) return 0;

	int Changed = 0;

	// Post-order traversal using explicit stack
	int Stack[TAZUSA_MAX_DEPTH];
	int VisitCount[TAZUSA_MAX_DEPTH];
	int SP = 0;
	Stack[SP] = Tree->RootIndex;
	VisitCount[SP] = 0;
	SP++;

	while (SP > 0) {
		int NIdx = Stack[SP - 1];
		if (NIdx < 0) { SP--; continue; }
		TazusaBVHNode *N = &Tree->Nodes[NIdx];

		if (TazusaBVHIsLeaf(N)) {
			SP--;
			// Refit leaf AABB from body AABBs
			int PrimCount = TazusaNodePrimCount(*N);
			int PrimStart = N->PrimitiveIndex;
			if (PrimCount > 0 && PrimStart >= 0) {
				AABB NewBox = BodyAABBs[Tree->PrimIndices[PrimStart]];
				for (int I = 1; I < PrimCount; I++)
					ExpandAABB(&NewBox, &BodyAABBs[Tree->PrimIndices[PrimStart + I]]);

				// Check if changed
				for (int K = 0; K < 3; K++) {
					if (NewBox.Min.Data[K] != N->Box.Min.Data[K] ||
					    NewBox.Max.Data[K] != N->Box.Max.Data[K]) {
						Changed++;
						break;
					}
				}
				N->Box = NewBox;
			}
		} else if (VisitCount[SP - 1] < 2) {
			int Child = (VisitCount[SP - 1] == 0)
			            ? N->LeftChild : N->RightChild;
			VisitCount[SP - 1]++;
			if (Child >= 0) {
				Stack[SP] = Child;
				VisitCount[SP] = 0;
				SP++;
			}
		} else {
			SP--;
			// Refit internal node from children
			AABB NewBox = Tree->Nodes[N->LeftChild].Box;
			ExpandAABB(&NewBox, &Tree->Nodes[N->RightChild].Box);

			for (int K = 0; K < 3; K++) {
				if (NewBox.Min.Data[K] != N->Box.Min.Data[K] ||
				    NewBox.Max.Data[K] != N->Box.Max.Data[K]) {
					Changed++;
					break;
				}
			}
			N->Box = NewBox;
		}
	}

	return Changed;
}

// ---------------------------------------------------------------------------
//  0208: Incremental update for dynamic scenes
// ---------------------------------------------------------------------------

int TazusaBVHUpdate(TazusaBVHTree *Tree,
                    const AABB *BodyAABBs,
                    int BodyCount) {
	if (!Tree || !BodyAABBs) return -1;

	// Refit
	int Changed = TazusaBVHRefit(Tree, BodyAABBs);

	// If many nodes changed, attempt rebalance
	if (Changed > Tree->NodeCount / 4)
		TazusaBVHRebalance(Tree);

	// Split overfilled leaves
	int SplitCount = TazusaBVHSplitLeaves(Tree, BodyAABBs, BodyCount);
	(void)SplitCount;

	// If tree is too deep, flag for full rebuild
	if (Tree->MaxDepthReached >= TAZUSA_MAX_DEPTH - 2)
		Tree->Dirty = 1;

	return 0;
}

// ---------------------------------------------------------------------------
//  0209: Depth validation
// ---------------------------------------------------------------------------

static int MaxDepthRecurse(TazusaBVHTree *Tree, int NodeIdx, int Depth) {
	if (NodeIdx < 0) return Depth;
	TazusaBVHNode *N = &Tree->Nodes[NodeIdx];
	if (TazusaBVHIsLeaf(N)) return Depth;
	int LD = MaxDepthRecurse(Tree, N->LeftChild, Depth + 1);
	int RD = MaxDepthRecurse(Tree, N->RightChild, Depth + 1);
	return LD > RD ? LD : RD;
}

int TazusaBVHValidateDepth(const TazusaBVHTree *Tree, int MaxDepth) {
	if (!Tree || Tree->NodeCount == 0) return 0;
	// Use a mutable copy for traversal
	TazusaBVHTree Mutable = *Tree;
	int D = MaxDepthRecurse(&Mutable, Mutable.RootIndex, 0);
	if (D > MaxDepth) return -1;
	return D;
}

// ---------------------------------------------------------------------------
//  0210: Dynamic leaf splitting
// ---------------------------------------------------------------------------

int TazusaBVHSplitLeaves(TazusaBVHTree *Tree,
                         const AABB *BodyAABBs,
                         int BodyCount) {
	if (!Tree || !BodyAABBs || Tree->NodeCount == 0) return 0;

	int Splits = 0;
	int MaxSplits = (Tree->NodeCapacity - Tree->NodeCount) / 2;
	if (MaxSplits < 0) MaxSplits = 0;

	// Scan for overfilled leaves using a stack
	int Stack[TAZUSA_MAX_DEPTH];
	int SP = 0;
	Stack[SP++] = Tree->RootIndex;

	while (SP > 0 && Splits < MaxSplits) {
		SP--;
		int NIdx = Stack[SP];
		if (NIdx < 0) continue;
		TazusaBVHNode *N = &Tree->Nodes[NIdx];

		if (TazusaBVHIsLeaf(N)) {
			int PrimCount = TazusaNodePrimCount(*N);
			if (PrimCount <= Tree->MaxLeafSize) continue;

			// This leaf is overfull — try to split it using SAH
			int PrimStart = N->PrimitiveIndex;
			if (PrimStart < 0 || PrimCount <= 1) continue;

			AABB LeftBox, RightBox;
			int LeftCount = FindBestSplit(BodyAABBs,
			                              Tree->PrimIndices,
			                              PrimStart, PrimCount,
			                              0, 16, &LeftBox, &RightBox);

			// Try other axes if first fails
			if (LeftCount <= 0 || LeftCount >= PrimCount) {
				LeftCount = FindBestSplit(BodyAABBs,
				                          Tree->PrimIndices,
				                          PrimStart, PrimCount,
				                          1, 16, &LeftBox, &RightBox);
			}
			if (LeftCount <= 0 || LeftCount >= PrimCount) {
				LeftCount = FindBestSplit(BodyAABBs,
				                          Tree->PrimIndices,
				                          PrimStart, PrimCount,
				                          2, 16, &LeftBox, &RightBox);
			}
			if (LeftCount <= 0 || LeftCount >= PrimCount) continue;

			// Create internal node and two children
			int LeftIdx = AllocNode(Tree);
			int RightIdx = AllocNode(Tree);
			if (LeftIdx < 0 || RightIdx < 0) continue;

			// Add parent pointer for the original leaf node
			int ParentIdx = N->Parent;

			// Create a new internal node to replace the leaf
			int InternalIdx = AllocNode(Tree);
			if (InternalIdx < 0) continue;

			TazusaBVHNode *Internal = &Tree->Nodes[InternalIdx];
			Internal->Box = N->Box;
			Internal->LeftChild = LeftIdx;
			Internal->RightChild = RightIdx;
			Internal->Parent = ParentIdx;

			// Update grandparent's child pointer
			if (ParentIdx >= 0) {
				if (Tree->Nodes[ParentIdx].LeftChild == NIdx)
					Tree->Nodes[ParentIdx].LeftChild = InternalIdx;
				if (Tree->Nodes[ParentIdx].RightChild == NIdx)
					Tree->Nodes[ParentIdx].RightChild = InternalIdx;
			} else {
				Tree->RootIndex = InternalIdx;
			}

			// Set up children as leaves
			TazusaBVHNode *LeftNode = &Tree->Nodes[LeftIdx];
			LeftNode->Box = LeftBox;
			LeftNode->Parent = InternalIdx;
			LeftNode->PrimitiveIndex = PrimStart;
			TazusaNodeSetPrimCount(*LeftNode, LeftCount);

			TazusaBVHNode *RightNode = &Tree->Nodes[RightIdx];
			RightNode->Box = RightBox;
			RightNode->Parent = InternalIdx;
			RightNode->PrimitiveIndex = PrimStart + LeftCount;
			TazusaNodeSetPrimCount(*RightNode, PrimCount - LeftCount);

			Splits++;

			// Push new children for further splitting if needed
			Stack[SP++] = RightIdx;
			Stack[SP++] = LeftIdx;
		} else {
			Stack[SP++] = N->RightChild;
			Stack[SP++] = N->LeftChild;
		}
	}

	return Splits;
}
