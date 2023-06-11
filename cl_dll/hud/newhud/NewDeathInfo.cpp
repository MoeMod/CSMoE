/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
//
// new deathinfo
//
#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

#include <string.h>
#include <stdio.h>
#include "draw_util.h"

#include "vgui_controls/controls.h"
#include "vgui/ILocalize.h"


namespace cl {
	DECLARE_MESSAGE(m_DeathInfo, DeathInfo)
		static CHudDeathInfo s_HudDeathInfo;

	CHudDeathInfo& HudDeathInfo(void)
	{
		return s_HudDeathInfo;
	}

	CHudDeathInfo::CHudDeathInfo()
	{
		memset(m_szMsgEnemyDeath, 0, sizeof(m_szMsgEnemyDeath));
		memset(m_szBody, 0, sizeof(m_szBody));
		memset(m_szAmmoCount, 0, sizeof(m_szAmmoCount));
		memset(m_szMsgDamageToKiller, 0, sizeof(m_szMsgDamageToKiller));
		memset(m_szMsgKillerState, 0, sizeof(m_szMsgKillerState));
		memset(m_szMsgDamagedNone, 0, sizeof(m_szMsgDamagedNone));

		memset(m_szTempKiller, 0, sizeof(m_szTempKiller));
		memset(m_szTempVictim, 0, sizeof(m_szTempVictim));

		memset(m_wszTotalKillerText, 0, sizeof(m_wszTotalKillerText));
		memset(m_wszTotalVictimText, 0, sizeof(m_wszTotalVictimText));
	}

	CHudDeathInfo::~CHudDeathInfo()
	{
	}
	void CHudDeathInfo::Init(void)
	{
		HOOK_MESSAGE(DeathInfo);

		memset(m_szTempKiller, 0, sizeof(m_szTempKiller));
		memset(m_szTempVictim, 0, sizeof(m_szTempVictim));

		memset(m_wszTotalKillerText, 0, sizeof(m_wszTotalKillerText));
		memset(m_wszTotalVictimText, 0, sizeof(m_wszTotalVictimText));

		memset(&Killer, 0, sizeof(deathinfo_t));
		memset(&Victim, 0, sizeof(deathinfo_t));
		memset(&Victim2, 0, sizeof(deathinfo_t));
	}

	void CHudDeathInfo::VidInit(void)
	{
		//Vgui Localize
		static char SzText[64]; sprintf(SzText, "CSO_EnemyDeathMsg");
		if (vgui2::localize()->Find(SzText))
			strcpy(m_szMsgEnemyDeath, UnicodeToUTF8(vgui2::localize()->Find(SzText)));
		else
			strcpy(m_szMsgEnemyDeath, SzText);

		ReplaceTokenAll(m_szMsgEnemyDeath, "\r\n", "\n");

		static char szTempBody[32];
		for (int i = 0; i < 5; i++)
		{
			switch (i)
			{
			case 0:
			{
				strcpy(szTempBody, "CSO_Head");
				break;
			}
			case 1:
			{
				sprintf(szTempBody, "CSO_Chest");
				break;
			}
			case 2:
			{
				sprintf(szTempBody, "CSO_Stomach");
				break;
			}
			case 3:
			{
				sprintf(szTempBody, "CSO_Arm");
				break;
			}
			case 4:
			{
				sprintf(szTempBody, "CSO_Leg");
				break;
			}
			break;
			}

			//Vgui Localize
			strcpy(SzText, szTempBody);
			if (vgui2::localize()->Find(SzText))
				strcpy(m_szBody[i], UnicodeToUTF8(vgui2::localize()->Find(SzText)));
			else
				strcpy(m_szBody[i], SzText);
		}

		strcpy(SzText, "CSO_AmmoCount");
		if (vgui2::localize()->Find(SzText))
			strcpy(m_szAmmoCount, UnicodeToUTF8(vgui2::localize()->Find(SzText)));
		else
			strcpy(m_szAmmoCount, SzText);

		strcpy(SzText, "CSO_KillMsg");
		if (vgui2::localize()->Find(SzText))
			strcpy(m_szMsgDamageToKiller, UnicodeToUTF8(vgui2::localize()->Find(SzText)));
		else
			strcpy(m_szMsgDamageToKiller, SzText);

		strcpy(SzText, "CSO_HpAp");
		if (vgui2::localize()->Find(SzText))
			strcpy(m_szMsgKillerState, UnicodeToUTF8(vgui2::localize()->Find(SzText)));
		else
			strcpy(m_szMsgKillerState, SzText);

		strcpy(SzText, "CSO_KillMsg2");
		if (vgui2::localize()->Find(SzText))
			strcpy(m_szMsgDamagedNone, UnicodeToUTF8(vgui2::localize()->Find(SzText)));
		else
			strcpy(m_szMsgDamagedNone, SzText);
	}

	void CHudDeathInfo::Set(deathinfo_t data, int type)
	{
		if (type == 1)
		{
			if (!data.iPlayer)
				return;

			memset(m_szTempKiller, 0, sizeof(m_szTempKiller));
			memset(m_wszTotalKillerText, 0, sizeof(m_wszTotalKillerText));

			hud_player_info_t hPlayer;
			gEngfuncs.pfnGetPlayerInfo(data.iPlayer, &hPlayer);

			strcpy(m_szTempKiller, m_szMsgEnemyDeath);
			ReplaceTokenFirst(m_szTempKiller, "%s", hPlayer.name);

			char SzTemp[512]; sprintf(SzTemp, "%d", data.iDist);
			ReplaceTokenFirst(m_szTempKiller, "%d", SzTemp);
			ReplaceTokenFirst(m_szTempKiller, "%s", data.szWeapon);

			sprintf(SzTemp, "%d", data.iDamage[5]);
			ReplaceTokenFirst(m_szTempKiller, "%d", SzTemp);

			for (int i = 0; i < 5; i++)
			{
				if (data.iDamage[i])
				{
					static char szShots[64];
					sprintf(szShots, "%s: %d(%d%s)", m_szBody[i], data.iDamage[i], data.iShot[i], m_szAmmoCount);
					strcat(m_szTempKiller, szShots);
					strcat(m_szTempKiller, "\n");
				}
			}

			wcscpy(m_wszTotalKillerText, UTF8ToUnicode(m_szTempKiller));
			Killer = data;
		}
		else if (type == 2)
		{
			if (!data.iPlayer)
				return;

			strcpy(m_szTempVictim, "");
			memset(m_wszTotalVictimText, 0, sizeof(m_wszTotalVictimText));

			hud_player_info_t hPlayer;
			gEngfuncs.pfnGetPlayerInfo(data.iPlayer, &hPlayer);

			//Vgui Localize
			static char SzText[512]; sprintf(SzText, "CSO_DeathMsg");
			if (vgui2::localize()->Find(SzText))
				strcpy(m_szTempVictim, UnicodeToUTF8(vgui2::localize()->Find(SzText)));
			else
				strcpy(m_szTempVictim, SzText);


			ReplaceTokenAll(m_szTempVictim, "\r\n", "\n");
			ReplaceTokenFirst(m_szTempVictim, "%s", hPlayer.name);

			char SzTemp[512]; sprintf(SzTemp, "%d", data.iDist);
			ReplaceTokenFirst(m_szTempVictim, "%d", SzTemp);
			ReplaceTokenFirst(m_szTempVictim, "%s", data.szWeapon);

			sprintf(SzTemp, "%d", data.iDamage[5]);
			ReplaceTokenFirst(m_szTempVictim, "%d", SzTemp);

			for (int i = 0; i < 5; i++)
			{
				if (data.iDamage[i])
				{
					static char szShots[64];
					sprintf(szShots, "%s: %d(%d%s)", m_szBody[i], data.iDamage[i], data.iShot[i], m_szAmmoCount);
					strcat(m_szTempVictim, szShots);
					strcat(m_szTempVictim, "\n");
				}
			}
			Victim = data;
		}
		else
		{
			if (data.iDamage[5])
			{
				ReplaceTokenFirst(m_szMsgDamageToKiller, "%s", data.szWeapon);

				char SzTemp[64]; sprintf(SzTemp, "%d", data.iDamage[5]);
				ReplaceTokenFirst(m_szMsgDamageToKiller, "%d", SzTemp);

				strcat(m_szTempVictim, m_szMsgDamageToKiller);

				for (int i = 0; i < 5; i++)
				{
					if (data.iDamage[i])
					{
						static char szShots[64];
						sprintf(szShots, "%s: %d(%d%s)", m_szBody[i], data.iDamage[i], data.iShot[i], m_szAmmoCount);
						strcat(m_szTempVictim, szShots);
						strcat(m_szTempVictim, "\n");
					}
				}
				sprintf(SzTemp, "%d", data.iHealth[0]);
				ReplaceTokenFirst(m_szMsgKillerState, "%d", SzTemp);
				sprintf(SzTemp, "%d", data.iHealth[1]);
				ReplaceTokenFirst(m_szMsgKillerState, "%d", SzTemp);

				strcat(m_szTempVictim, m_szMsgKillerState);
			}
			else
				strcat(m_szTempVictim, m_szMsgDamagedNone);

			wcscpy(m_wszTotalVictimText, UTF8ToUnicode(m_szTempVictim));
			Victim2 = data;
		}
	}
	void CHudDeathInfo::Redraw(void)
	{
		if (Killer.bDraw == true)
		{
			if (Killer.fDisplaytime <= gHUD.m_flTime)
			{
				if (Killer.fDisplaytime)
				{
					memset(&Killer, 0, sizeof(deathinfo_t));

					memset(m_wszTotalKillerText, 0, sizeof(m_wszTotalKillerText));
				}

				goto CHudDeathInfo_VC0;
			}

			//DrawUtils::DrawHudString(ScreenWidth * 0.55f, ScreenHeight * 0.55f, ScreenWidth, HudDeathInfo().UnicodeToUTF8(m_wszTotalKillerText), 183, 207, 245);

			DrawUtils::SetConsoleTextColor(183 / 255.0f, 207 / 255.0f, 245 / 255.0f);
			DrawUtils::DrawConsoleString(ScreenWidth * 0.55f, ScreenHeight * 0.55f, HudDeathInfo().UnicodeToUTF8(m_wszTotalKillerText));
			gEngfuncs.pfnConsolePrint(HudDeathInfo().UnicodeToUTF8(m_wszTotalKillerText));
		}

	CHudDeathInfo_VC0:
		if (Victim.bDraw == true)
		{
			if (Victim.fDisplaytime <= gHUD.m_flTime)
			{
				if (Victim.fDisplaytime)
				{
					memset(&Victim, 0, sizeof(deathinfo_t));
					memset(&Victim2, 0, sizeof(deathinfo_t));
					memset(m_wszTotalVictimText, 0, sizeof(m_wszTotalVictimText));
				}

				return;
			}
			//DrawUtils::DrawHudString(ScreenWidth * 0.39f, ScreenHeight * 0.52f, ScreenWidth, HudDeathInfo().UnicodeToUTF8(m_wszTotalVictimText), 242, 148, 148);
			//DrawUtils::DrawHudString(ScreenWidth * 0.39f, ScreenHeight * 0.55f, ScreenWidth, "\nTest\nTest2", 242, 148, 148);

			DrawUtils::SetConsoleTextColor(242 / 255.0f, 148 / 255.0f, 148 / 255.0f);
			DrawUtils::DrawConsoleString(ScreenWidth * 0.39f, ScreenHeight * 0.52f, HudDeathInfo().UnicodeToUTF8(m_wszTotalVictimText));
		}
	}
	void CHudDeathInfo::InitDeathInfo(deathinfo_t* temp)
	{
		temp->bDraw = false;
		temp->iPlayer = 0;
		temp->iDist = 0;
		for (int i = 0; i < 6; i++)
			temp->iDamage[i] = 0;
		for (int i = 0; i < 5; i++)
			temp->iShot[i] = 0;

		sprintf(temp->szWeapon, "");
		temp->iHealth[0] = 0;
		temp->iHealth[1] = 0;
		temp->fDisplaytime = 0.0;
	}

	int CHudDeathInfo::MsgFunc_DeathInfo(const char* pszName, int iSize, void* pbuf)
	{
		BufferReader reader(pszName, pbuf, iSize);

		deathinfo_t g_DeathInfo;

		int iType = reader.ReadByte();
		g_DeathInfo.iPlayer = reader.ReadByte();
		g_DeathInfo.iDist = reader.ReadByte();

		char* szWeapon = reader.ReadString();

		if (szWeapon && szWeapon[0])
		{
			//Vgui Localize
			static char SzText[64]; sprintf(SzText, "CSO_%s", szWeapon);
			if (vgui2::localize()->Find(SzText))
				strcpy(g_DeathInfo.szWeapon, UnicodeToUTF8(vgui2::localize()->Find(SzText)));
			else
				strcpy(g_DeathInfo.szWeapon, SzText);
		}
		else
			g_DeathInfo.szWeapon[0] = 0;

		g_DeathInfo.iDamage[5] = reader.ReadLong();

		for (int i = 0; i < 5; i++)
		{
			g_DeathInfo.iDamage[i] = reader.ReadLong();
			g_DeathInfo.iShot[i] = reader.ReadByte();
		}

		g_DeathInfo.fDisplaytime = gHUD.m_flTime + 5.0f;
		g_DeathInfo.bDraw = true;

		HudDeathInfo().Set(g_DeathInfo, iType);

		if (iType == 2)
		{
			char* szWeapon = reader.ReadString();
			if (szWeapon && szWeapon[0])
			{
				//Vgui Localize
				static char SzText[64]; sprintf(SzText, "CSO_%s", szWeapon);
				if (vgui2::localize()->Find(SzText))
					strcpy(g_DeathInfo.szWeapon, UnicodeToUTF8(vgui2::localize()->Find(SzText)));
				else
					strcpy(g_DeathInfo.szWeapon, SzText);
			}
			else
				g_DeathInfo.szWeapon[0] = 0;

			g_DeathInfo.iDamage[5] = reader.ReadLong();

			for (int i = 0; i < 5; i++)
			{
				g_DeathInfo.iDamage[i] = reader.ReadLong();
				g_DeathInfo.iShot[i] = reader.ReadByte();
			}

			g_DeathInfo.iHealth[0] = reader.ReadShort();
			g_DeathInfo.iHealth[1] = reader.ReadShort();

			HudDeathInfo().Set(g_DeathInfo, 3);
		}

		return 1;
	}



	//From Sme
	void CHudDeathInfo::ReplaceTokenFirst(char* szString, const char* szToken, const char* szSwitch)
	{
		if (!szString || !szToken || !strlen(szToken))
			return;

		if (!szSwitch)
			szSwitch = "";

		if (!strcmp(szToken, szSwitch))
			return;

		char* p = strstr(szString, szToken);
		if (p)
		{
			int dist = p - szString;

			int size = dist + 1;
			char* q = new char[size];
			strncpy(q, szString, size);
			q[size - 1] = NULL;

			size = strlen(szString) - dist - strlen(szToken) + 1;
			char* r = new char[size];
			strncpy(r, szString + dist + strlen(szToken), size);
			r[size - 1] = NULL;

			sprintf(szString, "%s%s%s", q, szSwitch, r);

			delete[] q;
			delete[] r;
		}
	}
	void CHudDeathInfo::ReplaceTokenAll(char* szString, const char* szToken, const char* szSwitch)
	{
		if (!szString || !szToken || !strlen(szToken))
			return;

		if (!szSwitch)
			szSwitch = "";

		if (!strcmp(szToken, szSwitch))
			return;

		char* p = strstr(szString, szToken);
		while (p)
		{
			int dist = p - szString;

			int size = dist + 1;
			char* q = new char[size];
			strncpy(q, szString, size);
			q[size - 1] = NULL;

			size = strlen(szString) - dist - strlen(szToken) + 1;
			char* r = new char[size];
			strncpy(r, szString + dist + strlen(szToken), size);
			r[size - 1] = NULL;

			sprintf(szString, "%s%s%s", q, szSwitch, r);

			delete[] q;
			delete[] r;

			p = strstr(szString, szToken);
		}
	}
	char* CHudDeathInfo::UnicodeToUTF8(const wchar_t* str)
	{
		static char result[1024];
		/*int textlen;
		textlen = WideCharToMultiByte(CP_UTF8, 0, str, -1, NULL, 0, NULL, NULL);
		memset(result, 0, sizeof(char) * (textlen + 1));
		WideCharToMultiByte(CP_UTF8, 0, str, -1, result, textlen, NULL, NULL);*/

		Q_UnicodeToUTF8(str, result, 1023);
		return result;
	}
	wchar_t* CHudDeathInfo::UTF8ToUnicode(const char* str)
	{
		static wchar_t result[1024];
		/*int textlen;
		textlen = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
		memset(result, 0, sizeof(char) * (textlen + 1));
		MultiByteToWideChar(CP_UTF8, 0, str, -1, (LPWSTR)result, textlen);*/

		Q_UTF8ToUnicode(str, result, 1023);
		return result;
	}
}