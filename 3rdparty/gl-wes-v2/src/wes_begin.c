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


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "wes.h"
#include "wes_begin.h"
#include "wes_shader.h"
#include "wes_matrix.h"
#include "wes_state.h"

GLenum          vt_mode;
GLuint          vt_count;
attrib_ptr_t    vt_attrib_pointer[WES_ANUM];
vertex_t        vt_vbuffer[WES_BUFFER_COUNT];
GLuint          vt_vbuffer_count;
GLshort         vt_ibuffer[WES_INDEX_COUNT];
GLuint          vt_ibuffer_count;
vertex_t        vt_current[1];
vertex_t        vt_const[1];
GLuint          vt_possize, vt_color0size, vt_color1size,
                vt_coordsize[WES_MULTITEX_NUM], vt_normalsize, vt_fogcoordsize;
GLubyte *nano_extensions_string = NULL;
vertex_t 		vt_ccurrent[1];

GLboolean arraysValid = GL_FALSE;
GLboolean pointersValid = GL_TRUE;

GLuint 		vt_vertcount = 0;
GLuint 		vt_indexarray = 0;
GLuint 		vt_indexcount = 0;
GLuint 		vt_mark = 0;
int 		vt_indexbase = 0;

GLenum          vt_clienttex;
int skipnanogl;
int vboarray;
int vbo_bkp_id;



wrapState2 wrapglState2;

static wrapState2 wrapglInitState2 =
{
	GL_TRUE,
 	0.0f,                            
    1.0f,  
	GL_LESS,
	0x7fffffff,
	GL_BACK,
	GL_ONE,                          
    GL_ZERO,
};
#ifdef WES_WEBGL
//vbo
GLuint vbo_id;
GLuint ibo_id;
//
#endif


GLvoid
wes_reset( void )
{
    int i;
    vt_mode = 0;
    vt_count = 0;
    vt_vertcount = 0;
    vt_indexarray = 0;
    vt_mark = 0;
    vt_indexcount = 0;
    *vt_current = *vt_const;
	if( skipnanogl )
		return;

    for(i = 0; i < WES_ANUM; i++)
    {
        if (vt_attrib_pointer[i].isenabled){
			arraysValid = 0;
            wes_gl->glEnableVertexAttribArray(i);
			if( vt_attrib_pointer[i].vbo_id )
				wes_gl->glBindBuffer(GL_ARRAY_BUFFER, vt_attrib_pointer[i].vbo_id );
			else
#ifdef WES_WEBGL
				wes_gl->glBindBuffer(GL_ARRAY_BUFFER, vt_attrib_pointer[i].webgl_vbo_id );
#else
				wes_gl->glBindBuffer(GL_ARRAY_BUFFER, 0 );
#endif

            wes_gl->glVertexAttribPointer(i, vt_attrib_pointer[i].size, vt_attrib_pointer[i].type,
				GL_FALSE, vt_attrib_pointer[i].stride, vt_attrib_pointer[i].vbo_id ?vt_attrib_pointer[i].ptr:NULL);
		}
    }
}
#ifdef WES_WEBGL
#define VERTEX_START 0
#define INDEX_START 0
#else
#define VERTEX_START (char*)vt_vbuffer
#define INDEX_START (char*)vt_ibuffer
#endif

GLvoid
wes_vertbuffer_flush( void )
{
    if (!vt_count){
        return;
    }

    wes_state_update();

	if( skipnanogl )
		return;
#ifdef WES_WEBGL
	wes_gl->glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
	wes_gl->glBufferData(GL_ARRAY_BUFFER, vt_count*sizeof(vertex_t), vt_vbuffer, GL_STREAM_DRAW);
#else
	wes_gl->glBindBuffer(GL_ARRAY_BUFFER, 0);
#endif
	
	if (!arraysValid)
	{
		wes_gl->glEnableVertexAttribArray(WES_APOS);
		wes_gl->glVertexAttribPointer(WES_APOS, vt_possize, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*)VERTEX_START);


		if (vt_coordsize[0]){
			wes_gl->glEnableVertexAttribArray(WES_ATEXCOORD0);
			wes_gl->glVertexAttribPointer(WES_ATEXCOORD0, vt_coordsize[0], GL_FLOAT, GL_FALSE, sizeof(vertex_t),
				(void*)(VERTEX_START + WES_OFFSET_TEXCOORD0*sizeof(float)));
		} else {
			wes_gl->glDisableVertexAttribArray(WES_ATEXCOORD0);
		}

		if (vt_coordsize[1]){
			wes_gl->glEnableVertexAttribArray(WES_ATEXCOORD1);
			wes_gl->glVertexAttribPointer(WES_ATEXCOORD1, vt_coordsize[1], GL_FLOAT, GL_FALSE, sizeof(vertex_t),
					(void*)(VERTEX_START + WES_OFFSET_TEXCOORD1*sizeof(float)));
		} else {
			wes_gl->glDisableVertexAttribArray(WES_ATEXCOORD1);
		}
		if (vt_coordsize[2]){
			wes_gl->glEnableVertexAttribArray(WES_ATEXCOORD2);
			wes_gl->glVertexAttribPointer(WES_ATEXCOORD2, vt_coordsize[2], GL_FLOAT, GL_FALSE, sizeof(vertex_t),
					(void*)(VERTEX_START + WES_OFFSET_TEXCOORD2*sizeof(float)));
		} else {
			wes_gl->glDisableVertexAttribArray(WES_ATEXCOORD2);
		}
		if (vt_coordsize[3]){
			wes_gl->glEnableVertexAttribArray(WES_ATEXCOORD3);
			wes_gl->glVertexAttribPointer(WES_ATEXCOORD3, vt_coordsize[3], GL_FLOAT, GL_FALSE, sizeof(vertex_t),
					(void*)(VERTEX_START + WES_OFFSET_TEXCOORD3*sizeof(float)));
		} else {
			wes_gl->glDisableVertexAttribArray(WES_ATEXCOORD3);
		}

		if (vt_normalsize){
			wes_gl->glEnableVertexAttribArray(WES_ANORMAL);
			wes_gl->glVertexAttribPointer(WES_ANORMAL, vt_normalsize, GL_FLOAT, GL_FALSE, sizeof(vertex_t),
				(void*)(VERTEX_START + WES_OFFSET_NORMAL*sizeof(float)));
		} else {
			wes_gl->glDisableVertexAttribArray(WES_ANORMAL);
		}

		if (vt_fogcoordsize){
			wes_gl->glEnableVertexAttribArray(WES_AFOGCOORD);
			wes_gl->glVertexAttribPointer(WES_AFOGCOORD, vt_fogcoordsize, GL_FLOAT, GL_FALSE, sizeof(vertex_t),
				(void*)(VERTEX_START + WES_OFFSET_FOGCOORD*sizeof(float)));
		} else {
			wes_gl->glDisableVertexAttribArray(WES_AFOGCOORD);
		}

		if (vt_color0size){
			wes_gl->glEnableVertexAttribArray(WES_ACOLOR0);
			wes_gl->glVertexAttribPointer(WES_ACOLOR0, vt_color0size, GL_FLOAT, GL_FALSE, sizeof(vertex_t),
				(void*)(VERTEX_START + WES_OFFSET_COLOR0*sizeof(float)));
		} else {
			wes_gl->glDisableVertexAttribArray(WES_ACOLOR0);
		}

		if (vt_color1size){
			wes_gl->glEnableVertexAttribArray(WES_ACOLOR1);
			wes_gl->glVertexAttribPointer(WES_ACOLOR1, vt_color1size, GL_FLOAT, GL_FALSE, sizeof(vertex_t),
				(void*)(VERTEX_START + WES_OFFSET_COLOR1*sizeof(float)));
		} else {
			wes_gl->glDisableVertexAttribArray(WES_ACOLOR1);
		}


		arraysValid = GL_TRUE;
		pointersValid = GL_FALSE;
	}
#ifdef WES_WEBGL
	wes_gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_id);
	wes_gl->glBufferData(GL_ELEMENT_ARRAY_BUFFER, vt_vertcount*sizeof(GLushort), vt_ibuffer, GL_STREAM_DRAW);
#else
	wes_gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
#endif

	wes_gl->glDrawElements(GL_TRIANGLES, vt_vertcount, GL_UNSIGNED_SHORT, (void*)INDEX_START);

	
    wes_reset();
}


GLvoid
wes_begin_init( void )
{
    int i;
    vt_clienttex = 0;
    wes_reset();
    vt_const->x = vt_const->y = vt_const->z = 0.0f;
    vt_const->w = 0.0f;
    vt_const->nx = 1.0f;
    vt_const->ny = vt_const->nz = 0.0f;
    vt_const->fog = 0.0f;
    vt_const->cr0 = vt_const->cg0 = vt_const->cb0 = 1.0f;
    vt_const->ca0 = 1.0f;
    vt_const->cr1 = vt_const->cg1 = vt_const->cb1 = 0.0f;
    for(i = 0 ; i != WES_MULTITEX_NUM; i++){
        vt_const->coord[i].s = vt_const->coord[i].t = vt_const->coord[i].r = 0.0f;
        vt_const->coord[i].q = 1.0;
    }
    *vt_current = *vt_const;
	*vt_ccurrent = *vt_const;

    for(i = 0 ; i != WES_ANUM; i++){
        vt_attrib_pointer[i].isenabled = GL_FALSE;
        vt_attrib_pointer[i].size = 0;
        vt_attrib_pointer[i].type = 0;
        vt_attrib_pointer[i].stride = 0;
        vt_attrib_pointer[i].ptr = NULL;
    }

	memcpy(&wrapglState2, &wrapglInitState2, sizeof(wrapState2));
#ifdef WES_WEBGL
	wes_gl->glGenBuffers(1, &vbo_id);
	wes_gl->glGenBuffers(1, &ibo_id);
#endif
}

GLvoid
wes_begin_destroy( void )
{
	if( !nano_extensions_string )
	{
		free(nano_extensions_string);
		nano_extensions_string = NULL;
	}
}

GLvoid
GL_MANGLE(glBegin)(GLenum mode)
{
    vt_mode = mode;

    vt_mark = vt_count;
    vt_indexbase = vt_indexcount;

	/*vt_possize = vt_normalsize = vt_fogcoordsize = 0;
    vt_color1size = vt_color0size = vt_coordsize[0] = vt_coordsize[1] = 0;
*/
	/*if (vt_mode == GL_QUAD_STRIP){
        vt_mode = GL_TRIANGLE_STRIP;
    }
    if (vt_mode == GL_POLYGON){
        vt_mode = GL_TRIANGLE_FAN;
	}*/

    /* Set Constant data */

/*?
    wes_gl->glVertexAttrib4f(WES_APOS, vt_current->x, vt_current->y, vt_current->z, vt_current->w);
    wes_gl->glVertexAttrib4f(WES_ATEXCOORD0, vt_current->coord[0].s, vt_current->coord[0].t, vt_current->coord[0].r, vt_current->coord[0].q);
*/

/*
    wes_gl->glVertexAttrib4f(WES_ATEXCOORD1, vt_current->coord[1].s, vt_current->coord[1].t, vt_current->coord[1].r, vt_current->coord[1].q);
    wes_gl->glVertexAttrib4f(WES_ATEXCOORD2, vt_current->coord[2].s, vt_current->coord[2].t, vt_current->coord[2].r, vt_current->coord[2].q);
    wes_gl->glVertexAttrib4f(WES_ATEXCOORD3, vt_current->coord[3].s, vt_current->coord[3].t, vt_current->coord[3].r, vt_current->coord[3].q);
*/



    wes_gl->glVertexAttrib3f(WES_ANORMAL, vt_current->nx, vt_current->ny, vt_current->nz);
    wes_gl->glVertexAttrib1f(WES_AFOGCOORD, vt_current->fog);

	//wes_gl->glVertexAttrib4f(WES_ACOLOR0, vt_current->cr0, vt_current->cg0, vt_current->cb0, vt_current->ca0);


    //wes_gl->glVertexAttrib3f(WES_ACOLOR1, vt_current->cr1, vt_current->cg1, vt_current->cb1);
    *vt_const = *vt_current;

}


//glVertex
GLvoid
GL_MANGLE(glVertex4f)(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    vt_possize = 4;
    vt_current->x = x;
    vt_current->y = y;
    vt_current->z = z;
    vt_current->w = w;
	vt_current->cr0 = vt_ccurrent->cr0;
	vt_current->cg0 = vt_ccurrent->cg0;
	vt_current->cb0 = vt_ccurrent->cb0;
	vt_current->ca0 = vt_ccurrent->ca0;

    vt_vbuffer[vt_count++] = vt_current[0];
}

GLvoid
GL_MANGLE(glVertex3f)(GLfloat x, GLfloat y, GLfloat z)
{
    if (vt_possize > 3){
	GL_MANGLE(glVertex4f)(x, y, z, 1.0);
    } else {
        vt_possize = 3;
        vt_current->x = x;
        vt_current->y = y;
        vt_current->z = z;

	vt_current->cr0 = vt_ccurrent->cr0;
	vt_current->cg0 = vt_ccurrent->cg0;
	vt_current->cb0 = vt_ccurrent->cb0;
	vt_current->ca0 = vt_ccurrent->ca0;

        vt_vbuffer[vt_count++] = vt_current[0];
    }
}

GLvoid
GL_MANGLE(glVertex3fv)(GLfloat *v)
{
	return GL_MANGLE(glVertex3f)(v[0], v[1], v[2]);
}
/*
GLvoid GL_MANGLE(glBlendFunc)(GLenum sfactor, GLenum dfactor)
{
	wes_vertbuffer_flush();
	wes_gl->glBlendFunc(sfactor, dfactor);
}
*/
/*
GLvoid GL_MANGLE(glDepthMask)( GLboolean flag )
{
	wes_vertbuffer_flush();
	wes_gl->glDepthMask( flag );
}
*/


GLvoid GL_MANGLE(glShadeModel) (GLenum mode)
{
	return;
}

GLvoid GL_MANGLE(glPointSize)( GLfloat size )
{

}



GLvoid
GL_MANGLE(glVertex2f)(GLfloat x, GLfloat y)
{
    if (vt_possize > 2) {
	GL_MANGLE(glVertex3f)(x, y, 0.0);
    } else {
        vt_possize = 2;
        vt_current->x = x;
        vt_current->y = y;

	vt_current->cr0 = vt_ccurrent->cr0;
	vt_current->cg0 = vt_ccurrent->cg0;
	vt_current->cb0 = vt_ccurrent->cb0;
	vt_current->ca0 = vt_ccurrent->ca0;

        vt_vbuffer[vt_count++] = vt_current[0];
    }
}

//glTexCoord
GLvoid
GL_MANGLE(glTexCoord4f)(GLfloat s, GLfloat t, GLfloat r, GLfloat q)
{
    vt_coordsize[0] = 4;
    vt_current->coord[0].s = s;
    vt_current->coord[0].t = t;
    vt_current->coord[0].r = r;
    vt_current->coord[0].q = q;
}

GLvoid
GL_MANGLE(glTexCoord3f)(GLfloat s, GLfloat t, GLfloat r)
{
    if (vt_coordsize[0] > 3){
	GL_MANGLE(glTexCoord4f)(s, t, r, 0);
    } else {
        vt_coordsize[0] = 3;
        vt_current->coord[0].s = s;
        vt_current->coord[0].t = t;
        vt_current->coord[0].r = r;
    }
}

GLvoid
GL_MANGLE(glTexCoord2f)(GLfloat s, GLfloat t)
{
    if (vt_coordsize[0] > 2){
	GL_MANGLE(glTexCoord3f)(s, t, 0);
    } else {
        vt_coordsize[0] = 2;
        vt_current->coord[0].s = s;
        vt_current->coord[0].t = t;
    }
}

GLvoid
GL_MANGLE(glTexCoord1f)(GLfloat s)
{
    if (vt_coordsize[0] > 1){
	GL_MANGLE(glTexCoord2f)(s, 0);
    } else {
        vt_coordsize[0] = 1;
        vt_current->coord[0].s = s;
    }
}

//glMultiTexCoord
GLvoid
GL_MANGLE(glMultiTexCoord4f)(GLenum tex, GLfloat s, GLfloat t, GLfloat r, GLfloat q)
{
    GLuint ind = tex - GL_TEXTURE0;
    vt_coordsize[ind] = 4;
    vt_current->coord[ind].s = s;
    vt_current->coord[ind].t = t;
    vt_current->coord[ind].r = r;
    vt_current->coord[ind].q = q;
}

GLvoid
GL_MANGLE(glMultiTexCoord3f)(GLenum tex, GLfloat s, GLfloat t, GLfloat r)
{
    GLuint ind = tex - GL_TEXTURE0;
    if (vt_coordsize[ind] > 3){
	GL_MANGLE(glMultiTexCoord4f)(tex, s, t, r, 1.0);
    } else {
        vt_coordsize[ind] = 3;
        vt_current->coord[ind].s = s;
        vt_current->coord[ind].t = t;
        vt_current->coord[ind].r = r;
    }
}

GLvoid
GL_MANGLE(glMultiTexCoord2f)(GLenum tex, GLfloat s, GLfloat t)
{
    GLuint ind = tex - GL_TEXTURE0;
    if (vt_coordsize[ind] > 2){
	GL_MANGLE(glMultiTexCoord3f)(tex, s, t, 0.0);
    } else {
        vt_coordsize[ind] = 2;
        vt_current->coord[ind].s = s;
        vt_current->coord[ind].t = t;
    }
}

GLvoid
GL_MANGLE(glMultiTexCoord1f)(GLenum tex, GLfloat s)
{
    GLuint ind = tex - GL_TEXTURE0;
    if (vt_coordsize[ind] > 1){
	GL_MANGLE(glMultiTexCoord2f)(tex, s, 0.0);
    } else {
        vt_coordsize[ind] = 1;
        vt_current->coord[ind].s = s;
    }
}

//glNormal
GLvoid
GL_MANGLE(glNormal3f)(GLfloat x, GLfloat y, GLfloat z)
{
    vt_normalsize = 3;
    vt_current->nx = x;
    vt_current->ny = y;
    vt_current->nz = z;
}

GLvoid
GL_MANGLE(glNormal3fv)( const GLfloat *v )
{
    vt_normalsize = 3;
    vt_current->nx = v[0];
    vt_current->ny = v[1];
    vt_current->nz = v[2];
}

//glFogCoord
GLvoid
GL_MANGLE(glFogCoordf)(GLfloat f)
{
    vt_fogcoordsize = 1;
    vt_current->fog = f;
}

//glColor
GLvoid
GL_MANGLE(glColor4f)(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
    vt_color0size = 4;
    vt_ccurrent->cr0 = r;
    vt_ccurrent->cg0 = g;
    vt_ccurrent->cb0 = b;
	vt_ccurrent->ca0 = a;

    vt_current->cr0 = vt_ccurrent->cr0;
    vt_current->cg0 = vt_ccurrent->cg0;
    vt_current->cb0 = vt_ccurrent->cb0;
    vt_current->ca0 = vt_ccurrent->ca0;
	if( !vt_attrib_pointer[WES_ACOLOR0].isenabled )
		wes_gl->glVertexAttrib4f(WES_ACOLOR0, r, g, b, a);

}

GLvoid
GL_MANGLE(glColor3f)(GLfloat r, GLfloat g, GLfloat b)
{
    if (vt_color0size > 3){
	GL_MANGLE(glColor4f)(r, g, b, 1);
    } else {
        vt_color0size = 3;
        vt_ccurrent->cr0 = r;
        vt_ccurrent->cg0 = g;
        vt_ccurrent->cb0 = b;

    vt_current->cr0 = vt_ccurrent->cr0;
    vt_current->cg0 = vt_ccurrent->cg0;
    vt_current->cb0 = vt_ccurrent->cb0;
    }
}

float ClampToFloat(GLubyte value)
    {
    float retval = (float)(value);
    if (retval > 1)
        {
        retval = 1;
        }
    return retval;
    }

const GLfloat ubtofloat = 1.0f / 255.0f;

GLvoid
GL_MANGLE(glColor4ub)(GLubyte r, GLubyte g, GLubyte b, GLubyte a)
{
    vt_color0size = 4;
    vt_ccurrent->cr0 = r / 255.0f;// * ubtofloat;
    vt_ccurrent->cg0 = g / 255.0f;// * ubtofloat;
    vt_ccurrent->cb0 = b / 255.0f;// * ubtofloat;
	vt_ccurrent->ca0 = a / 255.0f;// * ubtofloat;

    vt_current->cr0 = vt_ccurrent->cr0;
    vt_current->cg0 = vt_ccurrent->cg0;
    vt_current->cb0 = vt_ccurrent->cb0;
    vt_current->ca0 = vt_ccurrent->ca0;
}

GLvoid
GL_MANGLE(glColor4ubv)( GLubyte *p )
{
	GL_MANGLE(glColor4ub)( p[0], p[1], p[2], p[3] );
}

GLvoid
GL_MANGLE(glColor3ub)(GLubyte r, GLubyte g, GLubyte b)
{
    if (vt_color0size > 3){
	GL_MANGLE(glColor4ub)(r, g, b, 255);
    } else {
        vt_color0size = 3;
        vt_ccurrent->cr0 = r / 255.0f;// * ubtofloat;
        vt_ccurrent->cg0 = g / 255.0f;// * ubtofloat;
        vt_ccurrent->cb0 = b / 255.0f;// * ubtofloat;

   	 	vt_current->cr0 = vt_ccurrent->cr0;
   	 	vt_current->cg0 = vt_ccurrent->cg0;
    	vt_current->cb0 = vt_ccurrent->cb0;
    }
}


//glSecondaryColor
GLvoid
GL_MANGLE(glSecondaryColor3f)(GLfloat r, GLfloat g, GLfloat b){
    vt_color1size = 3;
    vt_current->cr1 = r;
    vt_current->cg1 = g;
    vt_current->cb1 = b;
}

GLvoid
GL_MANGLE(glEnd)( void )
{

if (vt_count < 3)
{
   return;
}

switch (vt_mode)
        {
	case GL_QUADS:
            {
		//int i;
		//int num = (vt_count-vt_mark) / 4;
		//for(i = 0; i < num; i += 1){
		    vt_ibuffer[vt_vertcount++] = vt_indexcount;
		    vt_ibuffer[vt_vertcount++] = vt_indexcount + 1;
		    vt_ibuffer[vt_vertcount++] = vt_indexcount + 2;
		    vt_ibuffer[vt_vertcount++] = vt_indexcount;	  //2
		    vt_ibuffer[vt_vertcount++] = vt_indexcount + 2;//3
		    vt_ibuffer[vt_vertcount++] = vt_indexcount + 3;//0
		    vt_indexcount+=4;
		//}
            }
            break;
        case GL_TRIANGLES:
            {
		int i;
		int num = (vt_count-vt_mark) / 3;
		for(i = 0; i < num; i += 1)
		{
		    vt_ibuffer[vt_vertcount++] = vt_indexcount;
		    vt_ibuffer[vt_vertcount++] = vt_indexcount + 1;
		    vt_ibuffer[vt_vertcount++] = vt_indexcount + 2;
		    vt_indexcount+=3;
		}


            }
            break;

        case GL_TRIANGLE_STRIP:
            {
/*
		int i;
		int num = (vt_count-vt_mark);

		for(i = 0; i < num; i += 1)
		{
		    vt_ibuffer[vt_vertcount++] = vt_indexcount++;
		}
*/

		int rev = 0;
		int i;
		int num = (vt_count-vt_mark) - 3;
		vt_ibuffer[vt_vertcount++] = vt_indexcount++;
		vt_ibuffer[vt_vertcount++] = vt_indexcount++;
		vt_ibuffer[vt_vertcount++] = vt_indexcount++;

		for(i = 0; i < num; i += 1)
		{
if (!rev)
{
		    vt_ibuffer[vt_vertcount++] = vt_indexcount-1;
		    vt_ibuffer[vt_vertcount++] = vt_indexcount-2;
		    vt_ibuffer[vt_vertcount++] = vt_indexcount;
		    vt_indexcount++;
rev=1;
}
else
{
		    vt_ibuffer[vt_vertcount++] = vt_indexcount-2;
		    vt_ibuffer[vt_vertcount++] = vt_indexcount-1;
		    vt_ibuffer[vt_vertcount++] = vt_indexcount;
		    vt_indexcount++;
rev=0;	
}
		}


/*
		vt_ibuffer[vt_indexarray++] = vt_indexcount;
            	vt_ibuffer[vt_indexarray++] = vt_indexcount+1;
            	vt_ibuffer[vt_indexarray++] = vt_indexcount+2;

            vt_indexcount+=3;
            int vcount = (vt_count-vt_mark)-3;
            if (vcount && ((long)vt_indexarray & 0x02))
                {
                vt_ibuffer[vt_indexarray++] = vt_indexcount-1; // 2 
                vt_ibuffer[vt_indexarray++] = vt_indexcount-2; // 1
                vt_ibuffer[vt_indexarray++] = vt_indexcount;   // 3
                vt_indexcount++;
                vcount-=1;    
                int odd = vcount&1;
                vcount/=2;
		int count = 0;

                for (count = 0; count < vcount; count += 1)
                    {
                    vt_ibuffer[vt_indexarray++] = (vt_indexcount-2) | ((vt_indexcount-1)<<16);                    
                    vt_ibuffer[vt_indexarray++] = (vt_indexcount) | ((vt_indexcount)<<16);                    
                    vt_ibuffer[vt_indexarray++] = (vt_indexcount-1) | ((vt_indexcount+1)<<16);
                    vt_indexcount+=2;
                    }

                if (odd)
                    {
                    vt_ibuffer[vt_indexarray++] = vt_indexcount-2; // 2 
                    vt_ibuffer[vt_indexarray++] = vt_indexcount-1; // 1
                    vt_ibuffer[vt_indexarray++] = vt_indexcount;   // 3
                    vt_indexcount++;    
                    }
                }
           else
                {
                //already aligned
                int odd = vcount&1;
                vcount/=2;
		int count = 0;

                for (count = 0; count < vcount; count += 1)
                    {                    
                    vt_ibuffer[vt_indexarray++] = (vt_indexcount-1) | ((vt_indexcount-2)<<16);                    
                    vt_ibuffer[vt_indexarray++] = (vt_indexcount) | ((vt_indexcount-1)<<16);                    
                    vt_ibuffer[vt_indexarray++] = (vt_indexcount) | ((vt_indexcount+1)<<16);
                    vt_indexcount+=2;

                    }

                if (odd)
                    {
                    
                    vt_ibuffer[vt_indexarray++] = vt_indexcount-1; // 2 
                    vt_ibuffer[vt_indexarray++] = vt_indexcount-2; // 1
                    vt_ibuffer[vt_indexarray++] = vt_indexcount;   // 3
                    vt_indexcount++;    
                    }
                }            
		vt_vertcount=vt_count+(vt_count-vt_mark-3)*2;
*/
            }


            break;
        case GL_POLYGON:
        case GL_TRIANGLE_FAN:
            {
		int i;
		int num = (vt_count-vt_mark) - 3;
		vt_indexbase = vt_indexcount;
		vt_ibuffer[vt_vertcount++] = vt_indexcount++;
		vt_ibuffer[vt_vertcount++] = vt_indexcount++;
		vt_ibuffer[vt_vertcount++] = vt_indexcount++;


		for(i = 0; i < num; i += 1)
		{
		    vt_ibuffer[vt_vertcount++] = vt_indexbase;
		    vt_ibuffer[vt_vertcount++] = vt_indexcount-1;
		    vt_ibuffer[vt_vertcount++] = vt_indexcount++;
		}

             }   
            break;

        default:
		LOGI("%x: uniplemented", vt_mode);
 		//wes_gl->glDrawArrays(vt_mode, 0, vt_count);
            break;
        }

   //wes_vertbuffer_flush();
	if( vt_count > 20000 || vt_vertcount > 40000 )
		wes_vertbuffer_flush();
}


static inline void wes_validate_pointers( void )
{
	int i;
	arraysValid = GL_FALSE;

	if( pointersValid )
		return;
#ifndef WES_WEBGL
	for( i = 0; i < WES_ANUM; i++ )
	{
		if( vt_attrib_pointer[i].isenabled )
		{
			wes_gl->glBindBuffer(GL_ARRAY_BUFFER, vt_attrib_pointer[i].vbo_id);
			wes_gl->glEnableVertexAttribArray(i);
			wes_gl->glVertexAttribPointer(i, vt_attrib_pointer[i].size, vt_attrib_pointer[i].type, i == WES_ACOLOR0 || i == WES_ACOLOR1, vt_attrib_pointer[i].stride, vt_attrib_pointer[i].ptr);
		}
		else
			wes_gl->glDisableVertexAttribArray(i);
	}
	wes_gl->glBindBuffer(GL_ARRAY_BUFFER, vbo_bkp_id);
#else
	for( i = 0; i < WES_ANUM; i++ )
	{
		if( vt_attrib_pointer[i].isenabled )
			wes_gl->glEnableVertexAttribArray(i);
		else
			wes_gl->glDisableVertexAttribArray(i);
	}
#endif
	pointersValid = GL_TRUE;
}

GLvoid
GL_MANGLE(glVertexPointer)(GLint size, GLenum type, GLsizei stride, const GLvoid *ptr)
{
   // wes_vertbuffer_flush(); ?
    vt_attrib_pointer[WES_APOS].isenabled = GL_TRUE;
    vt_attrib_pointer[WES_APOS].size = size;
    vt_attrib_pointer[WES_APOS].type = type;
    vt_attrib_pointer[WES_APOS].stride = stride;
    vt_attrib_pointer[WES_APOS].ptr = ptr;
	vt_attrib_pointer[WES_APOS].vbo_id = vbo_bkp_id;
#ifdef WES_WEBGL
	wes_gl->glBindBuffer(GL_ARRAY_BUFFER, vbo_bkp_id);
	if( vbo_bkp_id )
#endif
	wes_gl->glVertexAttribPointer(WES_APOS, size, type, GL_FALSE, stride, ptr);
	wes_validate_pointers();
}

GLvoid
GL_MANGLE(glNormalPointer)(GLenum type, GLsizei stride, const GLvoid *ptr)
{
    wes_vertbuffer_flush();
    vt_attrib_pointer[WES_ANORMAL].isenabled = GL_TRUE;
    vt_attrib_pointer[WES_ANORMAL].size = 3;
    vt_attrib_pointer[WES_ANORMAL].type = type;
    vt_attrib_pointer[WES_ANORMAL].stride = stride;
    vt_attrib_pointer[WES_ANORMAL].ptr = ptr;
	vt_attrib_pointer[WES_ANORMAL].vbo_id = vbo_bkp_id;
#ifdef WES_WEBGL
	wes_gl->glBindBuffer(GL_ARRAY_BUFFER, vbo_bkp_id);
	if( vbo_bkp_id )
#endif
	wes_gl->glVertexAttribPointer(WES_ANORMAL, 3, type, GL_FALSE, stride, ptr);
	wes_validate_pointers();
}

GLvoid
GL_MANGLE(glColorPointer)(GLint size, GLenum type, GLsizei stride, const GLvoid *ptr)
{
	wes_vertbuffer_flush();
    vt_attrib_pointer[WES_ACOLOR0].isenabled = GL_TRUE;
    vt_attrib_pointer[WES_ACOLOR0].size = size;
    vt_attrib_pointer[WES_ACOLOR0].type = type;
    vt_attrib_pointer[WES_ACOLOR0].stride = stride;
    vt_attrib_pointer[WES_ACOLOR0].ptr = ptr;
	vt_attrib_pointer[WES_ACOLOR0].vbo_id = vbo_bkp_id;
#ifdef WES_WEBGL
	wes_gl->glBindBuffer(GL_ARRAY_BUFFER, vbo_bkp_id);
	if( vbo_bkp_id )
#endif
	wes_gl->glVertexAttribPointer(WES_ACOLOR0, size, type, GL_TRUE, stride, ptr);
	wes_validate_pointers();
}

GLvoid
GL_MANGLE(glTexCoordPointer)(GLint size, GLenum type, GLsizei stride, const GLvoid *ptr)
{
    int i = WES_ATEXCOORD0 + vt_clienttex;
    //wes_vertbuffer_flush(); ?
    vt_attrib_pointer[i].isenabled = GL_TRUE;
    vt_attrib_pointer[i].size = size;
    vt_attrib_pointer[i].type = type;
    vt_attrib_pointer[i].stride = stride;
    vt_attrib_pointer[i].ptr = ptr;
	vt_attrib_pointer[i].vbo_id = vbo_bkp_id;
#ifdef WES_WEBGL
	wes_gl->glBindBuffer(GL_ARRAY_BUFFER, vbo_bkp_id);
	if( vbo_bkp_id )
#endif
	wes_gl->glVertexAttribPointer(i, size, type, GL_FALSE, stride, ptr);
	wes_validate_pointers();
}

GLvoid
GL_MANGLE(glSecondaryColorPointer)(GLint size, GLenum type, GLsizei stride, const GLvoid *ptr)
{
    wes_vertbuffer_flush();
    vt_attrib_pointer[WES_ACOLOR1].isenabled = GL_TRUE;
    vt_attrib_pointer[WES_ACOLOR1].size = size;
    vt_attrib_pointer[WES_ACOLOR1].type = type;
    vt_attrib_pointer[WES_ACOLOR1].stride = stride;
    vt_attrib_pointer[WES_ACOLOR1].ptr = ptr;
	vt_attrib_pointer[WES_ACOLOR1].vbo_id = vbo_bkp_id;
#ifdef WES_WEBGL
	wes_gl->glBindBuffer(GL_ARRAY_BUFFER, vbo_bkp_id);
	if( vbo_bkp_id )
#endif
    wes_gl->glVertexAttribPointer(WES_ACOLOR1, size, type, GL_FALSE, stride, ptr);
	wes_validate_pointers();
}

GLvoid
GL_MANGLE(glFogCoordPointer)(GLenum type, GLsizei stride, const GLvoid *ptr)
{
    wes_vertbuffer_flush();
    vt_attrib_pointer[WES_AFOGCOORD].isenabled = GL_TRUE;
    vt_attrib_pointer[WES_AFOGCOORD].size = 1;
    vt_attrib_pointer[WES_AFOGCOORD].type = type;
    vt_attrib_pointer[WES_AFOGCOORD].stride = stride;
    vt_attrib_pointer[WES_AFOGCOORD].ptr = ptr;
	vt_attrib_pointer[WES_AFOGCOORD].vbo_id = vbo_bkp_id;
#ifdef WES_WEBGL
	wes_gl->glBindBuffer(GL_ARRAY_BUFFER, vbo_bkp_id);
	if( vbo_bkp_id )
#endif
    wes_gl->glVertexAttribPointer(WES_AFOGCOORD, 1, type, GL_FALSE, stride, ptr);
	wes_validate_pointers();
}

GLvoid
GL_MANGLE(glEnableClientState)(GLenum array)
{
	wes_vertbuffer_flush();// ?

	arraysValid = GL_FALSE;

	switch(array)
    {
        case GL_VERTEX_ARRAY:
            wes_gl->glEnableVertexAttribArray(WES_APOS);
            vt_attrib_pointer[WES_APOS].isenabled = GL_TRUE;
			if( skipnanogl )
				vboarray = 1;
            break;
        case GL_NORMAL_ARRAY:
            wes_gl->glEnableVertexAttribArray(WES_ANORMAL);
            vt_attrib_pointer[WES_ANORMAL].isenabled = GL_TRUE;
            break;
        case GL_COLOR_ARRAY:
            wes_gl->glEnableVertexAttribArray(WES_ACOLOR0);
            vt_attrib_pointer[WES_ACOLOR0].isenabled = GL_TRUE;
            break;
        case GL_SECONDARY_COLOR_ARRAY:
            wes_gl->glEnableVertexAttribArray(WES_ACOLOR1);
            vt_attrib_pointer[WES_ACOLOR1].isenabled = GL_TRUE;
            break;
        case GL_FOG_COORD_ARRAY:
            wes_gl->glEnableVertexAttribArray(WES_AFOGCOORD);
            vt_attrib_pointer[WES_AFOGCOORD].isenabled = GL_TRUE;
            break;
        case GL_TEXTURE_COORD_ARRAY:
            wes_gl->glEnableVertexAttribArray(WES_ATEXCOORD0 + vt_clienttex);
            vt_attrib_pointer[WES_ATEXCOORD0 + vt_clienttex].isenabled = GL_TRUE;
            break;
        default:
            PRINT_ERROR("EnableClientState Unhandled enum");
    }

	wes_validate_pointers();
}

GLvoid
GL_MANGLE(glDisableClientState)(GLenum array)
{
	//wes_vertbuffer_flush(); //?

    switch(array)
    {
        case GL_VERTEX_ARRAY:
            wes_gl->glDisableVertexAttribArray(WES_APOS);
            vt_attrib_pointer[WES_APOS].isenabled = GL_FALSE;
			if( skipnanogl )
				vboarray = 0;
            break;
        case GL_NORMAL_ARRAY:
            wes_gl->glDisableVertexAttribArray(WES_ANORMAL);
            vt_attrib_pointer[WES_ANORMAL].isenabled = GL_FALSE;
            break;
        case GL_COLOR_ARRAY:
            wes_gl->glDisableVertexAttribArray(WES_ACOLOR0);
            vt_attrib_pointer[WES_ACOLOR0].isenabled = GL_FALSE;
            break;
        case GL_SECONDARY_COLOR_ARRAY:
            wes_gl->glDisableVertexAttribArray(WES_ACOLOR1);
            vt_attrib_pointer[WES_ACOLOR1].isenabled = GL_FALSE;
            break;
        case GL_FOG_COORD_ARRAY:
            wes_gl->glDisableVertexAttribArray(WES_AFOGCOORD);
            vt_attrib_pointer[WES_AFOGCOORD].isenabled = GL_FALSE;
            break;
        case GL_TEXTURE_COORD_ARRAY:
            wes_gl->glDisableVertexAttribArray(WES_ATEXCOORD0 + vt_clienttex);
            vt_attrib_pointer[WES_ATEXCOORD0 + vt_clienttex].isenabled = GL_FALSE;
            break;
        default:
            PRINT_ERROR("DisableClientState Unhandled enum");
    }

	wes_validate_pointers();
}

GLvoid
GL_MANGLE(glInterleavedArrays)(GLenum format, GLsizei stride, const GLvoid *pointer)
{
    GLint et, ec, en, st, sc, sv, tc, pc, pn, pv, s;
    GLint str;
    GLint f = sizeof(GLfloat);
    GLint c = 4 * sizeof(GLubyte);

    et = ec = en = GL_FALSE;
    st = sc = sv = 0;
    tc = GL_FLOAT;
    pc = pn = pv = s = 0;

    switch(format)
    {
        case GL_V2F:
            et = ec = en = GL_FALSE;
            sv = 2;
            pv = 0;
            s = 2 * f;
            break;

        case GL_V3F:
            et = ec = en = GL_FALSE;
            sv = 3;
            pv = 0;
            s = 3 * f;
            break;

        case GL_C4UB_V2F:
            ec = GL_TRUE; et = en = GL_FALSE;
            sv = 2; sc = 4;
            tc = GL_UNSIGNED_BYTE;
            pc = 0; pv = c;
            s = c + 2 * f;
            break;

        case GL_C4UB_V3F:
            ec = GL_TRUE; et = en = GL_FALSE;
            sv = 3; sc = 4;
            tc = GL_UNSIGNED_BYTE;
            pc = 0; pv = c;
            s = c + 3 * f;
            break;

        case GL_C3F_V3F:
            ec = GL_TRUE; et = en = GL_FALSE;
            sv = 3; sc = 3;
            tc = GL_FLOAT;
            pc = 0; pv = 3 * f;
            s = 6 * f;
            break;

        case GL_N3F_V3F:
            en = GL_TRUE; et = ec = GL_FALSE;
            sv = 3;
            pn = 0; pv = 3 * f;
            s = 6 * f;
            break;

        case GL_C4F_N3F_V3F:
            en = ec = GL_TRUE; et = GL_FALSE;
            sv = 3; sc = 4;
            tc = GL_FLOAT;
            pc = 0; pn = 4 * f; pv = 7 * f;
            s = 10 * f;
            break;

        case GL_T2F_V3F:
            et = GL_TRUE; en = ec = GL_FALSE;
            sv = 3; st = 2;
            pv = 2 * f;
            s = 5 * f;
            break;

        case GL_T4F_V4F:
            et = GL_TRUE; en = ec = GL_FALSE;
            sv = 4; st = 4;
            pv = 4 * f;
            s = 8 * f;
            break;

        case GL_T2F_C4UB_V3F:
            et = ec = GL_TRUE; en = GL_FALSE;
            sv = 3; st = 2; sc = 4;
            tc = GL_UNSIGNED_BYTE;
            pc = 2 * f; pv = 2 * f + c;
            s = 5 * f + c;
            break;

        case GL_T2F_C3F_V3F:
            et = ec = GL_TRUE; en = GL_FALSE;
            sv = 3; st = 2; sc = 3;
            tc = GL_FLOAT;
            pc = 2 * f; pv = 5 * f;
            s = 8 * f;
            break;

        case GL_T2F_N3F_V3F:
            et = en = GL_TRUE; ec = GL_FALSE;
            sv = 3; st = 2;
            pn = 2 * f; pv = 5 * f;
            s = 8 * f;
            break;

        case GL_T2F_C4F_N3F_V3F:
            en = et = ec = GL_TRUE;
            sv = 3; st = 2; sc = 4;
            tc = GL_FLOAT;
            pc = 2 * f; pn = 6 * f ; pv = 9 * f;
            s = 12 * f;
            break;

        case GL_T4F_C4F_N3F_V4F:
            en = et = ec = GL_TRUE;
            sv = 4; st = 4; sc = 4;
            tc = GL_FLOAT;
            pc = 4 * f; pn = 8 * f ; pv = 12 * f;
            s = 16 * f;
            break;
    }

    str = (stride == 0) ? (s) : (stride);

    wes_gl->glDisableVertexAttribArray(WES_AFOGCOORD);
    wes_gl->glDisableVertexAttribArray(WES_ACOLOR1);

    wes_gl->glEnableVertexAttribArray(WES_APOS);
    wes_gl->glVertexAttribPointer(WES_APOS, sv, GL_FLOAT, GL_FALSE, str, (GLubyte*)pointer + pv);
    if (et) {
        wes_gl->glEnableVertexAttribArray(WES_ATEXCOORD0 + vt_clienttex);
        wes_gl->glVertexAttribPointer(WES_ATEXCOORD0 + vt_clienttex, st, GL_FLOAT, GL_FALSE, str, pointer);
    } else {
        wes_gl->glDisableVertexAttribArray(WES_ATEXCOORD0 + vt_clienttex);
    }
    if (ec) {
        wes_gl->glEnableVertexAttribArray(WES_ACOLOR0);
        wes_gl->glVertexAttribPointer(WES_ACOLOR0, st, tc, GL_FALSE, str, (GLubyte*)pointer + pc);
    } else {
        wes_gl->glDisableVertexAttribArray(WES_ACOLOR0);
    }
    if (en) {
        wes_gl->glEnableVertexAttribArray(WES_ANORMAL);
        wes_gl->glVertexAttribPointer(WES_ANORMAL, 3, GL_FLOAT, GL_FALSE, str, (GLubyte*)pointer + pn);
    } else {
        wes_gl->glDisableVertexAttribArray(WES_ANORMAL);
    }
}

GLvoid
GL_MANGLE(glClientActiveTexture)(GLenum texture)
{
   // wes_vertbuffer_flush();
    vt_clienttex = texture - GL_TEXTURE0;
}

GLvoid
GL_MANGLE(glClientActiveTextureARB)(GLenum texture)
{
   // wes_vertbuffer_flush();
	vt_clienttex = texture - GL_TEXTURE0;
}

GLvoid
GL_MANGLE(glActiveTextureARB)(GLenum texture)
{
	wes_vertbuffer_flush();
	vt_clienttex = texture - GL_TEXTURE0;
	wes_gl->glActiveTexture( texture );
	u_activetex = texture - GL_TEXTURE0;
}


#ifdef WES_WEBGL
/*
Setup VBO for WebGL
*/
static void wes_vertex_attrib_pointer(int i, int count, GLboolean norm)
{
	long ptrdiff;
	int stride;

	if( !vt_attrib_pointer[i].isenabled || vt_attrib_pointer[i].vbo_id )
		return;

	if( !vt_attrib_pointer[i].webgl_vbo_id )
		wes_gl->glGenBuffers(1, &vt_attrib_pointer[i].webgl_vbo_id );

	// detect if we can fit multiple arrays to single VBO
	ptrdiff = (char*)vt_attrib_pointer[i].ptr - (char*)vt_attrib_pointer[0].ptr;
	stride = vt_attrib_pointer[i].stride;

	// detect stride by type
	if( stride == 0 )
	{
		if( vt_attrib_pointer[i].type == GL_UNSIGNED_BYTE )
			stride = vt_attrib_pointer[i].size;
		else
			stride = vt_attrib_pointer[i].size * 4;
	}

	if( i && vt_attrib_pointer[0].isenabled && !vt_attrib_pointer[0].vbo_id && ptrdiff > 0 && ptrdiff < stride )
	{
		// reuse existing array
		wes_gl->glBindBuffer( GL_ARRAY_BUFFER, vt_attrib_pointer[0].webgl_vbo_id );
		wes_gl->glVertexAttribPointer(i, vt_attrib_pointer[i].size, vt_attrib_pointer[i].type, norm, vt_attrib_pointer[i].stride, ptrdiff);
	}
	else
	{
		wes_gl->glBindBuffer( GL_ARRAY_BUFFER, vt_attrib_pointer[i].webgl_vbo_id );
		//printf("BufferData %d %d\n",vt_attrib_pointer[i].webgl_vbo_id, (count + 4) * stride );
		wes_gl->glBufferData( GL_ARRAY_BUFFER, (count + 4) * stride, (void*)vt_attrib_pointer[i].ptr, GL_STREAM_DRAW);
		wes_gl->glVertexAttribPointer(i, vt_attrib_pointer[i].size, vt_attrib_pointer[i].type, norm, vt_attrib_pointer[i].stride, 0);
	}
}
#endif


GLvoid
GL_MANGLE(glDrawArrays)(GLenum mode, GLint off, GLint num)
{
    wes_vertbuffer_flush();
    wes_state_update();
#ifdef WES_WEBGL
	wes_state_update();
	wes_vertex_attrib_pointer( WES_APOS, off + num, GL_FALSE );
	wes_vertex_attrib_pointer( WES_ACOLOR0, off + num, GL_TRUE );
	wes_vertex_attrib_pointer( WES_ATEXCOORD0, off + num, GL_FALSE );
	wes_vertex_attrib_pointer( WES_ATEXCOORD1, off + num, GL_FALSE );
	wes_vertex_attrib_pointer( WES_ATEXCOORD2, off + num, GL_FALSE );
	wes_vertex_attrib_pointer( WES_ATEXCOORD3, off + num, GL_FALSE );


#endif
	wes_gl->glBindBuffer( GL_ARRAY_BUFFER, vbo_bkp_id );
	wes_gl->glDrawArrays(mode, off, num);
}

/*
void GL_MANGLE(glDepthRange)(GLclampf zNear, GLclampf zFar)
{
	wes_gl->glDepthRangef( zNear, zFar );
}
*/
/*
void GL_MANGLE(glDepthFunc) (GLenum func)
{
	wes_vertbuffer_flush();
	wes_gl->glDepthFunc( func );
}
*/
/*GLvoid GL_MANGLE(glFinish)( void )
{
	wes_state_update();
}
*/
GLvoid GL_MANGLE(glPolygonMode)( void )
{

}

GLvoid GL_MANGLE(glPolygonOffset)( GLfloat factor, GLfloat units )
{
	wes_vertbuffer_flush();
	wes_gl->glPolygonOffset(factor, units);
}
#ifdef WES_WEBGL
GLvoid GL_MANGLE(glDrawRangeElements)( GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices )
{
	wes_vertbuffer_flush();

	wes_state_update();
	wes_vertex_attrib_pointer( WES_APOS, end, GL_FALSE );
	wes_vertex_attrib_pointer( WES_ACOLOR0, end, GL_TRUE );
	wes_vertex_attrib_pointer( WES_ATEXCOORD0, end, GL_FALSE );
	wes_vertex_attrib_pointer( WES_ATEXCOORD1, end, GL_FALSE );
	wes_vertex_attrib_pointer( WES_ATEXCOORD2, end, GL_FALSE );
	wes_vertex_attrib_pointer( WES_ATEXCOORD3, end, GL_FALSE );


	wes_gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_id);
	wes_gl->glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * 2, indices, GL_STREAM_DRAW);
	wes_gl->glBindBuffer(GL_ARRAY_BUFFER, vbo_bkp_id);

	wes_gl->glDrawElements(mode, count, type, 0);
}

GLvoid GL_MANGLE(glDrawElements)(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices )
{
	// As emscripten does not have memory acces restrictions, it should be safe, but slow
    GL_MANGLE(glDrawRangeElements)(mode, 0, 65536, count, type, indices );
}
#else
GLvoid GL_MANGLE(glDrawElements)( GLenum mode, GLsizei count, GLenum type, const GLvoid *indices )
{
	wes_vertbuffer_flush();
	//glClientActiveTexture( GL_TEXTURE0 );
	wes_state_update();
	wes_validate_pointers();
	wes_gl->glDrawElements(mode, count, type, indices);
}
GLvoid GL_MANGLE(glDrawRangeElements)( GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices )
{
	wes_vertbuffer_flush();
	//glClientActiveTexture( GL_TEXTURE0 );
	wes_state_update();
	wes_validate_pointers();
	wes_gl->glDrawElements(mode, count, type, indices);
}
#endif

GLvoid GL_MANGLE(glFrontFace) (GLenum mode)
{
	wes_vertbuffer_flush();
	wes_gl->glFrontFace(mode);
}
/*
GLvoid GL_MANGLE(glPolygonOffset)( GLfloat factor, GLfloat units )
{
    wes_vertbuffer_flush();
    wes_gl->glPolygonOffset(factor, units);
}
*/
GLvoid GL_MANGLE(glDeleteTextures)( GLsizei n, const GLuint *textures )
{
    wes_vertbuffer_flush();
    wes_gl->glDeleteTextures(n,textures);

}

GLvoid GL_MANGLE(glClearColor) (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{

	wes_vertbuffer_flush();
	wes_gl->glClearColor(red,green,blue,alpha);
}

GLenum GL_MANGLE(glGetError)( void )
{
	return wes_gl->glGetError();//GL_NO_ERROR;
}

GLvoid GL_MANGLE(glLineWidth)( GLfloat width ) {}

/*
GLvoid GL_MANGLE(glTexParameteri)(GLenum target, GLenum pname, GLint param)
{
	if (pname == 0x1004) { // GL_TEXTURE_BORDER_COLOR
		return; // not supported by opengl es
	}
	if (    (pname == GL_TEXTURE_WRAP_S ||
			 pname == GL_TEXTURE_WRAP_T) &&
			 param == 0x2900)   { // GL_CLAMP
		param = 0x812F;
	}

	wes_vertbuffer_flush();
	wes_gl->glTexParameteri(target, pname, param);
}*/

void GL_MANGLE(glArrayElement)(GLint i) {}
void GL_MANGLE(glCallList)( GLuint list ) {}
void GL_MANGLE(glColorMask)( GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha ) {}
void GL_MANGLE(glStencilFunc)( GLenum func, GLint ref, GLuint mask ) {}
void GL_MANGLE(glStencilOp)( GLenum fail, GLenum zfail, GLenum zpass ) {}
void GL_MANGLE(glStencilMask)( GLuint mask ) {}
void GL_MANGLE(glClearStencil)( GLint s ) {}

const GLubyte* GL_MANGLE(glGetString) (GLenum name)
{

#define EXT_STRING "GL_ARB_multitexture EXT_texture_env_add" // is this valid?

	if (name == GL_EXTENSIONS)
	{
		if( !nano_extensions_string )
		{
			const GLubyte *extstr = wes_gl->glGetString( name );
			size_t len = strlen( extstr ) + strlen( EXT_STRING ) + 2;
			nano_extensions_string = malloc( len * sizeof( GLubyte ));

			snprintf((char*)nano_extensions_string, len,
				"%s %s", extstr, EXT_STRING);
			nano_extensions_string[len - 1] = 0;
		}

		return nano_extensions_string;
	}
	return wes_gl->glGetString(name);
}

void GL_MANGLE(glGetIntegerv) (GLenum pname, GLint *params)
	{
	if( pname ==  0x84E2 ) // GL_MAX_TEXTURE_UNITS_ARB
		pname = GL_MAX_TEXTURE_IMAGE_UNITS;
	wes_gl->glGetIntegerv(pname, params);
	}

void GL_MANGLE(glGetFloatv) (GLenum pname, GLfloat *params)
	{
	wes_gl->glGetFloatv(pname, params);
	}

void GL_MANGLE(glHint)(GLenum target, GLenum mode)
{
	if( target == GL_FOG_HINT )
		return;

	wes_vertbuffer_flush();
	wes_gl->glHint(target, mode);
}

void GL_MANGLE(glReadPixels) (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels)
	{
	if (format == GL_DEPTH_COMPONENT)
		{
		// OpenglEs 1.1 does not support reading depth buffer without an extension
		memset(pixels, 0xff,4);
		return;
		}
	wes_vertbuffer_flush();
	wes_gl->glReadPixels(x,y,width,height,format,type,pixels);
	}
/*
void GL_MANGLE(glNormal3fv)( const GLfloat *v )
	{
	glNormal3f( v[0], v[1], v[2] );
	}
*/
/*
void GL_MANGLE(glCullFace) (GLenum mode)
{
	wes_vertbuffer_flush();
	wes_gl->glCullFace(mode);
}*/
#define glEsImpl wes_gl
#define FlushOnStateChange() wes_vertbuffer_flush()
void GL_MANGLE(glPixelStorei) (GLenum pname, GLint param)
{
	FlushOnStateChange();
	glEsImpl->glPixelStorei(pname, param);
}


void GL_MANGLE(glClear) (GLbitfield mask)
	{
	FlushOnStateChange();
	glEsImpl->glClear(mask);
	}


GLboolean GL_MANGLE(glIsTexture)(GLuint texture)
	{
	//FlushOnStateChange();
	return glEsImpl->glIsTexture(texture);
	}
/*
void glDrawBuffer(GLenum mode)
	{
	}
*/
void GL_MANGLE(glViewport) (GLint x, GLint y, GLsizei width, GLsizei height)
	{
	FlushOnStateChange();
	glEsImpl->glViewport(x,y,width,height);
	}
/*
void glBindTexture (GLenum target, GLuint texture)
	{
	glEsImpl->glBindTexture(target, texture);
	}
*/
/*
void glTexParameterf (GLenum target, GLenum pname, GLfloat param)
	{
	if (pname == 0x1004) { // GL_TEXTURE_BORDER_COLOR
		return; // not supported by opengl es
	}
	if (    (pname == GL_TEXTURE_WRAP_S ||
			 pname == GL_TEXTURE_WRAP_T) &&
			 param == 0x2900)   { // GL_CLAMP
		param = 0x812F;
	}

	FlushOnStateChange();
	glEsImpl->glTexParameterf(target, pname,param);
	}

void glTexParameterfv(	GLenum target, GLenum pname, const GLfloat *params)
	{
	glTexParameterf(target, pname, params[0]);
	}
*/


/*
void glDeleteTextures( GLsizei n, const GLuint *textures )
	{
	FlushOnStateChange();
	glEsImpl->glDeleteTextures(n,textures);
	}



GLvoid glClear (GLbitfield mask)
{
    wes_vertbuffer_flush();
    wes_gl->glClear(mask);
}
*/
GLvoid GL_MANGLE(glDepthMask) (GLboolean flag)
{

    if (wrapglState2.depthmask == flag)
    {
        return;
    }

    wrapglState2.depthmask = flag;
    wes_vertbuffer_flush();
    wes_gl->glDepthMask(flag);
}
/*
GLvoid glViewport (GLint x, GLint y, GLsizei width, GLsizei height)
{
    wes_vertbuffer_flush();
    wes_gl->glViewport(x,y,width,height);
}
*/
GLvoid GL_MANGLE(glBindTexture) (GLenum target, GLuint texture)
{
	if (wrapglState2.boundtexture == texture)
	{
        return;
    }


    wrapglState2.boundtexture = texture;
    wes_vertbuffer_flush();
    wes_gl->glBindTexture(target, texture);
}

GLvoid GL_MANGLE(glDepthRange)(GLclampd zNear, GLclampd zFar)
{
    if ((wrapglState2.depth_range_near == zNear) && (wrapglState2.depth_range_far == zFar))
    {
        return;
    }
    else
    {
        wrapglState2.depth_range_near = zNear;
        wrapglState2.depth_range_far = zFar;
    }

    wes_vertbuffer_flush();
    wes_gl->glDepthRangef(zNear, zFar);
}

GLvoid GL_MANGLE(glDepthFunc) (GLenum func)
{

    if (wrapglState2.depth_func == func)
    {
        return;
    }
    else
    {
        wrapglState2.depth_func = func;
    }

    wes_vertbuffer_flush();
    wes_gl->glDepthFunc(func);
}

GLvoid GL_MANGLE(glCullFace) (GLenum mode)
{

    if (wrapglState2.cullface == mode)
    {
        return;
    }
    else
    {
        wrapglState2.cullface = mode;
    }

    wes_vertbuffer_flush();
    wes_gl->glCullFace(mode);
}


GLvoid GL_MANGLE(glBlendFunc) (GLenum sfactor, GLenum dfactor)
{

	if( sfactor == wrapglState2.sfactor && dfactor == wrapglState2.dfactor )
		return;

	wes_vertbuffer_flush();

    wrapglState2.sfactor = sfactor;
	wrapglState2.dfactor = dfactor;

    wes_gl->glBlendFunc(sfactor, dfactor);
}

GLvoid GL_MANGLE(glFinish) (void)
{
    wes_vertbuffer_flush();
    wes_gl->glFinish();
}

GLvoid
GL_MANGLE(glDrawBuffer)(GLenum mode)
{

}
/*
GLvoid
GL_MANGLE(glPointSize)( GLfloat size )
{
    //wes_vertbuffer_flush();
}

GLvoid
GL_MANGLE(glPolygonMode)( GLenum face, GLenum mode )
{

}
*/


void GL_MANGLE(glBindBufferARB)( GLuint target, GLuint index )
{
//	if( index && !vbo_bkp_id && !skipnanogl )
	//	FlushOnStateChange();
	GL_MANGLE(glDisableClientState)( GL_COLOR_ARRAY );
	GL_MANGLE(glColor4f)( 1,1,1,1 );

	//if( index && !vbo_bkp_id && !skipnanogl )
		//wes_vertbuffer_flush();
	// prevent breaking our arrays by flush calls
	skipnanogl = (!!index) || vboarray;
	glEsImpl->glBindBuffer( target, index );
	//printf("glBindBufferARB %x %d\n", target, index );
	if( vbo_bkp_id && !index )
	{
		arraysValid = GL_FALSE;
	}
	vbo_bkp_id = index;
}

void GL_MANGLE(glGenBuffersARB)( GLuint count, GLuint *indexes )
{
	glEsImpl->glGenBuffers( count, indexes );
}

void GL_MANGLE(glDeleteBuffersARB)( GLuint count, GLuint *indexes )
{
	glEsImpl->glDeleteBuffers( count, indexes );
}

void GL_MANGLE(glBufferDataARB)( GLuint target, GLuint size, void *buffer, GLuint type )
{
	glEsImpl->glBufferData( target, size, buffer, type );
}

void GL_MANGLE(glBufferSubDataARB)( GLuint target, GLsizei offset, GLsizei size, void *buffer )
{
	glEsImpl->glBufferSubData( target, offset, size, buffer );
}

void GL_MANGLE(glDebugMessageControlARB)( GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint* ids, GLboolean enabled )
{
	if( glEsImpl->glDebugMessageControlKHR )
		glEsImpl->glDebugMessageControlKHR( source, type, severity, count, ids, enabled );
}

void GL_MANGLE(glDebugMessageInsertARB)( GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const char* buf )
{
	if( glEsImpl->glDebugMessageInsertKHR )
		glEsImpl->glDebugMessageInsertKHR( source, type, id, severity, length, buf );
}

void GL_MANGLE(glDebugMessageCallbackARB)( GL_DEBUG_PROC_ARB callback, void* userParam )
{
	if( glEsImpl->glDebugMessageCallbackKHR )
		glEsImpl->glDebugMessageCallbackKHR( callback, userParam );
}

GLuint GL_MANGLE(glGetDebugMessageLogARB)( GLuint count, GLsizei bufsize, GLenum* sources, GLenum* types, GLuint* ids, GLuint* severities, GLsizei* lengths, char* messageLog )
{
	if( glEsImpl->glGetDebugMessageLogKHR )
		return glEsImpl->glGetDebugMessageLogKHR( count, bufsize, sources, types, ids, severities, lengths, messageLog );
	return 0;
}

