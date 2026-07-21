/**
 * SceneSerializer implementation.
 * シーンシリアライザーの実装ね。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include "SceneSerializer.h"

#include <stdio.h>

#define SCENE_MAGIC 0x544F4852 /* 'TOHR' */

struct SceneHeader {
	uint32_t Magic;
	uint32_t Version;
	int32_t  BodyCount;
};

int SceneSaveBinary(const AppState *State, const char *FilePath) {
	if (!State || !FilePath) return 0;

	FILE *F = fopen(FilePath, "wb");
	if (!F) return 0;

	SceneHeader Header = { SCENE_MAGIC, 1, State->Stats.TotalBodies };
	fwrite(&Header, sizeof(SceneHeader), 1, F);
	fwrite(&State->Physics, sizeof(PhysicsParams), 1, F);

	fclose(F);
	return 1;
}

int SceneLoadBinary(AppState *State, const char *FilePath) {
	if (!State || !FilePath) return 0;

	FILE *F = fopen(FilePath, "rb");
	if (!F) return 0;

	SceneHeader Header = {0};
	if (fread(&Header, sizeof(SceneHeader), 1, F) != 1) {
		fclose(F);
		return 0;
	}

	if (Header.Magic != SCENE_MAGIC) {
		fclose(F);
		return 0;
	}

	if (fread(&State->Physics, sizeof(PhysicsParams), 1, F) != 1) {
		fclose(F);
		return 0;
	}

	fclose(F);
	return 1;
}
