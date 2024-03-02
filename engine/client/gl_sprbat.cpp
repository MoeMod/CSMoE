#include "common.h"
#include "client.h"
#include "gl_local.h"
#include "gl_texlru.h"
#include <triangleapi.h>
extern triangleapi_t gTriApi;
#include "gl_sprbat.h"
#include "gl_shader.h"

namespace xe {
#define XASH_SHADER
#ifdef XASH_SHADER
	typedef enum
	{
		SPRBAT_SHAEDR_VARIANT_NORMAL,
		SPRBAT_SHAEDR_VARIANT_ALPHATEST,
		MAX_SPRBAT_SHAEDR_VARIANTS
	} sprbatshadervariant_e;
	static const char* sprbatshadervariant_define[MAX_SPRBAT_SHAEDR_VARIANTS] = {
			"",
			"#define SPRBAT_ALPHA_TEST\n"
	};
#define XASH_VBO_BUFFER_COUNT 32
	typedef struct sprshader_s {
		GLuint program;
		GLuint vertex_shader;
		GLuint fragment_shader;
		struct {
			GLint v_up; // vec3
			GLint v_right; // vec3
			GLint muWVP; // mat4
			GLint texSprite; // mat4
			GLint frame; // vec4
			GLint org; // vec3
			GLint color; // vec4
			GLint scale;
		} uniform;
		struct {
			GLint vaTexCoord; // vec2
		} attrib;
		GLuint array_buffer[XASH_VBO_BUFFER_COUNT];
        int array_buffer_cur;
	} sprshader_t;
	sprshader_t g_SprShaderVariant[MAX_SPRBAT_SHAEDR_VARIANTS];
	sprshader_t *g_pSprShader = nullptr;
#endif

#ifdef XASH_SHADER
	void SprBat_UseShader(int variant)
	{
		if(g_pSprShader == &g_SprShaderVariant[variant])
			return;
		g_pSprShader = &g_SprShaderVariant[variant];
		pglUseProgramObjectARB(g_pSprShader->program);
	}

	void SprBat_DropShader()
	{
		if(!g_pSprShader)
			return;
		g_pSprShader = nullptr;
		pglUseProgramObjectARB(0);
	}
#endif

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

		if ( !(last_pass & SprBatPass_Blend) && (pass & SprBatPass_Blend) )
		{
			pglEnable( GL_BLEND );
		}
		if ( (last_pass & SprBatPass_Blend) && !(pass & SprBatPass_Blend) )
		{
			pglDisable(GL_BLEND);
		}

		if (add_cap & SprBatPass_AlphaBlend)
		{
			pglBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		}

		if(add_cap & SprBatPass_Additive)
		{
			//pglDisable( GL_FOG );
#ifdef XASH_GLES
            // HACK : gl4es handles glBlendFunc( GL_SRC_ALPHA, GL_ONE ) as glBlendFunc( GL_ONE, GL_ONE )
            // special case, as seen in Xash3D, but it breaks torus_trooper, so behind a parameter
            pglBlendFunc( GL_ONE, GL_ONE );
#else
			pglBlendFunc( GL_SRC_ALPHA, GL_ONE );
#endif
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
#ifdef XASH_SHADER
		pglUniform3fvARB(g_pSprShader->uniform.v_up, 1, v_up.data());
		pglUniform3fvARB(g_pSprShader->uniform.v_right, 1, v_right.data());
		pglUniformMatrix4fvARB(g_pSprShader->uniform.muWVP, 1, GL_FALSE,(const float *) &RI.worldviewProjectionMatrixTranspose);
		pglUniform1iARB(g_pSprShader->uniform.texSprite, glState.activeTMU);
		pglUniform4fARB(g_pSprShader->uniform.frame, frame->up, frame->down, frame->left, frame->right);
		pglUniform3fvARB(g_pSprShader->uniform.org, 1, org.data());
		pglUniform4fvARB(g_pSprShader->uniform.color, 1, color.data());
		pglUniform1fARB(g_pSprShader->uniform.scale, scale);

        int j = g_pSprShader->array_buffer_cur++;
        g_pSprShader->array_buffer_cur %= XASH_VBO_BUFFER_COUNT;
		pglBindBufferARB(GL_ARRAY_BUFFER_ARB, g_pSprShader->array_buffer[j]);
		pglEnableVertexAttribArrayARB(g_pSprShader->attrib.vaTexCoord);
		pglVertexAttribPointerARB(g_pSprShader->attrib.vaTexCoord, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
		pglDrawArrays(GL_TRIANGLE_FAN, 0, 6);
		pglDisableVertexAttribArrayARB(g_pSprShader->attrib.vaTexCoord);
		pglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
#else
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
#endif
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
#ifdef XASH_SHADER
		if(iter != mid)
			SprBat_UseShader(SPRBAT_SHAEDR_VARIANT_NORMAL);
#endif
		std::sort(iter, mid, SprBat_DrawDataCompare);
		std::for_each(iter, mid, SprBat_RenderDrawData);
		iter = mid;
		// render alpha test
		mid = std::stable_partition(iter, end, [](const DrawData & a){
			return (a.pass & SprBatPass_AlphaTest);
		});
#ifdef XASH_SHADER
		if(iter != mid)
			SprBat_UseShader(SPRBAT_SHAEDR_VARIANT_ALPHATEST);
#endif
		std::sort(iter, mid, SprBat_DrawDataCompare);
		std::for_each(iter, mid, SprBat_RenderDrawData);
		iter = mid;
		// render blend (no sort!)
		mid = std::stable_partition(iter, end, [](const DrawData & a){
            return (a.pass & SprBatPass_Blend);
		});
#ifdef XASH_SHADER
		if(iter != mid)
			SprBat_UseShader(SPRBAT_SHAEDR_VARIANT_NORMAL);
#endif
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
#ifdef XASH_SHADER
		SprBat_DropShader();
#else
		gTriApi.Color4f( 1, 1, 1, 1 );
#endif
	}

	void SprBat_InitShader()
	{
#ifdef XASH_SHADER
		for (int i = SPRBAT_SHAEDR_VARIANT_NORMAL; i < MAX_SPRBAT_SHAEDR_VARIANTS; ++i)
		{
			g_pSprShader = &g_SprShaderVariant[i];
			const GLcharARB *vertex_shader_list[3] = {
					xe::GetShaderVersionString(),
					sprbatshadervariant_define[i],
					R"(
	in vec2 vaTexCoord;

	uniform vec4 frame;
	uniform vec3 v_up;
	uniform vec3 v_right;
	uniform vec3 org;
	uniform float scale;
	uniform mat4 muWVP;

	out vec2 vvTexCoord;
	void main(void)
	{
		gl_Position = muWVP * vec4(mat3(v_right, v_up, org) * vec3(scale * ((1.0 - vaTexCoord) * frame.zx + vaTexCoord * frame.wy), 1.0), 1.0);
	    vvTexCoord = vaTexCoord;
	}
	)"
			};
			g_pSprShader->vertex_shader = pglCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
			pglShaderSourceARB(g_pSprShader->vertex_shader, 3, vertex_shader_list, NULL);
			pglCompileShaderARB(g_pSprShader->vertex_shader);
			xe::CheckShader(g_pSprShader->vertex_shader, "vertex shader");

			const GLcharARB *fragment_shader_list[4] = {
					xe::GetShaderVersionString(),
					xe::GetFragmentShaderPrecision(),
					sprbatshadervariant_define[i],
					R"(
	uniform sampler2D texSprite;
	uniform vec4 color;
	in vec2 vvTexCoord;
	out vec4 FragColor;

	void main (void)
	{
	    FragColor = texture(texSprite, vvTexCoord) * color;
#ifdef SPRBAT_ALPHA_TEST
		if(!(FragColor.a > 0.0))
			discard;
#endif
	}
	)"
			};
			g_pSprShader->fragment_shader = pglCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
			pglShaderSourceARB(g_pSprShader->fragment_shader, 4, fragment_shader_list, NULL);
			pglCompileShaderARB(g_pSprShader->fragment_shader);
			xe::CheckShader(g_pSprShader->fragment_shader, "fragment shader");

			g_pSprShader->program = pglCreateProgramObjectARB();
			pglAttachObjectARB(g_pSprShader->program, g_pSprShader->vertex_shader);
			pglAttachObjectARB(g_pSprShader->program, g_pSprShader->fragment_shader);
			pglLinkProgramARB(g_pSprShader->program);
			xe::CheckProgram(g_pSprShader->program, "shader program");


			g_pSprShader->uniform.frame = pglGetUniformLocationARB(g_pSprShader->program, "frame");
			g_pSprShader->uniform.v_up = pglGetUniformLocationARB(g_pSprShader->program, "v_up");
			g_pSprShader->uniform.v_right = pglGetUniformLocationARB(g_pSprShader->program, "v_right");
			g_pSprShader->uniform.muWVP = pglGetUniformLocationARB(g_pSprShader->program, "muWVP");
			g_pSprShader->uniform.texSprite = pglGetUniformLocationARB(g_pSprShader->program, "texSprite");
			g_pSprShader->uniform.org = pglGetUniformLocationARB(g_pSprShader->program, "org");
			g_pSprShader->uniform.color = pglGetUniformLocationARB(g_pSprShader->program, "color");
			g_pSprShader->uniform.scale = pglGetUniformLocationARB(g_pSprShader->program, "scale");

			g_pSprShader->attrib.vaTexCoord = pglGetAttribLocationARB(g_pSprShader->program, "vaTexCoord");

			pglGenBuffersARB(XASH_VBO_BUFFER_COUNT, g_pSprShader->array_buffer);
            // 0 1 2 0 2 3
            const float QuadVertexTexCoords[] = {
                    0, 1,
                    0, 0,
                    1, 0,
                    0, 1,
                    1, 0,
                    1, 1,
            };
            static_assert(sizeof(QuadVertexTexCoords) == 6 * 2 * sizeof(float));
            for(int j = 0; j < XASH_VBO_BUFFER_COUNT; ++j)
            {
                pglBindBufferARB(GL_ARRAY_BUFFER_ARB, g_pSprShader->array_buffer[j]);
                pglBufferDataARB(GL_ARRAY_BUFFER_ARB, sizeof(QuadVertexTexCoords), QuadVertexTexCoords, GL_STATIC_DRAW_ARB);
            }
            pglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
		}
		g_pSprShader = nullptr;
#endif
	}

	void SprBat_ShutdownShader()
	{
#ifdef XASH_SHADER
		for (int i = SPRBAT_SHAEDR_VARIANT_NORMAL; i < MAX_SPRBAT_SHAEDR_VARIANTS; ++i)
		{
			g_pSprShader = &g_SprShaderVariant[i];
			pglDetachObjectARB(g_pSprShader->program, g_pSprShader->vertex_shader);
			pglDetachObjectARB(g_pSprShader->program, g_pSprShader->fragment_shader);
			pglDeleteObjectARB(g_pSprShader->vertex_shader);
			pglDeleteObjectARB(g_pSprShader->fragment_shader);
			pglDeleteProgram(g_pSprShader->program);

            for(int j = 0; j < XASH_VBO_BUFFER_COUNT; ++j)
            {
                pglDeleteBuffersARB(1, &g_pSprShader->array_buffer[j]);
            }
		}
#endif
	}
}