/*
gl_triapi.cpp
Copyright (C) 2024 Moemod Hymei

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#include "common.h"
#include "client.h"
#include "sprite.h"
#include "gl_local.h"
#include "triangleapi.h"
#include "gl_triapi.h"
#include "gl_shader.h"

#include <vector>
#include <numeric>

#define XASH_SHADER_TRIAPI 1

/*
=================
TriApi implementation

=================
*/

#if XASH_SHADER_TRIAPI
namespace xe {
	struct TriApi_VertexData
	{
		vec3_t pos;
		vec2_t uv;
		vec4_t col;
	};

	struct TriApi_FixFunctionEmulator
	{
		int primitive_mode = TRI_TRIANGLES;
		std::vector<TriApi_VertexData> vertex_buffer;
		std::vector<unsigned short> index_buffer;

		struct {
			GLint tex;
			GLint muWVP;
			GLint uPointSize;
		} uniform;

		struct {
			GLint pos;
			GLint uv;
			GLint col;
		} attrib;

		GLuint vbo;
		GLuint ebo;

		GLhandleARB vertex_shader;
		GLhandleARB fragment_shader;
		GLhandleARB program;

		vec3_t cur_pos = { 0, 0, 0 };
		vec2_t cur_uv = { 0, 0 };
		vec4_t cur_col = { 1, 1, 1, 1 };

		float cur_point_size = 1.2f;

		void InsertVert()
		{
			vertex_buffer.emplace_back(TriApi_VertexData{ cur_pos, cur_uv, cur_col });
		}

		size_t genIotaIndices() {
			index_buffer.resize(vertex_buffer.size());
			//for (int i = 0; i < vertex_buffer.size() - 2; i+=3) {
			//	index_buffer.emplace_back(0 + i);
			//	index_buffer.emplace_back(1 + i);
			//	index_buffer.emplace_back(2 + i);
			//}
			std::iota(index_buffer.begin(), index_buffer.end(), 0);
			return index_buffer.size();
		}

		size_t genQuadsIndices()
		{
			index_buffer.clear();
			for (int i = 0; i < vertex_buffer.size() - 3; i+=4) {
				index_buffer.emplace_back(0 + i);
				index_buffer.emplace_back(1 + i);
				index_buffer.emplace_back(2 + i);

				index_buffer.emplace_back(0 + i);
				index_buffer.emplace_back(2 + i);
				index_buffer.emplace_back(3 + i);
			}
			return index_buffer.size();
		}

		size_t genTriangleStripIndices()
		{
			index_buffer.clear();
			for (int i = 2; i < vertex_buffer.size(); ++i) {
				if( i & 1 )
				{
					// draw triangle [n-1 n-2 n]
					index_buffer.emplace_back(i - 1);
					index_buffer.emplace_back(i - 2);
					index_buffer.emplace_back(i);
				}
				else
				{
					// draw triangle [n-2 n-1 n]
					index_buffer.emplace_back(i - 2);
					index_buffer.emplace_back(i - 1);
					index_buffer.emplace_back(i);
				}
			}
			return index_buffer.size();
		}

		size_t genTriangleFanIndices()
		{
			index_buffer.clear();
			for (int i = 2; i < vertex_buffer.size(); ++i) {
				// draw triangle [0 n-1 n]
				index_buffer.emplace_back(0);
				index_buffer.emplace_back(i - 1);
				index_buffer.emplace_back(i);
			}
			return index_buffer.size();
		}

		size_t genQuadsStripIndices()
		{
			index_buffer.clear();
			for (int i = 2; i < vertex_buffer.size() - 1; i+=2) {
				index_buffer.emplace_back(i - 2);
				index_buffer.emplace_back(i - 1);
				index_buffer.emplace_back(i);

				index_buffer.emplace_back(i - 2);
				index_buffer.emplace_back(i);
				index_buffer.emplace_back(i + 1);
			}
			return index_buffer.size();
		}

		size_t genPolygonIndices()
		{
			// TODO
			return genTriangleFanIndices();
		}

		size_t genIndices()
		{
			if(primitive_mode == TRI_TRIANGLES)
				return genIotaIndices();
			if(primitive_mode == TRI_QUADS)
				return genQuadsIndices();
			if(primitive_mode == TRI_TRIANGLE_STRIP)
				return genTriangleStripIndices();
			if(primitive_mode == TRI_TRIANGLE_FAN)
				return genTriangleFanIndices();
			if(primitive_mode == TRI_QUAD_STRIP)
				return genQuadsStripIndices();
			if(primitive_mode == TRI_POLYGON)
				return genPolygonIndices();
			if(primitive_mode == TRI_POINTS)
				return genIotaIndices();
			if(primitive_mode == TRI_LINES)
				return genIotaIndices();
			return 0;
		}
		
		void InitShader() 
		{
			const GLcharARB* vertex_shader_list[2] = {
					xe::GetShaderVersionString(),
					R"(
uniform mat4 muWVP;
uniform float uPointSize;
attribute vec3 pos;
attribute vec2 uv;
attribute vec4 col;

varying vec2 vvTexCoord;
varying vec4 vvColor;

void main(void)
{
    gl_Position = muWVP * vec4(pos.x, pos.y, pos.z, 1.0);
	gl_PointSize = uPointSize;
    vvTexCoord = uv;
	vvColor = col;
}
)"
			};
			this->vertex_shader = pglCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
			pglShaderSourceARB(this->vertex_shader, 2, vertex_shader_list, NULL);
			pglCompileShaderARB(this->vertex_shader);
			xe::CheckShader(this->vertex_shader, "vertex shader");

			const GLcharARB* fragment_shader_list[2] = {
					xe::GetShaderVersionString(),
					R"(
uniform sampler2D tex;
varying vec2 vvTexCoord;
varying vec4 vvColor;

void main (void)
{
    gl_FragColor = texture2D(tex, vvTexCoord) * vvColor;
}
)"
			};
			this->fragment_shader = pglCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
			pglShaderSourceARB(this->fragment_shader, 2, fragment_shader_list, NULL);
			pglCompileShaderARB(this->fragment_shader);
			xe::CheckShader(this->fragment_shader, "fragment shader");

			this->program = pglCreateProgramObjectARB();
			pglAttachObjectARB(this->program, this->vertex_shader);
			pglAttachObjectARB(this->program, this->fragment_shader);
			pglLinkProgramARB(this->program);
			xe::CheckProgram(this->program, "shader program");


			this->uniform.tex = pglGetUniformLocationARB(this->program, "tex");
			this->uniform.muWVP = pglGetUniformLocationARB(this->program, "muWVP");
			this->uniform.uPointSize = pglGetUniformLocationARB(this->program, "uPointSize");

			this->attrib.pos = pglGetAttribLocationARB(this->program, "pos");
			this->attrib.uv = pglGetAttribLocationARB(this->program, "uv");
			this->attrib.col = pglGetAttribLocationARB(this->program, "col");

			pglGenBuffersARB( 1, &this->vbo);
			pglGenBuffersARB( 1, &this->ebo);
		}

		void ShutdownShader()
		{
			pglDetachObjectARB(this->program, this->vertex_shader);
			pglDetachObjectARB(this->program, this->fragment_shader);
			pglDeleteObjectARB(this->vertex_shader);
			pglDeleteObjectARB(this->fragment_shader);
			pglDeleteObjectARB(this->program);

			pglDeleteBuffersARB( 1, &this->vbo);
			pglDeleteBuffersARB( 1, &this->ebo);
		}
	} TriFFE;
}
#endif

void TriAPI_Init()
{
#if XASH_SHADER_TRIAPI
	xe::TriFFE.InitShader();
#endif
}

void TriAPI_Shutdown()
{
#if XASH_SHADER_TRIAPI
	xe::TriFFE.ShutdownShader();
#endif
}

/*
=============
TriRenderMode

set rendermode
=============
*/
void GAME_EXPORT TriRenderMode( int mode )
{
	switch( mode )
	{
		case kRenderNormal:
		default:	pglDisable( GL_BLEND );
			//pglDisable( GL_ALPHA_TEST );
			//pglTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
			break;
		case kRenderTransColor:
		case kRenderTransAlpha:
		case kRenderTransTexture:
			// NOTE: TriAPI doesn't have 'solid' mode
			pglEnable( GL_BLEND );
			//pglDisable( GL_ALPHA_TEST );
			pglBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
			//pglTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
			break;
		case kRenderGlow:
		case kRenderTransAdd:
			pglEnable( GL_BLEND );
			//pglDisable( GL_ALPHA_TEST );
			pglBlendFunc( GL_SRC_ALPHA, GL_ONE );
			//pglTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
			break;
	}
}

/*
=============
TriBegin

begin triangle sequence
=============
*/
void GAME_EXPORT TriBegin( int mode )
{
#if XASH_SHADER_TRIAPI
	switch(mode) {
		case TRI_TRIANGLES:
		case TRI_TRIANGLE_STRIP:
		case TRI_QUADS:
		case TRI_TRIANGLE_FAN:
		case TRI_POLYGON:
		case TRI_QUAD_STRIP:
		case TRI_POINTS:
		case TRI_LINES:
			break;
		default:
			Sys_Break("TriBegin: unsupported primitive_mode %d\n", mode);
			return;
	}
	xe::TriFFE.primitive_mode = mode;
	xe::TriFFE.vertex_buffer.clear();
	xe::TriFFE.index_buffer.clear();
#else
	switch( mode )
	{
		case TRI_POINTS:
			mode = GL_POINTS;
			break;
		case TRI_TRIANGLES:
			mode = GL_TRIANGLES;
			break;
		case TRI_TRIANGLE_FAN:
			mode = GL_TRIANGLE_FAN;
			break;
		case TRI_QUADS:
			mode = GL_QUADS;
			break;
		case TRI_LINES:
			mode = GL_LINES;
			break;
		case TRI_TRIANGLE_STRIP:
			mode = GL_TRIANGLE_STRIP;
			break;
		case TRI_QUAD_STRIP:
			mode = GL_QUAD_STRIP;
			break;
		case TRI_POLYGON:
		default:	mode = GL_POLYGON;
			break;
	}

	pglBegin( mode );
#endif
}

/*
=============
TriEnd

draw triangle sequence
=============
*/
void GAME_EXPORT TriEnd( void )
{
#if XASH_SHADER_TRIAPI
	pglUseProgramObjectARB(xe::TriFFE.program);
	pglUniform1iARB(xe::TriFFE.uniform.tex, 0);
	pglUniformMatrix4fvARB(xe::TriFFE.uniform.muWVP, 1, GL_TRUE, (const float*)&RI.worldviewProjectionMatrix);
	pglUniform1fARB(xe::TriFFE.uniform.uPointSize, xe::TriFFE.cur_point_size);

	pglBindBufferARB(GL_ARRAY_BUFFER_ARB, xe::TriFFE.vbo);
	pglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, xe::TriFFE.ebo);

	pglEnableVertexAttribArrayARB(xe::TriFFE.attrib.pos);
	pglEnableVertexAttribArrayARB(xe::TriFFE.attrib.uv);
	pglEnableVertexAttribArrayARB(xe::TriFFE.attrib.col);
	pglVertexAttribPointerARB(xe::TriFFE.attrib.pos,   3, GL_FLOAT, GL_FALSE, sizeof(xe::TriApi_VertexData), (GLvoid*)offsetof(xe::TriApi_VertexData, pos));
	pglVertexAttribPointerARB(xe::TriFFE.attrib.uv,    2, GL_FLOAT, GL_FALSE, sizeof(xe::TriApi_VertexData), (GLvoid*)offsetof(xe::TriApi_VertexData, uv));
	pglVertexAttribPointerARB(xe::TriFFE.attrib.col, 4, GL_FLOAT, GL_FALSE,  sizeof(xe::TriApi_VertexData), (GLvoid*)offsetof(xe::TriApi_VertexData, col));

	// Upload vertex/index buffers
	xe::TriFFE.genIndices();
	pglBufferDataARB(GL_ARRAY_BUFFER_ARB, (GLsizeiptrARB)xe::TriFFE.vertex_buffer.size() * (int)sizeof(xe::TriApi_VertexData), (const GLvoid*)xe::TriFFE.vertex_buffer.data(), GL_STREAM_DRAW_ARB);
	pglBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, (GLsizeiptrARB)xe::TriFFE.index_buffer.size() * (int)sizeof(unsigned short), (const GLvoid*)xe::TriFFE.index_buffer.data(), GL_STREAM_DRAW_ARB);

	int mode = GL_TRIANGLES;
	if(xe::TriFFE.primitive_mode == TRI_POINTS)
	{
		mode = GL_POINTS;
	}
	if(xe::TriFFE.primitive_mode == TRI_LINES)
	{
		mode = GL_LINES;
	}
	pglDrawElements(mode, (GLsizei)xe::TriFFE.index_buffer.size(), GL_UNSIGNED_SHORT, (void*)(intptr_t)(0));

	pglDisableVertexAttribArrayARB(xe::TriFFE.attrib.pos);
	pglDisableVertexAttribArrayARB(xe::TriFFE.attrib.uv);
	pglDisableVertexAttribArrayARB(xe::TriFFE.attrib.col);
	// Restore modified GL state
	pglUseProgramObjectARB(0);
	pglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
	pglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);

	//pglDisable( GL_ALPHA_TEST );
#else
	pglEnd();
	//pglDisable( GL_ALPHA_TEST );
#endif
}

/*
=============
TriColor4f

=============
*/
void GAME_EXPORT TriColor4f( float r, float g, float b, float a )
{
#if XASH_SHADER_TRIAPI
	xe::TriFFE.cur_col = { r, g, b, a};
#else
	clgame.ds.triColor[0] = (byte)bound( 0, (r * 255.0f), 255 );
	clgame.ds.triColor[1] = (byte)bound( 0, (g * 255.0f), 255 );
	clgame.ds.triColor[2] = (byte)bound( 0, (b * 255.0f), 255 );
	clgame.ds.triColor[3] = (byte)bound( 0, (a * 255.0f), 255 );
	pglColor4ub( clgame.ds.triColor[0], clgame.ds.triColor[1], clgame.ds.triColor[2], clgame.ds.triColor[3] );
#endif
}

/*
=============
TriColor4ub

=============
*/
void GAME_EXPORT TriColor4ub( byte r, byte g, byte b, byte a )
{
#if XASH_SHADER_TRIAPI
	TriColor4f(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
#else
	clgame.ds.triColor[0] = r;
	clgame.ds.triColor[1] = g;
	clgame.ds.triColor[2] = b;
	clgame.ds.triColor[3] = a;
	pglColor4ub( r, g, b, a );
#endif
}

/*
=============
TriTexCoord2f

=============
*/
void GAME_EXPORT TriTexCoord2f( float u, float v )
{
#if XASH_SHADER_TRIAPI
	xe::TriFFE.cur_uv = { u, v };
#else
	pglTexCoord2f( u, v );
#endif
}

/*
=============
TriVertex3fv

=============
*/
void GAME_EXPORT TriVertex3fv( const vec3_t v )
{
#if XASH_SHADER_TRIAPI
	xe::TriFFE.cur_pos = v;
	xe::TriFFE.InsertVert();
#else
	pglVertex3fv( v.data() );
#endif
}

/*
=============
TriVertex3f

=============
*/
void GAME_EXPORT TriVertex3f( float x, float y, float z )
{
#if XASH_SHADER_TRIAPI
	TriVertex3fv({x,y,z});
#else
	pglVertex3f( x, y, z );
#endif
}

/*
=============
TriWorldToScreen

convert world coordinates (x,y,z) into screen (x, y)
=============
*/
int TriWorldToScreen( const vec3_t world, vec3_t_ref screen )
{
	int	retval;

	retval = R_WorldToScreen( world, screen );

	screen[0] =  0.5f * screen[0] * (float)cl.refdef.viewport[2];
	screen[1] = -0.5f * screen[1] * (float)cl.refdef.viewport[3];
	screen[0] += 0.5f * (float)cl.refdef.viewport[2];
	screen[1] += 0.5f * (float)cl.refdef.viewport[3];

	return retval;
}

/*
=============
TriSpriteTexture

bind current texture
=============
*/
int GAME_EXPORT TriSpriteTexture( model_t *pSpriteModel, int frame )
{
	int	gl_texturenum;
	msprite_t	*psprite;

	if(( gl_texturenum = R_GetSpriteTexture( pSpriteModel, frame )) == 0 )
		return 0;

	if( gl_texturenum <= 0 || gl_texturenum > R_TextureNum() )
	{
		MsgDev( D_ERROR, "TriSpriteTexture: bad index %i\n", gl_texturenum );
		gl_texturenum = tr.defaultTexture;
	}

	psprite = (msprite_t *)pSpriteModel->cache.data;
	if( psprite->texFormat == SPR_ALPHTEST )
	{
		//pglEnable( GL_ALPHA_TEST );
		//pglAlphaFunc( GL_GREATER, 0.0f );
	}

	GL_Bind( XASH_TEXTURE0, gl_texturenum );

	return 1;
}

/*
=============
TriFog

enables global fog on the level
=============
*/
void GAME_EXPORT TriFog( const vec3_t flFogColor, float flStart, float flEnd, int bOn )
{
	if( RI.fogEnabled || !gl_fog->integer ) return;
	//RI.fogCustom = true;

	if( !bOn )
	{
		pglDisable( GL_FOG );
		RI.fogCustom = false;
		return;
	}

	// copy fog params
	RI.fogColor[0] = flFogColor[0] / 255.0f;
	RI.fogColor[1] = flFogColor[1] / 255.0f;
	RI.fogColor[2] = flFogColor[2] / 255.0f;
	RI.fogStart = (flStart) ? flStart : 1.0f;
	RI.fogEnd = (flEnd || !RI.fogDensity) ? flEnd : 0.25f / RI.fogDensity;

	pglEnable(GL_FOG);
#if _WIN32 || TARGET_OS_OSX
	pglFogi(GL_FOG_MODE, GL_LINEAR);
	pglFogf(GL_FOG_START, RI.fogStart);
	pglFogf(GL_FOG_END, RI.fogEnd);
#else
	pglFogi(GL_FOG_MODE, GL_EXP);
	pglFogf(GL_FOG_DENSITY, RI.fogDensity);
#endif
	pglFogfv(GL_FOG_COLOR, RI.fogColor.data());
	pglHint(GL_FOG_HINT, GL_NICEST);
}

/*
=============
TriGetMatrix

very strange export
=============
*/
void GAME_EXPORT TriGetMatrix( const int pname, float *matrix )
{
	pglGetFloatv( pname, matrix );
}

/*
=============
TriForParams

=============
*/
void GAME_EXPORT TriFogParams( float flDensity, int iFogSkybox )
{
	RI.fogDensity = flDensity;
	RI.fogCustom = iFogSkybox;
}

/*
=============
TriCullFace

=============
*/
void GAME_EXPORT TriCullFace( TRICULLSTYLE mode )
{
	switch( mode )
	{
		case TRI_FRONT:
			clgame.ds.cullMode = GL_FRONT;
			break;
		default:
			clgame.ds.cullMode = GL_NONE;
			break;
	}
	GL_Cull( clgame.ds.cullMode );
}

/*
=============
TriBrightness

=============
*/
void GAME_EXPORT TriBrightness( float brightness )
{
#if XASH_SHADER_TRIAPI
	brightness = max( 0.0f, brightness );
	xe::TriFFE.cur_col *= brightness;
#else
	rgba_t	rgba;

	brightness = max( 0.0f, brightness );
	rgba[0] = clgame.ds.triColor[0] * brightness;
	rgba[1] = clgame.ds.triColor[1] * brightness;
	rgba[2] = clgame.ds.triColor[2] * brightness;
	rgba[3] = clgame.ds.triColor[3] * brightness;

	pglColor4ubv( rgba );
#endif
}

/*
=============
TriColor4fRendermode

Heavy legacy of Quake...
=============
*/
void GAME_EXPORT TriColor4fRendermode( float r, float g, float b, float a, int rendermode )
{
	if( rendermode == kRenderTransAlpha )
		TriColor4f( r, g, b, a );
	else TriColor4f( r * a, g * a, b * a, 1.0f );
}

void GAME_EXPORT TriPointSize(float x)
{
#if XASH_SHADER_TRIAPI
	xe::TriFFE.cur_point_size = x;
#else
	pglPointSize( x );
#endif
}
