/**
 * GJK — Gilbert-Johnson-Keerthi distance algorithm.
 * TohruPhysics用のGJK距離アルゴリズムね。
 *
 * Computes the minimum distance between two convex shapes using support
 * functions and simplex refinement in Minkowski difference space.
 *
 * DESIGN PHILOSOPHY:
 * GJK iteratively builds a simplex in the Minkowski difference set
 * A ⊖ B = {a − b | a ∈ A, b ∈ B}. Each iteration:
 * 1. Computes support point along current search direction
 * 2. Adds it to the simplex
 * 3. Finds the closest point on the simplex to the origin
 * 4. Updates search direction toward origin
 * 5. Checks termination (new support doesn't improve distance)
 *
 * The simplex evolves through four stages:
 * ┌──────────┬──────┬────────────────────────────────────────────┐
 * │ State    │ Pts  │ Detection                                  │
 * ├──────────┼──────┼────────────────────────────────────────────┤
 * │ Vertex   │ 1    │ Single point, origin at zero               │
 * │ Edge     │ 2    │ Line segment, origin projection            │
 * │ Triangle │ 3    │ Triangle, origin barycentric                │
 * │ Tetra  | 4    │ Tetrahedron, origin containment             │
 * └──────────┴──────┴────────────────────────────────────────────┘
 *
 * SIMPLEX LAYOUT:
 * Points[A..D] + Directions[A..D] store the support point and the
 * direction that generated it. Count indicates how many are valid.
 *
 * References:
 * - Gilbert, Johnson, Keerthi, "A fast procedure for computing the
 *   distance between objects in three-dimensional space" (1988)
 * - Gino van den Bergen, "Collision Detection in Interactive 3D
 *   Environments" (Chapter 4: GJK)
 * - Real-Time Collision Detection (Christer Ericson), Chapter 9
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#pragma once

#include <TohruPhysics/Vector3.h>

#define GJK_MAX_ITERATIONS 64
#define GJK_SIMPLEX_SIZE 4

// ---------------------------------------------------------------------------
//  Simplex vertex — support point + which direction generated it
//  シンプレックス頂点 — サポート点＋それを生成した方向ね。
// ---------------------------------------------------------------------------
typedef struct {
	Vector3 Point;
	Vector3 Direction;
} GJKVertex;

// ---------------------------------------------------------------------------
//  GJK state — distance result + simplex + iteration tracking
//  GJK状態 — 距離結果＋シンプレックス＋反復追跡ね。
// ---------------------------------------------------------------------------
typedef struct {
	Real      DistanceSq;       // squared distance between shapes
	Real      Tolerance;
	int       Iterations;
	int       MaxIterations;
	int       Converged;        // 1 if GJK converged, 0 if max iterations
	int       Degenerate;       // 1 if shapes are degenerate/overlapping
	int       SimplexCount;     // number of valid simplex points (1-4)
	GJKVertex Simplex[GJK_SIMPLEX_SIZE];
} GJKState;

// ---------------------------------------------------------------------------
//  SupportFunction — callback type for arbitrary shapes
//  サポート関数 — 任意形状のコールバック型ね。
// ---------------------------------------------------------------------------
typedef Vector3 (*GJKSupportFn)(const void *Shape, const Vector3 *Dir);

// ===========================================================================
//  GJK functions
// ===========================================================================

// 0141: Initialise GJK with initial search direction and support point.
void GJKInit(GJKState *State, const Vector3 *InitialDir,
             const void *ShapeA, GJKSupportFn SupportA,
             const void *ShapeB, GJKSupportFn SupportB,
             Real Tolerance, int MaxIter);

// 0142–0149: Run the full GJK iteration loop.
void GJKEvaluate(GJKState *State,
                 const void *ShapeA, GJKSupportFn SupportA,
                 const void *ShapeB, GJKSupportFn SupportB);

// Helper: compute closest point on simplex to origin
Vector3 GJKClosestPointOnSimplex(const GJKState *State);

// ===========================================================================
//  EPA — Expanding Polytope Algorithm (Section 1.17)
//  Takes GJK's final simplex (origin inside tetrahedron) and expands outward
//  to find the closest face → penetration depth + contact normal + point.
// ===========================================================================

#define EPA_MAX_ITERATIONS 64
#define EPA_MAX_FACES 128
#define EPA_MAX_VERTICES 64

typedef struct {
	Vector3 Normal;     // face normal (outward)
	Real    Distance;   // distance from origin to face plane
	int     Indices[3]; // vertex indices in EPAState.Vertices[]
	int     Valid;       // 1 if this face is still active
} EPAFace;

typedef struct {
	Vector3   Vertices[EPA_MAX_VERTICES];
	int       VertexCount;
	EPAFace   Faces[EPA_MAX_FACES];
	int       FaceCount;
	Real      Tolerance;
	int       Iterations;
	int       MaxIterations;
	int       Converged;

	// Results
	Real      PenetrationDepth;
	Vector3   ContactNormal;
	Vector3   ContactPoint;
} EPAState;

// 0151: Initialise EPA from GJK's final simplex
void EPAInit(EPAState *E, const GJKState *G, Real Tolerance, int MaxIter);

// 0152–0159: Run EPA iteration loop
void EPAEvaluate(EPAState *E,
                 const void *ShapeA, GJKSupportFn SupportA,
                 const void *ShapeB, GJKSupportFn SupportB);
