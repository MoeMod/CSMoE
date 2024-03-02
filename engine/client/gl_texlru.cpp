/*
gl_texlru.cpp
Copyright (C) 2023 Moemod Hymei

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
#include "gl_local.h"
#include "studio.h"

#include "gl_texlru.h"
#include <variant>
#include <unordered_map>
#include <unordered_set>

#include "util/lru_cache.h"
#include "imagelib.h"

namespace xe {
    struct texlru_src_external_s {
        std::string path;
    };
    struct texlru_src_internal_s {
        std::string texture_name;
        std::string model_name;
        const studiohdr_t *phdr;
        const mstudiotexture_t *texture;
        size_t size;
    };
    struct texlru_src_spr_s {
        std::string path;
        const dsprite_t *psprite;
        const dspriteframe_t *texture;
    };
    struct texlru_src_dds_s {
        std::string path;
	    const dsprite_t *psprite;
	    const dds_t *pin;
		int size;
    };
    struct texlru_src_skybox_s {
        std::string skyname;
    };
    struct texlru_extdata_s {
        int texnum;
        imgfilter_t *filter;
        std::variant<texlru_src_external_s, texlru_src_internal_s, texlru_src_spr_s, texlru_src_dds_s, texlru_src_skybox_s> src;
    };

    std::unordered_map<const mstudiotexture_t *, TextureAttr> g_textureAttrMap;
    std::unordered_map<const mstudiotexture_t *, int> g_textureReplaceMap;
    std::unordered_set<int> g_texureUploaded;

    void TexLru_UpdateAttr(const mstudiotexture_t *ptexture)
    {
        auto& attr = g_textureAttrMap.at(ptexture);
        if ( ptexture->name[0] == '$' )
        {
            attr.flags |= STUDIO_NF_FULLBRIGHT;

            if ( ptexture->name[1] == '0' && ptexture->name[2] == 'b' )
            {
                attr.flags |= STUDIO_NF_ADDITIVE; 
            }

            if (ptexture->name[1] == '0' && ptexture->name[2] == 'a')
            {
                attr.flags |= STUDIO_NF_ADDITIVE;
            }

        }


        gltexture_t *gltex = R_GetTexture(attr.index);
        if (gltex)
        {
            attr.width = gltex->width;
            attr.height = gltex->height;
        }
    }

    void TexLru_CreateAttr(const mstudiotexture_t *ptexture, int gl_texturenum)
    {
        TextureAttr attr = { ptexture->flags, ptexture->width, ptexture->height, gl_texturenum };

        if ( ptexture->name[0] == '$' )
        {
            attr.flags |= STUDIO_NF_FULLBRIGHT;

            if (ptexture->name[1] == '0' && ptexture->name[2] == 'b')
            {
                attr.flags |= STUDIO_NF_ADDITIVE;
            }

            if (ptexture->name[1] == '0' && ptexture->name[2] == 'a')
            {
                attr.flags |= STUDIO_NF_ADDITIVE;
            }
        }

        gltexture_t *gltex = R_GetTexture(gl_texturenum);
        if (gltex)
        {
            attr.index = gl_texturenum;
            attr.width = gltex->width;
            attr.height = gltex->height;
        }
#if 1
        auto [iter, succ] = g_textureAttrMap.emplace(ptexture, attr);
        if (!succ)
            (*iter).second = attr;
#else
        auto [iter, succ] = g_textureAttrMap.emplace(ptexture, TextureAttr{  ptexture->flags, ptexture->width, ptexture->height, gl_texturenum });
        if (!succ)
         (*iter).second = TextureAttr{ ptexture->flags, ptexture->width, ptexture->height, gl_texturenum };
#endif
        return TexLru_UpdateAttr(ptexture);
    }

    bool TexLru_LoadTexture(const char *name, int flags, imgfilter_t *filter, gltexture_t *&out_tex, int& texnum)
    {
        if( !name || !name[0] || !glw_state.initialized )
            return false;

        // see if already loaded
        int i = GL_FindTexture(name);
        if(i > 0)
        {
            // prolonge registration
            gltexture_t	*tex = R_GetTexture(i);
            tex->cacheframe = world.load_sequence;
            if (tex->name[0] != '*')
                tex->refCount++;

            out_tex = tex;
            texnum = i;
            return false;
        }

        uint picFlags = 0;

        if( flags & TF_NOFLIP_TGA )
            picFlags |= IL_DONTFLIP_TGA;

        if( flags & TF_KEEP_8BIT )
            picFlags |= IL_KEEP_8BIT;

        // TOFIX: save in lrudata
        // set some image flags
        Image_SetForceFlags( picFlags );

        // HACKHACK: get rid of black vertical line on a 'BlackMesa map'
        if( !Q_strcmp( name, "#lab1_map1.mip" ) || !Q_strcmp( name, "#lab1_map2.mip" ))
            flags |= TF_NEAREST;

        i = GL_AllocateTextureSlot(name);

        auto tex = R_GetTexture(i);
        Q_strncpy( tex->name, name, sizeof( tex->name ));
        tex->flags = (texFlags_t)flags;

        // init reference exclude brush model's texture
        if (tex->name[0] != '*')
            tex->refCount = 1;

        if( flags & TF_SKYSIDE )
            tex->texnum = i;//tr.skyboxbasenum++;
        else tex->texnum = i; // texnum is used for fast acess into r_textures array too

        // NOTE: always return texnum as index in array or engine will stop work !!!
        out_tex = tex;
        texnum = i;
        return true;
    }

    int TexLru_LoadTextureExternal(const char *name, int flags, imgfilter_t *filter)
    {
        gltexture_t* tex;
        int texnum;
        if(!TexLru_LoadTexture(name, flags, filter, tex, texnum))
            return texnum;

        // add to lru
        tex->texlru_extdata = std::make_unique<texlru_extdata_s>(texlru_extdata_s{ texnum, filter, texlru_src_external_s{ name } });

        return texnum;
    }

    int TexLru_LoadTextureInternal(const char *name, const char *model_name, const studiohdr_t *phdr, const mstudiotexture_t *ptexture, size_t size, int flags, imgfilter_t *filter)
    {
        gltexture_t* tex;
        int texnum;
        if(!TexLru_LoadTexture(name, flags, filter, tex, texnum))
            return texnum;

        // add to lru
        tex->texlru_extdata = std::make_unique<texlru_extdata_s>(texlru_extdata_s{ texnum, filter, texlru_src_internal_s{ name, model_name, phdr, ptexture, size } });

        return texnum;
    }

    int TexLru_LoadTextureSPR(const char *name, const dsprite_t *psprite, const byte *buf, const dspriteframe_t *frame, int flags, imgfilter_t *filter)
    {
        gltexture_t* tex;
        int texnum;
        if(!TexLru_LoadTexture(name, flags, filter, tex, texnum))
            return texnum;

        // add to lru
        tex->texlru_extdata = std::make_unique<texlru_extdata_s>(texlru_extdata_s{ texnum, filter, texlru_src_spr_s{ name, psprite, frame } });

        return texnum;
    }

    int TexLru_LoadTextureDDS(const char *name, const dsprite_t *psprite, const dds_t *pin, int filesize, int flags, imgfilter_t *filter)
    {
        gltexture_t* tex;
        int texnum;
        if(!TexLru_LoadTexture(name, flags, filter, tex, texnum))
            return texnum;

        // add to lru
        tex->texlru_extdata = std::make_unique<texlru_extdata_s>(texlru_extdata_s{ texnum, filter, texlru_src_dds_s{ name, psprite, pin, filesize } });

        return texnum;
    }

    int TexLru_LoadTextureSkybox(const char *name, int flags, imgfilter_t *filter)
    {
        uint		picFlags = 0;

        if( !name || !name[0] || !glw_state.initialized )
            return 0;

        string key;
        sprintf(key, "#CUBEMAP/%s", name);

        // see if already loaded
        int i = GL_FindTexture(key);
        if(i > 0)
        {
            // prolonge registration
            gltexture_t	*tex = R_GetTexture(i);
            tex->cacheframe = world.load_sequence;
            if (tex->name[0] != '*')
                tex->refCount++;

            return i;
        }

        if( flags & TF_NOFLIP_TGA )
            picFlags |= IL_DONTFLIP_TGA;

        if( flags & TF_KEEP_8BIT )
            picFlags |= IL_KEEP_8BIT;

        // set some image flags
        Image_SetForceFlags( picFlags );

        i = GL_AllocateTextureSlot(key);

        auto tex = R_GetTexture(i);
        Q_strncpy( tex->name, key, sizeof( tex->name ));
        tex->flags = (texFlags_t)flags;

        // init reference exclude brush model's texture
        if (tex->name[0] != '*')
            tex->refCount = 1;

        tex->texnum = i;

        // set sky data
        tex->target = GL_TEXTURE_CUBE_MAP_ARB;

        // add to lru
        tex->texlru_extdata = std::make_unique<texlru_extdata_t>(texlru_extdata_t{ i, filter, texlru_src_skybox_s{ name } });

        // NOTE: always return texnum as index in array or engine will stop work !!!
        return i;
    }

    int TexLru_ReleaseTexture(gltexture_t *image)
    {
        // release source
        if( image->flags & (TF_KEEP_RGBDATA|TF_KEEP_8BIT) && image->original )
        {
            image->original = nullptr;
        }

        //pglDeleteTextures( 1, &image->texnum );

        byte data2D[4] = {  };
        data2D[0] = data2D[1] = data2D[2] = 80;
        data2D[3] = 255;

        pglBindTexture( image->target, image->texnum );
        pglTexImage2D( image->target, 0, image->format, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data2D);
        pglBindTexture( image->target, 0 );
        MsgDev(D_INFO, "TexLru : swap out texture %s\n", image->name);

        return 0;
    }

    int TexLru_ReleaseSkybox(gltexture_t *image)
    {
        // release source
        if( image->flags & (TF_KEEP_RGBDATA|TF_KEEP_8BIT) && image->original )
        {
            image->original = nullptr;
        }

        //pglDeleteTextures( 1, &image->texnum );

        byte data2D[4] = {  };
        data2D[0] = data2D[1] = data2D[2] = 80;
        data2D[3] = 255;

        Assert(image->target == GL_TEXTURE_CUBE_MAP_ARB);

        pglActiveTextureARB(GL_TEXTURE1_ARB);
        pglBindTexture(GL_TEXTURE_CUBE_MAP_ARB, image->texnum);
        for (int j = 0; j < 6; ++j)
        {
            pglTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB + j, 0, image->format, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data2D);
        }
        pglBindTexture(GL_TEXTURE_CUBE_MAP_ARB, 0 );
        MsgDev(D_INFO, "TexLru : swap out texture %s\n", image->name);

        return 0;
    }

    void SPR_InstallPal(const dsprite_t *psprite)
    {
        auto buffer = (const byte *)psprite + sizeof(dsprite_t) + sizeof(short);
        // install palette
        switch (psprite->texFormat) {
            case SPR_ADDITIVE:
                FS_LoadImage("#normal.pal", buffer, 768);
                break;
            case SPR_INDEXALPHA:
                FS_LoadImage("#decal.pal", buffer, 768);
                break;
            case SPR_ALPHTEST:
                FS_LoadImage("#transparent.pal", buffer, 768);
                break;
            case SPR_NORMAL:
            default:
                FS_LoadImage("#normal.pal", buffer, 768);
                break;
        }
    }

    void SPR_InstallPalAsync(const dsprite_t *psprite, const std::function<void(uint* pal, int)>& callback)
    {
        auto LoadedCallback = [psprite, callback](image_ref pic, uint* pal, int rendermode) {
            if( !pic )
                return;
            if(!psprite)
                return;
            callback(pal, rendermode);
        };

        auto buffer = (const byte*)psprite + sizeof(dsprite_t) + sizeof(short);
        // install palette
        switch (psprite->texFormat) {
            case SPR_ADDITIVE:
                FS_LoadImageAsync("#normal.pal", LoadedCallback, buffer, 768);
                break;
            case SPR_INDEXALPHA:
                FS_LoadImageAsync("#decal.pal", LoadedCallback, buffer, 768);
                break;
            case SPR_ALPHTEST:
                FS_LoadImageAsync("#transparent.pal", LoadedCallback, buffer, 768);
                break;
            case SPR_NORMAL:
            default:
                FS_LoadImageAsync("#normal.pal", LoadedCallback, buffer, 768);
                break;
        }
    }

    struct TexLru_Uploader{
        gltexture_t	*tex;
        imgfilter_t *filter;
        std::shared_ptr<gltexture_t> operator()(texlru_src_external_s &src) const
        {
            int flags = tex->flags;
            auto pic = FS_LoadImage( src.path.c_str(), nullptr, 0 );
            if( !pic )
            {
                MsgDev(D_ERROR, "TexLru : fail to load external texture %s\n", src.path.c_str());
                return {}; // couldn't loading image
            }

            // force upload texture as RGB or RGBA (detail textures requires this)
            if( flags & TF_FORCE_COLOR ) pic->flags |= IMAGE_HAS_COLOR;

            GL_UploadTexture( pic, tex, false, filter );
            GL_TexFilter( tex, false ); // update texture filter, wrap etc

            if(!( flags & ( TF_KEEP_8BIT|TF_KEEP_RGBDATA )))
                pic = nullptr; // release source texture

            MsgDev(D_INFO, "TexLru : load external texture %s\n", src.path.c_str());
            return std::shared_ptr<gltexture_t>{ tex, TexLru_ReleaseTexture };
        }
        std::shared_ptr<gltexture_t> operator()(texlru_src_internal_s &src) const
        {
            const studiohdr_t *phdr = src.phdr;
            Image_SetMDLPointer((const byte *)phdr + src.texture->index);

            int flags = tex->flags;
            // hack : expected mstudiotexture_t* as buffer
            auto pic = FS_LoadImage( src.texture_name.c_str(), (const byte *)src.texture, src.size );
            if( !pic )
            {
                MsgDev(D_ERROR, "TexLru : fail to load internal texture %s from %s\n", src.texture->name, src.model_name.c_str());
                return {}; // couldn't loading image
            }

            // force upload texture as RGB or RGBA (detail textures requires this)
            if( flags & TF_FORCE_COLOR ) pic->flags |= IMAGE_HAS_COLOR;

            GL_UploadTexture( pic, tex, false, filter );
            GL_TexFilter( tex, false ); // update texture filter, wrap etc

            if(!( flags & ( TF_KEEP_8BIT|TF_KEEP_RGBDATA )))
                pic = nullptr; // release source texture

            MsgDev(D_INFO, "TexLru : load internal texture %s from %s\n", src.texture->name, src.model_name.c_str());
            return std::shared_ptr<gltexture_t>{ tex, TexLru_ReleaseTexture };
        }
        std::shared_ptr<gltexture_t> operator()(texlru_src_spr_s &src) const
        {
            SPR_InstallPal(src.psprite);
            int flags = tex->flags;
            auto pic = FS_LoadImage( src.path.c_str(), (const byte*)src.texture, src.texture->width * src.texture->height );
            if( !pic )
            {
                MsgDev(D_ERROR, "TexLru : fail to load sprite texture %s\n", src.path.c_str());
                return {}; // couldn't loading image
            }

            // force upload texture as RGB or RGBA (detail textures requires this)
            if( flags & TF_FORCE_COLOR ) pic->flags |= IMAGE_HAS_COLOR;

            GL_UploadTexture( pic, tex, false, filter );
            GL_TexFilter( tex, false ); // update texture filter, wrap etc

            if(!( flags & ( TF_KEEP_8BIT|TF_KEEP_RGBDATA )))
                pic = nullptr; // release source texture

            // set spr type
            tex->texType = TEX_SPRITE;

            MsgDev(D_INFO, "TexLru : load SPR texture %s\n", src.path.c_str());
            return std::shared_ptr<gltexture_t>{ tex, TexLru_ReleaseTexture };
        }
        std::shared_ptr<gltexture_t> operator()(texlru_src_dds_s &src) const
        {
            int flags = tex->flags;
	        auto pic = FS_LoadImage( src.path.c_str(), (const byte*)src.pin, src.size );
	        if( !pic )
	        {
		        MsgDev(D_ERROR, "TexLru : fail to load sprite texture %s\n", src.path.c_str());
		        return {}; // couldn't loading image
	        }

	        // force upload texture as RGB or RGBA (detail textures requires this)
	        if( flags & TF_FORCE_COLOR ) pic->flags |= IMAGE_HAS_COLOR;

	        GL_UploadTexture( pic, tex, false, filter );
	        GL_TexFilter( tex, false ); // update texture filter, wrap etc

	        if(!( flags & ( TF_KEEP_8BIT|TF_KEEP_RGBDATA )))
		        pic = nullptr; // release source texture

            // set spr type
            tex->texType = TEX_SPRITE;

            MsgDev(D_INFO, "TexLru : load DDS texture %s\n", src.path.c_str());
            return std::shared_ptr<gltexture_t>{ tex, TexLru_ReleaseTexture };
        }
        std::shared_ptr<gltexture_t> operator()(texlru_src_skybox_s &src) const
        {
            int flags = tex->flags;
            const char *skyname = src.skyname.c_str();
            constexpr const char *g_envsuf[6] = { "rt", "bk", "lf", "ft", "up", "dn" };

            image_ref img[6];

            for (int j = 0; j < 6; j++)
            {
                string path;
                sprintf(path, "gfx/env/%s%s.tga", skyname, g_envsuf[j]);
                auto pic = FS_LoadImage(path, nullptr, 0);
                if(!pic)
                {
                    MsgDev(D_ERROR, "TexLru : fail to load skybox %s\n", path);
                    return {}; // couldn't loading image
                }

                // force upload texture as RGB or RGBA (detail textures requires this)
                if( flags & TF_FORCE_COLOR ) pic->flags |= IMAGE_HAS_COLOR;

                img[j] = pic;
            }

            //GL_UploadTexture( pic, tex, false, filter );

            Assert(tex->target == GL_TEXTURE_CUBE_MAP_ARB);
            pglBindTexture(GL_TEXTURE_CUBE_MAP_ARB, tex->texnum);
            tex->texType = TEX_CUBEMAP;
            for(int j = 0; j < 6; ++j)
            {
                auto pic = img[j];

                tex->srcWidth = tex->width = pic->width;
                tex->srcHeight = tex->height = pic->height;
                tex->format = PFDesc[pic->type].glFormat;

                tex->fogParams[0] = pic->fogParams[0];
                tex->fogParams[1] = pic->fogParams[1];
                tex->fogParams[2] = pic->fogParams[2];
                tex->fogParams[3] = pic->fogParams[3];

                pglTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                pglTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                pglTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                pglTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                pglTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
                pglTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB + j, 0, GL_RGBA, pic->width, pic->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pic->buffer);
#ifdef XASH_GLES
				if( GL_Support( GL_SGIS_MIPMAPS_EXT ) )
				{
					pglGenerateMipmap(GL_TEXTURE_CUBE_MAP_ARB);
				}
#endif
            }
            pglBindTexture(GL_TEXTURE_CUBE_MAP_ARB, 0);

            GL_TexFilter( tex, false ); // update texture filter, wrap etc

            MsgDev(D_INFO, "TexLru : load cubemap gfx/env/%s\n", src.skyname.c_str());
            return std::shared_ptr<gltexture_t>{ tex, TexLru_ReleaseSkybox };
        }
    };

    static lru_cache<int, std::shared_ptr<gltexture_t>> textureCache(0);

    bool TexLru_Upload( int texnum )
    {
        gltexture_t	*tex = R_GetTexture(texnum);
        if(!tex->texlru_extdata)
            return false; // unknown texture

        if (textureCache.contains_and_make_first((GLuint)texnum)) {
            return true; // already present
        }

        textureCache.set_capacity(r_texlru->integer);

        auto &lrudata = *tex->texlru_extdata;
        
        #if 0
        if (std::holds_alternative<texlru_src_external_s>(lrudata.src)) {
            auto src = std::get<texlru_src_external_s>(lrudata.src);
            FS_LoadImageAsync(src.path.c_str(), [texnum, src, tex](image_ref pic, uint* pal, int rendermode) {
                if( !pic )
                {
                    MsgDev(D_ERROR, "TexLru : fail to load external texture %s\n", src.path.c_str());
                    return; // couldn't loading image
                }

                if(!tex->name[0])
                {
                    MsgDev(D_ERROR, "TexLru : fail to load external texture %s(%d) already released\n", src.path.c_str(), texnum);
                    return;
                }

                int flags = tex->flags;
                // force upload texture as RGB or RGBA (detail textures requires this)
                if( flags & TF_FORCE_COLOR ) pic->flags |= IMAGE_HAS_COLOR;

                GL_UploadTexture( pic, tex, false, tex->texlru_extdata->filter);
                GL_TexFilter( tex, false ); // update texture filter, wrap etc

                if(!( flags & ( TF_KEEP_8BIT|TF_KEEP_RGBDATA )))
                    pic = nullptr; // release source texture

                g_texureUploaded.insert(texnum);
                MsgDev(D_INFO, "TexLru : load external texture %s\n", src.path.c_str());
            } );
            textureCache.insert((GLuint)texnum, std::move(std::shared_ptr<gltexture_t>{ tex, TexLru_ReleaseTexture }));
        }
        else if(std::holds_alternative<texlru_src_internal_s>(lrudata.src)) {
            auto src = std::get<texlru_src_internal_s>(lrudata.src);
            const studiohdr_t *phdr = src.phdr;
            FS_LoadImageAsync(src.texture_name.c_str(), [texnum, src, tex](image_ref pic, uint* pal, int rendermode) {
                if( !pic )
                {
                    MsgDev(D_ERROR, "TexLru : fail to load internal texture %s from %s\n", src.texture->name, src.model_name.c_str());
                    return; // couldn't loading image
                }

                int flags = tex->flags;
                // force upload texture as RGB or RGBA (detail textures requires this)
                if( flags & TF_FORCE_COLOR ) pic->flags |= IMAGE_HAS_COLOR;

                GL_UploadTexture( pic, tex, false, tex->texlru_extdata->filter);
                GL_TexFilter( tex, false ); // update texture filter, wrap etc

                if(!( flags & ( TF_KEEP_8BIT|TF_KEEP_RGBDATA )))
                    pic = nullptr; // release source texture

                g_texureUploaded.insert(texnum);
                MsgDev(D_INFO, "TexLru : load internal texture %s from %s\n", src.texture->name, src.model_name.c_str());
            }, (const byte *)src.texture, src.size, nullptr, 0, (const byte *)phdr + src.texture->index);
            textureCache.insert((GLuint)texnum, std::move(std::shared_ptr<gltexture_t>{ tex, TexLru_ReleaseTexture }));
        }
        else if(std::holds_alternative<texlru_src_spr_s>(lrudata.src)) {
            auto src = std::get<texlru_src_spr_s>(lrudata.src);
            
            //SPR_InstallPal(src.psprite);
            //uint* pal;
            //int rendermode;
            //Image_GetGlobalPalette(pal, rendermode);

            //FS_LoadImageAsync(src.path.c_str(), [src, tex](image_ref pic, uint* pal, int rendermode) {
            //    if (!pic)
            //    {
            //        MsgDev(D_ERROR, "TexLru : fail to load sprite texture %s\n", src.path.c_str());
            //        return; // couldn't loading image
            //    }

            //    int flags = tex->flags;
            //    // force upload texture as RGB or RGBA (detail textures requires this)
            //    if (flags & TF_FORCE_COLOR) pic->flags |= IMAGE_HAS_COLOR;

            //    GL_UploadTexture(pic, tex, false, tex->texlru_extdata->filter);
            //    GL_TexFilter(tex, false); // update texture filter, wrap etc

            //    if (!(flags & (TF_KEEP_8BIT | TF_KEEP_RGBDATA)))
            //        pic = nullptr; // release source texture

            //    // set spr type
            //    tex->texType = TEX_SPRITE;

            //    MsgDev(D_INFO, "TexLru : load SPR texture %s\n", src.path.c_str());
            //}, (const byte*)src.texture, src.texture->width * src.texture->height, pal, rendermode);
            SPR_InstallPalAsync(src.psprite, [src, tex](uint* pal, int rendermode) {
                FS_LoadImageAsync(src.path.c_str(), [src, tex](image_ref pic, uint* pal, int rendermode) {
                    if (!pic)
                    {
                        MsgDev(D_ERROR, "TexLru : fail to load sprite texture %s\n", src.path.c_str());
                        return; // couldn't loading image
                    }

                    int flags = tex->flags;
                    // force upload texture as RGB or RGBA (detail textures requires this)
                    if (flags & TF_FORCE_COLOR) pic->flags |= IMAGE_HAS_COLOR;

                    GL_UploadTexture(pic, tex, false, tex->texlru_extdata->filter);
                    GL_TexFilter(tex, false); // update texture filter, wrap etc

                    if (!(flags & (TF_KEEP_8BIT | TF_KEEP_RGBDATA)))
                        pic = nullptr; // release source texture

                    // set spr type
                    tex->texType = TEX_SPRITE;

                    MsgDev(D_INFO, "TexLru : load SPR texture %s\n", src.path.c_str());
                }, (const byte*)src.texture, src.texture->width * src.texture->height, pal, rendermode);
            });
            textureCache.insert((GLuint)texnum, std::move(std::shared_ptr<gltexture_t>{ tex, TexLru_ReleaseTexture }));
        }
        else 
        #endif
        {
            auto deleter = std::visit(TexLru_Uploader{ tex, lrudata.filter }, lrudata.src);
            textureCache.insert((GLuint)texnum, std::move(deleter));
        }
        return true;
    }

    const TextureAttr &TexLru_GetAttr(const mstudiotexture_t *ptexture)
    {
        return g_textureAttrMap.at(ptexture);
    }

    void TexLru_FreeTexture(const mstudiotexture_t *ptexture)
    {
        auto iter = g_textureAttrMap.find(ptexture);
        if (iter == g_textureAttrMap.end())
            return;

        int orinalTextureID = iter->second.index;
        auto iter2 = g_textureReplaceMap.find(ptexture);
        if (iter2 != g_textureReplaceMap.end())
            orinalTextureID = iter2->second;

        if (!GL_FreeTextureIgnoreLru(orinalTextureID))
            return;

        if(orinalTextureID != iter->second.index)
            g_textureReplaceMap.erase(ptexture);

        gltexture_t* tex = R_GetTexture(orinalTextureID);
        tex->texlru_extdata = nullptr;
        if (textureCache.contains(orinalTextureID))
            textureCache.remove(orinalTextureID);
        g_textureAttrMap.erase(iter);
    }

    void TexLru_Free( int texnum )
    {
        gltexture_t	*tex = R_GetTexture(texnum);
        tex->texlru_extdata = nullptr;
        if(textureCache.contains(texnum))
            textureCache.remove(texnum);
        std::erase_if(g_textureAttrMap, [texnum](auto& x) { return x.second.index == texnum; });
    }

    void TexLru_Shrink()
    {
        while( textureCache.size() >= 64 ){
            // cache is full, evict the least recently used item
            textureCache.evict();
        }
    }

    void TexLru_Clear()
    {
        textureCache.clear();
    }

    void TexLru_Bind(const mstudiotexture_t *ptexture)
    {
        auto &attr = g_textureAttrMap.at(ptexture);

        // set size for external texture
        if(TexLru_Upload(attr.index))
            TexLru_UpdateAttr(ptexture);
        return GL_Bind( XASH_TEXTURE0, attr.index);
    }

    void TexLru_BindReplace(const mstudiotexture_t* ptexture, int texnum)
    {
        auto iter = g_textureAttrMap.find(ptexture);
        if (iter != g_textureAttrMap.end())
            g_textureReplaceMap.emplace(ptexture, iter->second.index);

        // set size for external texture
        if (TexLru_Upload(texnum))
            TexLru_CreateAttr(ptexture, texnum);
        return GL_Bind(XASH_TEXTURE0, texnum);
    }

    void TexLru_Bind(const msprite_t *psprite, const mspriteframe_t *pspriteframe)
    {
        TexLru_Upload(psprite, pspriteframe);
        return GL_Bind(XASH_TEXTURE0, pspriteframe->gl_texturenum);
    }

    void TexLru_Upload(const msprite_t *psprite, const mspriteframe_t *pspriteframe)
    {
        for(int i = 0; i < psprite->numframes; ++i)
            TexLru_Upload(psprite->frames[i].frameptr->gl_texturenum);
    }

    // for remove studio lru
    bool TexLru_IsTextureUploaded(int texnum)
    {
        return g_texureUploaded.erase(texnum) != 0;
    }
}