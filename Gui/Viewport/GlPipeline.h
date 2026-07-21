/**
 * GlPipeline — OpenGL shader compilation and buffer manager.
 * OpenGLシェーダーコンパイルとバッファマネージャーね。
 *
 * Handles shader program compilation, VAO/VBO creation, uniform setting.
 * Zero-initialised stub structures on initialization failure.
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#pragma once

#include "../Third/glad/glad.h"

struct GlShader {
	GLuint Program;
};

GlShader GlShaderCompile(const char *VertSrc, const char *FragSrc);
void     GlShaderDestroy(GlShader *Shader);
void     GlShaderUse(const GlShader *Shader);
void     GlShaderSetMat4(const GlShader *Shader, const char *Name, const float *Mat4);
void     GlShaderSetVec3(const GlShader *Shader, const char *Name, float X, float Y, float Z);
void     GlShaderSetVec4(const GlShader *Shader, const char *Name, float X, float Y, float Z, float W);
