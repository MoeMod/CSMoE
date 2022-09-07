/*
GL_MANGLE(gl-wes-v2:  OpenGL 2.0 to OGLESv2.0 wrapper
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

#if defined(_WIN32)
#include <windows.h>

#ifdef WINAPI_FAMILY
#if (!WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP) && WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP))
#define XASH_WINRT
#endif
#endif
#endif

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "wes.h"
#include "wes_begin.h"
#include "wes_shader.h"
#include "wes_matrix.h"

#if defined(_WIN32)
#ifdef XASH_WINRT
static void* dlopen(const char* dllname)
{
	wchar_t buffer[MAX_PATH];
	MultiByteToWideChar(CP_ACP, 0, dllname, -1, buffer, MAX_PATH);
	return LoadPackagedLibrary(buffer, 0);
}
static void dlclose(void* hInstance)
{
	FreeLibrary((HMODULE)hInstance);
}
static void* dlsym(void* hInstance, const char* name)
{
	return GetProcAddress((HMODULE)hInstance, name);
}
#else
    #define dlopen(A, B)    LoadLibrary(A)
    #define dlsym(A, B)     GetProcAddress((HINSTANCE__*) A, B)
    #define dlclose(A)      FreeLibrary((HINSTANCE__*) A)
#endif
#else
    #define __GNU_SOURCE
    #include <dlfcn.h>
#endif

void*           wes_libhandle = NULL;
gles2lib_t*     wes_gl = NULL;

//OpenGL ES 2 function names for runtime library loading:
const char* glfuncnames[] =
{
    "glActiveTexture",
    "glAttachShader",
    "glBindAttribLocation",
    "glBindBuffer",
    "glBindFramebuffer",
    "glBindRenderbuffer",
    "glBindTexture",
    "glBlendColor",
    "glBlendEquation",
    "glBlendEquationSeparate",
    "glBlendFunc",
    "glBlendFuncSeparate",
    "glBufferData",
    "glBufferSubData",
    "glCheckFramebufferStatus",
    "glClear",
    "glClearColor",
    "glClearDepthf",
    "glClearStencil",
    "glColorMask",
    "glCompileShader",
    "glCompressedTexImage2D",
    "glCompressedTexSubImage2D",
    "glCopyTexImage2D",
    "glCopyTexSubImage2D",
    "glCreateProgram",
    "glCreateShader",
    "glCullFace",
    "glDeleteBuffers",
    "glDeleteFramebuffers",
    "glDeleteTextures",
    "glDeleteProgram",
    "glDeleteRenderbuffers",
    "glDeleteShader",
    "glDetachShader",
    "glDepthFunc",
    "glDepthMask",
    "glDepthRangef",
    "glDisable",
    "glDisableVertexAttribArray",
    "glDrawArrays",
    "glDrawElements",
    "glEnable",
    "glEnableVertexAttribArray",
    "glFinish",
    "glFlush",
    "glFramebufferRenderbuffer",
    "glFramebufferTexture2D",
    "glFrontFace",
    "glGenBuffers",
    "glGenerateMipmap",
    "glGenFramebuffers",
    "glGenRenderbuffers",
    "glGenTextures",
    "glGetActiveAttrib",
    "glGetActiveUniform",
    "glGetAttachedShaders",
    "glGetAttribLocation",
    "glGetBooleanv",
    "glGetBufferParameteriv",
    "glGetError",
    "glGetFloatv",
    "glGetFramebufferAttachmentParameteriv",
    "glGetIntegerv",
    "glGetProgramiv",
    "glGetProgramInfoLog",
    "glGetRenderbufferParameteriv",
    "glGetShaderiv",
    "glGetShaderInfoLog",
    "glGetShaderPrecisionFormat",
    "glGetShaderSource",
    "glGetString",
    "glGetTexParameterfv",
    "glGetTexParameteriv",
    "glGetUniformfv",
    "glGetUniformiv",
    "glGetUniformLocation",
    "glGetVertexAttribfv",
    "glGetVertexAttribiv",
    "glGetVertexAttribPointerv",
    "glHint",
    "glIsBuffer",
    "glIsEnabled",
    "glIsFramebuffer",
    "glIsProgram",
    "glIsRenderbuffer",
    "glIsShader",
    "glIsTexture",
    "glLineWidth",
    "glLinkProgram",
    "glPixelStorei",
    "glPolygonOffset",
    "glReadPixels",
    "glReleaseShaderCompiler",
    "glRenderbufferStorage",
    "glSampleCoverage",
    "glScissor",
    "glShaderBinary",
    "glShaderSource",
    "glStencilFunc",
    "glStencilFuncSeparate",
    "glStencilMask",
    "glStencilMaskSeparate",
    "glStencilOp",
    "glStencilOpSeparate",
    "glTexImage2D",
    "glTexParameterf",
    "glTexParameterfv",
    "glTexParameteri",
    "glTexParameteriv",
    "glTexSubImage2D",
    "glUniform1f",
    "glUniform1fv",
    "glUniform1i",
    "glUniform1iv",
    "glUniform2f",
    "glUniform2fv",
    "glUniform2i",
    "glUniform2iv",
    "glUniform3f",
    "glUniform3fv",
    "glUniform3i",
    "glUniform3iv",
    "glUniform4f",
    "glUniform4fv",
    "glUniform4i",
    "glUniform4iv",
    "glUniformMatrix2fv",
    "glUniformMatrix3fv",
    "glUniformMatrix4fv",
    "glUseProgram",
    "glValidateProgram",
    "glVertexAttrib1f",
    "glVertexAttrib1fv",
    "glVertexAttrib2f",
    "glVertexAttrib2fv",
    "glVertexAttrib3f",
    "glVertexAttrib3fv",
    "glVertexAttrib4f",
    "glVertexAttrib4fv",
    "glVertexAttribPointer",
    "glViewport",
	"glDebugMessageControlKHR",
	"glDebugMessageInsertKHR",
	"glDebugMessageCallbackKHR",
	"glGetDebugMessageLogKHR",
};

#ifdef XASH_ANGLE
void (*eglGetProcAddress(const char *procname))();
#elif defined REF_DLL
extern void *GL_GetProcAddress( const char *name );
#elif defined XASH_SDL
void *SDL_GL_GetProcAddress( const char*);
#endif
GLvoid
wes_init(const char *gles2)
{
    int i;
    void** ptr;

    LOGI("Start wes_init()");

    wes_gl = (gles2lib_t*) malloc(sizeof(gles2lib_t));
    if (wes_gl == NULL)
    {
        LOGE("Could not load Allocate mem: %s", gles2);
    }

	LOGI("Memory alloc wes_init()");
#if !defined XASH_SDL && !defined REF_DLL
	wes_libhandle = dlopen(gles2, RTLD_NOW | RTLD_LOCAL);//RTLD_LAZY | RTLD_GLOBAL);

	if (wes_libhandle == NULL)
    {
        LOGE("Could not load OpenGL ES 2 runtime library: %s", gles2);
    }
#endif
	LOGI("lib loaded wes_init()");

    ptr = (void**) wes_gl;
    for(i = 0; i != WES_OGLESV2_FUNCTIONCOUNT+1; i++)
    {
#ifdef XASH_ANGLE
        void* pfunc = (void*)eglGetProcAddress(glfuncnames[i]);
#elif defined REF_DLL
		void* pfunc = GL_GetProcAddress(glfuncnames[i]);
#elif defined XASH_SDL
		void* pfunc = (void*) SDL_GL_GetProcAddress(glfuncnames[i]);
#else
		void* pfunc = (void*) dlsym(wes_libhandle, glfuncnames[i]);
#endif
        if (pfunc == NULL)
			LOGE("Could not find %s in %s", glfuncnames[i], gles2 );
		else LOGI("Loaded %s", glfuncnames[i]);

        *ptr++ = pfunc;
    }

	LOGI("methods loaded wes_init()");

    wes_shader_init();
	LOGI("wes_shader_init()");

    wes_matrix_init();
	LOGI("wes_matrix_init()");

    wes_begin_init();
	LOGI("wes_begin_init()");

    wes_state_init();

    LOGI("Finished wes_init()");
}

GLvoid
wes_destroy( void )
{
    dlclose(wes_libhandle);
    wes_shader_destroy();
    wes_begin_destroy();
}


GLvoid
GL_MANGLE(glMultiDrawArrays)(GLenum mode, GLint *first, GLsizei *count, GLsizei primcount)
{
    GLuint i;
    wes_vertbuffer_flush();
    for (i = 0; i < (unsigned)primcount; i++) {
        if (count[i] > 0){
            wes_gl->glDrawArrays(mode, first[i], count[i]);
        }
    }
}

GLvoid
GL_MANGLE(glMultiDrawElements)(GLenum mode, GLsizei *count, GLenum type, GLvoid **indices, GLsizei primcount)
{
    GLuint i;
    wes_vertbuffer_flush();
    for (i = 0; i < (unsigned)primcount; i++) {
        if (count[i] > 0){
            wes_gl->glDrawElements(mode, count[i], type, indices[i]);
        }
    }
}

GLvoid
GL_MANGLE(glScissor)(GLint x, GLint y, GLsizei width, GLsizei height)
{
	wes_gl->glScissor(x, y, width, height);
}
