/**
 * Unit tests for Array + Grid2D operations.
 * 配列 + Grid2D 操作の単体テストね。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include <TohruPhysics/Array.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TEST(cond, msg) do { \
	if (!(cond)) { \
		fprintf(stderr, "FAIL: %s (%s:%d)\n", msg, __FILE__, __LINE__); \
		exit(1); \
	} \
} while(0)

static int Passed = 0;
static int Failed = 0;

#define RUN_TEST(name, desc) do { \
	fprintf(stderr, "  %-40s ... ", desc); \
	name(); \
	fprintf(stderr, "ok\n"); \
	Passed++; \
} while(0)

// ===========================================================================
//  Comparators
// ===========================================================================

static int CmpInt(const void *A, const void *B) {
	int IA = *(const int *)A;
	int IB = *(const int *)B;
	return (IA > IB) - (IA < IB);
}

// ===========================================================================
//  Tests
// ===========================================================================

static void TestInit(void) {
	Arena A;
	TohruArenaInit(&A, 4096);

	Array Arr;
	TiltyArrayInit(&A, &Arr, sizeof(int), 16);
	TEST(Arr.Data != TohruZeroBlock, "data allocated");
	TEST(Arr.Capacity == 16, "capacity = 16");
	TEST(Arr.Length == 0, "length = 0");
	TEST(Arr.ElemSize == sizeof(int), "elem size");
	TEST(Arr.MemArena == &A, "arena set");

	TohruArenaDestroy(&A);
}

static void TestInitZeroCap(void) {
	Arena A;
	TohruArenaInit(&A, 4096);

	Array Arr;
	TiltyArrayInit(&A, &Arr, sizeof(int), 0);
	TEST(Arr.Capacity >= 4, "zero cap defaults to 4");

	TohruArenaDestroy(&A);
}

static void TestGetSet(void) {
	Arena A;
	TohruArenaInit(&A, 1024);

	Array Arr;
	TiltyArrayInit(&A, &Arr, sizeof(int), 8);
	int Vals[] = {10, 20, 30};
	for (size_t I = 0; I < 3; I++) {
		TiltyArrayPush(&Arr, &Vals[I]);
	}

	int *P0 = (int *)TiltyArrayGet(&Arr, 0);
	TEST(*P0 == 10, "get 0");
	int *P2 = (int *)TiltyArrayGet(&Arr, 2);
	TEST(*P2 == 30, "get 2");

	void *P99 = TiltyArrayGet(&Arr, 99);
	TEST(P99 == (void *)TohruZeroBlock, "OOB get returns ZeroBlock");

	int Val99 = 99;
	TiltyArraySet(&Arr, 99, &Val99);
	int *P0b = (int *)TiltyArrayGet(&Arr, 0);
	TEST(*P0b == 10, "OOB set doesn't corrupt");

	TohruArenaDestroy(&A);
}

static void TestPush(void) {
	Arena A;
	TohruArenaInit(&A, 64);

	Array Arr;
	TiltyArrayInit(&A, &Arr, sizeof(int), 4);
	int Vals[] = {1, 2, 3, 4, 5, 6, 7, 8};

	// Push within initial capacity.
	for (int I = 0; I < 4; I++) {
		TiltyArrayPush(&Arr, &Vals[I]);
	}
	TEST(Arr.Length == 4, "push 4");
	TEST(Arr.Capacity >= 4, "cap >= 4");

	// Push beyond — triggers growth.
	for (int I = 4; I < 8; I++) {
		TiltyArrayPush(&Arr, &Vals[I]);
	}
	TEST(Arr.Length == 8, "push 8");
	TEST(Arr.Capacity >= 8, "grew past");

	for (int I = 0; I < 8; I++) {
		int *V = (int *)TiltyArrayGet(&Arr, (size_t)I);
		TEST(*V == I + 1, "push values correct");
	}

	TohruArenaDestroy(&A);
}

static void TestPop(void) {
	Arena A;
	TohruArenaInit(&A, 256);

	Array Arr;
	TiltyArrayInit(&A, &Arr, sizeof(int), 8);
	int Vals[] = {10, 20, 30};
	for (size_t I = 0; I < 3; I++) TiltyArrayPush(&Arr, &Vals[I]);

	int *P = (int *)TiltyArrayPop(&Arr);
	TEST(*P == 30, "pop returns last");
	TEST(Arr.Length == 2, "len=2 after pop");

	P = (int *)TiltyArrayPop(&Arr);
	TEST(*P == 20, "pop 20");
	P = (int *)TiltyArrayPop(&Arr);
	TEST(*P == 10, "pop 10");
	TEST(Arr.Length == 0, "empty after 3 pops");

	void *Empty = TiltyArrayPop(&Arr);
	TEST(Empty == TohruZeroBlock, "pop empty = ZeroBlock");

	TohruArenaDestroy(&A);
}

static void TestReserve(void) {
	Arena A;
	TohruArenaInit(&A, 4096);

	Array Arr;
	TiltyArrayInit(&A, &Arr, sizeof(int), 4);
	int V = 42;
	TiltyArrayPush(&Arr, &V);
	TiltyArrayPush(&Arr, &V);
	TEST(Arr.Capacity == 4, "cap=4 before reserve");

	TiltyArrayReserve(&Arr, 64);
	TEST(Arr.Capacity == 64, "cap=64 after reserve");
	TEST(Arr.Length == 2, "len preserved");
	int *P = (int *)TiltyArrayGet(&Arr, 0);
	TEST(*P == 42, "data preserved after reserve");

	TohruArenaDestroy(&A);
}

static void TestSearch(void) {
	Arena A;
	TohruArenaInit(&A, 256);

	Array Arr;
	TiltyArrayInit(&A, &Arr, sizeof(int), 16);
	int Vals[] = {50, 20, 40, 10, 30};
	for (size_t I = 0; I < 5; I++) TiltyArrayPush(&Arr, &Vals[I]);

	int Key30 = 30;
	int Idx = TiltyArraySearch(&Arr, &Key30, CmpInt);
	TEST(Idx == 4, "search 30 -> index 4");

	int Key99 = 99;
	Idx = TiltyArraySearch(&Arr, &Key99, CmpInt);
	TEST(Idx == -1, "search 99 -> -1");

	TohruArenaDestroy(&A);
}

static void TestBinarySearch(void) {
	Arena A;
	TohruArenaInit(&A, 256);

	Array Arr;
	TiltyArrayInit(&A, &Arr, sizeof(int), 16);
	int Vals[] = {10, 20, 30, 40, 50};
	for (size_t I = 0; I < 5; I++) TiltyArrayPush(&Arr, &Vals[I]);

	int Key30 = 30;
	int Idx = TiltyArrayBinarySearch(&Arr, &Key30, CmpInt);
	TEST(Idx == 2, "bsearch 30 -> index 2");

	int Key99 = 99;
	Idx = TiltyArrayBinarySearch(&Arr, &Key99, CmpInt);
	TEST(Idx == -1, "bsearch 99 -> -1");

	TohruArenaDestroy(&A);
}

static void TestCopy(void) {
	Arena A;
	TohruArenaInit(&A, 4096);

	Array Src, Dst;
	TiltyArrayInit(&A, &Src, sizeof(int), 8);
	TiltyArrayInit(&A, &Dst, sizeof(int), 8);

	int Vals[] = {1, 2, 3, 4, 5};
	for (size_t I = 0; I < 5; I++) TiltyArrayPush(&Src, &Vals[I]);

	// Indexed copy: src[1..3] -> dst[0..2].
	TiltyArrayCopy(&Dst, 0, &Src, 1, 3);
	TEST(Dst.Length >= 3, "copy len");
	int *P0 = (int *)TiltyArrayGet(&Dst, 0);
	TEST(*P0 == 2, "copy[0] = src[1]");
	int *P2 = (int *)TiltyArrayGet(&Dst, 2);
	TEST(*P2 == 4, "copy[2] = src[3]");

	TohruArenaDestroy(&A);
}

static void TestContains(void) {
	Arena A;
	TohruArenaInit(&A, 256);

	Array Arr;
	TiltyArrayInit(&A, &Arr, sizeof(int), 8);
	int Vals[] = {10, 20, 30};
	for (size_t I = 0; I < 3; I++) TiltyArrayPush(&Arr, &Vals[I]);

	int Key20 = 20;
	TEST(TiltyArrayContains(&Arr, &Key20, CmpInt), "contains 20");

	int Key99 = 99;
	TEST(!TiltyArrayContains(&Arr, &Key99, CmpInt), "not contains 99");

	TohruArenaDestroy(&A);
}

static void TestSort(void) {
	Arena A;
	TohruArenaInit(&A, 256);

	Array Arr;
	TiltyArrayInit(&A, &Arr, sizeof(int), 16);
	int Vals[] = {50, 20, 40, 10, 30, 60};
	for (size_t I = 0; I < 6; I++) TiltyArrayPush(&Arr, &Vals[I]);

	TiltyArraySort(&Arr, CmpInt);
	int *P;
	for (int I = 0; I < 6; I++) {
		P = (int *)TiltyArrayGet(&Arr, (size_t)I);
		TEST(*P == (I + 1) * 10, "sorted order");
	}

	// Empty sort doesn't crash.
	Array Empty;
	TiltyArrayInit(&A, &Empty, sizeof(int), 4);
	TiltyArraySort(&Empty, CmpInt);
	TEST(Empty.Length == 0, "sort empty");

	// Single element sort doesn't crash.
	int V = 42;
	TiltyArrayPush(&Empty, &V);
	TiltyArraySort(&Empty, CmpInt);
	P = (int *)TiltyArrayGet(&Empty, 0);
	TEST(*P == 42, "sort single");

	TohruArenaDestroy(&A);
}

static void TestGrid2D(void) {
	Arena A;
	TohruArenaInit(&A, 4096);

	Grid2D G;
	TiltyGrid2DInit(&A, &G, 4, 3, sizeof(int));
	TEST(G.Width == 4, "grid width");
	TEST(G.Height == 3, "grid height");
	TEST(G.Backing.Length == 12, "grid total");

	// Fill as (x + y * width).
	for (size_t Y = 0; Y < 3; Y++) {
		for (size_t X = 0; X < 4; X++) {
			int V = (int)(X + Y * 4);
			memcpy(TiltyGrid2DGet(&G, X, Y), &V, sizeof(int));
		}
	}

	int *P0 = (int *)TiltyGrid2DGet(&G, 0, 0);
	TEST(*P0 == 0, "grid (0,0)");
	P0 = (int *)TiltyGrid2DGet(&G, 3, 0);
	TEST(*P0 == 3, "grid (3,0)");
	P0 = (int *)TiltyGrid2DGet(&G, 0, 1);
	TEST(*P0 == 4, "grid (0,1)");
	P0 = (int *)TiltyGrid2DGet(&G, 0, 2);
	TEST(*P0 == 8, "grid (0,2)");

	void *OOB = TiltyGrid2DGet(&G, 4, 0);
	TEST(OOB == (void *)TohruZeroBlock, "grid OOB X");
	OOB = TiltyGrid2DGet(&G, 0, 3);
	TEST(OOB == (void *)TohruZeroBlock, "grid OOB Y");

	TohruArenaDestroy(&A);
}

static void TestInsertRemove(void) {
	Arena A;
	TohruArenaInit(&A, 4096);
	Array Arr;
	TiltyArrayInit(&A, &Arr, sizeof(int), 4);

	int V;
	V = 10; TiltyArrayPush(&Arr, &V);
	V = 20; TiltyArrayPush(&Arr, &V);
	V = 30; TiltyArrayPush(&Arr, &V);

	V = 15; TiltyArrayInsert(&Arr, 1, &V);
	int *G = (int *)TiltyArrayGet(&Arr, 1);
	TEST(*G == 15, "insert at 1");
	TEST(Arr.Length == 4, "insert length");

	TiltyArrayRemove(&Arr, 1);
	G = (int *)TiltyArrayGet(&Arr, 1);
	TEST(*G == 20, "after remove idx 1");
	TEST(Arr.Length == 3, "remove length");
	TohruArenaDestroy(&A);
}

static void TestResizeShrink(void) {
	Arena A;
	TohruArenaInit(&A, 4096);
	Array Arr;
	TiltyArrayInit(&A, &Arr, sizeof(int), 4);

	int V;
	V = 1; TiltyArrayPush(&Arr, &V);
	V = 2; TiltyArrayPush(&Arr, &V);
	V = 3; TiltyArrayPush(&Arr, &V);

	TiltyArrayResize(&Arr, 10);
	TEST(Arr.Length == 10, "resize length=10");

	TiltyArrayResize(&Arr, 2);
	TEST(Arr.Length == 2, "resize length=2");
	TEST(TiltyArrayIsEmpty(&Arr) == 0, "not empty");
	TiltyArrayClear(&Arr);
	TEST(TiltyArrayIsEmpty(&Arr) == 1, "empty after clear");

	TohruArenaDestroy(&A);
}

static void TestFrontBackData(void) {
	Arena A;
	TohruArenaInit(&A, 4096);
	Array Arr;
	TiltyArrayInit(&A, &Arr, sizeof(int), 4);

	int V;
	V = 10; TiltyArrayPush(&Arr, &V);
	V = 20; TiltyArrayPush(&Arr, &V);
	V = 30; TiltyArrayPush(&Arr, &V);

	int *F = (int *)TiltyArrayFront(&Arr);
	TEST(*F == 10, "front = 10");
	int *B = (int *)TiltyArrayBack(&Arr);
	TEST(*B == 30, "back = 30");
	void *D = TiltyArrayData(&Arr);
	TEST(D == Arr.Data, "data ptr");

	TohruArenaDestroy(&A);
}

int main(void) {
	fprintf(stderr, "=== TestArray ===\n");

	RUN_TEST(TestInit, "Init basic");
	RUN_TEST(TestInitZeroCap, "Init zero capacity");
	RUN_TEST(TestGetSet, "Get / Set / OOB ZII");
	RUN_TEST(TestPush, "Push with growth");
	RUN_TEST(TestPop, "Pop / empty pop");
	RUN_TEST(TestReserve, "Reserve capacity");
	RUN_TEST(TestSearch, "Linear search");
	RUN_TEST(TestBinarySearch, "Binary search");
	RUN_TEST(TestCopy, "Indexed copy");
	RUN_TEST(TestContains, "Contains");
	RUN_TEST(TestSort, "Sort");
	RUN_TEST(TestGrid2D, "Grid2D");
	RUN_TEST(TestInsertRemove, "Insert / Remove");
	RUN_TEST(TestResizeShrink, "Resize / ShrinkToFit / Clear / IsEmpty");
	RUN_TEST(TestFrontBackData, "Front / Back / Data");

	fprintf(stderr, "\n=== %d passed, %d failed ===\n", Passed, Failed);
	return Failed > 0 ? 1 : 0;
}
