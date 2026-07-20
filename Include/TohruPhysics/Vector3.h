/**
 * Vector3 — contiguous 3-element vector for TohruPhysics.
 * TohruPhysics用の連続3要素ベクトルね。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#pragma once

#include <TohruPhysics/Math.h>

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

// 0024
Vector3 KannaVector3Add(const Vector3 *A, const Vector3 *B);
// 0025
Vector3 KannaVector3Sub(const Vector3 *A, const Vector3 *B);
// 0026
Vector3 KannaVector3Scale(const Vector3 *V, Real S);
// 0027
Real    KannaVector3Dot(const Vector3 *A, const Vector3 *B);
// 0028
Vector3 KannaVector3Cross(const Vector3 *A, const Vector3 *B);
// 0029
Real    KannaVector3LengthSq(const Vector3 *V);
// 0030
Vector3 KannaVector3Normalize(const Vector3 *V);
// 0031
Real    KannaVector3Dist(const Vector3 *A, const Vector3 *B);
// 0032
int     KannaVector3Equal(const Vector3 *A, const Vector3 *B);

// ---------------------------------------------------------------------------
//  Convenience constructors.
//  便利なコンストラクターね。
// ---------------------------------------------------------------------------
Vector3 KannaVector3Make(Real X, Real Y, Real Z);
