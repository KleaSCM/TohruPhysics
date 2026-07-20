/**
 * Array type — contiguous arena-backed collection.
 * 配列型 — アリーナバックアップの連続コレクションよ。
 *
 * ZII: every access returns valid memory. OOB Get → &TohruZeroBlock.
 * ZII: 全てのアクセスは有効なメモリを返すわ。範囲外 Get → &TohruZeroBlock。
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

//  0053 + 0054: Init with pre-allocation capacity.
void   TiltyArrayInit(Arena *A, Array *Arr, size_t ElemSize, size_t InitCap);
void   TiltyArrayDestroy(Array *Arr);

//  0055: Bounds-checked access.
void  *TiltyArrayGet(const Array *Arr, size_t Index);
void   TiltyArraySet(Array *Arr, size_t Index, const void *Elem);
void  *TiltyArrayPush(Array *Arr, const void *Elem);
void  *TiltyArrayPop(Array *Arr);

//  0061: Reserve capacity.
void   TiltyArrayReserve(Array *Arr, size_t NewCap);

//  0056: Search.
int    TiltyArraySearch(const Array *Arr, const void *Key,
		int (*Cmp)(const void *, const void *));
int    TiltyArrayBinarySearch(const Array *Arr, const void *Key,
		int (*Cmp)(const void *, const void *));

//  0057: Localized copy.
void   TiltyArrayCopy(Array *Dst, size_t DstStart,
		const Array *Src, size_t SrcStart, size_t Count);

//  0058: Unique membership test.
int    TiltyArrayContains(const Array *Arr, const void *Key,
		int (*Cmp)(const void *, const void *));

//  0059: In-place sort (quick sort with median-of-3 pivot).
void   TiltyArraySort(Array *Arr,
		int (*Cmp)(const void *, const void *));

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
