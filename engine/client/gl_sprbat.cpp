#include "common.h"
#include "client.h"
#include "gl_local.h"
#include "gl_texlru.h"
#include <triangleapi.h>
extern triangleapi_t gTriApi;
#include "gl_sprbat.h"

namespace xe {
	void SprBat_SetupRenderState(int pass, int last_pass)
	{
		if(pass == last_pass)
			return; // fast path

		int add_cap = pass & ~last_pass;
		int remove_cap = ~pass & last_pass;

		if(add_cap & SprBatPass_AlphaTest)
		{
			//pglEnable( GL_ALPHA_TEST );
			//pglAlphaFunc( GL_GREATER, 0.0f );
		}
		else if(remove_cap & SprBatPass_AlphaTest)
		{
			//pglDisable( GL_ALPHA_TEST );
		}

		if(add_cap & SprBatPass_Blend)
		{
			pglEnable( GL_BLEND );
			pglBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		}
		else if(remove_cap & SprBatPass_Blend)
		{
			pglDisable( GL_BLEND );
		}

		if(add_cap & SprBatPass_Src_One)
		{
			//pglDisable( GL_FOG );
			pglBlendFunc( GL_SRC_ALPHA, GL_ONE );
		}
		else if(remove_cap & SprBatPass_Src_One)
		{
			//if( RI.fogCustom || ( RI.fogEnabled && !glState.drawTrans ))
			//	pglEnable( GL_FOG );
			pglBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		}

		if(add_cap & SprBatPass_NoDepthTest)
		{
			pglDisable( GL_DEPTH_TEST );
		}
		else if(remove_cap & SprBatPass_NoDepthTest)
		{
			pglEnable( GL_DEPTH_TEST );
		}

		if(add_cap & SprBatPass_NoCull)
		{
			GL_Cull( GL_NONE );
		}
		else if(remove_cap & SprBatPass_NoCull)
		{
			GL_Cull( GL_FRONT );
		}
	}

	void SprBat_RestoreRenderState(int last_pass)
	{
		if( last_pass & SprBatPass_NoDepthTest )
			pglEnable( GL_DEPTH_TEST );

		//if( last_pass & SprBatPass_AlphaTest )
		//	pglDisable( GL_ALPHA_TEST );

		//if( last_pass & SprBatPass_Blend )
		//    pglDisable( GL_BLEND );

		//if( RI.fogCustom || ( RI.fogEnabled && !glState.drawTrans ))
		//	pglEnable( GL_FOG );

		if( last_pass & SprBatPass_NoCull )
			GL_Cull( GL_FRONT );
	}

	void SprBat_DrawSpriteQuad( msprite_t *psprite, mspriteframe_t *frame, vec3_t org, vec3_t v_right, vec3_t v_up, float scale, vec4_t color )
	{
		xe::TexLru_Bind(psprite, frame);
		gTriApi.Color4f(color[0], color[1], color[2], color[3]);

		vec3_t	point;

		r_stats.c_sprite_polys++;

		gTriApi.Begin( TRI_QUADS );
		gTriApi.TexCoord2f( 0.0f, 1.0f );
		VectorMA( org, frame->down * scale, v_up, point );
		VectorMA( point, frame->left * scale, v_right, point );
		gTriApi.Vertex3fv( point );
		gTriApi.TexCoord2f( 0.0f, 0.0f );
		VectorMA( org, frame->up * scale, v_up, point );
		VectorMA( point, frame->left * scale, v_right, point );
		gTriApi.Vertex3fv( point );
		gTriApi.TexCoord2f( 1.0f, 0.0f );
		VectorMA( org, frame->up * scale, v_up, point );
		VectorMA( point, frame->right * scale, v_right, point );
		gTriApi.Vertex3fv( point);
		gTriApi.TexCoord2f( 1.0f, 1.0f );
		VectorMA( org, frame->down * scale, v_up, point );
		VectorMA( point, frame->right * scale, v_right, point );
		gTriApi.Vertex3fv( point );
		gTriApi.End();
	}

	struct DrawData
	{
		int pass;
		msprite_t *psprite;
		mspriteframe_t *frame;
		float scale;
		vec3_t org, v_right, v_up;
		vec4_t color;
	};
    const int drawlist_maxnum = MAX_VISIBLE_PACKET * 4;
    DrawData drawlist[MAX_VISIBLE_PACKET * 4];
    int drawlist_num = 0;
	int sprbat_current_pass;

	void SprBat_RenderDrawData(const DrawData &data)
	{
		SprBat_SetupRenderState(data.pass, sprbat_current_pass);
        sprbat_current_pass = data.pass;
		SprBat_DrawSpriteQuad(data.psprite, data.frame, data.org, data.v_right, data.v_up, data.scale, data.color);
	}

	bool SprBat_DrawDataCompare(const DrawData &a, const DrawData &b)
	{
		return std::tie(a.pass, a.frame->gl_texturenum) < std::tie(b.pass, b.frame->gl_texturenum);
	}

	void SprBat_AddDrawList( int pass, msprite_t *psprite, mspriteframe_t *frame, vec3_t org, vec3_t v_right, vec3_t v_up, float scale, vec4_t color )
	{
        if(drawlist_num >= drawlist_maxnum)
            return;
		DrawData data{ pass, psprite, frame, scale, org, v_right, v_up, color };
		drawlist[drawlist_num++] = data;
	}

	void SprBat_RenderDrawList()
	{
		sprbat_current_pass = SprBatPass_Opaque;
		// all sprites can have color
		//pglTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
		auto iter = drawlist;
        auto end = drawlist + drawlist_num;
		// render opaque first
		auto mid = std::stable_partition(iter, end, [](const DrawData & a){
			return !(a.pass & SprBatPass_AlphaTest) && !(a.pass & SprBatPass_Blend);
		});
		std::sort(iter, mid, SprBat_DrawDataCompare);
		std::for_each(iter, mid, SprBat_RenderDrawData);
		iter = mid;
		// render alpha test
		mid = std::stable_partition(iter, end, [](const DrawData & a){
			return (a.pass & SprBatPass_AlphaTest) && !(a.pass & SprBatPass_Blend);
		});
		std::sort(iter, mid, SprBat_DrawDataCompare);
		std::for_each(iter, mid, SprBat_RenderDrawData);
		iter = mid;
		// render blend (no sort!)
		mid = std::stable_partition(iter, end, [](const DrawData & a){
            return (a.pass & SprBatPass_Blend);
		});
		//std::sort(iter, mid, SprBat_DrawDataCompare);
		std::for_each(iter, mid, SprBat_RenderDrawData);
		iter = mid;
        Assert(iter == end);
		if(sprbat_current_pass != SprBatPass_Opaque)
			SprBat_RestoreRenderState(sprbat_current_pass);
		sprbat_current_pass = SprBatPass_Opaque;
        drawlist_num = 0;
        pglDisable( GL_BLEND );
        pglDepthFunc( GL_LEQUAL );
		//pglTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
		gTriApi.Color4f( 1, 1, 1, 1 );
	}
}