/**
 * GJK — Gilbert-Johnson-Keerthi distance algorithm implementation.
 * TohruPhysics用のGJK距離アルゴリズムの実装ね。
 *
 * Simplex evolution: Vertex → Edge → Triangle → Tetrahedron.
 * Each step finds the closest point on the current simplex to the origin
 * and sets a new search direction.
 *
 * References:
 * - Gilbert, Johnson, Keerthi (1988)
 * - van den Bergen, Gino. "Collision Detection in Interactive 3D"
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include <TohruPhysics/GJK.h>
#include <TohruPhysics/Math.h>
#include <string.h>

// ===========================================================================
//  Simplex helper: closest point on line segment to origin
//  線分上の原点に最も近い点ね。
// ===========================================================================
static Real ClosestPointOnEdge(const Vector3 *A, const Vector3 *B,
                                Vector3 *Closest, Vector3 *Dir)
{
	Vector3 AB = KannaVector3Sub(B, A);
	Vector3 AO = KannaVector3Scale(A, -1.0); // origin - A

	Real ABdotAB = KannaVector3Dot(&AB, &AB);
	Real ABdotAO = KannaVector3Dot(&AB, &AO);

	if (ABdotAB <= REAL_ZERO) {
		*Closest = *A;
		*Dir = AO;
		return 0;
	}

	Real T = ABdotAO / ABdotAB;
	if (T < REAL_ZERO) T = REAL_ZERO;
	if (T > 1.0) T = 1.0;

	Vector3 ABT = KannaVector3Scale(&AB, T);
	*Closest = KannaVector3Add(A, &ABT);
	*Dir = KannaVector3Scale(Closest, -1.0);
	return KannaVector3LengthSq(Dir);
}

// ===========================================================================
//  Simplex helper: closest point on triangle to origin
//  三角形上の原点に最も近い点ね。
// ===========================================================================
static Real ClosestPointOnTriangle(const Vector3 *A, const Vector3 *B, const Vector3 *C,
                                    Vector3 *Closest, Vector3 *Dir)
{
	// Test each edge region using barycentric coordinates
	// 各辺の領域を重心座標でテストするの。
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

	// Clamp to triangle interior
	// 三角形内部にクランプ
	if (U < 0) U = 0;
	if (V < 0) V = 0;
	if (U + V > 1.0) {
		Real Sum = U + V;
		U /= Sum;
		V /= Sum;
	}

	Vector3 ABU = KannaVector3Scale(&AB, U);
	Vector3 ACV = KannaVector3Scale(&AC, V);
	Vector3 P = KannaVector3Add(A, &ABU);
	*Closest = KannaVector3Add(&P, &ACV);
	*Dir = KannaVector3Scale(Closest, -1.0);
	return KannaVector3LengthSq(Dir);
}

// ===========================================================================
//  0141: GJK initialisation
// ===========================================================================

void GJKInit(GJKState *State, const Vector3 *InitialDir,
             const void *ShapeA, GJKSupportFn SupportA,
             const void *ShapeB, GJKSupportFn SupportB,
             Real Tolerance, int MaxIter)
{
	memset(State, 0, sizeof(GJKState));
	State->Tolerance = Tolerance > REAL_ZERO ? Tolerance : (Real)1e-6;
	State->MaxIterations = MaxIter > 0 ? MaxIter : GJK_MAX_ITERATIONS;
	State->Degenerate = 0;
	State->Converged = 0;

	// Compute initial support point
	// 最初のサポート点を計算するの
	Vector3 NegDir = KannaVector3Scale(InitialDir, -1.0);
	Vector3 PA = SupportA(ShapeA, InitialDir);
	Vector3 NB = SupportB(ShapeB, &NegDir);
	Vector3 W = KannaVector3Sub(&PA, &NB);

	State->Simplex[0].Point = W;
	State->Simplex[0].Direction = *InitialDir;
	State->SimplexCount = 1;
	State->DistanceSq = KannaVector3LengthSq(&W);
}

// ===========================================================================
//  0142–0149: Full GJK iteration loop
// ===========================================================================

void GJKEvaluate(GJKState *State,
                 const void *ShapeA, GJKSupportFn SupportA,
                 const void *ShapeB, GJKSupportFn SupportB)
{
	Vector3 Dir = State->Simplex[0].Point;

	for (State->Iterations = 0; State->Iterations < State->MaxIterations; State->Iterations++) {
		// Direction toward origin
		// 原点に向かう方向
		Real DirLenSq = KannaVector3LengthSq(&Dir);
		if (NagisaIsZero(DirLenSq)) {
			Dir = KannaVector3Make(1, 0, 0);
			DirLenSq = 1.0;
		}
		Vector3 DirN = KannaVector3Scale(&Dir, 1.0 / SulettaSqrt(DirLenSq));

		// Get new support point
		// 新しいサポート点を取得
		Vector3 NegDir = KannaVector3Scale(&DirN, -1.0);
		Vector3 PA = SupportA(ShapeA, &DirN);
		Vector3 NB = SupportB(ShapeB, &NegDir);
		Vector3 W = KannaVector3Sub(&PA, &NB);

		// 0143: Termination — support point doesn't improve distance
		// 終了条件 — サポート点が距離を改善しない
		Real WDotDir = KannaVector3Dot(&W, &DirN);
		if (WDotDir - State->DistanceSq < -REAL_EPSILON) {
			// Not making progress — accept current distance
			// 進行なし — 現在の距離で受理
			State->Converged = 1;
			return;
		}

		// Add w to simplex
		// wをシンプレックスに追加
		if (State->SimplexCount >= GJK_SIMPLEX_SIZE) {
			// 0145 & 0147: Origin likely inside — overlap detected
			// 原点が内部にある可能性大 — 重なり検出
			State->Degenerate = 1;
			State->DistanceSq = REAL_ZERO;
			return;
		}
		State->Simplex[State->SimplexCount].Point = W;
		State->Simplex[State->SimplexCount].Direction = DirN;
		State->SimplexCount++;

		// Update closest point on simplex and new direction
		// シンプレックス上の最近接点と新しい方向を更新
		Vector3 Closest;
		switch (State->SimplexCount) {
		case 1:
			// Vertex
			Closest = State->Simplex[0].Point;
			break;

		case 2: {
			// Edge
			Real D = ClosestPointOnEdge(
				&State->Simplex[0].Point,
				&State->Simplex[1].Point,
				&Closest, &Dir);
			State->DistanceSq = D;
			break;
		}

		case 3: {
			// Triangle
			Real D = ClosestPointOnTriangle(
				&State->Simplex[0].Point,
				&State->Simplex[1].Point,
				&State->Simplex[2].Point,
				&Closest, &Dir);
			State->DistanceSq = D;
			break;
		}

		case 4: {
			// Tetrahedron — origin may be inside
			// 四面体 — 原点が内部にある可能性

			// Compute barycentric coordinates for tetrahedron
			// Use the sub-triangle closest to origin
			Real BestDist = 1e30;
			for (int I = 1; I < 4; I++) {
				for (int J = I + 1; J < 4; J++) {
					Vector3 TmpClose, TmpDir;
					Real D = ClosestPointOnTriangle(
						&State->Simplex[0].Point,
						&State->Simplex[I].Point,
						&State->Simplex[J].Point,
						&TmpClose, &TmpDir);
					if (D < BestDist) {
						BestDist = D;
						Closest = TmpClose;
						Dir = TmpDir;
					}
				}
			}

			// Also check each edge
			for (int I = 0; I < 4; I++) {
				for (int J = I + 1; J < 4; J++) {
					Vector3 TmpClose, TmpDir;
					Real D = ClosestPointOnEdge(
						&State->Simplex[I].Point,
						&State->Simplex[J].Point,
						&TmpClose, &TmpDir);
					if (D < BestDist) {
						BestDist = D;
						Closest = TmpClose;
						Dir = TmpDir;
					}
				}
			}

			State->DistanceSq = BestDist;

			// If origin is very close to the closest point, check inside
			// 原点が最近接点に非常に近い場合、内部チェック
			if (BestDist < State->Tolerance * State->Tolerance) {
				// Check if origin is inside tetrahedron
				Vector3 Tets[4] = {
					State->Simplex[0].Point,
					State->Simplex[1].Point,
					State->Simplex[2].Point,
					State->Simplex[3].Point
				};

				// Simple test: origin should be on same side of all faces
				int Inside = 1;
				for (int F = 0; F < 4 && Inside; F++) {
					int Idx[3] = {0, 0, 0};
					int K = 0;
					for (int V = 0; V < 4; V++) {
						if (V != F) Idx[K++] = V;
					}
					Vector3 E1 = KannaVector3Sub(&Tets[Idx[1]], &Tets[Idx[0]]);
					Vector3 E2 = KannaVector3Sub(&Tets[Idx[2]], &Tets[Idx[0]]);
					Vector3 N = KannaVector3Cross(&E1, &E2);
					Vector3 ToFace = KannaVector3Sub(&Tets[F], &Tets[Idx[0]]);
					if (KannaVector3Dot(&N, &ToFace) > REAL_ZERO) {
						N = KannaVector3Scale(&N, -1.0);
					}
					Vector3 ToOrigin = KannaVector3Scale(&Tets[Idx[0]], -1.0);
					if (KannaVector3Dot(&N, &ToOrigin) > REAL_ZERO) {
						Inside = 0;
					}
				}

				if (Inside) {
					State->Degenerate = 1;
					State->DistanceSq = REAL_ZERO;
					return;
				}
			}
			break;
		}

		default:
			break;
		}

		// 0148: Check convergence
		// 収束チェック
		if (State->DistanceSq <= State->Tolerance * State->Tolerance) {
			State->Converged = 1;
			return;
		}
	}
}

// ===========================================================================
//  Helper: closest point on simplex to origin
//  ヘルパー: シンプレックス上の原点に最も近い点
// ===========================================================================

Vector3 GJKClosestPointOnSimplex(const GJKState *State) {
	Vector3 Dir;
	Vector3 Closest = KannaVector3Zero();

	switch (State->SimplexCount) {
	case 1:
		return State->Simplex[0].Point;
	case 2:
		ClosestPointOnEdge(
			&State->Simplex[0].Point,
			&State->Simplex[1].Point,
			&Closest, &Dir);
		return Closest;
	case 3:
		ClosestPointOnTriangle(
			&State->Simplex[0].Point,
			&State->Simplex[1].Point,
			&State->Simplex[2].Point,
			&Closest, &Dir);
		return Closest;
	case 4: {
		Real BestDist = 1e30;
		Vector3 BestClose = KannaVector3Zero();
		for (int I = 1; I < 4; I++) {
			for (int J = I + 1; J < 4; J++) {
				Vector3 TmpClose, TmpDir;
				Real D = ClosestPointOnTriangle(
					&State->Simplex[0].Point,
					&State->Simplex[I].Point,
					&State->Simplex[J].Point,
					&TmpClose, &TmpDir);
				if (D < BestDist) {
					BestDist = D;
					BestClose = TmpClose;
				}
			}
		}
		return BestClose;
	}
	default:
		return KannaVector3Zero();
	}
}
