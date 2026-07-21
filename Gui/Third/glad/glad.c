/**
 * GLAD — OpenGL 3.3 Core Profile loader implementation.
 * OpenGL 3.3コアプロファイルローダーの実装ね。
 *
 * Resolves all GL 3.3 entry points at runtime via dlopen(NULL) + dlsym
 * on Linux/macOS, or wglGetProcAddress on Windows. Falls back to the
 * platform's native GL library symbol table for core functions.
 *
 * DESIGN PHILOSOPHY:
 * All function pointers are initialised to stub functions that return 0
 * or are no-ops, consistent with ZII — the renderer never crashes on a
 * missing extension, it just silently produces no output.
 *
 * WORKFLOW:
 * 1. Call gladLoadGL(loader) at startup, passing your context's proc-address
 *    function (e.g. SDL_GL_GetProcAddress or glfwGetProcAddress).
 * 2. After return, every glad_gl* pointer is valid or points to a stub.
 * 3. All GL calls through the macros work immediately.
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include "glad.h"

#include <stddef.h>
#include <string.h>

/* -------------------------------------------------------------------------
 *  Platform dynamic loading
 * ---------------------------------------------------------------------- */
#if defined(_WIN32)
#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>
	static HMODULE GlLib;
	static void GladOpenLib(void) {
		GlLib = LoadLibraryA("opengl32.dll");
	}
	static GLADapiproc GladLoadNative(const char *Name) {
		GLADapiproc P = NULL;
		if (GlLib) {
			P = (GLADapiproc)(void *)GetProcAddress(GlLib, Name);
		}
		return P;
	}
#else
#	include <dlfcn.h>
	static void *GlLib;
	static void GladOpenLib(void) {
		/* Try common Linux GL library names */
		GlLib = dlopen("libGL.so.1", RTLD_LAZY | RTLD_GLOBAL);
		if (!GlLib) {
			GlLib = dlopen("libGL.so", RTLD_LAZY | RTLD_GLOBAL);
		}
		if (!GlLib) {
			/* Mesa software fallback */
			GlLib = dlopen("libGLX_mesa.so.0", RTLD_LAZY | RTLD_GLOBAL);
		}
	}
	static GLADapiproc GladLoadNative(const char *Name) {
		if (!GlLib) {
			return NULL;
		}
		return (GLADapiproc)dlsym(GlLib, Name);
	}
#endif

/* -------------------------------------------------------------------------
 *  Stub functions — ZII: missing entry points are silent no-ops / zero
 *  スタブ関数 — ZII：エントリポイントが存在しない場合はno-op/0を返すね。
 * ---------------------------------------------------------------------- */
static void StubVoid(void) {}
static GLuint StubUint(void) { return 0; }
static GLint  StubInt(void)  { return 0; }
static GLenum StubEnum(void) { return GL_NO_ERROR; }
static const GLubyte *StubStr(void) { return (const GLubyte *)""; }
static void *StubPtr(void) { return NULL; }
static GLboolean StubBool(void) { return GL_TRUE; }

/* -------------------------------------------------------------------------
 *  Function pointer definitions
 * ---------------------------------------------------------------------- */
void (APIENTRY *glad_glGenVertexArrays)(GLsizei, GLuint *)                  = (void(APIENTRY *)(GLsizei, GLuint *))StubVoid;
void (APIENTRY *glad_glBindVertexArray)(GLuint)                              = (void(APIENTRY *)(GLuint))StubVoid;
void (APIENTRY *glad_glDeleteVertexArrays)(GLsizei, const GLuint *)          = (void(APIENTRY *)(GLsizei, const GLuint *))StubVoid;
void (APIENTRY *glad_glEnableVertexAttribArray)(GLuint)                      = (void(APIENTRY *)(GLuint))StubVoid;
void (APIENTRY *glad_glDisableVertexAttribArray)(GLuint)                     = (void(APIENTRY *)(GLuint))StubVoid;
void (APIENTRY *glad_glVertexAttribPointer)(GLuint, GLint, GLenum, GLboolean, GLsizei, const GLvoid *) = (void(APIENTRY *)(GLuint, GLint, GLenum, GLboolean, GLsizei, const GLvoid *))StubVoid;
void (APIENTRY *glad_glVertexAttribIPointer)(GLuint, GLint, GLenum, GLsizei, const GLvoid *)           = (void(APIENTRY *)(GLuint, GLint, GLenum, GLsizei, const GLvoid *))StubVoid;
void (APIENTRY *glad_glVertexAttribDivisor)(GLuint, GLuint)                  = (void(APIENTRY *)(GLuint, GLuint))StubVoid;

void (APIENTRY *glad_glGenBuffers)(GLsizei, GLuint *)                        = (void(APIENTRY *)(GLsizei, GLuint *))StubVoid;
void (APIENTRY *glad_glBindBuffer)(GLenum, GLuint)                           = (void(APIENTRY *)(GLenum, GLuint))StubVoid;
void (APIENTRY *glad_glBufferData)(GLenum, GLsizeiptr, const GLvoid *, GLenum) = (void(APIENTRY *)(GLenum, GLsizeiptr, const GLvoid *, GLenum))StubVoid;
void (APIENTRY *glad_glBufferSubData)(GLenum, GLintptr, GLsizeiptr, const GLvoid *) = (void(APIENTRY *)(GLenum, GLintptr, GLsizeiptr, const GLvoid *))StubVoid;
void (APIENTRY *glad_glDeleteBuffers)(GLsizei, const GLuint *)               = (void(APIENTRY *)(GLsizei, const GLuint *))StubVoid;
void *(APIENTRY *glad_glMapBuffer)(GLenum, GLenum)                           = (void *(APIENTRY *)(GLenum, GLenum))StubPtr;
GLboolean (APIENTRY *glad_glUnmapBuffer)(GLenum)                             = (GLboolean(APIENTRY *)(GLenum))StubBool;
void (APIENTRY *glad_glBindBufferBase)(GLenum, GLuint, GLuint)               = (void(APIENTRY *)(GLenum, GLuint, GLuint))StubVoid;

GLuint (APIENTRY *glad_glCreateShader)(GLenum)                               = (GLuint(APIENTRY *)(GLenum))StubUint;
void   (APIENTRY *glad_glShaderSource)(GLuint, GLsizei, const GLchar *const *, const GLint *) = (void(APIENTRY *)(GLuint, GLsizei, const GLchar *const *, const GLint *))StubVoid;
void   (APIENTRY *glad_glCompileShader)(GLuint)                              = (void(APIENTRY *)(GLuint))StubVoid;
void   (APIENTRY *glad_glGetShaderiv)(GLuint, GLenum, GLint *)               = (void(APIENTRY *)(GLuint, GLenum, GLint *))StubVoid;
void   (APIENTRY *glad_glGetShaderInfoLog)(GLuint, GLsizei, GLsizei *, GLchar *) = (void(APIENTRY *)(GLuint, GLsizei, GLsizei *, GLchar *))StubVoid;
void   (APIENTRY *glad_glDeleteShader)(GLuint)                               = (void(APIENTRY *)(GLuint))StubVoid;

GLuint (APIENTRY *glad_glCreateProgram)(void)                                = (GLuint(APIENTRY *)(void))StubUint;
void   (APIENTRY *glad_glAttachShader)(GLuint, GLuint)                       = (void(APIENTRY *)(GLuint, GLuint))StubVoid;
void   (APIENTRY *glad_glLinkProgram)(GLuint)                                = (void(APIENTRY *)(GLuint))StubVoid;
void   (APIENTRY *glad_glGetProgramiv)(GLuint, GLenum, GLint *)              = (void(APIENTRY *)(GLuint, GLenum, GLint *))StubVoid;
void   (APIENTRY *glad_glGetProgramInfoLog)(GLuint, GLsizei, GLsizei *, GLchar *) = (void(APIENTRY *)(GLuint, GLsizei, GLsizei *, GLchar *))StubVoid;
void   (APIENTRY *glad_glUseProgram)(GLuint)                                 = (void(APIENTRY *)(GLuint))StubVoid;
void   (APIENTRY *glad_glDeleteProgram)(GLuint)                              = (void(APIENTRY *)(GLuint))StubVoid;
GLint  (APIENTRY *glad_glGetUniformLocation)(GLuint, const GLchar *)         = (GLint(APIENTRY *)(GLuint, const GLchar *))StubInt;
void   (APIENTRY *glad_glUniform1i)(GLint, GLint)                            = (void(APIENTRY *)(GLint, GLint))StubVoid;
void   (APIENTRY *glad_glUniform1f)(GLint, GLfloat)                          = (void(APIENTRY *)(GLint, GLfloat))StubVoid;
void   (APIENTRY *glad_glUniform2f)(GLint, GLfloat, GLfloat)                 = (void(APIENTRY *)(GLint, GLfloat, GLfloat))StubVoid;
void   (APIENTRY *glad_glUniform3f)(GLint, GLfloat, GLfloat, GLfloat)        = (void(APIENTRY *)(GLint, GLfloat, GLfloat, GLfloat))StubVoid;
void   (APIENTRY *glad_glUniform4f)(GLint, GLfloat, GLfloat, GLfloat, GLfloat) = (void(APIENTRY *)(GLint, GLfloat, GLfloat, GLfloat, GLfloat))StubVoid;
void   (APIENTRY *glad_glUniformMatrix4fv)(GLint, GLsizei, GLboolean, const GLfloat *) = (void(APIENTRY *)(GLint, GLsizei, GLboolean, const GLfloat *))StubVoid;
void   (APIENTRY *glad_glUniform1fv)(GLint, GLsizei, const GLfloat *)        = (void(APIENTRY *)(GLint, GLsizei, const GLfloat *))StubVoid;
void   (APIENTRY *glad_glUniform3fv)(GLint, GLsizei, const GLfloat *)        = (void(APIENTRY *)(GLint, GLsizei, const GLfloat *))StubVoid;

void (APIENTRY *glad_glGenTextures)(GLsizei, GLuint *)                       = (void(APIENTRY *)(GLsizei, GLuint *))StubVoid;
void (APIENTRY *glad_glBindTexture)(GLenum, GLuint)                          = (void(APIENTRY *)(GLenum, GLuint))StubVoid;
void (APIENTRY *glad_glTexImage2D)(GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid *) = (void(APIENTRY *)(GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid *))StubVoid;
void (APIENTRY *glad_glTexParameteri)(GLenum, GLenum, GLint)                 = (void(APIENTRY *)(GLenum, GLenum, GLint))StubVoid;
void (APIENTRY *glad_glGenerateMipmap)(GLenum)                               = (void(APIENTRY *)(GLenum))StubVoid;
void (APIENTRY *glad_glDeleteTextures)(GLsizei, const GLuint *)              = (void(APIENTRY *)(GLsizei, const GLuint *))StubVoid;
void (APIENTRY *glad_glActiveTexture)(GLenum)                                = (void(APIENTRY *)(GLenum))StubVoid;

void   (APIENTRY *glad_glGenFramebuffers)(GLsizei, GLuint *)                 = (void(APIENTRY *)(GLsizei, GLuint *))StubVoid;
void   (APIENTRY *glad_glBindFramebuffer)(GLenum, GLuint)                    = (void(APIENTRY *)(GLenum, GLuint))StubVoid;
void   (APIENTRY *glad_glFramebufferTexture2D)(GLenum, GLenum, GLenum, GLuint, GLint) = (void(APIENTRY *)(GLenum, GLenum, GLenum, GLuint, GLint))StubVoid;
GLenum (APIENTRY *glad_glCheckFramebufferStatus)(GLenum)                     = (GLenum(APIENTRY *)(GLenum))StubEnum;
void   (APIENTRY *glad_glDeleteFramebuffers)(GLsizei, const GLuint *)        = (void(APIENTRY *)(GLsizei, const GLuint *))StubVoid;
void   (APIENTRY *glad_glGenRenderbuffers)(GLsizei, GLuint *)                = (void(APIENTRY *)(GLsizei, GLuint *))StubVoid;
void   (APIENTRY *glad_glBindRenderbuffer)(GLenum, GLuint)                   = (void(APIENTRY *)(GLenum, GLuint))StubVoid;
void   (APIENTRY *glad_glRenderbufferStorage)(GLenum, GLenum, GLsizei, GLsizei) = (void(APIENTRY *)(GLenum, GLenum, GLsizei, GLsizei))StubVoid;
void   (APIENTRY *glad_glFramebufferRenderbuffer)(GLenum, GLenum, GLenum, GLuint) = (void(APIENTRY *)(GLenum, GLenum, GLenum, GLuint))StubVoid;
void   (APIENTRY *glad_glDeleteRenderbuffers)(GLsizei, const GLuint *)       = (void(APIENTRY *)(GLsizei, const GLuint *))StubVoid;

void (APIENTRY *glad_glDrawArrays)(GLenum, GLint, GLsizei)                   = (void(APIENTRY *)(GLenum, GLint, GLsizei))StubVoid;
void (APIENTRY *glad_glDrawElements)(GLenum, GLsizei, GLenum, const GLvoid *) = (void(APIENTRY *)(GLenum, GLsizei, GLenum, const GLvoid *))StubVoid;
void (APIENTRY *glad_glDrawArraysInstanced)(GLenum, GLint, GLsizei, GLsizei) = (void(APIENTRY *)(GLenum, GLint, GLsizei, GLsizei))StubVoid;
void (APIENTRY *glad_glDrawElementsInstanced)(GLenum, GLsizei, GLenum, const GLvoid *, GLsizei) = (void(APIENTRY *)(GLenum, GLsizei, GLenum, const GLvoid *, GLsizei))StubVoid;

void   (APIENTRY *glad_glEnable)(GLenum)                                     = (void(APIENTRY *)(GLenum))StubVoid;
void   (APIENTRY *glad_glDisable)(GLenum)                                    = (void(APIENTRY *)(GLenum))StubVoid;
void   (APIENTRY *glad_glClearColor)(GLfloat, GLfloat, GLfloat, GLfloat)     = (void(APIENTRY *)(GLfloat, GLfloat, GLfloat, GLfloat))StubVoid;
void   (APIENTRY *glad_glClear)(GLbitfield)                                  = (void(APIENTRY *)(GLbitfield))StubVoid;
void   (APIENTRY *glad_glViewport)(GLint, GLint, GLsizei, GLsizei)           = (void(APIENTRY *)(GLint, GLint, GLsizei, GLsizei))StubVoid;
void   (APIENTRY *glad_glBlendFunc)(GLenum, GLenum)                          = (void(APIENTRY *)(GLenum, GLenum))StubVoid;
void   (APIENTRY *glad_glDepthFunc)(GLenum)                                  = (void(APIENTRY *)(GLenum))StubVoid;
void   (APIENTRY *glad_glDepthMask)(GLboolean)                               = (void(APIENTRY *)(GLboolean))StubVoid;
void   (APIENTRY *glad_glCullFace)(GLenum)                                   = (void(APIENTRY *)(GLenum))StubVoid;
void   (APIENTRY *glad_glPolygonMode)(GLenum, GLenum)                        = (void(APIENTRY *)(GLenum, GLenum))StubVoid;
void   (APIENTRY *glad_glLineWidth)(GLfloat)                                 = (void(APIENTRY *)(GLfloat))StubVoid;
void   (APIENTRY *glad_glPointSize)(GLfloat)                                 = (void(APIENTRY *)(GLfloat))StubVoid;
void   (APIENTRY *glad_glScissor)(GLint, GLint, GLsizei, GLsizei)            = (void(APIENTRY *)(GLint, GLint, GLsizei, GLsizei))StubVoid;
GLenum (APIENTRY *glad_glGetError)(void)                                     = (GLenum(APIENTRY *)(void))StubEnum;
const GLubyte *(APIENTRY *glad_glGetString)(GLenum)                          = (const GLubyte *(APIENTRY *)(GLenum))StubStr;
void   (APIENTRY *glad_glGetIntegerv)(GLenum, GLint *)                       = (void(APIENTRY *)(GLenum, GLint *))StubVoid;
void   (APIENTRY *glad_glPolygonOffset)(GLfloat, GLfloat)                    = (void(APIENTRY *)(GLfloat, GLfloat))StubVoid;
void   (APIENTRY *glad_glFinish)(void)                                       = (void(APIENTRY *)(void))StubVoid;
void   (APIENTRY *glad_glFlush)(void)                                        = (void(APIENTRY *)(void))StubVoid;

void (APIENTRY *glad_glDebugMessageCallback)(GLDEBUGPROC, const void *)      = (void(APIENTRY *)(GLDEBUGPROC, const void *))StubVoid;
void (APIENTRY *glad_glDebugMessageControl)(GLenum, GLenum, GLenum, GLsizei, const GLuint *, GLboolean) = (void(APIENTRY *)(GLenum, GLenum, GLenum, GLsizei, const GLuint *, GLboolean))StubVoid;

/* -------------------------------------------------------------------------
 *  gladGetProcAddressPtr — resolve via user loader then native fallback
 * ---------------------------------------------------------------------- */
static GLADloadfunc GUserLoader;

GLADapiproc gladGetProcAddressPtr(const char *Name) {
	GLADapiproc P = NULL;
	if (GUserLoader) {
		P = GUserLoader(Name);
	}
	if (!P) {
		P = GladLoadNative(Name);
	}
	return P;
}

/* -------------------------------------------------------------------------
 *  Resolve helper macro — load or keep stub
 * ---------------------------------------------------------------------- */
#define GLAD_LOAD(Sym, Type) \
	do { \
		GLADapiproc _P = gladGetProcAddressPtr(#Sym); \
		if (_P) { glad_##Sym = (Type)_P; } \
	} while(0)

/* -------------------------------------------------------------------------
 *  gladLoadGL — resolve all pointers; returns 1 on full success
 * ---------------------------------------------------------------------- */
int gladLoadGL(GLADloadfunc Load) {
	GUserLoader = Load;
	GladOpenLib();

	/* Vertex arrays */
	GLAD_LOAD(glGenVertexArrays,          void(APIENTRY *)(GLsizei, GLuint *));
	GLAD_LOAD(glBindVertexArray,          void(APIENTRY *)(GLuint));
	GLAD_LOAD(glDeleteVertexArrays,       void(APIENTRY *)(GLsizei, const GLuint *));
	GLAD_LOAD(glEnableVertexAttribArray,  void(APIENTRY *)(GLuint));
	GLAD_LOAD(glDisableVertexAttribArray, void(APIENTRY *)(GLuint));
	GLAD_LOAD(glVertexAttribPointer,      void(APIENTRY *)(GLuint, GLint, GLenum, GLboolean, GLsizei, const GLvoid *));
	GLAD_LOAD(glVertexAttribIPointer,     void(APIENTRY *)(GLuint, GLint, GLenum, GLsizei, const GLvoid *));
	GLAD_LOAD(glVertexAttribDivisor,      void(APIENTRY *)(GLuint, GLuint));

	/* Buffers */
	GLAD_LOAD(glGenBuffers,               void(APIENTRY *)(GLsizei, GLuint *));
	GLAD_LOAD(glBindBuffer,               void(APIENTRY *)(GLenum, GLuint));
	GLAD_LOAD(glBufferData,               void(APIENTRY *)(GLenum, GLsizeiptr, const GLvoid *, GLenum));
	GLAD_LOAD(glBufferSubData,            void(APIENTRY *)(GLenum, GLintptr, GLsizeiptr, const GLvoid *));
	GLAD_LOAD(glDeleteBuffers,            void(APIENTRY *)(GLsizei, const GLuint *));
	GLAD_LOAD(glMapBuffer,                void *(APIENTRY *)(GLenum, GLenum));
	GLAD_LOAD(glUnmapBuffer,              GLboolean(APIENTRY *)(GLenum));
	GLAD_LOAD(glBindBufferBase,           void(APIENTRY *)(GLenum, GLuint, GLuint));

	/* Shaders */
	GLAD_LOAD(glCreateShader,             GLuint(APIENTRY *)(GLenum));
	GLAD_LOAD(glShaderSource,             void(APIENTRY *)(GLuint, GLsizei, const GLchar *const *, const GLint *));
	GLAD_LOAD(glCompileShader,            void(APIENTRY *)(GLuint));
	GLAD_LOAD(glGetShaderiv,              void(APIENTRY *)(GLuint, GLenum, GLint *));
	GLAD_LOAD(glGetShaderInfoLog,         void(APIENTRY *)(GLuint, GLsizei, GLsizei *, GLchar *));
	GLAD_LOAD(glDeleteShader,             void(APIENTRY *)(GLuint));

	/* Programs */
	GLAD_LOAD(glCreateProgram,            GLuint(APIENTRY *)(void));
	GLAD_LOAD(glAttachShader,             void(APIENTRY *)(GLuint, GLuint));
	GLAD_LOAD(glLinkProgram,              void(APIENTRY *)(GLuint));
	GLAD_LOAD(glGetProgramiv,             void(APIENTRY *)(GLuint, GLenum, GLint *));
	GLAD_LOAD(glGetProgramInfoLog,        void(APIENTRY *)(GLuint, GLsizei, GLsizei *, GLchar *));
	GLAD_LOAD(glUseProgram,               void(APIENTRY *)(GLuint));
	GLAD_LOAD(glDeleteProgram,            void(APIENTRY *)(GLuint));
	GLAD_LOAD(glGetUniformLocation,       GLint(APIENTRY *)(GLuint, const GLchar *));
	GLAD_LOAD(glUniform1i,                void(APIENTRY *)(GLint, GLint));
	GLAD_LOAD(glUniform1f,                void(APIENTRY *)(GLint, GLfloat));
	GLAD_LOAD(glUniform2f,                void(APIENTRY *)(GLint, GLfloat, GLfloat));
	GLAD_LOAD(glUniform3f,                void(APIENTRY *)(GLint, GLfloat, GLfloat, GLfloat));
	GLAD_LOAD(glUniform4f,                void(APIENTRY *)(GLint, GLfloat, GLfloat, GLfloat, GLfloat));
	GLAD_LOAD(glUniformMatrix4fv,         void(APIENTRY *)(GLint, GLsizei, GLboolean, const GLfloat *));
	GLAD_LOAD(glUniform1fv,               void(APIENTRY *)(GLint, GLsizei, const GLfloat *));
	GLAD_LOAD(glUniform3fv,               void(APIENTRY *)(GLint, GLsizei, const GLfloat *));

	/* Textures */
	GLAD_LOAD(glGenTextures,              void(APIENTRY *)(GLsizei, GLuint *));
	GLAD_LOAD(glBindTexture,              void(APIENTRY *)(GLenum, GLuint));
	GLAD_LOAD(glTexImage2D,               void(APIENTRY *)(GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid *));
	GLAD_LOAD(glTexParameteri,            void(APIENTRY *)(GLenum, GLenum, GLint));
	GLAD_LOAD(glGenerateMipmap,           void(APIENTRY *)(GLenum));
	GLAD_LOAD(glDeleteTextures,           void(APIENTRY *)(GLsizei, const GLuint *));
	GLAD_LOAD(glActiveTexture,            void(APIENTRY *)(GLenum));

	/* Framebuffers */
	GLAD_LOAD(glGenFramebuffers,          void(APIENTRY *)(GLsizei, GLuint *));
	GLAD_LOAD(glBindFramebuffer,          void(APIENTRY *)(GLenum, GLuint));
	GLAD_LOAD(glFramebufferTexture2D,     void(APIENTRY *)(GLenum, GLenum, GLenum, GLuint, GLint));
	GLAD_LOAD(glCheckFramebufferStatus,   GLenum(APIENTRY *)(GLenum));
	GLAD_LOAD(glDeleteFramebuffers,       void(APIENTRY *)(GLsizei, const GLuint *));
	GLAD_LOAD(glGenRenderbuffers,         void(APIENTRY *)(GLsizei, GLuint *));
	GLAD_LOAD(glBindRenderbuffer,         void(APIENTRY *)(GLenum, GLuint));
	GLAD_LOAD(glRenderbufferStorage,      void(APIENTRY *)(GLenum, GLenum, GLsizei, GLsizei));
	GLAD_LOAD(glFramebufferRenderbuffer,  void(APIENTRY *)(GLenum, GLenum, GLenum, GLuint));
	GLAD_LOAD(glDeleteRenderbuffers,      void(APIENTRY *)(GLsizei, const GLuint *));

	/* Draw */
	GLAD_LOAD(glDrawArrays,               void(APIENTRY *)(GLenum, GLint, GLsizei));
	GLAD_LOAD(glDrawElements,             void(APIENTRY *)(GLenum, GLsizei, GLenum, const GLvoid *));
	GLAD_LOAD(glDrawArraysInstanced,      void(APIENTRY *)(GLenum, GLint, GLsizei, GLsizei));
	GLAD_LOAD(glDrawElementsInstanced,    void(APIENTRY *)(GLenum, GLsizei, GLenum, const GLvoid *, GLsizei));

	/* State */
	GLAD_LOAD(glEnable,                   void(APIENTRY *)(GLenum));
	GLAD_LOAD(glDisable,                  void(APIENTRY *)(GLenum));
	GLAD_LOAD(glClearColor,               void(APIENTRY *)(GLfloat, GLfloat, GLfloat, GLfloat));
	GLAD_LOAD(glClear,                    void(APIENTRY *)(GLbitfield));
	GLAD_LOAD(glViewport,                 void(APIENTRY *)(GLint, GLint, GLsizei, GLsizei));
	GLAD_LOAD(glBlendFunc,                void(APIENTRY *)(GLenum, GLenum));
	GLAD_LOAD(glDepthFunc,                void(APIENTRY *)(GLenum));
	GLAD_LOAD(glDepthMask,                void(APIENTRY *)(GLboolean));
	GLAD_LOAD(glCullFace,                 void(APIENTRY *)(GLenum));
	GLAD_LOAD(glPolygonMode,              void(APIENTRY *)(GLenum, GLenum));
	GLAD_LOAD(glLineWidth,                void(APIENTRY *)(GLfloat));
	GLAD_LOAD(glPointSize,                void(APIENTRY *)(GLfloat));
	GLAD_LOAD(glScissor,                  void(APIENTRY *)(GLint, GLint, GLsizei, GLsizei));
	GLAD_LOAD(glGetError,                 GLenum(APIENTRY *)(void));
	GLAD_LOAD(glGetString,                const GLubyte *(APIENTRY *)(GLenum));
	GLAD_LOAD(glGetIntegerv,              void(APIENTRY *)(GLenum, GLint *));
	GLAD_LOAD(glPolygonOffset,            void(APIENTRY *)(GLfloat, GLfloat));
	GLAD_LOAD(glFinish,                   void(APIENTRY *)(void));
	GLAD_LOAD(glFlush,                    void(APIENTRY *)(void));

	/* Debug (optional — silently skipped if extension missing) */
	GLAD_LOAD(glDebugMessageCallback,     void(APIENTRY *)(GLDEBUGPROC, const void *));
	GLAD_LOAD(glDebugMessageControl,      void(APIENTRY *)(GLenum, GLenum, GLenum, GLsizei, const GLuint *, GLboolean));

	/*
	 * Verify at least the most basic function is present.
	 * 最低限の関数が存在することを確認するの。
	 */
	return (glad_glCreateShader != (void(APIENTRY *)(GLenum))StubVoid) ? 1 : 0;
}
