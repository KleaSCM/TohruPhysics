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
#include <string.h>

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

AABB SabinaAABBMerge(const AABB *A, const AABB *B) {
	AABB Box;
	Box.Min = KannaVector3Min(&A->Min, &B->Min);
	Box.Max = KannaVector3Max(&A->Max, &B->Max);
	return Box;
}

AABB SabinaAABBExpand(const AABB *Box, Real Margin) {
	AABB E;
	E.Min.Data[0] = Box->Min.Data[0] - Margin;
	E.Min.Data[1] = Box->Min.Data[1] - Margin;
	E.Min.Data[2] = Box->Min.Data[2] - Margin;
	E.Max.Data[0] = Box->Max.Data[0] + Margin;
	E.Max.Data[1] = Box->Max.Data[1] + Margin;
	E.Max.Data[2] = Box->Max.Data[2] + Margin;
	return E;
}

Vector3 SabinaAABBCenter(const AABB *Box) {
	return KannaVector3Midpoint(&Box->Min, &Box->Max);
}

Vector3 SabinaAABBHalfExtents(const AABB *Box) {
	Vector3 D = KannaVector3Sub(&Box->Max, &Box->Min);
	D.Data[0] *= 0.5;
	D.Data[1] *= 0.5;
	D.Data[2] *= 0.5;
	return D;
}

Real SabinaAABBSurfaceArea(const AABB *Box) {
	Vector3 D = KannaVector3Sub(&Box->Max, &Box->Min);
	Real W = YuuAbs(D.Data[0]);
	Real H = YuuAbs(D.Data[1]);
	Real L = YuuAbs(D.Data[2]);
	return 2.0 * (W * H + H * L + L * W);
}

Real SabinaAABBVolume(const AABB *Box) {
	Vector3 D = KannaVector3Sub(&Box->Max, &Box->Min);
	return YuuAbs(D.Data[0]) * YuuAbs(D.Data[1]) * YuuAbs(D.Data[2]);
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

Vector3 SabinaAABBClosestPoint(const AABB *Box, const Vector3 *P) {
	Vector3 C;
	C.Data[0] = YuuClamp(P->Data[0], Box->Min.Data[0], Box->Max.Data[0]);
	C.Data[1] = YuuClamp(P->Data[1], Box->Min.Data[1], Box->Max.Data[1]);
	C.Data[2] = YuuClamp(P->Data[2], Box->Min.Data[2], Box->Max.Data[2]);
	return C;
}

int SabinaAABBIntersectRay(const AABB *Box, const Ray *R, Real *T0, Real *T1) {
	Real TMin = -1e30;
	Real TMax =  1e30;

	for (int I = 0; I < 3; I++) {
		Real InvD = 1.0 / R->Direction.Data[I];
		Real T1s = (Box->Min.Data[I] - R->Origin.Data[I]) * InvD;
		Real T2s = (Box->Max.Data[I] - R->Origin.Data[I]) * InvD;
		if (T1s > T2s) {
			Real Tmp = T1s; T1s = T2s; T2s = Tmp;
		}
		if (T1s > TMin) TMin = T1s;
		if (T2s < TMax) TMax = T2s;
		if (TMin > TMax) return 0;
	}

	if (TMax < REAL_ZERO) return 0;

	*T0 = TMin > REAL_ZERO ? TMin : REAL_ZERO;
	*T1 = TMax;
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

Real SabinaSphereVolume(const Sphere *S) {
	return (4.0 / 3.0) * REAL_PI * S->Radius * S->Radius * S->Radius;
}

Real SabinaSphereSurfaceArea(const Sphere *S) {
	return 4.0 * REAL_PI * S->Radius * S->Radius;
}

int SabinaSphereContains(const Sphere *S, const Vector3 *Point) {
	Real DistSq = KannaVector3DistanceSq(&S->Center, Point);
	return DistSq <= S->Radius * S->Radius;
}

int SabinaSphereOverlaps(const Sphere *A, const Sphere *B) {
	Real DistSq = KannaVector3DistanceSq(&A->Center, &B->Center);
	Real RSum = A->Radius + B->Radius;
	return DistSq <= RSum * RSum;
}

Vector3 SabinaSphereClosestPoint(const Sphere *S, const Vector3 *P) {
	Vector3 D = KannaVector3Sub(P, &S->Center);
	Real DistSq = KannaVector3LengthSq(&D);
	if (DistSq <= S->Radius * S->Radius) {
		return *P;
	}
	Real Dist = SulettaSqrt(DistSq);
	Real InvDist = 1.0 / Dist;
	Vector3 Dir = KannaVector3Scale(&D, InvDist);
	Vector3 Offset = KannaVector3Scale(&Dir, S->Radius);
	return KannaVector3Add(&S->Center, &Offset);
}

int SabinaSphereIntersectRay(const Sphere *S, const Ray *R, Real *T0, Real *T1) {
	Vector3 OC = KannaVector3Sub(&R->Origin, &S->Center);
	Real A = KannaVector3Dot(&R->Direction, &R->Direction);
	Real B = 2.0 * KannaVector3Dot(&OC, &R->Direction);
	Real C = KannaVector3Dot(&OC, &OC) - S->Radius * S->Radius;
	Real Disc = B * B - 4.0 * A * C;
	if (Disc < REAL_ZERO) return 0;

	Real SqrtDisc = SulettaSqrt(Disc);
	Real Inv2A = 1.0 / (2.0 * A);
	Real T1s = (-B - SqrtDisc) * Inv2A;
	Real T2s = (-B + SqrtDisc) * Inv2A;

	if (T2s < REAL_ZERO) return 0;

	*T0 = T1s > REAL_ZERO ? T1s : REAL_ZERO;
	*T1 = T2s;
	return 1;
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
	Vector3 Local = KannaVector3Sub(Point, &Box->Center);
	Quaternion Conj = EuphylliaQuaternionConjugate(&Box->Rotation);
	Local = EuphylliaQuaternionRotateVector(&Conj, &Local);

	for (int I = 0; I < 3; I++) {
		if (Local.Data[I] < -Box->HalfExtents.Data[I]) return 0;
		if (Local.Data[I] >  Box->HalfExtents.Data[I]) return 0;
	}
	return 1;
}

Vector3 SabinaOBBClosestPoint(const OBB *Box, const Vector3 *P) {
	Vector3 Local = KannaVector3Sub(P, &Box->Center);
	Quaternion Conj = EuphylliaQuaternionConjugate(&Box->Rotation);
	Local = EuphylliaQuaternionRotateVector(&Conj, &Local);

	// Clamp in local space.
	Local.Data[0] = YuuClamp(Local.Data[0], -Box->HalfExtents.Data[0], Box->HalfExtents.Data[0]);
	Local.Data[1] = YuuClamp(Local.Data[1], -Box->HalfExtents.Data[1], Box->HalfExtents.Data[1]);
	Local.Data[2] = YuuClamp(Local.Data[2], -Box->HalfExtents.Data[2], Box->HalfExtents.Data[2]);

	// Rotate back to world space.
	Vector3 World = EuphylliaQuaternionRotateVector(&Box->Rotation, &Local);
	return KannaVector3Add(&Box->Center, &World);
}

void SabinaOBBGetCorners(const OBB *Box, Vector3 Corners[8]) {
	Vector3 H = Box->HalfExtents;
	Vector3 Axes[3];
	Vector3 UX = KannaVector3Make(1, 0, 0);
	Vector3 UY = KannaVector3Make(0, 1, 0);
	Vector3 UZ = KannaVector3Make(0, 0, 1);
	Axes[0] = EuphylliaQuaternionRotateVector(&Box->Rotation, &UX);
	Axes[1] = EuphylliaQuaternionRotateVector(&Box->Rotation, &UY);
	Axes[2] = EuphylliaQuaternionRotateVector(&Box->Rotation, &UZ);

	Vector3 Ext[3];
	Ext[0] = KannaVector3Scale(&Axes[0], H.Data[0]);
	Ext[1] = KannaVector3Scale(&Axes[1], H.Data[1]);
	Ext[2] = KannaVector3Scale(&Axes[2], H.Data[2]);

	int I, J, K, Idx = 0;
	for (I = -1; I <= 1; I += 2) {
		for (J = -1; J <= 1; J += 2) {
			for (K = -1; K <= 1; K += 2) {
				Vector3 P = Box->Center;
				Vector3 EI = KannaVector3Scale(&Ext[0], (Real)I);
				Vector3 EJ = KannaVector3Scale(&Ext[1], (Real)J);
				Vector3 EK = KannaVector3Scale(&Ext[2], (Real)K);
				P = KannaVector3Add(&P, &EI);
				P = KannaVector3Add(&P, &EJ);
				P = KannaVector3Add(&P, &EK);
				Corners[Idx++] = P;
			}
		}
	}
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

int SabinaCapsuleContains(const Capsule *C, const Vector3 *P) {
	Vector3 CP = SabinaCapsuleClosestPoint(C, P);
	Real DistSq = KannaVector3DistanceSq(&CP, P);
	return DistSq <= C->Radius * C->Radius + REAL_EPSILON;
}

Vector3 SabinaCapsuleClosestPoint(const Capsule *C, const Vector3 *P) {
	Vector3 Axis = KannaVector3Sub(&C->End, &C->Start);
	Vector3 PToS = KannaVector3Sub(P, &C->Start);
	Real LenSq = KannaVector3LengthSq(&Axis);

	Real T;
	if (NagisaIsZero(LenSq)) {
		T = 0.0;
	} else {
		T = KannaVector3Dot(&PToS, &Axis) / LenSq;
		T = YuuClamp01(T);
	}

	Vector3 Closest = KannaVector3Scale(&Axis, T);
	Closest = KannaVector3Add(&C->Start, &Closest);

	Vector3 ToP = KannaVector3Sub(P, &Closest);
	Real DistSq = KannaVector3LengthSq(&ToP);
	if (DistSq > C->Radius * C->Radius && !NagisaIsZero(DistSq)) {
		Real Dist = SulettaSqrt(DistSq);
		ToP = KannaVector3Scale(&ToP, C->Radius / Dist);
		Closest = KannaVector3Add(&Closest, &ToP);
	}

	return Closest;
}

Real SabinaCapsuleLength(const Capsule *C) {
	return KannaVector3Distance(&C->Start, &C->End);
}

Real SabinaCapsuleSegmentLengthSq(const Capsule *C) {
	return KannaVector3DistanceSq(&C->Start, &C->End);
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

Plane SabinaPlaneNormalize(const Plane *P) {
	Real Len = KannaVector3Length(&P->Normal);
	if (NagisaIsZero(Len)) {
		Plane Z;
		Z.Normal = KannaVector3Make(0, 1, 0);
		Z.Distance = 0;
		return Z;
	}
	Real InvLen = 1.0 / Len;
	Plane N;
	N.Normal   = KannaVector3Scale(&P->Normal, InvLen);
	N.Distance = P->Distance * InvLen;
	return N;
}

Real SabinaPlaneSignedDistance(const Plane *P, const Vector3 *Point) {
	return KannaVector3Dot(&P->Normal, Point) - P->Distance;
}

Real SabinaPlaneDistance(const Plane *P, const Vector3 *Point) {
	return YuuAbs(SabinaPlaneSignedDistance(P, Point));
}

Vector3 SabinaPlaneReflect(const Plane *P, const Vector3 *V) {
	Real D = KannaVector3Dot(&P->Normal, V);
	Vector3 R;
	R.Data[0] = V->Data[0] - 2.0 * D * P->Normal.Data[0];
	R.Data[1] = V->Data[1] - 2.0 * D * P->Normal.Data[1];
	R.Data[2] = V->Data[2] - 2.0 * D * P->Normal.Data[2];
	return R;
}

Vector3 SabinaPlaneProject(const Plane *P, const Vector3 *V) {
	Real D = KannaVector3Dot(&P->Normal, V);
	Vector3 R;
	R.Data[0] = V->Data[0] - D * P->Normal.Data[0];
	R.Data[1] = V->Data[1] - D * P->Normal.Data[1];
	R.Data[2] = V->Data[2] - D * P->Normal.Data[2];
	return R;
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

Real SabinaRayClosestT(const Ray *R, const Vector3 *P) {
	Vector3 ToP = KannaVector3Sub(P, &R->Origin);
	Real Dot = KannaVector3Dot(&R->Direction, &ToP);
	Real LenSq = KannaVector3LengthSq(&R->Direction);
	if (NagisaIsZero(LenSq)) return REAL_ZERO;
	Real T = Dot / LenSq;
	return T > REAL_ZERO ? T : REAL_ZERO;
}

Real SabinaRayIntersectPlane(const Ray *R, const Plane *P) {
	Real Denom = KannaVector3Dot(&R->Direction, &P->Normal);
	if (NagisaIsZero(Denom)) return -1.0;
	Real T = (P->Distance - KannaVector3Dot(&R->Origin, &P->Normal)) / Denom;
	return T >= REAL_ZERO ? T : -1.0;
}

int SabinaRayIntersectSphere(const Ray *R, const Sphere *S, Real *T0, Real *T1) {
	return SabinaSphereIntersectRay(S, R, T0, T1);
}

int SabinaRayIntersectAABB(const Ray *R, const AABB *Box, Real *T0, Real *T1) {
	return SabinaAABBIntersectRay(Box, R, T0, T1);
}

int SabinaRayIntersectTriangle(const Ray *R, const Triangle *T, Real *TOut) {
	// Möller–Trumbore algorithm.
	// モラー・トランボアアルゴリズムね。
	Vector3 E1 = KannaVector3Sub(&T->V1, &T->V0);
	Vector3 E2 = KannaVector3Sub(&T->V2, &T->V0);
	Vector3 Pv = KannaVector3Cross(&R->Direction, &E2);
	Real Det = KannaVector3Dot(&E1, &Pv);

	if (NagisaIsZero(Det)) return 0;
	Real InvDet = 1.0 / Det;

	Vector3 TVec = KannaVector3Sub(&R->Origin, &T->V0);
	Real U = KannaVector3Dot(&TVec, &Pv) * InvDet;
	if (U < REAL_ZERO || U > 1.0) return 0;

	Vector3 Qv = KannaVector3Cross(&TVec, &E1);
	Real V = KannaVector3Dot(&R->Direction, &Qv) * InvDet;
	if (V < REAL_ZERO || U + V > 1.0) return 0;

	*TOut = KannaVector3Dot(&E2, &Qv) * InvDet;
	return *TOut >= REAL_ZERO ? 1 : 0;
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
	return KannaVector3Distance(&S->Start, &S->End);
}

Real SabinaSegmentLengthSq(const Segment *S) {
	return KannaVector3DistanceSq(&S->Start, &S->End);
}

Vector3 SabinaSegmentClosestPoint(const Segment *S, const Vector3 *P) {
	Real T = SabinaSegmentClosestT(S, P);
	Vector3 Axis = KannaVector3Sub(&S->End, &S->Start);
	Vector3 Closest = KannaVector3Scale(&Axis, T);
	Closest = KannaVector3Add(&S->Start, &Closest);
	return Closest;
}

Real SabinaSegmentClosestT(const Segment *S, const Vector3 *P) {
	Vector3 Axis = KannaVector3Sub(&S->End, &S->Start);
	Vector3 PToS = KannaVector3Sub(P, &S->Start);
	Real LenSq = KannaVector3LengthSq(&Axis);
	if (NagisaIsZero(LenSq)) return 0.0;
	Real T = KannaVector3Dot(&PToS, &Axis) / LenSq;
	return YuuClamp01(T);
}

Real SabinaSegmentDistanceToPoint(const Segment *S, const Vector3 *P) {
	Vector3 CP = SabinaSegmentClosestPoint(S, P);
	return KannaVector3Distance(&CP, P);
}

Real SabinaSegmentClosestPointBetween(const Segment *S1, const Segment *S2, Vector3 *P1, Vector3 *P2) {
	Vector3 D1 = KannaVector3Sub(&S1->End, &S1->Start);
	Vector3 D2 = KannaVector3Sub(&S2->End, &S2->Start);
	Vector3 R = KannaVector3Sub(&S1->Start, &S2->Start);

	Real A = KannaVector3Dot(&D1, &D1);
	Real B = KannaVector3Dot(&D1, &D2);
	Real C = KannaVector3Dot(&D2, &D2);
	Real D = KannaVector3Dot(&D1, &R);
	Real E = KannaVector3Dot(&D2, &R);

	Real Det = A * C - B * B;
	Real S, T;

	if (NagisaIsZero(Det)) {
		S = 0.0;
		T = C > REAL_ZERO ? E / C : 0.0;
	} else {
		S = (B * E - C * D) / Det;
		T = (A * E - B * D) / Det;
	}

	S = YuuClamp01(S);
	T = YuuClamp01(T);

	Vector3 PS1 = KannaVector3Scale(&D1, S);
	Vector3 PS2 = KannaVector3Scale(&D2, T);
	*P1 = KannaVector3Add(&S1->Start, &PS1);
	*P2 = KannaVector3Add(&S2->Start, &PS2);

	return KannaVector3Distance(P1, P2);
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

Real SabinaTrianglePerimeter(const Triangle *T) {
	Real A = KannaVector3Distance(&T->V0, &T->V1);
	Real B = KannaVector3Distance(&T->V1, &T->V2);
	Real C = KannaVector3Distance(&T->V2, &T->V0);
	return A + B + C;
}

Vector3 SabinaTriangleClosestPoint(const Triangle *T, const Vector3 *P) {
	// Project onto triangle plane, then barycentric clamp.
	// 三角形平面に投影して、重心座標でクランプするの。
	Vector3 N = SabinaTriangleNormal(T);
	Plane TriPlane = SabinaPlaneMake(&N, KannaVector3Dot(&N, &T->V0));
	Real SD = SabinaPlaneSignedDistance(&TriPlane, P);
	Vector3 NSD = KannaVector3Scale(&N, SD);
	Vector3 Proj = KannaVector3Sub(P, &NSD);

	// Barycentric coordinates of Proj.
	Vector3 V0 = T->V0, V1 = T->V1, V2 = T->V2;

	Vector3 V0V1 = KannaVector3Sub(&V1, &V0);
	Vector3 V0V2 = KannaVector3Sub(&V2, &V0);
	Vector3 V0P = KannaVector3Sub(&Proj, &V0);

	Real D00 = KannaVector3Dot(&V0V1, &V0V1);
	Real D01 = KannaVector3Dot(&V0V1, &V0V2);
	Real D11 = KannaVector3Dot(&V0V2, &V0V2);
	Real D20 = KannaVector3Dot(&V0P, &V0V1);
	Real D21 = KannaVector3Dot(&V0P, &V0V2);

	Real Denom = D00 * D11 - D01 * D01;
	if (NagisaIsZero(Denom)) Denom = 1.0;

	Real V = (D11 * D20 - D01 * D21) / Denom;
	Real W = (D00 * D21 - D01 * D20) / Denom;
	Real U = 1.0 - V - W;

	// Clamp to triangle interior.
	if (U < 0) U = 0;
	if (V < 0) V = 0;
	if (W < 0) W = 0;
	Real Sum = U + V + W;
	if (Sum > REAL_ZERO) {
		U /= Sum;
		V /= Sum;
		W /= Sum;
	} else {
		U = 1.0; V = 0.0; W = 0.0;
	}

	Vector3 CP;
	CP.Data[0] = U * V0.Data[0] + V * V1.Data[0] + W * V2.Data[0];
	CP.Data[1] = U * V0.Data[1] + V * V1.Data[1] + W * V2.Data[1];
	CP.Data[2] = U * V0.Data[2] + V * V1.Data[2] + W * V2.Data[2];
	return CP;
}

void SabinaTriangleBarycentric(const Triangle *T, const Vector3 *P, Real *U, Real *V, Real *W) {
	Vector3 N = SabinaTriangleNormal(T);
	Real Area2 = SabinaTriangleArea(T) * 2.0;
	if (NagisaIsZero(Area2)) {
		*U = 1.0; *V = 0.0; *W = 0.0;
		return;
	}

	Vector3 V0V1 = KannaVector3Sub(&T->V1, &T->V0);
	Vector3 V1V2 = KannaVector3Sub(&T->V2, &T->V1);
	Vector3 V2V0 = KannaVector3Sub(&T->V0, &T->V2);

	Vector3 PToV1 = KannaVector3Sub(&T->V1, P);
	Vector3 PToV2 = KannaVector3Sub(&T->V2, P);
	Vector3 PToV0 = KannaVector3Sub(&T->V0, P);

	Vector3 C0 = KannaVector3Cross(&V1V2, &PToV1);
	Vector3 C1 = KannaVector3Cross(&V2V0, &PToV2);
	Vector3 C2 = KannaVector3Cross(&V0V1, &PToV0);

	*U = KannaVector3Dot(&C0, &N) / Area2;
	*V = KannaVector3Dot(&C1, &N) / Area2;
	*W = KannaVector3Dot(&C2, &N) / Area2;
}

int SabinaTriangleContains(const Triangle *T, const Vector3 *P) {
	Real U, V, W;
	SabinaTriangleBarycentric(T, P, &U, &V, &W);
	return U >= 0.0 && V >= 0.0 && W >= 0.0;
}

int SabinaTriangleIntersectRay(const Triangle *T, const Ray *R, Real *TOut) {
	return SabinaRayIntersectTriangle(R, T, TOut);
}

// ===========================================================================
//  1.12 Cross-type intersection tests
// ===========================================================================

static void ProjectAABB(const AABB *Box, const Vector3 *Axis, Real *OutMin, Real *OutMax) {
	Vector3 Center = SabinaAABBCenter(Box);
	Real CDot = KannaVector3Dot(Axis, &Center);
	Vector3 HE = SabinaAABBHalfExtents(Box);
	Real Extent = YuuAbs(Axis->Data[0]) * HE.Data[0]
		+ YuuAbs(Axis->Data[1]) * HE.Data[1]
		+ YuuAbs(Axis->Data[2]) * HE.Data[2];
	*OutMin = CDot - Extent;
	*OutMax = CDot + Extent;
}

static void ProjectOBB(const OBB *Box, const Vector3 *Axis, Real *OutMin, Real *OutMax) {
	Real Center = KannaVector3Dot(Axis, &Box->Center);
	Quaternion Conj = EuphylliaQuaternionConjugate(&Box->Rotation);
	Vector3 LocalAxis = EuphylliaQuaternionRotateVector(&Conj, Axis);
	Real Extent = YuuAbs(LocalAxis.Data[0]) * Box->HalfExtents.Data[0]
		+ YuuAbs(LocalAxis.Data[1]) * Box->HalfExtents.Data[1]
		+ YuuAbs(LocalAxis.Data[2]) * Box->HalfExtents.Data[2];
	*OutMin = Center - Extent;
	*OutMax = Center + Extent;
}

static int TestOverlapOnAxis(Real AMin, Real AMax, Real BMin, Real BMax) {
	Real Eps = (Real)1e-12;
	if (AMax + Eps < BMin) return 0;
	if (BMax + Eps < AMin) return 0;
	return 1;
}

// 0101
int IntersectSphereSphere(const Sphere *A, const Sphere *B) {
	return SabinaSphereOverlaps(A, B);
}

// 0102
int IntersectSphereAABB(const Sphere *S, const AABB *Box) {
	Vector3 CP = SabinaAABBClosestPoint(Box, &S->Center);
	Real DistSq = KannaVector3DistanceSq(&CP, &S->Center);
	return DistSq <= S->Radius * S->Radius;
}

// 0103
int IntersectSpherePlane(const Sphere *S, const Plane *P) {
	Real D = SabinaPlaneSignedDistance(P, &S->Center);
	return YuuAbs(D) <= S->Radius;
}

// 0104
int IntersectSphereCapsule(const Sphere *S, const Capsule *C) {
	Vector3 CP = SabinaCapsuleClosestPoint(C, &S->Center);
	Real DistSq = KannaVector3DistanceSq(&CP, &S->Center);
	Real RSum = S->Radius + C->Radius;
	return DistSq <= RSum * RSum;
}

// 0105
int IntersectAABBAABB(const AABB *A, const AABB *B) {
	return SabinaAABBOverlaps(A, B);
}

// 0106
int IntersectAABBPlane(const AABB *Box, const Plane *P) {
	Vector3 Center = SabinaAABBCenter(Box);
	Vector3 HE = SabinaAABBHalfExtents(Box);
	Real PD = SabinaPlaneSignedDistance(P, &Center);
	Real Extent = YuuAbs(P->Normal.Data[0]) * HE.Data[0]
		+ YuuAbs(P->Normal.Data[1]) * HE.Data[1]
		+ YuuAbs(P->Normal.Data[2]) * HE.Data[2];
	return YuuAbs(PD) <= Extent;
}

// 0107
int IntersectAABBOBB(const AABB *Box, const OBB *O) {
	Vector3 OAxes[3];
	Vector3 UX = KannaVector3Make(1,0,0);
	Vector3 UY = KannaVector3Make(0,1,0);
	Vector3 UZ = KannaVector3Make(0,0,1);
	OAxes[0] = EuphylliaQuaternionRotateVector(&O->Rotation, &UX);
	OAxes[1] = EuphylliaQuaternionRotateVector(&O->Rotation, &UY);
	OAxes[2] = EuphylliaQuaternionRotateVector(&O->Rotation, &UZ);

	Vector3 WAxes[3];
	WAxes[0] = UX;
	WAxes[1] = UY;
	WAxes[2] = UZ;

	Vector3 Axes[15];
	int AxisCount = 0;
	int I, J;
	for (I = 0; I < 3; I++) Axes[AxisCount++] = WAxes[I];
	for (I = 0; I < 3; I++) Axes[AxisCount++] = OAxes[I];
	for (I = 0; I < 3; I++) {
		for (J = 0; J < 3; J++) {
			Vector3 Cr = KannaVector3Cross(&WAxes[I], &OAxes[J]);
			Real LenSq = KannaVector3LengthSq(&Cr);
			if (!NagisaIsZero(LenSq)) {
				Axes[AxisCount++] = Cr;
			}
		}
	}

	for (I = 0; I < AxisCount; I++) {
		Real AMin, AMax, BMin, BMax;
		ProjectAABB(Box, &Axes[I], &AMin, &AMax);
		ProjectOBB(O, &Axes[I], &BMin, &BMax);
		if (!TestOverlapOnAxis(AMin, AMax, BMin, BMax)) return 0;
	}
	return 1;
}

// 0108
int IntersectOBBOBB(const OBB *A, const OBB *B) {
	Vector3 AAxes[3], BAxes[3];
	Vector3 UX = KannaVector3Make(1,0,0);
	Vector3 UY = KannaVector3Make(0,1,0);
	Vector3 UZ = KannaVector3Make(0,0,1);
	AAxes[0] = EuphylliaQuaternionRotateVector(&A->Rotation, &UX);
	AAxes[1] = EuphylliaQuaternionRotateVector(&A->Rotation, &UY);
	AAxes[2] = EuphylliaQuaternionRotateVector(&A->Rotation, &UZ);
	BAxes[0] = EuphylliaQuaternionRotateVector(&B->Rotation, &UX);
	BAxes[1] = EuphylliaQuaternionRotateVector(&B->Rotation, &UY);
	BAxes[2] = EuphylliaQuaternionRotateVector(&B->Rotation, &UZ);

	Vector3 Axes[15];
	int AxisCount = 0;
	int I, J;
	for (I = 0; I < 3; I++) Axes[AxisCount++] = AAxes[I];
	for (I = 0; I < 3; I++) Axes[AxisCount++] = BAxes[I];
	for (I = 0; I < 3; I++) {
		for (J = 0; J < 3; J++) {
			Vector3 Cr = KannaVector3Cross(&AAxes[I], &BAxes[J]);
			Real LenSq = KannaVector3LengthSq(&Cr);
			if (!NagisaIsZero(LenSq)) {
				Axes[AxisCount++] = Cr;
			}
		}
	}

	for (I = 0; I < AxisCount; I++) {
		Real AMin, AMax, BMin, BMax;
		ProjectOBB(A, &Axes[I], &AMin, &AMax);
		ProjectOBB(B, &Axes[I], &BMin, &BMax);
		if (!TestOverlapOnAxis(AMin, AMax, BMin, BMax)) return 0;
	}
	return 1;
}

// 0109
int IntersectOBBPlane(const OBB *O, const Plane *P) {
	Vector3 Corners[8];
	SabinaOBBGetCorners(O, Corners);
	Real MinD = SabinaPlaneSignedDistance(P, &Corners[0]);
	Real MaxD = MinD;
	for (int I = 1; I < 8; I++) {
		Real D = SabinaPlaneSignedDistance(P, &Corners[I]);
		if (D < MinD) MinD = D;
		if (D > MaxD) MaxD = D;
	}
	return MinD <= 0.0 && MaxD >= 0.0;
}

// 0110
int IntersectCapsuleCapsule(const Capsule *A, const Capsule *B) {
	Segment S1 = SabinaSegmentMake(&A->Start, &A->End);
	Segment S2 = SabinaSegmentMake(&B->Start, &B->End);
	Vector3 P1, P2;
	Real Dist = SabinaSegmentClosestPointBetween(&S1, &S2, &P1, &P2);
	Real RSum = A->Radius + B->Radius;
	return Dist <= RSum + (Real)1e-12;
}

// ===========================================================================
//  1.13 Ray/segment intersection tests
// ===========================================================================

// 0111
int IntersectRaySphere(const Ray *R, const Sphere *S, Real *T0, Real *T1) {
	return SabinaSphereIntersectRay(S, R, T0, T1);
}

// 0112
int IntersectRayAABB(const Ray *R, const AABB *Box, Real *T0, Real *T1) {
	return SabinaAABBIntersectRay(Box, R, T0, T1);
}

// 0113
int IntersectRayOBB(const Ray *R, const OBB *O, Real *T0, Real *T1) {
	// Transform ray into OBB local space, use AABB slab method.
	// レイをOBBローカル空間に変換して、AABBスラブ法を使うの。
	Vector3 LocalOrigin = KannaVector3Sub(&R->Origin, &O->Center);
	Quaternion Conj = EuphylliaQuaternionConjugate(&O->Rotation);
	Vector3 Lo = EuphylliaQuaternionRotateVector(&Conj, &LocalOrigin);
	Vector3 Ld = EuphylliaQuaternionRotateVector(&Conj, &R->Direction);

	Real TMin = -1e30;
	Real TMax =  1e30;
	for (int I = 0; I < 3; I++) {
		Real InvD = 1.0 / Ld.Data[I];
		Real T1s = (-O->HalfExtents.Data[I] - Lo.Data[I]) * InvD;
		Real T2s = ( O->HalfExtents.Data[I] - Lo.Data[I]) * InvD;
		if (T1s > T2s) { Real Tmp = T1s; T1s = T2s; T2s = Tmp; }
		if (T1s > TMin) TMin = T1s;
		if (T2s < TMax) TMax = T2s;
		if (TMin > TMax) { *T0 = *T1 = 0; return 0; }
	}
	if (TMax < REAL_ZERO) { *T0 = *T1 = 0; return 0; }
	*T0 = TMin > REAL_ZERO ? TMin : REAL_ZERO;
	*T1 = TMax;
	return 1;
}

// 0114
int IntersectRayPlane(const Ray *R, const Plane *P, Real *TOut) {
	Real T = SabinaRayIntersectPlane(R, P);
	*TOut = T >= REAL_ZERO ? T : -1.0;
	return T >= REAL_ZERO ? 1 : 0;
}

// 0115
int IntersectRayCapsule(const Ray *R, const Capsule *C, Real *T0, Real *T1) {
	// Test ray against capsule as ray vs infinite cylinder + two hemispheres.
	// Simplified: capsule is treated as ray vs thick segment.
	// Since the spec mostly calls for detection, we use distance-based approach:
	// find closest point on ray to capsule segment, check if within radius.
	*T0 = -1.0; *T1 = -1.0;

	// Closest point on ray to capsule segment axis.
	// レイ上のカプセルセグメントに最も近い点を求めるの。
	Vector3 Axis = KannaVector3Sub(&C->End, &C->Start);
	Vector3 RayOrgToSeg = KannaVector3Sub(&R->Origin, &C->Start);
	Real AxisLenSq = KannaVector3LengthSq(&Axis);
	if (NagisaIsZero(AxisLenSq)) {
		// Degenerate capsule → sphere test
		Sphere S = SabinaSphereMake(&C->Start, C->Radius);
		return SabinaSphereIntersectRay(&S, R, T0, T1);
	}

	// Project ray origin onto capsule axis
	Real T = KannaVector3Dot(&RayOrgToSeg, &Axis) / AxisLenSq;
	T = YuuClamp01(T);
	Vector3 AxisScale = KannaVector3Scale(&Axis, T);
	Vector3 ClosestAxisPt = KannaVector3Add(&C->Start, &AxisScale);

	// Ray to closest capsule point
	Vector3 V = KannaVector3Sub(&ClosestAxisPt, &R->Origin);
	Real VDotD = KannaVector3Dot(&V, &R->Direction);
	Real DirLenSq = KannaVector3LengthSq(&R->Direction);
	if (NagisaIsZero(DirLenSq)) return 0;

	Real RayT = VDotD / DirLenSq;
	Vector3 DirScale = KannaVector3Scale(&R->Direction, RayT);
	Vector3 ClosestRayPt = KannaVector3Add(&R->Origin, &DirScale);
	Real DistSq = KannaVector3DistanceSq(&ClosestRayPt, &ClosestAxisPt);

	if (DistSq > C->Radius * C->Radius) return 0;

	// Simple hit approximation
	*T0 = RayT > REAL_ZERO ? RayT : REAL_ZERO;
	*T1 = RayT;
	return 1;
}

// 0116
int IntersectRayTriangle(const Ray *R, const Triangle *T, Real *TOut) {
	return SabinaRayIntersectTriangle(R, T, TOut);
}

// 0117
int IntersectSegmentSphere(const Segment *Seg, const Sphere *S, Real *T0, Real *T1) {
	// Extend segment to ray, test, then clamp t.
	Vector3 Dir = KannaVector3Sub(&Seg->End, &Seg->Start);
	Real Len = KannaVector3Length(&Dir);
	if (NagisaIsZero(Len)) {
		// Degenerate segment (point) — sphere contains point?
		Sphere Temp = SabinaSphereMake(&S->Center, S->Radius);
		if (SabinaSphereContains(&Temp, &Seg->Start)) {
			*T0 = 0; *T1 = 0;
			return 1;
		}
		*T0 = *T1 = -1;
		return 0;
	}
	Real InvLen = 1.0 / Len;
	Vector3 DirN = KannaVector3Scale(&Dir, InvLen);
	Ray Rr = SabinaRayMake(&Seg->Start, &DirN);
	if (!IntersectRaySphere(&Rr, S, T0, T1)) return 0;

	// Clamp to segment range
	if (*T0 > Len || *T1 < 0) { *T0 = *T1 = -1; return 0; }
	if (*T0 < 0) *T0 = 0;
	if (*T1 > Len) *T1 = Len;
	return 1;
}

// 0118
int IntersectSegmentAABB(const Segment *Seg, const AABB *Box, Real *T0, Real *T1) {
	Vector3 Dir = KannaVector3Sub(&Seg->End, &Seg->Start);
	Real Len = KannaVector3Length(&Dir);
	if (NagisaIsZero(Len)) {
		if (SabinaAABBContains(Box, &Seg->Start)) {
			*T0 = 0; *T1 = 0; return 1;
		}
		*T0 = *T1 = -1; return 0;
	}
	Real InvLen = 1.0 / Len;
	Vector3 DirN = KannaVector3Scale(&Dir, InvLen);
	Ray Rr = SabinaRayMake(&Seg->Start, &DirN);
	if (!IntersectRayAABB(&Rr, Box, T0, T1)) return 0;

	if (*T0 > Len || *T1 < 0) { *T0 = *T1 = -1; return 0; }
	if (*T0 < 0) *T0 = 0;
	if (*T1 > Len) *T1 = Len;
	return 1;
}

// 0119
int IntersectSegmentOBB(const Segment *Seg, const OBB *O, Real *T0, Real *T1) {
	Vector3 Dir = KannaVector3Sub(&Seg->End, &Seg->Start);
	Real Len = KannaVector3Length(&Dir);
	if (NagisaIsZero(Len)) {
		if (SabinaOBBContains(O, &Seg->Start)) {
			*T0 = 0; *T1 = 0; return 1;
		}
		*T0 = *T1 = -1; return 0;
	}
	Real InvLen = 1.0 / Len;
	Vector3 DirN = KannaVector3Scale(&Dir, InvLen);
	Ray Rr = SabinaRayMake(&Seg->Start, &DirN);
	if (!IntersectRayOBB(&Rr, O, T0, T1)) return 0;

	if (*T0 > Len || *T1 < 0) { *T0 = *T1 = -1; return 0; }
	if (*T0 < 0) *T0 = 0;
	if (*T1 > Len) *T1 = Len;
	return 1;
}

// 0120
int IntersectSegmentPlane(const Segment *Seg, const Plane *P, Real *TOut) {
	Real D0 = SabinaPlaneSignedDistance(P, &Seg->Start);
	Real D1 = SabinaPlaneSignedDistance(P, &Seg->End);
	if (D0 * D1 > REAL_ZERO) {
		*TOut = -1.0;
		return 0;
	}
	// Linear interpolation to find zero crossing
	Real Denom = D0 - D1;
	if (NagisaIsZero(Denom)) {
		*TOut = 0.0;
		return YuuAbs(D0) < REAL_EPSILON ? 1 : 0;
	}
	*TOut = D0 / Denom;
	return 1;
}
