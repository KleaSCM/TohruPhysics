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

Transform KaedeTransformMake(const Vector3 *Position, const Quaternion *Rotation) {
	Transform T;
	T.Position = *Position;
	T.Rotation = *Rotation;
	return T;
}

Vector3 KaedeTransformPoint(const Transform *Tfm, const Vector3 *P) {
	Vector3 Rot = EuphylliaQuaternionRotateVector(&Tfm->Rotation, P);
	return KannaVector3Add(&Rot, &Tfm->Position);
}

Vector3 KaedeInverseTransformPoint(const Transform *Tfm, const Vector3 *P) {
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
	Vector3 RotB = EuphylliaQuaternionRotateVector(&A->Rotation, &B->Position);
	Vector3 Pos = KannaVector3Add(&A->Position, &RotB);
	Quaternion Rot = EuphylliaQuaternionMul(&A->Rotation, &B->Rotation);
	Transform C;
	C.Position = Pos;
	C.Rotation = Rot;
	return C;
}

Transform KaedeTransformInverse(const Transform *Tfm) {
	Quaternion InvRot = EuphylliaQuaternionInverse(&Tfm->Rotation);
	Vector3 NegPos = KannaVector3Scale(&Tfm->Position, -1.0);
	Vector3 Pos = EuphylliaQuaternionRotateVector(&InvRot, &NegPos);
	Transform T;
	T.Position = Pos;
	T.Rotation = InvRot;
	return T;
}

void KaedeTransformSetPosition(Transform *Tfm, const Vector3 *P) {
	Tfm->Position = *P;
}

void KaedeTransformSetRotation(Transform *Tfm, const Quaternion *R) {
	Tfm->Rotation = *R;
}

void KaedeTransformLookAt(Transform *Tfm, const Vector3 *Target, const Vector3 *Up) {
	Vector3 Dir = KannaVector3DirectionFromTo(&Tfm->Position, Target);
	if (KannaVector3IsZero(&Dir)) {
		return;
	}
	Vector3 Fwd = KannaVector3Scale(&Dir, -1.0);
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

	Tfm->Rotation = EuphylliaMatrix3x3ToQuaternion(&M);
}
