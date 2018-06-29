#include "hud.h"
#include "followicon.h"
#include "cl_util.h"
#include "draw_util.h"
#include "triangleapi.h"

#include "calcscreen.h"

int CHudFollowIcon::Init(void)
{
	m_iBombTargetsNum = 0;
	m_iBombAB[0] = m_iBombAB[1] = 0;
	gHUD.AddHudElem(this);
	m_iFlags = 0;
	return 1;
}

int CHudFollowIcon::VidInit(void)
{
	m_iBombAB[0] = gRenderAPI.GL_LoadTexture("resource/helperhud/c4_a", NULL, 0, TF_NEAREST | TF_NOPICMIP | TF_NOMIPMAP | TF_CLAMP);
	m_iBombAB[1] = gRenderAPI.GL_LoadTexture("resource/helperhud/c4_b", NULL, 0, TF_NEAREST | TF_NOPICMIP | TF_NOMIPMAP | TF_CLAMP);
	m_iFlags |= HUD_DRAW;
	return 1;
}

void CHudFollowIcon::Shutdown(void)
{
	gRenderAPI.GL_FreeTexture(m_iBombAB[0]);
	gRenderAPI.GL_FreeTexture(m_iBombAB[1]);
}

void CHudFollowIcon::Reset(void)
{
	
}

int CHudFollowIcon::Draw(float time)
{
	for (int i = 0; i < m_iBombTargetsNum; ++i)
	{
		float xyScreen[2];
		if (CalcScreen(m_vecBombTargets[i], xyScreen))
		{
			gEngfuncs.pTriAPI->RenderMode(kRenderTransTexture);
			gEngfuncs.pTriAPI->Color4ub(255, 255, 255, 255);
			gRenderAPI.GL_SelectTexture(0);
			gRenderAPI.GL_Bind(0, m_iBombAB[i]);

			DrawUtils::Draw2DQuad(xyScreen[0] - 15, xyScreen[1] - 25, xyScreen[0] + 16, xyScreen[1] + 26);
		}
	}
	return 1;
}