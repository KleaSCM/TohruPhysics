/**
 * RigidStackDemo implementation.
 * 剛体スタックデモの実装ね。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include "RigidStackDemo.h"

void RigidStackDemoInit(AppState *State) {
	if (!State) return;
	State->Stats.TotalBodies = 100;
	State->Stats.ActiveBodies = 100;
	State->Stats.ContactCount = 250;
}

void RigidStackDemoUpdate(AppState *State, float Dt) {
	(void)State; (void)Dt;
}
