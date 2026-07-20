/**
 * Quaternion implementation.
 * クォータニオンの実装ね。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include <TohruPhysics/Quaternion.h>
#include <string.h>

// ---------------------------------------------------------------------------
//  Identity / constructors
// ---------------------------------------------------------------------------

Quaternion EuphylliaQuaternionIdentity(void) {
	Quaternion Q;
	Q.Data[0] = REAL_ZERO;
	Q.Data[1] = REAL_ZERO;
	Q.Data[2] = REAL_ZERO;
	Q.Data[3] = 1.0;
	return Q;
}

Quaternion EuphylliaQuaternionMake(Real X, Real Y, Real Z, Real W) {
	Quaternion Q;
	Q.Data[0] = X;
	Q.Data[1] = Y;
	Q.Data[2] = Z;
	Q.Data[3] = W;
	return Q;
}

Quaternion EuphylliaQuaternionFromAxisAngle(const Vector3 *Axis, Real AngleRad) {
	Real HalfAng = AngleRad * 0.5;
	Real S = SulettaSin(HalfAng);
	Vector3 NormAxis = KannaVector3Normalize(Axis);
	Quaternion Q;
	Q.Data[0] = NormAxis.Data[0] * S;
	Q.Data[1] = NormAxis.Data[1] * S;
	Q.Data[2] = NormAxis.Data[2] * S;
	Q.Data[3] = SulettaCos(HalfAng);
	return Q;
}

Quaternion EuphylliaQuaternionFromToRotation(const Vector3 *From, const Vector3 *To) {
	Vector3 F = KannaVector3Normalize(From);
	Vector3 T = KannaVector3Normalize(To);
	Real Dot = KannaVector3Dot(&F, &T);
	// If vectors are opposite, rotate 180° around an orthogonal axis.
	if (Dot < -1.0 + REAL_EPSILON) {
		Vector3 Orth = KannaVector3Orthogonal(&F);
		return EuphylliaQuaternionFromAxisAngle(&Orth, REAL_PI);
	}
	// If nearly parallel, identity.
	if (Dot > 1.0 - REAL_EPSILON) {
		return EuphylliaQuaternionIdentity();
	}
	Vector3 Axis = KannaVector3Cross(&F, &T);
	Real W = SulettaSqrt((1.0 + Dot) * 2.0);
	Real Scale = 1.0 / W;
	Quaternion Q;
	Q.Data[0] = Axis.Data[0] * Scale;
	Q.Data[1] = Axis.Data[1] * Scale;
	Q.Data[2] = Axis.Data[2] * Scale;
	Q.Data[3] = W * 0.5;
	return Q;
}

Quaternion EuphylliaQuaternionLookRotation(const Vector3 *Direction, const Vector3 *Up) {
	Vector3 Fwd = KannaVector3Normalize(Direction);
	if (KannaVector3IsZero(&Fwd)) {
		return EuphylliaQuaternionIdentity();
	}
	Vector3 Side = KannaVector3Cross(&Fwd, Up);
	Side = KannaVector3Normalize(&Side);
	Vector3 RealUp = KannaVector3Cross(&Side, &Fwd);

	Matrix3x3 M;
	M.Data[0] = Side.Data[0];
	M.Data[1] = RealUp.Data[0];
	M.Data[2] = -Fwd.Data[0];
	M.Data[3] = Side.Data[1];
	M.Data[4] = RealUp.Data[1];
	M.Data[5] = -Fwd.Data[1];
	M.Data[6] = Side.Data[2];
	M.Data[7] = RealUp.Data[2];
	M.Data[8] = -Fwd.Data[2];

	return EuphylliaMatrix3x3ToQuaternion(&M);
}

// ---------------------------------------------------------------------------
//  Arithmetic
// ---------------------------------------------------------------------------

Real EuphylliaQuaternionDot(const Quaternion *A, const Quaternion *B) {
	return A->Data[0] * B->Data[0]
		+ A->Data[1] * B->Data[1]
		+ A->Data[2] * B->Data[2]
		+ A->Data[3] * B->Data[3];
}

Real EuphylliaQuaternionLengthSq(const Quaternion *Q) {
	return EuphylliaQuaternionDot(Q, Q);
}

Real EuphylliaQuaternionLength(const Quaternion *Q) {
	return SulettaSqrt(EuphylliaQuaternionLengthSq(Q));
}

Quaternion EuphylliaQuaternionNormalize(const Quaternion *Q) {
	Real LenSq = EuphylliaQuaternionLengthSq(Q);
	if (NagisaIsZero(LenSq)) {
		return EuphylliaQuaternionIdentity();
	}
	Real InvLen = SulettaInvSqrt(LenSq);
	Quaternion R;
	R.Data[0] = Q->Data[0] * InvLen;
	R.Data[1] = Q->Data[1] * InvLen;
	R.Data[2] = Q->Data[2] * InvLen;
	R.Data[3] = Q->Data[3] * InvLen;
	return R;
}

Quaternion EuphylliaQuaternionMul(const Quaternion *A, const Quaternion *B) {
	Real Ax = A->Data[0], Ay = A->Data[1], Az = A->Data[2], Aw = A->Data[3];
	Real Bx = B->Data[0], By = B->Data[1], Bz = B->Data[2], Bw = B->Data[3];
	Quaternion R;
	R.Data[0] = Aw * Bx + Ax * Bw + Ay * Bz - Az * By;
	R.Data[1] = Aw * By - Ax * Bz + Ay * Bw + Az * Bx;
	R.Data[2] = Aw * Bz + Ax * By - Ay * Bx + Az * Bw;
	R.Data[3] = Aw * Bw - Ax * Bx - Ay * By - Az * Bz;
	return R;
}

Quaternion EuphylliaQuaternionConjugate(const Quaternion *Q) {
	Quaternion R;
	R.Data[0] = -Q->Data[0];
	R.Data[1] = -Q->Data[1];
	R.Data[2] = -Q->Data[2];
	R.Data[3] =  Q->Data[3];
	return R;
}

Quaternion EuphylliaQuaternionInverse(const Quaternion *Q) {
	Real LenSq = EuphylliaQuaternionLengthSq(Q);
	if (NagisaIsZero(LenSq)) {
		return EuphylliaQuaternionIdentity();
	}
	Real InvLenSq = 1.0 / LenSq;
	Quaternion Conj = EuphylliaQuaternionConjugate(Q);
	Quaternion R;
	R.Data[0] = Conj.Data[0] * InvLenSq;
	R.Data[1] = Conj.Data[1] * InvLenSq;
	R.Data[2] = Conj.Data[2] * InvLenSq;
	R.Data[3] = Conj.Data[3] * InvLenSq;
	return R;
}

// ---------------------------------------------------------------------------
//  Conversion
// ---------------------------------------------------------------------------

Matrix3x3 EuphylliaQuaternionToMatrix3x3(const Quaternion *Q) {
	Real X = Q->Data[0], Y = Q->Data[1], Z = Q->Data[2], W = Q->Data[3];
	Real X2 = X + X, Y2 = Y + Y, Z2 = Z + Z;
	Real XX = X * X2, XY = X * Y2, XZ = X * Z2;
	Real YY = Y * Y2, YZ = Y * Z2, ZZ = Z * Z2;
	Real WX = W * X2, WY = W * Y2, WZ = W * Z2;

	Matrix3x3 M;
	M.Data[0] = 1.0 - (YY + ZZ); M.Data[1] =       XY - WZ;  M.Data[2] =       XZ + WY;
	M.Data[3] =       XY + WZ;  M.Data[4] = 1.0 - (XX + ZZ); M.Data[5] =       YZ - WX;
	M.Data[6] =       XZ - WY;  M.Data[7] =       YZ + WX;  M.Data[8] = 1.0 - (XX + YY);
	return M;
}

Quaternion EuphylliaMatrix3x3ToQuaternion(const Matrix3x3 *M) {
	Real M00 = M->Data[0], M01 = M->Data[1], M02 = M->Data[2];
	Real M10 = M->Data[3], M11 = M->Data[4], M12 = M->Data[5];
	Real M20 = M->Data[6], M21 = M->Data[7], M22 = M->Data[8];

	Real Trace = M00 + M11 + M22;
	Quaternion Q;

	if (Trace > REAL_ZERO) {
		Real S = SulettaSqrt(Trace + 1.0);
		Real T = 0.5 / S;
		Q.Data[3] = 0.5 * S;
		Q.Data[0] = (M21 - M12) * T;
		Q.Data[1] = (M02 - M20) * T;
		Q.Data[2] = (M10 - M01) * T;
	} else if (M00 > M11 && M00 > M22) {
		Real S = SulettaSqrt(1.0 + M00 - M11 - M22);
		Real T = 0.5 / S;
		Q.Data[0] = 0.5 * S;
		Q.Data[1] = (M01 + M10) * T;
		Q.Data[2] = (M02 + M20) * T;
		Q.Data[3] = (M21 - M12) * T;
	} else if (M11 > M22) {
		Real S = SulettaSqrt(1.0 + M11 - M00 - M22);
		Real T = 0.5 / S;
		Q.Data[0] = (M01 + M10) * T;
		Q.Data[1] = 0.5 * S;
		Q.Data[2] = (M12 + M21) * T;
		Q.Data[3] = (M02 - M20) * T;
	} else {
		Real S = SulettaSqrt(1.0 + M22 - M00 - M11);
		Real T = 0.5 / S;
		Q.Data[0] = (M02 + M20) * T;
		Q.Data[1] = (M12 + M21) * T;
		Q.Data[2] = 0.5 * S;
		Q.Data[3] = (M10 - M01) * T;
	}

	return Q;
}

void EuphylliaQuaternionToEulerAngles(const Quaternion *Q, Real *Roll, Real *Pitch, Real *Yaw) {
	Real X = Q->Data[0], Y = Q->Data[1], Z = Q->Data[2], W = Q->Data[3];

	// Roll (x-axis rotation)
	Real SinRCosP = 2.0 * (W * X + Y * Z);
	Real CosRCosP = 1.0 - 2.0 * (X * X + Y * Y);
	*Roll = SulettaAtan2(SinRCosP, CosRCosP);

	// Pitch (y-axis rotation)
	Real SinP = 2.0 * (W * Y - Z * X);
	if (SinP >= 1.0) {
		*Pitch = REAL_PI_HALF;
	} else if (SinP <= -1.0) {
		*Pitch = -REAL_PI_HALF;
	} else {
		*Pitch = SulettaAtan2(SinP, SulettaSqrt(1.0 - SinP * SinP));
	}

	// Yaw (z-axis rotation)
	Real SinYCosP = 2.0 * (W * Z + X * Y);
	Real CosYCosP = 1.0 - 2.0 * (Y * Y + Z * Z);
	*Yaw = SulettaAtan2(SinYCosP, CosYCosP);
}

// ---------------------------------------------------------------------------
//  Interpolation
// ---------------------------------------------------------------------------

Quaternion EuphylliaQuaternionLerp(const Quaternion *A, const Quaternion *B, Real T) {
	Real Ct = YuuClamp01(T);
	Real Dot = EuphylliaQuaternionDot(A, B);
	Quaternion BAdj = *B;
	if (Dot < REAL_ZERO) {
		BAdj.Data[0] = -BAdj.Data[0];
		BAdj.Data[1] = -BAdj.Data[1];
		BAdj.Data[2] = -BAdj.Data[2];
		BAdj.Data[3] = -BAdj.Data[3];
	}
	Quaternion R;
	R.Data[0] = A->Data[0] * (1.0 - Ct) + BAdj.Data[0] * Ct;
	R.Data[1] = A->Data[1] * (1.0 - Ct) + BAdj.Data[1] * Ct;
	R.Data[2] = A->Data[2] * (1.0 - Ct) + BAdj.Data[2] * Ct;
	R.Data[3] = A->Data[3] * (1.0 - Ct) + BAdj.Data[3] * Ct;
	return R;
}

Quaternion EuphylliaQuaternionNLerp(const Quaternion *A, const Quaternion *B, Real T) {
	Quaternion L = EuphylliaQuaternionLerp(A, B, T);
	return EuphylliaQuaternionNormalize(&L);
}

Quaternion EuphylliaQuaternionSlerp(const Quaternion *A, const Quaternion *B, Real T) {
	Real CosOmega = EuphylliaQuaternionDot(A, B);

	Quaternion BAdj = *B;
	if (CosOmega < REAL_ZERO) {
		BAdj.Data[0] = -BAdj.Data[0];
		BAdj.Data[1] = -BAdj.Data[1];
		BAdj.Data[2] = -BAdj.Data[2];
		BAdj.Data[3] = -BAdj.Data[3];
		CosOmega = -CosOmega;
	}

	if (CosOmega > 1.0) CosOmega = 1.0;

	Real K0, K1;
	if (CosOmega < 0.9999) {
		Real Omega = SulettaAcos(CosOmega);
		Real InvSinOmega = 1.0 / SulettaSin(Omega);
		K0 = SulettaSin((1.0 - T) * Omega) * InvSinOmega;
		K1 = SulettaSin(T * Omega) * InvSinOmega;
	} else {
		K0 = 1.0 - T;
		K1 = T;
	}

	Quaternion R;
	R.Data[0] = A->Data[0] * K0 + BAdj.Data[0] * K1;
	R.Data[1] = A->Data[1] * K0 + BAdj.Data[1] * K1;
	R.Data[2] = A->Data[2] * K0 + BAdj.Data[2] * K1;
	R.Data[3] = A->Data[3] * K0 + BAdj.Data[3] * K1;
	return R;
}

// ---------------------------------------------------------------------------
//  Rotation
// ---------------------------------------------------------------------------

Vector3 EuphylliaQuaternionRotateVector(const Quaternion *Q, const Vector3 *V) {
	Vector3 U = KannaVector3Make(Q->Data[0], Q->Data[1], Q->Data[2]);
	Real W = Q->Data[3];

	Vector3 CrossUV = KannaVector3Cross(&U, V);
	Vector3 T = KannaVector3Scale(&CrossUV, 2.0);

	Vector3 WT = KannaVector3Scale(&T, W);
	Vector3 CrossUT = KannaVector3Cross(&U, &T);
	Vector3 R = KannaVector3Add(V, &WT);
	R = KannaVector3Add(&R, &CrossUT);
	return R;
}

// ---------------------------------------------------------------------------
//  Queries
// ---------------------------------------------------------------------------

Real EuphylliaQuaternionAngle(const Quaternion *A, const Quaternion *B) {
	Real Dot = YuuClamp(EuphylliaQuaternionDot(A, B), -1.0, 1.0);
	return 2.0 * SulettaAcos(YuuAbs(Dot));
}

void EuphylliaQuaternionAxis(const Quaternion *Q, Vector3 *Axis, Real *AngleRad) {
	*AngleRad = 2.0 * SulettaAcos(YuuClamp(Q->Data[3], -1.0, 1.0));
	Real SinHalfAng = SulettaSin(*AngleRad * 0.5);
	if (NagisaIsZero(SinHalfAng)) {
		*Axis = KannaVector3Make(0.0, 1.0, 0.0);
		*AngleRad = REAL_ZERO;
		return;
	}
	Real InvSin = 1.0 / SinHalfAng;
	Axis->Data[0] = Q->Data[0] * InvSin;
	Axis->Data[1] = Q->Data[1] * InvSin;
	Axis->Data[2] = Q->Data[2] * InvSin;
}

int EuphylliaQuaternionIsIdentity(const Quaternion *Q) {
	return NagisaIsZero(Q->Data[0])
		&& NagisaIsZero(Q->Data[1])
		&& NagisaIsZero(Q->Data[2])
		&& NagisaApproxEqual(Q->Data[3], 1.0, REAL_EPSILON);
}

int EuphylliaQuaternionEqual(const Quaternion *A, const Quaternion *B) {
	return NagisaApproxEqual(A->Data[0], B->Data[0], REAL_EPSILON)
		&& NagisaApproxEqual(A->Data[1], B->Data[1], REAL_EPSILON)
		&& NagisaApproxEqual(A->Data[2], B->Data[2], REAL_EPSILON)
		&& NagisaApproxEqual(A->Data[3], B->Data[3], REAL_EPSILON);
}
