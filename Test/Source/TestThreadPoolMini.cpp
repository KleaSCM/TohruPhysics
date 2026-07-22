/**
 * Debug v2: verify each task executes.
 */
#include <TohruPhysics/ThreadPool.h>
#include <stdio.h>
#include <stdlib.h>

static volatile int G_Marks[100];

static void MarkTask(void *Arg, int Ti, int Idx) {
	(void)Arg;
	(void)Ti;
	if (Idx >= 0 && Idx < 100)
		G_Marks[Idx] = 1;
}

int main(void) {
	ThreadPool Pool;
	ThreadPoolInit(&Pool, 2);

	for (int Run = 0; Run < 3; Run++) {
		int N = 10;
		for (int I = 0; I < N; I++) G_Marks[I] = 0;

		fprintf(stderr, "Run %d: start ParFor(N=%d)...\n", Run, N);
		ThreadPoolParFor(&Pool, N, MarkTask, nullptr);

		int Missed = 0;
		for (int I = 0; I < N; I++) {
			if (!G_Marks[I]) {
				fprintf(stderr, "  MISS: task %d not executed\n", I);
				Missed++;
			}
		}
		fprintf(stderr, "Run %d: %d/%d executed (%d missed)\n",
			Run, N - Missed, N, Missed);
		if (Missed) exit(1);
	}

	ThreadPoolDestroy(&Pool);
	fprintf(stderr, "All OK\n");
	return 0;
}
