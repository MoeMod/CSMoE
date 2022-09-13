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
#include "sprite.h"

namespace xe {

    struct TextureAttr
    {
        unsigned int	flags;
        int		width;
        int		height;
        int     index;
    };

    int TexLru_LoadTextureExternal(const char *name, int flags, imgfilter_t *filter);
	int TexLru_LoadTextureInternal(const char *name, const char *model_name, const studiohdr_t *phdr, const mstudiotexture_t *ptexture, size_t size, int flags, imgfilter_t *filter);
    int TexLru_LoadTextureSPR(const char *name, const dsprite_t *psprite, const byte *buf, const dspriteframe_t *frame, int flags, imgfilter_t *filter);

    const TextureAttr &TexLru_GetAttr(const mstudiotexture_t *ptexture);
    void TexLru_Bind(const mstudiotexture_t *ptexture);
    void TexLru_Bind(const mspriteframe_t *pspriteframe);
    void TexLru_CreateAttr(const mstudiotexture_t *ptexture, int gl_texturenum);
    //void TexLru_CreateAttr(const mspriteframe_t *pspriteframe, int gl_texturenum);
    void TexLru_FreeTexture(const mstudiotexture_t *ptexture);

	void TexLru_Shrink();
	void TexLru_Clear();
}