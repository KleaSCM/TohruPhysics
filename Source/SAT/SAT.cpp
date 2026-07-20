/**
 * SAT — Separating Axis Theorem implementation.
 * TohruPhysics用の分離軸定理の実装ね。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include <TohruPhysics/SAT.h>
#include <TohruPhysics/Math.h>
#include <string.h>

static void SATClear(SATResult *R) {
	memset(R, 0, sizeof(SATResult));
	R->Normal = KannaVector3Make(0, 1, 0);
}

static void SATRecordAxis(SATResult *R, Real Overlap) {
	if (R->AxisCount < SAT_MAX_AXES) {
		R->Overlaps[R->AxisCount++] = Overlap;
	}
}

// Project an OBB onto an axis, return min/max
static void ProjOBB(const OBB *Box, const Vector3 *Axis, Real *Min, Real *Max) {
	Real C = KannaVector3Dot(Axis, &Box->Center);
	Quaternion Conj = EuphylliaQuaternionConjugate(&Box->Rotation);
	Vector3 LA = EuphylliaQuaternionRotateVector(&Conj, Axis);
	Real E = YuuAbs(LA.Data[0]) * Box->HalfExtents.Data[0]
		+ YuuAbs(LA.Data[1]) * Box->HalfExtents.Data[1]
		+ YuuAbs(LA.Data[2]) * Box->HalfExtents.Data[2];
	*Min = C - E;
	*Max = C + E;
}

// Project an AABB onto an axis
static void ProjAABB(const AABB *Box, const Vector3 *Axis, Real *Min, Real *Max) {
	Vector3 Ctr = { (Box->Min.Data[0] + Box->Max.Data[0]) * 0.5,
	                (Box->Min.Data[1] + Box->Max.Data[1]) * 0.5,
	                (Box->Min.Data[2] + Box->Max.Data[2]) * 0.5 };
	Real C = KannaVector3Dot(Axis, &Ctr);
	Vector3 HE = { (Box->Max.Data[0] - Box->Min.Data[0]) * 0.5,
	               (Box->Max.Data[1] - Box->Min.Data[1]) * 0.5,
	               (Box->Max.Data[2] - Box->Min.Data[2]) * 0.5 };
	Real E = YuuAbs(Axis->Data[0]) * HE.Data[0]
		+ YuuAbs(Axis->Data[1]) * HE.Data[1]
		+ YuuAbs(Axis->Data[2]) * HE.Data[2];
	*Min = C - E;
	*Max = C + E;
}

// Test overlap on a single axis, return overlap amount (negative = separated)
static Real TestAxis(const Vector3 *Axis, Real AMin, Real AMax, Real BMin, Real BMax) {
	if (AMax < BMin || BMax < AMin) return -1.0;
	Real O1 = AMax - BMin;
	Real O2 = BMax - AMin;
	return O1 < O2 ? O1 : O2;
}

// ===========================================================================
//  0161: OBB-OBB
// ===========================================================================

SATResult SintoOBBOBB(const OBB *A, const OBB *B) {
	SATResult R;
	SATClear(&R);

	Vector3 AAxes[3], BAxes[3];
	Vector3 UX = {1,0,0}, UY = {0,1,0}, UZ = {0,0,1};
	AAxes[0] = EuphylliaQuaternionRotateVector(&A->Rotation, &UX);
	AAxes[1] = EuphylliaQuaternionRotateVector(&A->Rotation, &UY);
	AAxes[2] = EuphylliaQuaternionRotateVector(&A->Rotation, &UZ);
	BAxes[0] = EuphylliaQuaternionRotateVector(&B->Rotation, &UX);
	BAxes[1] = EuphylliaQuaternionRotateVector(&B->Rotation, &UY);
	BAxes[2] = EuphylliaQuaternionRotateVector(&B->Rotation, &UZ);

	Vector3 Axes[15];
	int Ac = 0;
	for (int I = 0; I < 3; I++) Axes[Ac++] = AAxes[I];
	for (int I = 0; I < 3; I++) Axes[Ac++] = BAxes[I];
	for (int I = 0; I < 3; I++)
		for (int J = 0; J < 3; J++) {
			Vector3 Cr = KannaVector3Cross(&AAxes[I], &BAxes[J]);
			if (!NagisaIsZero(KannaVector3LengthSq(&Cr)))
				Axes[Ac++] = Cr;
		}

	Real MinOverlap = 1e30;
	int First = 1;
	for (int I = 0; I < Ac; I++) {
		Real AMin, AMax, BMin, BMax;
		ProjOBB(A, &Axes[I], &AMin, &AMax);
		ProjOBB(B, &Axes[I], &BMin, &BMax);
		Real O = TestAxis(&Axes[I], AMin, AMax, BMin, BMax);
		SATRecordAxis(&R, O >= 0 ? O : 0);
		if (O < 0) { R.Intersect = 0; return R; }
		if (First || O < MinOverlap) {
			MinOverlap = O;
			R.Normal = Axes[I];
			First = 0;
		}
	}

	R.Intersect = 1;
	R.PenetrationDepth = MinOverlap;
	// Contact point: midpoint of overlap on deepest axis
	Real AMin, AMax, BMin, BMax;
	ProjOBB(A, &R.Normal, &AMin, &AMax);
	ProjOBB(B, &R.Normal, &BMin, &BMax);
	Real Mid = (AMax < BMax ? AMax : BMax) + (AMin > BMin ? AMin : BMin);
	Mid *= 0.5;
	R.ContactPoint = KannaVector3Scale(&R.Normal, Mid);
	return R;
}

// ===========================================================================
//  0162: AABB-OBB
// ===========================================================================

SATResult SintoAABBOBB(const AABB *Box, const OBB *O) {
	SATResult R;
	SATClear(&R);

	Vector3 OAxes[3];
	Vector3 UX = {1,0,0}, UY = {0,1,0}, UZ = {0,0,1};
	OAxes[0] = EuphylliaQuaternionRotateVector(&O->Rotation, &UX);
	OAxes[1] = EuphylliaQuaternionRotateVector(&O->Rotation, &UY);
	OAxes[2] = EuphylliaQuaternionRotateVector(&O->Rotation, &UZ);

	Vector3 WAxes[3] = {UX, UY, UZ};
	Vector3 Axes[15];
	int Ac = 0;
	for (int I = 0; I < 3; I++) Axes[Ac++] = WAxes[I];
	for (int I = 0; I < 3; I++) Axes[Ac++] = OAxes[I];
	for (int I = 0; I < 3; I++)
		for (int J = 0; J < 3; J++) {
			Vector3 Cr = KannaVector3Cross(&WAxes[I], &OAxes[J]);
			if (!NagisaIsZero(KannaVector3LengthSq(&Cr)))
				Axes[Ac++] = Cr;
		}

	Real MinOverlap = 1e30;
	int First = 1;
	for (int I = 0; I < Ac; I++) {
		Real AMin, AMax, BMin, BMax;
		ProjAABB(Box, &Axes[I], &AMin, &AMax);
		ProjOBB(O, &Axes[I], &BMin, &BMax);
		Real Ovl = TestAxis(&Axes[I], AMin, AMax, BMin, BMax);
		SATRecordAxis(&R, Ovl >= 0 ? Ovl : 0);
		if (Ovl < 0) { R.Intersect = 0; return R; }
		if (First || Ovl < MinOverlap) {
			MinOverlap = Ovl;
			R.Normal = Axes[I];
			First = 0;
		}
	}
	R.Intersect = 1;
	R.PenetrationDepth = MinOverlap;
	return R;
}

// ===========================================================================
//  0163: 2D convex polygon SAT
// ===========================================================================

static Vector2 Perp(const Vector2 *V) {
	Vector2 P = {-V->Y, V->X};
	return P;
}

static Real Dot2(const Vector2 *A, const Vector2 *B) {
	return A->X * B->X + A->Y * B->Y;
}

static Vector2 Sub2(const Vector2 *A, const Vector2 *B) {
	Vector2 R = {A->X - B->X, A->Y - B->Y};
	return R;
}

static void ProjPoly2D(const Vector2 *Verts, int Count,
                       const Vector2 *Axis, Real *Min, Real *Max)
{
	*Min = Dot2(&Verts[0], Axis);
	*Max = *Min;
	for (int I = 1; I < Count; I++) {
		Real D = Dot2(&Verts[I], Axis);
		if (D < *Min) *Min = D;
		if (D > *Max) *Max = D;
	}
}

Vector3 Poly2DTo3D(const Vector2 *V) {
	Vector3 R = {V->X, V->Y, 0};
	return R;
}

SATResult SintoPolyPoly2D(const Vector2 *VertsA, int CountA,
                           const Vector2 *VertsB, int CountB)
{
	SATResult R;
	SATClear(&R);

	Real MinOverlap = 1e30;
	int First = 1;

	// Test edge normals of A
	for (int I = 0; I < CountA; I++) {
		Vector2 E = Sub2(&VertsA[(I + 1) % CountA], &VertsA[I]);
		Vector2 N = Perp(&E);
		Real Len = SulettaSqrt(N.X * N.X + N.Y * N.Y);
		if (NagisaIsZero(Len)) continue;
		N.X /= Len; N.Y /= Len;

		Real AMin, AMax, BMin, BMax;
		ProjPoly2D(VertsA, CountA, &N, &AMin, &AMax);
		ProjPoly2D(VertsB, CountB, &N, &BMin, &BMax);
		Vector3 Axis3D = Poly2DTo3D(&N);
		Real Ovl = TestAxis(&Axis3D, AMin, AMax, BMin, BMax);
		if (Ovl < 0) { R.Intersect = 0; return R; }
		if (First || Ovl < MinOverlap) { MinOverlap = Ovl; First = 0; }
	}

	// Test edge normals of B
	for (int I = 0; I < CountB; I++) {
		Vector2 E = Sub2(&VertsB[(I + 1) % CountB], &VertsB[I]);
		Vector2 N = Perp(&E);
		Real Len = SulettaSqrt(N.X * N.X + N.Y * N.Y);
		if (NagisaIsZero(Len)) continue;
		N.X /= Len; N.Y /= Len;

		Real AMin, AMax, BMin, BMax;
		ProjPoly2D(VertsA, CountA, &N, &AMin, &AMax);
		ProjPoly2D(VertsB, CountB, &N, &BMin, &BMax);
		Vector3 Axis3D = Poly2DTo3D(&N);
		Real Ovl = TestAxis(&Axis3D, AMin, AMax, BMin, BMax);
		if (Ovl < 0) { R.Intersect = 0; return R; }
		if (Ovl < MinOverlap) { MinOverlap = Ovl; }
	}

	R.Intersect = 1;
	R.PenetrationDepth = MinOverlap;
	return R;
}

// ===========================================================================
//  0164: 3D convex polyhedron SAT
// ===========================================================================

static void ProjPoly3D(const Vector3 *Verts, int Count,
                       const Vector3 *Axis, Real *Min, Real *Max)
{
	*Min = KannaVector3Dot(&Verts[0], Axis);
	*Max = *Min;
	for (int I = 1; I < Count; I++) {
		Real D = KannaVector3Dot(&Verts[I], Axis);
		if (D < *Min) *Min = D;
		if (D > *Max) *Max = D;
	}
}

SATResult SintoPolyPoly3D(const Vector3 *VertsA, int CountA,
                           const Vector3 *VertsB, int CountB)
{
	SATResult R;
	SATClear(&R);

	if (CountA < 3 || CountB < 3) {
		R.Intersect = 0;
		return R;
	}

	Vector3 Axes[SAT_MAX_AXES];
	int Ac = 0;

	// Face normals from A (assume first 3 verts define a face)
	Vector3 E1 = KannaVector3Sub(&VertsA[1], &VertsA[0]);
	Vector3 E2 = KannaVector3Sub(&VertsA[2], &VertsA[0]);
	Vector3 FN = KannaVector3Cross(&E1, &E2);
	if (!NagisaIsZero(KannaVector3LengthSq(&FN)))
		Axes[Ac++] = KannaVector3Normalize(&FN);

	// Face normals from B
	E1 = KannaVector3Sub(&VertsB[1], &VertsB[0]);
	E2 = KannaVector3Sub(&VertsB[2], &VertsB[0]);
	FN = KannaVector3Cross(&E1, &E2);
	if (!NagisaIsZero(KannaVector3LengthSq(&FN)))
		Axes[Ac++] = KannaVector3Normalize(&FN);

	// Edge cross products
	for (int I = 0; I < CountA && Ac < SAT_MAX_AXES; I++) {
		Vector3 EA = KannaVector3Sub(&VertsA[(I + 1) % CountA], &VertsA[I]);
		for (int J = 0; J < CountB && Ac < SAT_MAX_AXES; J++) {
			Vector3 EB = KannaVector3Sub(&VertsB[(J + 1) % CountB], &VertsB[J]);
			Vector3 Cr = KannaVector3Cross(&EA, &EB);
			if (!NagisaIsZero(KannaVector3LengthSq(&Cr)))
				Axes[Ac++] = KannaVector3Normalize(&Cr);
		}
	}

	if (Ac == 0) { R.Intersect = 1; return R; }

	Real MinOverlap = 1e30;
	int First = 1;
	for (int I = 0; I < Ac; I++) {
		Real AMin, AMax, BMin, BMax;
		ProjPoly3D(VertsA, CountA, &Axes[I], &AMin, &AMax);
		ProjPoly3D(VertsB, CountB, &Axes[I], &BMin, &BMax);
		Real Ovl = TestAxis(&Axes[I], AMin, AMax, BMin, BMax);
		SATRecordAxis(&R, Ovl >= 0 ? Ovl : 0);
		if (Ovl < 0) { R.Intersect = 0; return R; }
		if (First || Ovl < MinOverlap) {
			MinOverlap = Ovl;
			R.Normal = Axes[I];
			First = 0;
		}
	}
	R.Intersect = 1;
	R.PenetrationDepth = MinOverlap;
	return R;
}
