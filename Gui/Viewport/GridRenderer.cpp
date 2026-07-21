/**
 * GridRenderer implementation.
 * グリッドレンダラーの実装ね。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include "GridRenderer.h"

#include <vector>

void GridRendererInit(GridRenderer *Grid, const char *VertSrc, const char *FragSrc) {
	if (!Grid) return;
	Grid->Shader = GlShaderCompile(VertSrc, FragSrc);

	std::vector<float> Verts;
	int HalfSize = 20;
	float Step = 1.0f;

	for (int i = -HalfSize; i <= HalfSize; ++i) {
		// Line along X
		Verts.push_back((float)-HalfSize * Step);
		Verts.push_back(0.0f);
		Verts.push_back((float)i * Step);

		Verts.push_back((float)HalfSize * Step);
		Verts.push_back(0.0f);
		Verts.push_back((float)i * Step);

		// Line along Z
		Verts.push_back((float)i * Step);
		Verts.push_back(0.0f);
		Verts.push_back((float)-HalfSize * Step);

		Verts.push_back((float)i * Step);
		Verts.push_back(0.0f);
		Verts.push_back((float)HalfSize * Step);
	}

	Grid->LineCount = (int)(Verts.size() / 3);

	glGenVertexArrays(1, &Grid->Vao);
	glGenBuffers(1, &Grid->Vbo);

	glBindVertexArray(Grid->Vao);
	glBindBuffer(GL_ARRAY_BUFFER, Grid->Vbo);
	glBufferData(GL_ARRAY_BUFFER, Verts.size() * sizeof(float), Verts.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	glBindVertexArray(0);
}

void GridRendererDestroy(GridRenderer *Grid) {
	if (!Grid) return;
	if (Grid->Vao) glDeleteVertexArrays(1, &Grid->Vao);
	if (Grid->Vbo) glDeleteBuffers(1, &Grid->Vbo);
	GlShaderDestroy(&Grid->Shader);
}

void GridRendererDraw(GridRenderer *Grid, const float *View, const float *Proj) {
	if (!Grid || !Grid->Shader.Program || Grid->LineCount == 0) return;

	GlShaderUse(&Grid->Shader);
	GlShaderSetMat4(&Grid->Shader, "uView", View);
	GlShaderSetMat4(&Grid->Shader, "uProjection", Proj);
	GlShaderSetVec4(&Grid->Shader, "uColor", 0.3f, 0.35f, 0.4f, 0.5f);

	glBindVertexArray(Grid->Vao);
	glDrawArrays(GL_LINES, 0, Grid->LineCount);
	glBindVertexArray(0);
}
