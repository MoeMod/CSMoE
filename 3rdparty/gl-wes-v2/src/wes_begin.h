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


#include "wes_state.h"

#ifndef __WES_BEGIN_H__
#define __WES_BEGIN_H__

/* Buffer sizes*/
#define WES_BUFFER_COUNT        (65535) //16000
#define WES_INDEX_COUNT         (100000) //16000

//offsets within Vertex struct
#define WES_OFFSET_VERT        0
#define WES_OFFSET_NORMAL      4
#define WES_OFFSET_FOGCOORD    7
#define WES_OFFSET_COLOR0      8
#define WES_OFFSET_COLOR1      12
#define WES_OFFSET_TEXCOORD0   15
#define WES_OFFSET_TEXCOORD1   19
#define WES_OFFSET_TEXCOORD2   23
#define WES_OFFSET_TEXCOORD3   27

typedef struct vertex_s vertex_t;
typedef struct attrib_ptr_s attrib_ptr_t;

struct vertex_s {
    GLfloat x, y, z, w;             //Position , 2-4 elements
    GLfloat nx, ny, nz;             //Normal, 3 elements
    GLfloat fog;                    //Fog Coordinate, 1 element.
    GLfloat cr0, cg0, cb0, ca0;     //Color, 3-4 elements
    GLfloat cr1, cg1, cb1;          //Secondary Color, 3 elements
    struct {
        GLfloat s, t, r, q;
    } coord[WES_MULTITEX_NUM];
};
struct attrib_ptr_s {
    GLboolean   isenabled;
    GLint       size;
    GLenum      type;
    GLsizei     stride;
    const GLvoid *ptr;
	GLuint vbo_id;
#ifdef WES_WEBGL
	GLuint webgl_vbo_id;
#endif
};

typedef struct
{
	GLboolean depthmask;
	GLclampf depth_range_near;
	GLclampf depth_range_far;
	GLenum depth_func;
	GLuint boundtexture;
	GLenum cullface;
	GLenum sfactor;
	GLenum dfactor;
} wrapState2;

extern wrapState2 wrapglState2;

//function declarations:
extern GLvoid wes_begin_init( void );
extern GLvoid wes_begin_destroy( void );
extern GLvoid wes_vertbuffer_flush( void );


#endif
