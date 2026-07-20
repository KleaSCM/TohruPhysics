/**
 * SAT — Separating Axis Theorem collision detection.
 * TohruPhysics用の分離軸定理衝突検出ね。
 *
 * Tests convex shapes for overlap by projecting onto potential separating axes.
 * If any axis separates the shapes, they don't intersect. If all axes show
 * overlap, the shapes intersect and the minimum overlap axis gives the
 * penetration depth and contact normal.
 *
 * DESIGN PHILOSOPHY:
 * SAT is the primary algorithm for box-box and convex-convex collision.
 * For OBB-OBB we test 15 axes: 3 face normals from each box + 9 edge
 * cross products. Early exit on first separating axis (0165). When all
 * axes overlap, the axis with minimum overlap gives penetration depth.
 *
 * AXIS COUNT BY PAIR:
 * ┌─────────────────┬──────┬────────────────────────────────────────────┐
 * │ Pair            │ Axes │ Composition                                │
 * ├─────────────────┼──────┼────────────────────────────────────────────┤
 * │ AABB-AABB       │ 3    │ World X, Y, Z                             │
 * │ AABB-OBB        │ 15   │ 3 world + 3 OBB face + 9 edge cross       │
 * │ OBB-OBB         │ 15   │ 3+3 face + 9 edge cross                  │
 * │ Poly2D-Poly2D   │ N    │ Edge normals of N-gon                     │
 * │ Poly3D-Poly3D   │ F+E  │ Face normals + edge cross products        │
 * └─────────────────┴──────┴────────────────────────────────────────────┘
 *
 * References:
 * - Real-Time Collision Detection (Ericson), Chapter 4.4
 * - David Eberly, "Dynamic Collision Detection using Oriented Bounding Boxes"
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#pragma once

#include <TohruPhysics/Geometry.h>

// 2D vector for polygon support
typedef struct { Real X, Y; } Vector2;

#define SAT_MAX_AXES 32
#define SAT_MAX_VERTICES 64

// ---------------------------------------------------------------------------
//  SAT result
// ---------------------------------------------------------------------------
typedef struct {
	int   Intersect;         // 1 if shapes intersect
	Real  PenetrationDepth;  // minimum overlap depth (0 if separated)
	Vector3 Normal;           // contact normal (along minimum overlap axis)
	Vector3 ContactPoint;     // point in the overlap region
	int   AxisCount;         // number of axes tested
	Real  Overlaps[SAT_MAX_AXES]; // overlap distance per axis
} SATResult;

// ---------------------------------------------------------------------------
//  0161: OBB-OBB with full 15-axis SAT
// ---------------------------------------------------------------------------
SATResult SintoOBBOBB(const OBB *A, const OBB *B);

//  0162: AABB-OBB with 15-axis SAT
SATResult SintoAABBOBB(const AABB *Box, const OBB *O);

//  0163: 2D convex polygon SAT
SATResult SintoPolyPoly2D(const Vector2 *VertsA, int CountA,
                           const Vector2 *VertsB, int CountB);

//  0164: 3D convex polyhedron SAT
SATResult SintoPolyPoly3D(const Vector3 *VertsA, int CountA,
                           const Vector3 *VertsB, int CountB);
