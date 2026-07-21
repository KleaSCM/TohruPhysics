/**
 * SceneJson — JSON scene save/load without external dependencies (§3.13).
 * 外部依存なしのJSONシーン保存／読み込みね。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#pragma once

#include "../App/AppState.h"

int SceneSaveJson(const AppState *State, const char *FilePath);
int SceneLoadJson(AppState *State, const char *FilePath);
