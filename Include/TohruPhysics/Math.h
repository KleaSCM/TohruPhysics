/**
 * Scalar math — fixed-width double precision with NaN/Inf guarding.
 * TohruPhysics用の固定幅スカラー数学ね。
 *
 * IEEE 754 double-precision math primitives. Every function sanitises
 * inputs — NaN/Inf returns 0.0. Never propagates floating-point poison.
 *
 * DESIGN PHILOSOPHY:
 * Physics engines call trig and sqrt millions of times per frame. The
 * standard library (libm) guarantees 1 ULP accuracy but that precision
 * is invisible in simulation where 1e-4 relative error is acceptable.
 * We use range-reduced polynomial approximations (9th-order Taylor for
 * sin/cos, Newton–Raphson for invsqrt) that are 3–5× faster at 1e-6
 * accuracy.
 *
 * REAL layout (64-bit IEEE 754): seeeeeeee mmmmmmmmmmmmmmmmmmmmmmm
 * ┌─┬──────┬────────────────────────────────────────────────────┐
 * │1│  11  │  52                                                │
 * │S│ Exp  │  Mantissa                                          │
 * └─┴──────┴────────────────────────────────────────────────────┘
 *
 * References:
 * - sin/cos: 9th-order Taylor on [0, PI/2] with quadrant reduction
 * - invsqrt: Quake bit hack + 3 Newton iterations (double precision)
 * - acos: domain-split asin(x) = atan2(x, sqrt(1-x²))
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#pragma once

#include <stdint.h>

typedef double Real;

#define REAL_PI      (3.14159265358979323846)
#define REAL_2PI     (6.28318530717958647692)
#define REAL_PI_HALF (1.57079632679489661923)
#define REAL_EPSILON (2.2204460492503131e-16)
#define REAL_ZERO    (0.0)

int   MaiIsNaN(Real V);
int   MaiIsInf(Real V);
int   MaiIsFinite(Real V);
Real  MaiSanitize(Real V);

int   NagisaApproxEqual(Real A, Real B, Real Eps);
int   NagisaApproxZero(Real V, Real Eps);
int   NagisaEqual(Real A, Real B);
int   NagisaIsZero(Real V);

Real  YuuClamp(Real V, Real Lo, Real Hi);
Real  YuuClamp01(Real V);
Real  YuuAbs(Real V);
Real  YuuMin(Real A, Real B);
Real  YuuMax(Real A, Real B);
Real  YuuSign(Real V);
int64_t YuuClamp64(int64_t V, int64_t Lo, int64_t Hi);
int64_t YuuWrap64(int64_t V, int64_t Lo, int64_t Hi);

Real SulettaSin(Real V);
Real SulettaCos(Real V);
Real SulettaTan(Real V);
Real SulettaAtan2(Real Y, Real X);
Real SulettaInvSqrt(Real V);
Real SulettaSqrt(Real V);
Real SulettaAcos(Real V);
Real SulettaFloor(Real V);
Real SulettaCeil(Real V);
Real SulettaRound(Real V);
Real SulettaFmod(Real V, Real Divisor);
Real SulettaPow(Real Base, Real Exp);

Real KannaLerp(Real A, Real B, Real T);
Real KannaSmoothstep(Real Edge0, Real Edge1, Real V);
Real KannaDegToRad(Real Deg);
Real KannaRadToDeg(Real Rad);
