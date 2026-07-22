/**
 * IliaSAP — sweep-and-prune broad phase for collision detection.
 * TohruPhysics用のSAP — スイープアンドプルーンによるBroadPhaseね。
 *
 * Maintains three axis-aligned endpoint lists (X, Y, Z) sorted by coordinate
 * value. Each body contributes two endpoints per axis (min and max). After
 * bodies move, the lists are re-sorted with incremental insertion sort (O(N)
 * for coherent motion). Pair generation sweeps one axis and validates overlap
 * on the remaining two, using a generation-based marker to suppress duplicates.
 *
 * DESIGN PHILOSOPHY:
 * Unlike SpatialGrid (uniform subdivision) or BVH (hierarchical tree), SAP
 * operates on sorted 1D intervals. It excels when:
 *   - Bodies move coherently (lists stay nearly sorted → O(N) update)
 *   - Most pairs are separated on at least one axis (early out)
 *   - N < 1000 (beyond which grid/BVH scale better)
 *
 * The marker uses a rolling generation counter stored in a short-per-pair
 * matrix (BodyCapacity² entries × 2 bytes). Each frame increments the
 * generation; a pair whose marker equals the current generation was already
 * emitted. This avoids O(P²) duplicate checks without clearing the matrix.
 *
 * LIMITATIONS:
 *   - Assumes bodies move by small amounts between frames.
 *   - Worst-case O(N²) when all bodies overlap on all axes.
 *   - Does not handle inserted/removed bodies during a frame (flush first).
 *
 * References:
 *   - Baraff & Witkin, "Large Steps in Cloth Simulation" (SIGGRAPH 98)
 *   - Ericson, "Real-Time Collision Detection" (2005, ch. 13)
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#pragma once

#include <TohruPhysics/Geometry.h>

// ===========================================================================
//  Constants
// ===========================================================================

#define ILIA_DEFAULT_BODY_CAPACITY  256
#define ILIA_MAX_BODY_CAPACITY      4096
#define ILIA_PAIR_BUFFER_GROWTH     4096
#define ILIA_SWEEP_AXIS             0  // primary sweep on X

// ===========================================================================
//  0211: Endpoint and axis-list types
// ===========================================================================

// A single sorted endpoint for one body on one axis.
typedef struct {
	Real Value;      // coordinate value (Min or Max on this axis)
	int  BodyIndex;  // which body this endpoint belongs to
	int  IsMin;      // 1 = min (opening), 0 = max (closing)
} IliaSapEndpoint;

// Per-axis sorted endpoint list.
typedef struct {
	IliaSapEndpoint *Endpoints;  // array[Count], sorted by Value
	int              Count;      // 2 * num active bodies
	int              Capacity;   // allocated size
	int              BodyCount;  // number of active bodies
	int              BodyCapacity;
	int             *BodyToIndex; // BodyIndex → index of its min endpoint (or -1)
	int              Dirty;       // non-zero if a full sort is needed
} IliaSapAxisList;

// ===========================================================================
//  0217: Generation-based pair marker
// ===========================================================================

// Compact marker for duplicate pair suppression.
typedef struct {
	short *Data;          // [BodyCapacity × BodyCapacity], stores generation
	short  Generation;    // current frame generation
	int    BodyCapacity;
} IliaSapPairMarker;

// ===========================================================================
//  Main SAP manager
// ===========================================================================

typedef struct {
	IliaSapAxisList    Axes[3];            // X, Y, Z
	IliaSapPairMarker  Marker;
	int               *OverlapBuffer;     // pairs (A, B, A, B, ...)
	int                OverlapCapacity;
	int                OverlapCount;      // number of pairs
	int                BodyCapacity;
} IliaSapManager;

// ===========================================================================
//  0211: Axis list initialisation
// ===========================================================================

// Initialise a single axis list. Returns 0 on success, -1 on alloc failure.
int  IliaSapAxisListInit(IliaSapAxisList *List, int BodyCapacity);

// Free resources.
void IliaSapAxisListDestroy(IliaSapAxisList *List);

// Reset to empty (preserves buffer).
void IliaSapAxisListReset(IliaSapAxisList *List);

// ===========================================================================
//  0212: Insert a body (adds its min/max endpoints)
// ===========================================================================

// Insert BodyIndex into all three axis lists using the given AABB.
// Returns 0 on success, -1 if BodyIndex already present or capacity full.
int IliaSapInsertBody(IliaSapManager *Mgr, int BodyIndex, const AABB *Box);

// ===========================================================================
//  0213: Remove a body
// ===========================================================================

// Remove BodyIndex from all three axis lists.
// Returns 0 on success, -1 if BodyIndex not found.
int IliaSapRemoveBody(IliaSapManager *Mgr, int BodyIndex);

// ===========================================================================
//  0214: Incremental insertion sort (coherent motion)
// ===========================================================================

// Re-sort each axis list using insertion sort (O(N) for nearly-sorted).
// Returns total number of swaps performed, or -1 on error.
int IliaSapUpdateSort(IliaSapManager *Mgr);

// ===========================================================================
//  0215: Sweep one axis to generate overlapping pairs
// ===========================================================================

// Sweep primary axis (X) to find all overlapping pairs, validate on Y/Z.
// Pairs are stored in OverlapBuffer as (BodyA, BodyB, BodyA, BodyB, ...).
// Returns number of pairs found, or -1 on error.
int IliaSapGeneratePairs(IliaSapManager *Mgr);

// ===========================================================================
//  0216: Multi-axis overlap test
// ===========================================================================

// Test whether two bodies overlap on all three axes.
// Returns 1 if overlapping, 0 if separated.
int IliaSapTestOverlap(IliaSapManager *Mgr, int BodyA, int BodyB);

// ===========================================================================
//  0217: Pair marker / duplicate filter
// ===========================================================================

// Init marker. Returns 0 on success.
int  IliaSapPairMarkerInit(IliaSapPairMarker *Mkr, int BodyCapacity);

// Free marker.
void IliaSapPairMarkerDestroy(IliaSapPairMarker *Mkr);

// Reset for new frame (increments generation).
void IliaSapPairMarkerNextFrame(IliaSapPairMarker *Mkr);

// Test and set marker for a pair. Returns 1 if already seen, 0 if new.
// Stores pair in canonical order (smaller BodyIndex first).
int IliaSapPairMarkerTest(IliaSapPairMarker *Mkr, int BodyA, int BodyB);

// ===========================================================================
//  0218: Dimension fallback for aligned objects
// ===========================================================================

// Test overlap on a given axis. Falls back to other axes when two distinct
// bodies have identical projected intervals (uses body index tiebreaker).
// Returns 1 if overlapping on this axis, 0 if separated.
int IliaSapDimensionTest(IliaSapAxisList *List, int BodyA, int BodyB);

// ===========================================================================
//  0219: Batch pair verification against actual AABBs
// ===========================================================================

// Verify all pairs in OverlapBuffer against the given AABB array.
// Removes pairs whose AABBs do not truly overlap (false positives from
// tolerances or stale data). Returns number of surviving pairs.
int IliaSapVerifyPairs(IliaSapManager *Mgr, const AABB *BodyAABBs);

// ===========================================================================
//  0220: Boundary limit checking
// ===========================================================================

// Clamp endpoint values to safe range. Returns number of clamped endpoints.
int IliaSapBoundaryCheck(IliaSapManager *Mgr);

// ===========================================================================
//  Manager-level initialisation
// ===========================================================================

// Initialise SAP manager with given body capacity.
// Returns 0 on success, -1 on alloc failure.
int IliaSapInit(IliaSapManager *Mgr, int BodyCapacity);

// Free all resources.
void IliaSapDestroy(IliaSapManager *Mgr);

// Reset to empty (preserves buffers).
void IliaSapReset(IliaSapManager *Mgr);
