/**
 * SceneManager implementation.
 * シーンマネージャーの実装ね。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include "SceneManager.h"
#include "SceneSerializer.h"
#include "SceneJson.h"

void SceneManagerReset(AppState *State) {
	if (!State) return;
	AppStateApplyDefaults(State);
	State->SceneDirty = 0;
}

int SceneManagerSave(AppState *State, const char *FilePath, int IsJson) {
	if (!State || !FilePath) return 0;
	int Res = IsJson ? SceneSaveJson(State, FilePath) : SceneSaveBinary(State, FilePath);
	if (Res) {
		State->SceneDirty = 0;
	}
	return Res;
}

int SceneManagerLoad(AppState *State, const char *FilePath, int IsJson) {
	if (!State || !FilePath) return 0;
	int Res = IsJson ? SceneLoadJson(State, FilePath) : SceneLoadBinary(State, FilePath);
	if (Res) {
		State->SceneDirty = 0;
	}
	return Res;
}
