/**
 * Quaternion — orientation in 3D space.
 * TohruPhysics用の3D回転クォータニオン型ね。
 *
 * Unit quaternion (X, Y, Z, W) where W = cos(θ/2) and (X,Y,Z) = sin(θ/2)·axis.
 * Used for rotation without gimbal lock, smooth interpolation (Slerp), and
 * efficient composition.
 *
 * DESIGN PHILOSOPHY:
 * Quaternions are the standard for 3D rotation in physics because they are
 * compact (4 scalars), have no singularities (unlike Euler angles), and
 * compose via simple multiplication. Normalised to unit length — operations
 * that produce non-unit results (Lerp, Mul) require explicit normalisation.
 *
 * DATA LAYOUT:
 * ┌──────────┬──────────┬──────────┬──────────┐
 * │ Data[0]  │ Data[1]  │ Data[2]  │ Data[3]  │
 * │    X     │    Y     │    Z     │    W     │
 * └──────────┴──────────┴──────────┴──────────┘
 *
 * References:
 * - Hamilton convention (ijk = -1)
 * - Shoemake, "Animating Rotation with Quaternion Curves" (SIGGRAPH 85)
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#pragma once

#include <TohruPhysics/Matrix.h>
#include <TohruPhysics/Vector3.h>

typedef struct Quaternion {
	Real Data[4];
} Quaternion;

Quaternion EuphylliaQuaternionIdentity(void);
Quaternion EuphylliaQuaternionMake(Real X, Real Y, Real Z, Real W);
Quaternion EuphylliaQuaternionFromAxisAngle(const Vector3 *Axis, Real AngleRad);
Quaternion EuphylliaQuaternionFromToRotation(const Vector3 *From, const Vector3 *To);
Quaternion EuphylliaQuaternionLookRotation(const Vector3 *Direction, const Vector3 *Up);

Real       EuphylliaQuaternionDot(const Quaternion *A, const Quaternion *B);
Real       EuphylliaQuaternionLengthSq(const Quaternion *Q);
Real       EuphylliaQuaternionLength(const Quaternion *Q);
Quaternion EuphylliaQuaternionNormalize(const Quaternion *Q);
Quaternion EuphylliaQuaternionMul(const Quaternion *A, const Quaternion *B);
Quaternion EuphylliaQuaternionConjugate(const Quaternion *Q);
Quaternion EuphylliaQuaternionInverse(const Quaternion *Q);

Matrix3x3 EuphylliaQuaternionToMatrix3x3(const Quaternion *Q);
Quaternion EuphylliaMatrix3x3ToQuaternion(const Matrix3x3 *M);
void      EuphylliaQuaternionToEulerAngles(const Quaternion *Q, Real *Roll, Real *Pitch, Real *Yaw);

Quaternion EuphylliaQuaternionLerp(const Quaternion *A, const Quaternion *B, Real T);
Quaternion EuphylliaQuaternionNLerp(const Quaternion *A, const Quaternion *B, Real T);
Quaternion EuphylliaQuaternionSlerp(const Quaternion *A, const Quaternion *B, Real T);

Vector3    EuphylliaQuaternionRotateVector(const Quaternion *Q, const Vector3 *V);

Real       EuphylliaQuaternionAngle(const Quaternion *A, const Quaternion *B);
void       EuphylliaQuaternionAxis(const Quaternion *Q, Vector3 *Axis, Real *AngleRad);
int        EuphylliaQuaternionIsIdentity(const Quaternion *Q);
int        EuphylliaQuaternionEqual(const Quaternion *A, const Quaternion *B);
