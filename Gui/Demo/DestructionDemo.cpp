/**
 * DestructionDemo implementation.
 * 破壊デモの実装ね。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include "DestructionDemo.h"

void DestructionDemoInit(AppState *State) {
	if (!State) return;
	State->Stats.TotalBodies = 150;
	State->Stats.ActiveBodies = 150;
}

void DestructionDemoUpdate(AppState *State, float Dt) {
	(void)State; (void)Dt;
}
