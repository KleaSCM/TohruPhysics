/**
 * Transform — spatial position + orientation.
 * TohruPhysics用の空間位置＋回転よ。
 *
 * Combines a Vector3 position and unit Quaternion rotation.
 * Supports forward/inverse point and direction transforms.
 *
 * DESIGN PHILOSOPHY:
 * A physics Transform is position + rotation only (no scale). Scale lives
 * separately in the collision shape or rendering data. This keeps the
 * Transform compact (7 scalars: 3 position + 4 quaternion), cache-friendly,
 * and unambiguous — R·P + T with no shear or non-uniform scale to worry
 * about. Composition is associative and fast (quaternion multiply +
 * rotate-and-add).
 *
 * COMPOSITION: C = A ∘ B
 *   C.Position = A.Position + A.Rotation · B.Position
 *   C.Rotation = A.Rotation · B.Rotation
 *
 * INVERSE:
 *   Inv.Position = -R⁻¹ · Position
 *   Inv.Rotation = R⁻¹
 *
 * References:
 * - SE(3) Lie group: rigid-body transforms in 3D
 * - Game engine transform hierarchies (scene graphs)
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#pragma once

#include <TohruPhysics/Vector3.h>
#include <TohruPhysics/Quaternion.h>

typedef struct {
	Vector3  Position;
	Quaternion Rotation;
} Transform;

Transform KaedeTransformIdentity(void);
Transform KaedeTransformMake(const Vector3 *Position, const Quaternion *Rotation);

Vector3 KaedeTransformPoint(const Transform *Tfm, const Vector3 *P);
Vector3 KaedeInverseTransformPoint(const Transform *Tfm, const Vector3 *P);
Vector3 KaedeTransformDirection(const Transform *Tfm, const Vector3 *D);
Vector3 KaedeInverseTransformDirection(const Transform *Tfm, const Vector3 *D);

Transform KaedeTransformCombine(const Transform *A, const Transform *B);
Transform KaedeTransformInverse(const Transform *Tfm);

void KaedeTransformSetPosition(Transform *Tfm, const Vector3 *P);
void KaedeTransformSetRotation(Transform *Tfm, const Quaternion *R);
void KaedeTransformLookAt(Transform *Tfm, const Vector3 *Target, const Vector3 *Up);
