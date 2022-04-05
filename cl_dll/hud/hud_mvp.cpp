#include "hud.h"
#include "r_efx.h"
#include "cl_util.h"

#include "parsemsg.h"
#include "draw_util.h"
#include "hud_mvp.h"
#include "gamemode/mods_const.h"

namespace cl {
DECLARE_MESSAGE(m_MVP, MVPInfo)

int CHudMVP::Init()
{
	gHUD.AddHudElem(this);
	HOOK_MESSAGE(MVPInfo);
	InitHUDData();

	return 1;
}

bool CHudMVP::IsZombieMod()
{
	return gHUD.IsZombieMod();
}

void CHudMVP::InitHUDData()
{
	m_iRow = 0;
	m_iType = 0;
	m_iRoundTime = 0;
	m_flShowTime = 0.0f;
	memset(m_MVPData, 0, sizeof(m_MVPData));

	memset(m_szVictoryTitle, 0, sizeof(m_szVictoryTitle));
	memset(m_wszGameTime, 0, sizeof(m_wszGameTime));
	memset(m_wszTitle, 0, sizeof(m_wszTitle));

	strcpy(m_wszLabel[0], "杀敌数");
	strcpy(m_wszLabel[1], "助攻");
	strcpy(m_wszLabel[2], "感染");

	strcpy(m_szScoreType[0], "杀敌");
	strcpy(m_szScoreType[1], "助攻");
	strcpy(m_szScoreType[2], "感染");
}

int CHudMVP::MsgFunc_MVPInfo(const char* pszName, int iSize, void* pbuf)
{
	BufferReader reader(pszName, pbuf, iSize);

	// 1-T & Zombie ; 2-CT & Human
	int iWinType = reader.ReadByte();

	// bomb planter & defuser
	int bombhandler = 0;
	if (iWinType == 3 || iWinType == 4)
		bombhandler = reader.ReadByte();

	int i;
	// kills
	int num = reader.ReadByte();
	int index, data;
	for (i = 0; i < num; i++)
	{
		index = reader.ReadByte();
		data = reader.ReadByte();
		SetContent(index, data, i, 0);
	}
	// assistace
	num = reader.ReadByte();
	for (i = 0; i < num; i++)
	{
		index = reader.ReadByte();
		data = reader.ReadByte();
		SetContent(index, data, i, 1);
	}

	m_iRow = 2;
	if (IsZombieMod())
	{
		// infections
		num = reader.ReadByte();
		for (i = 0; i < num; i++)
		{
			index = reader.ReadByte();
			data = reader.ReadByte();
			SetContent(index, data, i, 2);
		}
		m_iRow = 3;
	}
	m_iRoundTime = reader.ReadShort();

	UpdateData(iWinType, bombhandler);

	return 1;
}

void CHudMVP::UpdateData(int type, int bombhandler)
{
	m_iFlags |= HUD_DRAW;
	m_flShowTime = gHUD.m_flTime + 5.0f;
	m_iType = type;

	memset(m_szVictoryTitle, 0, sizeof(m_szVictoryTitle));
	memset(m_wszGameTime, 0, sizeof(m_wszGameTime));
	memset(m_wszTitle, 0, sizeof(m_wszTitle));

	switch (m_iType)
	{
	case 1:
	case 3:
		if (!IsZombieMod())
		{
			if (bombhandler)
				sprintf(m_wszTitle, "玩家%s为本场的拆弹专家!", g_PlayerInfoList[bombhandler].name);
			sprintf(m_szVictoryTitle, "T阵营 胜利");
		}
		else
			sprintf(m_szVictoryTitle, "僵尸胜利");
		break;
	case 2:
	case 4:
		if (!IsZombieMod())
		{
			if (bombhandler)
				sprintf(m_wszTitle, "玩家%s为本场的爆破高手!", g_PlayerInfoList[bombhandler].name);
			sprintf(m_szVictoryTitle, "CT阵营 胜利");
		}
		else
			sprintf(m_szVictoryTitle, "人类胜利");
		break;
	default:
		break;
	}

	snprintf(m_wszGameTime, sizeof(m_wszGameTime), "当前关卡进行时间： %d:%02d", m_iRoundTime / 60, m_iRoundTime % 60);

	if (!bombhandler)
	{
		bool bIsZbWin = IsZombieMod() && (m_iType == 1);
		if (m_MVPData[0][0].iIndex && m_MVPData[0][0].iData > 0 && m_MVPData[0][0].iData >= m_MVPData[1][0].iData && m_MVPData[0][0].iData >= m_MVPData[2][0].iData)
			snprintf(m_wszTitle, sizeof(m_wszGameTime), "玩家%s为本场的杀敌王牌", g_PlayerInfoList[m_MVPData[0][0].iIndex].name);
		else if (m_MVPData[1][0].iIndex && m_MVPData[1][0].iData > 0 && m_MVPData[1][0].iData >= m_MVPData[2][0].iData)
			snprintf(m_wszTitle, sizeof(m_wszGameTime), "玩家%s为本场的助攻王牌", g_PlayerInfoList[m_MVPData[1][0].iIndex].name);
		else if (bIsZbWin && m_MVPData[2][0].iIndex && m_MVPData[2][0].iData > 0)
			snprintf(m_wszTitle, sizeof(m_wszGameTime), "玩家%s为本场的僵尸王牌!", g_PlayerInfoList[m_MVPData[2][0].iIndex].name);
	}

	if (IsZombieMod())
		strcpy(m_wszLabel[0], "击毙僵尸");
	else
		strcpy(m_wszLabel[0], "杀敌数");
}

int CHudMVP::VidInit()
{
	R_InitTexture(m_pPopupboardBG, "resource/pvp2nd/pvp2_combatpopup_board");
	R_InitTexture(m_pPopupboard, "resource/pvp2nd/pvp2_combatpopup_logobg");
	R_InitTexture(m_pTopBlue, "resource/pvp2nd/pvp2_combatpopup_top_blue");
	R_InitTexture(m_pTopRed, "resource/pvp2nd/pvp2_combatpopup_top_red");

	return 1;
}

int CHudMVP::Draw(float flTime)
{
	if (m_flShowTime <= flTime)
	{
		m_iFlags &= ~HUD_DRAW;
		return 0;
	}

	auto pTexture = (m_iType & 1) ? m_pTopRed : m_pTopBlue;
	int iWidthTop = pTexture->w();
	int iHeightTop = pTexture->h();

	int iStartY = ScreenHeight / 8;
	int iX = ScreenWidth / 2 - iWidthTop / 2;
	int iY = iStartY + iHeightTop;

	m_pPopupboardBG->Draw2DQuadScaled(iX, iY - iHeightTop, iX + iWidthTop, iY + iHeightTop * m_iRow - 45);
	m_pPopupboardBG->Draw2DQuadScaled(iX, iY - iHeightTop, iX + iWidthTop, iY + iHeightTop * m_iRow - 45);

	for (int i = 0; i < m_iRow; i++)
	{
		m_pPopupboardBG->Draw2DQuadScaled(iX + 20, iY, iX + iWidthTop - 40, iY + 100);
		iY += 122;
	}

	iY = iStartY;
	pTexture->Draw2DQuadScaled(iX, iY, iX + iWidthTop, iY + iHeightTop);

	iY = iStartY + 5;

	int offsetY = -10;
	int r = 255, g = 255, b = 255;
	if (m_szVictoryTitle[0])
	{
		if (m_iType & 1)
			r = 205, g = 130, b = 125;
		else
			r = 140, g = 180, b = 220;

		//g_FontBold.SetWidth(25);
		DrawUtils::DrawHudString(iX + 20, iY + 25, ScreenWidth, m_szVictoryTitle, r, g, b);
	}

	iY += 30;

	//g_FontBold.SetWidth(18);
	if (m_iType & 1)
		r = 160, g = 90, b = 85;
	else
		r = 80, g = 120, b = 160;

	if (m_wszTitle[0])
		DrawUtils::DrawHudString(iX + 20, iY + 30, ScreenWidth, m_wszTitle, r, g, b);

	if (m_wszGameTime[0])
		DrawUtils::DrawHudString(iX + iWidthTop - 20 - 160, iY + 30, ScreenWidth, m_wszGameTime, r, g, b);

	iY = iStartY + iHeightTop;

	for (int i = 0; i < m_iRow; i++)
	{
		iX = ScreenWidth / 2 - iWidthTop / 2 + 35;

		//g_FontBold.SetWidth(18);
		DrawUtils::DrawHudString(iX, iY - 8 + offsetY, ScreenWidth, m_wszLabel[i], 255, 255, 255);

		for (int j = 0; j < 5; j++)
		{
			int index = m_MVPData[i][j].iIndex;
			if (!index)
				continue;

			//g_FontBold.SetWidth(15);
			if (g_PlayerExtraInfo[index].teamnumber == 1)
				r = 205, g = 130, b = 125;
			else
				r = 140, g = 180, b = 220;

			if (gEngfuncs.GetLocalPlayer()->index == index)
				m_pPopupboard->Draw2DQuadScaled(iX - 5, iY + 5 + 19 * j, iX + iWidthTop - 85, iY + 5 + 19 * j + 18);

			DrawUtils::DrawHudString(iX, iY + 16 + offsetY + 19 * j, 250, g_PlayerInfoList[index].name, r, g, b);

			static char szScore[32];
			sprintf(szScore, "%d %s", m_MVPData[i][j].iData, m_szScoreType[i]);
			DrawUtils::DrawHudString(ScreenWidth / 2 + iWidthTop / 2 - 35 - 70, iY + 16 + offsetY + 19 * j, 250, szScore, r, g, b);
		}
		iY += 122;
	}

	return 1;
}

void CHudMVP::Shutdown()
{
	m_pPopupboardBG = nullptr;
	m_pPopupboard = nullptr;
	m_pTopBlue = nullptr;
	m_pTopRed = nullptr;
}
}