/**
 * Array implementation.
 * 配列の実装ね。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include <TohruPhysics/Array.h>
#include <string.h>

// ---------------------------------------------------------------------------
//  Internal helpers
// ---------------------------------------------------------------------------

static void ArrayGrow(Array *Arr, size_t Need) {
	size_t NewCap = Arr->Capacity == 0 ? 4 : Arr->Capacity;
	while (NewCap < Need) {
		NewCap *= 2;
	}
	TiltyArrayReserve(Arr, NewCap);
}

static void SwapElems(void *A, void *B, size_t Size) {
	// Small-size fast path via uint64_t; fall back to byte loop.
	// NOTE (KleaSCM) memcpy works for trivially-copyable types.
	// 小さなサイズは uint64_t で高速パス、それ以外はバイトループ。
	size_t N = Size;
	while (N >= 8) {
		uint64_t Tmp;
		memcpy(&Tmp, A, 8);
		memcpy(A, B, 8);
		memcpy(B, &Tmp, 8);
		A = (char *)A + 8;
		B = (char *)B + 8;
		N -= 8;
	}
	while (N > 0) {
		char Tmp = *(char *)A;
		*(char *)A = *(char *)B;
		*(char *)B = Tmp;
		A = (char *)A + 1;
		B = (char *)B + 1;
		N--;
	}
}

// ---------------------------------------------------------------------------
//  Quick sort internals: median-of-3 pivot, Lomuto partition.
//  クイックソート内部: 3点中央値ピボット、Lomuto分割。
// ---------------------------------------------------------------------------

static void QuickSortRec(void *Base, size_t Count, size_t ElemSize,
		int (*Cmp)(const void *, const void *))
{
	if (Count <= 1) return;

	// Median-of-3 pivot selection.
	// 3点中央値ピボット選択。
	size_t Lo = 0, Hi = Count - 1, Mid = Count / 2;
	char *Arr = (char *)Base;
	if (Cmp(Arr + Lo * ElemSize, Arr + Mid * ElemSize) > 0) SwapElems(Arr + Lo * ElemSize, Arr + Mid * ElemSize, ElemSize);
	if (Cmp(Arr + Lo * ElemSize, Arr + Hi * ElemSize) > 0) SwapElems(Arr + Lo * ElemSize, Arr + Hi * ElemSize, ElemSize);
	if (Cmp(Arr + Mid * ElemSize, Arr + Hi * ElemSize) > 0) SwapElems(Arr + Mid * ElemSize, Arr + Hi * ElemSize, ElemSize);
	// Move pivot to end.
	SwapElems(Arr + Mid * ElemSize, Arr + Hi * ElemSize, ElemSize);

	char *Pivot = Arr + Hi * ElemSize;
	size_t I = 0;
	for (size_t J = 0; J < Hi; J++) {
		if (Cmp(Arr + J * ElemSize, Pivot) <= 0) {
			SwapElems(Arr + I * ElemSize, Arr + J * ElemSize, ElemSize);
			I++;
		}
	}
	SwapElems(Arr + I * ElemSize, Pivot, ElemSize);

	if (I > 0) QuickSortRec(Base, I, ElemSize, Cmp);
	QuickSortRec(Arr + (I + 1) * ElemSize, Count - I - 1, ElemSize, Cmp);
}

// ===========================================================================
//  Tilty — Array operations
// ===========================================================================

void TiltyArrayInit(Arena *A, Array *Arr, size_t ElemSize, size_t InitCap) {
	Arr->MemArena = A;
	Arr->Length = 0;
	Arr->ElemSize = ElemSize;

	if (InitCap == 0) InitCap = 4;
	size_t Bytes = InitCap * ElemSize;

	Arr->Data = KobayashiAlloc(A, Bytes);
	if (Arr->Data == TohruZeroBlock) {
		Arr->Capacity = 0;
		return;
	}
	Arr->Capacity = InitCap;
}

void TiltyArrayDestroy(Array *Arr) {
	// Arena owns memory — no individual free needed.
	// アリーナがメモリを管理してるから、個別の解放は不要ね。
	Arr->MemArena = NULL;
	Arr->Data = NULL;
	Arr->Length = 0;
	Arr->Capacity = 0;
}

void *TiltyArrayGet(const Array *Arr, size_t Index) {
	if (Index >= Arr->Length) {
		return (void *)TohruZeroBlock;
	}
	return (char *)Arr->Data + Index * Arr->ElemSize;
}

void TiltyArraySet(Array *Arr, size_t Index, const void *Elem) {
	void *Dst = TiltyArrayGet(Arr, Index);
	if (Dst != TohruZeroBlock) {
		memcpy(Dst, Elem, Arr->ElemSize);
	}
}

void *TiltyArrayPush(Array *Arr, const void *Elem) {
	if (Arr->Length >= Arr->Capacity) {
		ArrayGrow(Arr, Arr->Length + 1);
		if (Arr->Data == TohruZeroBlock) {
			return (void *)TohruZeroBlock;
		}
	}

	void *Slot = (char *)Arr->Data + Arr->Length * Arr->ElemSize;
	if (Elem) {
		memcpy(Slot, Elem, Arr->ElemSize);
	}
	Arr->Length++;
	return Slot;
}

void *TiltyArrayPop(Array *Arr) {
	if (Arr->Length == 0) {
		return (void *)TohruZeroBlock;
	}
	Arr->Length--;
	return (char *)Arr->Data + Arr->Length * Arr->ElemSize;
}

void TiltyArrayReserve(Array *Arr, size_t NewCap) {
	if (NewCap <= Arr->Capacity) return;

	size_t Bytes = NewCap * Arr->ElemSize;
	void *NewData = KobayashiAlloc(Arr->MemArena, Bytes);
	if (NewData == TohruZeroBlock) return;

	if (Arr->Data && Arr->Data != TohruZeroBlock && Arr->Length > 0) {
		memcpy(NewData, Arr->Data, Arr->Length * Arr->ElemSize);
	}

	Arr->Data = NewData;
	Arr->Capacity = NewCap;
}

int TiltyArraySearch(const Array *Arr, const void *Key,
		int (*Cmp)(const void *, const void *))
{
	for (size_t I = 0; I < Arr->Length; I++) {
		void *Elem = (char *)Arr->Data + I * Arr->ElemSize;
		if (Cmp(Elem, Key) == 0) {
			return (int)I;
		}
	}
	return -1;
}

int TiltyArrayBinarySearch(const Array *Arr, const void *Key,
		int (*Cmp)(const void *, const void *))
{
	size_t Lo = 0, Hi = Arr->Length;
	while (Lo < Hi) {
		size_t Mid = Lo + (Hi - Lo) / 2;
		void *Elem = (char *)Arr->Data + Mid * Arr->ElemSize;
		int Res = Cmp(Elem, Key);
		if (Res < 0) {
			Lo = Mid + 1;
		} else if (Res > 0) {
			Hi = Mid;
		} else {
			return (int)Mid;
		}
	}
	return -1;
}

void TiltyArrayCopy(Array *Dst, size_t DstStart,
		const Array *Src, size_t SrcStart, size_t Count)
{
	if (Count == 0) return;
	if (DstStart + Count > Dst->Capacity) {
		ArrayGrow(Dst, DstStart + Count);
		if (Dst->Data == TohruZeroBlock) return;
	}

	char *DstPtr = (char *)Dst->Data + DstStart * Dst->ElemSize;
	char *SrcPtr = (char *)Src->Data + SrcStart * Src->ElemSize;
	memcpy(DstPtr, SrcPtr, Count * Dst->ElemSize);

	if (DstStart + Count > Dst->Length) {
		Dst->Length = DstStart + Count;
	}
}

int TiltyArrayContains(const Array *Arr, const void *Key,
		int (*Cmp)(const void *, const void *))
{
	return TiltyArraySearch(Arr, Key, Cmp) >= 0;
}

void TiltyArraySort(Array *Arr,
		int (*Cmp)(const void *, const void *))
{
	if (Arr->Length <= 1) return;
	QuickSortRec(Arr->Data, Arr->Length, Arr->ElemSize, Cmp);
}

// ===========================================================================
//  Grid2D
// ===========================================================================

void TiltyGrid2DInit(Arena *A, Grid2D *G,
		size_t Width, size_t Height, size_t ElemSize)
{
	G->Width = Width;
	G->Height = Height;
	TiltyArrayInit(A, &G->Backing, ElemSize, Width * Height);
	G->Backing.Length = Width * Height;
}

void *TiltyGrid2DGet(const Grid2D *G, size_t X, size_t Y) {
	if (X >= G->Width || Y >= G->Height) {
		return (void *)TohruZeroBlock;
	}
	return TiltyArrayGet(&G->Backing, Y * G->Width + X);
}

void *TiltyGrid2DGetFlat(const Grid2D *G, size_t Index) {
	if (Index >= G->Width * G->Height) {
		return (void *)TohruZeroBlock;
	}
	return TiltyArrayGet(&G->Backing, Index);
}
