/**
 * Vector3 implementation.
 * Vector3の実装ね。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include <TohruPhysics/Vector3.h>

// ---------------------------------------------------------------------------
//  Constructors
// ---------------------------------------------------------------------------

Vector3 KannaVector3Make(Real X, Real Y, Real Z) {
	Vector3 V;
	V.Data[0] = X;
	V.Data[1] = Y;
	V.Data[2] = Z;
	return V;
}

Vector3 KannaVector3Zero(void) {
	Vector3 V;
	V.Data[0] = REAL_ZERO;
	V.Data[1] = REAL_ZERO;
	V.Data[2] = REAL_ZERO;
	return V;
}

Vector3 KannaVector3One(void) {
	Vector3 V;
	V.Data[0] = 1.0;
	V.Data[1] = 1.0;
	V.Data[2] = 1.0;
	return V;
}

Vector3 KannaVector3UnitX(void) {
	Vector3 V;
	V.Data[0] = 1.0;
	V.Data[1] = REAL_ZERO;
	V.Data[2] = REAL_ZERO;
	return V;
}

Vector3 KannaVector3UnitY(void) {
	Vector3 V;
	V.Data[0] = REAL_ZERO;
	V.Data[1] = 1.0;
	V.Data[2] = REAL_ZERO;
	return V;
}

Vector3 KannaVector3UnitZ(void) {
	Vector3 V;
	V.Data[0] = REAL_ZERO;
	V.Data[1] = REAL_ZERO;
	V.Data[2] = 1.0;
	return V;
}

// ---------------------------------------------------------------------------
//  Core arithmetic
// ---------------------------------------------------------------------------

Vector3 KannaVector3Add(const Vector3 *A, const Vector3 *B) {
	Vector3 R;
	R.Data[0] = A->Data[0] + B->Data[0];
	R.Data[1] = A->Data[1] + B->Data[1];
	R.Data[2] = A->Data[2] + B->Data[2];
	return R;
}

Vector3 KannaVector3Sub(const Vector3 *A, const Vector3 *B) {
	Vector3 R;
	R.Data[0] = A->Data[0] - B->Data[0];
	R.Data[1] = A->Data[1] - B->Data[1];
	R.Data[2] = A->Data[2] - B->Data[2];
	return R;
}

Vector3 KannaVector3Scale(const Vector3 *V, Real S) {
	Vector3 R;
	R.Data[0] = V->Data[0] * S;
	R.Data[1] = V->Data[1] * S;
	R.Data[2] = V->Data[2] * S;
	return R;
}

Real KannaVector3Dot(const Vector3 *A, const Vector3 *B) {
	return A->Data[0] * B->Data[0]
		+ A->Data[1] * B->Data[1]
		+ A->Data[2] * B->Data[2];
}

Vector3 KannaVector3Cross(const Vector3 *A, const Vector3 *B) {
	Vector3 R;
	R.Data[0] = A->Data[1] * B->Data[2] - A->Data[2] * B->Data[1];
	R.Data[1] = A->Data[2] * B->Data[0] - A->Data[0] * B->Data[2];
	R.Data[2] = A->Data[0] * B->Data[1] - A->Data[1] * B->Data[0];
	return R;
}

Real KannaVector3LengthSq(const Vector3 *V) {
	return KannaVector3Dot(V, V);
}

Real KannaVector3Length(const Vector3 *V) {
	return SulettaSqrt(KannaVector3LengthSq(V));
}

Vector3 KannaVector3Normalize(const Vector3 *V) {
	Real LenSq = KannaVector3LengthSq(V);
	if (NagisaIsZero(LenSq)) {
		return KannaVector3Make(REAL_ZERO, REAL_ZERO, REAL_ZERO);
	}
	Real InvLen = SulettaInvSqrt(LenSq);
	return KannaVector3Scale(V, InvLen);
}

Real KannaVector3Distance(const Vector3 *A, const Vector3 *B) {
	Vector3 D = KannaVector3Sub(A, B);
	return SulettaSqrt(KannaVector3LengthSq(&D));
}

Real KannaVector3DistanceSq(const Vector3 *A, const Vector3 *B) {
	Vector3 D = KannaVector3Sub(A, B);
	return KannaVector3LengthSq(&D);
}

int KannaVector3Equal(const Vector3 *A, const Vector3 *B) {
	return NagisaApproxEqual(A->Data[0], B->Data[0], REAL_EPSILON)
		&& NagisaApproxEqual(A->Data[1], B->Data[1], REAL_EPSILON)
		&& NagisaApproxEqual(A->Data[2], B->Data[2], REAL_EPSILON);
}

// ---------------------------------------------------------------------------
//  Component-wise operations
// ---------------------------------------------------------------------------

Vector3 KannaVector3Min(const Vector3 *A, const Vector3 *B) {
	Vector3 R;
	R.Data[0] = A->Data[0] < B->Data[0] ? A->Data[0] : B->Data[0];
	R.Data[1] = A->Data[1] < B->Data[1] ? A->Data[1] : B->Data[1];
	R.Data[2] = A->Data[2] < B->Data[2] ? A->Data[2] : B->Data[2];
	return R;
}

Vector3 KannaVector3Max(const Vector3 *A, const Vector3 *B) {
	Vector3 R;
	R.Data[0] = A->Data[0] > B->Data[0] ? A->Data[0] : B->Data[0];
	R.Data[1] = A->Data[1] > B->Data[1] ? A->Data[1] : B->Data[1];
	R.Data[2] = A->Data[2] > B->Data[2] ? A->Data[2] : B->Data[2];
	return R;
}

Vector3 KannaVector3Abs(const Vector3 *V) {
	Vector3 R;
	R.Data[0] = YuuAbs(V->Data[0]);
	R.Data[1] = YuuAbs(V->Data[1]);
	R.Data[2] = YuuAbs(V->Data[2]);
	return R;
}

Vector3 KannaVector3Negate(const Vector3 *V) {
	return KannaVector3Scale(V, -1.0);
}

Vector3 KannaVector3PerComponentMul(const Vector3 *A, const Vector3 *B) {
	Vector3 R;
	R.Data[0] = A->Data[0] * B->Data[0];
	R.Data[1] = A->Data[1] * B->Data[1];
	R.Data[2] = A->Data[2] * B->Data[2];
	return R;
}

// ---------------------------------------------------------------------------
//  Interpolation and blending
// ---------------------------------------------------------------------------

Vector3 KannaVector3Lerp(const Vector3 *A, const Vector3 *B, Real T) {
	Real Ct = YuuClamp01(T);
	Vector3 R;
	R.Data[0] = A->Data[0] + (B->Data[0] - A->Data[0]) * Ct;
	R.Data[1] = A->Data[1] + (B->Data[1] - A->Data[1]) * Ct;
	R.Data[2] = A->Data[2] + (B->Data[2] - A->Data[2]) * Ct;
	return R;
}

Vector3 KannaVector3Slerp(const Vector3 *A, const Vector3 *B, Real T) {
	Real Dot = KannaVector3Dot(A, B);
	Real ClampedDot = YuuClamp(Dot, -1.0, 1.0);
	Real Theta = SulettaAcos(ClampedDot);
	if (NagisaIsZero(Theta)) {
		return KannaVector3Lerp(A, B, T);
	}
	Real SinTheta = SulettaSin(Theta);
	Real ScaleA = SulettaSin((1.0 - T) * Theta) / SinTheta;
	Real ScaleB = SulettaSin(T * Theta) / SinTheta;
	Vector3 R;
	R.Data[0] = A->Data[0] * ScaleA + B->Data[0] * ScaleB;
	R.Data[1] = A->Data[1] * ScaleA + B->Data[1] * ScaleB;
	R.Data[2] = A->Data[2] * ScaleA + B->Data[2] * ScaleB;
	return R;
}

// ---------------------------------------------------------------------------
//  Reflection and projection
// ---------------------------------------------------------------------------

Vector3 KannaVector3Reflect(const Vector3 *V, const Vector3 *Normal) {
	Real D = KannaVector3Dot(V, Normal);
	Vector3 R;
	R.Data[0] = V->Data[0] - 2.0 * D * Normal->Data[0];
	R.Data[1] = V->Data[1] - 2.0 * D * Normal->Data[1];
	R.Data[2] = V->Data[2] - 2.0 * D * Normal->Data[2];
	return R;
}

Vector3 KannaVector3Project(const Vector3 *V, const Vector3 *Onto) {
	Real LenSq = KannaVector3LengthSq(Onto);
	if (NagisaIsZero(LenSq)) {
		return KannaVector3Zero();
	}
	Real Scale = KannaVector3Dot(V, Onto) / LenSq;
	return KannaVector3Scale(Onto, Scale);
}

Vector3 KannaVector3Reject(const Vector3 *V, const Vector3 *Onto) {
	Vector3 Proj = KannaVector3Project(V, Onto);
	return KannaVector3Sub(V, &Proj);
}

Vector3 KannaVector3ProjectOnPlane(const Vector3 *V, const Vector3 *PlaneNormal) {
	return KannaVector3Reject(V, PlaneNormal);
}

// ---------------------------------------------------------------------------
//  Geometric queries
// ---------------------------------------------------------------------------

Real KannaVector3Angle(const Vector3 *A, const Vector3 *B) {
	Real Dot = KannaVector3Dot(A, B);
	Real LenA = KannaVector3Length(A);
	Real LenB = KannaVector3Length(B);
	if (NagisaIsZero(LenA) || NagisaIsZero(LenB)) {
		return REAL_ZERO;
	}
	Real CosAngle = YuuClamp(Dot / (LenA * LenB), -1.0, 1.0);
	return SulettaAcos(CosAngle);
}

int KannaVector3IsUnit(const Vector3 *V, Real Eps) {
	Real LenSq = KannaVector3LengthSq(V);
	return NagisaApproxEqual(LenSq, 1.0, Eps);
}

int KannaVector3IsZero(const Vector3 *V) {
	return NagisaIsZero(V->Data[0])
		&& NagisaIsZero(V->Data[1])
		&& NagisaIsZero(V->Data[2]);
}

Vector3 KannaVector3Midpoint(const Vector3 *A, const Vector3 *B) {
	Vector3 R;
	R.Data[0] = (A->Data[0] + B->Data[0]) * 0.5;
	R.Data[1] = (A->Data[1] + B->Data[1]) * 0.5;
	R.Data[2] = (A->Data[2] + B->Data[2]) * 0.5;
	return R;
}

Vector3 KannaVector3DirectionFromTo(const Vector3 *From, const Vector3 *To) {
	Vector3 D = KannaVector3Sub(To, From);
	return KannaVector3Normalize(&D);
}

// ---------------------------------------------------------------------------
//  Clamping and limiting
// ---------------------------------------------------------------------------

Vector3 KannaVector3ClampLength(const Vector3 *V, Real MaxLen) {
	Real LenSq = KannaVector3LengthSq(V);
	if (NagisaIsZero(LenSq)) {
		return KannaVector3Zero();
	}
	Real Len = SulettaSqrt(LenSq);
	if (Len <= MaxLen) {
		return *V;
	}
	Real Scale = MaxLen / Len;
	return KannaVector3Scale(V, Scale);
}

Vector3 KannaVector3Clamp(const Vector3 *V, const Vector3 *Lo, const Vector3 *Hi) {
	Vector3 R;
	R.Data[0] = YuuClamp(V->Data[0], Lo->Data[0], Hi->Data[0]);
	R.Data[1] = YuuClamp(V->Data[1], Lo->Data[1], Hi->Data[1]);
	R.Data[2] = YuuClamp(V->Data[2], Lo->Data[2], Hi->Data[2]);
	return R;
}

// ---------------------------------------------------------------------------
//  Orthogonal basis helper
// ---------------------------------------------------------------------------

Vector3 KannaVector3Orthogonal(const Vector3 *V) {
	// Find a vector orthogonal to V by crossing with the smallest component axis.
	// 最小成分軸とのクロス積で直交ベクトルを求めるの。
	Real AbsX = YuuAbs(V->Data[0]);
	Real AbsY = YuuAbs(V->Data[1]);
	Real AbsZ = YuuAbs(V->Data[2]);

	Vector3 Ref;
	if (AbsX <= AbsY && AbsX <= AbsZ) {
		Ref = KannaVector3Make(1.0, 0.0, 0.0);
	} else if (AbsY <= AbsZ) {
		Ref = KannaVector3Make(0.0, 1.0, 0.0);
	} else {
		Ref = KannaVector3Make(0.0, 0.0, 1.0);
	}

	Vector3 Orth = KannaVector3Cross(V, &Ref);
	Real LenSq = KannaVector3LengthSq(&Orth);
	if (NagisaIsZero(LenSq)) {
		return KannaVector3Zero();
	}
	return KannaVector3Normalize(&Orth);
}
