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

#ifndef __WES_MATRIX_H__
#define __WES_MATRIX_H__

/* number of matrices for push/pop      */
#define WES_MODELVIEW_NUM           32
#define WES_PROJECTION_NUM          2
#define WES_TEXTURE_NUM             2
#define WES_TEXTURE_UNITS           WES_MULTITEX_NUM
#define WES_COLOR_NUM               2

/* flags for matrix operations          */
#define WES_M_IDENTITY              0x01
#define WES_M_SCALED                0x02
#define WES_M_ROTATED               0x04
#define WES_M_TRANSLATED            0x08
#define WES_M_PERSPECTIVE           0x10
#define WES_M_DIRTY                 0x20

/* matrix structure                     */
typedef struct matrix4_s matrix4_t;
typedef struct matrix3_s matrix3_t;

struct matrix4_s {
    GLuint  flags;
    GLfloat data[16];
};

struct matrix3_s {
    GLfloat data[9];
};

/* global variables                     */
extern GLenum       m_mode;

extern matrix4_t    *m_current;
extern matrix4_t    m_modelview[WES_MODELVIEW_NUM];
extern matrix4_t    m_projection[WES_PROJECTION_NUM];
extern matrix4_t    m_texture[WES_TEXTURE_NUM * WES_TEXTURE_UNITS];
extern matrix4_t    m_color[WES_COLOR_NUM];
extern matrix4_t    m_modelview_proj[1];
extern matrix3_t    m_modelview_normal[1];

extern GLuint       m_modelview_num, m_projection_num, m_texture_num, m_color_num;
extern GLboolean    m_modelview_mod;
extern GLboolean    m_projection_mod;


extern GLvoid       wes_matrix_init( void );
extern GLvoid       wes_matrix_update( void );
extern GLvoid       wes_matrix_destroy( void );
extern GLboolean    wes_matrix_mvp( void );
extern GLboolean    wes_matrix_normal( void );
extern GLvoid       wes_matrix_fprintf(FILE *f, matrix4_t *in);
extern GLvoid       wes_matvec4(matrix4_t *m, GLfloat *v, GLfloat *mv);


#endif
