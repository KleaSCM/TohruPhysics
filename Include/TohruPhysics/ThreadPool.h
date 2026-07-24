/**
 * ThreadPool — mutex-based thread pool for parallel task execution.
 * TohruPhysics用のスレッドプールね。
 *
 * Workers and the calling thread share a single mutex-protected queue of
 * task indices.  No lock-free work-stealing.
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#pragma once

#include <cstddef>
#include <cstdint>
#include <mutex>
#include <condition_variable>

#define THREADPOOL_MAX_THREADS     64
#define THREADPOOL_DEFAULT_THREADS 4

// Task function signature: Func(Arg, ThreadIndex, TaskIndex)
typedef void (*ThreadTaskFunc)(void *Arg, int ThreadIndex, int TaskIndex);

// Forward decl for the per-batch context stored on the pool
struct ParForCtx;

// 0187: Thread pool state.
// ThreadPoolInit must be called before use; zero-initialised state is safe
// to destroy (all pointers null → no-op).
typedef struct ThreadPool {
	// Opaque handle storage — implementation manages std::thread objects.
	// Zero-initialised = no threads.
	void         *ThreadData[THREADPOOL_MAX_THREADS];
	int           ThreadCount;
	int           StopFlag;

	// Synchronisation
	std::mutex              Mtx;
	std::condition_variable CV;

	// Per-batch context (set up by ParFor, read by workers under Mtx)
	struct ParForCtx *Ctx;
} ThreadPool;

// 0187: Initialise pool with N worker threads.
void ThreadPoolInit(ThreadPool *Pool, int ThreadCount);

// 0187: Destroy pool — signals stop, wakes idle threads, joins all.
void ThreadPoolDestroy(ThreadPool *Pool);

// 0187: Parallel-for: distribute [0, TaskCount) among threads.
void ThreadPoolParFor(ThreadPool *Pool,
                      int TaskCount,
                      ThreadTaskFunc Func,
                      void *Arg);

// 0187: Parallel-for with work-stealing (falls back to ParFor in cv version).
void ThreadPoolParForSteal(ThreadPool *Pool,
                           int TaskCount,
                           ThreadTaskFunc Func,
                           void *Arg,
                           int ChunkSize);
