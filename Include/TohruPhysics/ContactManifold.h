/**
 * ContactManifold — contact point generation for collision pairs.
 * TohruPhysics用の接触点生成よ。
 *
 * Takes two overlapping shapes (transformed to world space) and produces
 * up to 4 contact points forming a manifold. Simple shapes (sphere, capsule)
 * generate a single contact point from closest-point queries. Box-like shapes
 * (AABB, OBB) use SAT-based clipping (Sutherland–Hodgman) to produce up to
 * 4 contact points for stable face-face contacts.
 *
 * DESIGN PHILOSOPHY:
 * A single contact point per overlapping pair causes jitter and edge-on-edge
 * instability. A manifold with up to 4 points spreads the contact force across
 * an area, giving the constraint solver robust leverage.
 *
 * Clipping approach (Sutherland–Hodgman):
 * 1. SAT finds the collision axis (least-penetrating normal).
 * 2. Reference body = face most anti-aligned with normal; incident body = other.
 * 3. Incident face vertices are clipped against the reference face's side planes.
 * 4. Remaining points (up to 4) form the manifold.
 *
 * For non-polyhedral shapes (sphere, capsule), a single point from the
 * closest-point query suffices — the contact normal varies smoothly.
 *
 * MANIFOLD LAYOUT:
 * ┌──────────┬────────────────────────────────────────────────────┐
 * │ Points[] │ Up to 4 ContactPoint structures                    │
 * │ Count    │ Number of valid points (0–4)                       │
 * │ Normal   │ Shared contact normal (B → A)                      │
 * │ Penetr.  │ Maximum penetration depth in manifold              │
 * └──────────┴────────────────────────────────────────────────────┘
 *
 * References:
 * - Sutherland–Hodgman polygon clipping (1974)
 * - Box2D contact manifold generation (Erin Catto)
 * - Bullet Physics btManifoldResult
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#pragma once

#include <TohruPhysics/Vector3.h>
#include <TohruPhysics/Transform.h>
#include <TohruPhysics/Geometry.h>
#include <TohruPhysics/GJK.h>

#define MANIFOLD_MAX_POINTS 4

// ---------------------------------------------------------------------------
//  ContactPoint — single contact point with warm-start data
//  接触点 — ウォームスタートデータ付きね。
// ---------------------------------------------------------------------------
typedef struct {
	Vector3 Position;          // world-space contact position
	Vector3 Normal;            // contact normal (from B toward A)
	Real    Penetration;       // overlap depth (positive = penetrating)

	// Warm-start impulses (accumulated from previous frame)
	// ウォームスタートの力積（前フレームからの累積）
	Real    NormalImpulse;
	Real    TangentImpulse[2];

	// Feature tags for persistent contact matching
	// 持続接触マッチング用の特徴タグ
	int     FeatureA;          // feature index on body A
	int     FeatureB;          // feature index on body B
	int     FeatureTypeA;      // 0=vertex, 1=edge, 2=face
	int     FeatureTypeB;
} ContactPoint;

// ---------------------------------------------------------------------------
//  ContactManifold — up to 4 points between a body pair
//  接触マニフォールド — ボディペア間の最大4点よ。
// ---------------------------------------------------------------------------
typedef struct {
	ContactPoint Points[MANIFOLD_MAX_POINTS];
	int          PointCount;
	int          BodyA;        // body index A
	int          BodyB;        // body index B
	Vector3      Normal;       // shared contact normal (B → A)
	Real         Penetration;  // maximum penetration depth
} ContactManifold;

// ===========================================================================
//  Manifold generation functions (Section 1.19)
//  マニフォールド生成関数ね。
// ===========================================================================

// 0171: Sphere-Sphere
void ManifoldSphereSphere(
	const Sphere *SphA, const Transform *TxA,
	const Sphere *SphB, const Transform *TxB,
	ContactManifold *M);

// 0172: Sphere-AABB
void ManifoldSphereAABB(
	const Sphere *Sph, const Transform *TxS,
	const AABB *Box, const Transform *TxB,
	ContactManifold *M);

// 0173: Sphere-OBB
void ManifoldSphereOBB(
	const Sphere *Sph, const Transform *TxS,
	const OBB *O, const Transform *TxO,
	ContactManifold *M);

// 0174: Sphere-Capsule
void ManifoldSphereCapsule(
	const Sphere *Sph, const Transform *TxS,
	const Capsule *Cap, const Transform *TxC,
	ContactManifold *M);

// 0175: AABB-AABB
void ManifoldAABBAABB(
	const AABB *BoxA, const Transform *TxA,
	const AABB *BoxB, const Transform *TxB,
	ContactManifold *M);

// 0176: AABB-OBB
void ManifoldAABBOBB(
	const AABB *Box, const Transform *TxBox,
	const OBB *O, const Transform *TxO,
	ContactManifold *M);

// 0177: OBB-OBB (SAT + Sutherland–Hodgman clipping)
void ManifoldOBBOBB(
	const OBB *OBB_A, const Transform *TxA,
	const OBB *OBB_B, const Transform *TxB,
	ContactManifold *M);

// 0178: Capsule-Capsule
void ManifoldCapsuleCapsule(
	const Capsule *CapA, const Transform *TxA,
	const Capsule *CapB, const Transform *TxB,
	ContactManifold *M);

// 0179: General convex-convex via GJK+EPA
void ManifoldConvexConvex(
	const void *ShapeA, GJKSupportFn SupportA,
	const void *ShapeB, GJKSupportFn SupportB,
	Real MaxPenetration,
	ContactManifold *M);

// 0180: Clip polygon against a plane (Sutherland–Hodgman step)
int ClipPolygonToPlane(
	const Vector3 *InVerts, int InCount,
	const Plane *ClipPlane,
	Vector3 *OutVerts, int MaxOut);
