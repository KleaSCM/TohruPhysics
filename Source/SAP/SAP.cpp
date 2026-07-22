/**
 * IliaSAP — sweep-and-prune broad-phase implementation.
 * TohruPhysics用のSAP実装ね。
 *
 * Implements 0211-0220: axis lists, insert/remove, incremental sort,
 * pair generation, multi-axis test, pair marker, dimension fallback,
 * batch verification, and boundary limit checking.
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include <TohruPhysics/SAP.h>
#include <TohruPhysics/Math.h>
#include <stdlib.h>
#include <string.h>

// ===========================================================================
//  Internal helpers
// ===========================================================================

#define MIN_CAPACITY 16

// Clamp extreme coordinate values to prevent overflow in sorting.
static Real ClampCoord(Real V) {
	if (MaiIsInf(V) || MaiIsNaN(V)) return 0.0;
	if (V < -1e12) return -1e12;
	if (V >  1e12) return  1e12;
	return V;
}

// ---------------------------------------------------------------------------
//  0211: Axis list lifecycle
// ---------------------------------------------------------------------------

int IliaSapAxisListInit(IliaSapAxisList *List, int BodyCapacity) {
	if (!List || BodyCapacity <= 0) return -1;

	int Cap = BodyCapacity > MIN_CAPACITY ? BodyCapacity : MIN_CAPACITY;
	Cap *= 2;  // two endpoints per body

	List->Endpoints = (IliaSapEndpoint *)calloc((size_t)Cap, sizeof(IliaSapEndpoint));
	if (!List->Endpoints) return -1;

	List->BodyToIndex = (int *)calloc((size_t)BodyCapacity, sizeof(int));
	if (!List->BodyToIndex) {
		free(List->Endpoints);
		List->Endpoints = NULL;
		return -1;
	}

	// BodyToIndex = -1 means unused
	memset(List->BodyToIndex, -1, (size_t)BodyCapacity * sizeof(int));

	List->Count = 0;
	List->Capacity = Cap;
	List->BodyCount = 0;
	List->BodyCapacity = BodyCapacity;
	List->Dirty = 0;
	return 0;
}

void IliaSapAxisListDestroy(IliaSapAxisList *List) {
	if (!List) return;
	free(List->Endpoints);
	free(List->BodyToIndex);
	List->Endpoints = NULL;
	List->BodyToIndex = NULL;
	List->Count = 0;
	List->Capacity = 0;
	List->BodyCount = 0;
	List->BodyCapacity = 0;
}

void IliaSapAxisListReset(IliaSapAxisList *List) {
	if (!List) return;
	List->Count = 0;
	List->BodyCount = 0;
	List->Dirty = 0;
	if (List->BodyToIndex) {
		memset(List->BodyToIndex, -1, (size_t)List->BodyCapacity * sizeof(int));
	}
}

// ---------------------------------------------------------------------------
//  0212: Insert body
// ---------------------------------------------------------------------------

// Insert two endpoints (min, max) for one body into the list, maintaining
// sorted order. Since bodies are inserted one at a time (not from a snapshot),
// we append and bubble-sort backwards (O(N) per insert, but N is small during
// batch insertion). For single-body insertion into a coherent scene, the list
// is already nearly-sorted and the bubble runs O(1) amortised.
static int InsertEndpoints(IliaSapAxisList *List, int BodyIndex,
                           Real MinVal, Real MaxVal) {
	if (BodyIndex < 0 || BodyIndex >= List->BodyCapacity) return -1;
	if (List->Count + 2 > List->Capacity) {
		// Grow
		int NewCap = List->Capacity * 2;
		if (NewCap < MIN_CAPACITY) NewCap = MIN_CAPACITY;
		IliaSapEndpoint *NewEps = (IliaSapEndpoint *)realloc(
			List->Endpoints, (size_t)NewCap * sizeof(IliaSapEndpoint));
		if (!NewEps) return -1;
		List->Endpoints = NewEps;
		List->Capacity = NewCap;
	}

	// Append min endpoint
	int MinPos = List->Count;
	List->Endpoints[MinPos].Value = ClampCoord(MinVal);
	List->Endpoints[MinPos].BodyIndex = BodyIndex;
	List->Endpoints[MinPos].IsMin = 1;

	// Append max endpoint
	int MaxPos = MinPos + 1;
	List->Endpoints[MaxPos].Value = ClampCoord(MaxVal);
	List->Endpoints[MaxPos].BodyIndex = BodyIndex;
	List->Endpoints[MaxPos].IsMin = 0;

	List->Count += 2;
	List->BodyCount++;

	// Record position — will be invalidated by sort, but useful immediately
	List->BodyToIndex[BodyIndex] = MinPos;

	// Bubble both endpoints backward to maintain sorted order.
	// The min endpoint should stay before the max endpoint for the same body.
	// We insert by moving each endpoint to its correct sorted position.
	int I;
	for (I = MinPos; I > 0 && List->Endpoints[I].Value < List->Endpoints[I - 1].Value; I--) {
		if (I - 1 == MaxPos) {
			// max is already ahead of us — swap with max first
			IliaSapEndpoint Tmp = List->Endpoints[I];
			List->Endpoints[I] = List->Endpoints[I - 1];
			List->Endpoints[I - 1] = Tmp;
			MaxPos = I;
		} else {
			IliaSapEndpoint Tmp = List->Endpoints[I];
			List->Endpoints[I] = List->Endpoints[I - 1];
			List->Endpoints[I - 1] = Tmp;
		}
	}

	// Bubble max endpoint
	for (I = MaxPos; I > 0 && List->Endpoints[I].Value < List->Endpoints[I - 1].Value; I--) {
		IliaSapEndpoint Tmp = List->Endpoints[I];
		List->Endpoints[I] = List->Endpoints[I - 1];
		List->Endpoints[I - 1] = Tmp;
	}

	// Update BodyToIndex — the min endpoint's final position
	// Scan forward from I to find it
	for (int J = 0; J < List->Count; J++) {
		if (List->Endpoints[J].BodyIndex == BodyIndex && List->Endpoints[J].IsMin) {
			List->BodyToIndex[BodyIndex] = J;
			break;
		}
	}

	return 0;
}

int IliaSapInsertBody(IliaSapManager *Mgr, int BodyIndex, const AABB *Box) {
	if (!Mgr || !Box) return -1;
	if (BodyIndex < 0 || BodyIndex >= Mgr->BodyCapacity) return -1;

	// Check if already present
	for (int A = 0; A < 3; A++) {
		if (Mgr->Axes[A].BodyToIndex[BodyIndex] >= 0) return -1;
	}

	for (int A = 0; A < 3; A++) {
		Real MinVal = Box->Min.Data[A];
		Real MaxVal = Box->Max.Data[A];
		if (InsertEndpoints(&Mgr->Axes[A], BodyIndex, MinVal, MaxVal) != 0)
			return -1;
	}
	return 0;
}

// ---------------------------------------------------------------------------
//  0213: Remove body
// ---------------------------------------------------------------------------

// Remove both endpoints for BodyIndex from one axis list by swapping them
// to the end and decrementing Count. Updates BodyToIndex for the moved body.
static int RemoveEndpoints(IliaSapAxisList *List, int BodyIndex) {
	// Find both endpoints
	int MinPos = -1, MaxPos = -1;
	for (int I = 0; I < List->Count; I++) {
		if (List->Endpoints[I].BodyIndex == BodyIndex) {
			if (List->Endpoints[I].IsMin) MinPos = I;
			else MaxPos = I;
		}
	}
	if (MinPos < 0 || MaxPos < 0) return -1;

	// Ensure MinPos < MaxPos (swap if needed)
	if (MinPos > MaxPos) {
		int Tmp = MinPos; MinPos = MaxPos; MaxPos = Tmp;
	}

	// Remove max first (swap with last), then min
	// Remove max
	if (MaxPos < List->Count - 1) {
		int LastIdx = List->Count - 1;
		int MovedBody = List->Endpoints[LastIdx].BodyIndex;
		List->Endpoints[MaxPos] = List->Endpoints[LastIdx];
		// Update BodyToIndex for the moved body's min endpoint
		if (List->Endpoints[LastIdx].IsMin) {
			List->BodyToIndex[MovedBody] = MaxPos;
		}
	}
	List->Count--;

	// Remove min (may have shifted if max was before it — min is now at MinPos
	// since max removal didn't shift anything before MinPos)
	if (MinPos < List->Count - 1) {
		int LastIdx = List->Count - 1;
		int MovedBody = List->Endpoints[LastIdx].BodyIndex;
		List->Endpoints[MinPos] = List->Endpoints[LastIdx];
		if (List->Endpoints[LastIdx].IsMin) {
			List->BodyToIndex[MovedBody] = MinPos;
		}
	}
	List->Count--;

	List->BodyCount--;
	List->BodyToIndex[BodyIndex] = -1;
	return 0;
}

int IliaSapRemoveBody(IliaSapManager *Mgr, int BodyIndex) {
	if (!Mgr) return -1;
	int Result = 0;
	for (int A = 0; A < 3; A++) {
		if (RemoveEndpoints(&Mgr->Axes[A], BodyIndex) != 0)
			Result = -1;
	}
	return Result;
}

// ---------------------------------------------------------------------------
//  0214: Incremental insertion sort
// ---------------------------------------------------------------------------

// One pass of insertion sort on an axis list.
// Returns number of swaps performed.
static int SortAxis(IliaSapAxisList *List) {
	if (!List || List->Count <= 1) return 0;
	int Swaps = 0;

	for (int I = 1; I < List->Count; I++) {
		IliaSapEndpoint Key = List->Endpoints[I];
		int J = I - 1;

		// Scan backwards, shifting elements that are > Key
		while (J >= 0 && List->Endpoints[J].Value > Key.Value) {
			List->Endpoints[J + 1] = List->Endpoints[J];
			J--;
			Swaps++;
		}

		// Tiebreaker: if values equal, use body index to maintain
		// deterministic order (min endpoint before max for same body)
		while (J >= 0 &&
		       List->Endpoints[J].Value == Key.Value &&
		       (List->Endpoints[J].BodyIndex > Key.BodyIndex ||
		        (List->Endpoints[J].BodyIndex == Key.BodyIndex &&
		         List->Endpoints[J].IsMin > Key.IsMin))) {
			List->Endpoints[J + 1] = List->Endpoints[J];
			J--;
			Swaps++;
		}

		List->Endpoints[J + 1] = Key;
	}

	// Rebuild BodyToIndex mapping after sort
	memset(List->BodyToIndex, -1, (size_t)List->BodyCapacity * sizeof(int));
	for (int I = 0; I < List->Count; I++) {
		if (List->Endpoints[I].IsMin) {
			List->BodyToIndex[List->Endpoints[I].BodyIndex] = I;
		}
	}

	return Swaps;
}

int IliaSapUpdateSort(IliaSapManager *Mgr) {
	if (!Mgr) return -1;
	int TotalSwaps = 0;
	for (int A = 0; A < 3; A++) {
		TotalSwaps += SortAxis(&Mgr->Axes[A]);
		Mgr->Axes[A].Dirty = 0;
	}
	return TotalSwaps;
}

// ---------------------------------------------------------------------------
//  0215: Sweep pair generation
// ---------------------------------------------------------------------------

int IliaSapGeneratePairs(IliaSapManager *Mgr) {
	if (!Mgr) return -1;

	IliaSapAxisList *Primary = &Mgr->Axes[ILIA_SWEEP_AXIS];
	int EC = Primary->Count;

	Mgr->OverlapCount = 0;

	// Ensure overlap buffer is large enough
	if (Mgr->OverlapCapacity < ILIA_PAIR_BUFFER_GROWTH) {
		int NewCap = Mgr->OverlapCapacity > 0
			? Mgr->OverlapCapacity * 2 : ILIA_PAIR_BUFFER_GROWTH;
		int *NewBuf = (int *)realloc(Mgr->OverlapBuffer,
			(size_t)NewCap * sizeof(int));
		if (!NewBuf) return -1;
		Mgr->OverlapBuffer = NewBuf;
		Mgr->OverlapCapacity = NewCap;
	}

	// Active set: track which bodies are currently open (min seen, max not yet)
	int *Active = (int *)calloc((size_t)Primary->BodyCapacity, sizeof(int));
	if (!Active) return -1;
	int ActiveCount = 0;

	// Sweep
	IliaSapPairMarkerNextFrame(&Mgr->Marker);

	for (int I = 0; I < EC; I++) {
		IliaSapEndpoint *Ep = &Primary->Endpoints[I];
		int BIdx = Ep->BodyIndex;

		if (Ep->IsMin) {
			// Opening — add to active set
			Active[ActiveCount++] = BIdx;
		} else {
			// Closing — generate pairs with all active bodies (excluding self)
			// and remove from active set
			for (int J = 0; J < ActiveCount; J++) {
				int Other = Active[J];
				if (Other == BIdx) continue;

				// Check overlap on Y and Z
				if (!IliaSapTestOverlap(Mgr, BIdx, Other)) continue;

				// Deduplicate
				if (IliaSapPairMarkerTest(&Mgr->Marker, BIdx, Other)) continue;

				// Ensure buffer capacity
				if (Mgr->OverlapCount + 2 > Mgr->OverlapCapacity) {
					int NewCap = Mgr->OverlapCapacity * 2;
					int *NewBuf = (int *)realloc(Mgr->OverlapBuffer,
						(size_t)NewCap * sizeof(int));
					if (!NewBuf) { free(Active); return -1; }
					Mgr->OverlapBuffer = NewBuf;
					Mgr->OverlapCapacity = NewCap;
				}

				// Store pair in canonical order
				if (BIdx < Other) {
					Mgr->OverlapBuffer[Mgr->OverlapCount * 2]     = BIdx;
					Mgr->OverlapBuffer[Mgr->OverlapCount * 2 + 1] = Other;
				} else {
					Mgr->OverlapBuffer[Mgr->OverlapCount * 2]     = Other;
					Mgr->OverlapBuffer[Mgr->OverlapCount * 2 + 1] = BIdx;
				}
				Mgr->OverlapCount++;
			}

			// Remove self from active set (swap remove)
			for (int J = 0; J < ActiveCount; J++) {
				if (Active[J] == BIdx) {
					Active[J] = Active[ActiveCount - 1];
					ActiveCount--;
					break;
				}
			}
		}
	}

	free(Active);
	return Mgr->OverlapCount;
}

// ---------------------------------------------------------------------------
//  0216: Multi-axis overlap test
// ---------------------------------------------------------------------------

int IliaSapTestOverlap(IliaSapManager *Mgr, int BodyA, int BodyB) {
	// Check all three axes
	for (int A = 0; A < 3; A++) {
		if (!IliaSapDimensionTest(&Mgr->Axes[A], BodyA, BodyB))
			return 0;
	}
	return 1;
}

// ---------------------------------------------------------------------------
//  0218: Dimension test with fallback
// ---------------------------------------------------------------------------

int IliaSapDimensionTest(IliaSapAxisList *List, int BodyA, int BodyB) {
	if (BodyA == BodyB) return 1;

	// Find endpoints via linear scan (BodyToIndex may be stale after sort,
	// but we already re-sorted in UpdateSort, so it should be current)
	int MinA = -1, MaxA = -1, MinB = -1, MaxB = -1;

	for (int I = 0; I < List->Count; I++) {
		int BI = List->Endpoints[I].BodyIndex;
		if (BI == BodyA) {
			if (List->Endpoints[I].IsMin) MinA = I;
			else MaxA = I;
		} else if (BI == BodyB) {
			if (List->Endpoints[I].IsMin) MinB = I;
			else MaxB = I;
		}
	}

	if (MinA < 0 || MaxA < 0 || MinB < 0 || MaxB < 0) return 0;

	// BodyA and BodyB overlap on this axis iff:
	//   max(MinA_coord, MinB_coord) <= min(MaxA_coord, MaxB_coord)
	Real AMin = List->Endpoints[MinA].Value;
	Real AMax = List->Endpoints[MaxA].Value;
	Real BMin = List->Endpoints[MinB].Value;
	Real BMax = List->Endpoints[MaxB].Value;

	// Fallback for identical intervals: if both intervals are identical
	// (common for axis-aligned objects on one axis), use body index to
	// break tie — they still overlap.
	if (NagisaApproxEqual(AMin, BMin, 1e-10) &&
	    NagisaApproxEqual(AMax, BMax, 1e-10)) {
		return 1;
	}

	return (AMin <= BMax && BMin <= AMax);
}

// ---------------------------------------------------------------------------
//  0217: Pair marker
// ---------------------------------------------------------------------------

int IliaSapPairMarkerInit(IliaSapPairMarker *Mkr, int BodyCapacity) {
	if (!Mkr || BodyCapacity <= 0) return -1;
	size_t N = (size_t)BodyCapacity * (size_t)BodyCapacity;
	Mkr->Data = (short *)calloc(N, sizeof(short));
	if (!Mkr->Data) return -1;
	Mkr->Generation = 1;
	Mkr->BodyCapacity = BodyCapacity;
	return 0;
}

void IliaSapPairMarkerDestroy(IliaSapPairMarker *Mkr) {
	if (!Mkr) return;
	free(Mkr->Data);
	Mkr->Data = NULL;
	Mkr->Generation = 0;
	Mkr->BodyCapacity = 0;
}

void IliaSapPairMarkerNextFrame(IliaSapPairMarker *Mkr) {
	Mkr->Generation++;
	// Wrap-around: if generation wraps past 32767, reset to 1 and
	// clear the entire marker array to avoid stale matches.
	if (Mkr->Generation <= 0) {
		Mkr->Generation = 1;
		size_t N = (size_t)Mkr->BodyCapacity * (size_t)Mkr->BodyCapacity;
		memset(Mkr->Data, 0, N * sizeof(short));
	}
}

int IliaSapPairMarkerTest(IliaSapPairMarker *Mkr, int BodyA, int BodyB) {
	// Canonical order: smaller index first
	int A = BodyA < BodyB ? BodyA : BodyB;
	int B = BodyA < BodyB ? BodyB : BodyA;

	size_t Idx = (size_t)A * (size_t)Mkr->BodyCapacity + (size_t)B;
	if (Mkr->Data[Idx] == Mkr->Generation) {
		return 1;  // already seen
	}
	Mkr->Data[Idx] = Mkr->Generation;
	return 0;  // new pair
}

// ---------------------------------------------------------------------------
//  0219: Batch pair verification
// ---------------------------------------------------------------------------

int IliaSapVerifyPairs(IliaSapManager *Mgr, const AABB *BodyAABBs) {
	if (!Mgr || !BodyAABBs) return 0;

	int Out = 0;
	for (int I = 0; I < Mgr->OverlapCount; I++) {
		int A = Mgr->OverlapBuffer[I * 2];
		int B = Mgr->OverlapBuffer[I * 2 + 1];

		// Verify AABB overlap
		const AABB *BoxA = &BodyAABBs[A];
		const AABB *BoxB = &BodyAABBs[B];

		int Overlap = 1;
		for (int Ax = 0; Ax < 3; Ax++) {
			if (BoxA->Max.Data[Ax] < BoxB->Min.Data[Ax] ||
			    BoxB->Max.Data[Ax] < BoxA->Min.Data[Ax]) {
				Overlap = 0;
				break;
			}
		}

		if (Overlap) {
			Mgr->OverlapBuffer[Out * 2]     = A;
			Mgr->OverlapBuffer[Out * 2 + 1] = B;
			Out++;
		}
	}

	Mgr->OverlapCount = Out;
	return Out;
}

// ---------------------------------------------------------------------------
//  0220: Boundary limit checking
// ---------------------------------------------------------------------------

int IliaSapBoundaryCheck(IliaSapManager *Mgr) {
	if (!Mgr) return 0;
	int Clamped = 0;
	for (int A = 0; A < 3; A++) {
		IliaSapAxisList *List = &Mgr->Axes[A];
		for (int I = 0; I < List->Count; I++) {
			Real V = List->Endpoints[I].Value;
			Real CV = ClampCoord(V);
			if (CV != V) {
				List->Endpoints[I].Value = CV;
				Clamped++;
			}
		}
	}
	return Clamped;
}

// ---------------------------------------------------------------------------
//  Manager lifecycle
// ---------------------------------------------------------------------------

int IliaSapInit(IliaSapManager *Mgr, int BodyCapacity) {
	if (!Mgr) return -1;
	memset(Mgr, 0, sizeof(*Mgr));

	if (BodyCapacity <= 0) BodyCapacity = ILIA_DEFAULT_BODY_CAPACITY;
	if (BodyCapacity > ILIA_MAX_BODY_CAPACITY) BodyCapacity = ILIA_MAX_BODY_CAPACITY;

	for (int A = 0; A < 3; A++) {
		if (IliaSapAxisListInit(&Mgr->Axes[A], BodyCapacity) != 0) {
			// Partial cleanup
			for (int J = 0; J < A; J++)
				IliaSapAxisListDestroy(&Mgr->Axes[J]);
			return -1;
		}
	}

	if (IliaSapPairMarkerInit(&Mgr->Marker, BodyCapacity) != 0) {
		for (int A = 0; A < 3; A++)
			IliaSapAxisListDestroy(&Mgr->Axes[A]);
		return -1;
	}

	Mgr->OverlapBuffer = NULL;
	Mgr->OverlapCapacity = 0;
	Mgr->OverlapCount = 0;
	Mgr->BodyCapacity = BodyCapacity;
	return 0;
}

void IliaSapDestroy(IliaSapManager *Mgr) {
	if (!Mgr) return;
	for (int A = 0; A < 3; A++)
		IliaSapAxisListDestroy(&Mgr->Axes[A]);
	IliaSapPairMarkerDestroy(&Mgr->Marker);
	free(Mgr->OverlapBuffer);
	Mgr->OverlapBuffer = NULL;
	Mgr->OverlapCapacity = 0;
	Mgr->OverlapCount = 0;
}

void IliaSapReset(IliaSapManager *Mgr) {
	if (!Mgr) return;
	for (int A = 0; A < 3; A++)
		IliaSapAxisListReset(&Mgr->Axes[A]);
	Mgr->OverlapCount = 0;
	IliaSapPairMarkerNextFrame(&Mgr->Marker);
}
