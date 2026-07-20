/**
 * Primitive geometric shapes for TohruPhysics.
 * TohruPhysics用の基本幾何形状よ。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#pragma once

#include <TohruPhysics/Vector3.h>
#include <TohruPhysics/Quaternion.h>

// ---------------------------------------------------------------------------
//  0073: AABB — axis-aligned bounding box.
// ---------------------------------------------------------------------------
typedef struct {
	Vector3 Min;
	Vector3 Max;
} AABB;

//  0074: Sphere
typedef struct {
	Vector3 Center;
	Real    Radius;
} Sphere;

//  0075: OBB — oriented bounding box.
typedef struct {
	Vector3   Center;
	Vector3   HalfExtents;
	Quaternion Rotation;
} OBB;

//  0076: Capsule
typedef struct {
	Vector3 Start;
	Vector3 End;
	Real    Radius;
} Capsule;

//  0077: Plane — n·x = d
typedef struct {
	Vector3 Normal;
	Real    Distance;
} Plane;

//  0078: Ray — origin + direction (unit)
typedef struct {
	Vector3 Origin;
	Vector3 Direction;
} Ray;

//  0079: Segment
typedef struct {
	Vector3 Start;
	Vector3 End;
} Segment;

//  0080: Triangle
typedef struct {
	Vector3 V0;
	Vector3 V1;
	Vector3 V2;
} Triangle;

// ===========================================================================
//  Sabina — primitive constructors and queries
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
Real    SabinaCapsuleSegmentLengthSq(const Capsule *C);

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
