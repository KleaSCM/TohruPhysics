/**
 * SceneJson implementation.
 * JSONシーンの実装ね。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include "SceneJson.h"

#include <stdio.h>

int SceneSaveJson(const AppState *State, const char *FilePath) {
	if (!State || !FilePath) return 0;

	FILE *F = fopen(FilePath, "w");
	if (!F) return 0;

	fprintf(F, "{\n");
	fprintf(F, "  \"version\": 1,\n");
	fprintf(F, "  \"gravity\": [%.3f, %.3f, %.3f],\n", State->Physics.GravityX, State->Physics.GravityY, State->Physics.GravityZ);
	fprintf(F, "  \"restitution\": %.3f,\n", State->Physics.GlobalRestitution);
	fprintf(F, "  \"friction\": %.3f\n", State->Physics.StaticFriction);
	fprintf(F, "}\n");

	fclose(F);
	return 1;
}

int SceneLoadJson(AppState *State, const char *FilePath) {
	if (!State || !FilePath) return 0;

	FILE *F = fopen(FilePath, "r");
	if (!F) return 0;

	float Gx = 0, Gy = -9.81f, Gz = 0;
	float Rest = 0.3f, Fric = 0.5f;

	// Simple parser for key fields
	char Buffer[256];
	while (fgets(Buffer, sizeof(Buffer), F)) {
		if (sscanf(Buffer, " \"restitution\": %f,", &Rest) == 1) {
			State->Physics.GlobalRestitution = Rest;
		} else if (sscanf(Buffer, " \"friction\": %f", &Fric) == 1) {
			State->Physics.StaticFriction = Fric;
		}
	}

	fclose(F);
	return 1;
}
