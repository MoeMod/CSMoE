#include "hud.h"
#include "followicon.h"
#include "cl_util.h"
#include "draw_util.h"
#include "triangleapi.h"

#include "calcscreen.h"

#include "gamemode/mods_const.h"

int CHudFollowIcon::Init(void)
{
	m_iBombTargetsNum = 0;
	m_iBombAB[0] = m_iBombAB[1] = 0;
	m_iSupplybox = 0;
	gHUD.AddHudElem(this);
	m_iFlags = 0;
	return 1;
}

int CHudFollowIcon::VidInit(void)
{
	m_iBombAB[0] = gRenderAPI.GL_LoadTexture("resource/helperhud/c4_a", NULL, 0, TF_NEAREST | TF_NOPICMIP | TF_NOMIPMAP | TF_CLAMP);
	m_iBombAB[1] = gRenderAPI.GL_LoadTexture("resource/helperhud/c4_b", NULL, 0, TF_NEAREST | TF_NOPICMIP | TF_NOMIPMAP | TF_CLAMP);
	m_iSupplybox = gRenderAPI.GL_LoadTexture("resource/helperhud/supplybox", NULL, 0, TF_NEAREST | TF_NOPICMIP | TF_NOMIPMAP | TF_CLAMP);
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
	// AB Points
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

	if (gHUD.m_iModRunning == MOD_ZB2 || gHUD.m_iModRunning == MOD_ZB3)
	{
		if (g_PlayerExtraInfo[gHUD.m_Scoreboard.m_iPlayerNum].teamnumber == TEAM_CT)
		{
			for (int i = 0; i < MAX_HOSTAGES; i++)
			{
				if (g_HostageInfo[i].dead)
					continue;

				float xyScreen[2];
				if (CalcScreen(g_HostageInfo[i].origin, xyScreen))
				{
					gEngfuncs.pTriAPI->RenderMode(kRenderTransTexture);
					gEngfuncs.pTriAPI->Color4ub(255, 255, 255, 255);
					gRenderAPI.GL_SelectTexture(0);
					gRenderAPI.GL_Bind(0, m_iSupplybox);

					DrawUtils::Draw2DQuad(xyScreen[0] - 18, xyScreen[1] - 18, xyScreen[0] + 19, xyScreen[1] + 19);

					char szBuffer[16];
					sprintf(szBuffer, "[%im]", static_cast<int>((g_HostageInfo[i].origin - gHUD.m_vecOrigin).Length()));

					int textlen = DrawUtils::HudStringLen(szBuffer);
					DrawUtils::DrawHudString(xyScreen[0] - textlen * 0.5f, xyScreen[1] + 30, gHUD.m_scrinfo.iWidth, szBuffer, 150, 150, 250);
				}

			}
		}
		
	}

	return 1;
}