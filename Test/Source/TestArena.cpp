/**
 * Arena unit tests.
 * アリーナの単体テストね。
 *
 * KobayashiAlloc / KobayashiAllocAlign / KobayashiDup NEVER return null.
 * They return &TohruZeroBlock on OOM or invalid input. Callers never
 * null-check. Every pointer is always usable.
 * KobayashiAlloc / KobayashiAllocAlign / KobayashiDup は絶対にNULLを返さないの。
 * OOMや不正入力のときは &TohruZeroBlock を返すわ。呼び出し側はnullチェックしないでね。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include <TohruPhysics/Arena.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ---------------------------------------------------------------------------
//  Test harness
// ---------------------------------------------------------------------------
static int TestCount = 0;
static int PassCount = 0;
static int FailCount = 0;

#define TEST(Name)                                         \
	do {                                                     \
		TestCount++;                                           \
		fprintf(stderr, "  TEST %3d: %-50s ", TestCount, Name); \
	} while (0)

#define PASS()                                             \
	do {                                                     \
		PassCount++;                                           \
		fprintf(stderr, "PASS\n");                               \
	} while (0)

#define FAIL(Msg)                                          \
	do {                                                     \
		FailCount++;                                           \
		fprintf(stderr, "FAIL — %s\n", Msg);                     \
	} while (0)

#define CHECK(Cond, Msg)                                   \
	do {                                                     \
		if (!(Cond)) { FAIL(Msg); return; }                    \
	} while (0)

// ---------------------------------------------------------------------------
//  Tests
// ---------------------------------------------------------------------------

static void TestTohruInitAndDestroy(void) {
	TEST("TohruArenaInit default capacity");

	Arena A;
	Error Err = TohruArenaInit(&A, 0);
	CHECK(ErrIsOk(Err), "init failed");
	CHECK(A.Base != NULL, "base is null");
	CHECK(A.Base != TohruZeroBlock, "base is zeroblock on success");
	CHECK(A.Capacity >= ARENA_DEFAULT_CAPACITY, "capacity too small");
	CHECK(A.Offset == 0, "offset not zero");

	TohruArenaDestroy(&A);
	CHECK(A.Base == NULL, "base not null after destroy");
	PASS();
}

static void TestTohruInitFixed(void) {
	TEST("TohruArenaInitFixed");

	char Buf[256];
	Arena A;
	Error Err = TohruArenaInitFixed(&A, Buf, sizeof(Buf));
	CHECK(ErrIsOk(Err), "fixed init failed");
	CHECK(A.Base == Buf, "base mismatch");
	CHECK(A.Capacity == 256, "capacity mismatch");

	void *P = KobayashiAlloc(&A, 128);
	CHECK(P == Buf, "alloc not at base");
	CHECK(P != TohruZeroBlock, "got zeroblock when arena had space");

	TohruArenaDestroy(&A);
	PASS();
}

static void TestKobayashiAllocBasic(void) {
	TEST("KobayashiAlloc basic");

	Arena A;
	TohruArenaInit(&A, 256);

	void *P1 = KobayashiAlloc(&A, 16);
	CHECK(P1 != TohruZeroBlock, "first alloc returned zeroblock");
	CHECK(A.Offset == 16, "offset not 16");

	void *P2 = KobayashiAlloc(&A, 32);
	CHECK(P2 != TohruZeroBlock, "second alloc returned zeroblock");
	CHECK(A.Offset == 48, "offset not 48");

	char *B1 = (char *)P1;
	char *B2 = (char *)P2;
	CHECK((B2 >= B1 + 16) || (B1 >= B2 + 32), "allocations overlap");

	TohruArenaDestroy(&A);
	PASS();
}

static void TestKobayashiAllocAlignment(void) {
	TEST("KobayashiAllocAlign");

	Arena A;
	TohruArenaInit(&A, 1024);

	void *P1 = KobayashiAllocAlign(&A, 8, 64);
	CHECK(P1 != TohruZeroBlock, "aligned alloc returned zeroblock");
	CHECK(((uintptr_t)P1 & 63) == 0, "not 64-byte aligned");

	void *P2 = KobayashiAllocAlign(&A, 8, 128);
	CHECK(P2 != TohruZeroBlock, "second aligned alloc returned zeroblock");
	CHECK(((uintptr_t)P2 & 127) == 0, "not 128-byte aligned");

	CHECK(
		((char *)P2 >= (char *)P1 + 8) ||
		((char *)P1 >= (char *)P2 + 8),
		"aligned allocations overlap"
	);

	TohruArenaDestroy(&A);
	PASS();
}

static void TestKobayashiDup(void) {
	TEST("KobayashiDup");

	Arena A;
	TohruArenaInit(&A, 256);

	const char *Src = "Hello Tohru!";
	size_t Len = strlen(Src) + 1;

	char *Dst = (char *)KobayashiDup(&A, Src, Len);
	CHECK(Dst != TohruZeroBlock, "dup returned zeroblock");
	CHECK(memcmp(Dst, Src, Len) == 0, "content mismatch");
	CHECK(Dst != Src, "not a copy");

	TohruArenaDestroy(&A);
	PASS();
}

static void TestElmaReset(void) {
	TEST("ElmaArenaReset");

	Arena A;
	TohruArenaInit(&A, 256);

	KobayashiAlloc(&A, 64);
	CHECK(A.Offset == 64, "offset not 64 after alloc");
	CHECK(ElmaArenaUsed(&A) == 64, "used() not 64");

	ElmaArenaReset(&A);
	CHECK(A.Offset == 0, "offset not 0 after reset");
	CHECK(ElmaArenaRemaining(&A) == A.Capacity, "remaining not full");

	void *P = KobayashiAlloc(&A, 128);
	CHECK(P != TohruZeroBlock, "alloc after reset failed");
	CHECK(P == A.Base, "alloc after reset not at base");

	TohruArenaDestroy(&A);
	PASS();
}

static void TestElmaClear(void) {
	TEST("ElmaArenaClear");

	Arena A;
	TohruArenaInit(&A, 256);

	char *P = (char *)KobayashiAlloc(&A, 32);
	CHECK(P != TohruZeroBlock, "alloc before clear failed");
	memset(P, 0xAB, 32);

	ElmaArenaClear(&A);
	CHECK(A.Offset == 0, "offset not 0 after clear");

	for (size_t I = 0; I < 32; I++) {
		CHECK(((char *)A.Base)[I] == 0, "memory not zeroed after clear");
	}

	TohruArenaDestroy(&A);
	PASS();
}

static void TestArenaExpansion(void) {
	TEST("Arena automatic expansion");

	Arena A;
	TohruArenaInit(&A, 64);

	size_t InitialCap = A.Capacity;

	size_t Off1 = IluluOffset(&A, KobayashiAlloc(&A, 48));
	CHECK(A.Capacity == InitialCap, "capacity changed before expansion");

	void *P2 = KobayashiAlloc(&A, 48);
	CHECK(P2 != TohruZeroBlock, "second alloc returned zeroblock");
	CHECK(A.Capacity > InitialCap, "capacity did not grow");

	void *P1 = IluluPtr(&A, Off1);
	CHECK(P1 != TohruZeroBlock, "P1 reconstruction null after move");
	CHECK(IluluOwns(&A, P1), "P1 not owned after move");

	memset(P1, 0xAA, 48);
	memset(P2, 0xBB, 48);

	CHECK(((unsigned char *)P1)[0] == 0xAA, "P1 content mismatch");
	CHECK(((unsigned char *)P2)[0] == 0xBB, "P2 content mismatch");

	TohruArenaDestroy(&A);
	PASS();
}

static void TestExpansionStress(void) {
	TEST("Arena stress: big block forces expansions");

	Arena A;
	TohruArenaInit(&A, 128);

	int    Count = 1024;
	size_t Total = (size_t)Count * 7;
	void  *Block = KobayashiAlloc(&A, Total);
	CHECK(Block != TohruZeroBlock, "big block returned zeroblock");
	CHECK(A.Capacity > 128, "arena did not expand");

	for (int I = 0; I < Count; I++) {
		memset((char *)Block + (size_t)I * 7, (char)(I & 0xFF), 7);
	}

	for (int I = 0; I < Count; I++) {
		unsigned char *Buf = (unsigned char *)Block + (size_t)I * 7;
		for (int J = 0; J < 7; J++) {
			CHECK(Buf[J] == (unsigned char)(I & 0xFF),
				"data corruption in stress test");
		}
	}

	TohruArenaDestroy(&A);
	PASS();
}

static void TestIluluConversion(void) {
	TEST("Ilulu offset/ptr/owns");

	Arena A;
	TohruArenaInit(&A, 1024);

	void *P1 = KobayashiAlloc(&A, 64);
	void *P2 = KobayashiAlloc(&A, 32);

	size_t Off1 = IluluOffset(&A, P1);
	size_t Off2 = IluluOffset(&A, P2);

	CHECK(Off1 == 0, "first alloc offset not 0");
	CHECK(Off2 == 64, "second alloc offset not 64");

	void *Recon1 = IluluPtr(&A, Off1);
	void *Recon2 = IluluPtr(&A, Off2);
	CHECK(Recon1 == P1, "ptr reconstruction mismatch for P1");
	CHECK(Recon2 == P2, "ptr reconstruction mismatch for P2");

	CHECK(IluluOwns(&A, P1) != 0, "owns(P1) false");
	CHECK(IluluOwns(&A, P2) != 0, "owns(P2) false");

	char Outside = 0;
	CHECK(IluluOwns(&A, &Outside) == 0, "owns(outside) true");

	void *PastEnd = IluluPtr(&A, A.Capacity + 1);
	CHECK(PastEnd == TohruZeroBlock, "ptr past end not zeroblock");

	TohruArenaDestroy(&A);
	PASS();
}

static void TestYuyuArenaSet(void) {
	TEST("YuyuArenaSet");

	ArenaSet S;
	Error Err = YuyuArenaSetInit(&S, 256, 1024, 256);
	CHECK(ErrIsOk(Err), "arena set init failed");

	KobayashiAlloc(&S.Arenas[ArenaTypeFrame], 64);
	CHECK(
		ElmaArenaUsed(&S.Arenas[ArenaTypeFrame]) == 64,
		"frame used not 64"
	);
	YuyuArenaSetResetFrame(&S);
	CHECK(
		ElmaArenaUsed(&S.Arenas[ArenaTypeFrame]) == 0,
		"frame used not 0 after reset"
	);

	KobayashiAlloc(&S.Arenas[ArenaTypeWorld], 128);
	CHECK(
		ElmaArenaUsed(&S.Arenas[ArenaTypeWorld]) == 128,
		"world used not 128 after frame reset"
	);

	YuyuArenaSetDestroy(&S);

	for (int I = 0; I < (int)ArenaTypeCount; I++) {
		CHECK(S.Arenas[I].Base == NULL, "arena not destroyed");
	}

	PASS();
}

static void TestZeroBlockInvariant(void) {
	TEST("ZeroBlock invariant — zeroed memory is valid");

	Arena A;
	TohruArenaInit(&A, 256);

	char *P = (char *)KobayashiAlloc(&A, 64);
	memset(P, 0xFF, 64);

	ElmaArenaClear(&A);

	char *Q = (char *)KobayashiAlloc(&A, 64);
	for (size_t I = 0; I < 64; I++) {
		CHECK(Q[I] == 0, "non-zero after clear re-alloc");
	}

	TohruArenaDestroy(&A);
	PASS();
}

static void TestZeroBlockFallback(void) {
	TEST("ZeroBlock fallback — alloc always returns valid ptr");

	// Alloc with null arena returns ZeroBlock (not crash)
	// nullアリーナでの確保はZeroBlockを返すわ（クラッシュしない）
	void *P = KobayashiAlloc(NULL, 64);
	CHECK(P == TohruZeroBlock, "alloc with null arena != zeroblock");

	// Alloc with size 0
	void *Q = KobayashiAllocAlign(NULL, 0, 16);
	CHECK(Q == TohruZeroBlock, "allocAlign null/size0 != zeroblock");

	// Dup with null src
	void *R = KobayashiDup(NULL, NULL, 0);
	CHECK(R == TohruZeroBlock, "dup null != zeroblock");

	// Verify ZeroBlock is actually zeroed
	// ZeroBlockが実際にゼロ埋めされてるか確認するの。
	unsigned char *ZB = (unsigned char *)TohruZeroBlock;
	for (size_t I = 0; I < 64; I++) {
		CHECK(ZB[I] == 0, "TohruZeroBlock not zeroed");
	}

	PASS();
}

// ---------------------------------------------------------------------------
//  Main
// ---------------------------------------------------------------------------

int main(void) {
	fprintf(stderr, "TohruPhysics Arena Tests\n");
	fprintf(stderr, "========================\n\n");

	TestTohruInitAndDestroy();
	TestTohruInitFixed();
	TestKobayashiAllocBasic();
	TestKobayashiAllocAlignment();
	TestKobayashiDup();
	TestElmaReset();
	TestElmaClear();
	TestArenaExpansion();
	TestExpansionStress();
	TestIluluConversion();
	TestYuyuArenaSet();
	TestZeroBlockInvariant();
	TestZeroBlockFallback();

	fprintf(stderr, "\nResults: %d/%d passed, %d failed\n",
		PassCount, TestCount, FailCount);

	return FailCount > 0 ? 1 : 0;
}
