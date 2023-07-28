#pragma once

namespace xe {

	enum DrawSpritePassType {
		SprBatPass_Opaque = 0,
		SprBatPass_NoDepthTest = 1 << 0,
		SprBatPass_NoCull = 1 << 1,
		SprBatPass_Src_One = 1 << 2,

		SprBatPass_AlphaTest = 1 << 3,
		SprBatPass_Blend = 1 << 4,
	};

	void SprBat_SetupRenderState(int pass, int last_pass = 0);
	void SprBat_RestoreRenderState(int last_pass);
	void SprBat_DrawSpriteQuad( msprite_t *psprite, mspriteframe_t *frame, vec3_t org, vec3_t v_right, vec3_t v_up, float scale, vec4_t color );

	void SprBat_RenderDrawList();
	void SprBat_AddDrawList( int pass, msprite_t *psprite, mspriteframe_t *frame, vec3_t org, vec3_t v_right, vec3_t v_up, float scale, vec4_t color );
}