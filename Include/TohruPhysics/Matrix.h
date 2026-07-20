/**
 * Matrix3x3 and Matrix4x4 types for TohruPhysics.
 * TohruPhysics用の行列型ね。
 *
 * Row-major layout: Data[Row*Stride + Col].
 * 行優先レイアウト: Data[行*幅 + 列]。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#pragma once

#include <TohruPhysics/Vector3.h>
#include <TohruPhysics/Math.h>

// Forward declaration — Quaternion.h includes Matrix.h (circular).
// 前方宣言 — Quaternion.hがMatrix.hをインクルードするの（循環）。
struct Quaternion;

// ---------------------------------------------------------------------------
//  0033: Matrix3x3 — row-major, 9 elements.
//  行優先、9要素。
// ---------------------------------------------------------------------------
typedef struct {
	Real Data[9];
} Matrix3x3;

// ---------------------------------------------------------------------------
//  0039: Matrix4x4 — row-major, 16 elements.
//  行優先、16要素。
// ---------------------------------------------------------------------------
typedef struct {
	Real Data[16];
} Matrix4x4;

// ===========================================================================
//  Miorine — Matrix3x3 operations
// ===========================================================================

// Identity / constructors
Matrix3x3 MiorineMatrix3x3Identity(void);

Matrix3x3 MiorineMatrix3x3Make(
	Real M00, Real M01, Real M02,
	Real M10, Real M11, Real M12,
	Real M20, Real M21, Real M22
);

Matrix3x3 MiorineMatrix3x3Zero(void);
Matrix3x3 MiorineMatrix3x3FromQuaternion(const Quaternion *Q);

// Arithmetic
Matrix3x3 MiorineMatrix3x3Add(const Matrix3x3 *A, const Matrix3x3 *B);
Matrix3x3 MiorineMatrix3x3Sub(const Matrix3x3 *A, const Matrix3x3 *B);
Matrix3x3 MiorineMatrix3x3Mul(const Matrix3x3 *A, const Matrix3x3 *B);
Vector3   MiorineMatrix3x3VecMul(const Matrix3x3 *M, const Vector3 *V);

// Transforms
Matrix3x3 MiorineMatrix3x3Transpose(const Matrix3x3 *M);
Real      MiorineMatrix3x3Determinant(const Matrix3x3 *M);
Matrix3x3 MiorineMatrix3x3Adjugate(const Matrix3x3 *M);
Matrix3x3 MiorineMatrix3x3Inverse(const Matrix3x3 *M);
Real      MiorineMatrix3x3Trace(const Matrix3x3 *M);

// Construction from transforms
Matrix3x3 MiorineMatrix3x3RotationX(Real AngleRad);
Matrix3x3 MiorineMatrix3x3RotationY(Real AngleRad);
Matrix3x3 MiorineMatrix3x3RotationZ(Real AngleRad);
Matrix3x3 MiorineMatrix3x3Scale(Real Sx, Real Sy, Real Sz);

// Other
Matrix3x3 MiorineMatrix3x3OuterProduct(const Vector3 *A, const Vector3 *B);
Matrix3x3 MiorineMatrix3x3Lerp(const Matrix3x3 *A, const Matrix3x3 *B, Real T);
int       MiorineMatrix3x3Equal(const Matrix3x3 *A, const Matrix3x3 *B);

// ===========================================================================
//  Anisphia — Matrix4x4 operations
// ===========================================================================

// Identity / constructors
Matrix4x4 AnisphiaMatrix4x4Identity(void);

Matrix4x4 AnisphiaMatrix4x4Make(
	Real M00, Real M01, Real M02, Real M03,
	Real M10, Real M11, Real M12, Real M13,
	Real M20, Real M21, Real M22, Real M23,
	Real M30, Real M31, Real M32, Real M33
);

Matrix4x4 AnisphiaMatrix4x4Zero(void);
Matrix4x4 AnisphiaMatrix4x4Translation(const Vector3 *T);
Matrix4x4 AnisphiaMatrix4x4RotationX(Real AngleRad);
Matrix4x4 AnisphiaMatrix4x4RotationY(Real AngleRad);
Matrix4x4 AnisphiaMatrix4x4RotationZ(Real AngleRad);
Matrix4x4 AnisphiaMatrix4x4Scale(Real Sx, Real Sy, Real Sz);
Matrix4x4 AnisphiaMatrix4x4TRS(const Vector3 *T, const Quaternion *R, const Vector3 *S);

// Camera / projection
Matrix4x4 AnisphiaMatrix4x4LookAt(const Vector3 *Eye, const Vector3 *Target, const Vector3 *Up);
Matrix4x4 AnisphiaMatrix4x4Perspective(Real FovRad, Real Aspect, Real Near, Real Far);
Matrix4x4 AnisphiaMatrix4x4Ortho(Real Left, Real Right, Real Bottom, Real Top, Real Near, Real Far);

// Arithmetic
Matrix4x4 AnisphiaMatrix4x4Add(const Matrix4x4 *A, const Matrix4x4 *B);
Matrix4x4 AnisphiaMatrix4x4Sub(const Matrix4x4 *A, const Matrix4x4 *B);
Matrix4x4 AnisphiaMatrix4x4Mul(const Matrix4x4 *A, const Matrix4x4 *B);
Vector3   AnisphiaMatrix4x4VecMul(const Matrix4x4 *M, const Vector3 *V);
Vector3   AnisphiaMatrix4x4VecMul4(const Matrix4x4 *M, const Vector3 *V, Real W);

// Transforms
Matrix4x4 AnisphiaMatrix4x4Transpose(const Matrix4x4 *M);
Real      AnisphiaMatrix4x4Determinant(const Matrix4x4 *M);
Matrix4x4 AnisphiaMatrix4x4Inverse(const Matrix4x4 *M);
Matrix4x4 AnisphiaMatrix4x4InverseTranspose(const Matrix4x4 *M);

// Extraction (assumes TRS composition)
void      AnisphiaMatrix4x4Decompose(const Matrix4x4 *M, Vector3 *T, Quaternion *R, Vector3 *S);

// Utility
Matrix4x4 AnisphiaMatrix4x4Lerp(const Matrix4x4 *A, const Matrix4x4 *B, Real T);
int       AnisphiaMatrix4x4Equal(const Matrix4x4 *A, const Matrix4x4 *B);
