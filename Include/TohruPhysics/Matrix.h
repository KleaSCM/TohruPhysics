/**
 * Matrix3x3 / Matrix4x4 — row-major matrix types.
 * TohruPhysics用の行列型ね。
 *
 * Row-major layout: Data[Row × Stride + Col].
 * Matrices are stored row-wise in contiguous Real arrays.
 *
 * DESIGN PHILOSOPHY:
 * Both 3×3 and 4×4 are common in physics — 3×3 for inertia tensors
 * and rotation-only transforms, 4×4 for affine transforms (TRS) and
 * projection. We use row-major storage matching C++ 2D array layout
 * (M[Row][Col] = Data[Row*N + Col]).
 *
 * The multiplication convention treats vectors as ROW vectors on the
 * right (v' = v × M). This means translation lives in the LAST ROW
 * (Data[12–14]) rather than the last column.
 *
 * MATRIX3x3 LAYOUT:
 * ┌──────────────────────────────────────────────┐
 * │ Data[0] M00  Data[1] M01  Data[2] M02        │ Row 0
 * │ Data[3] M10  Data[4] M11  Data[5] M12        │ Row 1
 * │ Data[6] M20  Data[7] M21  Data[8] M22        │ Row 2
 * └──────────────────────────────────────────────┘
 *
 * MATRIX4x4 LAYOUT (affine):
 * ┌──────────────────────────────────────────────────────┐
 * │ Data[0] M00  Data[1] M01  Data[2] M02  Data[3]  0   │
 * │ Data[4] M10  Data[5] M11  Data[6] M12  Data[7]  0   │
 * │ Data[8] M20  Data[9] M21  Data[10] M22 Data[11] 0   │
 * │ Data[12] Tx  Data[13] Ty  Data[14] Tz  Data[15] 1   │
 * └──────────────────────────────────────────────────────┘
 *
 * References:
 * - Row-major vs column-major conventions
 * - Affine 4×4 matrices for 3D transforms
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#pragma once

#include <TohruPhysics/Vector3.h>
#include <TohruPhysics/Math.h>

struct Quaternion;

typedef struct {
	Real Data[9];
} Matrix3x3;

typedef struct {
	Real Data[16];
} Matrix4x4;

// ===========================================================================
//  Miorine — Matrix3x3
// ===========================================================================

Matrix3x3 MiorineMatrix3x3Identity(void);
Matrix3x3 MiorineMatrix3x3Zero(void);
Matrix3x3 MiorineMatrix3x3Make(
	Real M00, Real M01, Real M02,
	Real M10, Real M11, Real M12,
	Real M20, Real M21, Real M22
);
Matrix3x3 MiorineMatrix3x3FromQuaternion(const struct Quaternion *Q);

Matrix3x3 MiorineMatrix3x3Add(const Matrix3x3 *A, const Matrix3x3 *B);
Matrix3x3 MiorineMatrix3x3Sub(const Matrix3x3 *A, const Matrix3x3 *B);
Matrix3x3 MiorineMatrix3x3Mul(const Matrix3x3 *A, const Matrix3x3 *B);
Vector3   MiorineMatrix3x3VecMul(const Matrix3x3 *M, const Vector3 *V);
Matrix3x3 MiorineMatrix3x3Transpose(const Matrix3x3 *M);
Real      MiorineMatrix3x3Determinant(const Matrix3x3 *M);
Matrix3x3 MiorineMatrix3x3Adjugate(const Matrix3x3 *M);
Matrix3x3 MiorineMatrix3x3Inverse(const Matrix3x3 *M);
Real      MiorineMatrix3x3Trace(const Matrix3x3 *M);

Matrix3x3 MiorineMatrix3x3RotationX(Real AngleRad);
Matrix3x3 MiorineMatrix3x3RotationY(Real AngleRad);
Matrix3x3 MiorineMatrix3x3RotationZ(Real AngleRad);
Matrix3x3 MiorineMatrix3x3Scale(Real Sx, Real Sy, Real Sz);
Matrix3x3 MiorineMatrix3x3OuterProduct(const Vector3 *A, const Vector3 *B);
Matrix3x3 MiorineMatrix3x3Lerp(const Matrix3x3 *A, const Matrix3x3 *B, Real T);
int       MiorineMatrix3x3Equal(const Matrix3x3 *A, const Matrix3x3 *B);

// ===========================================================================
//  Anisphia — Matrix4x4
// ===========================================================================

Matrix4x4 AnisphiaMatrix4x4Identity(void);
Matrix4x4 AnisphiaMatrix4x4Zero(void);
Matrix4x4 AnisphiaMatrix4x4Make(
	Real M00, Real M01, Real M02, Real M03,
	Real M10, Real M11, Real M12, Real M13,
	Real M20, Real M21, Real M22, Real M23,
	Real M30, Real M31, Real M32, Real M33
);
Matrix4x4 AnisphiaMatrix4x4Translation(const Vector3 *T);
Matrix4x4 AnisphiaMatrix4x4RotationX(Real AngleRad);
Matrix4x4 AnisphiaMatrix4x4RotationY(Real AngleRad);
Matrix4x4 AnisphiaMatrix4x4RotationZ(Real AngleRad);
Matrix4x4 AnisphiaMatrix4x4Scale(Real Sx, Real Sy, Real Sz);
Matrix4x4 AnisphiaMatrix4x4TRS(const Vector3 *T, const struct Quaternion *R, const Vector3 *S);
Matrix4x4 AnisphiaMatrix4x4LookAt(const Vector3 *Eye, const Vector3 *Target, const Vector3 *Up);
Matrix4x4 AnisphiaMatrix4x4Perspective(Real FovRad, Real Aspect, Real Near, Real Far);
Matrix4x4 AnisphiaMatrix4x4Ortho(Real Left, Real Right, Real Bottom, Real Top, Real Near, Real Far);

Matrix4x4 AnisphiaMatrix4x4Add(const Matrix4x4 *A, const Matrix4x4 *B);
Matrix4x4 AnisphiaMatrix4x4Sub(const Matrix4x4 *A, const Matrix4x4 *B);
Matrix4x4 AnisphiaMatrix4x4Mul(const Matrix4x4 *A, const Matrix4x4 *B);
Vector3   AnisphiaMatrix4x4VecMul(const Matrix4x4 *M, const Vector3 *V);
Vector3   AnisphiaMatrix4x4VecMul4(const Matrix4x4 *M, const Vector3 *V, Real W);
Matrix4x4 AnisphiaMatrix4x4Transpose(const Matrix4x4 *M);
Real      AnisphiaMatrix4x4Determinant(const Matrix4x4 *M);
Matrix4x4 AnisphiaMatrix4x4Inverse(const Matrix4x4 *M);
Matrix4x4 AnisphiaMatrix4x4InverseTranspose(const Matrix4x4 *M);

void      AnisphiaMatrix4x4Decompose(const Matrix4x4 *M, Vector3 *T, struct Quaternion *R, Vector3 *S);
Matrix4x4 AnisphiaMatrix4x4Lerp(const Matrix4x4 *A, const Matrix4x4 *B, Real T);
int       AnisphiaMatrix4x4Equal(const Matrix4x4 *A, const Matrix4x4 *B);
