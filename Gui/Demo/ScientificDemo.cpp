/**
 * ScientificDemo implementation.
 * サイエンティフィックデモの実装ね。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include "ScientificDemo.h"

void ScientificDemoInit(AppState *State) {
	if (!State) return;
	State->Stats.TotalBodies = 150;
	State->Stats.ActiveBodies = 150;
	State->Stats.ContactCount = 300;
}

void ScientificDemoUpdate(AppState *State, float Dt) {
	(void)State; (void)Dt;
}
