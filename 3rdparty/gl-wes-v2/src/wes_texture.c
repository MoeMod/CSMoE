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


#include <stdlib.h>
#include <string.h>
#include "wes.h"
#include "wes_gl_defines.h"
#include "wes_begin.h"

GLboolean need_mipmap = GL_FALSE;

#if 0
// use this to remember internalformat for TexSubImage
typedef struct textureformat_s
{
	struct textureformat_s *next;
	GLenum internalformat;
	int texture;
} textureformat_t;

textureformat_t *textureformatlist;
#endif
GLvoid
GL_MANGLE(glTexParameteri) (GLenum target, GLenum pname, GLint param)
{
	if( pname == 0x8191) // GL_GENERATE_MIPMAP
	{
		need_mipmap = GL_TRUE;
		return;
	}
	if (pname == GL_TEXTURE_BORDER_COLOR)
	{
		return; // not supported by opengl es
	}
	if (    (pname == GL_TEXTURE_WRAP_S ||
			 pname == GL_TEXTURE_WRAP_T) &&
			 param == GL_CLAMP)   {
		param = 0x812F;
	}

	wes_vertbuffer_flush();

	wes_gl->glTexParameteri(target, pname, param);
}

GLvoid
GL_MANGLE(glTexParameterf) (GLenum target, GLenum pname, GLfloat param)
{
	if (pname == GL_TEXTURE_BORDER_COLOR)
		{
		return; // not supported by opengl es
		}
	if (    (pname == GL_TEXTURE_WRAP_S ||
			 pname == GL_TEXTURE_WRAP_T) &&
			 param == GL_CLAMP)
			 {
			 param = 0x812F;
			 }

	wes_vertbuffer_flush();

	wes_gl->glTexParameterf(target, pname, param);
}

GLvoid
GL_MANGLE(glTexParameterfv)(	GLenum target, GLenum pname, const GLfloat *params)
{
	GL_MANGLE(glTexParameterf)(target, pname, params[0]);
}

GLvoid
wes_convert_BGR2RGB(const GLubyte* inb, GLubyte* outb, GLint size)
{
	int i;
	for(i = 0; i < size; i += 3){
		outb[i + 2] = inb[i];
		outb[i + 1] = inb[i + 1];
		outb[i] = inb[i + 2];
	}
}

GLvoid
wes_convert_BGRA2RGBA(const GLubyte* inb, GLubyte* outb, GLint size)
{
	int i;
	for(i = 0; i < size; i += 4){
		outb[i + 2] = inb[i];
		outb[i + 1] = inb[i + 1];
		outb[i] = inb[i + 2];
		outb[i + 3] = inb[i + 3];
	}
}

GLvoid
wes_clear_alpha(const GLubyte* inb, GLubyte* outb, GLint size)
{
	int i;
	for(i = 0; i < size; i += 4){
		outb[i] = inb[i];
		outb[i + 1] = inb[i + 1];
		outb[i + 2] = inb[i + 2];
		outb[i + 3] = 255;
	}
}

GLvoid
wes_convert_RGBA2RGB(const GLubyte* inb, GLubyte* outb, GLint insize, GLint outsize)
{
	int i, j;
	for(i = 0, j = 0; i < insize && j < outsize; i += 4, j += 3){
		outb[j] = inb[i];
		outb[j + 1] = inb[i + 1];
		outb[j + 2] = inb[i + 2];
	}
}

GLvoid
wes_convert_I2LA(const GLubyte* inb, GLubyte* outb, GLint size)
{
	int i;
	for(i = 0; i < size; i += 1){
		outb[i*2 + 1] = outb[i*2] = inb[i];
	}
}
#if 0

void wes_addtextureformat(GLenum internalformat)
{
	textureformat_t *node;
	for( node = textureformatlist; node; node = node->next )
		if( node->texture == wrapglState2.boundtexture ) break;
	if( !node )
	{
		node = malloc( sizeof(textureformat_t));
		node->next = textureformatlist;
		textureformatlist = node;
		node->texture = wrapglState2.boundtexture;
	}
	node->internalformat = internalformat;
}

GLenum wes_gettextureformat()
{
	textureformat_t *node;

	for( node = textureformatlist; node; node = node->next )
		if( node->texture == wrapglState2.boundtexture )
			return node->internalformat;
	return 0;
}

void wes_deletetextureformat()
{
	textureformat_t *node;

	for( node = textureformatlist; node && node->texture != wrapglState2.boundtexture; node = node->next );
	if( node )
		node->internalformat = 0;
}
#endif

GLvoid
GL_MANGLE(glTexImage2D)(GLenum target, GLint level, GLenum internalFormat, GLsizei width, GLsizei height,
		   GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
	//wes_vertbuffer_flush(); //?

	GLvoid* data = (GLvoid*) pixels;

	/* conversion routines */
	if (format == GL_BGR){
		data = (GLvoid*) malloc(width * height * 3);
		wes_convert_BGR2RGB((GLubyte*) pixels, (GLubyte*) data, width * height * 3);
		format = GL_RGB;
	} else if (format == GL_BGRA){
		data = (GLvoid*) malloc(width * height * 4);
		wes_convert_BGRA2RGBA((GLubyte*) pixels, (GLubyte*) data, width * height * 4);
		format = GL_RGBA;
	} else if (format == GL_INTENSITY){
		data = (GLvoid*) malloc(width * height * 2);
		wes_convert_I2LA((GLubyte*) pixels, (GLubyte*) data, width * height * 2);
		format = GL_LUMINANCE_ALPHA;
	}
	if( pixels && format == GL_RGBA && (
		internalFormat == GL_RGB ||
		internalFormat == GL_RGB8 ||
		internalFormat == GL_RGB5 ||
		internalFormat == GL_LUMINANCE ||
		internalFormat == GL_LUMINANCE8 ||
		internalFormat == GL_LUMINANCE4 )) // strip alpha from texture
	{
		GLvoid *data2 = malloc(width * height * 4);
		//wes_addtextureformat(internalFormat);
		//wes_convert_RGBA2RGB((GLubyte*) data, (GLubyte*) data2, width * height * 4, width * height * 3);

		wes_clear_alpha((GLubyte*) pixels, (GLubyte*) data2, width * height * 4);
		if( data != pixels )
			free(data);
		data = data2;
		//format = GL_RGBA;
	}
	//else wes_deletetextureformat();

	wes_gl->glTexImage2D(target, level, format, width, height, 0, format, type, data);

	if (data != pixels)
		free(data);

	if( need_mipmap )
	{
		wes_gl->glGenerateMipmap(target);
		need_mipmap = GL_FALSE;
	}
}


GLvoid GL_MANGLE(glTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels)
{
	wes_gl->glTexSubImage2D(target,level,xoffset,yoffset,width,height,format,type,pixels);

	if( need_mipmap )
	{
		wes_gl->glGenerateMipmap(target);
		need_mipmap = GL_FALSE;
	}
}


void GL_MANGLE(glCopyTexImage2D)( GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border )
{
	wes_gl->glCopyTexImage2D(target, level, internalformat, x, y, width, height, border);
}


GLvoid GL_MANGLE(glTexImage1D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
    GL_MANGLE(glTexImage2D)(GL_TEXTURE_2D, level, internalformat,  width, 1, border, format, type, pixels);
}

GLvoid GL_MANGLE(glTexImage3D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
    GL_MANGLE(glTexImage2D)(GL_TEXTURE_2D, level, internalformat,  width, height, border, format, type, pixels);
}

GLvoid GL_MANGLE(glTexSubImage1D)( GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels )
{
    wes_gl->glTexSubImage2D(target,level,xoffset,0,width,1,format,type,pixels);
}

GLvoid GL_MANGLE(glTexSubImage3D)( GLenum target, GLint level,
                                         GLint xoffset, GLint yoffset,
                                         GLint zoffset, GLsizei width,
                                         GLsizei height, GLsizei depth,
                                         GLenum format,
                                         GLenum type, const GLvoid *pixels)
{
    wes_gl->glTexSubImage2D(target,level,xoffset,yoffset,width,height,format,type,pixels);
}

GLvoid
GL_MANGLE(glCompressedTexImage2D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border,  GLsizei imageSize, const void *data)
{
    wes_gl->glCompressedTexImage2D(target, level, internalformat, width, height, border, imageSize, data);
}

GLvoid
GL_MANGLE(glCompressedTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void* data)
{
    wes_gl->glCompressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format, imageSize, data);
}


GLvoid
wes_halveimage(GLint nw, GLint nh, GLint byteperpixel, char* data, char* newdata)
{
    int i, j;
    for(i = 0; i < nw; i++){
        for(j = 0; j < nh; j++){
            memcpy(&newdata[(i + j * nw) * byteperpixel], &data[(i + j * nw *2) * 2*byteperpixel], byteperpixel);
        }
    }
}

GLvoid
gluBuild2DMipmaps(GLenum target, GLint components, GLsizei width, GLsizei height,
                GLenum format, GLenum type, const GLvoid *pixels )
{
    int i = 1;
    GLuint byteperpixel = 0;
    char *data = NULL, *newdata = NULL;
    switch(type)
    {
        case GL_UNSIGNED_BYTE:
            byteperpixel = components;
            break;

        case GL_UNSIGNED_SHORT_4_4_4_4:
        case GL_UNSIGNED_SHORT_5_5_5_1:
        case GL_UNSIGNED_SHORT_5_6_5:
            byteperpixel = 2;
            break;
    }

    if (byteperpixel == 0)
    {
        return;
    }

    GL_MANGLE(glTexImage2D)(target, 0, format, width, height, 0, format, type, pixels);

    data = (char*) malloc(width * height * byteperpixel);
    memcpy(data, pixels, width * height * byteperpixel);
    while(width != 1 || height != 1){
        width  >>= 1;
        height >>= 1;
        if (width == 0) width = 1;
        if (height == 0) height = 1;
        newdata = (char*) malloc(width * height * byteperpixel);
        wes_halveimage(width, height, byteperpixel, data, newdata);
	GL_MANGLE(glTexImage2D)(target, i++, format, width, height, 0, format, type, newdata);
        free(data);
        data = newdata;
    }

    if (newdata != NULL)
        free(newdata);
}
