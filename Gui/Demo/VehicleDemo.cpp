/**
 * VehicleDemo implementation.
 * 車両デモの実装ね。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include "VehicleDemo.h"

void VehicleDemoInit(AppState *State) {
	if (!State) return;
	State->Stats.TotalBodies = 40;
	State->Stats.ActiveBodies = 36;
	State->Stats.ContactCount = 80;
}

void VehicleDemoUpdate(AppState *State, float Dt) {
	(void)State; (void)Dt;
}
