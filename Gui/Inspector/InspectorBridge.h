/**
 * InspectorBridge — body inspector helper functions.
 * ボディインスペクターのヘルパー関数ね。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#pragma once

#include "../App/AppState.h"

void InspectorSelectBody(AppState *State, int64_t BodyId, float X, float Y, float Z, float Mass);
void InspectorClearSelection(AppState *State);
