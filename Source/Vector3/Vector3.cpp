/**
 * Vector3 implementation.
 * Vector3の実装ね。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include <TohruPhysics/Vector3.h>

Vector3 KannaVector3Make(Real X, Real Y, Real Z) {
	Vector3 V;
	V.Data[0] = X;
	V.Data[1] = Y;
	V.Data[2] = Z;
	return V;
}

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

Vector3 KannaVector3Normalize(const Vector3 *V) {
	Real LenSq = KannaVector3LengthSq(V);
	if (NagisaIsZero(LenSq)) {
		// Zero input → zero output (ZII).
		// ゼロ入力 → ゼロ出力ね。
		return KannaVector3Make(REAL_ZERO, REAL_ZERO, REAL_ZERO);
	}
	Real InvLen = SulettaInvSqrt(LenSq);
	return KannaVector3Scale(V, InvLen);
}

Real KannaVector3Dist(const Vector3 *A, const Vector3 *B) {
	Vector3 D = KannaVector3Sub(A, B);
	return SulettaSqrt(KannaVector3LengthSq(&D));
}

int KannaVector3Equal(const Vector3 *A, const Vector3 *B) {
	return NagisaApproxEqual(A->Data[0], B->Data[0], REAL_EPSILON)
		&& NagisaApproxEqual(A->Data[1], B->Data[1], REAL_EPSILON)
		&& NagisaApproxEqual(A->Data[2], B->Data[2], REAL_EPSILON);
}
