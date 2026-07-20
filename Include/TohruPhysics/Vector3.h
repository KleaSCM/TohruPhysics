/**
 * Vector3 — contiguous 3-element vector for TohruPhysics.
 * TohruPhysics用の連続3要素ベクトルね。
 *
 * ZII: every operation returns a valid Vector3.
 * ZII: 全ての操作は有効な Vector3 を返すわ。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#pragma once

#include <TohruPhysics/Math.h>

#define VECTOR3_ZERO_INIT {0.0, 0.0, 0.0}

// ---------------------------------------------------------------------------
//  0023: Vector3 — raw array layout.
//  生の配列レイアウトよ。
// ---------------------------------------------------------------------------
typedef struct {
	Real Data[3];
} Vector3;

// ---------------------------------------------------------------------------
//  Kanna — Vector3 core operations.
//  Vector3の基本操作ね。
// ---------------------------------------------------------------------------

// 0024–0032: Core arithmetic
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

// Component-wise operations
Vector3 KannaVector3Min(const Vector3 *A, const Vector3 *B);
Vector3 KannaVector3Max(const Vector3 *A, const Vector3 *B);
Vector3 KannaVector3Abs(const Vector3 *V);
Vector3 KannaVector3Negate(const Vector3 *V);
Vector3 KannaVector3PerComponentMul(const Vector3 *A, const Vector3 *B);

// Interpolation and blending
Vector3 KannaVector3Lerp(const Vector3 *A, const Vector3 *B, Real T);
Vector3 KannaVector3Slerp(const Vector3 *A, const Vector3 *B, Real T);

// Reflection and projection
Vector3 KannaVector3Reflect(const Vector3 *V, const Vector3 *Normal);
Vector3 KannaVector3Project(const Vector3 *V, const Vector3 *Onto);
Vector3 KannaVector3Reject(const Vector3 *V, const Vector3 *Onto);
Vector3 KannaVector3ProjectOnPlane(const Vector3 *V, const Vector3 *PlaneNormal);

// Geometric queries
Real    KannaVector3Angle(const Vector3 *A, const Vector3 *B);
int     KannaVector3IsUnit(const Vector3 *V, Real Eps);
int     KannaVector3IsZero(const Vector3 *V);
Vector3 KannaVector3Midpoint(const Vector3 *A, const Vector3 *B);
Vector3 KannaVector3DirectionFromTo(const Vector3 *From, const Vector3 *To);

// Clamping
Vector3 KannaVector3ClampLength(const Vector3 *V, Real MaxLen);
Vector3 KannaVector3Clamp(const Vector3 *V, const Vector3 *Lo, const Vector3 *Hi);

// Orthogonal basis helpers
Vector3 KannaVector3Orthogonal(const Vector3 *V);
