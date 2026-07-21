/**
 * DebugRenderer implementation.
 * デバッグレンダラーの実装ね。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include "DebugRenderer.h"

void DebugRendererInit(DebugRenderer *Debug, const char *VertSrc, const char *FragSrc) {
	if (!Debug) return;
	Debug->Shader = GlShaderCompile(VertSrc, FragSrc);

	glGenVertexArrays(1, &Debug->Vao);
	glGenBuffers(1, &Debug->Vbo);

	glBindVertexArray(Debug->Vao);
	glBindBuffer(GL_ARRAY_BUFFER, Debug->Vbo);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(DebugVertex), (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(DebugVertex), (void*)(3 * sizeof(float)));

	glBindVertexArray(0);
}

void DebugRendererDestroy(DebugRenderer *Debug) {
	if (!Debug) return;
	if (Debug->Vao) glDeleteVertexArrays(1, &Debug->Vao);
	if (Debug->Vbo) glDeleteBuffers(1, &Debug->Vbo);
	GlShaderDestroy(&Debug->Shader);
}

void DebugRendererDrawLines(DebugRenderer *Debug, const DebugVertex *Verts, int Count, const float *View, const float *Proj) {
	if (!Debug || !Debug->Shader.Program || !Verts || Count <= 0) return;

	GlShaderUse(&Debug->Shader);
	GlShaderSetMat4(&Debug->Shader, "uView", View);
	GlShaderSetMat4(&Debug->Shader, "uProjection", Proj);

	glBindVertexArray(Debug->Vao);
	glBindBuffer(GL_ARRAY_BUFFER, Debug->Vbo);
	glBufferData(GL_ARRAY_BUFFER, Count * sizeof(DebugVertex), Verts, GL_DYNAMIC_DRAW);

	glDrawArrays(GL_LINES, 0, Count);
	glBindVertexArray(0);
}
