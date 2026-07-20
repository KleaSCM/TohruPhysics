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

// 0034
Matrix3x3 MiorineMatrix3x3Identity(void);

Matrix3x3 MiorineMatrix3x3Make(
	Real M00, Real M01, Real M02,
	Real M10, Real M11, Real M12,
	Real M20, Real M21, Real M22
);

// 0035
Matrix3x3 MiorineMatrix3x3Mul(const Matrix3x3 *A, const Matrix3x3 *B);
// 0036
Vector3   MiorineMatrix3x3VecMul(const Matrix3x3 *M, const Vector3 *V);
// 0037
Matrix3x3 MiorineMatrix3x3Transpose(const Matrix3x3 *M);
// 0038
Matrix3x3 MiorineMatrix3x3Inverse(const Matrix3x3 *M);
Real      MiorineMatrix3x3Determinant(const Matrix3x3 *M);

// ===========================================================================
//  Anisphia — Matrix4x4 operations
// ===========================================================================

// 0040
Matrix4x4 AnisphiaMatrix4x4Identity(void);

Matrix4x4 AnisphiaMatrix4x4Make(
	Real M00, Real M01, Real M02, Real M03,
	Real M10, Real M11, Real M12, Real M13,
	Real M20, Real M21, Real M22, Real M23,
	Real M30, Real M31, Real M32, Real M33
);

// 0041
Matrix4x4 AnisphiaMatrix4x4Mul(const Matrix4x4 *A, const Matrix4x4 *B);
// 0042
Vector3   AnisphiaMatrix4x4VecMul(const Matrix4x4 *M, const Vector3 *V);
// 0043
Matrix4x4 AnisphiaMatrix4x4Inverse(const Matrix4x4 *M);
Real      AnisphiaMatrix4x4Determinant(const Matrix4x4 *M);
