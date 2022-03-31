#include "hud.h"
#include "followicon.h"
#include "cl_util.h"
#include "draw_util.h"
#include "calcscreen.h"

#include <tuple>

#include "gamemode/mods_const.h"

namespace cl {

int CHudFollowIcon::Init(void)
{
	m_iBombTargetsNum = 0;
	gHUD.AddHudElem(this);
	m_iFlags = 0;
	return 1;
}

int CHudFollowIcon::VidInit(void)
{
	if (!m_pTexture_BombAB[0])
		m_pTexture_BombAB[0] = R_LoadTextureUnique("resource/helperhud/c4_a");
	if (!m_pTexture_BombAB[1])
		m_pTexture_BombAB[1] = R_LoadTextureUnique("resource/helperhud/c4_b");
	if (!m_pTexture_Supplybox)
		m_pTexture_Supplybox = R_LoadTextureUnique("resource/helperhud/supplybox");
	m_iFlags |= HUD_DRAW;
	return 1;
}

void CHudFollowIcon::Shutdown(void)
{
	m_pTexture_BombAB[0] = nullptr;
	m_pTexture_BombAB[1] = nullptr;
	m_pTexture_Supplybox = nullptr;
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
			m_pTexture_BombAB[i]->Draw2DQuadScaled(xyScreen[0] - 15, xyScreen[1] - 25, xyScreen[0] + 16, xyScreen[1] + 26);
		}
	}

	if (gHUD.m_iModRunning == MOD_ZB2 || gHUD.m_iModRunning == MOD_ZB3 || gHUD.m_iModRunning == MOD_ZBZ)
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
					m_pTexture_Supplybox->Draw2DQuadScaled(xyScreen[0] - 18, xyScreen[1] - 18, xyScreen[0] + 19, xyScreen[1] + 19);

					char szBuffer[16];
					sprintf(szBuffer, "[%im]", static_cast<int>((g_HostageInfo[i].origin - gHUD.m_vecOrigin).Length() / 42.0f));

					int textlen = DrawUtils::HudStringLen(szBuffer);
					int r, g, b;
					DrawUtils::UnpackRGB(r, g, b, RGB_LIGHTBLUE);
					DrawUtils::DrawHudString(xyScreen[0] - textlen * 0.5f, xyScreen[1] + 25, gHUD.m_scrinfo.iWidth, szBuffer, r, g, b);
				}

			}
		}

	}

	return 1;
}

}