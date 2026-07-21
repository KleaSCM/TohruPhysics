/**
 * StringTable — Internationalization string table interface.
 * TohruPhysics GUI用の国際化文字列テーブルね。
 *
 * Provides compile-time string tables for English and Japanese locales.
 * Zero-allocation string lookup adhering to ZII principles.
 *
 * DESIGN PHILOSOPHY:
 * Runtime locale changes do not allocate heap memory. All strings are
 * static string literals stored in flash/executable sections. Missing
 * strings default to English fallback or empty string stub.
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#pragma once

typedef enum {
	Language_English  = 0,
	Language_Japanese = 1,
	Language_Count
} Language;

typedef enum {
	StrKey_AppName,
	StrKey_Play,
	StrKey_Pause,
	StrKey_Step,
	StrKey_Reset,
	StrKey_TimeScale,
	StrKey_Gravity,
	StrKey_Damping,
	StrKey_Restitution,
	StrKey_Friction,
	StrKey_SolverIter,
	StrKey_SubSteps,
	StrKey_BodyCount,
	StrKey_ActiveBodies,
	StrKey_SleepingBodies,
	StrKey_Fps,
	StrKey_SimTime,
	StrKey_Memory,
	StrKey_DemoTitle,
	StrKey_Count
} StrKey;

const char *StringTableGet(Language Lang, StrKey Key);
