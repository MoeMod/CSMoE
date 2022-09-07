/*
gl-wes-v2:  OpenGL 2.0 to OGLESv2.0 wrapper
Contact:    lachlan.ts@gmail.com
Copyright (C) 2009  Lachlan Tychsen - Smith aka Adventus

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 3 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "wes_config.h"
#include "wes_gl_defines.h"
#include <stdio.h>

#ifndef __WES_H__
#define __WES_H__

#define     WES_OGLESV2_FUNCTIONCOUNT   145

#define WES_LIGHT_NUM           8
#define WES_CLIPPLANE_NUM       6
#define WES_MULTITEX_NUM        4
#define WES_FACE_NUM            2

#ifdef WES_MANGLE_PREPEND
#define GL_MANGLE( x ) p##x
#else
#define GL_MANGLE( x ) x
#endif

#define LOG_TAG "gl-wes-v2"

#ifdef __ANDROID__
#include <android/log.h>
#define LOG __android_log_print

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGD(...) if (DEBUG_NANO) __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG,__VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG,__VA_ARGS__)
#else

#define LOGI(...) printf("I: \n"__VA_ARGS__)
#define LOGD(...) if(DEBUG_NANO) printf("D: \n"__VA_ARGS__)
#define LOGE(...) printf("E: \n"__VA_ARGS__)
#define LOGW(...) printf("W: \n"__VA_ARGS__)

#endif

//WES2 Defines:
#ifdef WES_OUTPUT_ERRORS
#define PRINT_ERROR(...)        fprintf(stdout, __VA_ARGS__); fflush(stdout)
#else
#define PRINT_ERROR(...)
#endif


typedef struct gles2lib_s gles2lib_t;

#if !defined (__WINS__)
    #if	defined(__TARGET_FPU_VFP)
        #pragma softfp_linkage
    #endif
#endif

#ifdef SOFTFP_LINK
#define S __attribute__((pcs("aapcs")))
#else
#define S
#endif

typedef void ( *GL_DEBUG_PROC_KHR )( unsigned int source, unsigned int type, unsigned int id, unsigned int severity, int length, const char* message, void* userParam );
typedef void ( *GL_DEBUG_PROC_ARB )( unsigned int source, unsigned int type, unsigned int id, unsigned int severity, int length, const char* message, void* userParam );

struct gles2lib_s
{
     void         (*glActiveTexture)(GLenum texture) S;
     void         (*glAttachShader)(GLuint program, GLuint shader) S;
     void         (*glBindAttribLocation)(GLuint program, GLuint index, const char* name) S;
     void         (*glBindBuffer)(GLenum target, GLuint buffer) S;
     void         (*glBindFramebuffer)(GLenum target, GLuint framebuffer) S;
     void         (*glBindRenderbuffer)(GLenum target, GLuint renderbuffer) S;
     void         (*glBindTexture)(GLenum target, GLuint texture) S;
     void         (*glBlendColor)(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) S;
     void         (*glBlendEquation)( GLenum mode ) S;
     void         (*glBlendEquationSeparate)(GLenum modeRGB, GLenum modeAlpha) S;
     void         (*glBlendFunc)(GLenum sfactor, GLenum dfactor) S;
     void         (*glBlendFuncSeparate)(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha) S;
     void         (*glBufferData)(GLenum target, GLsizeiptr size, const void* data, GLenum usage) S;
     void         (*glBufferSubData)(GLenum target, GLintptr offset, GLsizeiptr size, const void* data) S;
     GLenum       (*glCheckFramebufferStatus)(GLenum target) S;
     void         (*glClear)(GLbitfield mask) S;
     void         (*glClearColor)(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) S;
     void         (*glClearDepthf)(GLclampf depth) S;
     void         (*glClearStencil)(GLint s) S;
     void         (*glColorMask)(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha) S;
     void         (*glCompileShader)(GLuint shader) S;
     void         (*glCompressedTexImage2D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void* data) S;
     void         (*glCompressedTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void* data) S;
     void         (*glCopyTexImage2D)(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border) S;
     void         (*glCopyTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height) S;
     GLuint       (*glCreateProgram)(void) S;
     GLuint       (*glCreateShader)(GLenum type) S;
     void         (*glCullFace)(GLenum mode) S;
     void         (*glDeleteBuffers)(GLsizei n, const GLuint* buffers) S;
     void         (*glDeleteFramebuffers)(GLsizei n, const GLuint* framebuffers) S;
     void         (*glDeleteTextures)(GLsizei n, const GLuint* textures) S;
     void         (*glDeleteProgram)(GLuint program) S;
     void         (*glDeleteRenderbuffers)(GLsizei n, const GLuint* renderbuffers) S;
     void         (*glDeleteShader)(GLuint shader) S;
     void         (*glDetachShader)(GLuint program, GLuint shader) S;
     void         (*glDepthFunc)(GLenum func) S;
     void         (*glDepthMask)(GLboolean flag) S;
     void         (*glDepthRangef)(GLclampf zNear, GLclampf zFar) S;
     void         (*glDisable)(GLenum cap) S;
     void         (*glDisableVertexAttribArray)(GLuint index) S;
     void         (*glDrawArrays)(GLenum mode, GLint first, GLsizei count) S;
     void         (*glDrawElements)(GLenum mode, GLsizei count, GLenum type, const void* indices) S;
     void         (*glEnable)(GLenum cap) S;
     void         (*glEnableVertexAttribArray)(GLuint index) S;
     void         (*glFinish)(void) S;
     void         (*glFlush)(void) S;
     void         (*glFramebufferRenderbuffer)(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer) S;
     void         (*glFramebufferTexture2D)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) S;
     void         (*glFrontFace)(GLenum mode) S;
     void         (*glGenBuffers)(GLsizei n, GLuint* buffers) S;
     void         (*glGenerateMipmap)(GLenum target) S;
     void         (*glGenFramebuffers)(GLsizei n, GLuint* framebuffers) S;
     void         (*glGenRenderbuffers)(GLsizei n, GLuint* renderbuffers) S;
     void         (*glGenTextures)(GLsizei n, GLuint* textures) S;
     void         (*glGetActiveAttrib)(GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, char* name) S;
     void         (*glGetActiveUniform)(GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, char* name) S;
     void         (*glGetAttachedShaders)(GLuint program, GLsizei maxcount, GLsizei* count, GLuint* shaders) S;
     int          (*glGetAttribLocation)(GLuint program, const char* name) S;
     void         (*glGetBooleanv)(GLenum pname, GLboolean* params) S;
     void         (*glGetBufferParameteriv)(GLenum target, GLenum pname, GLint* params) S;
     GLenum       (*glGetError)(void) S;
     void         (*glGetFloatv)(GLenum pname, GLfloat* params) S;
     void         (*glGetFramebufferAttachmentParameteriv)(GLenum target, GLenum attachment, GLenum pname, GLint* params) S;
     void         (*glGetIntegerv)(GLenum pname, GLint* params) S;
     void         (*glGetProgramiv)(GLuint program, GLenum pname, GLint* params) S;
     void         (*glGetProgramInfoLog)(GLuint program, GLsizei bufsize, GLsizei* length, char* infolog) S;
     void         (*glGetRenderbufferParameteriv)(GLenum target, GLenum pname, GLint* params) S;
     void         (*glGetShaderiv)(GLuint shader, GLenum pname, GLint* params) S;
     void         (*glGetShaderInfoLog)(GLuint shader, GLsizei bufsize, GLsizei* length, char* infolog) S;
     void         (*glGetShaderPrecisionFormat)(GLenum shadertype, GLenum precisiontype, GLint* range, GLint* precision) S;
     void         (*glGetShaderSource)(GLuint shader, GLsizei bufsize, GLsizei* length, char* source) S;
     const GLubyte* (*glGetString)(GLenum name) S;
     void         (*glGetTexParameterfv)(GLenum target, GLenum pname, GLfloat* params) S;
     void         (*glGetTexParameteriv)(GLenum target, GLenum pname, GLint* params) S;
     void         (*glGetUniformfv)(GLuint program, GLint location, GLfloat* params) S;
     void         (*glGetUniformiv)(GLuint program, GLint location, GLint* params) S;
     int          (*glGetUniformLocation)(GLuint program, const char* name) S;
     void         (*glGetVertexAttribfv)(GLuint index, GLenum pname, GLfloat* params) S;
     void         (*glGetVertexAttribiv)(GLuint index, GLenum pname, GLint* params) S;
     void         (*glGetVertexAttribPointerv)(GLuint index, GLenum pname, void** pointer) S;
     void         (*glHint)(GLenum target, GLenum mode) S;
     GLboolean    (*glIsBuffer)(GLuint buffer) S;
     GLboolean    (*glIsEnabled)(GLenum cap) S;
     GLboolean    (*glIsFramebuffer)(GLuint framebuffer) S;
     GLboolean    (*glIsProgram)(GLuint program) S;
     GLboolean    (*glIsRenderbuffer)(GLuint renderbuffer) S;
     GLboolean    (*glIsShader)(GLuint shader) S;
     GLboolean    (*glIsTexture)(GLuint texture) S;
     void         (*glLineWidth)(GLfloat width) S;
     void         (*glLinkProgram)(GLuint program) S;
     void         (*glPixelStorei)(GLenum pname, GLint param) S;
     void         (*glPolygonOffset)(GLfloat factor, GLfloat units) S;
     void         (*glReadPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void* pixels) S;
     void         (*glReleaseShaderCompiler)(void) S;
     void         (*glRenderbufferStorage)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height) S;
     void         (*glSampleCoverage)(GLclampf value, GLboolean invert) S;
     void         (*glScissor)(GLint x, GLint y, GLsizei width, GLsizei height) S;
     void         (*glShaderBinary)(GLint n, const GLuint* shaders, GLenum binaryformat, const void* binary, GLint length) S;
     void         (*glShaderSource)(GLuint shader, GLsizei count, const char** string, const GLint* length) S;
     void         (*glStencilFunc)(GLenum func, GLint ref, GLuint mask) S;
     void         (*glStencilFuncSeparate)(GLenum face, GLenum func, GLint ref, GLuint mask) S;
     void         (*glStencilMask)(GLuint mask) S;
     void         (*glStencilMaskSeparate)(GLenum face, GLuint mask) S;
     void         (*glStencilOp)(GLenum fail, GLenum zfail, GLenum zpass) S;
     void         (*glStencilOpSeparate)(GLenum face, GLenum fail, GLenum zfail, GLenum zpass) S;
     void         (*glTexImage2D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels) S;
     void         (*glTexParameterf)(GLenum target, GLenum pname, GLfloat param) S;
     void         (*glTexParameterfv)(GLenum target, GLenum pname, const GLfloat* params) S;
     void         (*glTexParameteri)(GLenum target, GLenum pname, GLint param) S;
     void         (*glTexParameteriv)(GLenum target, GLenum pname, const GLint* params) S;
     void         (*glTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels) S;
     void         (*glUniform1f)(GLint location, GLfloat x) S;
     void         (*glUniform1fv)(GLint location, GLsizei count, const GLfloat* v) S;
     void         (*glUniform1i)(GLint location, GLint x) S;
     void         (*glUniform1iv)(GLint location, GLsizei count, const GLint* v) S;
     void         (*glUniform2f)(GLint location, GLfloat x, GLfloat y) S;
     void         (*glUniform2fv)(GLint location, GLsizei count, const GLfloat* v) S;
     void         (*glUniform2i)(GLint location, GLint x, GLint y) S;
     void         (*glUniform2iv)(GLint location, GLsizei count, const GLint* v) S;
     void         (*glUniform3f)(GLint location, GLfloat x, GLfloat y, GLfloat z) S;
     void         (*glUniform3fv)(GLint location, GLsizei count, const GLfloat* v) S;
     void         (*glUniform3i)(GLint location, GLint x, GLint y, GLint z) S;
     void         (*glUniform3iv)(GLint location, GLsizei count, const GLint* v) S;
     void         (*glUniform4f)(GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w) S;
     void         (*glUniform4fv)(GLint location, GLsizei count, const GLfloat* v) S;
     void         (*glUniform4i)(GLint location, GLint x, GLint y, GLint z, GLint w) S;
     void         (*glUniform4iv)(GLint location, GLsizei count, const GLint* v) S;
     void         (*glUniformMatrix2fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) S;
     void         (*glUniformMatrix3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) S;
     void         (*glUniformMatrix4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) S;
     void         (*glUseProgram)(GLuint program) S;
     void         (*glValidateProgram)(GLuint program) S;
     void         (*glVertexAttrib1f)(GLuint indx, GLfloat x) S;
     void         (*glVertexAttrib1fv)(GLuint indx, const GLfloat* values) S;
     void         (*glVertexAttrib2f)(GLuint indx, GLfloat x, GLfloat y) S;
     void         (*glVertexAttrib2fv)(GLuint indx, const GLfloat* values) S;
     void         (*glVertexAttrib3f)(GLuint indx, GLfloat x, GLfloat y, GLfloat z) S;
     void         (*glVertexAttrib3fv)(GLuint indx, const GLfloat* values) S;
     void         (*glVertexAttrib4f)(GLuint indx, GLfloat x, GLfloat y, GLfloat z, GLfloat w) S;
     void         (*glVertexAttrib4fv)(GLuint indx, const GLfloat* values) S;
     void         (*glVertexAttribPointer)(GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* ptr) S;
     void         (*glViewport)(GLint x, GLint y, GLsizei width, GLsizei height) S;
     void         (*glDebugMessageControlKHR)( GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint* ids, GLboolean enabled ) S;
     void         (*glDebugMessageInsertKHR)( GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const char* buf ) S;
     void         (*glDebugMessageCallbackKHR)( GL_DEBUG_PROC_KHR callback, void* userParam ) S;
     GLuint       (*glGetDebugMessageLogKHR)( GLuint count, GLsizei bufsize, GLenum* sources, GLenum* types, GLuint* ids, GLuint* severities, GLsizei* lengths, char* messageLog ) S;
};

#if !defined (__WINS__)
    #if	defined(__TARGET_FPU_VFP)
        #pragma no_softfp_linkage
    #endif
#endif

extern gles2lib_t* wes_gl;

extern GLvoid wes_init(const char *gles2);
extern GLvoid wes_destroy( void );
extern GLvoid wes_vertbuffer_flush(void);
#endif
