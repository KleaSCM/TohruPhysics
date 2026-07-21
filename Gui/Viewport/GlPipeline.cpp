/**
 * GlPipeline implementation.
 * GlPipelineの実装ね。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include "GlPipeline.h"

#include <stdio.h>
#include <stdlib.h>

static GLuint CompileShaderStage(GLenum Type, const char *Src) {
	GLuint S = glCreateShader(Type);
	glShaderSource(S, 1, &Src, NULL);
	glCompileShader(S);

	GLint Success = 0;
	glGetShaderiv(S, GL_COMPILE_STATUS, &Success);
	if (!Success) {
		glDeleteShader(S);
		return 0;
	}
	return S;
}

GlShader GlShaderCompile(const char *VertSrc, const char *FragSrc) {
	GlShader Shader = {0};
	if (!VertSrc || !FragSrc) {
		return Shader;
	}

	GLuint VS = CompileShaderStage(GL_VERTEX_SHADER, VertSrc);
	GLuint FS = CompileShaderStage(GL_FRAGMENT_SHADER, FragSrc);
	if (!VS || !FS) {
		if (VS) glDeleteShader(VS);
		if (FS) glDeleteShader(FS);
		return Shader;
	}

	GLuint Prog = glCreateProgram();
	glAttachShader(Prog, VS);
	glAttachShader(Prog, FS);
	glLinkProgram(Prog);

	GLint Linked = 0;
	glGetProgramiv(Prog, GL_LINK_STATUS, &Linked);
	glDeleteShader(VS);
	glDeleteShader(FS);

	if (!Linked) {
		glDeleteProgram(Prog);
		return Shader;
	}

	Shader.Program = Prog;
	return Shader;
}

void GlShaderDestroy(GlShader *Shader) {
	if (Shader && Shader->Program) {
		glDeleteProgram(Shader->Program);
		Shader->Program = 0;
	}
}

void GlShaderUse(const GlShader *Shader) {
	if (Shader && Shader->Program) {
		glUseProgram(Shader->Program);
	}
}

void GlShaderSetMat4(const GlShader *Shader, const char *Name, const float *Mat4) {
	if (!Shader || !Shader->Program) return;
	GLint Loc = glGetUniformLocation(Shader->Program, Name);
	if (Loc >= 0) {
		glUniformMatrix4fv(Loc, 1, GL_FALSE, Mat4);
	}
}

void GlShaderSetVec3(const GlShader *Shader, const char *Name, float X, float Y, float Z) {
	if (!Shader || !Shader->Program) return;
	GLint Loc = glGetUniformLocation(Shader->Program, Name);
	if (Loc >= 0) {
		glUniform3f(Loc, X, Y, Z);
	}
}

void GlShaderSetVec4(const GlShader *Shader, const char *Name, float X, float Y, float Z, float W) {
	if (!Shader || !Shader->Program) return;
	GLint Loc = glGetUniformLocation(Shader->Program, Name);
	if (Loc >= 0) {
		glUniform4f(Loc, X, Y, Z, W);
	}
}
