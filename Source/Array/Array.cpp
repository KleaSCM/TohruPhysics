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

// Maximum safe capacity: Capacity * ElemSize must not overflow SIZE_MAX.
// 最大安全容量: Capacity * ElemSize が SIZE_MAX をオーバーフローしない値よ。
static size_t MaxCapacity(size_t ElemSize) {
	return SIZE_MAX / ElemSize;
}

static void ArrayGrow(Array *Arr, size_t Need) {
	size_t NewCap = Arr->Capacity == 0 ? 4 : Arr->Capacity;
	size_t MaxCap = MaxCapacity(Arr->ElemSize);
	while (NewCap < Need) {
		// Clamp to max capacity to avoid infinite loop.
		// 無限ループ防止のため最大容量でクランプするの。
		if (NewCap > MaxCap / 2) {
			NewCap = MaxCap;
			break;
		}
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
//  Introsort: quicksort with depth limit → heapsort fallback.
//  イントロソート: 深さ制限付きクイックソート → ヒープソートフォールバック。
//
//  Hoare partition handles equal elements without O(n²) degradation.
//  Hoare分割は同値要素でもO(n²)劣化しないの。
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

// Returns the split index J; recurse on [Lo..J] and [J+1..Hi].
static size_t HoarePartition(char *Arr, size_t Lo, size_t Hi, size_t ElemSize,
		int (*Cmp)(const void *, const void *))
{
	// Median-of-3 into Arr[Lo].
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

	// Hoare partition gives J where all [Lo..J] ≤ pivot and [J+1..Hi] ≥ pivot.
	size_t J = HoarePartition(Arr, 0, Count - 1, ElemSize, Cmp);

	// Recurse on smaller partition first to limit stack depth.
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
	// Clamp to max capacity.
	// 最大容量でクランプするの。
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

	// Clamp to max capacity to prevent overflow.
	// オーバーフロー防止のため最大容量でクランプするの。
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

	// Depth limit = 2 * floor(log2(n)) → heapsort fallback prevents O(n²).
	// 深さ制限 = 2 * floor(log2(n)) → ヒープソートフォールバックでO(n²)を防止。
	size_t N = Arr->Length;
	size_t DepthLimit = 0;
	while (N > 1) { N >>= 1; DepthLimit++; }
	DepthLimit *= 2;

	IntroSortRec((char *)Arr->Data, Arr->Length, Arr->ElemSize, Cmp, DepthLimit);
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
