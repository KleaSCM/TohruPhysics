/**
 * GridRenderer — 3D ground plane grid renderer.
 * 3Dグラウンドプレーングリッドレンダラーね。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#pragma once

#include "GlPipeline.h"

struct GridRenderer {
	GLuint   Vao;
	GLuint   Vbo;
	GlShader Shader;
	int      LineCount;
};

void GridRendererInit(GridRenderer *Grid, const char *VertSrc, const char *FragSrc);
void GridRendererDestroy(GridRenderer *Grid);
void GridRendererDraw(GridRenderer *Grid, const float *View, const float *Proj);
