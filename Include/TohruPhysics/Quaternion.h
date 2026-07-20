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

// ---------------------------------------------------------------------------
//  0044: Quaternion — contiguous 4-element layout.
//  連続4要素レイアウト。
// ---------------------------------------------------------------------------
typedef struct {
	Real Data[4]; // (X, Y, Z, W)
} Quaternion;

// ===========================================================================
//  Euphyllia — Quaternion operations
// ===========================================================================

// 0045
Quaternion EuphylliaQuaternionIdentity(void);
Quaternion EuphylliaQuaternionMake(Real X, Real Y, Real Z, Real W);

// 0052
Real EuphylliaQuaternionDot(const Quaternion *A, const Quaternion *B);
Real EuphylliaQuaternionLengthSq(const Quaternion *Q);

// 0046
Quaternion EuphylliaQuaternionNormalize(const Quaternion *Q);

// 0047
Quaternion EuphylliaQuaternionMul(const Quaternion *A, const Quaternion *B);

// 0048
Matrix3x3 EuphylliaQuaternionToMatrix3x3(const Quaternion *Q);

// 0049
Quaternion EuphylliaMatrix3x3ToQuaternion(const Matrix3x3 *M);

// 0050
Quaternion EuphylliaQuaternionSlerp(const Quaternion *A, const Quaternion *B, Real T);

// 0051
Quaternion EuphylliaQuaternionConjugate(const Quaternion *Q);
