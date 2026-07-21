/**
 * PerfBridge implementation.
 * パフォーマンスブリッジの実装ね。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include "PerfBridge.h"

void PerfRecordFrameTime(AppState *State, float FrameTimeMs, float SimTimeMs) {
	if (!State) {
		return;
	}
	float Fps = FrameTimeMs > 0.0001f ? 1000.0f / FrameTimeMs : 60.0f;
	State->Stats.FPS       = Fps;
	State->Stats.SimTimeMs = SimTimeMs;
	AppStatePushPerfSample(State, Fps, FrameTimeMs, SimTimeMs, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
}
