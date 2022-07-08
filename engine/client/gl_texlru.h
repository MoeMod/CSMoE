/*
texlru.h
Copyright (C) 2022 Moemod Hymei

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#pragma once

#include "common.h"
#include "studio.h"

namespace xe {
	int TexLru_LoadTextureExternal(const char *name, mstudiotexture_t* ptexture, int flags, imgfilter_t *filter);
	int TexLru_LoadTextureInternal(const char *name, const char *model_name, mstudiotexture_t *ptexture, size_t size, int flags, imgfilter_t *filter);
	void TexLru_Upload( mstudiotexture_t *ptexture );
	void TexLru_Shrink();
	void TexLru_Clear();
}