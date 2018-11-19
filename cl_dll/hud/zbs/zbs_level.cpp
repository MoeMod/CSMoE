#include "hud.h"
#include "followicon.h"
#include "cl_util.h"
#include "draw_util.h"
#include "triangleapi.h"

#include "zbs.h"
#include "zbs_level.h"

#include "string.h"

CHudZBSLevel::CHudZBSLevel(void)
{
	m_iLevel_HP = m_iLevel_ATK = m_iLevel_Wall = 0;
}

int CHudZBSLevel::VidInit(void)
{
	if (!m_pTexture_ZBSBoard_BG) 
		m_pTexture_ZBSBoard_BG = R_LoadTextureUnique("resource/hud/zbs/zbslevelupbg");
	if (!m_pTexture_ZBSBoard_BG_Wall) 
		m_pTexture_ZBSBoard_BG_Wall = R_LoadTextureUnique("resource/hud/zbs/zbslevelupwallbg");
	//m_iFlags |= HUD_DRAW;

	m_iLevel_HP = m_iLevel_ATK = m_iLevel_Wall = 1;
	return 1;
}

int CHudZBSLevel::Draw(float time)
{
	int x = ScreenWidth - 220;
	int y = ScreenHeight - 220;
	const float flScale = 0.0f;

	gEngfuncs.pTriAPI->RenderMode(kRenderTransAlpha);
	gEngfuncs.pTriAPI->Color4ub(255, 255, 255, 255);
	m_pTexture_ZBSBoard_BG->Bind();
	DrawUtils::Draw2DQuadScaled(x, y, x + 204, y + 85);
	
	const int r = 255, g = 255, b = 255;

	DrawUtils::DrawHudString(x + 20, y + 5, ScreenWidth, "HP", r, g, b, flScale);
	DrawUtils::DrawHudString(x + 20, y + 50, ScreenWidth, "ATT", r, g, b, flScale);

	char szBuffer[64];

	// HP Level
	if (m_iLevel_HP >= 40)
		sprintf(szBuffer, "Level MAX");
	else
		sprintf(szBuffer, "Level %d", m_iLevel_HP);
	DrawUtils::DrawHudString(x + 100, y + 5, ScreenWidth, szBuffer, r, g, b, flScale);

	// ATK Level
	if (m_iLevel_ATK >= 40)
		sprintf(szBuffer, "Level MAX");
	else
		sprintf(szBuffer, "Level %d", m_iLevel_ATK);
	DrawUtils::DrawHudString(x + 100, y + 40, ScreenWidth, szBuffer, r, g, b, flScale);

	// ATK Value
	if (m_iLevel_ATK >= 40)
		strcpy(szBuffer, "500%");
	else
		sprintf(szBuffer, "%d%%", m_iLevel_ATK * 10 + 90);
	DrawUtils::DrawHudString(x + 130, y + 65, ScreenWidth, szBuffer, r, g, b, flScale);
	
	// Wall Level
	if (m_iLevel_Wall)
	{
		y -= 32;
		m_pTexture_ZBSBoard_BG_Wall->Bind();
		DrawUtils::Draw2DQuadScaled(x, y, x + 204, y + 28);

		DrawUtils::DrawHudString(x + 20, y + 5, ScreenWidth, "Durability", r, g, b, flScale);
		if (m_iLevel_Wall >= 40)
			sprintf(szBuffer, "Level MAX");
		else
			sprintf(szBuffer, "Level %d", m_iLevel_Wall);
		DrawUtils::DrawHudString(x + 100, y + 5, ScreenWidth, szBuffer, r, g, b, flScale);
	}
	

	return 1;
}