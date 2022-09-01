#ifndef GL_STATIC_H
#define GL_STATIC_H

#if defined(XASH_WES)
#define GL_MANGLE( x ) p##x
#elif defined(XASH_GL4ES)
#define GL_MANGLE( x ) gl4es_##x
#else
#define GL_MANGLE( x ) x
#endif

extern "C" {
// helper opengl functions
GLenum GL_MANGLE(glGetError)(void);
const GLubyte *GL_MANGLE( glGetString)(GLenum name);

// base gl functions
void GL_MANGLE(glAccum)(GLenum op, GLfloat value);
void GL_MANGLE(glAlphaFunc)(GLenum func, GLclampf ref);
void GL_MANGLE(glArrayElement)(GLint i);
void GL_MANGLE(glBegin)(GLenum mode);
void GL_MANGLE(glBindTexture)(GLenum target, GLuint texture);
void GL_MANGLE(glBitmap)(GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap);
void GL_MANGLE(glBlendFunc)(GLenum sfactor, GLenum dfactor);
void GL_MANGLE(glCallList)(GLuint list);
void GL_MANGLE(glCallLists)(GLsizei n, GLenum type, const GLvoid *lists);
void GL_MANGLE(glClear)(GLbitfield mask);
void GL_MANGLE(glClearAccum)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
void GL_MANGLE(glClearColor)(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
void GL_MANGLE(glClearDepth)(GLclampd depth);
void GL_MANGLE(glClearIndex)(GLfloat c);
void GL_MANGLE(glClearStencil)(GLint s);
GLboolean GL_MANGLE(glIsEnabled)( GLenum cap );
GLboolean GL_MANGLE(glIsList)( GLuint list );
GLboolean GL_MANGLE(glIsTexture)( GLuint texture );
void GL_MANGLE(glClipPlane)(GLenum plane, const GLdouble *equation);
void GL_MANGLE(glColor3b)(GLbyte red, GLbyte green, GLbyte blue);
void GL_MANGLE(glColor3bv)(const GLbyte *v);
void GL_MANGLE(glColor3d)(GLdouble red, GLdouble green, GLdouble blue);
void GL_MANGLE(glColor3dv)(const GLdouble *v);
void GL_MANGLE(glColor3f)(GLfloat red, GLfloat green, GLfloat blue);
void GL_MANGLE(glColor3fv)(const GLfloat *v);
void GL_MANGLE(glColor3i)(GLint red, GLint green, GLint blue);
void GL_MANGLE(glColor3iv)(const GLint *v);
void GL_MANGLE(glColor3s)(GLshort red, GLshort green, GLshort blue);
void GL_MANGLE(glColor3sv)(const GLshort *v);
void GL_MANGLE(glColor3ub)(GLubyte red, GLubyte green, GLubyte blue);
void GL_MANGLE(glColor3ubv)(const GLubyte *v);
void GL_MANGLE(glColor3ui)(GLuint red, GLuint green, GLuint blue);
void GL_MANGLE(glColor3uiv)(const GLuint *v);
void GL_MANGLE(glColor3us)(GLushort red, GLushort green, GLushort blue);
void GL_MANGLE(glColor3usv)(const GLushort *v);
void GL_MANGLE(glColor4b)(GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha);
void GL_MANGLE(glColor4bv)(const GLbyte *v);
void GL_MANGLE(glColor4d)(GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha);
void GL_MANGLE(glColor4dv)(const GLdouble *v);
void GL_MANGLE(glColor4f)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
void GL_MANGLE(glColor4fv)(const GLfloat *v);
void GL_MANGLE(glColor4i)(GLint red, GLint green, GLint blue, GLint alpha);
void GL_MANGLE(glColor4iv)(const GLint *v);
void GL_MANGLE(glColor4s)(GLshort red, GLshort green, GLshort blue, GLshort alpha);
void GL_MANGLE(glColor4sv)(const GLshort *v);
void GL_MANGLE(glColor4ub)(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);
void GL_MANGLE(glColor4ubv)(const GLubyte *v);
void GL_MANGLE(glColor4ui)(GLuint red, GLuint green, GLuint blue, GLuint alpha);
void GL_MANGLE(glColor4uiv)(const GLuint *v);
void GL_MANGLE(glColor4us)(GLushort red, GLushort green, GLushort blue, GLushort alpha);
void GL_MANGLE(glColor4usv)(const GLushort *v);
void GL_MANGLE(glColorMask)(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
void GL_MANGLE(glColorMaterial)(GLenum face, GLenum mode);
void GL_MANGLE(glColorPointer)(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
void GL_MANGLE(glCopyPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum type);
void GL_MANGLE(glCopyTexImage1D)(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLint border);
void GL_MANGLE(glCopyTexImage2D)(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
void GL_MANGLE(glCopyTexSubImage1D)(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
void GL_MANGLE(glCopyTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
void GL_MANGLE(glCullFace)(GLenum mode);
void GL_MANGLE(glDeleteLists)(GLuint list, GLsizei range);
void GL_MANGLE(glDeleteTextures)(GLsizei n, const GLuint *textures);
void GL_MANGLE(glDepthFunc)(GLenum func);
void GL_MANGLE(glDepthMask)(GLboolean flag);
void GL_MANGLE(glDepthRange)(GLclampd zNear, GLclampd zFar);
void GL_MANGLE(glDisable)(GLenum cap);
void GL_MANGLE(glDisableClientState)(GLenum array);
void GL_MANGLE(glDrawArrays)(GLenum mode, GLint first, GLsizei count);
void GL_MANGLE(glDrawBuffer)(GLenum mode);
void GL_MANGLE(glDrawElements)(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);
void GL_MANGLE(glDrawPixels)(GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
void GL_MANGLE(glEdgeFlag)(GLboolean flag);
void GL_MANGLE(glEdgeFlagPointer)(GLsizei stride, const GLvoid *pointer);
void GL_MANGLE(glEdgeFlagv)(const GLboolean *flag);
void GL_MANGLE(glEnable)(GLenum cap);
void GL_MANGLE(glEnableClientState)(GLenum array);
void GL_MANGLE(glEnd)(void);
void GL_MANGLE(glEndList)(void);
void GL_MANGLE(glEvalCoord1d)(GLdouble u);
void GL_MANGLE(glEvalCoord1dv)(const GLdouble *u);
void GL_MANGLE(glEvalCoord1f)(GLfloat u);
void GL_MANGLE(glEvalCoord1fv)(const GLfloat *u);
void GL_MANGLE(glEvalCoord2d)(GLdouble u, GLdouble v);
void GL_MANGLE(glEvalCoord2dv)(const GLdouble *u);
void GL_MANGLE(glEvalCoord2f)(GLfloat u, GLfloat v);
void GL_MANGLE(glEvalCoord2fv)(const GLfloat *u);
void GL_MANGLE(glEvalMesh1)(GLenum mode, GLint i1, GLint i2);
void GL_MANGLE(glEvalMesh2)(GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2);
void GL_MANGLE(glEvalPoint1)(GLint i);
void GL_MANGLE(glEvalPoint2)(GLint i, GLint j);
void GL_MANGLE(glFeedbackBuffer)(GLsizei size, GLenum type, GLfloat *buffer);
void GL_MANGLE(glFinish)(void);
void GL_MANGLE(glFlush)(void);
void GL_MANGLE(glFogf)(GLenum pname, GLfloat param);
void GL_MANGLE(glFogfv)(GLenum pname, const GLfloat *params);
void GL_MANGLE(glFogi)(GLenum pname, GLint param);
void GL_MANGLE(glFogiv)(GLenum pname, const GLint *params);
void GL_MANGLE(glFrontFace)(GLenum mode);
void GL_MANGLE(glFrustum)(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
void GL_MANGLE(glGenTextures)(GLsizei n, GLuint *textures);
void GL_MANGLE(glGetBooleanv)(GLenum pname, GLboolean *params);
void GL_MANGLE(glGetClipPlane)(GLenum plane, GLdouble *equation);
void GL_MANGLE(glGetDoublev)(GLenum pname, GLdouble *params);
void GL_MANGLE(glGetFloatv)(GLenum pname, GLfloat *params);
void GL_MANGLE(glGetIntegerv)(GLenum pname, GLint *params);
void GL_MANGLE(glGetLightfv)(GLenum light, GLenum pname, GLfloat *params);
void GL_MANGLE(glGetLightiv)(GLenum light, GLenum pname, GLint *params);
void GL_MANGLE(glGetMapdv)(GLenum target, GLenum query, GLdouble *v);
void GL_MANGLE(glGetMapfv)(GLenum target, GLenum query, GLfloat *v);
void GL_MANGLE(glGetMapiv)(GLenum target, GLenum query, GLint *v);
void GL_MANGLE(glGetMaterialfv)(GLenum face, GLenum pname, GLfloat *params);
void GL_MANGLE(glGetMaterialiv)(GLenum face, GLenum pname, GLint *params);
void GL_MANGLE(glGetPixelMapfv)(GLenum map, GLfloat *values);
void GL_MANGLE(glGetPixelMapuiv)(GLenum map, GLuint *values);
void GL_MANGLE(glGetPixelMapusv)(GLenum map, GLushort *values);
void GL_MANGLE(glGetPointerv)(GLenum pname, GLvoid* *params);

void GL_MANGLE(glGetPolygonStipple)(GLubyte *mask);
void GL_MANGLE(glGetTexEnvfv)(GLenum target, GLenum pname, GLfloat *params);
void GL_MANGLE(glGetTexEnviv)(GLenum target, GLenum pname, GLint *params);
void GL_MANGLE(glGetTexGendv)(GLenum coord, GLenum pname, GLdouble *params);
void GL_MANGLE(glGetTexGenfv)(GLenum coord, GLenum pname, GLfloat *params);
void GL_MANGLE(glGetTexGeniv)(GLenum coord, GLenum pname, GLint *params);
void GL_MANGLE(glGetTexImage)(GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels);
void GL_MANGLE(glGetTexLevelParameterfv)(GLenum target, GLint level, GLenum pname, GLfloat *params);
void GL_MANGLE(glGetTexLevelParameteriv)(GLenum target, GLint level, GLenum pname, GLint *params);
void GL_MANGLE(glGetTexParameterfv)(GLenum target, GLenum pname, GLfloat *params);
void GL_MANGLE(glGetTexParameteriv)(GLenum target, GLenum pname, GLint *params);
void GL_MANGLE(glHint)(GLenum target, GLenum mode);
void GL_MANGLE(glIndexMask)(GLuint mask);
void GL_MANGLE(glIndexPointer)(GLenum type, GLsizei stride, const GLvoid *pointer);
void GL_MANGLE(glIndexd)(GLdouble c);
void GL_MANGLE(glIndexdv)(const GLdouble *c);
void GL_MANGLE(glIndexf)(GLfloat c);
void GL_MANGLE(glIndexfv)(const GLfloat *c);
void GL_MANGLE(glIndexi)(GLint c);
void GL_MANGLE(glIndexiv)(const GLint *c);
void GL_MANGLE(glIndexs)(GLshort c);
void GL_MANGLE(glIndexsv)(const GLshort *c);
void GL_MANGLE(glIndexub)(GLubyte c);
void GL_MANGLE(glIndexubv)(const GLubyte *c);
void GL_MANGLE(glInitNames)(void);
void GL_MANGLE(glInterleavedArrays)(GLenum format, GLsizei stride, const GLvoid *pointer);
void GL_MANGLE(glLightModelf)(GLenum pname, GLfloat param);
void GL_MANGLE(glLightModelfv)(GLenum pname, const GLfloat *params);
void GL_MANGLE(glLightModeli)(GLenum pname, GLint param);
void GL_MANGLE(glLightModeliv)(GLenum pname, const GLint *params);
void GL_MANGLE(glLightf)(GLenum light, GLenum pname, GLfloat param);
void GL_MANGLE(glLightfv)(GLenum light, GLenum pname, const GLfloat *params);
void GL_MANGLE(glLighti)(GLenum light, GLenum pname, GLint param);
void GL_MANGLE(glLightiv)(GLenum light, GLenum pname, const GLint *params);
void GL_MANGLE(glLineStipple)(GLint factor, GLushort pattern);
void GL_MANGLE(glLineWidth)(GLfloat width);
void GL_MANGLE(glListBase)(GLuint base);
void GL_MANGLE(glLoadIdentity)(void);
void GL_MANGLE(glLoadMatrixd)(const GLdouble *m);
void GL_MANGLE(glLoadMatrixf)(const GLfloat *m);
void GL_MANGLE(glLoadName)(GLuint name);
void GL_MANGLE(glLogicOp)(GLenum opcode);
void GL_MANGLE(glMap1d)(GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points);
void GL_MANGLE(glMap1f)(GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points);
void GL_MANGLE(glMap2d)(GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points);
void GL_MANGLE(glMap2f)(GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points);
void GL_MANGLE(glMapGrid1d)(GLint un, GLdouble u1, GLdouble u2);
void GL_MANGLE(glMapGrid1f)(GLint un, GLfloat u1, GLfloat u2);
void GL_MANGLE(glMapGrid2d)(GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2);
void GL_MANGLE(glMapGrid2f)(GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2);
void GL_MANGLE(glMaterialf)(GLenum face, GLenum pname, GLfloat param);
void GL_MANGLE(glMaterialfv)(GLenum face, GLenum pname, const GLfloat *params);
void GL_MANGLE(glMateriali)(GLenum face, GLenum pname, GLint param);
void GL_MANGLE(glMaterialiv)(GLenum face, GLenum pname, const GLint *params);
void GL_MANGLE(glMatrixMode)(GLenum mode);
void GL_MANGLE(glMultMatrixd)(const GLdouble *m);
void GL_MANGLE(glMultMatrixf)(const GLfloat *m);
void GL_MANGLE(glNewList)(GLuint list, GLenum mode);
void GL_MANGLE(glNormal3b)(GLbyte nx, GLbyte ny, GLbyte nz);
void GL_MANGLE(glNormal3bv)(const GLbyte *v);
void GL_MANGLE(glNormal3d)(GLdouble nx, GLdouble ny, GLdouble nz);
void GL_MANGLE(glNormal3dv)(const GLdouble *v);
void GL_MANGLE(glNormal3f)(GLfloat nx, GLfloat ny, GLfloat nz);
void GL_MANGLE(glNormal3fv)(const GLfloat *v);
void GL_MANGLE(glNormal3i)(GLint nx, GLint ny, GLint nz);
void GL_MANGLE(glNormal3iv)(const GLint *v);
void GL_MANGLE(glNormal3s)(GLshort nx, GLshort ny, GLshort nz);
void GL_MANGLE(glNormal3sv)(const GLshort *v);
void GL_MANGLE(glNormalPointer)(GLenum type, GLsizei stride, const GLvoid *pointer);
void GL_MANGLE(glOrtho)(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
void GL_MANGLE(glPassThrough)(GLfloat token);
void GL_MANGLE(glPixelMapfv)(GLenum map, GLsizei mapsize, const GLfloat *values);
void GL_MANGLE(glPixelMapuiv)(GLenum map, GLsizei mapsize, const GLuint *values);
void GL_MANGLE(glPixelMapusv)(GLenum map, GLsizei mapsize, const GLushort *values);
void GL_MANGLE(glPixelStoref)(GLenum pname, GLfloat param);
void GL_MANGLE(glPixelStorei)(GLenum pname, GLint param);
void GL_MANGLE(glPixelTransferf)(GLenum pname, GLfloat param);
void GL_MANGLE(glPixelTransferi)(GLenum pname, GLint param);
void GL_MANGLE(glPixelZoom)(GLfloat xfactor, GLfloat yfactor);
void GL_MANGLE(glPointSize)(GLfloat size);
void GL_MANGLE(glPolygonMode)(GLenum face, GLenum mode);
void GL_MANGLE(glPolygonOffset)(GLfloat factor, GLfloat units);
void GL_MANGLE(glPolygonStipple)(const GLubyte *mask);
void GL_MANGLE(glPopAttrib)(void);
void GL_MANGLE(glPopClientAttrib)(void);
void GL_MANGLE(glPopMatrix)(void);
void GL_MANGLE(glPopName)(void);
void GL_MANGLE(glPushAttrib)(GLbitfield mask);
void GL_MANGLE(glPushClientAttrib)(GLbitfield mask);
void GL_MANGLE(glPushMatrix)(void);
void GL_MANGLE(glPushName)(GLuint name);
void GL_MANGLE(glRasterPos2d)(GLdouble x, GLdouble y);
void GL_MANGLE(glRasterPos2dv)(const GLdouble *v);
void GL_MANGLE(glRasterPos2f)(GLfloat x, GLfloat y);
void GL_MANGLE(glRasterPos2fv)(const GLfloat *v);
void GL_MANGLE(glRasterPos2i)(GLint x, GLint y);
void GL_MANGLE(glRasterPos2iv)(const GLint *v);
void GL_MANGLE(glRasterPos2s)(GLshort x, GLshort y);
void GL_MANGLE(glRasterPos2sv)(const GLshort *v);
void GL_MANGLE(glRasterPos3d)(GLdouble x, GLdouble y, GLdouble z);
void GL_MANGLE(glRasterPos3dv)(const GLdouble *v);
void GL_MANGLE(glRasterPos3f)(GLfloat x, GLfloat y, GLfloat z);
void GL_MANGLE(glRasterPos3fv)(const GLfloat *v);
void GL_MANGLE(glRasterPos3i)(GLint x, GLint y, GLint z);
void GL_MANGLE(glRasterPos3iv)(const GLint *v);
void GL_MANGLE(glRasterPos3s)(GLshort x, GLshort y, GLshort z);
void GL_MANGLE(glRasterPos3sv)(const GLshort *v);
void GL_MANGLE(glRasterPos4d)(GLdouble x, GLdouble y, GLdouble z, GLdouble w);
void GL_MANGLE(glRasterPos4dv)(const GLdouble *v);
void GL_MANGLE(glRasterPos4f)(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
void GL_MANGLE(glRasterPos4fv)(const GLfloat *v);
void GL_MANGLE(glRasterPos4i)(GLint x, GLint y, GLint z, GLint w);
void GL_MANGLE(glRasterPos4iv)(const GLint *v);
void GL_MANGLE(glRasterPos4s)(GLshort x, GLshort y, GLshort z, GLshort w);
void GL_MANGLE(glRasterPos4sv)(const GLshort *v);
void GL_MANGLE(glReadBuffer)(GLenum mode);
void GL_MANGLE(glReadPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels);
void GL_MANGLE(glRectd)(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2);
void GL_MANGLE(glRectdv)(const GLdouble *v1, const GLdouble *v2);
void GL_MANGLE(glRectf)(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);
void GL_MANGLE(glRectfv)(const GLfloat *v1, const GLfloat *v2);
void GL_MANGLE(glRecti)(GLint x1, GLint y1, GLint x2, GLint y2);
void GL_MANGLE(glRectiv)(const GLint *v1, const GLint *v2);
void GL_MANGLE(glRects)(GLshort x1, GLshort y1, GLshort x2, GLshort y2);
void GL_MANGLE(glRectsv)(const GLshort *v1, const GLshort *v2);
void GL_MANGLE(glRotated)(GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
void GL_MANGLE(glRotatef)(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
void GL_MANGLE(glScaled)(GLdouble x, GLdouble y, GLdouble z);
void GL_MANGLE(glScalef)(GLfloat x, GLfloat y, GLfloat z);
void GL_MANGLE(glScissor)(GLint x, GLint y, GLsizei width, GLsizei height);
void GL_MANGLE(glSelectBuffer)(GLsizei size, GLuint *buffer);
void GL_MANGLE(glShadeModel)(GLenum mode);
void GL_MANGLE(glStencilFunc)(GLenum func, GLint ref, GLuint mask);
void GL_MANGLE(glStencilMask)(GLuint mask);
void GL_MANGLE(glStencilOp)(GLenum fail, GLenum zfail, GLenum zpass);
void GL_MANGLE(glTexCoord1d)(GLdouble s);
void GL_MANGLE(glTexCoord1dv)(const GLdouble *v);
void GL_MANGLE(glTexCoord1f)(GLfloat s);
void GL_MANGLE(glTexCoord1fv)(const GLfloat *v);
void GL_MANGLE(glTexCoord1i)(GLint s);
void GL_MANGLE(glTexCoord1iv)(const GLint *v);
void GL_MANGLE(glTexCoord1s)(GLshort s);
void GL_MANGLE(glTexCoord1sv)(const GLshort *v);
void GL_MANGLE(glTexCoord2d)(GLdouble s, GLdouble t);
void GL_MANGLE(glTexCoord2dv)(const GLdouble *v);
void GL_MANGLE(glTexCoord2f)(GLfloat s, GLfloat t);
void GL_MANGLE(glTexCoord2fv)(const GLfloat *v);
void GL_MANGLE(glTexCoord2i)(GLint s, GLint t);
void GL_MANGLE(glTexCoord2iv)(const GLint *v);
void GL_MANGLE(glTexCoord2s)(GLshort s, GLshort t);
void GL_MANGLE(glTexCoord2sv)(const GLshort *v);
void GL_MANGLE(glTexCoord3d)(GLdouble s, GLdouble t, GLdouble r);
void GL_MANGLE(glTexCoord3dv)(const GLdouble *v);
void GL_MANGLE(glTexCoord3f)(GLfloat s, GLfloat t, GLfloat r);
void GL_MANGLE(glTexCoord3fv)(const GLfloat *v);
void GL_MANGLE(glTexCoord3i)(GLint s, GLint t, GLint r);
void GL_MANGLE(glTexCoord3iv)(const GLint *v);
void GL_MANGLE(glTexCoord3s)(GLshort s, GLshort t, GLshort r);
void GL_MANGLE(glTexCoord3sv)(const GLshort *v);
void GL_MANGLE(glTexCoord4d)(GLdouble s, GLdouble t, GLdouble r, GLdouble q);
void GL_MANGLE(glTexCoord4dv)(const GLdouble *v);
void GL_MANGLE(glTexCoord4f)(GLfloat s, GLfloat t, GLfloat r, GLfloat q);
void GL_MANGLE(glTexCoord4fv)(const GLfloat *v);
void GL_MANGLE(glTexCoord4i)(GLint s, GLint t, GLint r, GLint q);
void GL_MANGLE(glTexCoord4iv)(const GLint *v);
void GL_MANGLE(glTexCoord4s)(GLshort s, GLshort t, GLshort r, GLshort q);
void GL_MANGLE(glTexCoord4sv)(const GLshort *v);
void GL_MANGLE(glTexCoordPointer)(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
void GL_MANGLE(glTexEnvf)(GLenum target, GLenum pname, GLfloat param);
void GL_MANGLE(glTexEnvfv)(GLenum target, GLenum pname, const GLfloat *params);
void GL_MANGLE(glTexEnvi)(GLenum target, GLenum pname, GLint param);
void GL_MANGLE(glTexEnviv)(GLenum target, GLenum pname, const GLint *params);
void GL_MANGLE(glTexGend)(GLenum coord, GLenum pname, GLdouble param);
void GL_MANGLE(glTexGendv)(GLenum coord, GLenum pname, const GLdouble *params);
void GL_MANGLE(glTexGenf)(GLenum coord, GLenum pname, GLfloat param);
void GL_MANGLE(glTexGenfv)(GLenum coord, GLenum pname, const GLfloat *params);
void GL_MANGLE(glTexGeni)(GLenum coord, GLenum pname, GLint param);
void GL_MANGLE(glTexGeniv)(GLenum coord, GLenum pname, const GLint *params);
void GL_MANGLE(glTexImage1D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
void GL_MANGLE(glTexImage2D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
void GL_MANGLE(glTexParameterf)(GLenum target, GLenum pname, GLfloat param);
void GL_MANGLE(glTexParameterfv)(GLenum target, GLenum pname, const GLfloat *params);
void GL_MANGLE(glTexParameteri)(GLenum target, GLenum pname, GLint param);
void GL_MANGLE(glTexParameteriv)(GLenum target, GLenum pname, const GLint *params);
void GL_MANGLE(glTexSubImage1D)(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels);
void GL_MANGLE(glTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
void GL_MANGLE(glTranslated)(GLdouble x, GLdouble y, GLdouble z);
void GL_MANGLE(glTranslatef)(GLfloat x, GLfloat y, GLfloat z);
void GL_MANGLE(glVertex2d)(GLdouble x, GLdouble y);
void GL_MANGLE(glVertex2dv)(const GLdouble *v);
void GL_MANGLE(glVertex2f)(GLfloat x, GLfloat y);
void GL_MANGLE(glVertex2fv)(const GLfloat *v);
void GL_MANGLE(glVertex2i)(GLint x, GLint y);
void GL_MANGLE(glVertex2iv)(const GLint *v);
void GL_MANGLE(glVertex2s)(GLshort x, GLshort y);
void GL_MANGLE(glVertex2sv)(const GLshort *v);
void GL_MANGLE(glVertex3d)(GLdouble x, GLdouble y, GLdouble z);
void GL_MANGLE(glVertex3dv)(const GLdouble *v);
void GL_MANGLE(glVertex3f)(GLfloat x, GLfloat y, GLfloat z);
void GL_MANGLE(glVertex3fv)(const GLfloat *v);
void GL_MANGLE(glVertex3i)(GLint x, GLint y, GLint z);
void GL_MANGLE(glVertex3iv)(const GLint *v);
void GL_MANGLE(glVertex3s)(GLshort x, GLshort y, GLshort z);
void GL_MANGLE(glVertex3sv)(const GLshort *v);
void GL_MANGLE(glVertex4d)(GLdouble x, GLdouble y, GLdouble z, GLdouble w);
void GL_MANGLE(glVertex4dv)(const GLdouble *v);
void GL_MANGLE(glVertex4f)(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
void GL_MANGLE(glVertex4fv)(const GLfloat *v);
void GL_MANGLE(glVertex4i)(GLint x, GLint y, GLint z, GLint w);
void GL_MANGLE(glVertex4iv)(const GLint *v);
void GL_MANGLE(glVertex4s)(GLshort x, GLshort y, GLshort z, GLshort w);
void GL_MANGLE(glVertex4sv)(const GLshort *v);
void GL_MANGLE(glVertexPointer)(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
void GL_MANGLE(glViewport)(GLint x, GLint y, GLsizei width, GLsizei height);
void GL_MANGLE(glPointParameterfEXT)( GLenum param, GLfloat value );
void GL_MANGLE(glPointParameterfvEXT)( GLenum param, const GLfloat *value );
void GL_MANGLE(glLockArraysEXT)(int , int);
void GL_MANGLE(glUnlockArraysEXT)(void);
void GL_MANGLE(glActiveTextureARB)( GLenum );
void GL_MANGLE(glClientActiveTextureARB)( GLenum );
void GL_MANGLE(glGetCompressedTexImage)( GLenum target, GLint lod, const GLvoid* data );
void GL_MANGLE(glDrawRangeElements)( GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices );
void GL_MANGLE(glDrawRangeElementsEXT)( GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices );
void GL_MANGLE(glMultiTexCoord1f)(GLenum, GLfloat);
void GL_MANGLE(glMultiTexCoord2f)(GLenum, GLfloat, GLfloat);
void GL_MANGLE(glMultiTexCoord3f)(GLenum, GLfloat, GLfloat, GLfloat);
void GL_MANGLE(glMultiTexCoord4f)(GLenum, GLfloat, GLfloat, GLfloat, GLfloat);
void GL_MANGLE(glActiveTexture)(GLenum);
void GL_MANGLE(glClientActiveTexture)(GLenum);
void GL_MANGLE(glCompressedTexImage3DARB)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void *data);
void GL_MANGLE(glCompressedTexImage2DARB)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border,  GLsizei imageSize, const void *data);
void GL_MANGLE(glCompressedTexImage1DARB)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const void *data);
void GL_MANGLE(glCompressedTexSubImage3DARB)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void *data);
void GL_MANGLE(glCompressedTexSubImage2DARB)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data);
void GL_MANGLE(glCompressedTexSubImage1DARB)(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void *data);
void GL_MANGLE(glCompressedTexImage3D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void *data);
void GL_MANGLE(glCompressedTexImage2D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border,  GLsizei imageSize, const void *data);
void GL_MANGLE(glCompressedTexImage1D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const void *data);
void GL_MANGLE(glCompressedTexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void *data);
void GL_MANGLE(glCompressedTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data);
void GL_MANGLE(glCompressedTexSubImage1D)(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void *data);
void GL_MANGLE(glDeleteObjectARB)(GLhandleARB obj);
GLhandleARB GL_MANGLE(glGetHandleARB)(GLenum pname);
void GL_MANGLE(glDetachObjectARB)(GLhandleARB containerObj, GLhandleARB attachedObj);
GLhandleARB GL_MANGLE(glCreateShaderObjectARB)(GLenum shaderType);
void GL_MANGLE(glShaderSourceARB)(GLhandleARB shaderObj, GLsizei count, const GLcharARB **string, const GLint *length);
void GL_MANGLE(glCompileShaderARB)(GLhandleARB shaderObj);
GLhandleARB GL_MANGLE(glCreateProgramObjectARB)(void);
void GL_MANGLE(glAttachObjectARB)(GLhandleARB containerObj, GLhandleARB obj);
void GL_MANGLE(glLinkProgramARB)(GLhandleARB programObj);
void GL_MANGLE(glUseProgramObjectARB)(GLhandleARB programObj);
void GL_MANGLE(glValidateProgramARB)(GLhandleARB programObj);
void GL_MANGLE(glBindProgramARB)(GLenum target, GLuint program);
void GL_MANGLE(glDeleteProgramsARB)(GLsizei n, const GLuint *programs);
void GL_MANGLE(glGenProgramsARB)(GLsizei n, GLuint *programs);
void GL_MANGLE(glProgramStringARB)(GLenum target, GLenum format, GLsizei len, const GLvoid *string);
void GL_MANGLE(glProgramEnvParameter4fARB)(GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
void GL_MANGLE(glProgramLocalParameter4fARB)(GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
void GL_MANGLE(glUniform1fARB)(GLint location, GLfloat v0);
void GL_MANGLE(glUniform2fARB)(GLint location, GLfloat v0, GLfloat v1);
void GL_MANGLE(glUniform3fARB)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
void GL_MANGLE(glUniform4fARB)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
void GL_MANGLE(glUniform1iARB)(GLint location, GLint v0);
void GL_MANGLE(glUniform2iARB)(GLint location, GLint v0, GLint v1);
void GL_MANGLE(glUniform3iARB)(GLint location, GLint v0, GLint v1, GLint v2);
void GL_MANGLE(glUniform4iARB)(GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
void GL_MANGLE(glUniform1fvARB)(GLint location, GLsizei count, const GLfloat *value);
void GL_MANGLE(glUniform2fvARB)(GLint location, GLsizei count, const GLfloat *value);
void GL_MANGLE(glUniform3fvARB)(GLint location, GLsizei count, const GLfloat *value);
void GL_MANGLE(glUniform4fvARB)(GLint location, GLsizei count, const GLfloat *value);
void GL_MANGLE(glUniform1ivARB)(GLint location, GLsizei count, const GLint *value);
void GL_MANGLE(glUniform2ivARB)(GLint location, GLsizei count, const GLint *value);
void GL_MANGLE(glUniform3ivARB)(GLint location, GLsizei count, const GLint *value);
void GL_MANGLE(glUniform4ivARB)(GLint location, GLsizei count, const GLint *value);
void GL_MANGLE(glUniformMatrix2fvARB)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
void GL_MANGLE(glUniformMatrix3fvARB)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
void GL_MANGLE(glUniformMatrix4fvARB)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
void GL_MANGLE(glGetObjectParameterfvARB)(GLhandleARB obj, GLenum pname, GLfloat *params);
void GL_MANGLE(glGetObjectParameterivARB)(GLhandleARB obj, GLenum pname, GLint *params);
void GL_MANGLE(glGetInfoLogARB)(GLhandleARB obj, GLsizei maxLength, GLsizei *length, GLcharARB *infoLog);
void GL_MANGLE(glGetAttachedObjectsARB)(GLhandleARB containerObj, GLsizei maxCount, GLsizei *count, GLhandleARB *obj);
GLint GL_MANGLE(glGetUniformLocationARB)(GLhandleARB programObj, const GLcharARB *name);
void GL_MANGLE(glGetActiveUniformARB)(GLhandleARB programObj, GLuint index, GLsizei maxLength, GLsizei *length, GLint *size, GLenum *type, GLcharARB *name);
void GL_MANGLE(glGetUniformfvARB)(GLhandleARB programObj, GLint location, GLfloat *params);
void GL_MANGLE(glGetUniformivARB)(GLhandleARB programObj, GLint location, GLint *params);
void GL_MANGLE(glGetShaderSourceARB)(GLhandleARB obj, GLsizei maxLength, GLsizei *length, GLcharARB *source);
void GL_MANGLE(glTexImage3D)( GLenum target, GLint level, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels );
void GL_MANGLE(glTexSubImage3D)( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels );
void GL_MANGLE(glCopyTexSubImage3D)( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height );
void GL_MANGLE(glBlendEquationEXT)(GLenum);
void GL_MANGLE(glStencilOpSeparate)(GLenum, GLenum, GLenum, GLenum);
void GL_MANGLE(glStencilFuncSeparate)(GLenum, GLenum, GLint, GLuint);
void GL_MANGLE(glActiveStencilFaceEXT)(GLenum);
void GL_MANGLE(glVertexAttribPointerARB)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer);
void GL_MANGLE(glEnableVertexAttribArrayARB)(GLuint index);
void GL_MANGLE(glDisableVertexAttribArrayARB)(GLuint index);
void GL_MANGLE(glBindAttribLocationARB)(GLhandleARB programObj, GLuint index, const GLcharARB *name);
void GL_MANGLE(glGetActiveAttribARB)(GLhandleARB programObj, GLuint index, GLsizei maxLength, GLsizei *length, GLint *size, GLenum *type, GLcharARB *name);
GLint GL_MANGLE(glGetAttribLocationARB)(GLhandleARB programObj, const GLcharARB *name);
void GL_MANGLE(glBindBufferARB)(GLenum target, GLuint buffer);
void GL_MANGLE(glDeleteBuffersARB)(GLsizei n, const GLuint *buffers);
void GL_MANGLE(glGenBuffersARB)(GLsizei n, GLuint *buffers);
GLboolean GL_MANGLE(glIsBufferARB)(GLuint buffer);
GLvoid* GL_MANGLE(glMapBufferARB)(GLenum target, GLenum access);
GLboolean GL_MANGLE(glUnmapBufferARB)(GLenum target);
void GL_MANGLE(glBufferDataARB)(GLenum target, GLsizeiptrARB size, const GLvoid *data, GLenum usage);
void GL_MANGLE(glBufferSubDataARB)(GLenum target, GLintptrARB offset, GLsizeiptrARB size, const GLvoid *data);
void GL_MANGLE(glGenQueriesARB)(GLsizei n, GLuint *ids);
void GL_MANGLE(glDeleteQueriesARB)(GLsizei n, const GLuint *ids);
GLboolean GL_MANGLE(glIsQueryARB)(GLuint id);
void GL_MANGLE(glBeginQueryARB)(GLenum target, GLuint id);
void GL_MANGLE(glEndQueryARB)(GLenum target);
void GL_MANGLE(glGetQueryivARB)(GLenum target, GLenum pname, GLint *params);
void GL_MANGLE(glGetQueryObjectivARB)(GLuint id, GLenum pname, GLint *params);
void GL_MANGLE(glGetQueryObjectuivARB)(GLuint id, GLenum pname, GLuint *params);
void GL_MANGLE(glSelectTextureSGIS)( GLenum );
void GL_MANGLE(glMTexCoord2fSGIS)( GLenum, GLfloat, GLfloat );
void GL_MANGLE(glSwapInterval)( int interval );
};
// mangle to xash3d style
#ifndef WES_MANGLE_PREPEND
#define pglGetError GL_MANGLE(glGetError)
#define pglGetString GL_MANGLE(glGetString)
#define pglAccum GL_MANGLE(glAccum)
#define pglAlphaFunc GL_MANGLE(glAlphaFunc)
#define pglArrayElement GL_MANGLE(glArrayElement)
#define pglBegin GL_MANGLE(glBegin)
#define pglBindTexture GL_MANGLE(glBindTexture)
#define pglBitmap GL_MANGLE(glBitmap)
#define pglBlendFunc GL_MANGLE(glBlendFunc)
#define pglCallList GL_MANGLE(glCallList)
#define pglCallLists GL_MANGLE(glCallLists)
#define pglClear GL_MANGLE(glClear)
#define pglClearAccum GL_MANGLE(glClearAccum)
#define pglClearColor GL_MANGLE(glClearColor)
#define pglClearDepth GL_MANGLE(glClearDepth)
#define pglClearIndex GL_MANGLE(glClearIndex)
#define pglClearStencil GL_MANGLE(glClearStencil)
#define pglIsEnabled GL_MANGLE(glIsEnabled)
#define pglIsList GL_MANGLE(glIsList)
#define pglIsTexture GL_MANGLE(glIsTexture)
#define pglClipPlane GL_MANGLE(glClipPlane)
#define pglColor3b GL_MANGLE(glColor3b)
#define pglColor3bv GL_MANGLE(glColor3bv)
#define pglColor3d GL_MANGLE(glColor3d)
#define pglColor3dv GL_MANGLE(glColor3dv)
#define pglColor3f GL_MANGLE(glColor3f)
#define pglColor3fv GL_MANGLE(glColor3fv)
#define pglColor3i GL_MANGLE(glColor3i)
#define pglColor3iv GL_MANGLE(glColor3iv)
#define pglColor3s GL_MANGLE(glColor3s)
#define pglColor3sv GL_MANGLE(glColor3sv)
#define pglColor3ub GL_MANGLE(glColor3ub)
#define pglColor3ubv GL_MANGLE(glColor3ubv)
#define pglColor3ui GL_MANGLE(glColor3ui)
#define pglColor3uiv GL_MANGLE(glColor3uiv)
#define pglColor3us GL_MANGLE(glColor3us)
#define pglColor3usv GL_MANGLE(glColor3usv)
#define pglColor4b GL_MANGLE(glColor4b)
#define pglColor4bv GL_MANGLE(glColor4bv)
#define pglColor4d GL_MANGLE(glColor4d)
#define pglColor4dv GL_MANGLE(glColor4dv)
#define pglColor4f GL_MANGLE(glColor4f)
#define pglColor4fv GL_MANGLE(glColor4fv)
#define pglColor4i GL_MANGLE(glColor4i)
#define pglColor4iv GL_MANGLE(glColor4iv)
#define pglColor4s GL_MANGLE(glColor4s)
#define pglColor4sv GL_MANGLE(glColor4sv)
#define pglColor4ub GL_MANGLE(glColor4ub)
#define pglColor4ubv GL_MANGLE(glColor4ubv)
#define pglColor4ui GL_MANGLE(glColor4ui)
#define pglColor4uiv GL_MANGLE(glColor4uiv)
#define pglColor4us GL_MANGLE(glColor4us)
#define pglColor4usv GL_MANGLE(glColor4usv)
#define pglColorMask GL_MANGLE(glColorMask)
#define pglColorMaterial GL_MANGLE(glColorMaterial)
#define pglColorPointer GL_MANGLE(glColorPointer)
#define pglCopyPixels GL_MANGLE(glCopyPixels)
#define pglCopyTexImage1D GL_MANGLE(glCopyTexImage1D)
#define pglCopyTexImage2D GL_MANGLE(glCopyTexImage2D)
#define pglCopyTexSubImage1D GL_MANGLE(glCopyTexSubImage1D)
#define pglCopyTexSubImage2D GL_MANGLE(glCopyTexSubImage2D)
#define pglCullFace GL_MANGLE(glCullFace)
#define pglDeleteLists GL_MANGLE(glDeleteLists)
#define pglDeleteTextures GL_MANGLE(glDeleteTextures)
#define pglDepthFunc GL_MANGLE(glDepthFunc)
#define pglDepthMask GL_MANGLE(glDepthMask)
#define pglDepthRange GL_MANGLE(glDepthRange)
#define pglDisable GL_MANGLE(glDisable)
#define pglDisableClientState GL_MANGLE(glDisableClientState)
#define pglDrawArrays GL_MANGLE(glDrawArrays)
#define pglDrawBuffer GL_MANGLE(glDrawBuffer)
#define pglDrawElements GL_MANGLE(glDrawElements)
#define pglDrawPixels GL_MANGLE(glDrawPixels)
#define pglEdgeFlag GL_MANGLE(glEdgeFlag)
#define pglEdgeFlagPointer GL_MANGLE(glEdgeFlagPointer)
#define pglEdgeFlagv GL_MANGLE(glEdgeFlagv)
#define pglEnable GL_MANGLE(glEnable)
#define pglEnableClientState GL_MANGLE(glEnableClientState)
#define pglEnd GL_MANGLE(glEnd)
#define pglEndList GL_MANGLE(glEndList)
#define pglEvalCoord1d GL_MANGLE(glEvalCoord1d)
#define pglEvalCoord1dv GL_MANGLE(glEvalCoord1dv)
#define pglEvalCoord1f GL_MANGLE(glEvalCoord1f)
#define pglEvalCoord1fv GL_MANGLE(glEvalCoord1fv)
#define pglEvalCoord2d GL_MANGLE(glEvalCoord2d)
#define pglEvalCoord2dv GL_MANGLE(glEvalCoord2dv)
#define pglEvalCoord2f GL_MANGLE(glEvalCoord2f)
#define pglEvalCoord2fv GL_MANGLE(glEvalCoord2fv)
#define pglEvalMesh1 GL_MANGLE(glEvalMesh1)
#define pglEvalMesh2 GL_MANGLE(glEvalMesh2)
#define pglEvalPoint1 GL_MANGLE(glEvalPoint1)
#define pglEvalPoint2 GL_MANGLE(glEvalPoint2)
#define pglFeedbackBuffer GL_MANGLE(glFeedbackBuffer)
#define pglFinish GL_MANGLE(glFinish)
#define pglFlush GL_MANGLE(glFlush)
#define pglFogf GL_MANGLE(glFogf)
#define pglFogfv GL_MANGLE(glFogfv)
#define pglFogi GL_MANGLE(glFogi)
#define pglFogiv GL_MANGLE(glFogiv)
#define pglFrontFace GL_MANGLE(glFrontFace)
#define pglFrustum GL_MANGLE(glFrustum)
#define pglGenTextures GL_MANGLE(glGenTextures)
#define pglGetBooleanv GL_MANGLE(glGetBooleanv)
#define pglGetClipPlane GL_MANGLE(glGetClipPlane)
#define pglGetDoublev GL_MANGLE(glGetDoublev)
#define pglGetFloatv GL_MANGLE(glGetFloatv)
#define pglGetIntegerv GL_MANGLE(glGetIntegerv)
#define pglGetLightfv GL_MANGLE(glGetLightfv)
#define pglGetLightiv GL_MANGLE(glGetLightiv)
#define pglGetMapdv GL_MANGLE(glGetMapdv)
#define pglGetMapfv GL_MANGLE(glGetMapfv)
#define pglGetMapiv GL_MANGLE(glGetMapiv)
#define pglGetMaterialfv GL_MANGLE(glGetMaterialfv)
#define pglGetMaterialiv GL_MANGLE(glGetMaterialiv)
#define pglGetPixelMapfv GL_MANGLE(glGetPixelMapfv)
#define pglGetPixelMapuiv GL_MANGLE(glGetPixelMapuiv)
#define pglGetPixelMapusv GL_MANGLE(glGetPixelMapusv)
#define pglGetPointerv GL_MANGLE(glGetPointerv)
#define pglGetPolygonStipple GL_MANGLE(glGetPolygonStipple)
#define pglGetTexEnvfv GL_MANGLE(glGetTexEnvfv)
#define pglGetTexEnviv GL_MANGLE(glGetTexEnviv)
#define pglGetTexGendv GL_MANGLE(glGetTexGendv)
#define pglGetTexGenfv GL_MANGLE(glGetTexGenfv)
#define pglGetTexGeniv GL_MANGLE(glGetTexGeniv)
#define pglGetTexImage GL_MANGLE(glGetTexImage)
#define pglGetTexLevelParameterfv GL_MANGLE(glGetTexLevelParameterfv)
#define pglGetTexLevelParameteriv GL_MANGLE(glGetTexLevelParameteriv)
#define pglGetTexParameterfv GL_MANGLE(glGetTexParameterfv)
#define pglGetTexParameteriv GL_MANGLE(glGetTexParameteriv)
#define pglHint GL_MANGLE(glHint)
#define pglIndexMask GL_MANGLE(glIndexMask)
#define pglIndexPointer GL_MANGLE(glIndexPointer)
#define pglIndexd GL_MANGLE(glIndexd)
#define pglIndexdv GL_MANGLE(glIndexdv)
#define pglIndexf GL_MANGLE(glIndexf)
#define pglIndexfv GL_MANGLE(glIndexfv)
#define pglIndexi GL_MANGLE(glIndexi)
#define pglIndexiv GL_MANGLE(glIndexiv)
#define pglIndexs GL_MANGLE(glIndexs)
#define pglIndexsv GL_MANGLE(glIndexsv)
#define pglIndexub GL_MANGLE(glIndexub)
#define pglIndexubv GL_MANGLE(glIndexubv)
#define pglInitNames GL_MANGLE(glInitNames)
#define pglInterleavedArrays GL_MANGLE(glInterleavedArrays)
#define pglLightModelf GL_MANGLE(glLightModelf)
#define pglLightModelfv GL_MANGLE(glLightModelfv)
#define pglLightModeli GL_MANGLE(glLightModeli)
#define pglLightModeliv GL_MANGLE(glLightModeliv)
#define pglLightf GL_MANGLE(glLightf)
#define pglLightfv GL_MANGLE(glLightfv)
#define pglLighti GL_MANGLE(glLighti)
#define pglLightiv GL_MANGLE(glLightiv)
#define pglLineStipple GL_MANGLE(glLineStipple)
#define pglLineWidth GL_MANGLE(glLineWidth)
#define pglListBase GL_MANGLE(glListBase)
#define pglLoadIdentity GL_MANGLE(glLoadIdentity)
#define pglLoadMatrixd GL_MANGLE(glLoadMatrixd)
#define pglLoadMatrixf GL_MANGLE(glLoadMatrixf)
#define pglLoadName GL_MANGLE(glLoadName)
#define pglLogicOp GL_MANGLE(glLogicOp)
#define pglMap1d GL_MANGLE(glMap1d)
#define pglMap1f GL_MANGLE(glMap1f)
#define pglMap2d GL_MANGLE(glMap2d)
#define pglMap2f GL_MANGLE(glMap2f)
#define pglMapGrid1d GL_MANGLE(glMapGrid1d)
#define pglMapGrid1f GL_MANGLE(glMapGrid1f)
#define pglMapGrid2d GL_MANGLE(glMapGrid2d)
#define pglMapGrid2f GL_MANGLE(glMapGrid2f)
#define pglMaterialf GL_MANGLE(glMaterialf)
#define pglMaterialfv GL_MANGLE(glMaterialfv)
#define pglMateriali GL_MANGLE(glMateriali)
#define pglMaterialiv GL_MANGLE(glMaterialiv)
#define pglMatrixMode GL_MANGLE(glMatrixMode)
#define pglMultMatrixd GL_MANGLE(glMultMatrixd)
#define pglMultMatrixf GL_MANGLE(glMultMatrixf)
#define pglNewList GL_MANGLE(glNewList)
#define pglNormal3b GL_MANGLE(glNormal3b)
#define pglNormal3bv GL_MANGLE(glNormal3bv)
#define pglNormal3d GL_MANGLE(glNormal3d)
#define pglNormal3dv GL_MANGLE(glNormal3dv)
#define pglNormal3f GL_MANGLE(glNormal3f)
#define pglNormal3fv GL_MANGLE(glNormal3fv)
#define pglNormal3i GL_MANGLE(glNormal3i)
#define pglNormal3iv GL_MANGLE(glNormal3iv)
#define pglNormal3s GL_MANGLE(glNormal3s)
#define pglNormal3sv GL_MANGLE(glNormal3sv)
#define pglNormalPointer GL_MANGLE(glNormalPointer)
#define pglOrtho GL_MANGLE(glOrtho)
#define pglPassThrough GL_MANGLE(glPassThrough)
#define pglPixelMapfv GL_MANGLE(glPixelMapfv)
#define pglPixelMapuiv GL_MANGLE(glPixelMapuiv)
#define pglPixelMapusv GL_MANGLE(glPixelMapusv)
#define pglPixelStoref GL_MANGLE(glPixelStoref)
#define pglPixelStorei GL_MANGLE(glPixelStorei)
#define pglPixelTransferf GL_MANGLE(glPixelTransferf)
#define pglPixelTransferi GL_MANGLE(glPixelTransferi)
#define pglPixelZoom GL_MANGLE(glPixelZoom)
#define pglPointSize GL_MANGLE(glPointSize)
#define pglPolygonMode GL_MANGLE(glPolygonMode)
#define pglPolygonOffset GL_MANGLE(glPolygonOffset)
#define pglPolygonStipple GL_MANGLE(glPolygonStipple)
#define pglPopAttrib GL_MANGLE(glPopAttrib)
#define pglPopClientAttrib GL_MANGLE(glPopClientAttrib)
#define pglPopMatrix GL_MANGLE(glPopMatrix)
#define pglPopName GL_MANGLE(glPopName)
#define pglPushAttrib GL_MANGLE(glPushAttrib)
#define pglPushClientAttrib GL_MANGLE(glPushClientAttrib)
#define pglPushMatrix GL_MANGLE(glPushMatrix)
#define pglPushName GL_MANGLE(glPushName)
#define pglRasterPos2d GL_MANGLE(glRasterPos2d)
#define pglRasterPos2dv GL_MANGLE(glRasterPos2dv)
#define pglRasterPos2f GL_MANGLE(glRasterPos2f)
#define pglRasterPos2fv GL_MANGLE(glRasterPos2fv)
#define pglRasterPos2i GL_MANGLE(glRasterPos2i)
#define pglRasterPos2iv GL_MANGLE(glRasterPos2iv)
#define pglRasterPos2s GL_MANGLE(glRasterPos2s)
#define pglRasterPos2sv GL_MANGLE(glRasterPos2sv)
#define pglRasterPos3d GL_MANGLE(glRasterPos3d)
#define pglRasterPos3dv GL_MANGLE(glRasterPos3dv)
#define pglRasterPos3f GL_MANGLE(glRasterPos3f)
#define pglRasterPos3fv GL_MANGLE(glRasterPos3fv)
#define pglRasterPos3i GL_MANGLE(glRasterPos3i)
#define pglRasterPos3iv GL_MANGLE(glRasterPos3iv)
#define pglRasterPos3s GL_MANGLE(glRasterPos3s)
#define pglRasterPos3sv GL_MANGLE(glRasterPos3sv)
#define pglRasterPos4d GL_MANGLE(glRasterPos4d)
#define pglRasterPos4dv GL_MANGLE(glRasterPos4dv)
#define pglRasterPos4f GL_MANGLE(glRasterPos4f)
#define pglRasterPos4fv GL_MANGLE(glRasterPos4fv)
#define pglRasterPos4i GL_MANGLE(glRasterPos4i)
#define pglRasterPos4iv GL_MANGLE(glRasterPos4iv)
#define pglRasterPos4s GL_MANGLE(glRasterPos4s)
#define pglRasterPos4sv GL_MANGLE(glRasterPos4sv)
#define pglReadBuffer GL_MANGLE(glReadBuffer)
#define pglReadPixels GL_MANGLE(glReadPixels)
#define pglRectd GL_MANGLE(glRectd)
#define pglRectdv GL_MANGLE(glRectdv)
#define pglRectf GL_MANGLE(glRectf)
#define pglRectfv GL_MANGLE(glRectfv)
#define pglRecti GL_MANGLE(glRecti)
#define pglRectiv GL_MANGLE(glRectiv)
#define pglRects GL_MANGLE(glRects)
#define pglRectsv GL_MANGLE(glRectsv)
#define pglRotated GL_MANGLE(glRotated)
#define pglRotatef GL_MANGLE(glRotatef)
#define pglScaled GL_MANGLE(glScaled)
#define pglScalef GL_MANGLE(glScalef)
#define pglScissor GL_MANGLE(glScissor)
#define pglSelectBuffer GL_MANGLE(glSelectBuffer)
#define pglShadeModel GL_MANGLE(glShadeModel)
#define pglStencilFunc GL_MANGLE(glStencilFunc)
#define pglStencilMask GL_MANGLE(glStencilMask)
#define pglStencilOp GL_MANGLE(glStencilOp)
#define pglTexCoord1d GL_MANGLE(glTexCoord1d)
#define pglTexCoord1dv GL_MANGLE(glTexCoord1dv)
#define pglTexCoord1f GL_MANGLE(glTexCoord1f)
#define pglTexCoord1fv GL_MANGLE(glTexCoord1fv)
#define pglTexCoord1i GL_MANGLE(glTexCoord1i)
#define pglTexCoord1iv GL_MANGLE(glTexCoord1iv)
#define pglTexCoord1s GL_MANGLE(glTexCoord1s)
#define pglTexCoord1sv GL_MANGLE(glTexCoord1sv)
#define pglTexCoord2d GL_MANGLE(glTexCoord2d)
#define pglTexCoord2dv GL_MANGLE(glTexCoord2dv)
#define pglTexCoord2f GL_MANGLE(glTexCoord2f)
#define pglTexCoord2fv GL_MANGLE(glTexCoord2fv)
#define pglTexCoord2i GL_MANGLE(glTexCoord2i)
#define pglTexCoord2iv GL_MANGLE(glTexCoord2iv)
#define pglTexCoord2s GL_MANGLE(glTexCoord2s)
#define pglTexCoord2sv GL_MANGLE(glTexCoord2sv)
#define pglTexCoord3d GL_MANGLE(glTexCoord3d)
#define pglTexCoord3dv GL_MANGLE(glTexCoord3dv)
#define pglTexCoord3f GL_MANGLE(glTexCoord3f)
#define pglTexCoord3fv GL_MANGLE(glTexCoord3fv)
#define pglTexCoord3i GL_MANGLE(glTexCoord3i)
#define pglTexCoord3iv GL_MANGLE(glTexCoord3iv)
#define pglTexCoord3s GL_MANGLE(glTexCoord3s)
#define pglTexCoord3sv GL_MANGLE(glTexCoord3sv)
#define pglTexCoord4d GL_MANGLE(glTexCoord4d)
#define pglTexCoord4dv GL_MANGLE(glTexCoord4dv)
#define pglTexCoord4f GL_MANGLE(glTexCoord4f)
#define pglTexCoord4fv GL_MANGLE(glTexCoord4fv)
#define pglTexCoord4i GL_MANGLE(glTexCoord4i)
#define pglTexCoord4iv GL_MANGLE(glTexCoord4iv)
#define pglTexCoord4s GL_MANGLE(glTexCoord4s)
#define pglTexCoord4sv GL_MANGLE(glTexCoord4sv)
#define pglTexCoordPointer GL_MANGLE(glTexCoordPointer)
#define pglTexEnvf GL_MANGLE(glTexEnvf)
#define pglTexEnvfv GL_MANGLE(glTexEnvfv)
#define pglTexEnvi GL_MANGLE(glTexEnvi)
#define pglTexEnviv GL_MANGLE(glTexEnviv)
#define pglTexGend GL_MANGLE(glTexGend)
#define pglTexGendv GL_MANGLE(glTexGendv)
#define pglTexGenf GL_MANGLE(glTexGenf)
#define pglTexGenfv GL_MANGLE(glTexGenfv)
#define pglTexGeni GL_MANGLE(glTexGeni)
#define pglTexGeniv GL_MANGLE(glTexGeniv)
#define pglTexImage1D GL_MANGLE(glTexImage1D)
#define pglTexImage2D GL_MANGLE(glTexImage2D)
#define pglTexParameterf GL_MANGLE(glTexParameterf)
#define pglTexParameterfv GL_MANGLE(glTexParameterfv)
#define pglTexParameteri GL_MANGLE(glTexParameteri)
#define pglTexParameteriv GL_MANGLE(glTexParameteriv)
#define pglTexSubImage1D GL_MANGLE(glTexSubImage1D)
#define pglTexSubImage2D GL_MANGLE(glTexSubImage2D)
#define pglTranslated GL_MANGLE(glTranslated)
#define pglTranslatef GL_MANGLE(glTranslatef)
#define pglVertex2d GL_MANGLE(glVertex2d)
#define pglVertex2dv GL_MANGLE(glVertex2dv)
#define pglVertex2f GL_MANGLE(glVertex2f)
#define pglVertex2fv GL_MANGLE(glVertex2fv)
#define pglVertex2i GL_MANGLE(glVertex2i)
#define pglVertex2iv GL_MANGLE(glVertex2iv)
#define pglVertex2s GL_MANGLE(glVertex2s)
#define pglVertex2sv GL_MANGLE(glVertex2sv)
#define pglVertex3d GL_MANGLE(glVertex3d)
#define pglVertex3dv GL_MANGLE(glVertex3dv)
#define pglVertex3f GL_MANGLE(glVertex3f)
#define pglVertex3fv GL_MANGLE(glVertex3fv)
#define pglVertex3i GL_MANGLE(glVertex3i)
#define pglVertex3iv GL_MANGLE(glVertex3iv)
#define pglVertex3s GL_MANGLE(glVertex3s)
#define pglVertex3sv GL_MANGLE(glVertex3sv)
#define pglVertex4d GL_MANGLE(glVertex4d)
#define pglVertex4dv GL_MANGLE(glVertex4dv)
#define pglVertex4f GL_MANGLE(glVertex4f)
#define pglVertex4fv GL_MANGLE(glVertex4fv)
#define pglVertex4i GL_MANGLE(glVertex4i)
#define pglVertex4iv GL_MANGLE(glVertex4iv)
#define pglVertex4s GL_MANGLE(glVertex4s)
#define pglVertex4sv GL_MANGLE(glVertex4sv)
#define pglVertexPointer GL_MANGLE(glVertexPointer)
#define pglViewport GL_MANGLE(glViewport)
#define pglPointParameterfEXT GL_MANGLE(glPointParameterfEXT)
#define pglPointParameterfvEXT GL_MANGLE(glPointParameterfvEXT)
#define pglLockArraysEXT GL_MANGLE(glLockArraysEXT)
#define pglUnlockArraysEXT GL_MANGLE(glUnlockArraysEXT)
#define pglActiveTextureARB GL_MANGLE(glActiveTextureARB)
#define pglClientActiveTextureARB GL_MANGLE(glClientActiveTextureARB)
#define pglGetCompressedTexImage GL_MANGLE(glGetCompressedTexImage)
#define pglDrawRangeElements GL_MANGLE(glDrawRangeElements)
#define pglDrawRangeElementsEXT GL_MANGLE(glDrawRangeElementsEXT)
#define pglMultiTexCoord1f GL_MANGLE(glMultiTexCoord1f)
#define pglMultiTexCoord2f GL_MANGLE(glMultiTexCoord2f)
#define pglMultiTexCoord3f GL_MANGLE(glMultiTexCoord3f)
#define pglMultiTexCoord4f GL_MANGLE(glMultiTexCoord4f)
#define pglActiveTexture GL_MANGLE(glActiveTexture)
#define pglClientActiveTexture GL_MANGLE(glClientActiveTexture)
#define pglCompressedTexImage3DARB GL_MANGLE(glCompressedTexImage3DARB)
#define pglCompressedTexImage2DARB GL_MANGLE(glCompressedTexImage2DARB)
#define pglCompressedTexImage1DARB GL_MANGLE(glCompressedTexImage1DARB)
#define pglCompressedTexSubImage3DARB GL_MANGLE(glCompressedTexSubImage3DARB)
#define pglCompressedTexSubImage2DARB GL_MANGLE(glCompressedTexSubImage2DARB)
#define pglCompressedTexSubImage1DARB GL_MANGLE(glCompressedTexSubImage1DARB)
#define pglCompressedTexImage3D GL_MANGLE(glCompressedTexImage3D)
#define pglCompressedTexImage2D GL_MANGLE(glCompressedTexImage2D)
#define pglCompressedTexImage1D GL_MANGLE(glCompressedTexImage1D)
#define pglCompressedTexSubImage3D GL_MANGLE(glCompressedTexSubImage3D)
#define pglCompressedTexSubImage2D GL_MANGLE(glCompressedTexSubImage2D)
#define pglCompressedTexSubImage1D GL_MANGLE(glCompressedTexSubImage1D)
#define pglDeleteObjectARB GL_MANGLE(glDeleteObjectARB)
#define pglGetHandleARB GL_MANGLE(glGetHandleARB)
#define pglDetachObjectARB GL_MANGLE(glDetachObjectARB)
#define pglCreateShaderObjectARB GL_MANGLE(glCreateShaderObjectARB)
#define pglShaderSourceARB GL_MANGLE(glShaderSourceARB)
#define pglCompileShaderARB GL_MANGLE(glCompileShaderARB)
#define pglCreateProgramObjectARB GL_MANGLE(glCreateProgramObjectARB)
#define pglAttachObjectARB GL_MANGLE(glAttachObjectARB)
#define pglLinkProgramARB GL_MANGLE(glLinkProgramARB)
#define pglUseProgramObjectARB GL_MANGLE(glUseProgramObjectARB)
#define pglValidateProgramARB GL_MANGLE(glValidateProgramARB)
#define pglBindProgramARB GL_MANGLE(glBindProgramARB)
#define pglDeleteProgramsARB GL_MANGLE(glDeleteProgramsARB)
#define pglGenProgramsARB GL_MANGLE(glGenProgramsARB)
#define pglProgramStringARB GL_MANGLE(glProgramStringARB)
#define pglProgramEnvParameter4fARB GL_MANGLE(glProgramEnvParameter4fARB)
#define pglProgramLocalParameter4fARB GL_MANGLE(glProgramLocalParameter4fARB)
#define pglUniform1fARB GL_MANGLE(glUniform1fARB)
#define pglUniform2fARB GL_MANGLE(glUniform2fARB)
#define pglUniform3fARB GL_MANGLE(glUniform3fARB)
#define pglUniform4fARB GL_MANGLE(glUniform4fARB)
#define pglUniform1iARB GL_MANGLE(glUniform1iARB)
#define pglUniform2iARB GL_MANGLE(glUniform2iARB)
#define pglUniform3iARB GL_MANGLE(glUniform3iARB)
#define pglUniform4iARB GL_MANGLE(glUniform4iARB)
#define pglUniform1fvARB GL_MANGLE(glUniform1fvARB)
#define pglUniform2fvARB GL_MANGLE(glUniform2fvARB)
#define pglUniform3fvARB GL_MANGLE(glUniform3fvARB)
#define pglUniform4fvARB GL_MANGLE(glUniform4fvARB)
#define pglUniform1ivARB GL_MANGLE(glUniform1ivARB)
#define pglUniform2ivARB GL_MANGLE(glUniform2ivARB)
#define pglUniform3ivARB GL_MANGLE(glUniform3ivARB)
#define pglUniform4ivARB GL_MANGLE(glUniform4ivARB)
#define pglUniformMatrix2fvARB GL_MANGLE(glUniformMatrix2fvARB)
#define pglUniformMatrix3fvARB GL_MANGLE(glUniformMatrix3fvARB)
#define pglUniformMatrix4fvARB GL_MANGLE(glUniformMatrix4fvARB)
#define pglGetObjectParameterfvARB GL_MANGLE(glGetObjectParameterfvARB)
#define pglGetObjectParameterivARB GL_MANGLE(glGetObjectParameterivARB)
#define pglGetInfoLogARB GL_MANGLE(glGetInfoLogARB)
#define pglGetAttachedObjectsARB GL_MANGLE(glGetAttachedObjectsARB)
#define pglGetUniformLocationARB GL_MANGLE(glGetUniformLocationARB)
#define pglGetActiveUniformARB GL_MANGLE(glGetActiveUniformARB)
#define pglGetUniformfvARB GL_MANGLE(glGetUniformfvARB)
#define pglGetUniformivARB GL_MANGLE(glGetUniformivARB)
#define pglGetShaderSourceARB GL_MANGLE(glGetShaderSourceARB)
#define pglTexImage3D GL_MANGLE(glTexImage3D)
#define pglTexSubImage3D GL_MANGLE(glTexSubImage3D)
#define pglCopyTexSubImage3D GL_MANGLE(glCopyTexSubImage3D)
#define pglBlendEquationEXT GL_MANGLE(glBlendEquationEXT)
#define pglStencilOpSeparate GL_MANGLE(glStencilOpSeparate)
#define pglStencilFuncSeparate GL_MANGLE(glStencilFuncSeparate)
#define pglActiveStencilFaceEXT GL_MANGLE(glActiveStencilFaceEXT)
#define pglVertexAttribPointerARB GL_MANGLE(glVertexAttribPointerARB)
#define pglEnableVertexAttribArrayARB GL_MANGLE(glEnableVertexAttribArrayARB)
#define pglDisableVertexAttribArrayARB GL_MANGLE(glDisableVertexAttribArrayARB)
#define pglBindAttribLocationARB GL_MANGLE(glBindAttribLocationARB)
#define pglGetActiveAttribARB GL_MANGLE(glGetActiveAttribARB)
#define pglGetAttribLocationARB GL_MANGLE(glGetAttribLocationARB)
#define pglBindBufferARB GL_MANGLE(glBindBufferARB)
#define pglDeleteBuffersARB GL_MANGLE(glDeleteBuffersARB)
#define pglGenBuffersARB GL_MANGLE(glGenBuffersARB)
#define pglIsBufferARB GL_MANGLE(glIsBufferARB)
#define pglMapBufferARB GL_MANGLE(glMapBufferARB)
#define pglUnmapBufferARB GL_MANGLE(glUnmapBufferARB)
#define pglBufferDataARB GL_MANGLE(glBufferDataARB)
#define pglBufferSubDataARB GL_MANGLE(glBufferSubDataARB)
#define pglGenQueriesARB GL_MANGLE(glGenQueriesARB)
#define pglDeleteQueriesARB GL_MANGLE(glDeleteQueriesARB)
#define pglIsQueryARB GL_MANGLE(glIsQueryARB)
#define pglBeginQueryARB GL_MANGLE(glBeginQueryARB)
#define pglEndQueryARB GL_MANGLE(glEndQueryARB)
#define pglGetQueryivARB GL_MANGLE(glGetQueryivARB)
#define pglGetQueryObjectivARB GL_MANGLE(glGetQueryObjectivARB)
#define pglGetQueryObjectuivARB GL_MANGLE(glGetQueryObjectuivARB)
#define pglSelectTextureSGIS GL_MANGLE(glSelectTextureSGIS)
#define pglMTexCoord2fSGIS GL_MANGLE(glMTexCoord2fSGIS)
#define pglSwapInterval GL_MANGLE(glSwapInterval)
#endif
#endif // GL_STATIC_H

