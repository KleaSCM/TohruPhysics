/**
 * SimRecorder — recording and playback manager (§3.42).
 * 録画と再生マネージャーね。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#pragma once

#include "../App/AppState.h"

void SimRecorderStart(AppState *State);
void SimRecorderStop(AppState *State);
void SimRecorderStep(AppState *State);
