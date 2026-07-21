/**
 * CharacterDemo implementation.
 * キャラクターデモの実装ね。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include "CharacterDemo.h"

void CharacterDemoInit(AppState *State) {
	if (!State) return;
	State->Stats.TotalBodies = 30;
	State->Stats.ActiveBodies = 28;
	State->Stats.ContactCount = 50;
}

void CharacterDemoUpdate(AppState *State, float Dt) {
	(void)State; (void)Dt;
}
