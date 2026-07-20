/**
 * Array type — contiguous arena-backed collection.
 * 配列型 — アリーナバックアップの連続コレクションよ。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#pragma once

#include <TohruPhysics/Arena.h>

// ---------------------------------------------------------------------------
//  0053: Array — contiguous arena-backed collection.
//  連続アリーナバックアップコレクション。
// ---------------------------------------------------------------------------
typedef struct {
	Arena  *MemArena;   // backing arena (may be NULL after init)
	void   *Data;       // element storage
	size_t  Length;     // number of elements
	size_t  Capacity;   // allocated element count
	size_t  ElemSize;   // bytes per element
} Array;

// ---------------------------------------------------------------------------
//  Tilty — Array operations
// ---------------------------------------------------------------------------

// Lifecycle
void   TiltyArrayInit(Arena *A, Array *Arr, size_t ElemSize, size_t InitCap);
void   TiltyArrayDestroy(Array *Arr);

// Bounds-checked access
void  *TiltyArrayGet(const Array *Arr, size_t Index);
void   TiltyArraySet(Array *Arr, size_t Index, const void *Elem);

// Push / Pop / Insert / Remove
void  *TiltyArrayPush(Array *Arr, const void *Elem);
void  *TiltyArrayPop(Array *Arr);
void   TiltyArrayInsert(Array *Arr, size_t Index, const void *Elem);
void   TiltyArrayRemove(Array *Arr, size_t Index);
void   TiltyArrayRemoveAt(Array *Arr, size_t Index);

// Capacity
void   TiltyArrayReserve(Array *Arr, size_t NewCap);
void   TiltyArrayResize(Array *Arr, size_t NewLength);
void   TiltyArrayShrinkToFit(Array *Arr);
void   TiltyArrayClear(Array *Arr);

// Front / Back / Data
void  *TiltyArrayFront(const Array *Arr);
void  *TiltyArrayBack(const Array *Arr);
void  *TiltyArrayData(const Array *Arr);
int    TiltyArrayIsEmpty(const Array *Arr);

// Search
int    TiltyArraySearch(const Array *Arr, const void *Key,
		int (*Cmp)(const void *, const void *));
int    TiltyArrayBinarySearch(const Array *Arr, const void *Key,
		int (*Cmp)(const void *, const void *));

// Copy
void   TiltyArrayCopy(Array *Dst, size_t DstStart,
		const Array *Src, size_t SrcStart, size_t Count);

// Membership test
int    TiltyArrayContains(const Array *Arr, const void *Key,
		int (*Cmp)(const void *, const void *));

// Sort (introsort: Quicksort + depth limit -> heapsort)
void   TiltyArraySort(Array *Arr,
		int (*Cmp)(const void *, const void *));

// ForEach
void   TiltyArrayForEach(const Array *Arr,
		void (*Fn)(void *Elem, void *Ctx), void *Ctx);

// ===========================================================================
//  0060: Grid2D — flat multi-dimensional grid.
//  フラット多次元グリッド。
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
