/**
 * Scalar math implementation — Zero-is-valid throughout.
 * スカラー数学の実装 — 全体がZero-is-validね。
 *
 * NaN/Inf inputs produce 0. Degenerate inputs produce 0.
 * Callers never branch-check results.
 * NaN/Inf入力は0になるの。退化入力も0よ。呼び出し側は結果を分岐チェックしないの。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include <TohruPhysics/Math.h>

// ---------------------------------------------------------------------------
//  Internal helpers
// ---------------------------------------------------------------------------

// Bit-cast helpers for NaN/Inf detection without <cmath>.
// NaN/Inf検出用のビットキャストヘルパー。 <cmath> を使わないの。
typedef union { uint64_t U; double F; } F64Bits;

#define F64_EXP_MASK 0x7FF0000000000000ULL
#define F64_MANT_MASK 0x000FFFFFFFFFFFFFULL
#define F64_EXP_INF 0x7FF0000000000000ULL

// ---------------------------------------------------------------------------
//  Mai — NaN / Inf detection
// ---------------------------------------------------------------------------

int MaiIsNaN(Real V) {
	F64Bits B;
	B.F = V;
	uint64_t Exp = B.U & F64_EXP_MASK;
	uint64_t Mant = B.U & F64_MANT_MASK;
	return (Exp == F64_EXP_INF && Mant != 0) ? 1 : 0;
}

int MaiIsInf(Real V) {
	F64Bits B;
	B.F = V;
	uint64_t Exp = B.U & F64_EXP_MASK;
	uint64_t Mant = B.U & F64_MANT_MASK;
	return (Exp == F64_EXP_INF && Mant == 0) ? 1 : 0;
}

int MaiIsFinite(Real V) {
	F64Bits B;
	B.F = V;
	uint64_t Exp = B.U & F64_EXP_MASK;
	return (Exp != F64_EXP_INF) ? 1 : 0;
}

Real MaiSanitize(Real V) {
	F64Bits B;
	B.F = V;
	uint64_t Exp = B.U & F64_EXP_MASK;
	if (Exp == F64_EXP_INF) {
		return REAL_ZERO;
	}
	return V;
}

// ---------------------------------------------------------------------------
//  Nagisa — bounded comparison
// ---------------------------------------------------------------------------

int NagisaApproxEqual(Real A, Real B, Real Eps) {
	if (Eps < REAL_ZERO) Eps = REAL_ZERO;
	// If either is NaN, return false (0).
	// NaNの場合はfalse(0)を返すの。
	if (MaiIsNaN(A) || MaiIsNaN(B)) return 0;
	Real Diff = A - B;
	return (Diff >= -Eps && Diff <= Eps) ? 1 : 0;
}

int NagisaApproxZero(Real V, Real Eps) {
	if (Eps < REAL_ZERO) Eps = REAL_ZERO;
	if (MaiIsNaN(V)) return 0;
	return (V >= -Eps && V <= Eps) ? 1 : 0;
}

int NagisaEqual(Real A, Real B) {
	return NagisaApproxEqual(A, B, REAL_EPSILON);
}

int NagisaIsZero(Real V) {
	return NagisaApproxZero(V, REAL_EPSILON);
}

// ---------------------------------------------------------------------------
//  Yuu — clamp, abs, wrap, bounds
// ---------------------------------------------------------------------------

Real YuuClamp(Real V, Real Lo, Real Hi) {
	if (MaiIsNaN(V)) return REAL_ZERO;
	// NOTE (KleaSCM) If Lo > Hi, clamp to zero instead of undefined.
	// Lo>Hiのときは未定義ではなく0にクランプするの。
	if (Lo > Hi) return REAL_ZERO;
	if (V < Lo) return Lo;
	if (V > Hi) return Hi;
	return V;
}

Real YuuClamp01(Real V) {
	return YuuClamp(V, REAL_ZERO, 1.0);
}

Real YuuAbs(Real V) {
	if (MaiIsNaN(V)) return REAL_ZERO;
	// Clear sign bit via bitmask.
	// 符号ビットをマスクでクリアするの。
	F64Bits B;
	B.F = V;
	B.U &= ~0x8000000000000000ULL;
	return B.F;
}

Real YuuMin(Real A, Real B) {
	if (MaiIsNaN(A)) A = REAL_ZERO;
	if (MaiIsNaN(B)) B = REAL_ZERO;
	return (A < B) ? A : B;
}

Real YuuMax(Real A, Real B) {
	if (MaiIsNaN(A)) A = REAL_ZERO;
	if (MaiIsNaN(B)) B = REAL_ZERO;
	return (A > B) ? A : B;
}

Real YuuSign(Real V) {
	if (MaiIsNaN(V)) return REAL_ZERO;
	if (V > REAL_ZERO) return 1.0;
	if (V < REAL_ZERO) return -1.0;
	return REAL_ZERO;
}

int64_t YuuClamp64(int64_t V, int64_t Lo, int64_t Hi) {
	if (Lo > Hi) return 0;
	if (V < Lo) return Lo;
	if (V > Hi) return Hi;
	return V;
}

int64_t YuuWrap64(int64_t V, int64_t Lo, int64_t Hi) {
	if (Lo > Hi) return 0;
	int64_t Range = Hi - Lo;
	if (Range == 0) return Lo;
	int64_t Wrapped = ((V - Lo) % Range);
	if (Wrapped < 0) Wrapped += Range;
	return Lo + Wrapped;
}

// ---------------------------------------------------------------------------
//  Suletta — fast trig: range-reduced polynomial.
//  範囲縮約多項式による高速三角関数ね。
// ---------------------------------------------------------------------------

// Reduce V to [-PI, PI] range.
// Vを[-PI, PI]の範囲に縮約するの。
static Real RangeReduce(Real V) {
	if (MaiIsNaN(V)) return REAL_ZERO;
	if (MaiIsInf(V)) return REAL_ZERO;

	// Shift to [-PI, PI] using remainder.
	// 剰余を使って[-PI, PI]にシフトするわ。
	Real N = V / REAL_PI;
	// Truncate toward zero
	int64_t K = (int64_t)N;
	Real R = V - (Real)K * REAL_PI;
	// Adjust to [-PI/2, PI/2] range for better accuracy
	// 精度向上のため[-PI/2, PI/2]範囲に調整するの。
	if (R > REAL_PI_HALF) {
		R = R - REAL_PI;
	} else if (R < -REAL_PI_HALF) {
		R = R + REAL_PI;
	}
	return R;
}

Real SulettaSin(Real V) {
	Real X = RangeReduce(V);
	if (MaiIsNaN(X)) return REAL_ZERO;

	// Taylor-like polynomial: sin(x) ≈ x - x^3/6 + x^5/120 - x^7/5040
	// テイラー級数: sin(x) ≈ x - x^3/6 + x^5/120 - x^7/5040
	Real X2 = X * X;
	Real X3 = X * X2;
	Real X5 = X3 * X2;
	Real X7 = X5 * X2;

	Real Result = X - X3 * (1.0 / 6.0)
		+ X5 * (1.0 / 120.0)
		- X7 * (1.0 / 5040.0);

	return Result;
}

Real SulettaCos(Real V) {
	// cos(x) = sin(x + PI/2)
	// cos(x) = sin(x + PI/2)
	return SulettaSin(V + REAL_PI_HALF);
}

// ---------------------------------------------------------------------------
//  Suletta — fast inverse sqrt (Newton iteration).
//  高速逆平方根（ニュートン法）。
// ---------------------------------------------------------------------------

Real SulettaInvSqrt(Real V) {
	if (MaiIsNaN(V)) return REAL_ZERO;
	if (MaiIsInf(V)) return REAL_ZERO;
	if (V <= REAL_ZERO) return REAL_ZERO;

	// Initial guess via bit manipulation (double precision).
	// ビット操作による初期推定値（倍精度）。
	F64Bits B;
	B.F = V;
	// Magic constant for double: 0x5FE6EB50C7B537A9
	// 倍精度のマジックナンバーね。
	B.U = 0x5FE6EB50C7B537A9ULL - (B.U >> 1);
	Real X = B.F;

	// Two Newton iterations.
	// ニュートン法を2回。
	for (int I = 0; I < 2; I++) {
		X = X * (1.5 - 0.5 * V * X * X);
	}

	return X;
}

Real SulettaSqrt(Real V) {
	if (MaiIsNaN(V)) return REAL_ZERO;
	if (MaiIsInf(V)) return REAL_ZERO;
	if (V <= REAL_ZERO) return REAL_ZERO;

	// sqrt(V) = V * invsqrt(V)
	// sqrt(V) = V * invsqrt(V)
	Real Inv = SulettaInvSqrt(V);
	return V * Inv;
}
