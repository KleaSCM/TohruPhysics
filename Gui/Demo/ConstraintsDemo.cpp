/**
 * ConstraintsDemo implementation.
 * 拘束デモの実装ね。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include "ConstraintsDemo.h"

void ConstraintsDemoInit(AppState *State) {
	if (!State) return;
	State->Stats.TotalBodies = 30;
	State->Stats.ConstraintCount = 35;
}

void ConstraintsDemoUpdate(AppState *State, float Dt) {
	(void)State; (void)Dt;
}
