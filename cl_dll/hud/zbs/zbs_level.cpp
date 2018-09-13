#include "hud.h"
#include "followicon.h"
#include "cl_util.h"
#include "draw_util.h"
#include "triangleapi.h"

#include "zbs.h"
#include "zbs_level.h"

#include "string.h"

int CHudZBSLevel::VidInit(void)
{
	m_iZBSBoard_BG = gRenderAPI.GL_LoadTexture("resource/hud/zbs/zbslevelupbg", NULL, 0, TF_NEAREST | TF_NOPICMIP | TF_NOMIPMAP | TF_CLAMP);
	m_iZBSBoard_BG_Wall = gRenderAPI.GL_LoadTexture("resource/hud/zbs/zbslevelupwallbg", NULL, 0, TF_NEAREST | TF_NOPICMIP | TF_NOMIPMAP | TF_CLAMP);
	//m_iFlags |= HUD_DRAW;

	m_iLevel_HP = m_iLevel_ATK = m_iLevel_Wall = 1;
	return 1;
}

void CHudZBSLevel::Shutdown(void)
{
	gRenderAPI.GL_FreeTexture(m_iZBSBoard_BG);
	gRenderAPI.GL_FreeTexture(m_iZBSBoard_BG_Wall);
}

int CHudZBSLevel::Draw(float time)
{
	int x = ScreenWidth - 220;
	int y = ScreenHeight - 220;
	const float flScale = 0.0f;

	gEngfuncs.pTriAPI->RenderMode(kRenderTransAlpha);
	gEngfuncs.pTriAPI->Color4ub(255, 255, 255, 255);
	gRenderAPI.GL_SelectTexture(0);
	gRenderAPI.GL_Bind(0, m_iZBSBoard_BG);
	DrawUtils::Draw2DQuad(x, y, x + 204, y + 85);
	
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
	y -= 30;
	gRenderAPI.GL_SelectTexture(0);
	gRenderAPI.GL_Bind(0, m_iZBSBoard_BG_Wall);
	DrawUtils::Draw2DQuad(x, y, x + 204, y + 28);

	DrawUtils::DrawHudString(x + 20, y + 5, ScreenWidth, "Durability", r, g, b, flScale);
	if (m_iLevel_Wall >= 40)
		sprintf(szBuffer, "Level MAX");
	else
		sprintf(szBuffer, "Level %d", m_iLevel_Wall);
	DrawUtils::DrawHudString(x + 100, y + 5, ScreenWidth, szBuffer, r, g, b, flScale);

	return 1;
}