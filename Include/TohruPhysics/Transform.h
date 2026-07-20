/**
 * Transform type — spatial position + rotation.
 * トランスフォーム型 — 空間位置＋回転よ。
 *
 * ZII: every operation returns a valid Transform/Vector3.
 * ZII: 全ての操作は有効な Transform/Vector3 を返すわ。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#pragma once

#include <TohruPhysics/Vector3.h>
#include <TohruPhysics/Quaternion.h>

// ---------------------------------------------------------------------------
//  0062: Transform — position and rotation.
//  位置と回転。
// ---------------------------------------------------------------------------
typedef struct {
	Vector3  Position;
	Quaternion Rotation;
} Transform;

// ===========================================================================
//  Kaede — Transform operations
// ===========================================================================

// Identity / constructors
Transform KaedeTransformIdentity(void);
Transform KaedeTransformMake(const Vector3 *Position, const Quaternion *Rotation);

// Point conversion
Vector3 KaedeTransformPoint(const Transform *Tfm, const Vector3 *P);
Vector3 KaedeInverseTransformPoint(const Transform *Tfm, const Vector3 *P);

// Direction conversion (no translation)
Vector3 KaedeTransformDirection(const Transform *Tfm, const Vector3 *D);
Vector3 KaedeInverseTransformDirection(const Transform *Tfm, const Vector3 *D);

// Combine / Inverse
Transform KaedeTransformCombine(const Transform *A, const Transform *B);
Transform KaedeTransformInverse(const Transform *Tfm);

// Setters
void KaedeTransformSetPosition(Transform *Tfm, const Vector3 *P);
void KaedeTransformSetRotation(Transform *Tfm, const Quaternion *R);

// LookAt
void KaedeTransformLookAt(Transform *Tfm, const Vector3 *Target, const Vector3 *Up);
