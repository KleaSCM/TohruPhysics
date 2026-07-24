/**
 * GJK — Gilbert-Johnson-Keerthi distance algorithm implementation.
 * TohruPhysics用のGJK距離アルゴリズムの実装ね。
 *
 * Simplex evolution: Vertex → Edge → Triangle → Tetrahedron.
 * Each iteration finds the closest point on the current simplex to the origin
 * and sets a new search direction. Terminates when the new support point
 * doesn't improve the distance or the tolerance is met.
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include <TohruPhysics/GJK.h>
#include <TohruPhysics/Math.h>
#include <string.h>

// ===========================================================================
//  Closest point on line segment to origin
//  線分上の原点に最も近い点ね。
// ===========================================================================
static Real ClosestOnEdge(const Vector3 *A, const Vector3 *B,
                           Vector3 *Closest, Vector3 *Dir)
{
	Vector3 AB = KannaVector3Sub(B, A);
	Vector3 AO = KannaVector3Scale(A, -1.0);
	Real ABdotAB = KannaVector3Dot(&AB, &AB);
	if (ABdotAB <= REAL_ZERO) {
		*Closest = *A;
		*Dir = KannaVector3Scale(A, -1.0);
		return KannaVector3LengthSq(A);
	}

	Real T = KannaVector3Dot(&AB, &AO) / ABdotAB;
	T = YuuClamp01(T);

	Vector3 AT = KannaVector3Scale(&AB, T);
	*Closest = KannaVector3Add(A, &AT);
	*Dir = KannaVector3Scale(Closest, -1.0);
	return KannaVector3LengthSq(Closest);
}

// ===========================================================================
//  Closest point on triangle to origin
//  三角形上の原点に最も近い点ね。
// ===========================================================================
static Real ClosestOnTri(const Vector3 *A, const Vector3 *B, const Vector3 *C,
                          Vector3 *Closest, Vector3 *Dir)
{
	Vector3 AB = KannaVector3Sub(B, A);
	Vector3 AC = KannaVector3Sub(C, A);
	Vector3 AO = KannaVector3Scale(A, -1.0);

	Real ABdotAB = KannaVector3Dot(&AB, &AB);
	Real ACdotAC = KannaVector3Dot(&AC, &AC);
	Real ABdotAC = KannaVector3Dot(&AB, &AC);
	Real ABdotAO = KannaVector3Dot(&AB, &AO);
	Real ACdotAO = KannaVector3Dot(&AC, &AO);

	Real Det = ABdotAB * ACdotAC - ABdotAC * ABdotAC;
	if (NagisaIsZero(Det)) Det = 1.0;

	Real U = (ACdotAC * ABdotAO - ABdotAC * ACdotAO) / Det;
	Real V = (ABdotAB * ACdotAO - ABdotAC * ABdotAO) / Det;
	if (U < 0) U = 0;
	if (V < 0) V = 0;
	if (U + V > 1.0) { Real S = U + V; U /= S; V /= S; }

	Vector3 BU = KannaVector3Scale(&AB, U);
	Vector3 CV = KannaVector3Scale(&AC, V);
	*Closest = KannaVector3Add(A, &BU);
	*Closest = KannaVector3Add(Closest, &CV);
	*Dir = KannaVector3Scale(Closest, -1.0);
	return KannaVector3LengthSq(Closest);
}

// ===========================================================================
//  0141: GJK init
// ===========================================================================

void GJKInit(GJKState *State, const Vector3 *InitialDir,
             const void *ShapeA, GJKSupportFn SupportA,
             const void *ShapeB, GJKSupportFn SupportB,
             Real Tolerance, int MaxIter)
{
	memset(State, 0, sizeof(GJKState));
	State->Tolerance = Tolerance > REAL_ZERO ? Tolerance : (Real)1e-6;
	State->MaxIterations = MaxIter > 0 ? MaxIter : GJK_MAX_ITERATIONS;

	Vector3 NegDir = KannaVector3Scale(InitialDir, -1.0);
	Vector3 PA = SupportA(ShapeA, InitialDir);
	Vector3 NB = SupportB(ShapeB, &NegDir);
	State->Simplex[0].Point = KannaVector3Sub(&PA, &NB);
	State->Simplex[0].Direction = *InitialDir;
	State->SimplexCount = 1;
	State->DistanceSq = KannaVector3LengthSq(&State->Simplex[0].Point);
}

// ===========================================================================
//  0142–0149: GJK main loop
// ===========================================================================

// ===========================================================================
//  0141b: GJK init with cache (warm-start)
// ===========================================================================

void GJKInitCached(GJKState *State, GJKCache *Cache,
                   const void *ShapeA, GJKSupportFn SupportA,
                   const void *ShapeB, GJKSupportFn SupportB,
                   Real Tolerance, int MaxIter)
{
	memset(State, 0, sizeof(GJKState));
	State->Tolerance = Tolerance > REAL_ZERO ? Tolerance : (Real)1e-6;
	State->MaxIterations = MaxIter > 0 ? MaxIter : GJK_MAX_ITERATIONS;

	if (Cache && Cache->SimplexCount > 0 && Cache->SimplexCount <= GJK_SIMPLEX_SIZE) {
		// Reuse cached simplex as starting guess
		for (int I = 0; I < Cache->SimplexCount; I++) {
			State->Simplex[I].Point = Cache->Simplex[I];
			State->Simplex[I].Direction = KannaVector3Scale(&Cache->Simplex[I], -1.0);
		}
		State->SimplexCount = Cache->SimplexCount;
		State->DistanceSq = Cache->DistanceSq;
	} else if (State->SimplexCount == 0) {
		// No cache — single point toward origin from default direction
		Vector3 InitDir = KannaVector3Make(1, 0, 0);
		Vector3 NegDir = KannaVector3Scale(&InitDir, -1.0);
		Vector3 PA = SupportA(ShapeA, &InitDir);
		Vector3 NB = SupportB(ShapeB, &NegDir);
		State->Simplex[0].Point = KannaVector3Sub(&PA, &NB);
		State->Simplex[0].Direction = InitDir;
		State->SimplexCount = 1;
		State->DistanceSq = KannaVector3LengthSq(&State->Simplex[0].Point);
	}
}

void GJKEvaluate(GJKState *State,
                 const void *ShapeA, GJKSupportFn SupportA,
                 const void *ShapeB, GJKSupportFn SupportB)
{
	// Initial search direction = toward origin from first point
	Vector3 Dir = KannaVector3Scale(&State->Simplex[0].Point, -1.0);

	for (State->Iterations = 0; State->Iterations < State->MaxIterations; State->Iterations++) {
		// Normalise search direction
		Real DirLenSq = KannaVector3LengthSq(&Dir);
		if (NagisaIsZero(DirLenSq)) {
			Dir = KannaVector3Make(1, 0, 0);
			DirLenSq = 1.0;
		}
		Vector3 DirN = KannaVector3Scale(&Dir, 1.0 / SulettaSqrt(DirLenSq));

		// 0142: Get new support point
		Vector3 NegDir = KannaVector3Scale(&DirN, -1.0);
		Vector3 PA = SupportA(ShapeA, &DirN);
		Vector3 NB = SupportB(ShapeB, &NegDir);
		Vector3 W = KannaVector3Sub(&PA, &NB);

		// Check if support point is ahead in search direction
		Real WDotDir = KannaVector3Dot(&W, &DirN);
		if (State->SimplexCount >= 2 && WDotDir <= REAL_ZERO) {
			// Support point behind origin — this simplex can't improve
			// サポート点が原点の背後 — このシンプレックスでは改善できない
			State->Converged = 1;
			return;
		}

		// Add support point to simplex
		if (State->SimplexCount >= GJK_SIMPLEX_SIZE) {
			// Tetrahedron full → origin likely inside → overlapping
			State->Degenerate = 1;
			State->DistanceSq = REAL_ZERO;
			return;
		}
		State->Simplex[State->SimplexCount].Point = W;
		State->Simplex[State->SimplexCount].Direction = DirN;
		State->SimplexCount++;

		// Find closest point on simplex to origin
		Vector3 Closest;
		Real NewDistSq;
		switch (State->SimplexCount) {
		case 1:
			Closest = State->Simplex[0].Point;
			NewDistSq = KannaVector3LengthSq(&Closest);
			Dir = KannaVector3Scale(&Closest, -1.0);
			break;
		case 2:
			NewDistSq = ClosestOnEdge(
				&State->Simplex[0].Point,
				&State->Simplex[1].Point,
				&Closest, &Dir);
			break;
		case 3:
			NewDistSq = ClosestOnTri(
				&State->Simplex[0].Point,
				&State->Simplex[1].Point,
				&State->Simplex[2].Point,
				&Closest, &Dir);
			break;
		case 4: {
			// Tetrahedron — try each sub-triangle and edge
			NewDistSq = 1e30;
			// Triangle faces
			int Faces[4][3] = {{1,2,3},{0,2,3},{0,1,3},{0,1,2}};
			for (int F = 0; F < 4; F++) {
				Vector3 TmpC, TmpD;
				Real D = ClosestOnTri(
					&State->Simplex[Faces[F][0]].Point,
					&State->Simplex[Faces[F][1]].Point,
					&State->Simplex[Faces[F][2]].Point,
					&TmpC, &TmpD);
				if (D < NewDistSq) {
					NewDistSq = D; Closest = TmpC; Dir = TmpD;
				}
			}
			// Edges
			int Edges[6][2] = {{0,1},{0,2},{0,3},{1,2},{1,3},{2,3}};
			for (int E = 0; E < 6; E++) {
				Vector3 TmpC, TmpD;
				Real D = ClosestOnEdge(
					&State->Simplex[Edges[E][0]].Point,
					&State->Simplex[Edges[E][1]].Point,
					&TmpC, &TmpD);
				if (D < NewDistSq) {
					NewDistSq = D; Closest = TmpC; Dir = TmpD;
				}
			}

			// 0144: Check if origin is inside tetrahedron
			if (NewDistSq < State->Tolerance * State->Tolerance) {
				State->Degenerate = 1;
				State->DistanceSq = REAL_ZERO;
				return;
			}
			break;
		}
		default:
			NewDistSq = State->DistanceSq;
			break;
		}

		// 0148: Check convergence
		if (NewDistSq >= State->DistanceSq - REAL_EPSILON) {
			State->Converged = 1;
			if (State->DistanceSq <= State->Tolerance * State->Tolerance) {
				State->Degenerate = 1;
			}
			State->DistanceSq = State->DistanceSq;
			return;
		}
		State->DistanceSq = NewDistSq;

		// 0147: Tolerance check — distance effectively zero
		if (NewDistSq <= State->Tolerance * State->Tolerance) {
			State->Converged = 1;
			State->Degenerate = 1;

			if (State->SimplexCount == 3) {
				// Triangle with origin on its plane. Perturb the vertices
				// slightly off-plane so the tetrahedron contains the origin.
				Vector3 E1 = KannaVector3Sub(&State->Simplex[1].Point, &State->Simplex[0].Point);
				Vector3 E2 = KannaVector3Sub(&State->Simplex[2].Point, &State->Simplex[0].Point);
				Vector3 N = KannaVector3Cross(&E1, &E2);
				Real NLen = KannaVector3Length(&N);
				if (NLen > REAL_EPSILON) {
					N = KannaVector3Scale(&N, 1.0 / NLen);
					// Perturb triangle vertices slightly in +N direction
					// 三角形の頂点を+N方向に微小にずらす
					Real Eps = (Real)1e-6;
					Vector3 Perturb = KannaVector3Scale(&N, Eps);
					State->Simplex[0].Point = KannaVector3Add(&State->Simplex[0].Point, &Perturb);
					State->Simplex[1].Point = KannaVector3Add(&State->Simplex[1].Point, &Perturb);
					State->Simplex[2].Point = KannaVector3Add(&State->Simplex[2].Point, &Perturb);
					// Get 4th vertex on the opposite side (-N)
					// 反対側(-N方向)に4つ目の頂点を取る
					Vector3 NegN = KannaVector3Scale(&N, -1.0);
					Vector3 PA = SupportA(ShapeA, &NegN);
					Vector3 NB = SupportB(ShapeB, &N);
					Vector3 W4 = KannaVector3Sub(&PA, &NB);
					State->Simplex[3].Point = W4;
					State->Simplex[3].Direction = NegN;
					State->SimplexCount = 4;
				} else {
					// Degenerate triangle (collinear or zero-area) — use EPAInit rebuild
					State->SimplexCount = 3;
				}
			}

			return;
		}
	}
}

Vector3 GJKClosestPointOnSimplex(const GJKState *State) {
	switch (State->SimplexCount) {
	case 1: return State->Simplex[0].Point;
	case 2: {
		Vector3 C, D;
		ClosestOnEdge(&State->Simplex[0].Point, &State->Simplex[1].Point, &C, &D);
		return C;
	}
	case 3: {
		Vector3 C, D;
		ClosestOnTri(&State->Simplex[0].Point, &State->Simplex[1].Point,
			&State->Simplex[2].Point, &C, &D);
		return C;
	}
	case 4: {
		Real Best = 1e30;
		Vector3 BestC = KannaVector3Zero();
		int Faces[4][3] = {{1,2,3},{0,2,3},{0,1,3},{0,1,2}};
		for (int F = 0; F < 4; F++) {
			Vector3 C, D;
			Real Dst = ClosestOnTri(
				&State->Simplex[Faces[F][0]].Point,
				&State->Simplex[Faces[F][1]].Point,
				&State->Simplex[Faces[F][2]].Point, &C, &D);
			if (Dst < Best) { Best = Dst; BestC = C; }
		}
		return BestC;
	}
	default: return KannaVector3Zero();
	}
}

// ===========================================================================
//  ClosestOnTriWithBary — closest point on triangle to origin + barycentric
//  三角形上の原点に最も近い点＋重心座標ね。
// ===========================================================================
static Real ClosestOnTriWithBary(const Vector3 *A, const Vector3 *B, const Vector3 *C,
                                  Vector3 *Closest, Real Bary[3])
{
	Vector3 AB = KannaVector3Sub(B, A);
	Vector3 AC = KannaVector3Sub(C, A);
	Vector3 AO = KannaVector3Scale(A, -1.0);

	Real ABdotAB = KannaVector3Dot(&AB, &AB);
	Real ACdotAC = KannaVector3Dot(&AC, &AC);
	Real ABdotAC = KannaVector3Dot(&AB, &AC);
	Real ABdotAO = KannaVector3Dot(&AB, &AO);
	Real ACdotAO = KannaVector3Dot(&AC, &AO);

	Real Det = ABdotAB * ACdotAC - ABdotAC * ABdotAC;
	if (NagisaIsZero(Det)) Det = 1.0;

	Real U = (ACdotAC * ABdotAO - ABdotAC * ACdotAO) / Det;
	Real V = (ABdotAB * ACdotAO - ABdotAC * ABdotAO) / Det;
	Real W = 0;
	if (U < 0) { U = 0; }
	if (V < 0) { V = 0; }
	if (U + V > 1.0) { Real S = U + V; if (S > 0) { U /= S; V /= S; } else { U = 0; V = 1; } }
	W = 1.0 - U - V;

	Vector3 BU = KannaVector3Scale(&AB, U);
	Vector3 CV = KannaVector3Scale(&AC, V);
	*Closest = KannaVector3Add(A, &BU);
	*Closest = KannaVector3Add(Closest, &CV);

	Bary[0] = W;  // weight for A
	Bary[1] = U;  // weight for B
	Bary[2] = V;  // weight for C

	return KannaVector3LengthSq(Closest);
}

// ===========================================================================
//  EPA — Expanding Polytope Algorithm (Section 1.17)
// ===========================================================================

// Compute face normal pointing outward (away from origin)
static void EPAComputeFaceNormal(const Vector3 *V0, const Vector3 *V1,
                                  const Vector3 *V2, Vector3 *Normal)
{
	Vector3 E1 = KannaVector3Sub(V1, V0);
	Vector3 E2 = KannaVector3Sub(V2, V0);
	*Normal = KannaVector3Cross(&E1, &E2);
	Real Len = KannaVector3Length(Normal);
	if (!NagisaIsZero(Len)) {
		*Normal = KannaVector3Scale(Normal, 1.0 / Len);
	}
	// Ensure normal points outward (away from origin toward vertex)
	Real Dot = KannaVector3Dot(Normal, V0);
	if (Dot < REAL_ZERO) {
		*Normal = KannaVector3Scale(Normal, -1.0);
	}
}

// Check if the origin is on or very close to a face plane
// 原点がどれかの面の上か非常に近いかをチェック
static int OriginOnFace(const Vector3 Verts[4], int FaceIdx[4][3]) {
	for (int F = 0; F < 4; F++) {
		int I0 = FaceIdx[F][0], I1 = FaceIdx[F][1], I2 = FaceIdx[F][2];
		Vector3 E1 = KannaVector3Sub(&Verts[I1], &Verts[I0]);
		Vector3 E2 = KannaVector3Sub(&Verts[I2], &Verts[I0]);
		Vector3 N = KannaVector3Cross(&E1, &E2);
		Real Len = KannaVector3Length(&N);
		if (NagisaIsZero(Len)) return 1;
		N = KannaVector3Scale(&N, 1.0 / Len);
		Real D = KannaVector3Dot(&N, &Verts[I0]);
		if (D < REAL_ZERO) D = -D;
		if (D < (Real)1e-6) return 1;
	}
	return 0;
}

// 0151: Initialise EPA from GJK's final simplex
void EPAInit(EPAState *E, const GJKState *G,
             const void *ShapeA, GJKSupportFn SupportA,
             const void *ShapeB, GJKSupportFn SupportB,
             Real Tolerance, int MaxIter) {
	E->VertexCount = 0;
	E->FaceCount = 0;
	E->Iterations = 0;
	E->MaxIterations = MaxIter > 0 ? MaxIter : EPA_MAX_ITERATIONS;
	E->Tolerance = Tolerance > REAL_ZERO ? Tolerance : (Real)1e-3;
	E->Converged = 0;
	E->PenetrationDepth = 0;
	E->ContactNormal = KannaVector3Make(0, 1, 0);
	E->ContactPoint = KannaVector3Zero();
	E->Barycentric[0] = E->Barycentric[1] = E->Barycentric[2] = 1.0/3.0;

	// Copy GJK simplex vertices
	int SV = G->SimplexCount < 4 ? G->SimplexCount : 4;
	Vector3 V[4];

	// Check if the GJK has a proper tetrahedron (4 vertices, origin inside)
	int FaceIdx[4][3] = {{0,1,2},{0,2,3},{0,3,1},{1,3,2}};
	int NeedRebuild = 0;
	if (SV >= 4) {
		for (int I = 0; I < 4; I++) V[I] = G->Simplex[I].Point;
		NeedRebuild = OriginOnFace(V, FaceIdx);
	} else {
		NeedRebuild = 1;
	}

	if (NeedRebuild) {
		// Build a balanced tetrahedron using 4 support points.
		// 原点が面上にある場合、4つのサポート点でバランスの取れた四面体を構築。
		Vector3 Dir[4] = {
			KannaVector3Make(1, 1, 1),
			KannaVector3Make(1, -1, -1),
			KannaVector3Make(-1, 1, -1),
			KannaVector3Make(-1, -1, 1)
		};
		for (int I = 0; I < 4; I++) {
			Dir[I] = KannaVector3Normalize(&Dir[I]);
			Vector3 NegDir = KannaVector3Scale(&Dir[I], -1.0);
			Vector3 PA = SupportA(ShapeA, &Dir[I]);
			Vector3 NB = SupportB(ShapeB, &NegDir);
			V[I] = KannaVector3Sub(&PA, &NB);
		}
		E->VertexCount = 4;
	} else {
		E->VertexCount = SV;
	}

	// Copy vertices into EPA state
	for (int I = 0; I < E->VertexCount; I++) {
		E->Vertices[I] = V[I];
	}

	if (E->VertexCount < 4) {
		E->Converged = 1;
		E->PenetrationDepth = SulettaSqrt(G->DistanceSq);
		return;
	}

	// Build initial 4 faces of the tetrahedron
	for (int F = 0; F < 4; F++) {
		EPAFace *Face = &E->Faces[E->FaceCount++];
		Face->Indices[0] = FaceIdx[F][0];
		Face->Indices[1] = FaceIdx[F][1];
		Face->Indices[2] = FaceIdx[F][2];
		Face->Valid = 1;
		EPAComputeFaceNormal(
			&E->Vertices[Face->Indices[0]],
			&E->Vertices[Face->Indices[1]],
			&E->Vertices[Face->Indices[2]],
			&Face->Normal);
		Face->Distance = KannaVector3Dot(&Face->Normal, &E->Vertices[Face->Indices[0]]);
		if (Face->Distance < REAL_ZERO) Face->Distance = -Face->Distance;
	}

	// Check each face: if the opposite vertex is on the normal side,
	// the normal points inward (toward interior). Flip it.
	// 各面をチェック: 反対側の頂点が法線側にあるなら法線は内向き。反転する。
	for (int F = 0; F < 4; F++) {
		int I0 = FaceIdx[F][0], I1 = FaceIdx[F][1], I2 = FaceIdx[F][2];
		int Opp = 6 - I0 - I1 - I2; // 0+1+2+3 - A - B - C
		Vector3 ToOpp = KannaVector3Sub(&E->Vertices[Opp], &E->Vertices[I0]);
		Real DotOpp = KannaVector3Dot(&E->Faces[F].Normal, &ToOpp);
		if (DotOpp > REAL_ZERO) {
			// Opposite vertex is on normal side → flip to point outward
			// 反対側頂点が法線側 → 外向きに反転
			E->Faces[F].Normal = KannaVector3Scale(&E->Faces[F].Normal, -1.0);
		}
	}
}

// 0152–0159: EPA main loop
void EPAEvaluate(EPAState *E,
                 const void *ShapeA, GJKSupportFn SupportA,
                 const void *ShapeB, GJKSupportFn SupportB)
{
	for (E->Iterations = 0; E->Iterations < E->MaxIterations; E->Iterations++) {
		// 0154: Find closest triangle face to origin
		int ClosestFaceIdx = -1;
		Real ClosestDist = 1e30;
		for (int F = 0; F < E->FaceCount; F++) {
			if (!E->Faces[F].Valid) continue;
			if (E->Faces[F].Distance < ClosestDist) {
				ClosestDist = E->Faces[F].Distance;
				ClosestFaceIdx = F;
			}
		}

		if (ClosestFaceIdx < 0) break;

		// Track best penetration depth even if not converged
		// 収束していなくても最深侵入深さを追跡
		if (ClosestDist > E->PenetrationDepth) {
			E->PenetrationDepth = ClosestDist;
			E->ContactNormal = E->Faces[ClosestFaceIdx].Normal;
		}

		EPAFace *ClosestFace = &E->Faces[ClosestFaceIdx];

		// Get support point in the face normal direction
		Vector3 NegNormal = KannaVector3Scale(&ClosestFace->Normal, -1.0);
		Vector3 PA = SupportA(ShapeA, &ClosestFace->Normal);
		Vector3 NB = SupportB(ShapeB, &NegNormal);
		Vector3 W = KannaVector3Sub(&PA, &NB);

		// 0153: Check if this support point is within tolerance
		Real WDist = KannaVector3Dot(&W, &ClosestFace->Normal);
		if (WDist - ClosestDist < E->Tolerance) {
			// Converged — use this face for contact info
			// 収束 — この面を接触情報として使う
			E->Converged = 1;
			E->PenetrationDepth = ClosestDist;
			E->ContactNormal = ClosestFace->Normal;

			// 0155: Contact point via barycentric
			const Vector3 *V0 = &E->Vertices[ClosestFace->Indices[0]];
			const Vector3 *V1 = &E->Vertices[ClosestFace->Indices[1]];
			const Vector3 *V2 = &E->Vertices[ClosestFace->Indices[2]];
			Vector3 Closest;
			ClosestOnTriWithBary(V0, V1, V2, &Closest, E->Barycentric);
			E->ContactPoint = Closest;
			return;
		}

		// 0157: Check for degenerate support
		Real WLenSq = KannaVector3LengthSq(&W);
		if (NagisaIsZero(WLenSq)) {
			E->Converged = 1;
			E->PenetrationDepth = ClosestDist;
			E->ContactNormal = ClosestFace->Normal;
			E->ContactPoint = KannaVector3Zero();
			E->Barycentric[0] = E->Barycentric[1] = E->Barycentric[2] = 1.0/3.0;
			return;
		}

		// Add support point to polytope
		// サポート点をポリトープに追加
		if (E->VertexCount >= EPA_MAX_VERTICES) break;
		int NewIdx = E->VertexCount++;
		E->Vertices[NewIdx] = W;

		// Invalidate faces that can see the new point
		// 新しい点が見える面を無効化
		for (int F = 0; F < E->FaceCount; F++) {
			if (!E->Faces[F].Valid) continue;
			Vector3 ToNew = KannaVector3Sub(&W, &E->Vertices[E->Faces[F].Indices[0]]);
			Real Dot = KannaVector3Dot(&E->Faces[F].Normal, &ToNew);
			if (Dot > REAL_ZERO) {
				E->Faces[F].Valid = 0;
			}
		}

		// Create new faces by connecting new point to edges of visible faces
		// 可視面のエッジに新しい点を接続して新しい面を作成
		// For simplicity, create faces for valid edges
		// Track which edges border visible faces using a simple edge set
		int EdgeCount = 0;
		struct { int A, B; } Edges[EPA_MAX_FACES * 3];

		for (int F = 0; F < E->FaceCount; F++) {
			if (!E->Faces[F].Valid) continue;
			for (int EIdx = 0; EIdx < 3; EIdx++) {
				int A = E->Faces[F].Indices[EIdx];
				int B = E->Faces[F].Indices[(EIdx + 1) % 3];
				// Check if this edge is shared (appears twice → internal edge)
				int Found = 0;
				for (int PE = 0; PE < EdgeCount; PE++) {
					if ((Edges[PE].A == A && Edges[PE].B == B) ||
					    (Edges[PE].A == B && Edges[PE].B == A)) {
						Found = 1;
						// Remove this edge (it's internal)
						Edges[PE] = Edges[--EdgeCount];
						break;
					}
				}
				if (!Found && EdgeCount < EPA_MAX_FACES * 3) {
					Edges[EdgeCount].A = A;
					Edges[EdgeCount].B = B;
					EdgeCount++;
				}
			}
		}

		// Create new faces from edges + new point
		// エッジ＋新しい点から新しい面を作成
		for (int EIdx = 0; EIdx < EdgeCount && E->FaceCount < EPA_MAX_FACES; EIdx++) {
			EPAFace *Face = &E->Faces[E->FaceCount++];
			Face->Indices[0] = Edges[EIdx].A;
			Face->Indices[1] = Edges[EIdx].B;
			Face->Indices[2] = NewIdx;
			Face->Valid = 1;
			EPAComputeFaceNormal(
				&E->Vertices[Face->Indices[0]],
				&E->Vertices[Face->Indices[1]],
				&E->Vertices[Face->Indices[2]],
				&Face->Normal);
			Face->Distance = KannaVector3Dot(&Face->Normal, &E->Vertices[Face->Indices[0]]);
			if (Face->Distance < REAL_ZERO) Face->Distance = -Face->Distance;
		}
	}
}
