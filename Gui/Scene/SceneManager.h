/**
 * SceneManager — high-level scene operations manager (§3.13).
 * ハイレベルシーン操作マネージャーね。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#pragma once

#include "../App/AppState.h"

void SceneManagerReset(AppState *State);
int  SceneManagerSave(AppState *State, const char *FilePath, int IsJson);
int  SceneManagerLoad(AppState *State, const char *FilePath, int IsJson);
