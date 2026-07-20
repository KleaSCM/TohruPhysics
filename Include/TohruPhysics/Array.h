/**
 * Array — arena-backed contiguous dynamic array.
 * TohruPhysics用のアリーナバックアップ動的配列よ。
 *
 * Generic array stored in an arena. Grows by doubling capacity when
 * full. Bounds-checked access returns &TohruZeroBlock on OOB / empty.
 * Never branches — callers always get a valid pointer.
 *
 * DESIGN PHILOSOPHY:
 * Each frame produces transient arrays (contacts, constraints, pairs).
 * These must be fast to fill (amortised O(1) push), fast to clear (O(1)
 * length reset), and never allocate from the heap. The arena backing
 * satisfies all three: push only bumps the length (or triggers a
 * realloc within the same arena), clear resets length to 0, and the
 * backing memory is owned by the arena — no malloc/free per frame.
 *
 * DATA LAYOUT:
 * ┌──────┬──────┬──────┬──────┬──────────────────────────────────┐
 * │ Elem │ Elem │ Elem │ Elem │ ... (Capacity slots total)       │
 * │  0   │  1   │  2   │  3   │                                  │
 * └──────┴──────┴──────┴──────┴──────────────────────────────────┘
 *  Length→│←────────── Capacity ────────────────────────────────→│
 *
 * OOB Get returns TohruZeroBlock pointer (no crash, no branch).
 *
 * References:
 * - Standard resizable array pattern (std::vector semantics)
 * - Arena allocation for frame-temporary data
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#pragma once

#include <TohruPhysics/Arena.h>

typedef struct {
	Arena  *MemArena;
	void   *Data;
	size_t  Length;
	size_t  Capacity;
	size_t  ElemSize;
} Array;

void   TiltyArrayInit(Arena *A, Array *Arr, size_t ElemSize, size_t InitCap);
void   TiltyArrayDestroy(Array *Arr);

void  *TiltyArrayGet(const Array *Arr, size_t Index);
void   TiltyArraySet(Array *Arr, size_t Index, const void *Elem);

void  *TiltyArrayPush(Array *Arr, const void *Elem);
void  *TiltyArrayPop(Array *Arr);
void   TiltyArrayInsert(Array *Arr, size_t Index, const void *Elem);
void   TiltyArrayRemove(Array *Arr, size_t Index);
void   TiltyArrayRemoveAt(Array *Arr, size_t Index);

void   TiltyArrayReserve(Array *Arr, size_t NewCap);
void   TiltyArrayResize(Array *Arr, size_t NewLength);
void   TiltyArrayShrinkToFit(Array *Arr);
void   TiltyArrayClear(Array *Arr);

void  *TiltyArrayFront(const Array *Arr);
void  *TiltyArrayBack(const Array *Arr);
void  *TiltyArrayData(const Array *Arr);
int    TiltyArrayIsEmpty(const Array *Arr);

int    TiltyArraySearch(const Array *Arr, const void *Key,
		int (*Cmp)(const void *, const void *));
int    TiltyArrayBinarySearch(const Array *Arr, const void *Key,
		int (*Cmp)(const void *, const void *));

void   TiltyArrayCopy(Array *Dst, size_t DstStart,
		const Array *Src, size_t SrcStart, size_t Count);

int    TiltyArrayContains(const Array *Arr, const void *Key,
		int (*Cmp)(const void *, const void *));

void   TiltyArraySort(Array *Arr,
		int (*Cmp)(const void *, const void *));

void   TiltyArrayForEach(const Array *Arr,
		void (*Fn)(void *Elem, void *Ctx), void *Ctx);

// ===========================================================================
//  Grid2D — flat multi-dimensional grid
//  フラット多次元グリッド
// ===========================================================================
typedef struct {
	Array   Backing;
	size_t  Width;
	size_t  Height;
} Grid2D;

void   TiltyGrid2DInit(Arena *A, Grid2D *G,
		size_t Width, size_t Height, size_t ElemSize);
void  *TiltyGrid2DGet(const Grid2D *G, size_t X, size_t Y);
void  *TiltyGrid2DGetFlat(const Grid2D *G, size_t Index);
void   TiltyGrid2DSet(const Grid2D *G, size_t X, size_t Y, const void *Elem);
void   TiltyGrid2DClear(Grid2D *G);
