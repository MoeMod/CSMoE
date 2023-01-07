/*
Scoreboard.cpp - CSMoE Client HUD : Scoreboard
Copyright (C) 2019 Moemod Yanase

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "com_weapons.h"
#include "cdll_dll.h"

#include <string.h>
#include <stdio.h>
#include "draw_util.h"
#include <algorithm>
#include <numeric>
#include <tuple>

#include "hud_sub_impl.h"
#include "gamemode/mods_const.h"

#include "legacy/hud_scoreboard_legacy.h"
#ifdef XASH_IMGUI
#include "imgui.h"
#include "imgui_utils.h"
#endif

namespace cl {

hud_player_info_t   g_PlayerInfoList[MAX_PLAYERS + 1]; // player info from the engine
extra_player_info_t	g_PlayerExtraInfo[MAX_PLAYERS + 1]; // additional player info sent directly to the client dll
RoundPlayerInfo     g_PlayerExtraInfoEx[MAX_PLAYERS + 1];
team_info_t         g_TeamInfo[MAX_TEAMS + 1];
hostage_info_t      g_HostageInfo[MAX_HOSTAGES + 1];
int g_iUser1;
int g_iUser2;
int g_iUser3;
int g_iTeamNumber;

inline bool IsConnected(int playerIndex)
{
	return (g_PlayerInfoList[playerIndex].name && g_PlayerInfoList[playerIndex].name[0] != 0);
}

inline int GetTeamCounts(short teamnumber)
{
	int count = 0;

	for (int i = 1; i <= MAX_PLAYERS; i++)
	{
		GetPlayerInfo(i, &g_PlayerInfoList[i]);
		if (!IsConnected(i))
			continue;

		if (g_PlayerExtraInfo[i].teamnumber == teamnumber)
			count++;
	}

	return count;
}

inline int GetTeamAliveCounts(short teamnumber)
{
	int count = 0;

	for (int i = 1; i <= MAX_PLAYERS; i++)
	{
		GetPlayerInfo(i, &g_PlayerInfoList[i]);
		if (!IsConnected(i))
			continue;

		if (g_PlayerExtraInfo[i].teamnumber == teamnumber && g_PlayerExtraInfo[i].dead == false)
			count++;
	}

	return count;
}

// X positions

//#include "vgui_TeamFortressViewport.h"

DECLARE_COMMAND(m_Scoreboard, ShowScores)
DECLARE_COMMAND(m_Scoreboard, HideScores)
DECLARE_COMMAND(m_Scoreboard, ShowScoreboard2)
DECLARE_COMMAND(m_Scoreboard, HideScoreboard2)

DECLARE_MESSAGE(m_Scoreboard, ScoreInfo)
DECLARE_MESSAGE(m_Scoreboard, TeamInfo)
DECLARE_MESSAGE(m_Scoreboard, TeamScore)

struct CHudScoreboard::impl_t : THudSubDispatcher<CHudScoreBoardLegacy> {};

CHudScoreboard::CHudScoreboard() = default;
CHudScoreboard::~CHudScoreboard() = default;

int CHudScoreboard::Init(void)
{
	pimpl.reset(new impl_t());

	gHUD.AddHudElem(this);

	// Hook messages & commands here
	HOOK_COMMAND("+showscores", ShowScores);
	HOOK_COMMAND("-showscores", HideScores);
	HOOK_COMMAND("showscoreboard2", ShowScoreboard2);
	HOOK_COMMAND("hidescoreboard2", HideScoreboard2);

	HOOK_MESSAGE(ScoreInfo);
	HOOK_MESSAGE(TeamScore);
	HOOK_MESSAGE(TeamInfo);

	InitHUDData();

	cl_showpacketloss = CVAR_CREATE("cl_showpacketloss", "0", FCVAR_ARCHIVE);

	return 1;
}

int CHudScoreboard::VidInit(void)
{
	m_iFlags |= HUD_ACTIVE;

	m_bForceDraw = false;

	// Load sprites here
	pimpl->for_each(&IBaseHudSub::VidInit);

	if (!m_pOSTexture[0])
		R_InitTexture(m_pOSTexture[0], "resource/basic/windows");
	if (!m_pOSTexture[1])
		R_InitTexture(m_pOSTexture[1], "resource/basic/apple");
	if (!m_pOSTexture[2])
		R_InitTexture(m_pOSTexture[2], "resource/basic/android");

	return 1;
}

void CHudScoreboard::InitHUDData(void)
{
	memset(g_PlayerExtraInfo, 0, sizeof g_PlayerExtraInfo);
	m_iLastKilledBy = 0;
	m_fLastKillTime = 0;
	m_iPlayerNum = 0;
	m_iNumTeams = 0;
	memset(g_TeamInfo, 0, sizeof g_TeamInfo);

	m_iFlags &= ~HUD_DRAW;  // starts out inactive

	m_iFlags |= HUD_INTERMISSION; // is always drawn during an intermission

	m_iTeamScore_Max = 0;
	m_iTeamScore_T = 0;
	m_iTeamScore_CT = 0;
	m_iTeamAlive_T = 0;
	m_iTeamAlive_CT = 0;
	m_flNextCache = 0;

	pimpl->for_each(&IBaseHudSub::InitHUDData);
}

void CHudScoreboard::Shutdown(void)
{
	std::fill(std::begin(m_pOSTexture), std::end(m_pOSTexture), nullptr);
	pimpl = nullptr;
}

int CHudScoreboard::Draw(float time)
{
	pimpl->for_each(&IBaseHudSub::Draw, time);

	if (!m_bForceDraw)
	{
		if ((!m_bShowscoresHeld && gHUD.m_Health.m_iHealth > 0 && !gHUD.m_iIntermission))
			return 1;
		else
		{
			m_colors.r = 0;
			m_colors.g = 0;
			m_colors.b = 0;
			m_colors.a = 153;
			m_bDrawStroke = true;
		}
	}

	return DrawScoreboard(time);
}

void CHudScoreboard::Think()
{
	if (gHUD.m_flTime > m_flNextCache)
	{
		CacheTeamAliveNumber();
		m_flNextCache = gHUD.m_flTime + 1.0;
	}
	pimpl->for_each(&IBaseHudSub::Think);
}

void CHudScoreboard::Reset()
{
	pimpl->for_each(&IBaseHudSub::Reset);
}

const char *GetGameModeName()
{
	switch (gHUD.m_iModRunning)
	{
	case MOD_NONE	: return "Original";
	case MOD_DM		: return "DeathMatch";
	case MOD_TDM	: return "Team DeathMatch";
	case MOD_ZB1	: return "Zombie Mod 1";
	case MOD_ZB2	: return "Zombie Mod 2";
	case MOD_ZBU	: return "Zombie United";
	case MOD_ZB3	: return "Zombie Mod 3";
	case MOD_ZBS	: return "Zombie Scenario";
	case MOD_ZE		: return "Zombie Escape";
	case MOD_ZB4	: return "Zombie Darkness";
	case MOD_GD		: return "GunDeath Match";
	case MOD_ZBB	: return "Zombie BaseBuilder";
	case MOD_ZBZ	: return "Zombie Z";
	default			: break;
	}
	return "Unknown";
}

int CHudScoreboard::DrawScoreboard(float fTime)
{
	GetAllPlayersInfo();
	char szTitle[90];

	const int iStartX = (float)ScreenHeight / (float)ScreenWidth < 0.75 ? 100 * (ScreenHeight / 768.0) + 4 : 4;
	const int iStartY = 40 * (ScreenHeight / 768.0);
	const int iStartW = ScreenWidth - 2 * iStartX + 4;
	const int iStartH = ScreenHeight - 2 * iStartY + 25;
	DrawUtils::DrawRectangle(iStartX, iStartY, iStartW, iStartH, m_colors.r, m_colors.g, m_colors.b, m_colors.a, m_bDrawStroke);

	if (gHUD.m_szServerName[0])
		snprintf(szTitle, 80, "%s (%s)", gHUD.m_szServerName, GetGameModeName());
	else
		snprintf(szTitle, 80, "CSMoE Server (%s)", GetGameModeName());

	const float flScale = 1.6f;
	const auto iCenterX = ScreenWidth / 2;
	const auto iTextLen = DrawUtils::HudStringLen(szTitle, flScale);
	DrawUtils::DrawHudString(iCenterX - iTextLen / 2, iStartY + 35, 1000, szTitle, 255, 255, 255, flScale);

	if ((gHUD.m_iModRunning == MOD_ZB1 || gHUD.m_iModRunning == MOD_ZB2 || gHUD.m_iModRunning == MOD_ZB3 || gHUD.m_iModRunning == MOD_ZB4 || gHUD.m_iModRunning == MOD_ZBB || gHUD.m_iModRunning == MOD_ZE || gHUD.m_iModRunning == MOD_DM || gHUD.m_iModRunning == MOD_ZBS || gHUD.m_iModRunning == MOD_ZBZ))
	{
		DrawScoreNew(false);
	}
	else
	{
		DrawScoreNew(true);
	}
	return 1;
}

void CHudScoreboard::DrawScoreNew(bool bDivideTeam)
{
	int iLocalIndex = gEngfuncs.GetLocalPlayer()->index;
	const int iCharHeightOffset = 0;
	const int iStartX = (float)ScreenHeight / (float)ScreenWidth < 0.75 ? 100 * (ScreenHeight / 768.0) + 4 : 4;
	const int iStartY = 40 * (ScreenHeight / 768.0);
	const int iStartW = ScreenWidth - 2 * iStartX + 4;
	const int iStartH = ScreenHeight - 2 * iStartY + 25;

	size_t SortedId[MAX_PLAYERS + 1];
	std::iota(std::begin(SortedId), std::end(SortedId), 0);
	std::sort(std::begin(SortedId), std::end(SortedId), [](size_t a, size_t b) {
		return std::make_pair(g_PlayerExtraInfo[a].frags, -g_PlayerExtraInfo[a].deaths) > std::make_pair(g_PlayerExtraInfo[b].frags, -g_PlayerExtraInfo[b].deaths);
	});

	int r = 255, g = 255, b = 255, a = 255;
	const bool bIsZombieMode = gHUD.m_iModRunning == MOD_ZB1 || gHUD.m_iModRunning == MOD_ZB2 || gHUD.m_iModRunning == MOD_ZB3 || gHUD.m_iModRunning == MOD_ZB4 || gHUD.m_iModRunning == MOD_ZBB || gHUD.m_iModRunning == MOD_ZBZ;

	for (int iColumn = 1; iColumn <= 2; iColumn++)
	{
		int iDraw = 0;

		int x = iStartX + (iColumn - 1) * ((iStartW / 2) - 12);
		int y = iStartY;

		int iTotalScore = 0, iTotalDeath = 0, iTotalPing = 0, iPingDivider = 0, iPlayerCount = 0;

		char szBuf[128];
		const float flScale = 0.0f;

		for (int i = 0; i < MAX_PLAYERS + 1; i++)
		{
			const int id = SortedId[i];

			if (!IsConnected(id))
				continue;

			if (g_PlayerExtraInfo[id].teamnumber != TEAM_CT && g_PlayerExtraInfo[id].teamnumber != TEAM_TERRORIST)
				continue;

			if (bDivideTeam && g_PlayerExtraInfo[id].teamnumber != (iColumn == 1 ? TEAM_TERRORIST : TEAM_CT))
				continue;

			if (!bDivideTeam && ((i & 1) != (iColumn - 1)))
				continue;

			if (id == iLocalIndex)
			{
				std::tie(r, g, b, a) = std::make_tuple(255, 255, 255, 255);
			}
			else if (g_PlayerExtraInfo[id].teamnumber == TEAM_CT)
			{
				std::tie(r, g, b, a) = std::make_tuple(173, 201, 235, 255);
			}
			else if (g_PlayerExtraInfo[id].teamnumber == TEAM_TERRORIST)
			{
				std::tie(r, g, b, a) = std::make_tuple(216, 81, 80, 255);
			}
			else
			{
				std::tie(r, g, b, a) = std::make_tuple(255, 255, 255, 255);
			}

			iDraw++;

			int offsetY = 120 + 21 * iDraw + iCharHeightOffset - 6;

			//if (m_bHostOwnBuff)
			//	GL_DrawTGA2(g_Texture[m_iBuffIcon].iTexture, x + 80, y + offsetY - 14, g_Texture[m_iBuffIcon].iWidth, g_Texture[m_iBuffIcon].iHeight, 255);
			switch (g_PlayerExtraInfo[id].os)
			{
			case OS_Windows:
				m_pOSTexture[0]->Draw2DQuadScaled(x + 105 - 25, y + offsetY, x + 105 - 10, y + offsetY + 15);
				break;
			case OS_Apple:
				m_pOSTexture[1]->Draw2DQuadScaled(x + 105 - 25, y + offsetY, x + 105 - 10, y + offsetY + 15);
				break;
			case OS_Android:
				m_pOSTexture[2]->Draw2DQuadScaled(x + 105 - 25, y + offsetY, x + 105 - 10, y + offsetY + 15);
				break;;
			default:
				break;
			}
			DrawUtils::DrawHudString(x + 105, y + offsetY, 1000, g_PlayerInfoList[id].name, r, g, b, flScale);

			sprintf(szBuf, "%d", g_PlayerExtraInfo[id].frags);
			DrawUtils::DrawHudStringReverse(x + (iStartW / 2) - 5 - 145, y + offsetY, 0, szBuf, r, g, b, flScale);

			sprintf(szBuf, "%d", g_PlayerExtraInfo[id].deaths);
			DrawUtils::DrawHudStringReverse(x + (iStartW / 2) - 5 - 75, y + offsetY, 0, szBuf, r, g, b, flScale);


			if (g_PlayerExtraInfo[id].dead)
			{
				sprintf(szBuf, "死亡");
				DrawUtils::DrawHudStringReverse(x + (iStartW / 2) - 5 - 220, y + offsetY, 0, szBuf, r, g, b, flScale);
			}
			else if (g_PlayerExtraInfo[id].has_c4)
			{
				sprintf(szBuf, "炸弹");
				DrawUtils::DrawHudStringReverse(x + (iStartW / 2) - 5 - 220, y + offsetY, 0, szBuf, r, g, b, flScale);
			}
			else if (g_PlayerExtraInfo[id].vip)
			{
				if(gHUD.m_iModRunning == MOD_ZB3 || gHUD.m_iModRunning == MOD_ZBZ)
					sprintf(szBuf, "英雄");
				else
					sprintf(szBuf, "VIP");
				DrawUtils::DrawHudStringReverse(x + (iStartW / 2) - 5 - 220, y + offsetY, 0, szBuf, r, g, b, flScale);
			}
			else if (g_PlayerExtraInfo[id].zombie)
			{
				sprintf(szBuf, "僵尸");
				DrawUtils::DrawHudStringReverse(x + (iStartW / 2) - 5 - 220, y + offsetY, 0, szBuf, r, g, b, flScale);
			}

			if (g_PlayerInfoList[id].ping == 0)
			{
				const char *isBotString = gEngfuncs.PlayerInfo_ValueForKey(id, "*bot");
				if (isBotString && atoi(isBotString) > 0)
					sprintf(szBuf, "BOT");
				else
					sprintf(szBuf, "HOST");
			}
			else
			{
				sprintf(szBuf, "%d", g_PlayerInfoList[id].ping);
			}
			DrawUtils::DrawHudStringReverse(x + (iStartW / 2) - 5, y + offsetY, 0, szBuf, r, g, b, flScale);

			iTotalScore += g_PlayerExtraInfo[id].frags;
			iTotalDeath += g_PlayerExtraInfo[id].deaths;
			iTotalPing += g_PlayerInfoList[id].ping;
			if (g_PlayerInfoList[id].ping)
				iPingDivider++;
			iPlayerCount++;
		}


		std::tie(r, g, b, a) = std::make_tuple(255, 188, 0, 255);
		sprintf(szBuf, "杀敌");
		DrawUtils::DrawHudStringReverse(x + (iStartW / 2) - 5 - 145, y + 80 + iCharHeightOffset, 0, szBuf, r, g, b, flScale);
		sprintf(szBuf, "死亡");
		DrawUtils::DrawHudStringReverse(x + (iStartW / 2) - 5 - 75, y + 80 + iCharHeightOffset, 0, szBuf, r, g, b, flScale);
		sprintf(szBuf, "延迟");
		DrawUtils::DrawHudStringReverse(x + (iStartW / 2) - 5 - 5, y + 80 + iCharHeightOffset, 0, szBuf, r, g, b, flScale);

		gEngfuncs.pfnFillRGBA(x + 9, y + 96, (iStartW / 2) - 16, 1, 172, 104, 0, 255);

		if (bDivideTeam && iPlayerCount)
		{
			const bool bIsZombieMode = gHUD.m_iModRunning == MOD_ZB1 || gHUD.m_iModRunning == MOD_ZB2 || gHUD.m_iModRunning == MOD_ZB3 || gHUD.m_iModRunning == MOD_ZB4 || gHUD.m_iModRunning == MOD_ZBB || gHUD.m_iModRunning == MOD_ZBZ;
			if (iColumn == 2)
				sprintf(szBuf, "%s  (%d)", bIsZombieMode ? "人类" : "CT", iPlayerCount);
			else
				sprintf(szBuf, "%s  (%d)", bIsZombieMode ? "僵尸" : "TR", iPlayerCount);

			DrawUtils::DrawHudString(x + 105, y + 110 + iCharHeightOffset, 1000, szBuf, r, g, b, flScale);

			gEngfuncs.pfnFillRGBA(x + 10, y + 125, (iStartW / 2) - 16, 1, 188, 112, 0, 255);

			sprintf(szBuf, "%d", iTotalScore / iPlayerCount);
			DrawUtils::DrawHudStringReverse(x + (iStartW / 2) - 5 - 145, y + 110 + iCharHeightOffset, 0, szBuf, r, g, b, flScale);

			sprintf(szBuf, "%d", iTotalDeath / iPlayerCount);
			DrawUtils::DrawHudStringReverse(x + (iStartW / 2) - 5 - 75, y + 110 + iCharHeightOffset, 0, szBuf, r, g, b, flScale);

			if (iPingDivider)
			{
				sprintf(szBuf, "%d", iTotalPing / iPingDivider);
				DrawUtils::DrawHudStringReverse(x + (iStartW / 2) - 5 - 0, y + 110 + iCharHeightOffset, 0, szBuf, r, g, b, flScale);

			}

		}
		else if (!bDivideTeam && iPlayerCount)
		{
			if (iColumn == 2)
				sprintf(szBuf, "%s  (%d)", bIsZombieMode ? "人类" : "CT", g_PlayerExtraInfo[SortedId[0]].frags);
			else
				sprintf(szBuf, "%s  (%d)", bIsZombieMode ? "僵尸" : "TR", g_PlayerExtraInfo[SortedId[0]].frags);

			DrawUtils::DrawHudString(x + 105, y + 110 + iCharHeightOffset, 1000, szBuf, r, g, b, flScale);

			sprintf(szBuf, "%d (%s)", g_TeamInfo[iColumn == 1 ? 1 : 0].frags, "Rounds");
			DrawUtils::DrawHudStringReverse(x + (iStartW / 2) - 5 - 2, y + 110 + iCharHeightOffset, 0, szBuf, r, g, b, flScale);

			gEngfuncs.pfnFillRGBA(x + 10, y + 125, (iStartW / 2) - 15, 1, 188, 112, 0, 255);
		}


		if (iColumn == 1)
		{
			std::tie(r, g, b, a) = std::make_tuple(255, 188, 0, 255);
			sprintf(szBuf, "Contributors");
			DrawUtils::DrawHudString(x + 29, y + iStartH + iCharHeightOffset - 229, 1000, szBuf, r, g, b, flScale);

			gEngfuncs.pfnFillRGBA(x + 19, y + iStartH - 212, (iStartW / 2) - 49, 1, 188, 112, 0, 255);
			std::tie(r, g, b, a) = std::make_tuple(188, 112, 0, 255);
			for (int i = 0; i < 3; i++)
			{
				static constexpr const char *szLeaderNames[] = { "CSMoE Team", "BTE Team", "Xash3D FWGS" };
				sprintf(szBuf, "%d. %s", i + 1, szLeaderNames[i]);
				DrawUtils::DrawHudString(x + 34, y + iStartH - 194 + 25 * i, 1000, szBuf, r, g, b, flScale);
			}
		}
		else
		{
			iPlayerCount = 0;
			for (int id = 1; id <= 32; id++)
			{
				if (!g_PlayerInfoList[id].name)
					continue;
				if (g_PlayerExtraInfo[id].teamnumber == TEAM_TERRORIST || g_PlayerExtraInfo[id].teamnumber == TEAM_CT)
					continue;

				iPlayerCount++;
				std::tie(r, g, b, a) = std::make_tuple(255, 255, 255, 255);

				int offsetY = iStartH - 213 + 21 * iPlayerCount;
				DrawUtils::DrawHudString(x + 35, y + offsetY, 1000, g_PlayerInfoList[id].name, r, g, b, flScale);
			}
			if (iPlayerCount)
			{
				std::tie(r, g, b, a) = std::make_tuple(255, 188, 0, 255);
				DrawUtils::DrawHudString(x + 35, y + iStartH - 212, (iStartW / 2) - 49 - (x + 35), "观察者", r, g, b, flScale);
				gEngfuncs.pfnFillRGBA(x + 19, y + iStartH - 212, (iStartW / 2) - 49, 1, 188, 112, 0, 255);
			}
		}
	}
}

int CHudScoreboard::FindBestPlayer(const char *team)
{
	int best_player = 0;
	int highest_frags = -99999;	int lowest_deaths = 99999;
	for (int i = 1; i < MAX_PLAYERS; i++)
	{
		if (g_PlayerInfoList[i].name && g_PlayerExtraInfo[i].frags >= highest_frags)
		{
			if (!team || !stricmp(g_PlayerExtraInfo[i].teamname, team))  // make sure it is the specified team
			{
				extra_player_info_t *pl_info = &g_PlayerExtraInfo[i];
				if (pl_info->frags > highest_frags || pl_info->deaths < lowest_deaths)
				{
					best_player = i;
					lowest_deaths = pl_info->deaths;
					highest_frags = pl_info->frags;
				}
			}
		}
	}
	return best_player;
}

void CHudScoreboard::GetAllPlayersInfo(void)
{
	for (int i = 1; i < MAX_PLAYERS; i++)
	{
		GetPlayerInfo(i, &g_PlayerInfoList[i]);

		if (g_PlayerInfoList[i].thisplayer)
			m_iPlayerNum = i;  // !!!HACK: this should be initialized elsewhere... maybe gotten from the engine
	}
}

int CHudScoreboard::MsgFunc_ScoreInfo(const char *pszName, int iSize, void *pbuf)
{
	m_iFlags |= HUD_DRAW;

	BufferReader reader(pszName, pbuf, iSize);
	short cl = reader.ReadByte();
	short frags = reader.ReadShort();
	short deaths = reader.ReadShort();
	short playerclass = reader.ReadShort();
	short teamnumber = reader.ReadShort();

	if (cl > 0 && cl <= MAX_PLAYERS)
	{
		g_PlayerExtraInfo[cl].frags = frags;
		g_PlayerExtraInfo[cl].deaths = deaths;
		g_PlayerExtraInfo[cl].playerclass = playerclass;
		g_PlayerExtraInfo[cl].teamnumber = teamnumber;

		//gViewPort->UpdateOnPlayerInfo();
	}

	return 1;
}

// Message handler for TeamInfo message
// accepts two values:
//		byte: client number
//		string: client team name
int CHudScoreboard::MsgFunc_TeamInfo(const char *pszName, int iSize, void *pbuf)
{
	BufferReader reader(pszName, pbuf, iSize);
	short cl = reader.ReadByte();
	int teamNumber = 0;

	if (cl > 0 && cl <= MAX_PLAYERS)
	{
		// set the players team
		char teamName[MAX_TEAM_NAME];
		strncpy(teamName, reader.ReadString(), MAX_TEAM_NAME);

		if (!strcmp(teamName, "TERRORIST"))
			teamNumber = TEAM_TERRORIST;
		else if (!strcmp(teamName, "CT"))
			teamNumber = TEAM_CT;
		else if (!strcmp(teamName, "SPECTATOR") || !strcmp(teamName, "UNASSIGNED"))
		{
			teamNumber = TEAM_SPECTATOR;
			strncpy(teamName, "SPECTATOR", MAX_TEAM_NAME);
		}
		// just in case
		else teamNumber = TEAM_UNASSIGNED;

		strncpy(g_PlayerExtraInfo[cl].teamname, teamName, MAX_TEAM_NAME);
		g_PlayerExtraInfo[cl].teamnumber = teamNumber;
	}

	// rebuild the list of teams

	// clear out player counts from teams
	for (int i = 1; i <= m_iNumTeams; i++)
	{
		g_TeamInfo[i].players = 0;
	}

	// rebuild the team list
	GetAllPlayersInfo();
	m_iNumTeams = 0;

	for (int i = 1; i < MAX_PLAYERS; i++)
	{
		int j;
		//if ( g_PlayerInfoList[i].name == NULL )
		//	continue;

		if (g_PlayerExtraInfo[i].teamname[0] == 0)
			continue; // skip over players who are not in a team

		// is this player in an existing team?
		for (j = 1; j <= m_iNumTeams; j++)
		{
			if (g_TeamInfo[j].name[0] == '\0')
				break;

			if (!stricmp(g_PlayerExtraInfo[i].teamname, g_TeamInfo[j].name))
				break;
		}

		if (j > m_iNumTeams)
		{
			// they aren't in a listed team, so make a new one
			for (j = 1; j <= m_iNumTeams; j++)
			{
				if (g_TeamInfo[j].name[0] == '\0')
					break;
			}


			m_iNumTeams = max(j, m_iNumTeams);

			strncpy(g_TeamInfo[j].name, g_PlayerExtraInfo[i].teamname, MAX_TEAM_NAME);
			g_TeamInfo[j].teamnumber = g_PlayerExtraInfo[i].teamnumber;
			g_TeamInfo[j].players = 0;
		}

		g_TeamInfo[j].players++;
	}

	// clear out any empty teams
	for (int i = 1; i <= m_iNumTeams; i++)
	{
		if (g_TeamInfo[i].players < 1)
			memset(&g_TeamInfo[i], 0, sizeof(team_info_t));
	}

	return 1;
}

// Message handler for TeamScore message
// accepts three values:
//		string: team name
//		short: teams kills
//		short: teams deaths 
// if this message is never received, then scores will simply be the combined totals of the players.
int CHudScoreboard::MsgFunc_TeamScore(const char *pszName, int iSize, void *pbuf)
{
	BufferReader reader(pszName, pbuf, iSize);
	char *TeamName = reader.ReadString();
	int i;

	// find the team matching the name
	for (i = 1; i <= m_iNumTeams; i++)
	{
		if (!stricmp(TeamName, g_TeamInfo[i].name))
			break;
	}
	if (i > m_iNumTeams)
		return 1;

	// use this new score data instead of combined player scores
	g_TeamInfo[i].scores_overriden = TRUE;
	g_TeamInfo[i].frags = reader.ReadShort();
	g_TeamInfo[i].deaths = reader.ReadShort();

	if (TeamName[0] == 'T')
	{
		m_iTeamScore_T = g_TeamInfo[i].frags;
	}
	else if (TeamName[0] == 'C')
	{
		m_iTeamScore_CT = g_TeamInfo[i].frags;
	}

	return 1;
}

void CHudScoreboard::DeathMsg(int killer, int victim)
{
	// if we were the one killed,  or the world killed us, set the scoreboard to indicate suicide
	if (victim == m_iPlayerNum || killer == 0)
	{
		m_iLastKilledBy = killer ? killer : m_iPlayerNum;
		m_fLastKillTime = gHUD.m_flTime + 10;	// display who we were killed by for 10 seconds

		if (killer == m_iPlayerNum)
			m_iLastKilledBy = m_iPlayerNum;
	}
}

void CHudScoreboard::UserCmd_ShowScores(void)
{
	m_bForceDraw = false;
	m_bShowscoresHeld = true;
}

void CHudScoreboard::UserCmd_HideScores(void)
{
	m_bForceDraw = m_bShowscoresHeld = false;
}


void CHudScoreboard::UserCmd_ShowScoreboard2()
{
	if (gEngfuncs.Cmd_Argc() != 9)
	{
		ConsolePrint("showscoreboard2 <xstart> <xend> <ystart> <yend> <r> <g> <b> <a>");
	}

	m_colors.r = atoi(gEngfuncs.Cmd_Argv(5));
	m_colors.b = atoi(gEngfuncs.Cmd_Argv(6));
	m_colors.b = atoi(gEngfuncs.Cmd_Argv(7));
	m_colors.a = atoi(gEngfuncs.Cmd_Argv(8));
	m_bDrawStroke = false;
	m_bForceDraw = true;
}

void CHudScoreboard::UserCmd_HideScoreboard2()
{
	m_bForceDraw = m_bShowscoresHeld = false; // and disable it
}

void CHudScoreboard::CacheTeamAliveNumber(void)
{
	m_iTeamAlive_T = GetTeamAliveCounts(TEAM_TERRORIST);
	m_iTeamAlive_CT = GetTeamAliveCounts(TEAM_CT);
}

}