/**
 * ThreadPool — mutex-based thread pool implementation.
 * TohruPhysics用のスレッドプール実装ね。
 *
 * Uses a shared task queue protected by a mutex + condition variable.
 * Workers and the calling thread all draw from the same queue, so no
 * task can be duplicated or lost.  No lock-free work-stealing deque.
 *
 * DESIGN:
 * Workers and the main thread share a single FIFO queue of task indices
 * protected by a mutex.  The main thread pushes all tasks, then helps
 * execute them alongside workers.  When the queue is empty and all
 * tasks are completed, ParFor returns.
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include <TohruPhysics/ThreadPool.h>
#include <TohruPhysics/Math.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <cstdlib>

// Handle accessors — opaque → typed
// 不透明ハンドル → 型付きポインタ
static inline std::thread **ThreadPtr(ThreadPool *P, int I) {
	return reinterpret_cast<std::thread **>(&P->ThreadData[I]);
}

// ---------------------------------------------------------------------------
//  Worker entry point
// ---------------------------------------------------------------------------

// Each ParFor batch uses a shared context on the pool.  The workers read
// the context fields after being notified.  The pool's mutex serialises
// all access.
// 各ParForバッチはプール上の共有コンテキストを使う。ワーカーは通知を受けて
// コンテキストフィールドを読み取る。プールのミューテックスが全てのアクセスを
// 直列化する。
struct ParForCtx {
	ThreadTaskFunc Func;
	void          *Arg;
	int            TaskCount;
	int            TotalTasks;
	int            NextTask;   // next unclaimed task index (0187)
	int            Completed;  // count of completed tasks (0187)
};

static void WorkerMain(ThreadPool *Pool, int Ti) {
	(void)Ti;
	while (true) {
		int TaskIdx;
		ParForCtx *MyCtx;
		{
			std::unique_lock<std::mutex> Lock(Pool->Mtx);
			Pool->CV.wait(Lock, [Pool]{
				return Pool->StopFlag ||
				       (Pool->Ctx && Pool->Ctx->NextTask < Pool->Ctx->TotalTasks);
			});
			if (Pool->StopFlag) return;
			// Capture the context pointer while under lock, then use
			// the local for the rest of the batch.  This is safe even
			// if the main thread later nulls Pool->Ctx.
			// ロック中にコンテキストポインタをローカルに保存。メインスレッドが
			// 後で Pool->Ctx を null にしても安全。
			MyCtx = Pool->Ctx;
			TaskIdx = MyCtx->NextTask++;
		}
		// Execute (outside lock so workers can run in parallel)
		// 実行（ロック解除 — ワーカーは並列に実行できる）
		MyCtx->Func(MyCtx->Arg, Ti, TaskIdx);
		{
			std::lock_guard<std::mutex> Lock(Pool->Mtx);
			MyCtx->Completed++;
			if (MyCtx->Completed >= MyCtx->TotalTasks)
				Pool->CV.notify_all(); // wake main (may be waiting)
		}
	}
}

// ---------------------------------------------------------------------------
//  Init
// ---------------------------------------------------------------------------

void ThreadPoolInit(ThreadPool *Pool, int ThreadCount) {
	// Zero C-style members; C++ members (Mtx, CV) are already constructed
	// by their default initialisers when the Pool was declared.
	// Cスタイルメンバーをゼロ初期化；C++メンバーは宣言時のデフォルト初期化済み。
	Pool->ThreadCount = 0;
	Pool->StopFlag    = 0;
	Pool->Ctx         = nullptr;
	for (int I = 0; I < THREADPOOL_MAX_THREADS; I++)
		Pool->ThreadData[I] = nullptr;

	if (ThreadCount < 1) ThreadCount = 1;
	if (ThreadCount > THREADPOOL_MAX_THREADS)
		ThreadCount = THREADPOOL_MAX_THREADS;
	Pool->ThreadCount = ThreadCount;

	// Spawn threads
	for (int I = 0; I < ThreadCount; I++) {
		auto *T = new std::thread(WorkerMain, Pool, I);
		*ThreadPtr(Pool, I) = T;
	}
}

// ---------------------------------------------------------------------------
//  Destroy
// ---------------------------------------------------------------------------

void ThreadPoolDestroy(ThreadPool *Pool) {
	if (!Pool || Pool->ThreadCount == 0) return;

	{
		std::lock_guard<std::mutex> Lock(Pool->Mtx);
		Pool->StopFlag = 1;
	}
	Pool->CV.notify_all();

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
//  ParFor — shared-queue parallel-for
// ---------------------------------------------------------------------------

void ThreadPoolParFor(ThreadPool *Pool,
                      int TaskCount,
                      ThreadTaskFunc Func,
                      void *Arg) {
	if (TaskCount <= 0 || !Pool || Pool->ThreadCount == 0) return;

	// Set up context on the pool (worker threads will read it)
	// プール上にコンテキストを設定（ワーカースレッドが読み取る）
	ParForCtx Ctx;
	Ctx.Func       = Func;
	Ctx.Arg        = Arg;
	Ctx.TaskCount  = TaskCount;
	Ctx.TotalTasks = TaskCount;
	Ctx.NextTask   = 0;
	Ctx.Completed  = 0;

	{
		std::lock_guard<std::mutex> Lock(Pool->Mtx);
		Pool->Ctx = &Ctx;
	}
	Pool->CV.notify_all();  // wake workers — they'll read Ctx under the mutex

	// Main thread also participates: grab tasks from the queue
	// メインスレッドも参加：キューからタスクを取得
	while (true) {
		int TaskIdx;
		{
			std::unique_lock<std::mutex> Lock(Pool->Mtx);
			// If all done, break
			if (Ctx.Completed >= TaskCount) break;
			// If queue is empty AND all tasks have been claimed, wait
			if (Ctx.NextTask >= TaskCount) {
				Pool->CV.wait(Lock, [&Ctx, TaskCount]{
					return Ctx.Completed >= TaskCount;
				});
				break;
			}
			// Claim next task
			TaskIdx = Ctx.NextTask++;
		}
		// Execute (outside lock)
		Func(Arg, Pool->ThreadCount, TaskIdx);
		{
			std::lock_guard<std::mutex> Lock(Pool->Mtx);
			Ctx.Completed++;
		}
	}

	// All tasks complete — ensure workers exit the batch
	// 全タスク完了 — ワーカーがこのバッチを抜けるのを確認
	{
		std::lock_guard<std::mutex> Lock(Pool->Mtx);
		Pool->Ctx = nullptr;
	}
	// No need to notify — workers will recheck Ctx and sleep
	// 通知不要 — ワーカーはCtxを再チェックして待機
}

// ---------------------------------------------------------------------------
//  ParForSteal — fine-grained work-stealing (not supported in mutex version)
//  Falls back to regular ParFor.
// ---------------------------------------------------------------------------

void ThreadPoolParForSteal(ThreadPool *Pool,
                           int TaskCount,
                           ThreadTaskFunc Func,
                           void *Arg,
                           int ChunkSize) {
	(void)ChunkSize;
	ThreadPoolParFor(Pool, TaskCount, Func, Arg);
}
