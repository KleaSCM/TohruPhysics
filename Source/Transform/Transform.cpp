/**
 * Transform implementation.
 * トランスフォームの実装ね。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include <TohruPhysics/Transform.h>

Transform KaedeTransformIdentity(void) {
	Transform T;
	T.Position = KannaVector3Zero();
	T.Rotation = EuphylliaQuaternionIdentity();
	return T;
}

Vector3 KaedeTransformPoint(const Transform *Tfm, const Vector3 *P) {
	// P' = R·P + T
	Vector3 Rot = EuphylliaQuaternionRotateVector(&Tfm->Rotation, P);
	return KannaVector3Add(&Rot, &Tfm->Position);
}

Vector3 KaedeInverseTransformPoint(const Transform *Tfm, const Vector3 *P) {
	// P = R⁻¹·(P' − T)
	Vector3 Delta = KannaVector3Sub(P, &Tfm->Position);
	Quaternion Conj = EuphylliaQuaternionConjugate(&Tfm->Rotation);
	return EuphylliaQuaternionRotateVector(&Conj, &Delta);
}

Vector3 KaedeTransformDirection(const Transform *Tfm, const Vector3 *D) {
	return EuphylliaQuaternionRotateVector(&Tfm->Rotation, D);
}

Vector3 KaedeInverseTransformDirection(const Transform *Tfm, const Vector3 *D) {
	Quaternion Conj = EuphylliaQuaternionConjugate(&Tfm->Rotation);
	return EuphylliaQuaternionRotateVector(&Conj, D);
}

Transform KaedeTransformCombine(const Transform *A, const Transform *B) {
	// C.P = A.P + A.R · B.P
	Vector3 RotB = EuphylliaQuaternionRotateVector(&A->Rotation, &B->Position);
	Vector3 Pos = KannaVector3Add(&A->Position, &RotB);

	// C.R = A.R · B.R
	Quaternion Rot = EuphylliaQuaternionMul(&A->Rotation, &B->Rotation);

	Transform C;
	C.Position = Pos;
	C.Rotation = Rot;
	return C;
}
