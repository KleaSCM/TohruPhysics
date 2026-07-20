/**
 * Geometric primitive implementations.
 * 基本幾何形状の実装ね。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include <TohruPhysics/Geometry.h>
#include <TohruPhysics/Math.h>
#include <TohruPhysics/Quaternion.h>

// ===========================================================================
//  AABB
// ===========================================================================

AABB SabinaAABBMake(const Vector3 *Min, const Vector3 *Max) {
	AABB Box;
	Box.Min = *Min;
	Box.Max = *Max;
	return Box;
}

AABB SabinaAABBMakeCenterExtents(const Vector3 *Center, const Vector3 *HalfExtents) {
	AABB Box;
	Box.Min.Data[0] = Center->Data[0] - HalfExtents->Data[0];
	Box.Min.Data[1] = Center->Data[1] - HalfExtents->Data[1];
	Box.Min.Data[2] = Center->Data[2] - HalfExtents->Data[2];
	Box.Max.Data[0] = Center->Data[0] + HalfExtents->Data[0];
	Box.Max.Data[1] = Center->Data[1] + HalfExtents->Data[1];
	Box.Max.Data[2] = Center->Data[2] + HalfExtents->Data[2];
	return Box;
}

int SabinaAABBContains(const AABB *Box, const Vector3 *Point) {
	for (int I = 0; I < 3; I++) {
		if (Point->Data[I] < Box->Min.Data[I]) return 0;
		if (Point->Data[I] > Box->Max.Data[I]) return 0;
	}
	return 1;
}

int SabinaAABBOverlaps(const AABB *A, const AABB *B) {
	for (int I = 0; I < 3; I++) {
		if (A->Max.Data[I] < B->Min.Data[I]) return 0;
		if (A->Min.Data[I] > B->Max.Data[I]) return 0;
	}
	return 1;
}

// ===========================================================================
//  Sphere
// ===========================================================================

Sphere SabinaSphereMake(const Vector3 *Center, Real Radius) {
	Sphere S;
	S.Center = *Center;
	S.Radius = Radius;
	return S;
}

int SabinaSphereContains(const Sphere *S, const Vector3 *Point) {
	Vector3 D = KannaVector3Sub(Point, &S->Center);
	Real DistSq = KannaVector3Dot(&D, &D);
	return DistSq <= S->Radius * S->Radius;
}

int SabinaSphereOverlaps(const Sphere *A, const Sphere *B) {
	Vector3 D = KannaVector3Sub(&A->Center, &B->Center);
	Real DistSq = KannaVector3Dot(&D, &D);
	Real RSum = A->Radius + B->Radius;
	return DistSq <= RSum * RSum;
}

// ===========================================================================
//  OBB
// ===========================================================================

OBB SabinaOBBMake(const Vector3 *Center, const Vector3 *HalfExtents, const Quaternion *Rotation) {
	OBB Box;
	Box.Center = *Center;
	Box.HalfExtents = *HalfExtents;
	Box.Rotation = *Rotation;
	return Box;
}

int SabinaOBBContains(const OBB *Box, const Vector3 *Point) {
	// Transform point to OBB local space.
	// 点をOBBのローカル空間に変換するの。
	Vector3 Local = KannaVector3Sub(Point, &Box->Center);
	Quaternion Conj = EuphylliaQuaternionConjugate(&Box->Rotation);
	Local = EuphylliaQuaternionRotateVector(&Conj, &Local);

	for (int I = 0; I < 3; I++) {
		if (Local.Data[I] < -Box->HalfExtents.Data[I]) return 0;
		if (Local.Data[I] >  Box->HalfExtents.Data[I]) return 0;
	}
	return 1;
}

// ===========================================================================
//  Capsule
// ===========================================================================

Capsule SabinaCapsuleMake(const Vector3 *Start, const Vector3 *End, Real Radius) {
	Capsule C;
	C.Start = *Start;
	C.End   = *End;
	C.Radius = Radius;
	return C;
}

Vector3 SabinaCapsuleClosestPoint(const Capsule *C, const Vector3 *P) {
	// Project P onto segment [Start, End], clamp, offset toward P by radius.
	// Pを線分[Start, End]に投影し、クランプ、P方向に半径分オフセット。
	Vector3 Axis = KannaVector3Sub(&C->End, &C->Start);
	Vector3 PToS = KannaVector3Sub(P, &C->Start);
	Real LenSq = KannaVector3Dot(&Axis, &Axis);

	Real T;
	if (NagisaIsZero(LenSq)) {
		T = 0.0;
	} else {
		T = KannaVector3Dot(&PToS, &Axis) / LenSq;
		T = YuuClamp01(T);
	}

	Vector3 Closest = KannaVector3Scale(&Axis, T);
	Closest = KannaVector3Add(&C->Start, &Closest);

	// Push toward P by radius.
	Vector3 ToP = KannaVector3Sub(P, &Closest);
	Real DistSq = KannaVector3LengthSq(&ToP);
	if (DistSq > C->Radius * C->Radius && !NagisaIsZero(DistSq)) {
		Real Dist = SulettaSqrt(DistSq);
		ToP = KannaVector3Scale(&ToP, C->Radius / Dist);
		Closest = KannaVector3Add(&Closest, &ToP);
	}

	return Closest;
}

// ===========================================================================
//  Plane
// ===========================================================================

Plane SabinaPlaneMake(const Vector3 *Normal, Real Distance) {
	Plane P;
	P.Normal   = *Normal;
	P.Distance = Distance;
	return P;
}

Plane SabinaPlaneMakeFromPoints(const Vector3 *A, const Vector3 *B, const Vector3 *C) {
	Vector3 AB = KannaVector3Sub(B, A);
	Vector3 AC = KannaVector3Sub(C, A);
	Vector3 Nml = KannaVector3Cross(&AB, &AC);
	Nml = KannaVector3Normalize(&Nml);

	Plane P;
	P.Normal   = Nml;
	P.Distance = KannaVector3Dot(&Nml, A);
	return P;
}

Real SabinaPlaneSignedDistance(const Plane *P, const Vector3 *Point) {
	return KannaVector3Dot(&P->Normal, Point) - P->Distance;
}

// ===========================================================================
//  Ray
// ===========================================================================

Ray SabinaRayMake(const Vector3 *Origin, const Vector3 *Direction) {
	Ray R;
	R.Origin    = *Origin;
	R.Direction = *Direction;
	return R;
}

Vector3 SabinaRayPointAt(const Ray *R, Real T) {
	Vector3 Offset = KannaVector3Scale(&R->Direction, T);
	return KannaVector3Add(&R->Origin, &Offset);
}

Real SabinaRayClosestPoint(const Ray *R, const Vector3 *P) {
	Vector3 ToP = KannaVector3Sub(P, &R->Origin);
	Real Dot = KannaVector3Dot(&R->Direction, &ToP);
	// For a ray (not line), clamp T >= 0.
	return Dot > REAL_ZERO ? Dot : REAL_ZERO;
}

// ===========================================================================
//  Segment
// ===========================================================================

Segment SabinaSegmentMake(const Vector3 *Start, const Vector3 *End) {
	Segment S;
	S.Start = *Start;
	S.End   = *End;
	return S;
}

Real SabinaSegmentLength(const Segment *S) {
	Vector3 D = KannaVector3Sub(&S->End, &S->Start);
	return SulettaSqrt(KannaVector3LengthSq(&D));
}

Vector3 SabinaSegmentClosestPoint(const Segment *S, const Vector3 *P) {
	Vector3 Axis = KannaVector3Sub(&S->End, &S->Start);
	Vector3 PToS = KannaVector3Sub(P, &S->Start);
	Real LenSq = KannaVector3LengthSq(&Axis);
	Real T = 0.0;
	if (!NagisaIsZero(LenSq)) {
		T = KannaVector3Dot(&PToS, &Axis) / LenSq;
		T = YuuClamp01(T);
	}
	Vector3 Closest = KannaVector3Scale(&Axis, T);
	Closest = KannaVector3Add(&S->Start, &Closest);
	return Closest;
}

// ===========================================================================
//  Triangle
// ===========================================================================

Triangle SabinaTriangleMake(const Vector3 *V0, const Vector3 *V1, const Vector3 *V2) {
	Triangle T;
	T.V0 = *V0;
	T.V1 = *V1;
	T.V2 = *V2;
	return T;
}

Vector3 SabinaTriangleNormal(const Triangle *T) {
	Vector3 AB = KannaVector3Sub(&T->V1, &T->V0);
	Vector3 AC = KannaVector3Sub(&T->V2, &T->V0);
	Vector3 Nml = KannaVector3Cross(&AB, &AC);
	return KannaVector3Normalize(&Nml);
}

Real SabinaTriangleArea(const Triangle *T) {
	Vector3 AB = KannaVector3Sub(&T->V1, &T->V0);
	Vector3 AC = KannaVector3Sub(&T->V2, &T->V0);
	Vector3 Cr = KannaVector3Cross(&AB, &AC);
	return 0.5 * SulettaSqrt(KannaVector3LengthSq(&Cr));
}
