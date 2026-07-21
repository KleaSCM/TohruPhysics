/**
 * HeadlessMain — headless demo runner for automated testing.
 * 自動テスト用のヘッドレスデモランナーね。
 *
 * DESIGN PHILOSOPHY:
 * No display, no OpenGL, no Slint event loop. Runs a simulation demo
 * for a fixed number of frames, prints periodic stats to stdout, and
 * returns. Useful for benchmarking, regression testing, and CI.
 *
 * LIFECYCLE:
 * 1. Init AppState (zero + defaults)
 * 2. Call demo Init (populates scene within AppState)
 * 3. Loop for N frames, calling demo Update + output
 * 4. Exit
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include "AppState.h"
#include "Demo/RigidStackDemo.h"

#include <cstdio>

int main() {
	AppState State;
	AppStateInit(&State);
	AppStateApplyDefaults(&State);

	/* Load a demo scene */
	RigidStackDemoInit(&State);

	const int   TotalFrames = 10000;
	const float Dt          = 1.0f / 60.0f;
	const int   ReportEvery = 1000;

	for (int Frame = 0; Frame < TotalFrames; Frame++) {
		RigidStackDemoUpdate(&State, Dt);

		if (Frame % ReportEvery == 0) {
			printf("[Frame %5d / %d] Bodies=%d  Active=%d  Contacts=%d"
				"  FPS=%.1f\n",
				Frame, TotalFrames,
				State.Stats.TotalBodies,
				State.Stats.ActiveBodies,
				State.Stats.ContactCount,
				State.Stats.FPS);
		}
	}

	printf("Done. %d frames simulated.\n", TotalFrames);
	return 0;
}
