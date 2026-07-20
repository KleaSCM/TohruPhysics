/**
 * Primitive geometric shapes for TohruPhysics.
 * TohruPhysics用の基本幾何形状よ。
 *
 * ZII: every shape is zero-initializable. Zero is valid.
 * ZII: 全ての形状はゼロ初期化可能。ゼロは有効よ。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#pragma once

#include <TohruPhysics/Vector3.h>

// ---------------------------------------------------------------------------
//  0073: AABB — axis-aligned bounding box.
// ---------------------------------------------------------------------------
typedef struct {
	Vector3 Min;
	Vector3 Max;
} AABB;

//  0074: Sphere
// ---------------------------------------------------------------------------
typedef struct {
	Vector3 Center;
	Real    Radius;
} Sphere;

//  0075: OBB — oriented bounding box.
// ---------------------------------------------------------------------------
typedef struct {
	Vector3   Center;
	Vector3   HalfExtents; // half-width along local X/Y/Z
	Quaternion Rotation;
} OBB;

//  0076: Capsule
// ---------------------------------------------------------------------------
typedef struct {
	Vector3 Start;
	Vector3 End;
	Real    Radius;
} Capsule;

//  0077: Plane — n·x = d
// ---------------------------------------------------------------------------
typedef struct {
	Vector3 Normal;  // unit length
	Real    Distance;
} Plane;

//  0078: Ray — origin + direction
// ---------------------------------------------------------------------------
typedef struct {
	Vector3 Origin;
	Vector3 Direction; // unit length
} Ray;

//  0079: Segment
// ---------------------------------------------------------------------------
typedef struct {
	Vector3 Start;
	Vector3 End;
} Segment;

//  0080: Triangle
// ---------------------------------------------------------------------------
typedef struct {
	Vector3 V0;
	Vector3 V1;
	Vector3 V2;
} Triangle;

// ===========================================================================
//  Sabina — primitive constructors and queries
// ===========================================================================

// AABB
AABB    SabinaAABBMake(const Vector3 *Min, const Vector3 *Max);
AABB    SabinaAABBMakeCenterExtents(const Vector3 *Center, const Vector3 *HalfExtents);
int     SabinaAABBContains(const AABB *Box, const Vector3 *Point);
int     SabinaAABBOverlaps(const AABB *A, const AABB *B);

// Sphere
Sphere  SabinaSphereMake(const Vector3 *Center, Real Radius);
int     SabinaSphereContains(const Sphere *S, const Vector3 *Point);
int     SabinaSphereOverlaps(const Sphere *A, const Sphere *B);

// OBB
OBB     SabinaOBBMake(const Vector3 *Center, const Vector3 *HalfExtents, const Quaternion *Rotation);
int     SabinaOBBContains(const OBB *Box, const Vector3 *Point);

// Capsule
Capsule SabinaCapsuleMake(const Vector3 *Start, const Vector3 *End, Real Radius);
Vector3 SabinaCapsuleClosestPoint(const Capsule *C, const Vector3 *P);

// Plane
Plane   SabinaPlaneMake(const Vector3 *Normal, Real Distance);
Plane   SabinaPlaneMakeFromPoints(const Vector3 *A, const Vector3 *B, const Vector3 *C);
Real    SabinaPlaneSignedDistance(const Plane *P, const Vector3 *Point);

// Ray
Ray     SabinaRayMake(const Vector3 *Origin, const Vector3 *Direction);
Vector3 SabinaRayPointAt(const Ray *R, Real T);
Real    SabinaRayClosestPoint(const Ray *R, const Vector3 *P);

// Segment
Segment SabinaSegmentMake(const Vector3 *Start, const Vector3 *End);
Real    SabinaSegmentLength(const Segment *S);
Vector3 SabinaSegmentClosestPoint(const Segment *S, const Vector3 *P);

// Triangle
Triangle SabinaTriangleMake(const Vector3 *V0, const Vector3 *V1, const Vector3 *V2);
Vector3  SabinaTriangleNormal(const Triangle *T);
Real     SabinaTriangleArea(const Triangle *T);
