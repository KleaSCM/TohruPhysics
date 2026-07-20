/**
 * Geometry — primitive collision shapes.
 * TohruPhysics用の基本衝突形状よ。
 *
 * Eight primitive types (AABB, Sphere, OBB, Capsule, Plane, Ray, Segment,
 * Triangle) with construction, containment, closest-point, and ray-intersection
 * queries. Every shape is zero-initialisable — a zeroed shape is valid.
 *
 * DESIGN PHILOSOPHY:
 * Collision detection operates on geometric primitives, not meshes.
 * These eight cover the vast majority of real-world physics interactions:
 * - AABB/Sphere: fast broad-phase culling
 * - OBB: oriented objects (boxes, crates)
 * - Capsule: character controllers, ragdoll bones
 * - Plane: infinite ground planes, triggers
 * - Ray: line-of-sight, weapon hits, mouse picking
 * - Segment: cable/wire collision
 * - Triangle: mesh surface representation
 *
 * Each type carries its own query functions rather than a virtual
 * interface — keeps the library C-compatible and avoids vtable overhead.
 *
 * References:
 * - Real-Time Collision Detection (Christer Ericson)
 * - Möller–Trumbore ray-triangle intersection algorithm
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#pragma once

#include <TohruPhysics/Vector3.h>
#include <TohruPhysics/Quaternion.h>

// ---------------------------------------------------------------------------
//  Primitive type definitions
//  基本形状の型定義ね。
// ---------------------------------------------------------------------------

typedef struct { Vector3 Min; Vector3 Max; } AABB;
typedef struct { Vector3 Center; Real Radius; } Sphere;
typedef struct { Vector3 Center; Vector3 HalfExtents; Quaternion Rotation; } OBB;
typedef struct { Vector3 Start; Vector3 End; Real Radius; } Capsule;
typedef struct { Vector3 Normal; Real Distance; } Plane;
typedef struct { Vector3 Origin; Vector3 Direction; } Ray;
typedef struct { Vector3 Start; Vector3 End; } Segment;
typedef struct { Vector3 V0; Vector3 V1; Vector3 V2; } Triangle;

// ===========================================================================
//  Sabina — primitive construction and queries
// ===========================================================================

// ---- AABB ----
AABB    SabinaAABBMake(const Vector3 *Min, const Vector3 *Max);
AABB    SabinaAABBMakeCenterExtents(const Vector3 *Center, const Vector3 *HalfExtents);
AABB    SabinaAABBMerge(const AABB *A, const AABB *B);
AABB    SabinaAABBExpand(const AABB *Box, Real Margin);
Vector3 SabinaAABBCenter(const AABB *Box);
Vector3 SabinaAABBHalfExtents(const AABB *Box);
Real    SabinaAABBSurfaceArea(const AABB *Box);
Real    SabinaAABBVolume(const AABB *Box);
int     SabinaAABBContains(const AABB *Box, const Vector3 *Point);
int     SabinaAABBOverlaps(const AABB *A, const AABB *B);
Vector3 SabinaAABBClosestPoint(const AABB *Box, const Vector3 *P);
int     SabinaAABBIntersectRay(const AABB *Box, const Ray *R, Real *T0, Real *T1);

// ---- Sphere ----
Sphere  SabinaSphereMake(const Vector3 *Center, Real Radius);
Real    SabinaSphereVolume(const Sphere *S);
Real    SabinaSphereSurfaceArea(const Sphere *S);
int     SabinaSphereContains(const Sphere *S, const Vector3 *Point);
int     SabinaSphereOverlaps(const Sphere *A, const Sphere *B);
Vector3 SabinaSphereClosestPoint(const Sphere *S, const Vector3 *P);
int     SabinaSphereIntersectRay(const Sphere *S, const Ray *R, Real *T0, Real *T1);

// ---- OBB ----
OBB     SabinaOBBMake(const Vector3 *Center, const Vector3 *HalfExtents, const Quaternion *Rotation);
int     SabinaOBBContains(const OBB *Box, const Vector3 *Point);
Vector3 SabinaOBBClosestPoint(const OBB *Box, const Vector3 *P);
void    SabinaOBBGetCorners(const OBB *Box, Vector3 Corners[8]);

// ---- Capsule ----
Capsule SabinaCapsuleMake(const Vector3 *Start, const Vector3 *End, Real Radius);
int     SabinaCapsuleContains(const Capsule *C, const Vector3 *P);
Vector3 SabinaCapsuleClosestPoint(const Capsule *C, const Vector3 *P);
Real    SabinaCapsuleLength(const Capsule *C);

// ---- Plane ----
Plane   SabinaPlaneMake(const Vector3 *Normal, Real Distance);
Plane   SabinaPlaneMakeFromPoints(const Vector3 *A, const Vector3 *B, const Vector3 *C);
Plane   SabinaPlaneNormalize(const Plane *P);
Real    SabinaPlaneSignedDistance(const Plane *P, const Vector3 *Point);
Real    SabinaPlaneDistance(const Plane *P, const Vector3 *Point);
Vector3 SabinaPlaneReflect(const Plane *P, const Vector3 *V);
Vector3 SabinaPlaneProject(const Plane *P, const Vector3 *V);

// ---- Ray ----
Ray     SabinaRayMake(const Vector3 *Origin, const Vector3 *Direction);
Vector3 SabinaRayPointAt(const Ray *R, Real T);
Real    SabinaRayClosestT(const Ray *R, const Vector3 *P);
Real    SabinaRayIntersectPlane(const Ray *R, const Plane *P);
int     SabinaRayIntersectSphere(const Ray *R, const Sphere *S, Real *T0, Real *T1);
int     SabinaRayIntersectAABB(const Ray *R, const AABB *Box, Real *T0, Real *T1);
int     SabinaRayIntersectTriangle(const Ray *R, const Triangle *T, Real *TOut);

// ---- Segment ----
Segment SabinaSegmentMake(const Vector3 *Start, const Vector3 *End);
Real    SabinaSegmentLength(const Segment *S);
Real    SabinaSegmentLengthSq(const Segment *S);
Vector3 SabinaSegmentClosestPoint(const Segment *S, const Vector3 *P);
Real    SabinaSegmentClosestT(const Segment *S, const Vector3 *P);
Real    SabinaSegmentDistanceToPoint(const Segment *S, const Vector3 *P);
Real    SabinaSegmentClosestPointBetween(const Segment *S1, const Segment *S2, Vector3 *P1, Vector3 *P2);

// ---- Triangle ----
Triangle SabinaTriangleMake(const Vector3 *V0, const Vector3 *V1, const Vector3 *V2);
Vector3  SabinaTriangleNormal(const Triangle *T);
Real     SabinaTriangleArea(const Triangle *T);
Real     SabinaTrianglePerimeter(const Triangle *T);
Vector3  SabinaTriangleClosestPoint(const Triangle *T, const Vector3 *P);
void     SabinaTriangleBarycentric(const Triangle *T, const Vector3 *P, Real *U, Real *V, Real *W);
int      SabinaTriangleContains(const Triangle *T, const Vector3 *P);
int      SabinaTriangleIntersectRay(const Triangle *T, const Ray *R, Real *TOut);

// ===========================================================================
//  Intersect — cross-type intersection tests (Section 1.12)
//  クロスタイプ交差テストね。
// ===========================================================================
//  Intersect — cross-type intersection tests (Section 1.12)
// ===========================================================================

int IntersectSphereSphere(const Sphere *A, const Sphere *B);
int IntersectSphereAABB(const Sphere *S, const AABB *Box);
int IntersectSpherePlane(const Sphere *S, const Plane *P);
int IntersectSphereCapsule(const Sphere *S, const Capsule *C);
int IntersectAABBAABB(const AABB *A, const AABB *B);
int IntersectAABBPlane(const AABB *Box, const Plane *P);
int IntersectAABBOBB(const AABB *Box, const OBB *O);
int IntersectOBBOBB(const OBB *A, const OBB *B);
int IntersectOBBPlane(const OBB *O, const Plane *P);
int IntersectCapsuleCapsule(const Capsule *A, const Capsule *B);

// ===========================================================================
//  IntersectRay*/IntersectSegment* — advanced ray/segment intersection (1.13)
// ===========================================================================

// 0111
int IntersectRaySphere(const Ray *R, const Sphere *S, Real *T0, Real *T1);
// 0112
int IntersectRayAABB(const Ray *R, const AABB *Box, Real *T0, Real *T1);
// 0113
int IntersectRayOBB(const Ray *R, const OBB *O, Real *T0, Real *T1);
// 0114
int IntersectRayPlane(const Ray *R, const Plane *P, Real *TOut);
// 0115
int IntersectRayCapsule(const Ray *R, const Capsule *C, Real *T0, Real *T1);
// 0116
int IntersectRayTriangle(const Ray *R, const Triangle *T, Real *TOut);
// 0117
int IntersectSegmentSphere(const Segment *Seg, const Sphere *S, Real *T0, Real *T1);
// 0118
int IntersectSegmentAABB(const Segment *Seg, const AABB *Box, Real *T0, Real *T1);
// 0119
int IntersectSegmentOBB(const Segment *Seg, const OBB *O, Real *T0, Real *T1);
// 0120
int IntersectSegmentPlane(const Segment *Seg, const Plane *P, Real *TOut);

// ===========================================================================
//  ClosestPoint / Distance queries (Section 1.14)
// ===========================================================================

// 0121
Vector3 ClosestPointPointSphere(const Sphere *S, const Vector3 *P);
// 0122
Vector3 ClosestPointPointAABB(const AABB *Box, const Vector3 *P);
// 0123
Vector3 ClosestPointPointOBB(const OBB *Box, const Vector3 *P);
// 0124
Vector3 ClosestPointPointCapsule(const Capsule *C, const Vector3 *P);
// 0125
Vector3 ClosestPointPointPlane(const Plane *P, const Vector3 *Pt);
// 0126
Vector3 ClosestPointPointTriangle(const Triangle *T, const Vector3 *P);
// 0127
Real    ClosestPointSegmentSegment(const Segment *S1, const Segment *S2, Vector3 *P1, Vector3 *P2);
// 0128
Real    DistanceSphereSphere(const Sphere *A, const Sphere *B);
// 0129
Real    DistanceAABBAABB(const AABB *A, const AABB *B);
// 0130
Real    DistanceOBBOBB(const OBB *A, const OBB *B);

// ===========================================================================
//  Additional shape types (for support functions)
// ===========================================================================
typedef struct {
	Vector3 Center;
	Real    Radius;
	Real    HalfHeight;
} Cylinder;

typedef struct {
	Vector3 Center;
	Real    Radius;
	Real    HalfHeight;
} Cone;

typedef struct {
	Vector3 *Vertices;
	int      VertexCount;
} ConvexHull;

// ===========================================================================
//  Support functions (Section 1.15) — farthest point along direction
//  サポート関数 — 方向に沿った最遠点を返すの。
// ===========================================================================

// 0131
Vector3 SupportSphere(const Sphere *S, const Vector3 *Dir);
// 0132
Vector3 SupportAABB(const AABB *Box, const Vector3 *Dir);
// 0133
Vector3 SupportOBB(const OBB *Box, const Vector3 *Dir);
// 0134
Vector3 SupportCapsule(const Capsule *C, const Vector3 *Dir);
// 0135
Vector3 SupportConvexHull(const ConvexHull *Hull, const Vector3 *Dir);
// 0136
Vector3 SupportTriangleMesh(const Triangle *Triangles, int TriCount, const Vector3 *Dir);
// 0137
Vector3 SupportCompoundShape(const void **Shapes, const int *Types, int Count, const Vector3 *Dir);
// 0138
Vector3 SupportHeightField(const Real *Heights, int Width, int Depth, const Vector3 *Dir);
// 0139
Vector3 SupportCylinder(const Cylinder *C, const Vector3 *Dir);
// 0140
Vector3 SupportCone(const Cone *C, const Vector3 *Dir);
