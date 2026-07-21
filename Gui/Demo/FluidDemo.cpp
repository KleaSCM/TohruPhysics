/**
 * FluidDemo implementation.
 * SPH流体デモの実装ね。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include "FluidDemo.h"

void FluidDemoInit(AppState *State) {
	if (!State) return;
	State->Stats.TotalBodies = 1000;
	State->Stats.ActiveBodies = 1000;
}

void FluidDemoUpdate(AppState *State, float Dt) {
	(void)State; (void)Dt;
}
