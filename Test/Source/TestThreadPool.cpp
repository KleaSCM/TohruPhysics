/**
 * Unit tests for ThreadPool work-stealing thread pool.
 * ThreadPoolの単体テストね。
 *
 * Tests parallel-for correctness, work-stealing with uneven tasks,
 * and stress test with many small tasks.
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include <TohruPhysics/ThreadPool.h>
#include <TohruPhysics/Math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Use <atomic> for test-side counters only (not in ThreadPool API)
#include <atomic>

#define TEST(cond, msg) do { \
	if (!(cond)) { \
		fprintf(stderr, "FAIL: %s (%s:%d)\n", msg, __FILE__, __LINE__); \
		exit(1); \
	} \
} while(0)

static int Passed = 0;

#define RUN_TEST(name, desc) do { \
	fprintf(stderr, "  %-45s ... ", desc); \
	name(); \
	fprintf(stderr, "ok\n"); \
	Passed++; \
} while(0)

// ===========================================================================
//  Test data for parallel tasks
// ===========================================================================

static std::atomic<int> G_AtomicCounter{0};
static std::atomic<int> G_TaskResults[1024];

static void ResetResults(int N) {
	G_AtomicCounter.store(0, std::memory_order_relaxed);
	for (int I = 0; I < N; I++)
		G_TaskResults[I].store(0, std::memory_order_relaxed);
}

// Simple increment task
static void IncrementTask(void *Arg, int ThreadIndex, int TaskIndex) {
	(void)Arg;
	(void)ThreadIndex;
	G_AtomicCounter.fetch_add(1, std::memory_order_relaxed);
	G_TaskResults[TaskIndex].store(1, std::memory_order_relaxed);
}

// Sum task: adds TaskIndex to a shared counter
static void SumTask(void *Arg, int ThreadIndex, int TaskIndex) {
	(void)ThreadIndex;
	int *SumPtr = (int *)Arg;
	__atomic_add_fetch(SumPtr, TaskIndex, __ATOMIC_RELAXED);
}

// Slow task: spins briefly to trigger work-stealing
static void SlowTask(void *Arg, int ThreadIndex, int TaskIndex) {
	(void)Arg;
	(void)ThreadIndex;
	volatile int Dummy = 0;
	for (int I = 0; I < 10000; I++) Dummy += I;
	G_TaskResults[TaskIndex].store(1, std::memory_order_relaxed);
	(void)Dummy;
}

// ===========================================================================
//  Tests
// ===========================================================================

// Init/destroy lifecycle
static void TestInitDestroy(void) {
	ThreadPool Pool;
	ThreadPoolInit(&Pool, 2);
	TEST(Pool.ThreadCount == 2, "pool has 2 threads");
	ThreadPoolDestroy(&Pool);
	TEST(Pool.ThreadCount == 0, "pool destroyed — count zero");
}

// ParFor with many small tasks
static void TestParForSimple(void) {
	ThreadPool Pool;
	ThreadPoolInit(&Pool, 4);

	const int N = 100;
	ResetResults(N);

	ThreadPoolParFor(&Pool, N, IncrementTask, nullptr);

	// Verify all tasks executed exactly once
	for (int I = 0; I < N; I++) {
		TEST(G_TaskResults[I].load(std::memory_order_relaxed) == 1,
		     "ParFor: all tasks executed");
	}
	TEST(G_AtomicCounter.load(std::memory_order_relaxed) == N,
	     "ParFor: atomic counter matches task count");

	ThreadPoolDestroy(&Pool);
}

// ParFor with sum accumulation
static void TestParForSum(void) {
	ThreadPool Pool;
	ThreadPoolInit(&Pool, 4);

	const int N = 50;
	int Sum = 0;
	ThreadPoolParFor(&Pool, N, SumTask, &Sum);

	int Expected = N * (N - 1) / 2;
	TEST(Sum == Expected, "ParForSum: sum of indices correct");

	ThreadPoolDestroy(&Pool);
}

// ParFor with single thread
static void TestParForSingleThread(void) {
	ThreadPool Pool;
	ThreadPoolInit(&Pool, 1);

	const int N = 20;
	ResetResults(N);

	ThreadPoolParFor(&Pool, N, IncrementTask, nullptr);

	for (int I = 0; I < N; I++) {
		TEST(G_TaskResults[I].load(std::memory_order_relaxed) == 1,
		     "ParFor single: all tasks executed");
	}

	ThreadPoolDestroy(&Pool);
}

// ParForSteal with fine-grained tasks triggers stealing
static void TestParForStealBasic(void) {
	ThreadPool Pool;
	ThreadPoolInit(&Pool, 4);

	const int N = 200;
	ResetResults(N);

	ThreadPoolParForSteal(&Pool, N, SlowTask, nullptr, 1);

	for (int I = 0; I < N; I++) {
		TEST(G_TaskResults[I].load(std::memory_order_relaxed) == 1,
		     "ParForSteal: all tasks executed");
	}

	ThreadPoolDestroy(&Pool);
}

// Empty task list — should not crash
static void TestEmptyTaskList(void) {
	ThreadPool Pool;
	ThreadPoolInit(&Pool, 2);

	ThreadPoolParFor(&Pool, 0, IncrementTask, nullptr);
	ThreadPoolParForSteal(&Pool, 0, IncrementTask, nullptr, 1);

	ThreadPoolDestroy(&Pool);
}

// Stress: many small tasks across many threads
static void TestStressManyTasks(void) {
	ThreadPool Pool;
	ThreadPoolInit(&Pool, 8);

	const int N = 500;
	ResetResults(N);

	ThreadPoolParForSteal(&Pool, N, SlowTask, nullptr, 1);

	int Executed = 0;
	for (int I = 0; I < N; I++) {
		if (G_TaskResults[I].load(std::memory_order_relaxed) == 1)
			Executed++;
	}
	TEST(Executed == N, "Stress: all tasks executed");

	ThreadPoolDestroy(&Pool);
}

// Sequential ParFor calls (reuse pool)
// Uses a persistent global accumulator to avoid races where stale
// worker tasks from a previous batch write to a recycled stack address.
static int G_RepeatedSum = 0;

static void SumTaskPersist(void *Arg, int ThreadIndex, int TaskIndex) {
	(void)Arg;
	(void)ThreadIndex;
	__atomic_add_fetch(&G_RepeatedSum, TaskIndex, __ATOMIC_RELAXED);
}

static void TestRepeatedParFor(void) {
	ThreadPool Pool;
	ThreadPoolInit(&Pool, 4);

	G_RepeatedSum = 0;
	for (int Run = 0; Run < 5; Run++) {
		const int N = 40;
		ThreadPoolParFor(&Pool, N, SumTaskPersist, NULL);
	}
	int Expected = 5 * 40 * (40 - 1) / 2;
	TEST(G_RepeatedSum == Expected, "Repeated ParFor: sum correct");

	ThreadPoolDestroy(&Pool);
}

// ===========================================================================
//  Main
// ===========================================================================

int main(void) {
	fprintf(stderr, "=== TestThreadPool ===\n");

	RUN_TEST(TestInitDestroy, "ThreadPool: init/destroy");
	RUN_TEST(TestParForSimple, "ThreadPool: ParFor simple");
	RUN_TEST(TestParForSum, "ThreadPool: ParFor sum");
	RUN_TEST(TestParForSingleThread, "ThreadPool: ParFor single thread");
	RUN_TEST(TestParForStealBasic, "ThreadPool: ParForSteal basic");
	RUN_TEST(TestEmptyTaskList, "ThreadPool: empty task list");
	RUN_TEST(TestStressManyTasks, "ThreadPool: stress many tasks");
	RUN_TEST(TestRepeatedParFor, "ThreadPool: repeated ParFor");

	fprintf(stderr, "\n=== %d passed, 0 failed ===\n", Passed);
	return 0;
}
