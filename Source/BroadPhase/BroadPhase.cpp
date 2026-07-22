/**
 * BroadPhase — coarse collision pair generation implementation.
 * TohruPhysics用のBroadPhase実装ね。
 *
 * Brute-force O(n²) AABB overlap test with collision group filtering,
 * motion prediction AABB expansion, and pair persistence tracking.
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include <TohruPhysics/BroadPhase.h>
#include <TohruPhysics/ThreadPool.h>
#include <TohruPhysics/Math.h>
#include <string.h>
#include <cstdlib>

// 0181: Initialise broad phase — everything starts zeroed.
// 0181: BroadPhase初期化 — すべてゼロ初期化ね。
void MiyabiBroadPhaseInit(BroadPhase *BP) {
	memset(BP, 0, sizeof(BroadPhase));
	for (int I = 0; I < BROADPHASE_MAX_BODIES; I++) {
		BP->BodyMap[I] = BROADPHASE_INVALID_INDEX;
	}
}

// 0182: Add a body. Returns internal index for later updates.
// 0182: ボディ追加。更新用に内部インデックスを返すの。
int MiyabiBroadPhaseAddBody(BroadPhase *BP, const BroadPhaseBody *Body) {
	if (BP->BodyCount >= BROADPHASE_MAX_BODIES) {
		return BROADPHASE_INVALID_INDEX;
	}
	int Idx = BP->BodyCount++;
	BP->Bodies[Idx] = *Body;
	BP->Bodies[Idx].PredictedAABB = Body->AABBBox;
	if (Body->BodyId >= 0 && Body->BodyId < BROADPHASE_MAX_BODIES) {
		BP->BodyMap[Body->BodyId] = Idx;
	}
	return Idx;
}

// 0182: Remove a body by internal index (swap with last).
// 0182: 内部インデックスでボディ削除（最後尾とスワップ）。
void MiyabiBroadPhaseRemoveBody(BroadPhase *BP, int BodyIndex) {
	if (BodyIndex < 0 || BodyIndex >= BP->BodyCount) return;
	int LastIdx = BP->BodyCount - 1;

	// Capture removed body's ID before any mutation
	int RemovedId = BP->Bodies[BodyIndex].BodyId;

	if (BodyIndex != LastIdx) {
		// Swap with last element
		BP->Bodies[BodyIndex] = BP->Bodies[LastIdx];
		// Update sparse map for the moved body
		int MovedId = BP->Bodies[BodyIndex].BodyId;
		if (MovedId >= 0 && MovedId < BROADPHASE_MAX_BODIES) {
			BP->BodyMap[MovedId] = BodyIndex;
		}
	}

	// Clear the last slot and decrement count
	memset(&BP->Bodies[LastIdx], 0, sizeof(BroadPhaseBody));
	BP->BodyCount--;

	// Invalidate map for removed body
	if (RemovedId >= 0 && RemovedId < BROADPHASE_MAX_BODIES) {
		BP->BodyMap[RemovedId] = BROADPHASE_INVALID_INDEX;
	}
}

// 0182: Update body AABB and velocity.
// 0182: ボディのAABBと速度を更新。
void MiyabiBroadPhaseUpdateBody(BroadPhase *BP, int BodyIndex,
                                const AABB *NewAABB,
                                const Vector3 *LinearVelocity,
                                const Vector3 *AngularVelocity) {
	if (BodyIndex < 0 || BodyIndex >= BP->BodyCount) return;
	BP->Bodies[BodyIndex].AABBBox = *NewAABB;
	if (LinearVelocity) {
		BP->Bodies[BodyIndex].LinearVelocity = *LinearVelocity;
	}
	if (AngularVelocity) {
		BP->Bodies[BodyIndex].AngularVelocity = *AngularVelocity;
	}
}

// 0183: Expand body AABB by motion prediction.
// Uses linear velocity * dt plus a small angular expansion factor.
// 0183: 運動予測でAABBを拡張。線形速度×dtと角速度分の余裕を足すの。
void MiyabiBroadPhaseExpandAABB(BroadPhase *BP, int BodyIndex, Real DeltaTime) {
	if (BodyIndex < 0 || BodyIndex >= BP->BodyCount) return;
	BroadPhaseBody *Body = &BP->Bodies[BodyIndex];
	AABB *Pred = &Body->PredictedAABB;

	if (Body->BodyType == BPBodyType_Static) {
		*Pred = Body->AABBBox;
		return;
	}

	// Expand by linear velocity
	Vector3 VDelta = KannaVector3Scale(&Body->LinearVelocity, DeltaTime);
	Vector3 AbsVDelta = KannaVector3Abs(&VDelta);

	// Angular expansion: estimate from angular velocity * radius
	Vector3 HalfExtents = SabinaAABBHalfExtents(Pred);
	Real MaxExtent = YuuMax(YuuMax(HalfExtents.Data[0], HalfExtents.Data[1]), HalfExtents.Data[2]);
	Real AngExp = KannaVector3Length(&Body->AngularVelocity) * MaxExtent * DeltaTime;
	Real AngExpClamped = AngExp > REAL_ZERO ? AngExp : REAL_ZERO;

	Vector3 Expand = KannaVector3Make(
		AbsVDelta.Data[0] + AngExpClamped,
		AbsVDelta.Data[1] + AngExpClamped,
		AbsVDelta.Data[2] + AngExpClamped);

	Vector3 NewMin = KannaVector3Sub(&Body->AABBBox.Min, &Expand);
	Vector3 NewMax = KannaVector3Add(&Body->AABBBox.Max, &Expand);
	*Pred = SabinaAABBMake(&NewMin, &NewMax);
}

// 0184: Test collision group filtering.
// Two bodies collide if BodyA is in a group that BodyB collides with
// AND BodyB is in a group that BodyA collides with.
// 0184: 衝突グループフィルタリング。双方向で許可されている必要があるの。
int MiyabiBroadPhaseCanCollide(const BroadPhase *BP, int BodyA, int BodyB) {
	if (BodyA < 0 || BodyA >= BP->BodyCount) return 0;
	if (BodyB < 0 || BodyB >= BP->BodyCount) return 0;
	const BroadPhaseBody *BA = &BP->Bodies[BodyA];
	const BroadPhaseBody *BB = &BP->Bodies[BodyB];
	// A collides with B if A's group is in B's mask and B's group is in A's mask
	int ABit = (BA->CollisionGroup & BB->CollidesWith) != 0;
	int BBit = (BB->CollisionGroup & BA->CollidesWith) != 0;
	return ABit && BBit;
}

// 0188: Check if a pair existed in the previous frame.
static int WasPersistent(BroadPhase *BP, int BodyA, int BodyB) {
	for (int I = 0; I < BP->LastFramePairCount; I++) {
		int PA = BP->LastFramePairs[I * 2];
		int PB = BP->LastFramePairs[I * 2 + 1];
		if ((PA == BodyA && PB == BodyB) || (PA == BodyB && PB == BodyA)) {
			return 1;
		}
	}
	return 0;
}

// 0182–0183: Generate overlapping pairs for all bodies.
// 0182–0183: 全ボディのオーバーラップペアを生成。
void MiyabiBroadPhaseEvaluate(BroadPhase *BP, Real DeltaTime) {
	int AABBTests = 0;

	// Step 1: Expand AABBs by motion prediction
	for (int I = 0; I < BP->BodyCount; I++) {
		MiyabiBroadPhaseExpandAABB(BP, I, DeltaTime);
	}

	// Step 2: Generate new pairs via brute-force AABB overlap
	BP->PairCount = 0;
	for (int I = 0; I < BP->BodyCount; I++) {
		for (int J = I + 1; J < BP->BodyCount; J++) {
			AABBTests++;
			if (!MiyabiBroadPhaseCanCollide(BP, I, J)) continue;

			const AABB *PredA = &BP->Bodies[I].PredictedAABB;
			const AABB *PredB = &BP->Bodies[J].PredictedAABB;
			if (!SabinaAABBOverlaps(PredA, PredB)) continue;

			if (BP->PairCount >= BROADPHASE_MAX_PAIRS) break;

			BroadPhasePair *Pair = &BP->Pairs[BP->PairCount++];
			Pair->BodyA = I;
			Pair->BodyB = J;
			Pair->Persistent = WasPersistent(BP, I, J);
		}
		if (BP->PairCount >= BROADPHASE_MAX_PAIRS) break;
	}

	// Step 4: Update persistence cache
	for (int I = 0; I < BP->PairCount && I < BROADPHASE_MAX_PAIRS; I++) {
		BP->LastFramePairs[I * 2] = BP->Pairs[I].BodyA;
		BP->LastFramePairs[I * 2 + 1] = BP->Pairs[I].BodyB;
	}
	BP->LastFramePairCount = BP->PairCount;

	// Step 5: Update stats
	int NewCount = 0;
	int PersCount = 0;
	for (int I = 0; I < BP->PairCount; I++) {
		if (BP->Pairs[I].Persistent) PersCount++;
		else NewCount++;
	}

	BP->Stats.BodyCount = BP->BodyCount;
	BP->Stats.ActiveBodyCount = BP->BodyCount;
	BP->Stats.PairCount = BP->PairCount;
	BP->Stats.PersistentPairCount = PersCount;
	BP->Stats.NewPairCount = NewCount;
	BP->Stats.TotalPairsGenerated += BP->PairCount;
	BP->Stats.AABBTestsPerFrame = AABBTests;

	if (BP->PairCount > BP->Stats.MaxPairsPerFrame) {
		BP->Stats.MaxPairsPerFrame = BP->PairCount;
	}
}

// 0188: Clear all pairs.
// 0188: 全ペアをクリア。
void MiyabiBroadPhaseClearPairs(BroadPhase *BP) {
	BP->PairCount = 0;
}

// 0190: Get current statistics.
// 0190: 現在の統計を取得。
BroadPhaseStats MiyabiBroadPhaseGetStats(const BroadPhase *BP) {
	return BP->Stats;
}

// 0190: Reset statistics counters.
// 0190: 統計カウンタをリセット。
void MiyabiBroadPhaseResetStats(BroadPhase *BP) {
	memset(&BP->Stats, 0, sizeof(BroadPhaseStats));
}

// 0189: Validate pair generation — check for duplicates and misses.
// Returns number of issues found.
// 0189: ペア生成の検証 — 重複や見落としをチェック。
// 問題数（0=クリーン）を返すの。
int MiyabiBroadPhaseValidate(BroadPhase *BP) {
	int Issues = 0;

	// Check for duplicate pairs
	for (int I = 0; I < BP->PairCount; I++) {
		for (int J = I + 1; J < BP->PairCount; J++) {
			int AI = BP->Pairs[I].BodyA;
			int BI = BP->Pairs[I].BodyB;
			int AJ = BP->Pairs[J].BodyA;
			int BJ = BP->Pairs[J].BodyB;
			if ((AI == AJ && BI == BJ) || (AI == BJ && BI == AJ)) {
				Issues++;
			}
		}
	}

	// Check for self-pairs
	for (int I = 0; I < BP->PairCount; I++) {
		if (BP->Pairs[I].BodyA == BP->Pairs[I].BodyB) {
			Issues++;
		}
	}

	// Check for out-of-range indices
	for (int I = 0; I < BP->PairCount; I++) {
		if (BP->Pairs[I].BodyA < 0 || BP->Pairs[I].BodyA >= BP->BodyCount) Issues++;
		if (BP->Pairs[I].BodyB < 0 || BP->Pairs[I].BodyB >= BP->BodyCount) Issues++;
	}

	return Issues;
}

// Overlap test for two AABBs (used internally — wraps SabinaAABBOverlaps)
int MiyabiBroadPhaseTestOverlap(const BroadPhase *BP, int BodyA, int BodyB) {
	if (BodyA < 0 || BodyA >= BP->BodyCount) return 0;
	if (BodyB < 0 || BodyB >= BP->BodyCount) return 0;
	return SabinaAABBOverlaps(
		&BP->Bodies[BodyA].PredictedAABB,
		&BP->Bodies[BodyB].PredictedAABB);
}

// ===========================================================================
//  0185: Debug visualisation
// ===========================================================================

void MiyabiBroadPhaseDebugAABBs(BroadPhase *BP,
                                BroadPhaseDebugCallback Callback,
                                void *UserData) {
	if (!Callback) return;
	for (int I = 0; I < BP->BodyCount; I++) {
		Callback(I,
		         &BP->Bodies[I].AABBBox,
		         &BP->Bodies[I].PredictedAABB,
		         BP->Bodies[I].BodyType,
		         UserData);
	}
}

// ===========================================================================
//  0187: Threaded Evaluate helpers
// ===========================================================================

// Pair-generation task — each body I checks all J > I
typedef struct {
	BroadPhase *BP;
	int        *LocalPairs;  // per-thread pair buffer
	int         LocalCount;
} PairGenCtx;

static void PairGenTask(void *Arg, int ThreadIndex, int TaskIndex) {
	PairGenCtx *Ctx = &((PairGenCtx *)Arg)[ThreadIndex];
	BroadPhase *BP = Ctx->BP;
	int BodyI = TaskIndex;
	int *Pairs = Ctx->LocalPairs;
	int *Count = &Ctx->LocalCount;

	for (int J = BodyI + 1; J < BP->BodyCount; J++) {
		if (!MiyabiBroadPhaseCanCollide(BP, BodyI, J)) continue;
		const AABB *PredA = &BP->Bodies[BodyI].PredictedAABB;
		const AABB *PredB = &BP->Bodies[J].PredictedAABB;
		if (!SabinaAABBOverlaps(PredA, PredB)) continue;

		int Idx = __atomic_fetch_add(Count, 1, __ATOMIC_RELAXED);
		if (Idx * 2 + 1 < BROADPHASE_MAX_PAIRS * 2) {
			Pairs[Idx * 2]     = BodyI;
			Pairs[Idx * 2 + 1] = J;
		}
	}
}

// AABB expansion task for threaded Evaluate
typedef struct { BroadPhase *BP; Real DT; } ExpandCtxBP;

static void ExpandAABBTaskBP(void *Arg, int Ti, int Idx) {
	(void)Ti;
	ExpandCtxBP *C = (ExpandCtxBP *)Arg;
	MiyabiBroadPhaseExpandAABB(C->BP, Idx, C->DT);
}

void MiyabiBroadPhaseEvaluateThreaded(BroadPhase *BP,
                                      Real DeltaTime,
                                      ThreadPool *Pool) {
	if (!Pool) {
		MiyabiBroadPhaseEvaluate(BP, DeltaTime);
		return;
	}

	int NT = Pool->ThreadCount;

	// Phase 1: Parallel AABB expansion
	ExpandCtxBP ECtx;
	ECtx.BP = BP;
	ECtx.DT = DeltaTime;
	ThreadPoolParFor(Pool, BP->BodyCount, ExpandAABBTaskBP, &ECtx);

	// Phase 2: Allocate per-thread pair buffers (NT + 1 slots for
	// NT worker threads + main thread at index NT)
	PairGenCtx *CtxArray = (PairGenCtx *)calloc(
		(size_t)(NT + 1), sizeof(PairGenCtx));
	for (int T = 0; T <= NT; T++) {
		CtxArray[T].BP = BP;
		CtxArray[T].LocalPairs = (int *)calloc(
			BROADPHASE_MAX_PAIRS * 2, sizeof(int));
		CtxArray[T].LocalCount = 0;
	}

	// Phase 3: Parallel pair generation
	// Each body task writes pairs (I, J) where J > I, ensuring no duplicates.
	// Per-thread PairGenCtx is indexed by ThreadIndex.
	ThreadPoolParFor(Pool, BP->BodyCount, PairGenTask, CtxArray);

	// Phase 4: Merge per-thread buffers
	int TotalPairs = 0;
	for (int T = 0; T <= NT; T++) {
		int LC = CtxArray[T].LocalCount;
		if (LC > BROADPHASE_MAX_PAIRS) LC = BROADPHASE_MAX_PAIRS;
		int *Src = CtxArray[T].LocalPairs;
		for (int I = 0; I < LC && TotalPairs < BROADPHASE_MAX_PAIRS; I++) {
			BroadPhasePair *Pair = &BP->Pairs[TotalPairs++];
			Pair->BodyA = Src[I * 2];
			Pair->BodyB = Src[I * 2 + 1];
			Pair->Persistent = WasPersistent(BP, Pair->BodyA, Pair->BodyB);
		}
		free(Src);
	}
	free(CtxArray);
	BP->PairCount = TotalPairs;

	// Phase 5: Update persistence cache
	for (int I = 0; I < BP->PairCount && I < BROADPHASE_MAX_PAIRS; I++) {
		BP->LastFramePairs[I * 2] = BP->Pairs[I].BodyA;
		BP->LastFramePairs[I * 2 + 1] = BP->Pairs[I].BodyB;
	}
	BP->LastFramePairCount = BP->PairCount;

	// Phase 6: Stats
	int NewCount = 0, PersCount = 0;
	for (int I = 0; I < BP->PairCount; I++) {
		if (BP->Pairs[I].Persistent) PersCount++;
		else NewCount++;
	}
	BP->Stats.BodyCount = BP->BodyCount;
	BP->Stats.ActiveBodyCount = BP->BodyCount;
	BP->Stats.PairCount = BP->PairCount;
	BP->Stats.PersistentPairCount = PersCount;
	BP->Stats.NewPairCount = NewCount;
	BP->Stats.TotalPairsGenerated += BP->PairCount;
	if (BP->PairCount > BP->Stats.MaxPairsPerFrame)
		BP->Stats.MaxPairsPerFrame = BP->PairCount;
}
