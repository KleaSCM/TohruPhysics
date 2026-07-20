/**
 * NoHeap — compile-time guard against dynamic allocation.
 * TohruPhysicsのヒープ禁止ガードよ。
 *
 * Deletes all global operator new/delete overloads. Any translation unit
 * that includes this header will fail to link if it calls new or delete.
 * Hot paths in the engine must never heap-allocate.
 *
 * DESIGN PHILOSOPHY:
 * A single malloc in a physics hot path (per-frame contact generation,
 * constraint solving) destroys cache coherence and introduces unpredictable
 * latency. By deleting the operators at compile time, we guarantee that
 * no library code accidentally uses the heap. This is the same approach
 * used by embedded and real-time systems (PS4, Xbox, QNX).
 *
 * References:
 * - ISO C++ [basic.stc.dynamic]/2 — program-defined replacements
 * - Real-time programming: "no heap allocation in hot paths"
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#pragma once

#include <stddef.h>

void *operator new(size_t) = delete;
void *operator new[](size_t) = delete;
void operator delete(void *) noexcept = delete;
void operator delete[](void *) noexcept = delete;
void operator delete(void *, size_t) noexcept = delete;
void operator delete[](void *, size_t) noexcept = delete;
void *operator new(size_t, void *) = delete;
void *operator new[](size_t, void *) = delete;
