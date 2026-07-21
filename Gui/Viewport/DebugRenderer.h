/**
 * DebugRenderer — debug geometry drawing (AABBs, lines, points).
 * デバッグジオメトリ描画ね。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#pragma once

#include "GlPipeline.h"

struct DebugVertex {
	float Position[3];
	float Color[3];
};

struct DebugRenderer {
	GLuint   Vao;
	GLuint   Vbo;
	GlShader Shader;
};

void DebugRendererInit(DebugRenderer *Debug, const char *VertSrc, const char *FragSrc);
void DebugRendererDestroy(DebugRenderer *Debug);
void DebugRendererDrawLines(DebugRenderer *Debug, const DebugVertex *Verts, int Count, const float *View, const float *Proj);
