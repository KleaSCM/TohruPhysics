/**
 * Shared test assertion tracking for TohruPhysics.
 * TohruPhysicsのテストアサーショントラッキングね。
 *
 * Provides consistent assertion macros across all tests with
 * pass/fail counting and formatted output.
 * 全てのテストで一貫したアサーションマクロを提供するの。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#pragma once

#include <stdio.h>
#include <stdlib.h>

static int TohruTestPassed = 0;
static int TohruTestFailed = 0;

#define TTEST(Cond, Msg) do { \
	if (!(Cond)) { \
		fprintf(stderr, "  FAIL: %s (%s:%d)\n", Msg, __FILE__, __LINE__); \
		TohruTestFailed++; \
	} else { \
		TohruTestPassed++; \
	} \
} while(0)

#define TRUN(TestFn) do { \
	fprintf(stderr, "  %s ... ", #TestFn); \
	TestFn(); \
	fprintf(stderr, "ok\n"); \
} while(0)

#define THEADER(Name) do { \
	TohruTestPassed = 0; \
	TohruTestFailed = 0; \
	fprintf(stderr, "=== %s ===\n", Name); \
} while(0)

#define TFOOTER() do { \
	fprintf(stderr, "\n=== %d passed, %d failed ===\n", \
		TohruTestPassed, TohruTestFailed); \
	if (TohruTestFailed > 0) exit(1); \
} while(0)
