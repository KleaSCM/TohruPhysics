/**
 * Quaternion type for TohruPhysics.
 * TohruPhysics用のクォータニオン型ね。
 *
 * Layout: Data[4] = (X, Y, Z, W).
 * レイアウト: Data[4] = (X, Y, Z, W)。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#pragma once

#include <TohruPhysics/Matrix.h>
#include <TohruPhysics/Vector3.h>

// ---------------------------------------------------------------------------
//  0044: Quaternion — contiguous 4-element layout.
//  連続4要素レイアウト。
// ---------------------------------------------------------------------------
typedef struct Quaternion {
	Real Data[4]; // (X, Y, Z, W)
} Quaternion;

// ===========================================================================
//  Euphyllia — Quaternion operations
// ===========================================================================

// Identity / constructors
Quaternion EuphylliaQuaternionIdentity(void);
Quaternion EuphylliaQuaternionMake(Real X, Real Y, Real Z, Real W);
Quaternion EuphylliaQuaternionFromAxisAngle(const Vector3 *Axis, Real AngleRad);
Quaternion EuphylliaQuaternionFromToRotation(const Vector3 *From, const Vector3 *To);
Quaternion EuphylliaQuaternionLookRotation(const Vector3 *Direction, const Vector3 *Up);

// Arithmetic
Real EuphylliaQuaternionDot(const Quaternion *A, const Quaternion *B);
Real EuphylliaQuaternionLengthSq(const Quaternion *Q);
Real EuphylliaQuaternionLength(const Quaternion *Q);
Quaternion EuphylliaQuaternionNormalize(const Quaternion *Q);
Quaternion EuphylliaQuaternionMul(const Quaternion *A, const Quaternion *B);
Quaternion EuphylliaQuaternionConjugate(const Quaternion *Q);
Quaternion EuphylliaQuaternionInverse(const Quaternion *Q);

// Conversion
Matrix3x3 EuphylliaQuaternionToMatrix3x3(const Quaternion *Q);
Quaternion EuphylliaMatrix3x3ToQuaternion(const Matrix3x3 *M);
void      EuphylliaQuaternionToEulerAngles(const Quaternion *Q, Real *Roll, Real *Pitch, Real *Yaw);

// Interpolation
Quaternion EuphylliaQuaternionLerp(const Quaternion *A, const Quaternion *B, Real T);
Quaternion EuphylliaQuaternionNLerp(const Quaternion *A, const Quaternion *B, Real T);
Quaternion EuphylliaQuaternionSlerp(const Quaternion *A, const Quaternion *B, Real T);

// Rotation
Vector3    EuphylliaQuaternionRotateVector(const Quaternion *Q, const Vector3 *V);

// Queries
Real EuphylliaQuaternionAngle(const Quaternion *A, const Quaternion *B);
void EuphylliaQuaternionAxis(const Quaternion *Q, Vector3 *Axis, Real *AngleRad);
int  EuphylliaQuaternionIsIdentity(const Quaternion *Q);
int  EuphylliaQuaternionEqual(const Quaternion *A, const Quaternion *B);
