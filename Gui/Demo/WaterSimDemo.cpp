/**
 * WaterSimDemo implementation.
 * 水面シミュレーションデモの実装ね。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include "WaterSimDemo.h"

void WaterSimDemoInit(AppState *State) {
	if (!State) return;
	State->Stats.TotalBodies = 50;
	State->Stats.ActiveBodies = 50;
	State->Stats.ContactCount = 120;
}

void WaterSimDemoUpdate(AppState *State, float Dt) {
	(void)State; (void)Dt;
}
