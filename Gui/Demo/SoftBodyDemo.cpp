/**
 * SoftBodyDemo implementation.
 * 柔軟体デモの実装ね。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include "SoftBodyDemo.h"

void SoftBodyDemoInit(AppState *State) {
	if (!State) return;
	State->Stats.TotalBodies = 400;
	State->Stats.ActiveBodies = 400;
	State->Stats.ConstraintCount = 600;
}

void SoftBodyDemoUpdate(AppState *State, float Dt) {
	(void)State; (void)Dt;
}
