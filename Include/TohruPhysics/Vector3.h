/**
 * Vector3 — contiguous 3-element vector.
 * TohruPhysics用の連続3要素ベクトルね。
 *
 * Raw Real[3] array layout. All operations return valid results;
 * zero vector in → zero vector out (never NaN).
 *
 * DESIGN PHILOSOPHY:
 * Struct-of-arrays (SoA) and array-of-structs (AoS) both have merit
 * depending on access pattern. We start with AoS (Vector3 struct)
 * for simplicity. The layout is explicitly 3× contiguous Reals,
 * castable to `Real*` for batch/SIMD processing later.
 *
 * DATA LAYOUT:
 * ┌──────────┬──────────┬──────────┐
 * │ Data[0]  │ Data[1]  │ Data[2]  │
 * │    X     │    Y     │    Z     │
 * └──────────┴──────────┴──────────┘
 *  Offset 0  Offset 8  Offset 16  (sizeof(Real) = 8)
 *
 * References:
 * - Standard 3D vector conventions (right-handed coordinate system)
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#pragma once

#include <TohruPhysics/Math.h>

#define VECTOR3_ZERO_INIT {0.0, 0.0, 0.0}

typedef struct {
	Real Data[3];
} Vector3;

Vector3 KannaVector3Make(Real X, Real Y, Real Z);
Vector3 KannaVector3Zero(void);
Vector3 KannaVector3One(void);
Vector3 KannaVector3UnitX(void);
Vector3 KannaVector3UnitY(void);
Vector3 KannaVector3UnitZ(void);

Vector3 KannaVector3Add(const Vector3 *A, const Vector3 *B);
Vector3 KannaVector3Sub(const Vector3 *A, const Vector3 *B);
Vector3 KannaVector3Scale(const Vector3 *V, Real S);
Real    KannaVector3Dot(const Vector3 *A, const Vector3 *B);
Vector3 KannaVector3Cross(const Vector3 *A, const Vector3 *B);
Real    KannaVector3LengthSq(const Vector3 *V);
Real    KannaVector3Length(const Vector3 *V);
Vector3 KannaVector3Normalize(const Vector3 *V);
Real    KannaVector3Distance(const Vector3 *A, const Vector3 *B);
Real    KannaVector3DistanceSq(const Vector3 *A, const Vector3 *B);
int     KannaVector3Equal(const Vector3 *A, const Vector3 *B);

Vector3 KannaVector3Min(const Vector3 *A, const Vector3 *B);
Vector3 KannaVector3Max(const Vector3 *A, const Vector3 *B);
Vector3 KannaVector3Abs(const Vector3 *V);
Vector3 KannaVector3Negate(const Vector3 *V);
Vector3 KannaVector3PerComponentMul(const Vector3 *A, const Vector3 *B);

Vector3 KannaVector3Lerp(const Vector3 *A, const Vector3 *B, Real T);
Vector3 KannaVector3Slerp(const Vector3 *A, const Vector3 *B, Real T);

Vector3 KannaVector3Reflect(const Vector3 *V, const Vector3 *Normal);
Vector3 KannaVector3Project(const Vector3 *V, const Vector3 *Onto);
Vector3 KannaVector3Reject(const Vector3 *V, const Vector3 *Onto);
Vector3 KannaVector3ProjectOnPlane(const Vector3 *V, const Vector3 *PlaneNormal);

Real    KannaVector3Angle(const Vector3 *A, const Vector3 *B);
int     KannaVector3IsUnit(const Vector3 *V, Real Eps);
int     KannaVector3IsZero(const Vector3 *V);
Vector3 KannaVector3Midpoint(const Vector3 *A, const Vector3 *B);
Vector3 KannaVector3DirectionFromTo(const Vector3 *From, const Vector3 *To);

Vector3 KannaVector3ClampLength(const Vector3 *V, Real MaxLen);
Vector3 KannaVector3Clamp(const Vector3 *V, const Vector3 *Lo, const Vector3 *Hi);

Vector3 KannaVector3Orthogonal(const Vector3 *V);
