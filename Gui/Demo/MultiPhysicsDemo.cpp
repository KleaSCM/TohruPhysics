/**
 * MultiPhysicsDemo implementation.
 * マルチフィジクスデモの実装ね。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include "MultiPhysicsDemo.h"

void MultiPhysicsDemoInit(AppState *State) {
	if (!State) return;
	State->Stats.TotalBodies = 200;
	State->Stats.ActiveBodies = 180;
	State->Stats.ContactCount = 500;
}

void MultiPhysicsDemoUpdate(AppState *State, float Dt) {
	(void)State; (void)Dt;
}
