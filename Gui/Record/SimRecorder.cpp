/**
 * SimRecorder implementation.
 * シミュレーションレコーダーの実装ね。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include "SimRecorder.h"

void SimRecorderStart(AppState *State) {
	if (!State) return;
	State->Record.Mode = RecordMode_Recording;
	State->Record.FrameCount = 0;
	State->Record.CurrentFrame = 0;
}

void SimRecorderStop(AppState *State) {
	if (!State) return;
	State->Record.Mode = RecordMode_Idle;
}

void SimRecorderStep(AppState *State) {
	if (!State) return;
	if (State->Record.Mode == RecordMode_Recording) {
		State->Record.FrameCount++;
		State->Record.CurrentFrame++;
	} else if (State->Record.Mode == RecordMode_Playing) {
		if (State->Record.CurrentFrame < State->Record.FrameCount) {
			State->Record.CurrentFrame++;
		} else if (State->Record.Loop) {
			State->Record.CurrentFrame = 0;
		}
	}
}
