/**
 * Expandable arena backing store — mmap/mremap growth.
 * 拡張可能なアリーナのバッキングストア — mmap/mremap で拡張するの。
 *
 * We use anonymous mmap for the initial allocation and mremap
 * (with MREMAP_MAYMOVE) when the arena runs out. This keeps
 * all physics data in contiguous virtual memory.
 * 最初は anonymous mmap で確保して、足りなくなったら mremap で拡張するわ。
 * これで物理データが仮想メモリ上で連続したままになるの。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include <TohruPhysics/Arena.h>

#include <string.h>
#include <unistd.h>
#include <sys/mman.h>

// ---------------------------------------------------------------------------
//  Internal helpers
// ---------------------------------------------------------------------------

// システムページサイズをキャッシュしておくの。
static size_t GetPageSize(void) {
	static size_t PageSz = 0;
	if (!PageSz) {
		PageSz = (size_t)sysconf(_SC_PAGESIZE);
	}
	return PageSz;
}

// リサイズ後の最小アリーナサイズね。
static size_t GrowCapacity(size_t Current) {
	size_t Grown = Current * 2;
	size_t Min = ARENA_DEFAULT_CAPACITY;
	return Grown > Min ? Grown : Min;
}

// ページ境界に切り上げるの。
static size_t RoundUpPage(size_t Sz) {
	size_t PS = GetPageSize();
	return (Sz + PS - 1) & ~(PS - 1);
}

// 現在のマップ済みサイズ（ページ境界）を返すわ。
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

	size_t Mapped = MappedSize(A);
	munmap(A->Base, Mapped);

	A->Base = NULL;
	A->Capacity = 0;
	A->Offset = 0;
}

// ---------------------------------------------------------------------------
//  Kobayashi — allocation
// ---------------------------------------------------------------------------

// Try to grow the arena by `Needed` bytes.
// アリーナをNeededバイト分拡張しようとするの。
static Error EnsureSpace(Arena *A, size_t Needed) {
	size_t Remaining = A->Capacity - A->Offset;
	if (Needed <= Remaining) {
		return ErrOk();
	}

	// Keep doubling until we have room.
	// 容量が足りるまで倍にしていくわ。
	size_t NewCap = A->Capacity;
	while (NewCap - A->Offset < Needed) {
		NewCap = GrowCapacity(NewCap);
	}

	size_t OldMapped = MappedSize(A);
	size_t NewMapped = RoundUpPage(NewCap);

	// Try mremap first — keeps virtual address contiguous.
	// まず mremap を試すの — 仮想アドレスが連続したままになるわ。
	void *NewBase = mremap(A->Base, OldMapped, NewMapped, MREMAP_MAYMOVE);
	if (NewBase == MAP_FAILED) {
		return ErrMake(-3);
	}

	A->Base = NewBase;
	A->Capacity = NewCap;
	return ErrOk();
}

void *KobayashiAlloc(Arena *A, size_t Size) {
	if (!A || Size == 0) {
		return NULL;
	}

	// NOTE (KleaSCM) EnsureSpace grows the arena transparently —
	// callers never see a full arena.
	if (ErrIsFail(EnsureSpace(A, Size))) {
		return NULL;
	}

	void *Ptr = (void *)((char *)A->Base + A->Offset);
	A->Offset += Size;
	return Ptr;
}

void *KobayashiAllocAlign(Arena *A, size_t Size, size_t Align) {
	if (!A || Size == 0) {
		return NULL;
	}

	// Alignment must be power of two.
	// アラインメントは2の累乗じゃないとね。
	if ((Align & (Align - 1)) != 0) {
		Align = ARENA_MIN_ALIGNMENT;
	}
	if (Align < ARENA_MIN_ALIGNMENT) {
		Align = ARENA_MIN_ALIGNMENT;
	}

	// Align the current offset up.
	uintptr_t Cur = (uintptr_t)A->Base + A->Offset;
	uintptr_t Aligned = (Cur + Align - 1) & ~(Align - 1);
	size_t    Padding = (size_t)(Aligned - Cur);

	if (ErrIsFail(EnsureSpace(A, Padding + Size))) {
		return NULL;
	}

	A->Offset += Padding;
	void *Ptr = (void *)((char *)A->Base + A->Offset);
	A->Offset += Size;
	return Ptr;
}

void *KobayashiDup(Arena *A, const void *Src, size_t Size) {
	if (!A || !Src || Size == 0) {
		return NULL;
	}

	void *Dst = KobayashiAlloc(A, Size);
	if (Dst) {
		memcpy(Dst, Src, Size);
	}
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

	// NOTE (KleaSCM) Zero-fill guarantees the ZeroBlock invariant:
	// every bit pattern produced by the arena is a valid default state.
	// ゼロフィルは ZeroBlock 不変条件を保証するの：
	// アリーナが生成するすべてのビットパターンが有効なデフォルト状態になるわ。
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
		return NULL;
	}
	return (void *)((char *)A->Base + Offset);
}

int IluluOwns(Arena *A, const void *Ptr) {
	if (!A || !Ptr) {
		return 0;
	}
	const char *Base = (const char *)A->Base;
	const char *End  = Base + A->Offset; // NOTE: only check used range
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

	// Frame: per-timestep scratch, aggressively sized.
	// フレーム: タイムステップごとのスクラッチ、積極的に確保するの。
	Err = TohruArenaInit(&S->Arenas[ArenaType_Frame],
		FrameCap > 0 ? FrameCap : ARENA_DEFAULT_CAPACITY);
	if (ErrIsFail(Err)) return Err;

	// World: persistent, largest.
	// ワールド: 永続的、一番大きいの。
	Err = TohruArenaInit(&S->Arenas[ArenaType_World],
		WorldCap > 0 ? WorldCap : ARENA_DEFAULT_CAPACITY * 8);
	if (ErrIsFail(Err)) {
		TohruArenaDestroy(&S->Arenas[ArenaType_Frame]);
		return Err;
	}

	// Worker: per-thread scratch.
	// ワーカー: スレッドごとのスクラッチね。
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
