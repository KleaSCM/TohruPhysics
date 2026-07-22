/**
 * BroadPhase — coarse collision pair generation.
 * TohruPhysics用のBroadPhase — 粗い衝突ペア生成ね。
 *
 * Manages body AABBs, expands them with motion prediction, and generates
 * overlapping pairs filtered by collision groups. Uses brute-force O(n²)
 * overlap testing as the base algorithm.
 *
 * DESIGN PHILOSOPHY:
 * The broad phase identifies potentially overlapping body pairs quickly
 * so the narrow phase (GJK/SAT) only runs on candidates. Bodies are stored
 * in a dense array; removal swaps with the last element (O(1) remove).
 * The pair cache tracks persistent pairs from frame to frame.
 *
 * DATA LAYOUT:
 * ┌──────────────┬──────────────┬──────────────────────────────────────┐
 * │ Bodies[]     │ Dense array  │ Active bodies, swap-remove on delete │
 * │ Pairs[]      │ Dense array  │ Overlapping pairs this frame         │
 * │ BodyMap[]    │ Sparse map   │ External body ID → internal index    │
 * └──────────────┴──────────────┴──────────────────────────────────────┘
 *
 * References:
 * - Real-Time Collision Detection (Ericson), Chapter 3
 * - Bullet Physics btBroadphaseInterface
 * - Box2D b2BroadPhase
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#pragma once

#include <TohruPhysics/Geometry.h>
#include <TohruPhysics/Vector3.h>

// Forward declaration for threaded Evaluate (0187)
typedef struct ThreadPool ThreadPool;

#define BROADPHASE_MAX_BODIES 2048
#define BROADPHASE_MAX_PAIRS (BROADPHASE_MAX_BODIES * 8)
#define BROADPHASE_INVALID_INDEX (-1)

// Collision group bitmasks
#define COLLISIONGROUP_ALL       (~0)
#define COLLISIONGROUP_NONE      (0)
#define COLLISIONGROUP_STATIC    (1 << 0)
#define COLLISIONGROUP_DYNAMIC   (1 << 1)
#define COLLISIONGROUP_SENSOR    (1 << 2)
#define COLLISIONGROUP_CUSTOM1   (1 << 3)
#define COLLISIONGROUP_CUSTOM2   (1 << 4)
#define COLLISIONGROUP_CUSTOM3   (1 << 5)
#define COLLISIONGROUP_CUSTOM4   (1 << 6)

// 0184: Body type enum
typedef enum {
	BPBodyType_Static   = 0,
	BPBodyType_Dynamic  = 1,
	BPBodyType_Sensor   = 2
} BroadPhaseBodyType;

// 0181: Per-body data tracked by the broad phase
typedef struct {
	int            BodyId;          // external body identifier
	int            BodyType;        // BroadPhaseBodyType
	int            CollisionGroup;  // bitmask for group filtering
	int            CollidesWith;    // bitmask: groups this body collides with
	AABB           AABBBox;         // current world-space AABB
	AABB           PredictedAABB;   // AABB expanded by motion prediction
	Vector3        LinearVelocity;
	Vector3        AngularVelocity;
} BroadPhaseBody;

// 0188: Overlapping body pair
typedef struct {
	int  BodyA;        // index into Bodies[]
	int  BodyB;        // index into Bodies[]
	int  Persistent;   // 1 if this pair also existed last frame
} BroadPhasePair;

// 0190: Performance statistics
typedef struct {
	int    BodyCount;
	int    ActiveBodyCount;
	int    PairCount;
	int    PersistentPairCount;
	int    NewPairCount;
	int    TotalPairsGenerated;
	double UpdateTimeMs;
	double PairGenerationTimeMs;
	int    MaxPairsPerFrame;
	int    AABBTestsPerFrame;
} BroadPhaseStats;

// 0185: Debug visualisation callback.
// Called for each active body with its current and predicted AABBs.
typedef void (*BroadPhaseDebugCallback)(int BodyIndex,
                                        const AABB *CurrentAABB,
                                        const AABB *PredictedAABB,
                                        int BodyType,
                                        void *UserData);

// 0181: Main broad phase state
typedef struct {
	BroadPhaseBody   Bodies[BROADPHASE_MAX_BODIES];
	int              BodyCount;
	BroadPhasePair   Pairs[BROADPHASE_MAX_PAIRS];
	int              PairCount;
	BroadPhaseStats  Stats;

	// Sparse map: external BodyId → internal index
	int              BodyMap[BROADPHASE_MAX_BODIES];

	// Internal pair cache for persistence tracking
	int              LastFramePairs[BROADPHASE_MAX_PAIRS];
	int              LastFramePairCount;
} BroadPhase;

// 0181: Initialise broad phase — zero state
void MiyabiBroadPhaseInit(BroadPhase *BP);

// 0182: Add a body. Returns internal index (use for updates/removal).
int MiyabiBroadPhaseAddBody(BroadPhase *BP, const BroadPhaseBody *Body);

// 0182: Remove a body by internal index (swap-remove).
void MiyabiBroadPhaseRemoveBody(BroadPhase *BP, int BodyIndex);

// 0182: Update body AABB and velocity for motion prediction.
void MiyabiBroadPhaseUpdateBody(BroadPhase *BP, int BodyIndex,
                                const AABB *NewAABB,
                                const Vector3 *LinearVelocity,
                                const Vector3 *AngularVelocity);

// 0183: Expand body AABB by motion prediction. Internal — called by Evaluate.
void MiyabiBroadPhaseExpandAABB(BroadPhase *BP, int BodyIndex, Real DeltaTime);

// 0184: Test whether two bodies can collide based on group filtering.
int MiyabiBroadPhaseCanCollide(const BroadPhase *BP, int BodyA, int BodyB);

// 0182–0183: Generate overlapping pairs for all active bodies (single-threaded).
void MiyabiBroadPhaseEvaluate(BroadPhase *BP, Real DeltaTime);

// 0187: Generate overlapping pairs using a thread pool for parallel AABB
// expansion and pair generation. Falls back to single-threaded if Pool is NULL.
void MiyabiBroadPhaseEvaluateThreaded(BroadPhase *BP,
                                      Real DeltaTime,
                                      ThreadPool *Pool);

// 0182–0183: Test whether two bodies' predicted AABBs overlap.
int MiyabiBroadPhaseTestOverlap(const BroadPhase *BP, int BodyA, int BodyB);

// 0188: Clear all pairs from the current frame.
void MiyabiBroadPhaseClearPairs(BroadPhase *BP);

// 0190: Get current statistics.
BroadPhaseStats MiyabiBroadPhaseGetStats(const BroadPhase *BP);

// 0190: Reset statistics counters.
void MiyabiBroadPhaseResetStats(BroadPhase *BP);

// 0189: Validate pair generation — check for missed or duplicate pairs.
// Returns number of issues found (0 = clean).
int MiyabiBroadPhaseValidate(BroadPhase *BP);

// 0185: Debug visualisation — iterate over all bodies and call the callback
// with each body's current and predicted AABB.
void MiyabiBroadPhaseDebugAABBs(BroadPhase *BP,
                                BroadPhaseDebugCallback Callback,
                                void *UserData);
