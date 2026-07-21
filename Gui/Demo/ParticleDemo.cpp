/**
 * ParticleDemo implementation.
 * パーティクルデモの実装ね。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include "ParticleDemo.h"

void ParticleDemoInit(AppState *State) {
	if (!State) return;
	State->Stats.TotalBodies = 2000;
	State->Stats.ActiveBodies = 2000;
}

void ParticleDemoUpdate(AppState *State, float Dt) {
	(void)State; (void)Dt;
}
