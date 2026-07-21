/**
 * PerfBridge — performance monitoring helper functions.
 * パフォーマンス監視のヘルパー関数ね。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#pragma once

#include "../App/AppState.h"

void PerfRecordFrameTime(AppState *State, float FrameTimeMs, float SimTimeMs);
