/**
 * Expandable page-aligned bump arena for TohruPhysics.
 * TohruPhysics用の拡張可能なページアラインメントバンプアリーナね。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#pragma once

#include <stddef.h>
#include <stdint.h>

// ---------------------------------------------------------------------------
//  Error value — for init/startup only, not for runtime paths.
//  エラー値 — 初期化/起動専用、実行パスでは使わないの。
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
//  ZeroBlock — global fallback zeroed page.
//  グローバルフォールバックのゼロ埋めページね。
//
//  Every runtime allocation returns a valid pointer. When the arena cannot
//  grow, it returns &TohruZeroBlock. The first write might segfault but
//  in practice the arena sizing is tuned so this path is never hit in
//  production — this is a safety net for malformed scenes, not a normal
//  code path.
//  全てのランタイム確保が有効なポインタを返すの。アリーナが拡張できないときは
//  &TohruZeroBlockを返すわ。プロダクションではこのパスは絶対に通らない
//  セーフティネットよ。
// ---------------------------------------------------------------------------
#define ZEROBLOCK_SIZE ((size_t)4096)

extern char TohruZeroBlock[ZEROBLOCK_SIZE];

// ---------------------------------------------------------------------------
//  Arena — expandable bump allocator.
//  拡張可能なバンプアロケーターね。
// ---------------------------------------------------------------------------
typedef struct {
	void  *Base;
	size_t Capacity;
	size_t Offset;
} Arena;

#define ARENA_DEFAULT_CAPACITY ((size_t)64 * 1024)
#define ARENA_MIN_ALIGNMENT    ((size_t)16)

/**
 * Tohru — initialise an arena with page-aligned mmap.
 * mmapでページアラインメントしてアリーナを初期化するわ。
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
 * Kobayashi — allocate bytes. Never returns null.
 * バイト列を確保するの。絶対にnullにならないわ。
 *
 * On OOM returns &TohruZeroBlock. Callers never check for null.
 * OOMのときは&TohruZeroBlockを返すの。呼び出し側はnullチェックしないでね。
 */
void *KobayashiAlloc(Arena *A, size_t Size);

/**
 * Kobayashi — allocate with explicit alignment. Never null.
 * 指定したアラインメントで確保するの。絶対にnullにならないわ。
 */
void *KobayashiAllocAlign(Arena *A, size_t Size, size_t Align);

/**
 * Kobayashi — duplicate a memory block. Never null.
 * メモリブロックを複製するの。絶対にnullにならないわ。
 */
void *KobayashiDup(Arena *A, const void *Src, size_t Size);

/**
 * Elma — reset the arena offset (O(1), does NOT zero memory).
 * アリーナのオフセットをリセットするの（メモリはゼロにしない）。
 */
void ElmaArenaReset(Arena *A);

/**
 * Elma — zero everything and reset (O(n)).
 * 全メモリをゼロにしてリセットするの。
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
 */
size_t IluluOffset(Arena *A, const void *Ptr);
void  *IluluPtr(Arena *A, size_t Offset);
int    IluluOwns(Arena *A, const void *Ptr);

// ---------------------------------------------------------------------------
//  Arena partitioning — scoped sub-arenas.
//  アリーナ分割 — スコープ付きサブアリーナね。
// ---------------------------------------------------------------------------
typedef enum {
	ArenaType_Frame,
	ArenaType_World,
	ArenaType_Worker,
	ArenaType_Count
} ArenaType;

typedef struct {
	Arena Arenas[ArenaType_Count];
} ArenaSet;

Error YuyuArenaSetInit(ArenaSet *S, size_t FrameCap, size_t WorldCap, size_t WorkerCap);
void  YuyuArenaSetDestroy(ArenaSet *S);
void  YuyuArenaSetResetFrame(ArenaSet *S);
