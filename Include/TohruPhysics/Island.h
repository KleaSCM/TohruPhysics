/**
 * KuyuIsland — simulation island management for partitioned physics.
 * TohruPhysics用のシミュレーションアイランド管理ね。
 *
 * Islands are groups of interacting bodies that can be simulated
 * independently. Two bodies belong to the same island if a path of
 * overlapping AABB pairs connects them. Islands enable:
 *   - Sleeping: entire island deactivates when all bodies are at rest
 *   - Parallelism: independent islands run on separate threads
 *   - Deep sleep: islands with stable joints skip solver iterations
 *
 * DESIGN PHILOSOPHY:
 * The adjacency graph is rebuilt every frame from the current broad-phase
 * pair list (which captures all potentially-colliding body pairs). A DFS
 * over this graph yields the connected components, each becoming one island.
 * This is O(N + P) per frame where N = body count and P = pair count.
 *
 * Island state transitions:
 *   ACTIVE ↔ DEACTIVATING (sleep timer counting) → SLEEPING
 *   SLEEPING → ACTIVE (on new pair or external impulse)
 *   SLEEPING → DEEP_SLEEP (when joints are settled)
 *
 * References:
 *   - Erin Catto, "Modeling and Solving Constraints" (GDC 2009)
 *   - Box2D island sleeping implementation
 *   - Bullet Physics simIsland code path
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#pragma once

#include <TohruPhysics/BodyState.h>

// ===========================================================================
//  Constants
// ===========================================================================

#define KUYU_MAX_BODIES    4096
#define KUYU_MAX_ISLANDS   1024
#define KUYU_SLEEP_FRAMES  60   // ~1 second at 60fps before sleep
#define KUYU_SLEEP_LINEAR  0.01 // m/s threshold for linear velocity
#define KUYU_SLEEP_ANGULAR 0.01 // rad/s threshold for angular velocity

// ===========================================================================
//  0221: Island group
// ===========================================================================

// A single simulation island — a set of connected bodies.
typedef struct {
	int  FirstBody;     // head index of body linked list, -1 = empty
	int  BodyCount;
	int  IsActive;      // 1 = simulate, 0 = sleeping
	int  IsDeepSleep;   // 1 = deeply sleeping (joints stable)
	Real Energy;        // total kinetic energy (for diagnostics)
	int  SleepTimer;    // frames since any body showed significant motion
} KuyuIslandGroup;

// Per-body island membership (linked list node).
typedef struct {
	int  IslandIndex;   // which island, -1 = unassigned
	int  IslandPrev;    // previous body in island linked list, -1 = head
	int  IslandNext;    // next body in island linked list, -1 = tail
	int  Visited;       // for graph traversal (0/1)
} KuyuBodyIslandInfo;

// ===========================================================================
//  0222/0227: Adjacency graph (CSR)
// ===========================================================================

// Compressed sparse row graph built from broad-phase pair list.
typedef struct {
	int *RowStarts;   // [BodyCount + 1]
	int *Neighbors;   // flat array of all neighbor indices
	int  BodyCount;
	int  EdgeCount;   // number of undirected edges (pairs)
} KuyuAdjacencyGraph;

// ===========================================================================
//  0223+: Island manager
// ===========================================================================

typedef struct {
	KuyuBodyIslandInfo *BodyInfo;    // [BodyCapacity]
	KuyuIslandGroup    *Islands;     // [IslandCapacity]
	KuyuAdjacencyGraph  Graph;
	int                *WorkStack;   // DFS stack
	int                *WorkQueue;   // BFS queue for component fill
	int                 BodyCapacity;
	int                 IslandCount;
	int                 IslandCapacity;
	Real                SleepThresholdLinear;
	Real                SleepThresholdAngular;
	int                 SleepFramesThreshold;
} KuyuIslandManager;

// ===========================================================================
//  0221: Island lifecycle
// ===========================================================================

// Initialise manager. Returns 0 on success, -1 on alloc failure.
int  KuyuIslandInit(KuyuIslandManager *Mgr,
                    int BodyCapacity, int IslandCapacity);

// Free all resources.
void KuyuIslandDestroy(KuyuIslandManager *Mgr);

// Reset to empty (preserves buffers).
void KuyuIslandReset(KuyuIslandManager *Mgr);

// ===========================================================================
//  0222: Build adjacency graph from pair list
// ===========================================================================

// Build CSR adjacency from a flat pair array [A0, B0, A1, B1, ...].
// Returns 0 on success, -1 on error.
int KuyuIslandBuildGraph(KuyuIslandManager *Mgr,
                         const int *Pairs, int PairCount);

// ===========================================================================
//  0223: Connected components (island assignment)
// ===========================================================================

// Run DFS over the adjacency graph to assign each body to an island.
// Returns number of islands found, or -1 on error.
int KuyuIslandFindComponents(KuyuIslandManager *Mgr);

// ===========================================================================
//  0224: Activation
// ===========================================================================

// Wake a single body and its entire island.
// Returns 0 on success, -1 if body not found.
int KuyuIslandActivateBody(KuyuIslandManager *Mgr, int BodyIndex);

// Wake all bodies in a specific island.
int KuyuIslandActivateGroup(KuyuIslandManager *Mgr, int IslandIndex);

// ===========================================================================
//  0225: Deactivation (energy threshold)
// ===========================================================================

// Check all bodies for sleep eligibility. Deactivates entire island when
// all bodies have been below threshold for SleepFramesThreshold frames.
// Returns number of islands deactivated.
int KuyuIslandDeactivateCheck(KuyuIslandManager *Mgr,
                              const RigidBodyState *States,
                              int BodyCount);

// ===========================================================================
//  0226: Deep sleep qualification
// ===========================================================================

// Mark an island as deep-sleeping if average joint tension is below
// the given threshold. Deep islands skip joint warm-start and solver.
// Returns 1 if island entered deep sleep, 0 otherwise, -1 on error.
int KuyuIslandDeepSleepCheck(KuyuIslandManager *Mgr,
                             int IslandIndex,
                             const Real *JointImpulses, int JointCount,
                             Real Threshold);

// ===========================================================================
//  0227: Query connections for a body
// ===========================================================================

// Get the neighbour list for BodyIndex. Returns neighbour count,
// or -1 if BodyIndex out of range.
int KuyuIslandGetConnections(const KuyuIslandManager *Mgr,
                             int BodyIndex,
                             const int **OutNeighbors);

// ===========================================================================
//  0228: Island split
// ===========================================================================

// After a graph update, split an island if its bodies are now in
// multiple disconnected components. Returns number of new islands created.
int KuyuIslandSplit(KuyuIslandManager *Mgr, int OldIslandIndex);

// ===========================================================================
//  0229: Island merge
// ===========================================================================

// Merge two islands into one. Bodies from IslandB are moved into IslandA.
// Returns 0 on success, -1 on error.
int KuyuIslandMerge(KuyuIslandManager *Mgr,
                    int IslandAIndex, int IslandBIndex);

// ===========================================================================
//  0230: Load balancing
// ===========================================================================

// Assign islands to threads using a greedy body-count heuristic.
// IslandToThread[IslandIndex] receives the thread assignment.
// Returns number of islands assigned (should equal IslandCount).
int KuyuIslandLoadBalance(KuyuIslandManager *Mgr,
                          int *IslandToThread,
                          int ThreadCount);

// ===========================================================================
//  Convenience: full rebuild from pair list
// ===========================================================================

// Build graph, find components, split old islands, merge new connections.
// Returns island count, or -1 on error.
int KuyuIslandRebuild(KuyuIslandManager *Mgr,
                      const int *Pairs, int PairCount);
