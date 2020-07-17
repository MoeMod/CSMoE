/*
ime_stub.cpp
Copyright (C) 2020 Moemod Haoyuan

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

extern "C" {
#include "common.h"
#include "input_ime.h"
#include "gl_vidnt.h"

#ifdef XASH_SDL
#include <SDL_keyboard.h>
#include <platform/sdl/events.h>
#endif

#if defined(SDL_VIDEO_DRIVER_COCOA)
#include "platform/macos/vid_macos.h"
#endif
}

const char* IME_GetCompositionString()
{
	return "";
}

size_t IME_GetCandidateListCount()
{
	return 0;
}

size_t IME_GetCandidateListPageStart()
{
	return 0;
}

size_t IME_GetCandidateListPageSize()
{
	return 0;
}

size_t IME_GetCandidateListSelectedItem()
{
	return 0;
}

int IME_CandidateListStartsAtOne()
{
	return 0;
}

const char* IME_GetCandidate(size_t i)
{
	return "";
}

void IME_SetInputScreenPos(int x, int y)
{
#ifdef XASH_SDL
	// dont know why macos should scale again
#ifdef __APPLE__
	y += 16;
	if(float dpi; VID_GetDPI(&dpi))
	{
		x /= dpi;
		y /= dpi;
	}
#endif

	SDL_Rect rect = { x, y, 0, 0 };
	SDL_SetTextInputRect(&rect);
#endif
}

void IME_GetInputScreenPos(int* x, int* y)
{
	;
}

int IME_ShouldShowBuiltInCandidateWindow()
{
	return false;
}

void IME_CreateContext()
{
	;
}

void IME_SetIMEEnabled(int enable, int force)
{
#if XASH_INPUT == INPUT_SDL
	SDLash_EnableTextInput(enable, force);
#elif XASH_INPUT == INPUT_ANDROID
	Android_EnableTextInput(enable, force);
#endif
}