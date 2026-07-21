/**
 * ContactManifold — contact point generation implementation.
 * TohruPhysics用の接触点生成の実装ね。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include <TohruPhysics/ContactManifold.h>
#include <TohruPhysics/Math.h>
#include <string.h>

// ===========================================================================
//  Internal helpers
//  内部ヘルパーね。
// ===========================================================================

static Sphere TransformSphere(const Sphere *S, const Transform *Tx) {
	Sphere WS;
	WS.Center = KaedeTransformPoint(Tx, &S->Center);
	WS.Radius = S->Radius;
	return WS;
}

static Capsule TransformCapsule(const Capsule *C, const Transform *Tx) {
	Capsule WC;
	WC.Start = KaedeTransformPoint(Tx, &C->Start);
	WC.End = KaedeTransformPoint(Tx, &C->End);
	WC.Radius = C->Radius;
	return WC;
}

static void AddContact(ContactManifold *M, const Vector3 *Pos,
                        const Vector3 *Normal, Real Penetration)
{
	if (M->PointCount >= MANIFOLD_MAX_POINTS) return;
	int I = M->PointCount++;
	M->Points[I].Position = *Pos;
	M->Points[I].Normal = *Normal;
	M->Points[I].Penetration = Penetration;
	M->Points[I].NormalImpulse = 0;
	M->Points[I].TangentImpulse[0] = 0;
	M->Points[I].TangentImpulse[1] = 0;
	M->Points[I].FeatureA = 0;
	M->Points[I].FeatureB = 0;
	M->Points[I].FeatureTypeA = 0;
	M->Points[I].FeatureTypeB = 0;
	if (Penetration > M->Penetration) M->Penetration = Penetration;
	M->Normal = *Normal;
}

// ===========================================================================
//  OBB internal helpers for clipping
//  OBBクリッピング用内部ヘルパーね。
// ===========================================================================

// Get world-space axes of an OBB
static void GetOBBWorldAxes(const OBB *OBB_WS, Vector3 Axes[3]) {
	Vector3 UX = KannaVector3Make(1,0,0);
	Vector3 UY = KannaVector3Make(0,1,0);
	Vector3 UZ = KannaVector3Make(0,0,1);
	Axes[0] = EuphylliaQuaternionRotateVector(&OBB_WS->Rotation, &UX);
	Axes[1] = EuphylliaQuaternionRotateVector(&OBB_WS->Rotation, &UY);
	Axes[2] = EuphylliaQuaternionRotateVector(&OBB_WS->Rotation, &UZ);
}

// Get the 4 world-space vertices of one face of an OBB
static int GetOBBFaceVerts(const OBB *OBB_WS, int FaceIdx, Vector3 Verts[4],
                            Vector3 *FaceNormalOut)
{
	int Sign = (FaceIdx % 2 == 0) ? 1 : -1;
	int AX = FaceIdx / 2;

	Vector3 Axes[3];
	GetOBBWorldAxes(OBB_WS, Axes);

	Vector3 NS = KannaVector3Scale(&Axes[AX], (Real)Sign);
	*FaceNormalOut = NS;

	int T1 = (AX + 1) % 3;
	int T2 = (AX + 2) % 3;

	Vector3 Offset = KannaVector3Scale(&Axes[AX],
		(Real)Sign * OBB_WS->HalfExtents.Data[AX]);
	Vector3 FC = KannaVector3Add(&OBB_WS->Center, &Offset);

	Vector3 TV1 = KannaVector3Scale(&Axes[T1], OBB_WS->HalfExtents.Data[T1]);
	Vector3 TV2 = KannaVector3Scale(&Axes[T2], OBB_WS->HalfExtents.Data[T2]);

	Vector3 P1 = KannaVector3Add(&FC, &TV1);
	Vector3 P2 = KannaVector3Sub(&FC, &TV1);
	Verts[0] = KannaVector3Add(&P1, &TV2);
	Verts[1] = KannaVector3Add(&P2, &TV2);
	Verts[2] = KannaVector3Sub(&P2, &TV2);
	Verts[3] = KannaVector3Sub(&P1, &TV2);
	return 4;
}

// Find the face (0-5) on an OBB whose outward normal is most aligned with Dir
static int FindFaceMostAligned(const OBB *OBB_WS, const Vector3 *Dir) {
	Vector3 Axes[3];
	GetOBBWorldAxes(OBB_WS, Axes);

	int BestFace = 0;
	Real BestDot = -1e30;
	for (int AX = 0; AX < 3; AX++) {
		for (int Sgn = -1; Sgn <= 1; Sgn += 2) {
			int FI = AX * 2 + (Sgn == 1 ? 0 : 1);
			Vector3 FN = KannaVector3Scale(&Axes[AX], (Real)Sgn);
			Real D = KannaVector3Dot(Dir, &FN);
			if (D > BestDot) { BestDot = D; BestFace = FI; }
		}
	}
	return BestFace;
}

// ===========================================================================
//  0180: Clip polygon against a plane (Sutherland–Hodgman step)
// ===========================================================================

int ClipPolygonToPlane(const Vector3 *InVerts, int InCount,
                        const Plane *ClipPlane,
                        Vector3 *OutVerts, int MaxOut)
{
	if (InCount < 1 || MaxOut < 1) return 0;
	int OutCount = 0;

	Vector3 Prev = InVerts[InCount - 1];
	Real PrevDist = SabinaPlaneSignedDistance(ClipPlane, &Prev);

	for (int I = 0; I < InCount; I++) {
		Vector3 Curr = InVerts[I];
		Real CurrDist = SabinaPlaneSignedDistance(ClipPlane, &Curr);

		if (CurrDist >= REAL_ZERO) {
			if (PrevDist < REAL_ZERO) {
				Real T = -PrevDist / (CurrDist - PrevDist);
				Vector3 Edge = KannaVector3Sub(&Curr, &Prev);
				Vector3 Scaled = KannaVector3Scale(&Edge, T);
				Vector3 Pt = KannaVector3Add(&Prev, &Scaled);
				if (OutCount < MaxOut) OutVerts[OutCount++] = Pt;
			}
			if (OutCount < MaxOut) OutVerts[OutCount++] = Curr;
		} else if (PrevDist >= REAL_ZERO) {
			Real T = -PrevDist / (CurrDist - PrevDist);
			Vector3 Edge = KannaVector3Sub(&Curr, &Prev);
			Vector3 Scaled = KannaVector3Scale(&Edge, T);
			Vector3 Pt = KannaVector3Add(&Prev, &Scaled);
			if (OutCount < MaxOut) OutVerts[OutCount++] = Pt;
		}

		Prev = Curr;
		PrevDist = CurrDist;
	}
	return OutCount;
}

// Clip incident face against the 4 side planes of the reference face
static int ClipIncidentAgainstRef(const Vector3 *IncidentVerts, int InCount,
                                   const Vector3 *RefVerts,
                                   const Vector3 *RefNormal,
                                   Vector3 *OutVerts, int MaxOut)
{
	Vector3 Scratch[16];
	int Count = InCount;
	const Vector3 *Src = IncidentVerts;

	for (int EI = 0; EI < 4; EI++) {
		int Nxt = (EI + 1) % 4;
		Vector3 Edge = KannaVector3Sub(&RefVerts[Nxt], &RefVerts[EI]);
		Vector3 PlaneN = KannaVector3Cross(&Edge, RefNormal);
		Real PlaneNLen = KannaVector3Length(&PlaneN);
		if (NagisaIsZero(PlaneNLen)) continue;
		PlaneN = KannaVector3Scale(&PlaneN, 1.0f / PlaneNLen);

		Real D = KannaVector3Dot(&PlaneN, &RefVerts[EI]);
		Plane CP;
		CP.Normal = PlaneN;
		CP.Distance = D;

		int NewCount = ClipPolygonToPlane(Src, Count, &CP, Scratch, 16);
		if (NewCount == 0) return 0;

		Src = Scratch;
		Count = NewCount;
		if (Count >= MaxOut) break;
	}

	int OutCount = Count < MaxOut ? Count : MaxOut;
	for (int I = 0; I < OutCount; I++) OutVerts[I] = Src[I];
	return OutCount;
}

// ===========================================================================
//  0177: OBB-OBB manifold (SAT + Sutherland–Hodgman clipping)
// ===========================================================================

void ManifoldOBBOBB(const OBB *OBB_A, const Transform *TxA,
                     const OBB *OBB_B, const Transform *TxB,
                     ContactManifold *M)
{
	memset(M, 0, sizeof(ContactManifold));

	// Transform OBBs to world space
	Vector3 WCA = KaedeTransformPoint(TxA, &OBB_A->Center);
	Vector3 WCB = KaedeTransformPoint(TxB, &OBB_B->Center);
	Quaternion WRA = EuphylliaQuaternionMul(&TxA->Rotation, &OBB_A->Rotation);
	Quaternion WRB = EuphylliaQuaternionMul(&TxB->Rotation, &OBB_B->Rotation);

	OBB OBB_WS_A, OBB_WS_B;
	OBB_WS_A.Center = WCA;
	OBB_WS_A.HalfExtents = OBB_A->HalfExtents;
	OBB_WS_A.Rotation = WRA;
	OBB_WS_B.Center = WCB;
	OBB_WS_B.HalfExtents = OBB_B->HalfExtents;
	OBB_WS_B.Rotation = WRB;

	// Collision normal: direction from B's center to A's center
	Vector3 AB = KannaVector3Sub(&WCA, &WCB);
	Real ABLen = KannaVector3Length(&AB);
	Vector3 CollisionNormal;
	if (ABLen > REAL_EPSILON) {
		CollisionNormal = KannaVector3Scale(&AB, 1.0f / ABLen);
	} else {
		CollisionNormal = KannaVector3Make(0, 1, 0);
	}

	// Find reference and incident faces
	Vector3 NegNormal = KannaVector3Scale(&CollisionNormal, -1.0f);
	int RefFace = FindFaceMostAligned(&OBB_WS_A, &CollisionNormal);
	int IncFace = FindFaceMostAligned(&OBB_WS_B, &NegNormal);

	Vector3 RefVerts[4], RefNormal, IncVerts[4], IncNormal;
	GetOBBFaceVerts(&OBB_WS_A, RefFace, RefVerts, &RefNormal);
	GetOBBFaceVerts(&OBB_WS_B, IncFace, IncVerts, &IncNormal);

	// Clip the incident face against the reference face's side planes
	Vector3 Clipped[8];
	int ClipCount = ClipIncidentAgainstRef(IncVerts, 4,
		RefVerts, &RefNormal, Clipped, 8);

	if (ClipCount == 0) return;

	Real RefOffset = KannaVector3Dot(&RefNormal, &RefVerts[0]);
	for (int I = 0; I < ClipCount && M->PointCount < MANIFOLD_MAX_POINTS; I++) {
		Real Dist = KannaVector3Dot(&RefNormal, &Clipped[I]) - RefOffset;
		if (Dist < REAL_EPSILON * 10.0f) {
			Vector3 NS = KannaVector3Scale(&RefNormal, Dist);
			Vector3 Proj = KannaVector3Sub(&Clipped[I], &NS);
			AddContact(M, &Proj, &CollisionNormal, -Dist);
		}
	}

	if (M->PointCount == 0) {
		Vector3 Mid = KannaVector3Add(&WCA, &WCB);
		Mid = KannaVector3Scale(&Mid, 0.5f);
		AddContact(M, &Mid, &CollisionNormal, 0);
	}
}

// ===========================================================================
//  0175: AABB-AABB manifold
// ===========================================================================

void ManifoldAABBAABB(const AABB *BoxA, const Transform *TxA,
                       const AABB *BoxB, const Transform *TxB,
                       ContactManifold *M)
{
	memset(M, 0, sizeof(ContactManifold));

	Vector3 CA = SabinaAABBCenter(BoxA);
	Vector3 HA = SabinaAABBHalfExtents(BoxA);
	Vector3 CB = SabinaAABBCenter(BoxB);
	Vector3 HB = SabinaAABBHalfExtents(BoxB);

	Quaternion Id = EuphylliaQuaternionIdentity();

	OBB OBA, OBB;
	OBA.Center = CA; OBA.HalfExtents = HA; OBA.Rotation = Id;
	OBB.Center = CB; OBB.HalfExtents = HB; OBB.Rotation = Id;

	ManifoldOBBOBB(&OBA, TxA, &OBB, TxB, M);
}

// ===========================================================================
//  0176: AABB-OBB manifold
// ===========================================================================

void ManifoldAABBOBB(const AABB *Box, const Transform *TxBox,
                      const OBB *O, const Transform *TxO,
                      ContactManifold *M)
{
	memset(M, 0, sizeof(ContactManifold));

	Vector3 C = SabinaAABBCenter(Box);
	Vector3 H = SabinaAABBHalfExtents(Box);
	Quaternion Id = EuphylliaQuaternionIdentity();

	OBB OBB_Box;
	OBB_Box.Center = C;
	OBB_Box.HalfExtents = H;
	OBB_Box.Rotation = Id;

	ManifoldOBBOBB(&OBB_Box, TxBox, O, TxO, M);
}

// ===========================================================================
//  0171: Sphere-Sphere manifold
// ===========================================================================

void ManifoldSphereSphere(const Sphere *SphA, const Transform *TxA,
                           const Sphere *SphB, const Transform *TxB,
                           ContactManifold *M)
{
	memset(M, 0, sizeof(ContactManifold));

	Sphere WSA = TransformSphere(SphA, TxA);
	Sphere WSB = TransformSphere(SphB, TxB);

	Vector3 AB = KannaVector3Sub(&WSB.Center, &WSA.Center);
	Real DistSq = KannaVector3LengthSq(&AB);
	Real RadiusSum = WSA.Radius + WSB.Radius;

	if (DistSq >= RadiusSum * RadiusSum) return;

	Real Dist = SulettaSqrt(DistSq);
	Vector3 Normal;
	if (Dist > REAL_EPSILON) {
		Normal = KannaVector3Scale(&AB, 1.0f / Dist);
	} else {
		Normal = KannaVector3Make(0, 1, 0);
	}

	Real Penetration = RadiusSum - Dist;
	Vector3 Sum = KannaVector3Add(&WSA.Center, &WSB.Center);
	Vector3 Mid = KannaVector3Scale(&Sum, 0.5f);
	AddContact(M, &Mid, &Normal, Penetration);
}

// ===========================================================================
//  0172: Sphere-AABB manifold
// ===========================================================================

void ManifoldSphereAABB(const Sphere *Sph, const Transform *TxS,
                         const AABB *Box, const Transform *TxB,
                         ContactManifold *M)
{
	memset(M, 0, sizeof(ContactManifold));

	Sphere WS = TransformSphere(Sph, TxS);

	Vector3 BC = SabinaAABBCenter(Box);
	Vector3 BH = SabinaAABBHalfExtents(Box);
	Quaternion Id = EuphylliaQuaternionIdentity();

	OBB OBB_B;
	OBB_B.Center = BC;
	OBB_B.HalfExtents = BH;
	OBB_B.Rotation = Id;

	Vector3 WBO = KaedeTransformPoint(TxB, &OBB_B.Center);
	Quaternion WBR = EuphylliaQuaternionMul(&TxB->Rotation, &OBB_B.Rotation);

	OBB OBB_WS;
	OBB_WS.Center = WBO;
	OBB_WS.HalfExtents = OBB_B.HalfExtents;
	OBB_WS.Rotation = WBR;

	Vector3 Closest = SabinaOBBClosestPoint(&OBB_WS, &WS.Center);
	Vector3 Delta = KannaVector3Sub(&Closest, &WS.Center);
	Real DistSq = KannaVector3LengthSq(&Delta);

	if (DistSq >= WS.Radius * WS.Radius) return;

	Real Dist = SulettaSqrt(DistSq);
	Vector3 Normal;
	if (Dist > REAL_EPSILON) {
		Normal = KannaVector3Scale(&Delta, -1.0f / Dist);
	} else {
		Normal = KannaVector3Make(0, 1, 0);
	}

	Real Penetration = WS.Radius - Dist;
	Real HalfPen = Penetration * 0.5f;
	Vector3 NS = KannaVector3Scale(&Normal, WS.Radius - HalfPen);
	Vector3 CP = KannaVector3Sub(&WS.Center, &NS);
	AddContact(M, &CP, &Normal, Penetration);
}

// ===========================================================================
//  0173: Sphere-OBB manifold
// ===========================================================================

void ManifoldSphereOBB(const Sphere *Sph, const Transform *TxS,
                        const OBB *O, const Transform *TxO,
                        ContactManifold *M)
{
	memset(M, 0, sizeof(ContactManifold));

	Sphere WS = TransformSphere(Sph, TxS);

	Vector3 WCO = KaedeTransformPoint(TxO, &O->Center);
	Quaternion WRO = EuphylliaQuaternionMul(&TxO->Rotation, &O->Rotation);
	OBB OBB_WS;
	OBB_WS.Center = WCO;
	OBB_WS.HalfExtents = O->HalfExtents;
	OBB_WS.Rotation = WRO;

	Vector3 Closest = SabinaOBBClosestPoint(&OBB_WS, &WS.Center);
	Vector3 Delta = KannaVector3Sub(&Closest, &WS.Center);
	Real DistSq = KannaVector3LengthSq(&Delta);

	if (DistSq >= WS.Radius * WS.Radius) return;

	Real Dist = SulettaSqrt(DistSq);
	Vector3 Normal;
	if (Dist > REAL_EPSILON) {
		Normal = KannaVector3Scale(&Delta, -1.0f / Dist);
	} else {
		Normal = KannaVector3Make(0, 1, 0);
	}

	Real Penetration = WS.Radius - Dist;
	Real HalfPen = Penetration * 0.5f;
	Vector3 NS = KannaVector3Scale(&Normal, WS.Radius - HalfPen);
	Vector3 CP = KannaVector3Sub(&WS.Center, &NS);
	AddContact(M, &CP, &Normal, Penetration);
}

// ===========================================================================
//  0174: Sphere-Capsule manifold
// ===========================================================================

void ManifoldSphereCapsule(const Sphere *Sph, const Transform *TxS,
                            const Capsule *Cap, const Transform *TxC,
                            ContactManifold *M)
{
	memset(M, 0, sizeof(ContactManifold));

	Sphere WS = TransformSphere(Sph, TxS);
	Capsule WC = TransformCapsule(Cap, TxC);

	Vector3 Closest = SabinaCapsuleClosestPoint(&WC, &WS.Center);
	Vector3 Delta = KannaVector3Sub(&Closest, &WS.Center);
	Real DistSq = KannaVector3LengthSq(&Delta);
	Real RadiusSum = WS.Radius + WC.Radius;

	if (DistSq >= RadiusSum * RadiusSum) return;

	Real Dist = SulettaSqrt(DistSq);
	Vector3 Normal;
	if (Dist > REAL_EPSILON) {
		Normal = KannaVector3Scale(&Delta, -1.0f / Dist);
	} else {
		Normal = KannaVector3Make(0, 1, 0);
	}

	Real Penetration = RadiusSum - Dist;
	Real HalfPen = Penetration * 0.5f;
	Vector3 NS = KannaVector3Scale(&Normal, WS.Radius - HalfPen);
	Vector3 CP = KannaVector3Sub(&WS.Center, &NS);
	AddContact(M, &CP, &Normal, Penetration);
}

// ===========================================================================
//  0178: Capsule-Capsule manifold
// ===========================================================================

void ManifoldCapsuleCapsule(const Capsule *CapA, const Transform *TxA,
                             const Capsule *CapB, const Transform *TxB,
                             ContactManifold *M)
{
	memset(M, 0, sizeof(ContactManifold));

	Capsule WCA = TransformCapsule(CapA, TxA);
	Capsule WCB = TransformCapsule(CapB, TxB);

	Segment SegA;
	SegA.Start = WCA.Start;
	SegA.End = WCA.End;
	Segment SegB;
	SegB.Start = WCB.Start;
	SegB.End = WCB.End;

	Vector3 PA, PB;
	SabinaSegmentClosestPointBetween(&SegA, &SegB, &PA, &PB);

	Vector3 AB = KannaVector3Sub(&PB, &PA);
	Real DistSq = KannaVector3LengthSq(&AB);
	Real RadiusSum = WCA.Radius + WCB.Radius;

	if (DistSq >= RadiusSum * RadiusSum) return;

	Real Dist = SulettaSqrt(DistSq);
	Vector3 Normal;
	if (Dist > REAL_EPSILON) {
		Normal = KannaVector3Scale(&AB, 1.0f / Dist);
	} else {
		Normal = KannaVector3Make(0, 1, 0);
	}

	Real Penetration = RadiusSum - Dist;
	Vector3 Sum = KannaVector3Add(&PA, &PB);
	Vector3 ContactPos = KannaVector3Scale(&Sum, 0.5f);
	AddContact(M, &ContactPos, &Normal, Penetration);
}

// ===========================================================================
//  0179: General convex-convex via GJK+EPA
// ===========================================================================

void ManifoldConvexConvex(const void *ShapeA, GJKSupportFn SupportA,
                           const void *ShapeB, GJKSupportFn SupportB,
                           Real MaxPenetration,
                           ContactManifold *M)
{
	(void)MaxPenetration;
	memset(M, 0, sizeof(ContactManifold));

	Vector3 InitDir = KannaVector3Make(1, 0, 0);

	GJKState G;
	GJKInit(&G, &InitDir, ShapeA, SupportA, ShapeB, SupportB, (Real)1e-6f, 64);
	GJKEvaluate(&G, ShapeA, SupportA, ShapeB, SupportB);

	if (!G.Degenerate) return;

	EPAState E;
	EPAInit(&E, &G, (Real)1e-6f, 64);
	EPAEvaluate(&E, ShapeA, SupportA, ShapeB, SupportB);

	if (E.Converged && E.PenetrationDepth > REAL_ZERO) {
		AddContact(M, &E.ContactPoint, &E.ContactNormal, E.PenetrationDepth);
	}
}
