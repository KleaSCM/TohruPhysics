/**
 * Fixed-width scalar math for TohruPhysics.
 * TohruPhysics用の固定幅スカラー数学ね。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#pragma once

#include <stdint.h>

// ---------------------------------------------------------------------------
//  0013: Real precision type.
//  実数精度型ね。
// ---------------------------------------------------------------------------
typedef double Real;

// Convenience constants
// 便利な定数よ。
#define REAL_PI      (3.14159265358979323846)
#define REAL_2PI     (6.28318530717958647692)
#define REAL_PI_HALF (1.57079632679489661923)
#define REAL_EPSILON (2.2204460492503131e-16)
#define REAL_ZERO    (0.0)

// ---------------------------------------------------------------------------
//  Mai — NaN / Inf detection.
//  NaN/Inf検出ね。
//
//  Returns 0 or 1. Never branches.
//  0か1を返すの。分岐しないわ。
// ---------------------------------------------------------------------------

int MaiIsNaN(Real V);
int MaiIsInf(Real V);
int MaiIsFinite(Real V);

// ---------------------------------------------------------------------------
//  Mai — sanitize: returns 0 on NaN/Inf, raw value otherwise.
//  サニタイズ — NaN/Infのときは0を返し、それ以外は生の値を返すの。
// ---------------------------------------------------------------------------
Real MaiSanitize(Real V);

// ---------------------------------------------------------------------------
//  Nagisa — bounded comparison with epsilon.
//  イプシロン付きの比較ね。
// ---------------------------------------------------------------------------

int  NagisaApproxEqual(Real A, Real B, Real Eps);
int  NagisaApproxZero(Real V, Real Eps);
int  NagisaEqual(Real A, Real B);       // uses REAL_EPSILON
int  NagisaIsZero(Real V);              // uses REAL_EPSILON

// ---------------------------------------------------------------------------
//  Yuu — clamp, abs, wrap, bounds.
//  クランプ、絶対値、ラップ、境界ね。
// ---------------------------------------------------------------------------

Real  YuuClamp(Real V, Real Lo, Real Hi);
Real  YuuClamp01(Real V);
Real  YuuAbs(Real V);
Real  YuuMin(Real A, Real B);
Real  YuuMax(Real A, Real B);
Real  YuuSign(Real V);                  // -1, 0, or +1

// integer wraparound (safe, no overflow traps)
// 整数ラップアラウンド（オーバーフロートラップなし）
int64_t YuuClamp64(int64_t V, int64_t Lo, int64_t Hi);
int64_t YuuWrap64(int64_t V, int64_t Lo, int64_t Hi);

// ---------------------------------------------------------------------------
//  Suletta — fast trig and special math.
//  高速三角関数と特殊数学ね。
// ---------------------------------------------------------------------------

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

/**
 * Kanna — interpolation and mapping utilities
 * 補間とマッピングユーティリティ
 */
Real KannaLerp(Real A, Real B, Real T);
Real KannaSmoothstep(Real Edge0, Real Edge1, Real V);
Real KannaDegToRad(Real Deg);
Real KannaRadToDeg(Real Rad);
