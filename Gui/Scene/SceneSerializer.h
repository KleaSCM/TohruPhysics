/**
 * SceneSerializer — binary scene file format save/load (§3.13).
 * バイナリシーンファイルフォーマットの保存／読み込みね。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#pragma once

#include "../App/AppState.h"

int SceneSaveBinary(const AppState *State, const char *FilePath);
int SceneLoadBinary(AppState *State, const char *FilePath);
