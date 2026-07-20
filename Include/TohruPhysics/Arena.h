/**
 * Arena — page-aligned expandable bump allocator.
 * TohruPhysics用の拡張可能なページアラインメントバンプアリーナね。
 *
 * Contiguous virtual memory region with bump-pointer allocation.
 * Grows via mremap(2) when exhausted. Never returns null — OOM returns
 * &TohruZeroBlock (a zeroed .bss page) so callers never branch.
 *
 * DESIGN PHILOSOPHY:
 * Physics engines allocate millions of short-lived objects per frame
 * (contacts, constraints, islands). Per-object malloc/free destroys
 * cache locality and fragments the heap. A bump arena allocates in
 * O(1), has zero fragmentation, and batch-frees by resetting the
 * offset — no per-element destructors needed.
 *
 * Backed by mmap/mremap with MREMAP_MAYMOVE so the arena can grow
 * without reserving virtual address space up front. Doubles capacity
 * on each expansion (geometric growth).
 *
 * MEMORY LAYOUT:
 * ┌──────────────────────────────────────────────────────────┐
 * │  mmap'd page-aligned region (or external fixed buffer)   │
 * ├──────────────────────┬───────────────────────────────────┤
 * │    Allocated data    │        Free (remaining)           │
 * ├──────────────────────┴───────────────────────────────────┤
 * │ 0                Offset                              Capacity │
 * └──────────────────────────────────────────────────────────┘
 *
 * ZEROBLOCK:
 * ┌──────────────────────────────────────────────────────────┐
 * │ 4096 bytes of .bss zeroes (global TohruZeroBlock)        │
 * │ Every OOM path aliases here. Never a null deref.         │
 * └──────────────────────────────────────────────────────────┘
 *
 * References:
 * - mmap(2), mremap(2) — Linux virtual memory syscalls
 * - "Arena Allocator" pattern in game engine memory systems
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#pragma once

#include <stddef.h>
#include <stdint.h>
#include <TohruPhysics/Error.h>

#define ZEROBLOCK_SIZE ((size_t)4096)

extern char TohruZeroBlock[ZEROBLOCK_SIZE];

// ---------------------------------------------------------------------------
//  Arena — bump allocator state
//  バンプアロケーターの状態ね。
// ---------------------------------------------------------------------------
typedef struct {
	void  *Base;
	size_t Capacity;
	size_t Offset;
} Arena;

#define ARENA_DEFAULT_CAPACITY ((size_t)64 * 1024)
#define ARENA_MIN_ALIGNMENT    ((size_t)16)

Error TohruArenaInit(Arena *A, size_t InitCap);
Error TohruArenaInitFixed(Arena *A, void *Buffer, size_t Capacity);
void  TohruArenaDestroy(Arena *A);

void *KobayashiAlloc(Arena *A, size_t Size);
void *KobayashiAllocZeroed(Arena *A, size_t Size);
void *KobayashiAllocAlign(Arena *A, size_t Size, size_t Align);
void *KobayashiDup(Arena *A, const void *Src, size_t Size);

void   ElmaArenaReset(Arena *A);
void   ElmaArenaClear(Arena *A);
size_t ElmaArenaUsed(Arena *A);
size_t ElmaArenaRemaining(Arena *A);
size_t ElmaArenaSnapshot(Arena *A);
void   ElmaArenaRollback(Arena *A, size_t Snapshot);

size_t IluluOffset(Arena *A, const void *Ptr);
void  *IluluPtr(Arena *A, size_t Offset);
int    IluluOwns(Arena *A, const void *Ptr);

// ---------------------------------------------------------------------------
//  ArenaSet — partitioned sub-arenas by lifetime scope
//  ライフタイムスコープ別の分割サブアリーナね。
// ---------------------------------------------------------------------------
typedef enum {
	ArenaTypeFrame,
	ArenaTypeWorld,
	ArenaTypeWorker,
	ArenaTypeCount
} ArenaType;

typedef struct {
	Arena Arenas[ArenaTypeCount];
} ArenaSet;

Error YuyuArenaSetInit(ArenaSet *S, size_t FrameCap, size_t WorldCap, size_t WorkerCap);
void  YuyuArenaSetDestroy(ArenaSet *S);
void  YuyuArenaSetResetFrame(ArenaSet *S);
