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

Transform KaedeTransformIdentity(void);

// 0063: Local → world: P' = R·P + T
Vector3 KaedeTransformPoint(const Transform *Tfm, const Vector3 *P);

// 0064: World → local: P = R⁻¹·(P' − T)
Vector3 KaedeInverseTransformPoint(const Transform *Tfm, const Vector3 *P);

// 0065: Rotate direction only (no translation)
Vector3 KaedeTransformDirection(const Transform *Tfm, const Vector3 *D);

// 0066: Inverse rotate direction
Vector3 KaedeInverseTransformDirection(const Transform *Tfm, const Vector3 *D);

// 0067: Combine: C = A * B
//   Position:  C.P = A.P + A.R · B.P
//   Rotation:  C.R = A.R · B.R
Transform KaedeTransformCombine(const Transform *A, const Transform *B);
