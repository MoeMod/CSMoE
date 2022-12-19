/*
vid_sdl.c - SDL GL component
Copyright (C) 2010 Uncle Mike

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
#if XASH_VIDEO == VIDEO_SDL
#include "client.h"
#include "gl_local.h"
#include "mod_local.h"
#include "gl_vidnt.h"
#include <SDL.h>
#include <SDL_syswm.h>
#ifdef XASH_NANOGL
#include <GL/nanogl.h>
#endif
#ifdef XASH_GL4ES
#include "gl4esinit.h"
#include "gl4eshint.h"
#endif
#ifdef XASH_EGL
#include <EGL/egl.h>
#endif

#ifdef WIN32
// Enable NVIDIA High Performance Graphics while using Integrated Graphics.
__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;

// Enable AMD High Performance Graphics while using Integrated Graphics.
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
#endif

#ifndef XASH_GL_STATIC
#define GL_CALL( x ) #x, (void **)&p##x
#define GL_CALL_EX( x, y ) #x, (void **)&p##y
#else
#define GL_CALL( x ) #x, NULL
#define GL_CALL_EX( x, y ) #x, NULL
#endif

static dllfunc_t opengl_110funcs[] =
{
{ GL_CALL( glClearColor )	},
{ GL_CALL( glClear ) },
{ GL_CALL( glAlphaFunc ) },
{ GL_CALL( glBlendFunc ) },
{ GL_CALL( glCullFace ) },
{ GL_CALL( glDrawBuffer ) },
{ GL_CALL( glReadBuffer ) },
{ GL_CALL( glEnable ) },
{ GL_CALL( glDisable ) },
{ GL_CALL( glEnableClientState ) },
{ GL_CALL( glDisableClientState ) },
{ GL_CALL( glGetBooleanv ) },
{ GL_CALL( glGetDoublev ) },
{ GL_CALL( glGetFloatv ) },
{ GL_CALL( glGetIntegerv ) },
{ GL_CALL( glGetError ) },
{ GL_CALL( glGetString ) },
{ GL_CALL( glFinish ) },
{ GL_CALL( glFlush ) },
{ GL_CALL( glClearDepth ) },
{ GL_CALL( glDepthFunc ) },
{ GL_CALL( glDepthMask ) },
{ GL_CALL( glDepthRange ) },
{ GL_CALL( glFrontFace ) },
{ GL_CALL( glDrawElements ) },
{ GL_CALL( glDrawArrays ) },
{ GL_CALL( glColorMask ) },
{ GL_CALL( glIndexPointer ) },
{ GL_CALL( glVertexPointer ) },
{ GL_CALL( glNormalPointer ) },
{ GL_CALL( glColorPointer ) },
{ GL_CALL( glTexCoordPointer ) },
{ GL_CALL( glArrayElement ) },
{ GL_CALL( glColor3f ) },
{ GL_CALL( glColor3fv ) },
{ GL_CALL( glColor4f ) },
{ GL_CALL( glColor4fv ) },
{ GL_CALL( glColor3ub ) },
{ GL_CALL( glColor4ub ) },
{ GL_CALL( glColor4ubv ) },
{ GL_CALL( glTexCoord1f ) },
{ GL_CALL( glTexCoord2f ) },
{ GL_CALL( glTexCoord3f ) },
{ GL_CALL( glTexCoord4f ) },
{ GL_CALL( glTexGenf ) },
{ GL_CALL( glTexGenfv ) },
{ GL_CALL( glTexGeni ) },
{ GL_CALL( glVertex2f ) },
{ GL_CALL( glVertex3f ) },
{ GL_CALL( glVertex3fv ) },
{ GL_CALL( glNormal3f ) },
{ GL_CALL( glNormal3fv ) },
{ GL_CALL( glBegin ) },
{ GL_CALL( glEnd ) },
{ GL_CALL( glLineWidth ) },
{ GL_CALL( glPointSize ) },
{ GL_CALL( glMatrixMode ) },
{ GL_CALL( glOrtho ) },
{ GL_CALL( glRasterPos2f ) },
{ GL_CALL( glFrustum ) },
{ GL_CALL( glViewport ) },
{ GL_CALL( glPushMatrix ) },
{ GL_CALL( glPopMatrix ) },
{ GL_CALL( glPushAttrib ) },
{ GL_CALL( glPopAttrib ) },
{ GL_CALL( glLoadIdentity ) },
{ GL_CALL( glLoadMatrixd ) },
{ GL_CALL( glLoadMatrixf ) },
{ GL_CALL( glMultMatrixd ) },
{ GL_CALL( glMultMatrixf ) },
{ GL_CALL( glRotated ) },
{ GL_CALL( glRotatef ) },
{ GL_CALL( glScaled ) },
{ GL_CALL( glScalef ) },
{ GL_CALL( glTranslated ) },
{ GL_CALL( glTranslatef ) },
{ GL_CALL( glReadPixels ) },
{ GL_CALL( glDrawPixels ) },
{ GL_CALL( glStencilFunc ) },
{ GL_CALL( glStencilMask ) },
{ GL_CALL( glStencilOp ) },
{ GL_CALL( glClearStencil ) },
{ GL_CALL( glIsEnabled ) },
{ GL_CALL( glIsList ) },
{ GL_CALL( glIsTexture ) },
{ GL_CALL( glTexEnvf ) },
{ GL_CALL( glTexEnvfv ) },
{ GL_CALL( glTexEnvi ) },
{ GL_CALL( glTexParameterf ) },
{ GL_CALL( glTexParameterfv ) },
{ GL_CALL( glTexParameteri ) },
{ GL_CALL( glHint ) },
{ GL_CALL( glPixelStoref ) },
{ GL_CALL( glPixelStorei ) },
{ GL_CALL( glGenTextures ) },
{ GL_CALL( glDeleteTextures ) },
{ GL_CALL( glBindTexture ) },
{ GL_CALL( glTexImage1D ) },
{ GL_CALL( glTexImage2D ) },
{ GL_CALL( glTexSubImage1D ) },
{ GL_CALL( glTexSubImage2D ) },
{ GL_CALL( glCopyTexImage1D ) },
{ GL_CALL( glCopyTexImage2D ) },
{ GL_CALL( glCopyTexSubImage1D ) },
{ GL_CALL( glCopyTexSubImage2D ) },
{ GL_CALL( glScissor ) },
{ GL_CALL( glGetTexEnviv ) },
{ GL_CALL( glPolygonOffset ) },
{ GL_CALL( glPolygonMode ) },
{ GL_CALL( glPolygonStipple ) },
{ GL_CALL( glClipPlane ) },
{ GL_CALL( glGetClipPlane ) },
{ GL_CALL( glShadeModel ) },
{ GL_CALL( glFogfv ) },
{ GL_CALL( glFogf ) },
{ GL_CALL( glFogi ) },
{ NULL, NULL }
};

static dllfunc_t pointparametersfunc[] =
{
{ GL_CALL( glPointParameterfEXT ) },
{ GL_CALL( glPointParameterfvEXT ) },
{ NULL, NULL }
};

static dllfunc_t drawrangeelementsfuncs[] =
{
{ GL_CALL( glDrawRangeElements ) },
{ NULL, NULL }
};

static dllfunc_t drawrangeelementsextfuncs[] =
{
{ GL_CALL( glDrawRangeElementsEXT ) },
{ NULL, NULL }
};

static dllfunc_t debugoutputfuncs[] =
{
{ GL_CALL( glDebugMessageControlARB ) },
{ GL_CALL( glDebugMessageInsertARB ) },
{ GL_CALL( glDebugMessageCallbackARB ) },
{ GL_CALL( glGetDebugMessageLogARB ) },
{ NULL, NULL }
};

static dllfunc_t sgis_multitexturefuncs[] =
{
{ GL_CALL( glSelectTextureSGIS ) },
{ GL_CALL( glMTexCoord2fSGIS ) }, // ,
{ NULL, NULL }
};

static dllfunc_t multitexturefuncs[] =
{
{ GL_CALL_EX( glMultiTexCoord1fARB, glMultiTexCoord1f ) },
{ GL_CALL_EX( glMultiTexCoord2fARB, glMultiTexCoord2f ) },
{ GL_CALL_EX( glMultiTexCoord3fARB, glMultiTexCoord3f ) },
{ GL_CALL_EX( glMultiTexCoord4fARB, glMultiTexCoord4f ) },
{ GL_CALL( glActiveTextureARB ) },
{ GL_CALL_EX( glClientActiveTextureARB, glClientActiveTexture ) },
{ GL_CALL( glClientActiveTextureARB ) },
{ NULL, NULL }
};

static dllfunc_t compiledvertexarrayfuncs[] =
{
{ GL_CALL( glLockArraysEXT ) },
{ GL_CALL( glUnlockArraysEXT ) },
{ NULL, NULL }
};

static dllfunc_t texture3dextfuncs[] =
{
{ GL_CALL_EX( glTexImage3DEXT, glTexImage3D ) },
{ GL_CALL_EX( glTexSubImage3DEXT, glTexSubImage3D ) },
{ GL_CALL_EX( glCopyTexSubImage3DEXT, glCopyTexSubImage3D ) },
{ NULL, NULL }
};

static dllfunc_t atiseparatestencilfuncs[] =
{
{ GL_CALL_EX( glStencilOpSeparateATI, glStencilOpSeparate ) },
{ GL_CALL_EX( glStencilFuncSeparateATI, glStencilFuncSeparate ) },
{ NULL, NULL }
};

static dllfunc_t gl2separatestencilfuncs[] =
{
{ GL_CALL( glStencilOpSeparate ) },
{ GL_CALL( glStencilFuncSeparate ) },
{ NULL, NULL }
};

static dllfunc_t stenciltwosidefuncs[] =
{
{ GL_CALL( glActiveStencilFaceEXT ) },
{ NULL, NULL }
};

static dllfunc_t blendequationfuncs[] =
{
{ GL_CALL( glBlendEquationEXT ) },
{ NULL, NULL }
};

static dllfunc_t shaderobjectsfuncs[] =
{
{ GL_CALL( glDeleteObjectARB ) },
{ GL_CALL( glGetHandleARB ) },
{ GL_CALL( glDetachObjectARB ) },
{ GL_CALL( glCreateShaderObjectARB ) },
{ GL_CALL( glShaderSourceARB ) },
{ GL_CALL( glCompileShaderARB ) },
{ GL_CALL( glCreateProgramObjectARB ) },
{ GL_CALL( glAttachObjectARB ) },
{ GL_CALL( glLinkProgramARB ) },
{ GL_CALL( glUseProgramObjectARB ) },
{ GL_CALL( glValidateProgramARB ) },
{ GL_CALL( glUniform1fARB ) },
{ GL_CALL( glUniform2fARB ) },
{ GL_CALL( glUniform3fARB ) },
{ GL_CALL( glUniform4fARB ) },
{ GL_CALL( glUniform1iARB ) },
{ GL_CALL( glUniform2iARB ) },
{ GL_CALL( glUniform3iARB ) },
{ GL_CALL( glUniform4iARB ) },
{ GL_CALL( glUniform1fvARB ) },
{ GL_CALL( glUniform2fvARB ) },
{ GL_CALL( glUniform3fvARB ) },
{ GL_CALL( glUniform4fvARB ) },
{ GL_CALL( glUniform1ivARB ) },
{ GL_CALL( glUniform2ivARB ) },
{ GL_CALL( glUniform3ivARB ) },
{ GL_CALL( glUniform4ivARB ) },
{ GL_CALL( glUniformMatrix2fvARB ) },
{ GL_CALL( glUniformMatrix3fvARB ) },
{ GL_CALL( glUniformMatrix4fvARB ) },
{ GL_CALL( glGetObjectParameterfvARB ) },
{ GL_CALL( glGetObjectParameterivARB ) },
{ GL_CALL( glGetInfoLogARB ) },
{ GL_CALL( glGetAttachedObjectsARB ) },
{ GL_CALL( glGetUniformLocationARB ) },
{ GL_CALL( glGetActiveUniformARB ) },
{ GL_CALL( glGetUniformfvARB ) },
{ GL_CALL( glGetUniformivARB ) },
{ GL_CALL( glGetShaderSourceARB ) },
{ GL_CALL( glVertexAttribPointerARB ) },
{ GL_CALL( glEnableVertexAttribArrayARB ) },
{ GL_CALL( glDisableVertexAttribArrayARB ) },
{ GL_CALL( glBindAttribLocationARB ) },
{ GL_CALL( glGetActiveAttribARB ) },
{ GL_CALL( glGetAttribLocationARB ) },
{ NULL, NULL }
};

static dllfunc_t vertexshaderfuncs[] =
{
{ GL_CALL( glVertexAttribPointerARB ) },
{ GL_CALL( glEnableVertexAttribArrayARB ) },
{ GL_CALL( glDisableVertexAttribArrayARB ) },
{ GL_CALL( glBindAttribLocationARB ) },
{ GL_CALL( glGetActiveAttribARB ) },
{ GL_CALL( glGetAttribLocationARB ) },
{ NULL, NULL }
};

static dllfunc_t vbofuncs[] =
{
{ GL_CALL( glBindBufferARB ) },
{ GL_CALL( glDeleteBuffersARB ) },
{ GL_CALL( glGenBuffersARB ) },
{ GL_CALL( glIsBufferARB ) },
{ GL_CALL( glMapBufferARB ) },
{ GL_CALL( glUnmapBufferARB ) }, // ,
{ GL_CALL( glBufferDataARB ) },
{ GL_CALL( glBufferSubDataARB ) },
{ NULL, NULL}
};

static dllfunc_t occlusionfunc[] =
{
{ GL_CALL( glGenQueriesARB ) },
{ GL_CALL( glDeleteQueriesARB ) },
{ GL_CALL( glIsQueryARB ) },
{ GL_CALL( glBeginQueryARB ) },
{ GL_CALL( glEndQueryARB ) },
{ GL_CALL( glGetQueryivARB ) },
{ GL_CALL( glGetQueryObjectivARB ) }, //,
{ GL_CALL( glGetQueryObjectuivARB ) },
{ NULL, NULL }
};

static dllfunc_t texturecompressionfuncs[] =
{
{ GL_CALL( glCompressedTexImage3DARB ) },
{ GL_CALL( glCompressedTexImage2DARB ) },
{ GL_CALL( glCompressedTexImage1DARB ) },
{ GL_CALL( glCompressedTexSubImage3DARB ) },
{ GL_CALL( glCompressedTexSubImage2DARB ) },
{ GL_CALL( glCompressedTexSubImage1DARB ) },
{ GL_CALL_EX( glGetCompressedTexImageARB, glGetCompressedTexImage ) },

{ GL_CALL( glCompressedTexImage3D ) },
{ GL_CALL( glCompressedTexImage2D ) },
{ GL_CALL( glCompressedTexSubImage3D ) },
{ GL_CALL( glCompressedTexSubImage2D ) },

{ NULL, NULL }
};

/*
========================
DebugCallback
For ARB_debug_output
========================
*/
static void GAME_EXPORT APIENTRY GL_DebugOutput( GLuint source, GLuint type, GLuint id, GLuint severity, GLint length, const GLcharARB *message, GLvoid *userParam )
{
	switch( type )
	{
	case GL_DEBUG_TYPE_ERROR_ARB:
		if( host.developer < D_ERROR )	// "-dev 2"
			return;
		Con_Printf( "^1OpenGL Error:^7 %s\n", message );
		break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB:
		if( host.developer < D_WARN )		// "-dev 3"
			return;
		Con_Printf( "^3OpenGL Warning:^7 %s\n", message );
		break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB:
		if( host.developer < D_WARN )		// "-dev 3"
			return;
		Con_Printf( "^3OpenGL Warning:^7 %s\n", message );
		break;
	case GL_DEBUG_TYPE_PORTABILITY_ARB:
		if( host.developer < D_NOTE )	// "-dev 4"
			return;
		Con_Printf( "^3OpenGL Warning:^7 %s\n", message );
		break;
	case GL_DEBUG_TYPE_PERFORMANCE_ARB:
		if( host.developer < D_NOTE )	// "-dev 4"
			return;
		Con_Printf( "OpenGL Notify: %s\n", message );
		break;
	case GL_DEBUG_TYPE_OTHER_ARB:
	default:
		if( host.developer <= D_NOTE )		// "-dev 5"
			return;
		Con_Printf( "OpenGL: %s\n", message );
		break;
	}
}

/*
=================
GL_GetProcAddress
=================
*/
void EXPORT *GL_GetProcAddress( const char *name )
{
    // GetProcAddress for desktop OpenGL 1.x functions
#if defined( XASH_NANOGL ) || defined( XASH_WES )
	void *func = nullptr; // not supported
#elif defined( XASH_QINDIEGL )
	PROC wrap_wglGetProcAddress(LPCSTR s);
	void* func = wrap_wglGetProcAddress(name);
#elif defined( XASH_GL4ES )
	void* func = gl4es_GetProcAddress( name );
#else
	void *func = SDL_GL_GetProcAddress(name);
#endif

	if( !func )
	{
		MsgDev(D_ERROR, "Error: GL_GetProcAddress failed for %s\n", name);
	}
	return func;
}

/*
=================
GL_CheckExtension
=================
*/
void GL_CheckExtension( const char *name, const dllfunc_t *funcs, const char *cvarname, int r_ext )
{
	const dllfunc_t	*func;
	convar_t		*parm;

	MsgDev( D_NOTE, "GL_CheckExtension: %s ", name );

	if( cvarname )
	{
		// system config disable extensions
		parm = Cvar_Get( cvarname, "1", CVAR_GLCONFIG, va( "enable or disable %s", name ));

		if( parm->integer == 0 || ( gl_extensions->integer == 0 && r_ext != GL_OPENGL_110 ))
		{
			MsgDev( D_NOTE, "- disabled\n" );
			GL_SetExtension( r_ext, false );
			return; // nothing to process at
		}
		GL_SetExtension( r_ext, true );
	}

	if(( name[2] == '_' || name[3] == '_' ) && !Q_strstr( glConfig.extensions_string, name ))
	{
		GL_SetExtension( r_ext, false );	// update render info
		MsgDev( D_NOTE, "- ^1failed\n" );
		return;
	}

	// clear exports
#ifndef XASH_GL_STATIC // we don't use function ptrs if static
	for( func = funcs; func && func->name; func++ )
		*func->func = NULL;
#endif


	GL_SetExtension( r_ext, true ); // predict extension state
#ifndef XASH_GL_STATIC // we don't use function ptrs if static
	for( func = funcs; func && func->name != NULL; func++ )
	{
		// functions are cleared before all the extensions are evaluated
		if(!(*func->func = (void *)GL_GetProcAddress( func->name )))
			GL_SetExtension( r_ext, false ); // one or more functions are invalid, extension will be disabled
	}
#endif

	if( GL_Support( r_ext ))
		MsgDev( D_NOTE, "- ^2enabled\n" );
	else MsgDev( D_NOTE, "- ^1failed\n" );
}

/*
===============
GL_UpdateSwapInterval
===============
*/
void GL_UpdateSwapInterval( void )
{
	if( gl_swapInterval->modified )
	{
		gl_swapInterval->modified = false;
#if defined(XASH_EGL)
        eglSwapInterval(eglGetCurrentDisplay(), gl_swapInterval->integer);
#elif defined(XASH_QINDIEGL)
        BOOL wglSwapInterval(int interval);
		wglSwapInterval(gl_swapInterval->integer);
#else
		if( SDL_GL_SetSwapInterval( gl_swapInterval->integer ) )
			MsgDev( D_ERROR, "SDL_GL_SetSwapInterval: %s\n", SDL_GetError( ) );
#endif
	}
}

#define SAFE_NO 0
#define SAFE_NOACC 1
#define SAFE_NODEPTH 2
#define SAFE_NOATTRIB 3
#define SAFE_DONTCARE 4


/*
==================
GL_SetupAttributes
==================
*/
void GL_SetupAttributes()
{
#if defined XASH_QINDIEGL || defined XASH_EGL
	// stub
#else
	int samples;

#if !defined(_WIN32)
	SDL_SetHint( "SDL_VIDEO_X11_XRANDR", "1" );
	SDL_SetHint( "SDL_VIDEO_X11_XVIDMODE", "1" );

	// Hints for Sailfish OS
	// NOTE: landscape is just a hint to compositor, it will NOT rotate window
	SDL_SetHint( "SDL_QTWAYLAND_CONTENT_ORIENTATION", "landscape" );
#endif

	SDL_GL_ResetAttributes();


#ifdef XASH_GLES
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_EGL, 1 );

#ifdef XASH_NANOGL
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 1 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 1 );
#elif defined( XASH_WES ) || defined( XASH_REGAL ) || defined( XASH_GL4ES )
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 2 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 0 );
#endif

#elif !defined XASH_GL_STATIC
	if( Sys_CheckParm( "-gldebug" ) && host.developer >= 1 )
	{
		MsgDev( D_NOTE, "Creating an extended GL context for debug...\n" );
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG );
		glw_state.extended = true;
	}

#endif // XASH_GLES

	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

	if( glw_state.safe > SAFE_DONTCARE )
	{
		glw_state.safe = -1;
		return;
	}

	if( glw_state.safe > SAFE_NO )
		Msg("Trying safe opengl mode %d\n", glw_state.safe );

	if( glw_state.safe >= SAFE_NOACC )
		SDL_GL_SetAttribute( SDL_GL_ACCELERATED_VISUAL, 1 );

	Msg ("bpp %d\n", glw_state.desktopBitsPixel );

	if( glw_state.safe < SAFE_NODEPTH )
		SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 24 );
	else if( glw_state.safe < 5 )
		SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 8 );


	if( glw_state.safe < SAFE_NOATTRIB )
	{
		if( glw_state.desktopBitsPixel >= 24 )
		{
			if( glw_state.desktopBitsPixel == 32 )
				SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 8 );

			SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
			SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
			SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
		}
		else
		{
			SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 5 );
			SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 6 );
			SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 5 );
		}
	}

	if( glw_state.safe >= SAFE_DONTCARE )
		return;

	SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, gl_stencilbits->integer );

	switch( gl_msaa->integer )
	{
	case 2:
	case 4:
	case 8:
	case 16:
		samples = gl_msaa->integer;
		break;
	default:
		samples = 0; // don't use, because invalid parameter is passed
	}

	if( samples )
	{
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, samples);
	}
	else
	{
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
	}
#endif
}

#ifdef XASH_GLES
void GL_InitExtensionsGLES( void )
{
	// initalize until base opengl functions loaded
	GL_SetExtension( GL_DRAW_RANGEELEMENTS_EXT, true );
	GL_SetExtension( GL_ARB_MULTITEXTURE, true );
	pglGetIntegerv( GL_MAX_TEXTURE_UNITS_ARB, &glConfig.max_texture_units );
	glConfig.max_texture_coords = glConfig.max_texture_units = 4;

	GL_SetExtension( GL_ENV_COMBINE_EXT, true );
	GL_SetExtension( GL_DOT3_ARB_EXT, true );
	GL_SetExtension( GL_TEXTURE_3D_EXT, false );
	GL_SetExtension( GL_SGIS_MIPMAPS_EXT, true ); // gles specs
	GL_SetExtension( GL_ARB_VERTEX_BUFFER_OBJECT_EXT, true ); // gles specs

	// hardware cubemaps
	GL_CheckExtension( "GL_OES_texture_cube_map", NULL, "gl_texture_cubemap", GL_TEXTURECUBEMAP_EXT );

	if( GL_Support( GL_TEXTURECUBEMAP_EXT ))
		pglGetIntegerv( GL_MAX_CUBE_MAP_TEXTURE_SIZE_ARB, &glConfig.max_cubemap_size );

	GL_SetExtension( GL_ARB_SEAMLESS_CUBEMAP, false );

	GL_SetExtension( GL_EXT_POINTPARAMETERS, false );
	GL_CheckExtension( "GL_OES_texture_npot", NULL, "gl_texture_npot", GL_ARB_TEXTURE_NPOT_EXT );

	GL_SetExtension( GL_TEXTURE_COMPRESSION_EXT, false );
	GL_SetExtension( GL_CUSTOM_VERTEX_ARRAY_EXT, false );
	GL_SetExtension( GL_CLAMPTOEDGE_EXT, true ); // by gles1 specs
	GL_SetExtension( GL_ANISOTROPY_EXT, false );
	GL_SetExtension( GL_TEXTURE_LODBIAS, false );
	GL_SetExtension( GL_CLAMP_TEXBORDER_EXT, false );
	GL_SetExtension( GL_BLEND_MINMAX_EXT, false );
	GL_SetExtension( GL_BLEND_SUBTRACT_EXT, false );
	GL_SetExtension( GL_SEPARATESTENCIL_EXT, false );
	GL_SetExtension( GL_STENCILTWOSIDE_EXT, false );
	GL_SetExtension( GL_TEXTURE_ENV_ADD_EXT,false  );
	GL_SetExtension( GL_SHADER_OBJECTS_EXT, false );
	GL_SetExtension( GL_SHADER_GLSL100_EXT, false );
	GL_SetExtension( GL_VERTEX_SHADER_EXT,false );
	GL_SetExtension( GL_FRAGMENT_SHADER_EXT, false );
	GL_SetExtension( GL_SHADOW_EXT, false );
	GL_SetExtension( GL_ARB_DEPTH_FLOAT_EXT, false );
	GL_SetExtension( GL_OCCLUSION_QUERIES_EXT,false );
	GL_CheckExtension( "GL_OES_depth_texture", NULL, "gl_depthtexture", GL_DEPTH_TEXTURE );
    GL_CheckExtension( "GL_KHR_texture_compression_astc_ldr", NULL, "gl_astc_format", GL_ASTC_EXT );

	glConfig.texRectangle = glConfig.max_2d_rectangle_size = 0; // no rectangle

	Cvar_FullSet( "gl_allow_mirrors", "0", CVAR_READ_ONLY); // No support for GLES

}
#else
void GL_InitExtensionsBigGL()
{
	// initalize until base opengl functions loaded
	GL_CheckExtension( "glDrawRangeElements", drawrangeelementsfuncs, "gl_drawrangeelments", GL_DRAW_RANGEELEMENTS_EXT );

	if( !GL_Support( GL_DRAW_RANGEELEMENTS_EXT ))
		GL_CheckExtension( "GL_EXT_draw_range_elements", drawrangeelementsextfuncs, "gl_drawrangeelments", GL_DRAW_RANGEELEMENTS_EXT );

	// multitexture
	glConfig.max_texture_units = glConfig.max_texture_coords = glConfig.max_teximage_units = 1;
	GL_CheckExtension( "GL_ARB_multitexture", multitexturefuncs, "gl_arb_multitexture", GL_ARB_MULTITEXTURE );

	if( GL_Support( GL_ARB_MULTITEXTURE ))
	{
		pglGetIntegerv( GL_MAX_TEXTURE_UNITS_ARB, &glConfig.max_texture_units );
		GL_CheckExtension( "GL_ARB_texture_env_combine", NULL, "gl_texture_env_combine", GL_ENV_COMBINE_EXT );

		if( !GL_Support( GL_ENV_COMBINE_EXT ))
			GL_CheckExtension( "GL_EXT_texture_env_combine", NULL, "gl_texture_env_combine", GL_ENV_COMBINE_EXT );

		if( GL_Support( GL_ENV_COMBINE_EXT ))
			GL_CheckExtension( "GL_ARB_texture_env_dot3", NULL, "gl_texture_env_dot3", GL_DOT3_ARB_EXT );
	}
	else
	{
		GL_CheckExtension( "GL_SGIS_multitexture", sgis_multitexturefuncs, "gl_sgis_multitexture", GL_ARB_MULTITEXTURE );
		if( GL_Support( GL_ARB_MULTITEXTURE )) glConfig.max_texture_units = 2;
	}

	if( glConfig.max_texture_units == 1 )
		GL_SetExtension( GL_ARB_MULTITEXTURE, false );

	// 3d texture support
	GL_CheckExtension( "GL_EXT_texture3D", texture3dextfuncs, "gl_texture_3d", GL_TEXTURE_3D_EXT );

	if( GL_Support( GL_TEXTURE_3D_EXT ))
	{
		pglGetIntegerv( GL_MAX_3D_TEXTURE_SIZE, &glConfig.max_3d_texture_size );

		if( glConfig.max_3d_texture_size < 32 )
		{
			GL_SetExtension( GL_TEXTURE_3D_EXT, false );
			MsgDev( D_ERROR, "GL_EXT_texture3D reported bogus GL_MAX_3D_TEXTURE_SIZE, disabled\n" );
		}
	}

	GL_CheckExtension( "GL_SGIS_generate_mipmap", NULL, "gl_sgis_generate_mipmaps", GL_SGIS_MIPMAPS_EXT );

	// hardware cubemaps
	GL_CheckExtension( "GL_ARB_texture_cube_map", NULL, "gl_texture_cubemap", GL_TEXTURECUBEMAP_EXT );

	if( GL_Support( GL_TEXTURECUBEMAP_EXT ))
	{
		pglGetIntegerv( GL_MAX_CUBE_MAP_TEXTURE_SIZE_ARB, &glConfig.max_cubemap_size );

		// check for seamless cubemaps too
		GL_CheckExtension( "GL_ARB_seamless_cube_map", NULL, "gl_seamless_cubemap", GL_ARB_SEAMLESS_CUBEMAP );
	}

	// point particles extension
	GL_CheckExtension( "GL_EXT_point_parameters", pointparametersfunc, NULL, GL_EXT_POINTPARAMETERS );

	GL_CheckExtension( "GL_ARB_texture_non_power_of_two", NULL, "gl_texture_npot", GL_ARB_TEXTURE_NPOT_EXT );
	GL_CheckExtension( "GL_ARB_texture_compression", texturecompressionfuncs, "gl_dds_hardware_support", GL_TEXTURE_COMPRESSION_EXT );
	GL_CheckExtension( "GL_EXT_compiled_vertex_array", compiledvertexarrayfuncs, "gl_cva_support", GL_CUSTOM_VERTEX_ARRAY_EXT );

	if( !GL_Support( GL_CUSTOM_VERTEX_ARRAY_EXT ))
		GL_CheckExtension( "GL_SGI_compiled_vertex_array", compiledvertexarrayfuncs, "gl_cva_support", GL_CUSTOM_VERTEX_ARRAY_EXT );

	GL_CheckExtension( "GL_EXT_texture_edge_clamp", NULL, "gl_clamp_to_edge", GL_CLAMPTOEDGE_EXT );

	if( !GL_Support( GL_CLAMPTOEDGE_EXT ))
		GL_CheckExtension("GL_SGIS_texture_edge_clamp", NULL, "gl_clamp_to_edge", GL_CLAMPTOEDGE_EXT );

	glConfig.max_texture_anisotropy = 0.0f;
	GL_CheckExtension( "GL_EXT_texture_filter_anisotropic", NULL, "gl_ext_anisotropic_filter", GL_ANISOTROPY_EXT );

	if( GL_Support( GL_ANISOTROPY_EXT ))
		pglGetFloatv( GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &glConfig.max_texture_anisotropy );

	GL_CheckExtension( "GL_EXT_texture_lod_bias", NULL, "gl_ext_texture_lodbias", GL_TEXTURE_LODBIAS );
	if( GL_Support( GL_TEXTURE_LODBIAS ))
		pglGetFloatv( GL_MAX_TEXTURE_LOD_BIAS_EXT, &glConfig.max_texture_lodbias );

	GL_CheckExtension( "GL_ARB_texture_border_clamp", NULL, "gl_ext_texborder_clamp", GL_CLAMP_TEXBORDER_EXT );

	GL_CheckExtension( "GL_EXT_blend_minmax", blendequationfuncs, "gl_ext_customblend", GL_BLEND_MINMAX_EXT );
	GL_CheckExtension( "GL_EXT_blend_subtract", blendequationfuncs, "gl_ext_customblend", GL_BLEND_SUBTRACT_EXT );

	GL_CheckExtension( "glStencilOpSeparate", gl2separatestencilfuncs, "gl_separate_stencil", GL_SEPARATESTENCIL_EXT );

	if( !GL_Support( GL_SEPARATESTENCIL_EXT ))
		GL_CheckExtension("GL_ATI_separate_stencil", atiseparatestencilfuncs, "gl_separate_stencil", GL_SEPARATESTENCIL_EXT );

	GL_CheckExtension( "GL_EXT_stencil_two_side", stenciltwosidefuncs, "gl_stenciltwoside", GL_STENCILTWOSIDE_EXT );
	GL_CheckExtension( "GL_ARB_vertex_buffer_object", vbofuncs, "gl_vertex_buffer_object", GL_ARB_VERTEX_BUFFER_OBJECT_EXT );

#ifndef XASH_GL_STATIC
	// we don't care if it's an extension or not, they are identical functions, so keep it simple in the rendering code
	if( pglDrawRangeElementsEXT == NULL ) pglDrawRangeElementsEXT = pglDrawRangeElements;
#endif
	GL_CheckExtension( "GL_ARB_texture_env_add", NULL, "gl_texture_env_add", GL_TEXTURE_ENV_ADD_EXT );

	// vp and fp shaders
	GL_CheckExtension( "GL_ARB_shader_objects", shaderobjectsfuncs, "gl_shaderobjects", GL_SHADER_OBJECTS_EXT );
	GL_CheckExtension( "GL_ARB_shading_language_100", NULL, "gl_glslprogram", GL_SHADER_GLSL100_EXT );
	GL_CheckExtension( "GL_ARB_vertex_shader", vertexshaderfuncs, "gl_vertexshader", GL_VERTEX_SHADER_EXT );
	GL_CheckExtension( "GL_ARB_fragment_shader", NULL, "gl_pixelshader", GL_FRAGMENT_SHADER_EXT );

	GL_CheckExtension( "GL_ARB_depth_texture", NULL, "gl_depthtexture", GL_DEPTH_TEXTURE );
	GL_CheckExtension( "GL_ARB_shadow", NULL, "gl_arb_shadow", GL_SHADOW_EXT );

	GL_CheckExtension( "GL_ARB_texture_float", NULL, "gl_arb_texture_float", GL_ARB_TEXTURE_FLOAT_EXT );
	GL_CheckExtension( "GL_ARB_depth_buffer_float", NULL, "gl_arb_depth_float", GL_ARB_DEPTH_FLOAT_EXT );

	if( glw_state.extended )
		GL_CheckExtension( "GL_ARB_debug_output", debugoutputfuncs, "gl_debug_output", GL_DEBUG_OUTPUT );

	// occlusion queries
	GL_CheckExtension( "GL_ARB_occlusion_query", occlusionfunc, "gl_occlusion_queries", GL_OCCLUSION_QUERIES_EXT );

	// nobody implement ASTC with no bug on Win32
#ifdef _WIN32
	GL_SetExtension(GL_ASTC_EXT, false);
#else
    GL_CheckExtension( "GL_KHR_texture_compression_astc_ldr", NULL, "gl_astc_format", GL_ASTC_EXT );
#endif

	if( GL_Support( GL_SHADER_GLSL100_EXT ))
	{
		pglGetIntegerv( GL_MAX_TEXTURE_COORDS_ARB, &glConfig.max_texture_coords );
		pglGetIntegerv( GL_MAX_TEXTURE_IMAGE_UNITS_ARB, &glConfig.max_teximage_units );
	}
	else
	{
		// just get from multitexturing
		glConfig.max_texture_coords = glConfig.max_teximage_units = glConfig.max_texture_units;
	}

	// rectangle textures support
	if( Q_strstr( glConfig.extensions_string, "GL_NV_texture_rectangle" ))
	{
		glConfig.texRectangle = GL_TEXTURE_RECTANGLE_NV;
		pglGetIntegerv( GL_MAX_RECTANGLE_TEXTURE_SIZE_NV, &glConfig.max_2d_rectangle_size );
	}
	else if( Q_strstr( glConfig.extensions_string, "GL_EXT_texture_rectangle" ))
	{
		glConfig.texRectangle = GL_TEXTURE_RECTANGLE_EXT;
		pglGetIntegerv( GL_MAX_RECTANGLE_TEXTURE_SIZE_EXT, &glConfig.max_2d_rectangle_size );
	}
	else glConfig.texRectangle = glConfig.max_2d_rectangle_size = 0; // no rectangle

	Cvar_Set( "gl_anisotropy", va( "%f", bound( 0, gl_texture_anisotropy->value, glConfig.max_texture_anisotropy )));
#ifndef XASH_GL_STATIC
	// enable gldebug if allowed
	if( GL_Support( GL_DEBUG_OUTPUT ))
	{
		if( host.developer >= D_ERROR )
		{
			MsgDev( D_NOTE, "Installing GL_DebugOutput...\n");
			pglDebugMessageCallbackARB( GL_DebugOutput, NULL );
		}

		// force everything to happen in the main thread instead of in a separate driver thread
		if( host.developer >= D_WARN )
			pglEnable( GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB );

		// enable all the low priority messages
		if( host.developer >= D_NOTE )
			pglDebugMessageControlARB( GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_LOW_ARB, 0, NULL, true );
	}
#endif
}
#endif

void GL_InitExtensions( void )
{
	// initialize gl extensions
	GL_CheckExtension( "OpenGL 1.1.0", opengl_110funcs, NULL, GL_OPENGL_110 );

	// get our various GL strings
	glConfig.vendor_string = (const char*)pglGetString( GL_VENDOR );
	glConfig.renderer_string = (const char*)pglGetString( GL_RENDERER );
	glConfig.version_string = (const char*)pglGetString( GL_VERSION );
	glConfig.extensions_string = (const char*)pglGetString( GL_EXTENSIONS );
	MsgDev( D_INFO, "Video: %s\n", glConfig.renderer_string );

#ifdef XASH_GLES
	GL_InitExtensionsGLES();
#else
	GL_InitExtensionsBigGL();
#endif

	glConfig.max_2d_texture_size = 0;
	pglGetIntegerv( GL_MAX_TEXTURE_SIZE, &glConfig.max_2d_texture_size );
	if( glConfig.max_2d_texture_size <= 0 ) glConfig.max_2d_texture_size = 256;

	Cvar_Get( "gl_max_texture_size", "0", CVAR_INIT, "opengl texture max dims" );
	Cvar_Set( "gl_max_texture_size", va( "%i", glConfig.max_2d_texture_size ));

	pglGetIntegerv( GL_MAX_VERTEX_UNIFORM_COMPONENTS_ARB, &glConfig.max_vertex_uniforms );
	pglGetIntegerv( GL_MAX_VERTEX_ATTRIBS_ARB, &glConfig.max_vertex_attribs );


	Cvar_Set( "gl_anisotropy", va( "%f", bound( 0, gl_texture_anisotropy->value, glConfig.max_texture_anisotropy )));

	// software mipmap generator does wrong result with NPOT textures ...
	if( !GL_Support( GL_SGIS_MIPMAPS_EXT ))
		GL_SetExtension( GL_ARB_TEXTURE_NPOT_EXT, false );

	if( GL_Support( GL_TEXTURE_COMPRESSION_EXT ))
		Image_AddCmdFlags( IL_DDS_HARDWARE );

	// MCD has buffering issues
#ifdef _WIN32
	if( Q_strstr( glConfig.renderer_string, "gdi" ))
		Cvar_SetFloat( "gl_finish", 1 );
#endif

	glw_state.initialized = true;

	tr.framecount = tr.visframecount = 1;
}

#ifdef XASH_WES
extern "C" void wes_init();
#endif
#ifdef XASH_GL4ES
void GL4ES_GetMainFBSize( int *width, int *height )
{
	*width = glState.width;
	*height = glState.height;
}
void *GL4ES_GetProcAddress( const char *name )
{
	if( !Q_strcmp(name, "glShadeModel") )
		// combined gles/gles2/gl implementation exports this, but it is invalid
		return NULL;
	return (void *)eglGetProcAddress( name );
}
#endif

/*
=================
GL_CreateContext
=================
*/
qboolean GL_CreateContext( void )
{
	int colorBits[3];
#ifdef XASH_NANOGL
	nanoGL_Init();
#endif
#ifdef XASH_EGL
    // empty
    EGLDisplay dpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if(dpy == nullptr)
    {
        MsgDev(D_ERROR, "GL_CreateContext: ANGLE eglGetDisplay failed\n");
        return false;
    }
    EGLint version[2];
    if(!eglInitialize(dpy, &version[0], &version[1]))
    {
        MsgDev(D_ERROR, "GL_CreateContext: ANGLE eglInitialize failed\n");
        return false;
    }
    EGLint config_attr[] = {
            EGL_STENCIL_SIZE, gl_stencilbits->integer,
            EGL_DEPTH_SIZE, 8,
            EGL_RED_SIZE,   8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE,  8,
            EGL_ALPHA_SIZE, 8,
            EGL_NONE
    };
    EGLConfig config;
    int num_config;
    if(!eglChooseConfig(dpy, config_attr, &config, 1, &num_config))
    {
        MsgDev(D_ERROR, "GL_CreateContext: ANGLE eglChooseConfig failed\n");
        return false;
    }
    EGLint context_attr[] = {
            EGL_CONTEXT_CLIENT_VERSION, 2,
            EGL_NONE
    };
    auto ctx = eglCreateContext(dpy, config, EGL_NO_CONTEXT, context_attr);
    if(ctx == EGL_NO_CONTEXT)
    {
        MsgDev(D_ERROR, "GL_CreateContext: ANGLE eglCreateContext failed\n");
        return false;
    }

    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo(host.hWnd, &wmInfo);
#if ( TARGET_OS_IOS || TARGET_OS_IPHONE )
    auto hwnd = wmInfo.info.uikit.window;
#elif __ANDROID__
    auto hwnd = wmInfo.info.android.window;
#elif defined XASH_ANGLE && defined __APPLE__
    // macOS with Angle (metal backend)
    auto metalView = SDL_Metal_CreateView(host.hWnd);
    auto hwnd = SDL_Metal_GetLayer(metalView);
#elif defined _WIN32
    auto hwnd = wmInfo.info.win.window;
#endif
    EGLint surface_attr[] = {
            EGL_NONE
    };
    EGLSurface surface = eglCreateWindowSurface(dpy, config, hwnd, surface_attr);
    if(surface == EGL_NO_SURFACE)
    {
        MsgDev(D_ERROR, "GL_CreateContext: ANGLE eglCreateWindowSurface failed\n");
#if defined XASH_ANGLE && defined __APPLE__
        SDL_Metal_DestroyView(hwnd);
#endif
        eglDestroyContext(dpy, ctx);
        return false;
    }
    if(!eglMakeCurrent(dpy, surface, surface, ctx))
    {
        MsgDev(D_ERROR, "GL_CreateContext: ANGLE eglMakeCurrent failed\n");
#if defined XASH_ANGLE && defined __APPLE__
        SDL_Metal_DestroyView(hwnd);
#endif
        eglDestroySurface(dpy, surface);
        eglDestroyContext(dpy, ctx);
        return false;
    }
    glw_state.surface = surface;
    glw_state.context = ctx;
#elif defined XASH_QINDIEGL
	void QindieGL_Init(void);
	HGLRC wrap_wglCreateContext(HDC hdc);
	QindieGL_Init();

	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	SDL_GetWindowWMInfo(host.hWnd, &wmInfo);
	HWND hwnd = wmInfo.info.win.window;
	if ((glw_state.context = wrap_wglCreateContext(GetDC(hwnd))) == NULL)
	{
		MsgDev(D_ERROR, "GL_CreateContext: QindieGL wrap_wglCreateContext failed\n");
		return GL_DeleteContext();
	}

	pglGetIntegerv(GL_RED_BITS, &colorBits[0]);
	pglGetIntegerv(GL_GREEN_BITS, &colorBits[1]);
	pglGetIntegerv(GL_BLUE_BITS, &colorBits[2]);
	glConfig.color_bits = colorBits[0] + colorBits[1] + colorBits[2];
	pglGetIntegerv(GL_ALPHA_BITS, &glConfig.alpha_bits);
	pglGetIntegerv(GL_DEPTH_BITS, &glConfig.depth_bits);
	pglGetIntegerv(GL_STENCIL_BITS, &glConfig.stencil_bits);
	glState.stencilEnabled = glConfig.stencil_bits ? true : false;

	pglGetIntegerv(GL_SAMPLES_ARB, &glConfig.msaasamples);
	
#else
	if( ( glw_state.context = SDL_GL_CreateContext( host.hWnd ) ) == NULL)
	{
		MsgDev(D_ERROR, "GL_CreateContext: %s\n", SDL_GetError());
		return GL_DeleteContext();
	}
	
	SDL_GL_GetAttribute( SDL_GL_RED_SIZE, &colorBits[0] );
	SDL_GL_GetAttribute( SDL_GL_GREEN_SIZE, &colorBits[1] );
	SDL_GL_GetAttribute( SDL_GL_BLUE_SIZE, &colorBits[2] );
	glConfig.color_bits = colorBits[0] + colorBits[1] + colorBits[2];

	SDL_GL_GetAttribute( SDL_GL_ALPHA_SIZE, &glConfig.alpha_bits );
	SDL_GL_GetAttribute( SDL_GL_DEPTH_SIZE, &glConfig.depth_bits );
	SDL_GL_GetAttribute( SDL_GL_STENCIL_SIZE, &glConfig.stencil_bits );
	glState.stencilEnabled = glConfig.stencil_bits ? true : false;

	SDL_GL_GetAttribute( SDL_GL_MULTISAMPLESAMPLES, &glConfig.msaasamples );
	
#endif
#ifdef XASH_WES
	wes_init();
#endif
#ifdef XASH_GL4ES
	set_getprocaddress( GL4ES_GetProcAddress );
	set_getmainfbsize( GL4ES_GetMainFBSize );
	initialize_gl4es();

	// merge glBegin/glEnd in beams and console
	pglHint( GL_BEGINEND_HINT_GL4ES, 1 );
	// dxt unpacked to 16-bit looks ugly
	pglHint( GL_AVOID16BITS_HINT_GL4ES, 1 );
#endif

	return true;
}

/*
=================
GL_UpdateContext
=================
*/
qboolean GL_UpdateContext( void )
{
#ifdef XASH_EGL
    eglMakeCurrent(eglGetCurrentDisplay(), glw_state.surface, glw_state.surface, glw_state.context);
#elif defined XASH_QINDIEGL
	BOOL wrap_wglMakeCurrent(HDC hdc, HGLRC hglrc);
	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	SDL_GetWindowWMInfo(host.hWnd, &wmInfo);
	HWND hwnd = wmInfo.info.win.window;
	if(!wrap_wglMakeCurrent(GetDC(hwnd), glw_state.context))
	{
		MsgDev(D_ERROR, "GL_UpdateContext: QindieGL wrap_wglMakeCurrent failed");
		return GL_DeleteContext();
	}
#else
	if(!( SDL_GL_MakeCurrent( host.hWnd, glw_state.context ) ) )
	{
		MsgDev(D_ERROR, "GL_UpdateContext: %s", SDL_GetError());
		return GL_DeleteContext();
	}
#endif

	return true;
}

#ifndef EGL_LIB
#define EGL_LIB NULL
#endif

/*
=================
GL_DeleteContext
=================
*/
qboolean GL_DeleteContext( void )
{
#ifdef XASH_EGL
    eglDestroySurface(eglGetCurrentDisplay(), glw_state.surface);
    eglDestroyContext(eglGetCurrentDisplay(), glw_state.context);
#elif defined XASH_QINDIEGL
	HGLRC wrap_wglGetCurrentContext();
	BOOL wrap_wglDeleteContext(HGLRC hglrc);
	wrap_wglDeleteContext(wrap_wglGetCurrentContext());
	void QindieGL_Destroy(void);
	QindieGL_Destroy();
#elif defined XASH_GL4ES
	close_gl4es();
#else
	if (glw_state.context)
	{
		SDL_GL_DeleteContext(glw_state.context);
		glw_state.context = NULL;
	}
#endif
	
	return false;
}



/*
==================
R_Init_OpenGL
==================
*/
qboolean R_Init_OpenGL( void )
{
	SDL_DisplayMode displayMode;
	string safe;

	SDL_GetCurrentDisplayMode(0, &displayMode);
	glw_state.desktopBitsPixel = SDL_BITSPERPIXEL(displayMode.format);
	glw_state.desktopWidth = displayMode.w;
	glw_state.desktopHeight = displayMode.h;

	if( !glw_state.safe && Sys_GetParmFromCmdLine( "-safegl", safe ) )
	{
		glw_state.safe = Q_atoi( safe );
		if( glw_state.safe < SAFE_NOACC || glw_state.safe > SAFE_DONTCARE  )
			glw_state.safe = SAFE_DONTCARE;
	}

	if( glw_state.safe < SAFE_NO || glw_state.safe > SAFE_DONTCARE  )
		return false;


#if defined XASH_QINDIEGL || defined XASH_EGL
	
#else
	GL_SetupAttributes();
	
	if( SDL_GL_LoadLibrary( EGL_LIB ) )
	{
		MsgDev(D_ERROR, "Couldn't initialize OpenGL: %s\n", SDL_GetError());
		return false;
	}
#endif

	return VID_SetMode();
}


/*
==================
R_Free_OpenGL
==================
*/
void R_Free_OpenGL( void )
{
	GL_DeleteContext ();

	VID_DestroyWindow ();

#if defined XASH_QINDIEGL || defined XASH_EGL

#else
	SDL_GL_UnloadLibrary ();
#endif

	// now all extensions are disabled
	Q_memset( glConfig.extension, 0, sizeof( glConfig.extension[0] ) * GL_EXTCOUNT );
	glw_state.initialized = false;
}

#endif // XASH_VIDEO
