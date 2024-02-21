#ifndef GL_STATIC_H
#define GL_STATIC_H

#if XASH_GL_STATIC
#if defined(XASH_WES)
#define GL_MANGLE( x ) p##x
#elif defined(XASH_GL4ES)
#define GL_MANGLE( x ) gl4es_##x
#else
#define GL_MANGLE( x ) x
#endif
#else
#define GL_MANGLE( x ) ( APIENTRY *p##x )
#endif

extern "C" {
// helper opengl functions
#define GL_API(R, NAME, ...) R GL_MANGLE(NAME)(__VA_ARGS__);
#include "gl_iter.h"
};
// mangle to xash3d style
#ifndef WES_MANGLE_PREPEND

// ES2 1.0
#define pglBindTexture GL_MANGLE(glBindTexture)
#define pglBlendFunc GL_MANGLE(glBlendFunc)
#define pglClear GL_MANGLE(glClear)
#define pglClearColor GL_MANGLE(glClearColor)
#define pglClearDepthf GL_MANGLE(glClearDepthf)
#define pglClearStencil GL_MANGLE(glClearStencil)
#define pglColorMask GL_MANGLE(glColorMask)
#define pglCopyTexImage2D GL_MANGLE(glCopyTexImage2D)
#define pglCopyTexSubImage2D GL_MANGLE(glCopyTexSubImage2D)
#define pglCullFace GL_MANGLE(glCullFace)
#define pglDeleteTextures GL_MANGLE(glDeleteTextures)
#define pglDepthFunc GL_MANGLE(glDepthFunc)
#define pglDepthMask GL_MANGLE(glDepthMask)
#define pglDepthRange GL_MANGLE(glDepthRangef)
#define pglDisable GL_MANGLE(glDisable)
#define pglEnable GL_MANGLE(glEnable)
#define pglFinish GL_MANGLE(glFinish)
#define pglFlush GL_MANGLE(glFlush)
#define pglFrontFace GL_MANGLE(glFrontFace)
#define pglGenTextures GL_MANGLE(glGenTextures)
#define pglGetBooleanv GL_MANGLE(glGetBooleanv)
#define pglGetDoublev GL_MANGLE(glGetDoublev)
#define pglGetFloatv GL_MANGLE(glGetFloatv)
#define pglGetIntegerv GL_MANGLE(glGetIntegerv)
#define pglGetError GL_MANGLE(glGetError)
#define pglGetString GL_MANGLE(glGetString)
#define pglGetTexParameterfv GL_MANGLE(glGetTexParameterfv)
#define pglGetTexParameteriv GL_MANGLE(glGetTexParameteriv)
#define pglHint GL_MANGLE(glHint)
#define pglIsEnabled GL_MANGLE(glIsEnabled)
#define pglIsTexture GL_MANGLE(glIsTexture)
#define pglLineWidth GL_MANGLE(glLineWidth)
#define pglPixelStorei GL_MANGLE(glPixelStorei)
#define pglPolygonOffset GL_MANGLE(glPolygonOffset)
#define pglReadPixels GL_MANGLE(glReadPixels)
#define pglScissor GL_MANGLE(glScissor)
#define pglStencilFunc GL_MANGLE(glStencilFunc)
#define pglStencilMask GL_MANGLE(glStencilMask)
#define pglStencilOp GL_MANGLE(glStencilOp)
#define pglTexImage2D GL_MANGLE(glTexImage2D)
#define pglTexParameterf GL_MANGLE(glTexParameterf)
#define pglTexParameterfv GL_MANGLE(glTexParameterfv)
#define pglTexParameteri GL_MANGLE(glTexParameteri)
#define pglTexParameteriv GL_MANGLE(glTexParameteriv)
#define pglTexSubImage2D GL_MANGLE(glTexSubImage2D)
#define pglViewport GL_MANGLE(glViewport)

// ES2 1.1 vertex array
#define pglDrawArrays GL_MANGLE(glDrawArrays)
#define pglDrawElements GL_MANGLE(glDrawElements)

// ES2 1.3 multisample
#define pglSampleCoverageARB GL_MANGLE(glSampleCoverage)

// ES2 1.3 multitexture
#define pglActiveTextureARB GL_MANGLE(glActiveTexture)

// ES2 1.3 compressed texture
#define pglCompressedTexImage2DARB GL_MANGLE(glCompressedTexImage2D)
#define pglCompressedTexSubImage2DARB GL_MANGLE(glCompressedTexSubImage2D)

// ES2 1.4 blend separate
#define pglBlendColor GL_MANGLE(glBlendColor)
#define pglBlendEquation GL_MANGLE(glBlendEquation)

// ES2 1.5 VBO
#define pglBindBufferARB GL_MANGLE(glBindBuffer)
#define pglBufferDataARB GL_MANGLE(glBufferData)
#define pglBufferSubDataARB GL_MANGLE(glBufferSubData)
#define pglDeleteBuffersARB GL_MANGLE(glDeleteBuffers)
#define pglGenBuffersARB GL_MANGLE(glGenBuffers)
#define pglGetBufferParameterivARB GL_MANGLE(glGetBufferParameteriv)
#define pglIsBufferARB GL_MANGLE(glIsBuffer)

// ES2 2.0 blend separate
#define pglBlendEquationSeparate GL_MANGLE(glBlendEquationSeparate)
#define pglBlendFuncSeparate GL_MANGLE(glBlendFuncSeparate)

// ES2 2.0 stencil separate
#define pglStencilFuncSeparate GL_MANGLE(glStencilFuncSeparate)
#define pglStencilMaskSeparate GL_MANGLE(glStencilMaskSeparate)
#define pglStencilOpSeparate GL_MANGLE(glStencilOpSeparate)

// ES2 2.0 Shader
#define pglAttachObjectARB GL_MANGLE(glAttachShader)
#define glBindAttribLocationARB GL_MANGLE(glBindAttribLocation)
#define pglCompileShaderARB GL_MANGLE(glCompileShader)
#define pglCreateProgramObjectARB GL_MANGLE(glCreateProgram)
#define pglCreateShaderObjectARB GL_MANGLE(glCreateShader)
#define pglDeleteProgram GL_MANGLE(glDeleteProgram)
#define pglDeleteObjectARB GL_MANGLE(glDeleteShader)
#define pglDetachObjectARB GL_MANGLE(glDetachShader)
#define pglDisableVertexAttribArrayARB GL_MANGLE(glDisableVertexAttribArray)
#define pglEnableVertexAttribArrayARB GL_MANGLE(glEnableVertexAttribArray)
#define pglGetActiveAttribARB GL_MANGLE(glGetActiveAttrib)
#define pglGetActiveUniformARB GL_MANGLE(glGetActiveUniform)
#define pglGetAttachedObjectsARB GL_MANGLE(glGetAttachedObjects)
#define pglGetAttribLocationARB GL_MANGLE(glGetAttribLocation)
#define pglGetProgramInfoLog GL_MANGLE(glGetProgramInfoLog)
#define pglGetProgramiv GL_MANGLE(glGetProgramiv)
#define pglGetShaderSourceARB GL_MANGLE(glGetShaderSource)
#define pglGetShaderInfoLog GL_MANGLE(glGetShaderInfoLog)
#define pglGetShaderiv GL_MANGLE(glGetShaderiv)
#define pglGetUniformfvARB GL_MANGLE(glGetUniformfv)
#define pglGetUniformivARB GL_MANGLE(glGetUniformiv)
#define pglGetUniformLocationARB GL_MANGLE(glGetUniformLocation)
#define pglGetVertexAttribfvARB GL_MANGLE(glGetVertexAttribfv)
#define pglGetVertexAttribivARB GL_MANGLE(glGetVertexAttribiv)
#define pglGetVertexAttribPointervARB GL_MANGLE(glGetVertexAttribPointerv)
#define pglIsProgram GL_MANGLE(glIsProgram)
#define pglIsShader GL_MANGLE(glIsShader)
#define pglLinkProgramARB GL_MANGLE(glLinkProgram)
#define pglReleaseShaderCompiler GL_MANGLE(glReleaseShaderCompiler)
#define pglShaderBinary GL_MANGLE(glShaderBinary)
#define pglShaderSourceARB GL_MANGLE(glShaderSource)
#define pglUniform1fARB GL_MANGLE(glUniform1f)
#define pglUniform2fARB GL_MANGLE(glUniform2f)
#define pglUniform3fARB GL_MANGLE(glUniform3f)
#define pglUniform4fARB GL_MANGLE(glUniform4f)
#define pglUniform1iARB GL_MANGLE(glUniform1i)
#define pglUniform2iARB GL_MANGLE(glUniform2i)
#define pglUniform3iARB GL_MANGLE(glUniform3i)
#define pglUniform4iARB GL_MANGLE(glUniform4i)
#define pglUniform1fvARB GL_MANGLE(glUniform1fv)
#define pglUniform2fvARB GL_MANGLE(glUniform2fv)
#define pglUniform3fvARB GL_MANGLE(glUniform3fv)
#define pglUniform4fvARB GL_MANGLE(glUniform4fv)
#define pglUniform1ivARB GL_MANGLE(glUniform1iv)
#define pglUniform2ivARB GL_MANGLE(glUniform2iv)
#define pglUniform3ivARB GL_MANGLE(glUniform3iv)
#define pglUniform4ivARB GL_MANGLE(glUniform4iv)
#define pglUniformMatrix2fvARB GL_MANGLE(glUniformMatrix2fv)
#define pglUniformMatrix3fvARB GL_MANGLE(glUniformMatrix3fv)
#define pglUniformMatrix4fvARB GL_MANGLE(glUniformMatrix4fv)
#define pglUseProgramObjectARB GL_MANGLE(glUseProgram)
#define pglValidateProgramARB GL_MANGLE(glValidateProgram)
#define pglVertexAttrib1fARB GL_MANGLE(glVertexAttrib1f)
#define pglVertexAttrib2fARB GL_MANGLE(glVertexAttrib2f)
#define pglVertexAttrib3fARB GL_MANGLE(glVertexAttrib3f)
#define pglVertexAttrib4fARB GL_MANGLE(glVertexAttrib4f)
#define pglVertexAttrib1fvARB GL_MANGLE(glVertexAttrib1fv)
#define pglVertexAttrib2fvARB GL_MANGLE(glVertexAttrib2fv)
#define pglVertexAttrib3fvARB GL_MANGLE(glVertexAttrib3fv)
#define pglVertexAttrib4fvARB GL_MANGLE(glVertexAttrib4fv)
#define pglVertexAttribPointerARB GL_MANGLE(glVertexAttribPointer)

// ES2 3.0 FBO
#define pglBindFramebuffer GL_MANGLE(glBindFramebuffer)
#define pglBindRenderbuffer GL_MANGLE(glBindRenderbuffer)
#define pglCheckFramebufferStatus GL_MANGLE(glCheckFramebufferStatus)
#define pglDeleteFramebuffers GL_MANGLE(glDeleteFramebuffers)
#define pglDeleteRenderbuffers GL_MANGLE(glDeleteRenderbuffers)
#define pglFramebufferRenderbuffer GL_MANGLE(glFramebufferRenderbuffer)
#define pglFramebufferTexture2D GL_MANGLE(glFramebufferTexture2D)
#define pglGenFramebuffers GL_MANGLE(glGenFramebuffers)
#define pglGenRenderbuffers GL_MANGLE(glGenRenderbuffers)
#define pglGetFramebufferAttachmentParameteriv GL_MANGLE(glGetFramebufferAttachmentParameteriv)
#define pglGetRenderbufferParameteriv GL_MANGLE(glGetRenderbufferParameteriv)
#define pglIsFramebuffer GL_MANGLE(glIsFramebuffer)
#define pglIsRenderbuffer GL_MANGLE(glIsRenderbuffer)
#define pglRenderbufferStorage GL_MANGLE(glRenderbufferStorage)

// ES2 3.0 mipmap
#define pglGenerateMipmap GL_MANGLE(glGenerateMipmap)

// ES3 1.2 draw range elements
#define pglDrawRangeElements GL_MANGLE(glDrawRangeElements)

// ES3 1.5 VBO map buffer
#define pglMapBufferARB GL_MANGLE(glMapBufferARB)
#define pglUnmapBufferARB GL_MANGLE(glUnmapBufferARB)

// ES3 3.0 UBO
#define pglBindBufferRange GL_MANGLE(glBindBufferRange)
#define pglBindBufferBase GL_MANGLE(glBindBufferBase)
#define pglGetUniformBlockIndex GL_MANGLE(glGetUniformBlockIndex)
#define pglUniformBlockBinding GL_MANGLE(glUniformBlockBinding)

// ES3 3.0 VAO
#define pglBindVertexArray GL_MANGLE(glBindVertexArray)
#define pglDeleteVertexArrays GL_MANGLE(glDeleteVertexArrays)
#define pglGenVertexArrays GL_MANGLE(glGenVertexArrays)
#define pglIsVertexArray GL_MANGLE(glIsVertexArray)

#endif

#endif // GL_STATIC_H

