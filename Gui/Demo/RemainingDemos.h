/**
 * Remaining demo scenes header definitions (§3.22–§3.26).
 * 残りのデモシーンのヘッダー定義ね。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#pragma once
#include "../App/AppState.h"

void DestructionDemoInit(AppState *State);
void DestructionDemoUpdate(AppState *State, float Dt);

void VehicleDemoInit(AppState *State);
void VehicleDemoUpdate(AppState *State, float Dt);

void CharacterDemoInit(AppState *State);
void CharacterDemoUpdate(AppState *State, float Dt);

void MultiPhysicsDemoInit(AppState *State);
void MultiPhysicsDemoUpdate(AppState *State, float Dt);

void ScientificDemoInit(AppState *State);
void ScientificDemoUpdate(AppState *State, float Dt);
