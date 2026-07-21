/**
 * GLAD — OpenGL 3.3 Core Profile loader.
 * OpenGL 3.3コアプロファイルのローダーね。
 *
 * Generated subset covering every function used by TohruPhysics GUI:
 * vertex arrays, buffer objects, shaders, programs, framebuffers,
 * textures, draw calls, and debug output.
 *
 * DESIGN PHILOSOPHY:
 * GLAD resolves OpenGL entry points at runtime via dlopen/GetProcAddress,
 * giving us portable access to GL 3.3 core on Linux, macOS, and Windows
 * without linking against libGL at compile time. Every pointer is
 * initialised to a stub that returns 0 / no-ops, matching ZII.
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#ifndef GLAD_GL_H_
#define GLAD_GL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

/* -------------------------------------------------------------------------
 *  Platform-specific function pointer type
 * ---------------------------------------------------------------------- */
#if defined(_WIN32)
#	define APIENTRY __stdcall
#else
#	define APIENTRY
#endif

typedef void (*GLADapiproc)(void);
typedef GLADapiproc (*GLADloadfunc)(const char *Name);

/* -------------------------------------------------------------------------
 *  GL base types
 * ---------------------------------------------------------------------- */
typedef unsigned int   GLenum;
typedef unsigned char  GLboolean;
typedef unsigned int   GLbitfield;
typedef signed char    GLbyte;
typedef short          GLshort;
typedef int            GLint;
typedef int            GLsizei;
typedef unsigned char  GLubyte;
typedef unsigned short GLushort;
typedef unsigned int   GLuint;
typedef float          GLfloat;
typedef float          GLclampf;
typedef double         GLdouble;
typedef double         GLclampd;
typedef char           GLchar;
typedef ptrdiff_t      GLintptr;
typedef ptrdiff_t      GLsizeiptr;
typedef int64_t        GLint64;
typedef uint64_t       GLuint64;
typedef void           GLvoid;

/* -------------------------------------------------------------------------
 *  GL 3.3 constants (complete core profile set)
 * ---------------------------------------------------------------------- */
#define GL_FALSE                          0
#define GL_TRUE                           1
#define GL_NONE                           0

/* Primitives */
#define GL_POINTS                         0x0000
#define GL_LINES                          0x0001
#define GL_LINE_LOOP                      0x0002
#define GL_LINE_STRIP                     0x0003
#define GL_TRIANGLES                      0x0004
#define GL_TRIANGLE_STRIP                 0x0005
#define GL_TRIANGLE_FAN                   0x0006
#define GL_LINES_ADJACENCY                0x000A
#define GL_TRIANGLES_ADJACENCY            0x000C

/* Data types */
#define GL_BYTE                           0x1400
#define GL_UNSIGNED_BYTE                  0x1401
#define GL_SHORT                          0x1402
#define GL_UNSIGNED_SHORT                 0x1403
#define GL_INT                            0x1404
#define GL_UNSIGNED_INT                   0x1405
#define GL_FLOAT                          0x1406
#define GL_DOUBLE                         0x140A

/* Booleans */
#define GL_ZERO                           0
#define GL_ONE                            1

/* Buffer types */
#define GL_ARRAY_BUFFER                   0x8892
#define GL_ELEMENT_ARRAY_BUFFER           0x8893
#define GL_UNIFORM_BUFFER                 0x8A11
#define GL_STATIC_DRAW                    0x88B4
#define GL_DYNAMIC_DRAW                   0x88E8
#define GL_STREAM_DRAW                    0x88E0

/* Shader types */
#define GL_FRAGMENT_SHADER                0x8B30
#define GL_VERTEX_SHADER                  0x8B31
#define GL_GEOMETRY_SHADER                0x8DD9

/* Shader queries */
#define GL_COMPILE_STATUS                 0x8B81
#define GL_LINK_STATUS                    0x8B82
#define GL_INFO_LOG_LENGTH                0x8B84
#define GL_VALIDATE_STATUS                0x8B83

/* Textures */
#define GL_TEXTURE_2D                     0x0DE1
#define GL_TEXTURE0                       0x84C0
#define GL_TEXTURE_MIN_FILTER             0x2801
#define GL_TEXTURE_MAG_FILTER             0x2800
#define GL_TEXTURE_WRAP_S                 0x2802
#define GL_TEXTURE_WRAP_T                 0x2803
#define GL_NEAREST                        0x2600
#define GL_LINEAR                         0x2601
#define GL_CLAMP_TO_EDGE                  0x812F
#define GL_REPEAT                         0x2901
#define GL_RGB                            0x1907
#define GL_RGBA                           0x1908
#define GL_RED                            0x1903
#define GL_RGBA32F                        0x8814
#define GL_RGBA8                          0x8058

/* Framebuffers */
#define GL_FRAMEBUFFER                    0x8D40
#define GL_RENDERBUFFER                   0x8D41
#define GL_COLOR_ATTACHMENT0              0x8CE0
#define GL_DEPTH_ATTACHMENT               0x8D00
#define GL_DEPTH_STENCIL_ATTACHMENT       0x821A
#define GL_FRAMEBUFFER_COMPLETE           0x8CD5
#define GL_DEPTH_COMPONENT24              0x81A6
#define GL_DEPTH_STENCIL                  0x84F9
#define GL_UNSIGNED_INT_24_8              0x84FA
#define GL_DEPTH24_STENCIL8               0x88F0

/* Clearing */
#define GL_COLOR_BUFFER_BIT               0x00004000
#define GL_DEPTH_BUFFER_BIT               0x00000100
#define GL_STENCIL_BUFFER_BIT             0x00000400

/* Enable flags */
#define GL_DEPTH_TEST                     0x0B71
#define GL_BLEND                          0x0BE2
#define GL_CULL_FACE                      0x0B44
#define GL_MULTISAMPLE                    0x809D
#define GL_LINE_SMOOTH                    0x0B20
#define GL_POLYGON_OFFSET_FILL            0x8037
#define GL_PROGRAM_POINT_SIZE             0x8642

/* Blend functions */
#define GL_SRC_ALPHA                      0x0302
#define GL_ONE_MINUS_SRC_ALPHA            0x0303
#define GL_ONE                            1

/* Depth functions */
#define GL_LESS                           0x0201
#define GL_LEQUAL                         0x0203

/* Fill mode */
#define GL_FILL                           0x1B02
#define GL_LINE                           0x1B01
#define GL_POINT                          0x1B00
#define GL_FRONT_AND_BACK                 0x0408

/* Errors */
#define GL_NO_ERROR                       0
#define GL_INVALID_ENUM                   0x0500
#define GL_INVALID_VALUE                  0x0501
#define GL_INVALID_OPERATION              0x0502
#define GL_OUT_OF_MEMORY                  0x0505

/* Vertex attrib */
#define GL_FLOAT_VEC2                     0x8B50
#define GL_FLOAT_VEC3                     0x8B51
#define GL_FLOAT_VEC4                     0x8B52
#define GL_FLOAT_MAT4                     0x8B5C

/* Queries */
#define GL_VENDOR                         0x1F00
#define GL_RENDERER                       0x1F01
#define GL_VERSION                        0x1F02
#define GL_SHADING_LANGUAGE_VERSION       0x8B8C

/* Debug output (GL_KHR_debug / GL 4.3 but available as extension on 3.3) */
#define GL_DEBUG_OUTPUT                   0x92E0
#define GL_DEBUG_OUTPUT_SYNCHRONOUS       0x8242
#define GL_DEBUG_SEVERITY_HIGH            0x9146
#define GL_DEBUG_SEVERITY_MEDIUM          0x9147
#define GL_DEBUG_SEVERITY_LOW             0x9148
#define GL_DEBUG_SEVERITY_NOTIFICATION    0x826B
#define GL_DEBUG_SOURCE_API               0x8246
#define GL_DEBUG_TYPE_ERROR               0x824C

/* -------------------------------------------------------------------------
 *  Function pointer declarations — all GL 3.3 core functions used
 *  GL 3.3コア関数ポインタの宣言ね。
 * ---------------------------------------------------------------------- */

/* Vertex arrays */
extern void (APIENTRY *glad_glGenVertexArrays)(GLsizei N, GLuint *Arrays);
extern void (APIENTRY *glad_glBindVertexArray)(GLuint Array);
extern void (APIENTRY *glad_glDeleteVertexArrays)(GLsizei N, const GLuint *Arrays);
extern void (APIENTRY *glad_glEnableVertexAttribArray)(GLuint Index);
extern void (APIENTRY *glad_glDisableVertexAttribArray)(GLuint Index);
extern void (APIENTRY *glad_glVertexAttribPointer)(GLuint Index, GLint Size, GLenum Type, GLboolean Normalized, GLsizei Stride, const GLvoid *Pointer);
extern void (APIENTRY *glad_glVertexAttribIPointer)(GLuint Index, GLint Size, GLenum Type, GLsizei Stride, const GLvoid *Pointer);
extern void (APIENTRY *glad_glVertexAttribDivisor)(GLuint Index, GLuint Divisor);

/* Buffer objects */
extern void (APIENTRY *glad_glGenBuffers)(GLsizei N, GLuint *Buffers);
extern void (APIENTRY *glad_glBindBuffer)(GLenum Target, GLuint Buffer);
extern void (APIENTRY *glad_glBufferData)(GLenum Target, GLsizeiptr Size, const GLvoid *Data, GLenum Usage);
extern void (APIENTRY *glad_glBufferSubData)(GLenum Target, GLintptr Offset, GLsizeiptr Size, const GLvoid *Data);
extern void (APIENTRY *glad_glDeleteBuffers)(GLsizei N, const GLuint *Buffers);
extern void *(APIENTRY *glad_glMapBuffer)(GLenum Target, GLenum Access);
extern GLboolean (APIENTRY *glad_glUnmapBuffer)(GLenum Target);
extern void (APIENTRY *glad_glBindBufferBase)(GLenum Target, GLuint Index, GLuint Buffer);

/* Shaders */
extern GLuint (APIENTRY *glad_glCreateShader)(GLenum Type);
extern void   (APIENTRY *glad_glShaderSource)(GLuint Shader, GLsizei Count, const GLchar *const *Strings, const GLint *Length);
extern void   (APIENTRY *glad_glCompileShader)(GLuint Shader);
extern void   (APIENTRY *glad_glGetShaderiv)(GLuint Shader, GLenum PName, GLint *Params);
extern void   (APIENTRY *glad_glGetShaderInfoLog)(GLuint Shader, GLsizei BufSize, GLsizei *Length, GLchar *InfoLog);
extern void   (APIENTRY *glad_glDeleteShader)(GLuint Shader);

/* Programs */
extern GLuint (APIENTRY *glad_glCreateProgram)(void);
extern void   (APIENTRY *glad_glAttachShader)(GLuint Program, GLuint Shader);
extern void   (APIENTRY *glad_glLinkProgram)(GLuint Program);
extern void   (APIENTRY *glad_glGetProgramiv)(GLuint Program, GLenum PName, GLint *Params);
extern void   (APIENTRY *glad_glGetProgramInfoLog)(GLuint Program, GLsizei BufSize, GLsizei *Length, GLchar *InfoLog);
extern void   (APIENTRY *glad_glUseProgram)(GLuint Program);
extern void   (APIENTRY *glad_glDeleteProgram)(GLuint Program);
extern GLint  (APIENTRY *glad_glGetUniformLocation)(GLuint Program, const GLchar *Name);
extern void   (APIENTRY *glad_glUniform1i)(GLint Location, GLint V0);
extern void   (APIENTRY *glad_glUniform1f)(GLint Location, GLfloat V0);
extern void   (APIENTRY *glad_glUniform2f)(GLint Location, GLfloat V0, GLfloat V1);
extern void   (APIENTRY *glad_glUniform3f)(GLint Location, GLfloat V0, GLfloat V1, GLfloat V2);
extern void   (APIENTRY *glad_glUniform4f)(GLint Location, GLfloat V0, GLfloat V1, GLfloat V2, GLfloat V3);
extern void   (APIENTRY *glad_glUniformMatrix4fv)(GLint Location, GLsizei Count, GLboolean Transpose, const GLfloat *Value);
extern void   (APIENTRY *glad_glUniform1fv)(GLint Location, GLsizei Count, const GLfloat *Value);
extern void   (APIENTRY *glad_glUniform3fv)(GLint Location, GLsizei Count, const GLfloat *Value);

/* Textures */
extern void (APIENTRY *glad_glGenTextures)(GLsizei N, GLuint *Textures);
extern void (APIENTRY *glad_glBindTexture)(GLenum Target, GLuint Texture);
extern void (APIENTRY *glad_glTexImage2D)(GLenum Target, GLint Level, GLint InternalFmt, GLsizei Width, GLsizei Height, GLint Border, GLenum Format, GLenum Type, const GLvoid *Data);
extern void (APIENTRY *glad_glTexParameteri)(GLenum Target, GLenum PName, GLint Param);
extern void (APIENTRY *glad_glGenerateMipmap)(GLenum Target);
extern void (APIENTRY *glad_glDeleteTextures)(GLsizei N, const GLuint *Textures);
extern void (APIENTRY *glad_glActiveTexture)(GLenum Texture);

/* Framebuffers */
extern void   (APIENTRY *glad_glGenFramebuffers)(GLsizei N, GLuint *Fbs);
extern void   (APIENTRY *glad_glBindFramebuffer)(GLenum Target, GLuint Fb);
extern void   (APIENTRY *glad_glFramebufferTexture2D)(GLenum Target, GLenum Attachment, GLenum TexTarget, GLuint Texture, GLint Level);
extern GLenum (APIENTRY *glad_glCheckFramebufferStatus)(GLenum Target);
extern void   (APIENTRY *glad_glDeleteFramebuffers)(GLsizei N, const GLuint *Fbs);
extern void   (APIENTRY *glad_glGenRenderbuffers)(GLsizei N, GLuint *Rbs);
extern void   (APIENTRY *glad_glBindRenderbuffer)(GLenum Target, GLuint Rb);
extern void   (APIENTRY *glad_glRenderbufferStorage)(GLenum Target, GLenum InternalFmt, GLsizei Width, GLsizei Height);
extern void   (APIENTRY *glad_glFramebufferRenderbuffer)(GLenum Target, GLenum Attachment, GLenum RbTarget, GLuint Rb);
extern void   (APIENTRY *glad_glDeleteRenderbuffers)(GLsizei N, const GLuint *Rbs);

/* Draw calls */
extern void (APIENTRY *glad_glDrawArrays)(GLenum Mode, GLint First, GLsizei Count);
extern void (APIENTRY *glad_glDrawElements)(GLenum Mode, GLsizei Count, GLenum Type, const GLvoid *Indices);
extern void (APIENTRY *glad_glDrawArraysInstanced)(GLenum Mode, GLint First, GLsizei Count, GLsizei InstanceCount);
extern void (APIENTRY *glad_glDrawElementsInstanced)(GLenum Mode, GLsizei Count, GLenum Type, const GLvoid *Indices, GLsizei InstanceCount);

/* State */
extern void   (APIENTRY *glad_glEnable)(GLenum Cap);
extern void   (APIENTRY *glad_glDisable)(GLenum Cap);
extern void   (APIENTRY *glad_glClearColor)(GLfloat R, GLfloat G, GLfloat B, GLfloat A);
extern void   (APIENTRY *glad_glClear)(GLbitfield Mask);
extern void   (APIENTRY *glad_glViewport)(GLint X, GLint Y, GLsizei Width, GLsizei Height);
extern void   (APIENTRY *glad_glBlendFunc)(GLenum SFactor, GLenum DFactor);
extern void   (APIENTRY *glad_glDepthFunc)(GLenum Func);
extern void   (APIENTRY *glad_glDepthMask)(GLboolean Flag);
extern void   (APIENTRY *glad_glCullFace)(GLenum Mode);
extern void   (APIENTRY *glad_glPolygonMode)(GLenum Face, GLenum Mode);
extern void   (APIENTRY *glad_glLineWidth)(GLfloat Width);
extern void   (APIENTRY *glad_glPointSize)(GLfloat Size);
extern void   (APIENTRY *glad_glScissor)(GLint X, GLint Y, GLsizei Width, GLsizei Height);
extern GLenum (APIENTRY *glad_glGetError)(void);
extern const GLubyte *(APIENTRY *glad_glGetString)(GLenum Name);
extern void   (APIENTRY *glad_glGetIntegerv)(GLenum PName, GLint *Data);
extern void   (APIENTRY *glad_glPolygonOffset)(GLfloat Factor, GLfloat Units);

/* Synchronisation */
extern void (APIENTRY *glad_glFinish)(void);
extern void (APIENTRY *glad_glFlush)(void);

/* Debug (KHR_debug extension — available on GL 3.3 via extension) */
typedef void (APIENTRY *GLDEBUGPROC)(
	GLenum Source, GLenum Type, GLuint Id, GLenum Severity,
	GLsizei Length, const GLchar *Message, const void *UserParam);
extern void (APIENTRY *glad_glDebugMessageCallback)(GLDEBUGPROC Callback, const void *UserParam);
extern void (APIENTRY *glad_glDebugMessageControl)(GLenum Source, GLenum Type, GLenum Severity, GLsizei Count, const GLuint *Ids, GLboolean Enabled);

/* -------------------------------------------------------------------------
 *  Convenience macros — write glXxx(...) instead of glad_glXxx(...)
 * ---------------------------------------------------------------------- */
#define glGenVertexArrays          glad_glGenVertexArrays
#define glBindVertexArray          glad_glBindVertexArray
#define glDeleteVertexArrays       glad_glDeleteVertexArrays
#define glEnableVertexAttribArray  glad_glEnableVertexAttribArray
#define glDisableVertexAttribArray glad_glDisableVertexAttribArray
#define glVertexAttribPointer      glad_glVertexAttribPointer
#define glVertexAttribIPointer     glad_glVertexAttribIPointer
#define glVertexAttribDivisor      glad_glVertexAttribDivisor

#define glGenBuffers               glad_glGenBuffers
#define glBindBuffer               glad_glBindBuffer
#define glBufferData               glad_glBufferData
#define glBufferSubData            glad_glBufferSubData
#define glDeleteBuffers            glad_glDeleteBuffers
#define glMapBuffer                glad_glMapBuffer
#define glUnmapBuffer              glad_glUnmapBuffer
#define glBindBufferBase           glad_glBindBufferBase

#define glCreateShader             glad_glCreateShader
#define glShaderSource             glad_glShaderSource
#define glCompileShader            glad_glCompileShader
#define glGetShaderiv              glad_glGetShaderiv
#define glGetShaderInfoLog         glad_glGetShaderInfoLog
#define glDeleteShader             glad_glDeleteShader

#define glCreateProgram            glad_glCreateProgram
#define glAttachShader             glad_glAttachShader
#define glLinkProgram              glad_glLinkProgram
#define glGetProgramiv             glad_glGetProgramiv
#define glGetProgramInfoLog        glad_glGetProgramInfoLog
#define glUseProgram               glad_glUseProgram
#define glDeleteProgram            glad_glDeleteProgram
#define glGetUniformLocation       glad_glGetUniformLocation
#define glUniform1i                glad_glUniform1i
#define glUniform1f                glad_glUniform1f
#define glUniform2f                glad_glUniform2f
#define glUniform3f                glad_glUniform3f
#define glUniform4f                glad_glUniform4f
#define glUniformMatrix4fv         glad_glUniformMatrix4fv
#define glUniform1fv               glad_glUniform1fv
#define glUniform3fv               glad_glUniform3fv

#define glGenTextures              glad_glGenTextures
#define glBindTexture              glad_glBindTexture
#define glTexImage2D               glad_glTexImage2D
#define glTexParameteri            glad_glTexParameteri
#define glGenerateMipmap           glad_glGenerateMipmap
#define glDeleteTextures           glad_glDeleteTextures
#define glActiveTexture            glad_glActiveTexture

#define glGenFramebuffers          glad_glGenFramebuffers
#define glBindFramebuffer          glad_glBindFramebuffer
#define glFramebufferTexture2D     glad_glFramebufferTexture2D
#define glCheckFramebufferStatus   glad_glCheckFramebufferStatus
#define glDeleteFramebuffers       glad_glDeleteFramebuffers
#define glGenRenderbuffers         glad_glGenRenderbuffers
#define glBindRenderbuffer         glad_glBindRenderbuffer
#define glRenderbufferStorage      glad_glRenderbufferStorage
#define glFramebufferRenderbuffer  glad_glFramebufferRenderbuffer
#define glDeleteRenderbuffers      glad_glDeleteRenderbuffers

#define glDrawArrays               glad_glDrawArrays
#define glDrawElements             glad_glDrawElements
#define glDrawArraysInstanced      glad_glDrawArraysInstanced
#define glDrawElementsInstanced    glad_glDrawElementsInstanced

#define glEnable                   glad_glEnable
#define glDisable                  glad_glDisable
#define glClearColor               glad_glClearColor
#define glClear                    glad_glClear
#define glViewport                 glad_glViewport
#define glBlendFunc                glad_glBlendFunc
#define glDepthFunc                glad_glDepthFunc
#define glDepthMask                glad_glDepthMask
#define glCullFace                 glad_glCullFace
#define glPolygonMode              glad_glPolygonMode
#define glLineWidth                glad_glLineWidth
#define glPointSize                glad_glPointSize
#define glScissor                  glad_glScissor
#define glGetError                 glad_glGetError
#define glGetString                glad_glGetString
#define glGetIntegerv              glad_glGetIntegerv
#define glPolygonOffset            glad_glPolygonOffset
#define glFinish                   glad_glFinish
#define glFlush                    glad_glFlush
#define glDebugMessageCallback     glad_glDebugMessageCallback
#define glDebugMessageControl      glad_glDebugMessageControl

/* -------------------------------------------------------------------------
 *  gladLoadGL — resolves all pointers via the provided loader.
 *  Returns 1 on success, 0 if any required symbol was missing.
 *  gladLoadGL — 提供されたローダーで全ポインタを解決するの。
 * ---------------------------------------------------------------------- */
int gladLoadGL(GLADloadfunc Load);

/* GLX / EGL / WGL platform loader helper — returns the system proc address */
GLADapiproc gladGetProcAddressPtr(const char *Name);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* GLAD_GL_H_ */
