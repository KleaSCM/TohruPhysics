/**
 * Expandable arena backing store — mmap/mremap growth with ZeroBlock fallback.
 * 拡張可能なアリーナのバッキングストア — mmap/mremap で拡張、ZeroBlockフォールバック。
 *
 * Zero-is-valid: KobayashiAlloc/KobayashiAllocAlign never return null.
 * On mremap failure they return &TohruZeroBlock. Callers never check.
 * Zero-is-valid: KobayashiAlloc/KobayashiAllocAlign は絶対にnullを返さないの。
 * mremap が失敗したら &TohruZeroBlock を返すわ。呼び出し側はチェックしないでね。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include <TohruPhysics/Arena.h>

#include <string.h>
#include <unistd.h>
#include <sys/mman.h>

// ---------------------------------------------------------------------------
//  TohruZeroBlock — one zeroed page in .bss.
//  .bss 上のゼロ埋めページよ。
// ---------------------------------------------------------------------------
char TohruZeroBlock[ZEROBLOCK_SIZE] = {0};

// ---------------------------------------------------------------------------
//  Internal helpers
// ---------------------------------------------------------------------------

static size_t GetPageSize(void) {
	static size_t PageSz = 0;
	if (!PageSz) {
		PageSz = (size_t)sysconf(_SC_PAGESIZE);
	}
	return PageSz;
}

static size_t GrowCapacity(size_t Current) {
	size_t Grown = Current * 2;
	size_t Min = ARENA_DEFAULT_CAPACITY;
	return Grown > Min ? Grown : Min;
}

static size_t RoundUpPage(size_t Sz) {
	size_t PS = GetPageSize();
	return (Sz + PS - 1) & ~(PS - 1);
}

static size_t MappedSize(Arena *A) {
	return RoundUpPage(A->Capacity);
}

// ---------------------------------------------------------------------------
//  Tohru — arena lifecycle
// ---------------------------------------------------------------------------

Error TohruArenaInit(Arena *A, size_t InitCap) {
	if (!A) {
		return ErrMake(-1);
	}

	size_t Cap = InitCap > 0 ? InitCap : ARENA_DEFAULT_CAPACITY;
	size_t Mapped = RoundUpPage(Cap);

	void *Base = mmap(
		NULL, Mapped,
		PROT_READ | PROT_WRITE,
		MAP_PRIVATE | MAP_ANONYMOUS,
		-1, 0
	);

	if (Base == MAP_FAILED) {
		// NOTE (KleaSCM) Init failure IS possible (OOM, ulimit).
		// Initialise as a fixed arena on the ZeroBlock as fallback.
		// 初期化失敗はありえるの。フォールバックとしてZeroBlockで固定アリーナにするわ。
		A->Base = TohruZeroBlock;
		A->Capacity = ZEROBLOCK_SIZE;
		A->Offset = 0;
		return ErrMake(-2);
	}

	A->Base = Base;
	A->Capacity = Cap;
	A->Offset = 0;
	return ErrOk();
}

Error TohruArenaInitFixed(Arena *A, void *Buffer, size_t Capacity) {
	if (!A || !Buffer || Capacity == 0) {
		return ErrMake(-1);
	}

	A->Base = Buffer;
	A->Capacity = Capacity;
	A->Offset = 0;
	return ErrOk();
}

void TohruArenaDestroy(Arena *A) {
	if (!A || !A->Base) {
		return;
	}

	//  ZeroBlock のときは munmap しないわ。
	if (A->Base != (void *)TohruZeroBlock) {
		size_t Mapped = MappedSize(A);
		munmap(A->Base, Mapped);
	}

	A->Base = NULL;
	A->Capacity = 0;
	A->Offset = 0;
}

// ---------------------------------------------------------------------------
//  Kobayashi — allocation (zero-is-valid: never null)
// ---------------------------------------------------------------------------

// Try to grow. Returns 0 on success, -1 on failure (caller uses ZeroBlock).
static int EnsureSpace(Arena *A, size_t Needed) {
	size_t Remaining = A->Capacity - A->Offset;
	if (Needed <= Remaining) {
		return 0;
	}

	size_t NewCap = A->Capacity;
	while (NewCap - A->Offset < Needed) {
		NewCap = GrowCapacity(NewCap);
	}

	size_t OldMapped = MappedSize(A);
	size_t NewMapped = RoundUpPage(NewCap);

	void *NewBase = mremap(A->Base, OldMapped, NewMapped, MREMAP_MAYMOVE);
	if (NewBase == MAP_FAILED) {
		return -1;
	}

	A->Base = NewBase;
	A->Capacity = NewCap;
	return 0;
}

void *KobayashiAlloc(Arena *A, size_t Size) {
	if (!A || Size == 0) {
		return (void *)TohruZeroBlock;
	}

	if (EnsureSpace(A, Size) != 0) {
		// Zero-is-valid: return the global stub instead of NULL.
		// 呼び出し側は常に有効なポインタを得るの。
		return (void *)TohruZeroBlock;
	}

	void *Ptr = (void *)((char *)A->Base + A->Offset);
	A->Offset += Size;
	return Ptr;
}

void *KobayashiAllocAlign(Arena *A, size_t Size, size_t Align) {
	if (!A || Size == 0) {
		return (void *)TohruZeroBlock;
	}

	if ((Align & (Align - 1)) != 0) {
		Align = ARENA_MIN_ALIGNMENT;
	}
	if (Align < ARENA_MIN_ALIGNMENT) {
		Align = ARENA_MIN_ALIGNMENT;
	}

	uintptr_t Cur = (uintptr_t)A->Base + A->Offset;
	uintptr_t Aligned = (Cur + Align - 1) & ~(Align - 1);
	size_t    Padding = (size_t)(Aligned - Cur);

	if (EnsureSpace(A, Padding + Size) != 0) {
		return (void *)TohruZeroBlock;
	}

	A->Offset += Padding;
	void *Ptr = (void *)((char *)A->Base + A->Offset);
	A->Offset += Size;
	return Ptr;
}

void *KobayashiDup(Arena *A, const void *Src, size_t Size) {
	if (!A || !Src || Size == 0) {
		return (void *)TohruZeroBlock;
	}

	void *Dst = KobayashiAlloc(A, Size);
	memcpy(Dst, Src, Size);
	return Dst;
}

// ---------------------------------------------------------------------------
//  Elma — arena lifecycle management
// ---------------------------------------------------------------------------

void ElmaArenaReset(Arena *A) {
	if (A) {
		A->Offset = 0;
	}
}

void ElmaArenaClear(Arena *A) {
	if (!A) {
		return;
	}

	memset(A->Base, 0, A->Capacity);
	A->Offset = 0;
}

size_t ElmaArenaUsed(Arena *A) {
	return A ? A->Offset : 0;
}

size_t ElmaArenaRemaining(Arena *A) {
	return A ? (A->Capacity - A->Offset) : 0;
}

// ---------------------------------------------------------------------------
//  Ilulu — pointer / offset conversion
// ---------------------------------------------------------------------------

size_t IluluOffset(Arena *A, const void *Ptr) {
	if (!A || !Ptr) {
		return 0;
	}
	return (size_t)((const char *)Ptr - (const char *)A->Base);
}

void *IluluPtr(Arena *A, size_t Offset) {
	if (!A || Offset >= A->Capacity) {
		return (void *)TohruZeroBlock;
	}
	return (void *)((char *)A->Base + Offset);
}

int IluluOwns(Arena *A, const void *Ptr) {
	if (!A || !Ptr) {
		return 0;
	}
	const char *Base = (const char *)A->Base;
	const char *End  = Base + A->Offset;
	const char *P    = (const char *)Ptr;
	return (P >= Base && P < End) ? 1 : 0;
}

// ---------------------------------------------------------------------------
//  Yuyu — arena set (multi-layer partitioning)
// ---------------------------------------------------------------------------

Error YuyuArenaSetInit(ArenaSet *S, size_t FrameCap, size_t WorldCap, size_t WorkerCap) {
	if (!S) {
		return ErrMake(-1);
	}

	Error Err;

	Err = TohruArenaInit(&S->Arenas[ArenaType_Frame],
		FrameCap > 0 ? FrameCap : ARENA_DEFAULT_CAPACITY);
	if (ErrIsFail(Err)) return Err;

	Err = TohruArenaInit(&S->Arenas[ArenaType_World],
		WorldCap > 0 ? WorldCap : ARENA_DEFAULT_CAPACITY * 8);
	if (ErrIsFail(Err)) {
		TohruArenaDestroy(&S->Arenas[ArenaType_Frame]);
		return Err;
	}

	Err = TohruArenaInit(&S->Arenas[ArenaType_Worker],
		WorkerCap > 0 ? WorkerCap : ARENA_DEFAULT_CAPACITY);
	if (ErrIsFail(Err)) {
		TohruArenaDestroy(&S->Arenas[ArenaType_Frame]);
		TohruArenaDestroy(&S->Arenas[ArenaType_World]);
		return Err;
	}

	return ErrOk();
}

void YuyuArenaSetDestroy(ArenaSet *S) {
	if (!S) return;

	for (int I = 0; I < (int)ArenaType_Count; I++) {
		TohruArenaDestroy(&S->Arenas[I]);
	}
}

void YuyuArenaSetResetFrame(ArenaSet *S) {
	if (!S) return;
	ElmaArenaReset(&S->Arenas[ArenaType_Frame]);
}
