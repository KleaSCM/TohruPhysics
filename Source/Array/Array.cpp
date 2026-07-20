/**
 * Array implementation.
 * 配列の実装ね。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include <TohruPhysics/Array.h>
#include <string.h>
#include <stdint.h>

// ---------------------------------------------------------------------------
//  Internal helpers
// ---------------------------------------------------------------------------

static size_t MaxCapacity(size_t ElemSize) {
	return SIZE_MAX / ElemSize;
}

static void ArrayGrow(Array *Arr, size_t Need) {
	size_t NewCap = Arr->Capacity == 0 ? 4 : Arr->Capacity;
	size_t MaxCap = MaxCapacity(Arr->ElemSize);
	while (NewCap < Need) {
		if (NewCap > MaxCap / 2) {
			NewCap = MaxCap;
			break;
		}
		NewCap *= 2;
	}
	TiltyArrayReserve(Arr, NewCap);
}

static void SwapElems(void *A, void *B, size_t Size) {
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
//  Introsort
// ---------------------------------------------------------------------------

static void SiftDown(char *Arr, size_t Count, size_t Root, size_t ElemSize,
		int (*Cmp)(const void *, const void *))
{
	while (1) {
		size_t Largest = Root;
		size_t L = 2 * Root + 1;
		size_t R = L + 1;
		if (L < Count && Cmp(Arr + L * ElemSize, Arr + Largest * ElemSize) > 0)
			Largest = L;
		if (R < Count && Cmp(Arr + R * ElemSize, Arr + Largest * ElemSize) > 0)
			Largest = R;
		if (Largest == Root) break;
		SwapElems(Arr + Root * ElemSize, Arr + Largest * ElemSize, ElemSize);
		Root = Largest;
	}
}

static void HeapSort(char *Arr, size_t Count, size_t ElemSize,
		int (*Cmp)(const void *, const void *))
{
	if (Count < 2) return;
	size_t I = Count / 2;
	while (I > 0) { I--; SiftDown(Arr, Count, I, ElemSize, Cmp); }
	for (size_t End = Count; End > 1; End--) {
		SwapElems(Arr, Arr + (End - 1) * ElemSize, ElemSize);
		SiftDown(Arr, End - 1, 0, ElemSize, Cmp);
	}
}

static size_t HoarePartition(char *Arr, size_t Lo, size_t Hi, size_t ElemSize,
		int (*Cmp)(const void *, const void *))
{
	size_t Mid = Lo + (Hi - Lo) / 2;
	if (Cmp(Arr + Lo * ElemSize, Arr + Mid * ElemSize) > 0)
		SwapElems(Arr + Lo * ElemSize, Arr + Mid * ElemSize, ElemSize);
	if (Cmp(Arr + Lo * ElemSize, Arr + Hi * ElemSize) > 0)
		SwapElems(Arr + Lo * ElemSize, Arr + Hi * ElemSize, ElemSize);
	if (Cmp(Arr + Mid * ElemSize, Arr + Hi * ElemSize) > 0)
		SwapElems(Arr + Mid * ElemSize, Arr + Hi * ElemSize, ElemSize);
	SwapElems(Arr + Lo * ElemSize, Arr + Mid * ElemSize, ElemSize);

	char *Pivot = Arr + Lo * ElemSize;
	size_t I = Lo - 1;
	size_t J = Hi + 1;
	while (1) {
		do { I++; } while (Cmp(Arr + I * ElemSize, Pivot) < 0);
		do { J--; } while (Cmp(Arr + J * ElemSize, Pivot) > 0);
		if (I >= J) return J;
		SwapElems(Arr + I * ElemSize, Arr + J * ElemSize, ElemSize);
	}
}

static void IntroSortRec(char *Arr, size_t Count, size_t ElemSize,
		int (*Cmp)(const void *, const void *), size_t DepthLimit)
{
	if (Count < 2) return;
	if (DepthLimit == 0) {
		HeapSort(Arr, Count, ElemSize, Cmp);
		return;
	}
	size_t J = HoarePartition(Arr, 0, Count - 1, ElemSize, Cmp);
	size_t LeftCount = J + 1;
	size_t RightCount = Count - LeftCount;
	if (LeftCount < RightCount) {
		IntroSortRec(Arr, LeftCount, ElemSize, Cmp, DepthLimit - 1);
		IntroSortRec(Arr + (J + 1) * ElemSize, RightCount, ElemSize, Cmp, DepthLimit - 1);
	} else {
		IntroSortRec(Arr + (J + 1) * ElemSize, RightCount, ElemSize, Cmp, DepthLimit - 1);
		IntroSortRec(Arr, LeftCount, ElemSize, Cmp, DepthLimit - 1);
	}
}

// ===========================================================================
//  Tilty — Array operations
// ===========================================================================

void TiltyArrayInit(Arena *A, Array *Arr, size_t ElemSize, size_t InitCap) {
	Arr->MemArena = A;
	Arr->Length = 0;
	Arr->ElemSize = ElemSize;

	if (InitCap == 0) InitCap = 4;
	size_t MaxCap = MaxCapacity(ElemSize);
	if (InitCap > MaxCap) InitCap = MaxCap;
	size_t Bytes = InitCap * ElemSize;

	Arr->Data = KobayashiAlloc(A, Bytes);
	if (Arr->Data == TohruZeroBlock) {
		Arr->Capacity = 0;
		return;
	}
	Arr->Capacity = InitCap;
}

void TiltyArrayDestroy(Array *Arr) {
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

void TiltyArrayInsert(Array *Arr, size_t Index, const void *Elem) {
	if (Index > Arr->Length) Index = Arr->Length;
	if (Arr->Length >= Arr->Capacity) {
		ArrayGrow(Arr, Arr->Length + 1);
		if (Arr->Data == TohruZeroBlock) return;
	}
	// Shift elements right.
	char *Pos = (char *)Arr->Data + Index * Arr->ElemSize;
	size_t ToMove = Arr->Length - Index;
	if (ToMove > 0) {
		memmove(Pos + Arr->ElemSize, Pos, ToMove * Arr->ElemSize);
	}
	memcpy(Pos, Elem, Arr->ElemSize);
	Arr->Length++;
}

void TiltyArrayRemove(Array *Arr, size_t Index) {
	if (Index >= Arr->Length) return;
	// Shift elements left.
	char *Pos = (char *)Arr->Data + Index * Arr->ElemSize;
	size_t ToMove = Arr->Length - Index - 1;
	if (ToMove > 0) {
		memmove(Pos, Pos + Arr->ElemSize, ToMove * Arr->ElemSize);
	}
	Arr->Length--;
}

void TiltyArrayRemoveAt(Array *Arr, size_t Index) {
	TiltyArrayRemove(Arr, Index);
}

void TiltyArrayReserve(Array *Arr, size_t NewCap) {
	if (NewCap <= Arr->Capacity) return;

	size_t MaxCap = MaxCapacity(Arr->ElemSize);
	if (NewCap > MaxCap) NewCap = MaxCap;
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

void TiltyArrayResize(Array *Arr, size_t NewLength) {
	if (NewLength > Arr->Capacity) {
		ArrayGrow(Arr, NewLength);
		if (Arr->Data == TohruZeroBlock) return;
	}
	Arr->Length = NewLength;
}

void TiltyArrayShrinkToFit(Array *Arr) {
	if (Arr->Length >= Arr->Capacity || Arr->Capacity == 0) return;
	size_t MaxCap = MaxCapacity(Arr->ElemSize);
	size_t NewCap = Arr->Length > 0 ? Arr->Length : 4;
	if (NewCap > MaxCap) NewCap = MaxCap;

	size_t Bytes = NewCap * Arr->ElemSize;
	void *NewData = KobayashiAlloc(Arr->MemArena, Bytes);
	if (NewData == TohruZeroBlock) return;

	if (Arr->Data && Arr->Data != TohruZeroBlock && Arr->Length > 0) {
		memcpy(NewData, Arr->Data, Arr->Length * Arr->ElemSize);
	}

	Arr->Data = NewData;
	Arr->Capacity = NewCap;
}

void TiltyArrayClear(Array *Arr) {
	Arr->Length = 0;
}

void *TiltyArrayFront(const Array *Arr) {
	return TiltyArrayGet(Arr, 0);
}

void *TiltyArrayBack(const Array *Arr) {
	if (Arr->Length == 0) return (void *)TohruZeroBlock;
	return (char *)Arr->Data + (Arr->Length - 1) * Arr->ElemSize;
}

void *TiltyArrayData(const Array *Arr) {
	return Arr->Data;
}

int TiltyArrayIsEmpty(const Array *Arr) {
	return Arr->Length == 0;
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
	if (Arr->Length < 2) return;

	size_t N = Arr->Length;
	size_t DepthLimit = 0;
	while (N > 1) { N >>= 1; DepthLimit++; }
	DepthLimit *= 2;

	IntroSortRec((char *)Arr->Data, Arr->Length, Arr->ElemSize, Cmp, DepthLimit);
}

void TiltyArrayForEach(const Array *Arr,
		void (*Fn)(void *Elem, void *Ctx), void *Ctx)
{
	for (size_t I = 0; I < Arr->Length; I++) {
		void *Elem = (char *)Arr->Data + I * Arr->ElemSize;
		Fn(Elem, Ctx);
	}
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
	if (G->Backing.Data != TohruZeroBlock) {
		G->Backing.Length = Width * Height;
	}
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

void TiltyGrid2DSet(const Grid2D *G, size_t X, size_t Y, const void *Elem) {
	void *Dst = TiltyGrid2DGet(G, X, Y);
	if (Dst != TohruZeroBlock) {
		memcpy(Dst, Elem, G->Backing.ElemSize);
	}
}

void TiltyGrid2DClear(Grid2D *G) {
	TiltyArrayClear(&G->Backing);
}
