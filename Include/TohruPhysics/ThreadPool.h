/**
 * ThreadPool — work-stealing thread pool for parallel task execution.
 * TohruPhysics用のワークスティーリングスレッドプールね。
 *
 * Provides parallel-for with adaptive work-stealing for load-balanced
 * execution across heterogeneous tasks.
 *
 * DESIGN PHILOSOPHY:
 * Each worker thread owns a lock-free work-stealing deque (Top/Bottom
 * atomic indices over a fixed ring buffer). Threads push/pop from their
 * own deque bottom (LIFO). When empty, they steal from a random victim's
 * deque top (FIFO), minimising contention. When no work is available,
 * workers sleep_for(100 µs) instead of busy-waiting.
 *
 * References:
 * - Chase & Lev, "Dynamic Circular Work-Stealing Deque" (2005)
 * - Intel TBB work-stealing scheduler
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#pragma once

#include <cstddef>
#include <cstdint>

#define THREADPOOL_MAX_THREADS 64
#define THREADPOOL_DEFAULT_THREADS 4
#define THREADPOOL_DEQUE_CAPACITY 4096

// Task function signature: Func(Arg, ThreadIndex, TaskIndex)
typedef void (*ThreadTaskFunc)(void *Arg, int ThreadIndex, int TaskIndex);

// 0187: Per-thread work-stealing deque (lock-free ring buffer).
typedef struct {
	int        Buffer[THREADPOOL_DEQUE_CAPACITY];
	int64_t    Bottom;
	int64_t    Top;
} WorkStealingDeque;

// 0187: Thread pool state.
// ThreadPoolInit must be called before use; zero-initialised state is safe
// to destroy (all pointers null → no-op).
typedef struct {
	// Opaque handle storage — implementation manages std::thread objects.
	// Zero-initialised = no threads.
	void         *ThreadData[THREADPOOL_MAX_THREADS];
	int           ThreadCount;
	int           StopFlag;

	// Current parallel-for task
	ThreadTaskFunc CurrentFunc;
	void          *CurrentArg;
	int            TotalTasks;
	int64_t        TasksCompleted;

	// Work-stealing deques
	WorkStealingDeque Deques[THREADPOOL_MAX_THREADS];
} ThreadPool;

// 0187: Initialise pool with N worker threads.
void ThreadPoolInit(ThreadPool *Pool, int ThreadCount);

// 0187: Destroy pool — signals stop, wakes idle threads, joins all.
void ThreadPoolDestroy(ThreadPool *Pool);

// 0187: Parallel-for: divide [0, TaskCount) equally among threads.
void ThreadPoolParFor(ThreadPool *Pool,
                      int TaskCount,
                      ThreadTaskFunc Func,
                      void *Arg);

// 0187: Parallel-for with work-stealing for dynamic load balancing.
// Tasks ≤ ChunkSize use direct assignment; larger sets use work-stealing.
void ThreadPoolParForSteal(ThreadPool *Pool,
                           int TaskCount,
                           ThreadTaskFunc Func,
                           void *Arg,
                           int ChunkSize);
