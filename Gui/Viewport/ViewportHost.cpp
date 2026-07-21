/**
 * ViewportHost implementation.
 * ビューポートホストの実装ね。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include "ViewportHost.h"

#include <math.h>

static const char *GridVertShader =
	"#version 330 core\n"
	"layout (location = 0) in vec3 aPos;\n"
	"uniform mat4 uView;\n"
	"uniform mat4 uProjection;\n"
	"void main() {\n"
	"    gl_Position = uProjection * uView * vec4(aPos, 1.0);\n"
	"}\n";

static const char *GridFragShader =
	"#version 330 core\n"
	"out vec4 FragColor;\n"
	"uniform vec4 uColor;\n"
	"void main() {\n"
	"    FragColor = uColor;\n"
	"}\n";

static const char *DebugVertShader =
	"#version 330 core\n"
	"layout (location = 0) in vec3 aPos;\n"
	"layout (location = 1) in vec3 aColor;\n"
	"out vec3 VertColor;\n"
	"uniform mat4 uView;\n"
	"uniform mat4 uProjection;\n"
	"void main() {\n"
	"    VertColor = aColor;\n"
	"    gl_Position = uProjection * uView * vec4(aPos, 1.0);\n"
	"}\n";

static const char *DebugFragShader =
	"#version 330 core\n"
	"out vec4 FragColor;\n"
	"in vec3 VertColor;\n"
	"void main() {\n"
	"    FragColor = vec4(VertColor, 1.0);\n"
	"}\n";

void ViewportHostInit(ViewportHost *Host) {
	if (!Host) return;
	GridRendererInit(&Host->Grid, GridVertShader, GridFragShader);
	DebugRendererInit(&Host->Debug, DebugVertShader, DebugFragShader);
	Host->Initialized = 1;
}

void ViewportHostDestroy(ViewportHost *Host) {
	if (!Host || !Host->Initialized) return;
	GridRendererDestroy(&Host->Grid);
	DebugRendererDestroy(&Host->Debug);
	Host->Initialized = 0;
}

static void MakePerspective(float *M, float FovDeg, float Aspect, float Near, float Far) {
	float Rad = FovDeg * (3.14159265f / 180.0f);
	float TanHalfFov = tanf(Rad / 2.0f);

	for (int i = 0; i < 16; ++i) M[i] = 0.0f;

	M[0] = 1.0f / (Aspect * TanHalfFov);
	M[5] = 1.0f / TanHalfFov;
	M[10] = -(Far + Near) / (Far - Near);
	M[11] = -1.0f;
	M[14] = -(2.0f * Far * Near) / (Far - Near);
}

static void MakeLookAt(float *M, float EyeX, float EyeY, float EyeZ, float TargetX, float TargetY, float TargetZ) {
	float Fx = TargetX - EyeX;
	float Fy = TargetY - EyeY;
	float Fz = TargetZ - EyeZ;
	float LenF = sqrtf(Fx*Fx + Fy*Fy + Fz*Fz);
	if (LenF > 0.0001f) { Fx /= LenF; Fy /= LenF; Fz /= LenF; }

	// Up = (0, 1, 0)
	float Rx = Fy * 0.0f - Fz * 1.0f;
	float Ry = Fz * 0.0f - Fx * 0.0f;
	float Rz = Fx * 1.0f - Fy * 0.0f;
	float LenR = sqrtf(Rx*Rx + Ry*Ry + Rz*Rz);
	if (LenR > 0.0001f) { Rx /= LenR; Ry /= LenR; Rz /= LenR; }

	float Ux = Ry * Fz - Rz * Fy;
	float Uy = Rz * Fx - Rx * Fz;
	float Uz = Rx * Fy - Ry * Fx;

	M[0] = Rx; M[4] = Ry; M[8] = Rz; M[12] = -(Rx*EyeX + Ry*EyeY + Rz*EyeZ);
	M[1] = Ux; M[5] = Uy; M[9] = Uz; M[13] = -(Ux*EyeX + Uy*EyeY + Uz*EyeZ);
	M[2] = -Fx; M[6] = -Fy; M[10] = -Fz; M[14] = (Fx*EyeX + Fy*EyeY + Fz*EyeZ);
	M[3] = 0.0f; M[7] = 0.0f; M[11] = 0.0f; M[15] = 1.0f;
}

void ViewportHostRender(ViewportHost *Host, const AppState *State, int Width, int Height) {
	if (!Host || !Host->Initialized || !State) return;

	glViewport(0, 0, Width, Height);
	glClearColor(0.08f, 0.08f, 0.10f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	float Aspect = Height > 0 ? (float)Width / (float)Height : 1.0f;
	float Proj[16];
	MakePerspective(Proj, State->Camera.FovDeg, Aspect, State->Camera.NearPlane, State->Camera.FarPlane);

	float View[16];
	MakeLookAt(View, State->Camera.PosX, State->Camera.PosY, State->Camera.PosZ,
		State->Camera.TargetX, State->Camera.TargetY, State->Camera.TargetZ);

	if (State->Prefs.ShowGrid) {
		GridRendererDraw(&Host->Grid, View, Proj);
	}
}
