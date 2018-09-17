#include "hud.h"
#include "followicon.h"
#include "cl_util.h"
#include "draw_util.h"
#include "triangleapi.h"

#include "zbs.h"
#include "zbs_roundclear.h"

#include "string.h"
#include "assert.h"

int CHudZBSRoundClear::VidInit(void)
{
	m_iRoundClear = gRenderAPI.GL_LoadTexture("resource/hud/zbs/roundclear", NULL, 0, TF_NEAREST | TF_NOPICMIP | TF_NOMIPMAP | TF_CLAMP);

	return 1;
}

void CHudZBSRoundClear::Shutdown(void)
{
	gRenderAPI.GL_FreeTexture(m_iRoundClear);
}

int CHudZBSRoundClear::Draw(float time)
{
	int x = ScreenWidth / 2;
	int y = 5;
	const float flScale = 0.0f;

	gEngfuncs.pTriAPI->RenderMode(kRenderTransAlpha);
	gEngfuncs.pTriAPI->Color4ub(255, 255, 255, 255);
	gRenderAPI.GL_SelectTexture(0);
	gRenderAPI.GL_Bind(0, m_iRoundClear);
	DrawUtils::Draw2DQuad(x - 373 / 2, y, x + 373 / 2, y + 51);
	return 1;
}