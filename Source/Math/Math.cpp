/**
 * Scalar math implementation.
 * スカラー数学の実装ね。
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

// 9th-order sin approximation on [0, PI/2].
// [0, PI/2]における9次sin近似。
static Real SinApprox(Real X) {
	Real X2 = X * X;
	Real X3 = X * X2;
	Real X5 = X3 * X2;
	Real X7 = X5 * X2;
	Real X9 = X7 * X2;

	return X - X3 * (1.0 / 6.0)
		+ X5 * (1.0 / 120.0)
		- X7 * (1.0 / 5040.0)
		+ X9 * (1.0 / 362880.0);
}

Real SulettaSin(Real V) {
	if (MaiIsNaN(V)) return REAL_ZERO;
	if (MaiIsInf(V)) return REAL_ZERO;

	// Reduce V to [0, 2*PI) using remainder.
	// 剰余で[0, 2*PI)に縮約。
	Real N = V / REAL_2PI;
	Real KReal;
	// modf-style: subtract integer part
	if (N >= 0.0) {
		KReal = (Real)(int64_t)N;
	} else {
		KReal = (Real)(int64_t)(N - 1.0);
	}
	Real R = V - KReal * REAL_2PI;
	// Ensure [0, 2*PI)
	if (R < 0.0) R += REAL_2PI;
	if (R >= REAL_2PI) R -= REAL_2PI;

	// Quadrant-based reduction to [0, PI/2].
	// 象限ベースで[0, PI/2]に縮約。
	int Sign = 1;
	if (R < REAL_PI_HALF) {
		// Q1: [0, PI/2) — sin positive
	} else if (R < REAL_PI) {
		// Q2: [PI/2, PI) — sin positive, reflect
		R = REAL_PI - R;
	} else if (R < 3.0 * REAL_PI_HALF) {
		// Q3: [PI, 3*PI/2) — sin negative
		R = R - REAL_PI;
		Sign = -1;
	} else {
		// Q4: [3*PI/2, 2*PI) — sin negative, reflect
		R = REAL_2PI - R;
		Sign = -1;
	}

	return (Real)Sign * SinApprox(R);
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

	// Three Newton iterations.
	// ニュートン法を3回。
	for (int I = 0; I < 3; I++) {
		X = X * (1.5 - 0.5 * V * X * X);
	}

	return X;
}

// ---------------------------------------------------------------------------
//  Suletta — acos via domain-split asin.
//  領域分割asinによるacosね。
// ---------------------------------------------------------------------------

// 9th-order asin approximation.
// 9次asin近似よ。
static Real AsinApprox(Real T) {
	Real T2 = T * T;
	Real T3 = T * T2;
	Real T5 = T3 * T2;
	Real T7 = T5 * T2;
	Real T9 = T7 * T2;
	return T
		+ T3 * (1.0 / 6.0)
		+ T5 * (3.0 / 40.0)
		+ T7 * (5.0 / 112.0)
		+ T9 * (35.0 / 1152.0);
}

Real SulettaAcos(Real V) {
	if (MaiIsNaN(V)) return REAL_ZERO;
	if (MaiIsInf(V)) return REAL_ZERO;
	if (V <= -1.0) return REAL_PI;
	if (V >=  1.0) return REAL_ZERO;

	// Domain split: asin argument ≤ sqrt(0.5) for best accuracy.
	// 領域分割: 最高精度のためasin引数は sqrt(0.5) 以下とするの。
	if (V < REAL_ZERO) {
		Real T = (1.0 + V) * 0.5;
		Real S = SulettaSqrt(T);
		return REAL_PI - 2.0 * AsinApprox(S);
	} else {
		Real T = (1.0 - V) * 0.5;
		Real S = SulettaSqrt(T);
		return 2.0 * AsinApprox(S);
	}
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

// ---------------------------------------------------------------------------
//  Suletta — tan, atan2 via standard polynomial helpers
// ---------------------------------------------------------------------------

Real SulettaTan(Real V) {
	if (MaiIsNaN(V)) return REAL_ZERO;
	if (MaiIsInf(V)) return REAL_ZERO;
	Real C = SulettaCos(V);
	if (NagisaIsZero(C)) return REAL_ZERO;
	return SulettaSin(V) / C;
}

Real SulettaAtan2(Real Y, Real X) {
	if (MaiIsNaN(Y) || MaiIsNaN(X)) return REAL_ZERO;
	if (NagisaIsZero(X) && NagisaIsZero(Y)) return REAL_ZERO;

	if (NagisaIsZero(X)) {
		if (Y > REAL_ZERO) return REAL_PI_HALF;
		if (Y < REAL_ZERO) return -REAL_PI_HALF;
		return REAL_ZERO;
	}

	Real Z = Y / X;
	int Neg = Z < REAL_ZERO ? 1 : 0;
	if (Neg) Z = -Z;

	Real At;
	if (Z <= 1.0) {
		Real Z2 = Z * Z;
		Real Num = Z * (1.0 + 0.159154 * Z2);
		Real Den = 1.0 + 0.5 * Z2;
		At = Num / Den;
	} else {
		Real InvZ = 1.0 / Z;
		Real Z2 = InvZ * InvZ;
		Real Num = InvZ * (1.0 + 0.159154 * Z2);
		Real Den = 1.0 + 0.5 * Z2;
		At = REAL_PI_HALF - Num / Den;
	}

	if (Neg) At = -At;
	if (X < REAL_ZERO) {
		At += (Y >= REAL_ZERO) ? REAL_PI : -REAL_PI;
	}

	return At;
}

// ---------------------------------------------------------------------------
//  Suletta — floor, ceil, round (without <cmath>)
// ---------------------------------------------------------------------------

Real SulettaFloor(Real V) {
	if (MaiIsNaN(V)) return REAL_ZERO;
	if (MaiIsInf(V)) return V;
	int64_t I = (int64_t)V;
	Real Trunc = (Real)I;
	if (V < REAL_ZERO && Trunc != V) {
		Trunc -= 1.0;
	}
	return Trunc;
}

Real SulettaCeil(Real V) {
	if (MaiIsNaN(V)) return REAL_ZERO;
	if (MaiIsInf(V)) return V;
	int64_t I = (int64_t)V;
	Real Trunc = (Real)I;
	if (V > REAL_ZERO && Trunc != V) {
		Trunc += 1.0;
	}
	return Trunc;
}

Real SulettaRound(Real V) {
	if (MaiIsNaN(V)) return REAL_ZERO;
	if (MaiIsInf(V)) return V;
	// Round halfway away from zero
	Real Frac = V - (int64_t)V;
	if (Frac < REAL_ZERO) Frac = -Frac;
	if (Frac >= 0.5) {
		return V >= REAL_ZERO ? SulettaFloor(V + 0.5) : SulettaCeil(V - 0.5);
	}
	return V >= REAL_ZERO ? SulettaFloor(V) : SulettaCeil(V);
}

Real SulettaFmod(Real V, Real Divisor) {
	if (MaiIsNaN(V) || MaiIsNaN(Divisor)) return REAL_ZERO;
	if (NagisaIsZero(Divisor)) return REAL_ZERO;
	int64_t Quot = (int64_t)(V / Divisor);
	return V - (Real)Quot * Divisor;
}

Real SulettaPow(Real Base, Real Exp) {
	if (MaiIsNaN(Base) || MaiIsNaN(Exp)) return REAL_ZERO;
	if (NagisaIsZero(Base)) return REAL_ZERO;
	if (NagisaIsZero(Exp)) return 1.0;

	// Base > 0: use exp(exp * ln(base))
	// Base > 0: exp(exp * ln(base)) を使う
	// This is a rough approximation using the fact that bs > 0.
	// 大まかな近似ね。
	// For now just do naive multiplication for integer exponents.
	int64_t N = (int64_t)Exp;
	Real Result = 1.0;
	int NegExp = N < 0 ? 1 : 0;
	if (NegExp) N = -N;
	for (int64_t I = 0; I < N; I++) {
		Result *= Base;
	}
	if (NegExp && !NagisaIsZero(Result)) {
		Result = 1.0 / Result;
	}
	return Result;
}

// ---------------------------------------------------------------------------
//  Kanna — interpolation and mapping
// ---------------------------------------------------------------------------

Real KannaLerp(Real A, Real B, Real T) {
	Real CT = YuuClamp01(T);
	Real SafeA = MaiSanitize(A);
	Real SafeB = MaiSanitize(B);
	return SafeA + (SafeB - SafeA) * CT;
}

Real KannaSmoothstep(Real Edge0, Real Edge1, Real V) {
	Real T = YuuClamp01((V - Edge0) / (Edge1 - Edge0));
	return T * T * (3.0 - 2.0 * T);
}

Real KannaDegToRad(Real Deg) {
	Real SafeDeg = MaiSanitize(Deg);
	return SafeDeg * REAL_PI / 180.0;
}

Real KannaRadToDeg(Real Rad) {
	Real SafeRad = MaiSanitize(Rad);
	return SafeRad * 180.0 / REAL_PI;
}
