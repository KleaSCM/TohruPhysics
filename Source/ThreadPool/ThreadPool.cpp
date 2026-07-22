/**
 * ThreadPool — work-stealing thread pool implementation.
 * TohruPhysics用のスレッドプール実装ね。
 *
 * Uses C++11 threading primitives via opaque handle storage.
 *
 * DESIGN:
 * Worker threads spin on their own deque (LIFO pop), then steal from a
 * random victim (FIFO). When no work is available they sleep_for(100us)
 * to avoid busy-wasting CPU. The main thread calls ParFor/ParForSteal,
 * pushes tasks to all deques, then helps steal until completion. Workers
 * never exit between ParFor calls.
 *
 * No condition variable — sleep_for + work re-check avoids the lost-wakeup
 * race inherent in CV-based signalling. This is acceptable for a physics
 * engine where work comes in bursts per simulation step.
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include <TohruPhysics/ThreadPool.h>
#include <TohruPhysics/Math.h>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <cstring>

// Handle accessors — opaque → typed
static inline std::thread **ThreadPtr(ThreadPool *P, int I) {
	return reinterpret_cast<std::thread **>(&P->ThreadData[I]);
}

// ---------------------------------------------------------------------------
//  Atomic helpers (GCC builtins for C-compatible atomics)
// ---------------------------------------------------------------------------

static inline int64_t Load(volatile int64_t *P) {
	return __atomic_load_n(P, __ATOMIC_ACQUIRE);
}
static inline void Store(volatile int64_t *P, int64_t V) {
	__atomic_store_n(P, V, __ATOMIC_RELEASE);
}
static inline int64_t LoadR(volatile int64_t *P) {
	return __atomic_load_n(P, __ATOMIC_RELAXED);
}
static inline void StoreR(volatile int64_t *P, int64_t V) {
	__atomic_store_n(P, V, __ATOMIC_RELAXED);
}
static inline int CAS(volatile int64_t *P, int64_t E, int64_t D) {
	return __atomic_compare_exchange_n(P, &E, D, false,
	                                   __ATOMIC_RELEASE, __ATOMIC_RELAXED);
}
static inline int LoadInt(volatile int *P) {
	return __atomic_load_n(P, __ATOMIC_ACQUIRE);
}
static inline void StoreInt(volatile int *P, int V) {
	__atomic_store_n(P, V, __ATOMIC_RELEASE);
}

// ---------------------------------------------------------------------------
//  Work-stealing deque (Chase-Lev lock-free deque)
// ---------------------------------------------------------------------------

static void DequeInit(WorkStealingDeque *DQ) {
	StoreR(&DQ->Bottom, 0);
	StoreR(&DQ->Top, 0);
}

// Owner pushes to bottom (LIFO). The deque buffer is a ring buffer
// indexed by Bottom & (CAPACITY-1). No ABA problem because only one
// thread pushes to a given deque.
static void DequePushBottom(WorkStealingDeque *DQ, int Task) {
	int64_t B = LoadR(&DQ->Bottom);
	DQ->Buffer[B & (THREADPOOL_DEQUE_CAPACITY - 1)] = Task;
	Store(&DQ->Bottom, B + 1);
}

// Owner pops from bottom (LIFO). Returns 1 on success.
static int DequePopBottom(WorkStealingDeque *DQ, int *Out) {
	int64_t B = LoadR(&DQ->Bottom) - 1;
	StoreR(&DQ->Bottom, B);
	int64_t T = Load(&DQ->Top);
	if (T <= B) {
		*Out = DQ->Buffer[B & (THREADPOOL_DEQUE_CAPACITY - 1)];
		if (T == B) {
			// Last item — race with stealers
			if (!CAS(&DQ->Top, T, T + 1)) {
				StoreR(&DQ->Bottom, B + 1);
				return 0;
			}
		}
		return 1;
	}
	StoreR(&DQ->Bottom, B + 1);
	return 0;
}

// Thief steals from top (FIFO). Returns 1 on success.
static int DequeSteal(WorkStealingDeque *DQ, int *Out) {
	int64_t T = Load(&DQ->Top);
	int64_t B = Load(&DQ->Bottom);
	if (T >= B) return 0;
	*Out = DQ->Buffer[T & (THREADPOOL_DEQUE_CAPACITY - 1)];
	return CAS(&DQ->Top, T, T + 1);
}

// ---------------------------------------------------------------------------
//  Worker entry point
// ---------------------------------------------------------------------------

static void WorkerMain(ThreadPool *Pool, int Ti) {
	// Pseudo-random seed per thread
	int Rnd = Ti * 7471969;

	while (!LoadInt(&Pool->StopFlag)) {
		// 1) Pop from own deque (LIFO — best locality)
		int TaskIdx;
		if (DequePopBottom(&Pool->Deques[Ti], &TaskIdx)) {
			Pool->CurrentFunc(Pool->CurrentArg, Ti, TaskIdx);
			__atomic_add_fetch(&Pool->TasksCompleted, 1, __ATOMIC_RELEASE);
			continue;
		}

		// 2) Steal from a random victim (FIFO — minimises contention)
		Rnd = Rnd * 1103515245 + 12345;
		int Victim = (unsigned int)Rnd % Pool->ThreadCount;
		if (Victim == Ti)
			Victim = (Victim + 1) % Pool->ThreadCount;

		if (DequeSteal(&Pool->Deques[Victim], &TaskIdx)) {
			Pool->CurrentFunc(Pool->CurrentArg, Ti, TaskIdx);
			__atomic_add_fetch(&Pool->TasksCompleted, 1, __ATOMIC_RELEASE);
			continue;
		}

		// 3) No work — brief sleep to avoid busy-waste
		//    100µs is enough to yield the core without adding perceptible
		//    latency (a physics tick is typically 8–16 ms).
		std::this_thread::sleep_for(std::chrono::microseconds(100));
	}
}

// ---------------------------------------------------------------------------
//  Init
// ---------------------------------------------------------------------------

void ThreadPoolInit(ThreadPool *Pool, int ThreadCount) {
	memset(Pool, 0, sizeof(*Pool));
	if (ThreadCount < 1) ThreadCount = 1;
	if (ThreadCount > THREADPOOL_MAX_THREADS)
		ThreadCount = THREADPOOL_MAX_THREADS;
	Pool->ThreadCount = ThreadCount;

	for (int I = 0; I < ThreadCount; I++)
		DequeInit(&Pool->Deques[I]);

	// Spawn threads
	for (int I = 0; I < ThreadCount; I++) {
		auto *T = new std::thread(WorkerMain, Pool, I);
		*ThreadPtr(Pool, I) = T;
	}

	// Wait for all threads to reach the loop body
	// (No explicit barrier needed — threads start immediately)
}

// ---------------------------------------------------------------------------
//  Destroy
// ---------------------------------------------------------------------------

void ThreadPoolDestroy(ThreadPool *Pool) {
	if (!Pool || Pool->ThreadCount == 0) return;

	StoreInt(&Pool->StopFlag, 1);

	// Workers will wake from sleep_for and see StopFlag
	for (int I = 0; I < Pool->ThreadCount; I++) {
		auto *T = *ThreadPtr(Pool, I);
		if (T && T->joinable()) T->join();
		delete T;
		*ThreadPtr(Pool, I) = nullptr;
	}

	Pool->ThreadCount = 0;
	Pool->StopFlag = 0;
}

// ---------------------------------------------------------------------------
//  ParFor — simple chunked distribution
// ---------------------------------------------------------------------------

void ThreadPoolParFor(ThreadPool *Pool,
                      int TaskCount,
                      ThreadTaskFunc Func,
                      void *Arg) {
	if (TaskCount <= 0 || !Pool || Pool->ThreadCount == 0) return;

	int NT = Pool->ThreadCount;
	int Chunk = (TaskCount + NT - 1) / NT;

	// CRITICAL: Set state BEFORE pushing tasks. Workers may wake from
	// sleep_for at any time and will read CurrentFunc/CurrentArg and
	// increment TasksCompleted. If we set these after pushing, a worker
	// could consume a task using stale state and the increment would be
	// lost when TasksCompleted is reset.
	Pool->CurrentFunc = Func;
	Pool->CurrentArg = Arg;
	Store(&Pool->TasksCompleted, 0);

	// Push chunked tasks to each deque
	for (int T = 0; T < NT; T++) {
		int S = T * Chunk;
		int E = S + Chunk;
		if (E > TaskCount) E = TaskCount;
		for (int I = S; I < E; I++)
			DequePushBottom(&Pool->Deques[T], I);
	}

	// Main thread helps steal until all tasks are done.
	// Workers wake from sleep_for (within 100 µs) and find tasks.
	while (Load(&Pool->TasksCompleted) < TaskCount) {
		int TaskIdx, Stolen = 0;
		for (int T = 0; T < NT && !Stolen; T++) {
			if (DequeSteal(&Pool->Deques[T], &TaskIdx)) {
				Func(Arg, NT, TaskIdx);
				__atomic_add_fetch(&Pool->TasksCompleted, 1,
				                   __ATOMIC_RELEASE);
				Stolen = 1;
			}
		}
		if (!Stolen)
			std::this_thread::yield();
	}
}

// ---------------------------------------------------------------------------
//  ParForSteal — fine-grained work-stealing
// ---------------------------------------------------------------------------

void ThreadPoolParForSteal(ThreadPool *Pool,
                           int TaskCount,
                           ThreadTaskFunc Func,
                           void *Arg,
                           int ChunkSize) {
	if (TaskCount <= 0 || !Pool || Pool->ThreadCount == 0) return;
	if (ChunkSize < 1) ChunkSize = 1;

	int NT = Pool->ThreadCount;

	// Set state BEFORE pushing (see ParFor for rationale)
	Pool->CurrentFunc = Func;
	Pool->CurrentArg = Arg;
	Store(&Pool->TasksCompleted, 0);

	// Spread fine-grained chunks across deques for work-stealing.
	// Round-robin distribution ensures no deque gets all the tasks.
	for (int T = 0; T < NT; T++) {
		for (int I = T * ChunkSize; I < TaskCount; I += NT * ChunkSize) {
			int E = I + ChunkSize;
			if (E > TaskCount) E = TaskCount;
			for (int J = I; J < E; J++)
				DequePushBottom(&Pool->Deques[T], J);
		}
	}

	// Main thread helps steal until all tasks are done
	while (Load(&Pool->TasksCompleted) < TaskCount) {
		int TaskIdx, Stolen = 0;
		for (int T = 0; T < NT && !Stolen; T++) {
			if (DequeSteal(&Pool->Deques[T], &TaskIdx)) {
				Func(Arg, NT, TaskIdx);
				__atomic_add_fetch(&Pool->TasksCompleted, 1,
				                   __ATOMIC_RELEASE);
				Stolen = 1;
			}
		}
		if (!Stolen)
			std::this_thread::yield();
	}
}
