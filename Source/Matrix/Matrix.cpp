/**
 * Matrix3x3 and Matrix4x4 implementation.
 * 行列の実装ね。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include <TohruPhysics/Matrix.h>

// ---------------------------------------------------------------------------
//  Miorine — Matrix3x3
// ---------------------------------------------------------------------------

Matrix3x3 MiorineMatrix3x3Identity(void) {
	Matrix3x3 M;
	int I;
	for (I = 0; I < 9; I++) M.Data[I] = REAL_ZERO;
	M.Data[0] = 1.0;
	M.Data[4] = 1.0;
	M.Data[8] = 1.0;
	return M;
}

Matrix3x3 MiorineMatrix3x3Make(
	Real M00, Real M01, Real M02,
	Real M10, Real M11, Real M12,
	Real M20, Real M21, Real M22
) {
	Matrix3x3 M;
	M.Data[0] = M00; M.Data[1] = M01; M.Data[2] = M02;
	M.Data[3] = M10; M.Data[4] = M11; M.Data[5] = M12;
	M.Data[6] = M20; M.Data[7] = M21; M.Data[8] = M22;
	return M;
}

Matrix3x3 MiorineMatrix3x3Mul(const Matrix3x3 *A, const Matrix3x3 *B) {
	Matrix3x3 R;
	int I, J, K;
	for (I = 0; I < 3; I++) {
		for (J = 0; J < 3; J++) {
			Real Sum = REAL_ZERO;
			for (K = 0; K < 3; K++) {
				Sum += A->Data[I * 3 + K] * B->Data[K * 3 + J];
			}
			R.Data[I * 3 + J] = Sum;
		}
	}
	return R;
}

Vector3 MiorineMatrix3x3VecMul(const Matrix3x3 *M, const Vector3 *V) {
	Vector3 R;
	R.Data[0] = M->Data[0] * V->Data[0] + M->Data[1] * V->Data[1] + M->Data[2] * V->Data[2];
	R.Data[1] = M->Data[3] * V->Data[0] + M->Data[4] * V->Data[1] + M->Data[5] * V->Data[2];
	R.Data[2] = M->Data[6] * V->Data[0] + M->Data[7] * V->Data[1] + M->Data[8] * V->Data[2];
	return R;
}

Matrix3x3 MiorineMatrix3x3Transpose(const Matrix3x3 *M) {
	Matrix3x3 R;
	R.Data[0] = M->Data[0]; R.Data[1] = M->Data[3]; R.Data[2] = M->Data[6];
	R.Data[3] = M->Data[1]; R.Data[4] = M->Data[4]; R.Data[5] = M->Data[7];
	R.Data[6] = M->Data[2]; R.Data[7] = M->Data[5]; R.Data[8] = M->Data[8];
	return R;
}

Real MiorineMatrix3x3Determinant(const Matrix3x3 *M) {
	Real A = M->Data[0], B = M->Data[1], C = M->Data[2];
	Real D = M->Data[3], E = M->Data[4], F = M->Data[5];
	Real G = M->Data[6], H = M->Data[7], I = M->Data[8];
	return A * (E * I - F * H)
		- B * (D * I - F * G)
		+ C * (D * H - E * G);
}

Matrix3x3 MiorineMatrix3x3Inverse(const Matrix3x3 *M) {
	Real Det = MiorineMatrix3x3Determinant(M);
	if (NagisaIsZero(Det)) {
		// Singular → zero matrix (ZII).
		// 特異行列 → ゼロ行列ね。
		Matrix3x3 Z;
		int I;
		for (I = 0; I < 9; I++) Z.Data[I] = REAL_ZERO;
		return Z;
	}

	Real InvDet = 1.0 / Det;

	Real A = M->Data[0], B = M->Data[1], C = M->Data[2];
	Real D = M->Data[3], E = M->Data[4], F = M->Data[5];
	Real G = M->Data[6], H = M->Data[7], I = M->Data[8];

	Matrix3x3 R;
	R.Data[0] = (E * I - F * H) * InvDet;
	R.Data[1] = (C * H - B * I) * InvDet;
	R.Data[2] = (B * F - C * E) * InvDet;
	R.Data[3] = (F * G - D * I) * InvDet;
	R.Data[4] = (A * I - C * G) * InvDet;
	R.Data[5] = (C * D - A * F) * InvDet;
	R.Data[6] = (D * H - E * G) * InvDet;
	R.Data[7] = (B * G - A * H) * InvDet;
	R.Data[8] = (A * E - B * D) * InvDet;
	return R;
}

// ---------------------------------------------------------------------------
//  Anisphia — Matrix4x4
// ---------------------------------------------------------------------------

Matrix4x4 AnisphiaMatrix4x4Identity(void) {
	Matrix4x4 M;
	int I;
	for (I = 0; I < 16; I++) M.Data[I] = REAL_ZERO;
	M.Data[0] = 1.0;
	M.Data[5] = 1.0;
	M.Data[10] = 1.0;
	M.Data[15] = 1.0;
	return M;
}

Matrix4x4 AnisphiaMatrix4x4Make(
	Real M00, Real M01, Real M02, Real M03,
	Real M10, Real M11, Real M12, Real M13,
	Real M20, Real M21, Real M22, Real M23,
	Real M30, Real M31, Real M32, Real M33
) {
	Matrix4x4 M;
	M.Data[0]  = M00; M.Data[1]  = M01; M.Data[2]  = M02; M.Data[3]  = M03;
	M.Data[4]  = M10; M.Data[5]  = M11; M.Data[6]  = M12; M.Data[7]  = M13;
	M.Data[8]  = M20; M.Data[9]  = M21; M.Data[10] = M22; M.Data[11] = M23;
	M.Data[12] = M30; M.Data[13] = M31; M.Data[14] = M32; M.Data[15] = M33;
	return M;
}

Matrix4x4 AnisphiaMatrix4x4Mul(const Matrix4x4 *A, const Matrix4x4 *B) {
	Matrix4x4 R;
	int I, J, K;
	for (I = 0; I < 4; I++) {
		for (J = 0; J < 4; J++) {
			Real Sum = REAL_ZERO;
			for (K = 0; K < 4; K++) {
				Sum += A->Data[I * 4 + K] * B->Data[K * 4 + J];
			}
			R.Data[I * 4 + J] = Sum;
		}
	}
	return R;
}

Vector3 AnisphiaMatrix4x4VecMul(const Matrix4x4 *M, const Vector3 *V) {
	// Treat V as (x, y, z, 1) — affine transform, then project.
	// Vを(x, y, z, 1)として扱い、アフィン変換してから投影するの。
	Real X = V->Data[0], Y = V->Data[1], Z = V->Data[2];
	Real W = M->Data[3] * X + M->Data[7] * Y + M->Data[11] * Z + M->Data[15];
	if (NagisaIsZero(W)) {
		return KannaVector3Make(REAL_ZERO, REAL_ZERO, REAL_ZERO);
	}
	Real InvW = 1.0 / W;
	Vector3 R;
	R.Data[0] = (M->Data[0] * X + M->Data[4] * Y + M->Data[8]  * Z + M->Data[12]) * InvW;
	R.Data[1] = (M->Data[1] * X + M->Data[5] * Y + M->Data[9]  * Z + M->Data[13]) * InvW;
	R.Data[2] = (M->Data[2] * X + M->Data[6] * Y + M->Data[10] * Z + M->Data[14]) * InvW;
	return R;
}

Real AnisphiaMatrix4x4Determinant(const Matrix4x4 *M) {
	Real A0 = M->Data[0], A1 = M->Data[1], A2 = M->Data[2], A3 = M->Data[3];
	Real B0 = M->Data[4], B1 = M->Data[5], B2 = M->Data[6], B3 = M->Data[7];
	Real C0 = M->Data[8], C1 = M->Data[9], C2 = M->Data[10], C3 = M->Data[11];
	Real D0 = M->Data[12], D1 = M->Data[13], D2 = M->Data[14], D3 = M->Data[15];

	Real Det =
		A0 * (B1 * (C2 * D3 - C3 * D2) - B2 * (C1 * D3 - C3 * D1) + B3 * (C1 * D2 - C2 * D1))
		- A1 * (B0 * (C2 * D3 - C3 * D2) - B2 * (C0 * D3 - C3 * D0) + B3 * (C0 * D2 - C2 * D0))
		+ A2 * (B0 * (C1 * D3 - C3 * D1) - B1 * (C0 * D3 - C3 * D0) + B3 * (C0 * D1 - C1 * D0))
		- A3 * (B0 * (C1 * D2 - C2 * D1) - B1 * (C0 * D2 - C2 * D0) + B2 * (C0 * D1 - C1 * D0));
	return Det;
}

Matrix4x4 AnisphiaMatrix4x4Inverse(const Matrix4x4 *M) {
	Real Det = AnisphiaMatrix4x4Determinant(M);
	if (NagisaIsZero(Det)) {
		Matrix4x4 Z;
		int I;
		for (I = 0; I < 16; I++) Z.Data[I] = REAL_ZERO;
		return Z;
	}

	Real InvDet = 1.0 / Det;

	Real A0 = M->Data[0], A1 = M->Data[1], A2 = M->Data[2], A3 = M->Data[3];
	Real B0 = M->Data[4], B1 = M->Data[5], B2 = M->Data[6], B3 = M->Data[7];
	Real C0 = M->Data[8], C1 = M->Data[9], C2 = M->Data[10], C3 = M->Data[11];
	Real D0 = M->Data[12], D1 = M->Data[13], D2 = M->Data[14], D3 = M->Data[15];

	// Cofactors.
	Real C00 = B1 * (C2 * D3 - C3 * D2) - B2 * (C1 * D3 - C3 * D1) + B3 * (C1 * D2 - C2 * D1);
	Real C01 = B0 * (C2 * D3 - C3 * D2) - B2 * (C0 * D3 - C3 * D0) + B3 * (C0 * D2 - C2 * D0);
	Real C02 = B0 * (C1 * D3 - C3 * D1) - B1 * (C0 * D3 - C3 * D0) + B3 * (C0 * D1 - C1 * D0);
	Real C03 = B0 * (C1 * D2 - C2 * D1) - B1 * (C0 * D2 - C2 * D0) + B2 * (C0 * D1 - C1 * D0);
	Real C10 = A1 * (C2 * D3 - C3 * D2) - A2 * (C1 * D3 - C3 * D1) + A3 * (C1 * D2 - C2 * D1);
	Real C11 = A0 * (C2 * D3 - C3 * D2) - A2 * (C0 * D3 - C3 * D0) + A3 * (C0 * D2 - C2 * D0);
	Real C12 = A0 * (C1 * D3 - C3 * D1) - A1 * (C0 * D3 - C3 * D0) + A3 * (C0 * D1 - C1 * D0);
	Real C13 = A0 * (C1 * D2 - C2 * D1) - A1 * (C0 * D2 - C2 * D0) + A2 * (C0 * D1 - C1 * D0);
	Real C20 = A1 * (B2 * D3 - B3 * D2) - A2 * (B1 * D3 - B3 * D1) + A3 * (B1 * D2 - B2 * D1);
	Real C21 = A0 * (B2 * D3 - B3 * D2) - A2 * (B0 * D3 - B3 * D0) + A3 * (B0 * D2 - B2 * D0);
	Real C22 = A0 * (B1 * D3 - B3 * D1) - A1 * (B0 * D3 - B3 * D0) + A3 * (B0 * D1 - B1 * D0);
	Real C23 = A0 * (B1 * D2 - B2 * D1) - A1 * (B0 * D2 - B2 * D0) + A2 * (B0 * D1 - B1 * D0);
	Real C30 = A1 * (B2 * C3 - B3 * C2) - A2 * (B1 * C3 - B3 * C1) + A3 * (B1 * C2 - B2 * C1);
	Real C31 = A0 * (B2 * C3 - B3 * C2) - A2 * (B0 * C3 - B3 * C0) + A3 * (B0 * C2 - B2 * C0);
	Real C32 = A0 * (B1 * C3 - B3 * C1) - A1 * (B0 * C3 - B3 * C0) + A3 * (B0 * C1 - B1 * C0);
	Real C33 = A0 * (B1 * C2 - B2 * C1) - A1 * (B0 * C2 - B2 * C0) + A2 * (B0 * C1 - B1 * C0);

	// Transposed adjugate with alternating sign.
	Matrix4x4 R;
	R.Data[0]  =  C00 * InvDet;
	R.Data[1]  = -C10 * InvDet;
	R.Data[2]  =  C20 * InvDet;
	R.Data[3]  = -C30 * InvDet;
	R.Data[4]  = -C01 * InvDet;
	R.Data[5]  =  C11 * InvDet;
	R.Data[6]  = -C21 * InvDet;
	R.Data[7]  =  C31 * InvDet;
	R.Data[8]  =  C02 * InvDet;
	R.Data[9]  = -C12 * InvDet;
	R.Data[10] =  C22 * InvDet;
	R.Data[11] = -C32 * InvDet;
	R.Data[12] = -C03 * InvDet;
	R.Data[13] =  C13 * InvDet;
	R.Data[14] = -C23 * InvDet;
	R.Data[15] =  C33 * InvDet;
	return R;
}
