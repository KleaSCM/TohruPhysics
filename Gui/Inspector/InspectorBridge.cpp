/**
 * InspectorBridge implementation.
 * インスペクターブリッジの実装ね。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include "InspectorBridge.h"

void InspectorSelectBody(AppState *State, int64_t BodyId, float X, float Y, float Z, float Mass) {
	if (!State) {
		return;
	}
	State->Inspector.Primary.BodyId = BodyId;
	State->Inspector.Primary.PosX   = X;
	State->Inspector.Primary.PosY   = Y;
	State->Inspector.Primary.PosZ   = Z;
	State->Inspector.Primary.Mass   = Mass;
	State->Inspector.SelectionCount  = 1;
}

void InspectorClearSelection(AppState *State) {
	if (!State) {
		return;
	}
	State->Inspector.Primary.BodyId = -1;
	State->Inspector.SelectionCount  = 0;
}
