/**
 * Expandable page-aligned bump arena for TohruPhysics.
 * TohruPhysics用の拡張可能なページアラインメントバンプアリーナね。
 *
 * Backed by mmap/mremap — doubles capacity automatically when exhausted
 * so we never run out of memory mid-physics. Zero fill on clear always
 * produces a valid state.
 * mmap/mremapを使ってて、枯渇すると自動で倍になるの。クリアすると常にゼロ埋めで有効状態になるわ。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#pragma once

#include <stddef.h>
#include <stdint.h>

// ---------------------------------------------------------------------------
//  Error value — the only error type in the engine.
//  エラー値 — エンジンで唯一のエラー型よ。
// ---------------------------------------------------------------------------
typedef struct {
	int    Code;
	char   Message[256];
} Error;

static inline Error ErrOk(void) {
	Error E = {0, {0}};
	return E;
}
static inline Error ErrMake(int Code) {
	Error E = {Code, {0}};
	return E;
}
#define ErrIsOk(E)       ((E).Code == 0)
#define ErrIsFail(E)     ((E).Code != 0)

// ---------------------------------------------------------------------------
//  Arena — expandable bump allocator.
//  拡張可能なバンプアロケーターね。
// ---------------------------------------------------------------------------
typedef struct {
	void  *Base;
	size_t Capacity;
	size_t Offset;
} Arena;

// Default initial capacity (64KB — grows fast via doubling)
#define ARENA_DEFAULT_CAPACITY ((size_t)64 * 1024)

// Minimum alignment for all arena allocations
#define ARENA_MIN_ALIGNMENT    ((size_t)16)

/**
 * Tohru — initialise an arena with page-aligned mmap.
 * mmapでページアラインメントしてアリーナを初期化するわ。
 *
 * @param A          — uninitialised arena (must not be null).
 * @param InitCap    — initial capacity in bytes (0 = ARENA_DEFAULT_CAPACITY).
 * @returns          — ErrOk() on success, ErrMake(Code) on mmap failure.
 */
Error TohruArenaInit(Arena *A, size_t InitCap);

/**
 * Tohru — initialise an arena from an externally owned buffer (no growth).
 * 外部バッファからアリーナを初期化するわ（拡張なし）。
 */
Error TohruArenaInitFixed(Arena *A, void *Buffer, size_t Capacity);

/**
 * Tohru — release the backing pages.
 * バッキングページを解放するの。
 */
void TohruArenaDestroy(Arena *A);

/**
 * Kobayashi — allocate uninitialised bytes from the arena.
 * アリーナから未初期化のバイト列を確保するわ。
 *
 * Grows the backing map if insufficient space remains.
 * 残り容量が足りないときはバッキングマップを拡張するの。
 *
 * @returns a pointer to at least `Size` bytes, guaranteed non-null
 *          when the arena backing store can be expanded.
 *          バッキングストアが拡張可能なら非NULLが保証されるわ。
 */
void *KobayashiAlloc(Arena *A, size_t Size);

/**
 * Kobayashi — allocate with explicit alignment.
 * 指定したアラインメントで確保するの。
 *
 * Alignment must be a power of two.
 * アラインメントは2の累乗じゃないとダメよ。
 */
void *KobayashiAllocAlign(Arena *A, size_t Size, size_t Align);

/**
 * Kobayashi — duplicate a memory block into the arena.
 * メモリブロックをアリーナに複製するわ。
 *
 * Returns a pointer to the newly allocated copy, or the original
 * pointer when Size == 0.
 * 新しく確保したコピーへのポインタを返すの。Size == 0のときは元のポインタを返すわ。
 */
void *KobayashiDup(Arena *A, const void *Src, size_t Size);

/**
 * Elma — reset the arena offset (does NOT zero memory).
 * アリーナのオフセットをリセットするの（メモリはゼロにしない）。
 *
 * O(1), use this in the per-frame allocator path.
 * フレームアロケーターのパスではこっちを使ってね。
 */
void ElmaArenaReset(Arena *A);

/**
 * Elma — zero everything and reset.
 * 全メモリをゼロにしてリセットするの。
 *
 * O(n) — use for teardown, not per-frame.
 * 後片付け用ね。フレームごとには使わないで。
 */
void ElmaArenaClear(Arena *A);

/**
 * Elma — query arena state.
 * アリーナの状態を確認するの。
 */
size_t ElmaArenaUsed(Arena *A);
size_t ElmaArenaRemaining(Arena *A);

/**
 * Ilulu — pointer / offset conversion.
 * ポインタとオフセットの変換をするの。
 *
 * Useful for serialisation and deterministic state capture.
 * シリアライゼーションと決定論的状態キャプチャに便利よ。
 */
size_t IluluOffset(Arena *A, const void *Ptr);
void  *IluluPtr(Arena *A, size_t Offset);
int    IluluOwns(Arena *A, const void *Ptr);

// ---------------------------------------------------------------------------
//  Arena partitioning — scoped sub-arenas for different simulation phases.
//  アリーナ分割 — シミュレーションフェーズごとにスコープを分けるの。
// ---------------------------------------------------------------------------
typedef enum {
	ArenaType_Frame,   // per-timestep scratch (reset each step)
	ArenaType_World,   // persistent world state
	ArenaType_Worker,  // per-worker scratch
	ArenaType_Count
} ArenaType;

typedef struct {
	Arena Arenas[ArenaType_Count];
} ArenaSet;

/**
 * Yuyu — initialise a full arena set with per-type capacities.
 * アリーナセット全体をタイプごとの容量で初期化するの。
 *
 * Any capacity can be 0 to use ARENA_DEFAULT_CAPACITY.
 * 容量を0にするとARENA_DEFAULT_CAPACITYが使われるわ。
 */
Error YuyuArenaSetInit(ArenaSet *S, size_t FrameCap, size_t WorldCap, size_t WorkerCap);

/**
 * Yuyu — destroy all arenas in the set.
 * セット内の全アリーナを破棄するの。
 */
void YuyuArenaSetDestroy(ArenaSet *S);

/**
 * Yuyu — reset only the frame arena (O(1)).
 * フレームアリーナだけをリセットするの（O(1)）。
 */
void YuyuArenaSetResetFrame(ArenaSet *S);
