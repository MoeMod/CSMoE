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


#include <EGL/egl.h>
#include "SDL.h"
#include <SDL/SDL_syswm.h>
#include "wes.h"

EGLint		VersionMajor;
EGLint		VersionMinor;

EGLDisplay  Display;
EGLContext  Context;
EGLConfig   Config;
EGLSurface  Surface;

EGLNativeDisplayType    Device;
EGLNativeWindowType     Handle;

const EGLint ConfigAttribs[] = {
	EGL_LEVEL,				0,
	EGL_DEPTH_SIZE,         16,
	EGL_STENCIL_SIZE,       8,
	EGL_SURFACE_TYPE,		EGL_WINDOW_BIT,
	EGL_RENDERABLE_TYPE,	EGL_OPENGL_ES2_BIT,
	EGL_NATIVE_RENDERABLE,	EGL_FALSE,
	EGL_NONE
};

const EGLint ContextAttribs[] = {
	EGL_CONTEXT_CLIENT_VERSION, 	2,
	EGL_NONE
};

const char* EGLErrorString(){
	EGLint nErr = eglGetError();
	switch(nErr){
		case EGL_SUCCESS: 				return "EGL_SUCCESS";
		case EGL_BAD_DISPLAY:			return "EGL_BAD_DISPLAY";
		case EGL_NOT_INITIALIZED:		return "EGL_NOT_INITIALIZED";
		case EGL_BAD_ACCESS:			return "EGL_BAD_ACCESS";
		case EGL_BAD_ALLOC:				return "EGL_BAD_ALLOC";
		case EGL_BAD_ATTRIBUTE:			return "EGL_BAD_ATTRIBUTE";
		case EGL_BAD_CONFIG:			return "EGL_BAD_CONFIG";
		case EGL_BAD_CONTEXT:			return "EGL_BAD_CONTEXT";
		case EGL_BAD_CURRENT_SURFACE:	return "EGL_BAD_CURRENT_SURFACE";
		case EGL_BAD_MATCH:				return "EGL_BAD_MATCH";
		case EGL_BAD_NATIVE_PIXMAP:		return "EGL_BAD_NATIVE_PIXMAP";
		case EGL_BAD_NATIVE_WINDOW:		return "EGL_BAD_NATIVE_WINDOW";
		case EGL_BAD_PARAMETER:			return "EGL_BAD_PARAMETER";
		case EGL_BAD_SURFACE:			return "EGL_BAD_SURFACE";
		default:						return "unknown";
	}
};

void
wes_context_open(unsigned int  w, unsigned int  h, unsigned int  vsync)
{
    Device = (EGLNativeDisplayType) EGL_DEFAULT_DISPLAY;
    EGLint nConfigs;

    SDL_Init(SDL_INIT_EVERYTHING);
	if (SDL_SetVideoMode(w, h, 32, SDL_HWSURFACE) == NULL){
		PRINT_ERROR("\nCould not set Video Mode");
	}

	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);
	SDL_GetWMInfo(&info);
	Handle = (EGLNativeWindowType) info.window;
	Device = GetDC(Handle);
    Display = eglGetDisplay((EGLNativeDisplayType) Device);

    if (Display == EGL_NO_DISPLAY){
        PRINT_ERROR("\nDisplay Get failed: %s", EGLErrorString());
    }

    if (!eglInitialize(Display, &VersionMajor, &VersionMinor)){
        PRINT_ERROR("\nDisplay Initialize failed: %s", EGLErrorString());
    }

    if (!eglChooseConfig(Display, ConfigAttribs, &Config, 1, &nConfigs)){
        PRINT_ERROR("\nConfiguration failed: %s", EGLErrorString());
    } else if (nConfigs != 1){
        PRINT_ERROR("\nConfiguration failed: nconfig %i, %s", nConfigs, EGLErrorString());
    }

    Surface = eglCreateWindowSurface(Display, Config, Handle, NULL);
    if (Surface == EGL_NO_SURFACE){
		PRINT_ERROR("\nSurface Creation failed: %s will attempt without window...", EGLErrorString());
        Surface = eglCreateWindowSurface(Display, Config, NULL, NULL);
        if (Surface == EGL_NO_SURFACE){
            PRINT_ERROR("\nSurface Creation failed: %s", EGLErrorString());
        }
    }
    eglBindAPI(EGL_OPENGL_ES_API);

    Context = eglCreateContext(Display, Config, EGL_NO_CONTEXT, ContextAttribs);
    if (Context == EGL_NO_CONTEXT){
        PRINT_ERROR("\nContext Creation failed: %s", EGLErrorString());
    }

    if (!eglMakeCurrent(Display, Surface, Surface, Context)){
        PRINT_ERROR("\nMake Current failed: %s", EGLErrorString());
    };
    eglSwapInterval(Display, vsync);
    freopen( "CON", "w", stdout );
    freopen( "CON", "w", stderr );
}

void
wes_context_update()
{
    eglSwapBuffers(Display, Surface);
}

void
wes_context_close()
{
    eglSwapBuffers(Display, Surface);
	eglMakeCurrent(Display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
 	eglDestroyContext(Display, Context);
	eglDestroySurface(Display, Surface);
   	eglTerminate(Display);
}
