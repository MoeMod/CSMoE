#include "hud.h"
#include "cl_util.h"
#include "draw_util.h"
#include "triangleapi.h"

#include "hud_scoreboard_legacy.h"

#include "gamemode/mods_const.h"


CHudScoreBoardLegacy::CHudScoreBoardLegacy()
{
	
}

CHudScoreBoardLegacy::~CHudScoreBoardLegacy()
{
	
}

int CHudScoreBoardLegacy::VidInit(void)
{
	BuildHudNumberRect(m_iNum_L, m_rcNumber_Large, 13, 13, 1, 1);
	BuildHudNumberRect(m_iNum_S, m_rcNumber_Small, 10, 10, 1, 1);

	m_iOriginalBG = gHUD.GetSpriteIndex("SBOriginalBG");
	m_iTeamDeathBG = gHUD.GetSpriteIndex("SBTeamDeathBG");
	m_iUnitehBG = gHUD.GetSpriteIndex("SBUnitehBG");
	m_iNum_L = gHUD.GetSpriteIndex("SBNum_L");
	m_iNum_S = gHUD.GetSpriteIndex("SBNum_S");
	m_iText_CT = gHUD.GetSpriteIndex("SBText_CT");
	m_iText_T = gHUD.GetSpriteIndex("SBText_T");
	m_iText_TR = gHUD.GetSpriteIndex("SBText_TR");
	m_iText_HM = gHUD.GetSpriteIndex("SBText_HM");
	m_iText_ZB = gHUD.GetSpriteIndex("SBText_ZB");
	m_iText_1st = gHUD.GetSpriteIndex("SBText_1st");
	m_iText_Kill = gHUD.GetSpriteIndex("SBText_Kill");
	m_iText_Round = gHUD.GetSpriteIndex("SBText_Round");

	return 1;
}

int CHudScoreBoardLegacy::Draw(float flTime)
{
	if (g_iUser1)
		return 1;

	int idx = gEngfuncs.GetLocalPlayer()->index;

	if (g_PlayerExtraInfo[idx].dead == true)
		return 1;

	if (m_iBGIndex == -1)
		return 1;

	HSPRITE bgSprite = gHUD.GetSprite(m_iBGIndex);
	wrect_t bgRect = gHUD.GetSpriteRect(m_iBGIndex);
	int bgHeight = (bgRect.bottom - bgRect.top);
	int bgWidth = (bgRect.right - bgRect.left);
	int bgY = 2;
	int bgX = (ScreenWidth - bgWidth) / 2;

	if (bgSprite)
	{
		SPR_Set(bgSprite, 255, 255, 255);
		SPR_DrawHoles(0, bgX, bgY, &bgRect);
	}

	HSPRITE textSprite = gHUD.GetSprite(m_iTextIndex);

	if (textSprite)
	{
		wrect_t textRect = gHUD.GetSpriteRect(m_iTextIndex);

		SPR_Set(textSprite, 128, 128, 128);
		SPR_DrawAdditive(0, (ScreenWidth - (textRect.right - textRect.left)) / 2, bgY + 29, &textRect);
	}

	HSPRITE textSprite_T = gHUD.GetSprite(m_iTTextIndex);

	if (textSprite_T)
	{
		wrect_t textRect = gHUD.GetSpriteRect(m_iTTextIndex);

		SPR_Set(textSprite_T, 128, 128, 128);
		SPR_DrawAdditive(0, (ScreenWidth) / 2 - 50, bgY + 11, &textRect);
	}

	HSPRITE textSprite_CT = gHUD.GetSprite(m_iCTTextIndex);

	if (textSprite_CT)
	{
		wrect_t textRect = gHUD.GetSpriteRect(m_iCTTextIndex);

		SPR_Set(textSprite_CT, 128, 128, 128);
		SPR_DrawAdditive(0, (ScreenWidth) / 2 + 27, bgY + 11, &textRect);
	}

	int aliveCT = gHUD.m_Scoreboard.m_iTeamAlive_CT;
	int aliveT = gHUD.m_Scoreboard.m_iTeamAlive_T;
	int textWidth_CTAlive = GetHudNumberWidth(m_iNum_S, m_rcNumber_Small, DHN_2DIGITS | DHN_DRAWZERO, aliveCT);
	int textWidth_TAlive = GetHudNumberWidth(m_iNum_S, m_rcNumber_Small, DHN_2DIGITS | DHN_DRAWZERO, aliveT);
	int scoreCT = gHUD.m_Scoreboard.m_iTeamScore_CT;
	int scoreT = gHUD.m_Scoreboard.m_iTeamScore_T;
	int scoreMax = gHUD.m_Scoreboard.m_iTeamScore_Max;
	int roundNumber = scoreMax ? scoreMax : scoreT + scoreCT + 1;
	

	if (gHUD.m_iModRunning == MOD_DM)
	{
		int best_player = gHUD.m_Scoreboard.FindBestPlayer();
		scoreCT = g_PlayerExtraInfo[gEngfuncs.GetLocalPlayer()->index].frags;
		scoreT = best_player ? g_PlayerExtraInfo[best_player].frags : 0;

		roundNumber = scoreMax ? scoreMax : 0;
	}

	if (roundNumber >= 1000)
	{
		int textWidth = GetHudNumberWidth(m_iNum_S, m_rcNumber_Small, DHN_4DIGITS | DHN_3DIGITS | DHN_2DIGITS | DHN_DRAWZERO, roundNumber);

		if (textWidth > 0)
			DrawHudNumber(m_iNum_S, m_rcNumber_Small, (ScreenWidth - textWidth) / 2, bgY + 10, DHN_4DIGITS | DHN_3DIGITS | DHN_2DIGITS | DHN_DRAWZERO, roundNumber, 128, 128, 128);
	}
	else if (roundNumber >= 1000)
	{
		int textWidth = GetHudNumberWidth(m_iNum_L, m_rcNumber_Large, DHN_3DIGITS | DHN_2DIGITS | DHN_DRAWZERO, roundNumber);

		if (textWidth > 0)
			DrawHudNumber(m_iNum_L, m_rcNumber_Large, (ScreenWidth - textWidth) / 2, bgY + 10, DHN_3DIGITS | DHN_2DIGITS | DHN_DRAWZERO, roundNumber, 128, 128, 128);
	}
	else
	{
		int textWidth = GetHudNumberWidth(m_iNum_L, m_rcNumber_Large, DHN_2DIGITS | DHN_DRAWZERO, roundNumber);

		if (textWidth > 0)
			DrawHudNumber(m_iNum_L, m_rcNumber_Large, (ScreenWidth - textWidth) / 2, bgY + 10, DHN_2DIGITS | DHN_DRAWZERO, roundNumber, 128, 128, 128);
	}

	if (scoreT >= 1000)
	{
		int textWidth_T = GetHudNumberWidth(m_iNum_S, m_rcNumber_Small, DHN_4DIGITS | DHN_3DIGITS | DHN_2DIGITS | DHN_DRAWZERO, scoreT);

		if (textWidth_T > 0)
			DrawHudNumber(m_iNum_S, m_rcNumber_Small, (ScreenWidth) / 2 - 90, bgY + 11, DHN_4DIGITS | DHN_3DIGITS | DHN_2DIGITS | DHN_DRAWZERO, scoreT, 128, 128, 128);
	}
	else if (scoreT >= 100)
	{
		int textWidth_T = GetHudNumberWidth(m_iNum_L, m_rcNumber_Large, DHN_3DIGITS | DHN_DRAWZERO, scoreT);

		if (textWidth_T > 0)
			DrawHudNumber(m_iNum_L, m_rcNumber_Large, (ScreenWidth) / 2 - 89, bgY + 10, DHN_3DIGITS | DHN_DRAWZERO, scoreT, 128, 128, 128);
	}
	else
	{
		int textWidth_T = GetHudNumberWidth(m_iNum_L, m_rcNumber_Large, DHN_2DIGITS | DHN_DRAWZERO, scoreT);

		if (textWidth_T > 0)
			DrawHudNumber(m_iNum_L, m_rcNumber_Large, (ScreenWidth) / 2 - 89, bgY + 10, DHN_2DIGITS | DHN_DRAWZERO, scoreT, 128, 128, 128);
	}

	if (scoreCT >= 1000)
	{
		int textWidth_CT = GetHudNumberWidth(m_iNum_S, m_rcNumber_Small, DHN_4DIGITS | DHN_3DIGITS | DHN_2DIGITS | DHN_DRAWZERO, scoreCT);

		if (textWidth_CT > 0)
			DrawHudNumber(m_iNum_S, m_rcNumber_Small, ((ScreenWidth) / 2) + 71 - (textWidth_CT / 2), bgY + 11, DHN_4DIGITS | DHN_3DIGITS | DHN_2DIGITS | DHN_DRAWZERO, scoreCT, 128, 128, 128);
	}
	else if (scoreCT >= 100)
	{
		int textWidth_CT = GetHudNumberWidth(m_iNum_L, m_rcNumber_Large, DHN_3DIGITS | DHN_2DIGITS | DHN_DRAWZERO, scoreCT);

		if (textWidth_CT > 0)
			DrawHudNumber(m_iNum_L, m_rcNumber_Large, ((ScreenWidth) / 2) + 70 - (textWidth_CT / 2), bgY + 10, DHN_3DIGITS | DHN_DRAWZERO, scoreCT, 128, 128, 128);
	}
	else
	{
		int textWidth_CT = GetHudNumberWidth(m_iNum_L, m_rcNumber_Large, DHN_2DIGITS | DHN_DRAWZERO, scoreCT);

		if (textWidth_CT > 0)
			DrawHudNumber(m_iNum_L, m_rcNumber_Large, ((ScreenWidth) / 2) + 73 - (textWidth_CT / 2), bgY + 10, DHN_2DIGITS | DHN_DRAWZERO, scoreCT, 128, 128, 128);
	}

	if (m_iBGIndex != m_iTeamDeathBG)
	{
		if (textWidth_TAlive > 0)
			DrawHudNumber(m_iNum_S, m_rcNumber_Small, (ScreenWidth) / 2 - 69, bgY + 30, DHN_2DIGITS | DHN_DRAWZERO, aliveT, 128, 128, 128);

		if (textWidth_CTAlive > 0)
			DrawHudNumber(m_iNum_S, m_rcNumber_Small, (ScreenWidth) / 2 + 47, bgY + 30, DHN_2DIGITS | DHN_DRAWZERO, aliveCT, 128, 128, 128);
	}

	return 1;
}

void CHudScoreBoardLegacy::Reset(void)
{
	m_iBGIndex = m_iOriginalBG;
	m_iTextIndex = m_iText_Round;
	m_iTTextIndex = m_iText_TR;
	m_iCTTextIndex = m_iText_CT;

	switch (gHUD.m_iModRunning)
	{
	case MOD_NONE:
	{
		m_iTextIndex = m_iText_Kill;
		break;
	}
	case MOD_TDM:
	{
		m_iBGIndex = m_iTeamDeathBG;
		m_iTextIndex = m_iText_Kill;
		break;
	}
	case MOD_DM:
	{
		m_iBGIndex = m_iTeamDeathBG;
		m_iTextIndex = m_iText_Kill;
		m_iTTextIndex = m_iText_1st;
		m_iCTTextIndex = m_iText_Kill;
		break;
	}
	case MOD_ZB3:
	case MOD_ZB2:
	case MOD_ZB1:
	{
		m_iTTextIndex = m_iText_ZB;
		m_iCTTextIndex = m_iText_HM;
		break;
	}
	case MOD_ZBS:
	{
		// already drawn in zbs/zbs_scoreboard.cpp
		m_iBGIndex = -1;
		break;
	}
	default:
	{
		// shut clang warnings
		break;
	}
	}
}


void CHudScoreBoardLegacy::BuildHudNumberRect(int moe, wrect_t *prc, int w, int h, int xOffset, int yOffset)
{
	wrect_t rc = gHUD.GetSpriteRect(moe);
	int x = rc.left;
	int y = rc.top;

	for (int i = 0; i < 10; i++)
	{
		prc[i].left = x;
		prc[i].top = 0;
		prc[i].right = prc[i].left + w + xOffset;
		prc[i].bottom = h + yOffset;

		x += w;
		y += h;
	}
}

int CHudScoreBoardLegacy::DrawHudNumber(int moe, wrect_t *prc, int x, int y, int iFlags, int iNumber, int r, int g, int b)
{
	int iWidth = prc[0].right - prc[0].left;
	int k;
	wrect_t rc;

	if (iNumber >= 10000)
	{
		k = iNumber / 10000;
		SPR_Set(gHUD.GetSprite(moe), r, g, b);
		SPR_DrawAdditive(0, x, y, &prc[k]);
		x += iWidth;
	}
	else if (iFlags & (DHN_5DIGITS))
	{
		if (iFlags & DHN_DRAWZERO)
		{
			SPR_Set(gHUD.GetSprite(moe), r, g, b);
			SPR_DrawAdditive(0, x, y, &prc[0]);
		}

		x += iWidth;
	}

	if (iNumber >= 1000)
	{
		k = (iNumber % 10000) / 1000;
		SPR_Set(gHUD.GetSprite(moe), r, g, b);
		SPR_DrawAdditive(0, x, y, &prc[k]);
		x += iWidth;
	}
	else if (iFlags & (DHN_5DIGITS | DHN_4DIGITS))
	{
		if (iFlags & DHN_DRAWZERO)
		{
			SPR_Set(gHUD.GetSprite(moe), r, g, b);
			SPR_DrawAdditive(0, x, y, &prc[0]);
		}

		x += iWidth;
	}

	if (iNumber >= 100)
	{
		k = (iNumber % 1000) / 100;
		SPR_Set(gHUD.GetSprite(moe), r, g, b);
		SPR_DrawAdditive(0, x, y, &prc[k]);
		x += iWidth;
	}
	else if (iFlags & (DHN_5DIGITS | DHN_4DIGITS | DHN_3DIGITS))
	{
		if (iFlags & DHN_DRAWZERO)
		{
			SPR_Set(gHUD.GetSprite(moe), r, g, b);
			SPR_DrawAdditive(0, x, y, &prc[0]);
		}

		x += iWidth;
	}

	if (iNumber >= 10)
	{
		k = (iNumber % 100) / 10;
		rc = prc[k];
		SPR_Set(gHUD.GetSprite(moe), r, g, b);
		SPR_DrawAdditive(0, x, y, &rc);
		x += iWidth;
	}
	else if (iFlags & (DHN_5DIGITS | DHN_4DIGITS | DHN_3DIGITS | DHN_2DIGITS))
	{
		if (iFlags & DHN_DRAWZERO)
		{
			SPR_Set(gHUD.GetSprite(moe), r, g, b);
			SPR_DrawAdditive(0, x, y, &prc[0]);
		}

		x += iWidth;
	}

	k = iNumber % 10;
	SPR_Set(gHUD.GetSprite(moe), r, g, b);
	SPR_DrawAdditive(0, x, y, &prc[k]);
	x += iWidth;

	return x;
}

int CHudScoreBoardLegacy::GetHudNumberWidth(int moe, wrect_t *prc, int iFlags, int iNumber)
{
	int iWidth = prc[0].right - prc[0].left;
	int k;
	wrect_t rc;
	int x = 0;

	if (iNumber >= 10000)
	{
		k = iNumber / 10000;
		x += iWidth;
	}
	else if (iFlags & (DHN_5DIGITS))
		x += iWidth;

	if (iNumber >= 1000)
	{
		k = (iNumber % 10000) / 1000;
		x += iWidth;
	}
	else if (iFlags & (DHN_5DIGITS | DHN_4DIGITS))
		x += iWidth;

	if (iNumber >= 100)
	{
		k = (iNumber % 1000) / 100;
		x += iWidth;
	}
	else if (iFlags & (DHN_5DIGITS | DHN_4DIGITS | DHN_3DIGITS))
		x += iWidth;

	if (iNumber >= 10)
	{
		k = (iNumber % 100) / 10;
		rc = prc[k];
		x += iWidth;
	}
	else if (iFlags & (DHN_5DIGITS | DHN_4DIGITS | DHN_3DIGITS | DHN_2DIGITS))
		x += iWidth;

	k = iNumber % 10;
	x += iWidth;

	return x;
}

