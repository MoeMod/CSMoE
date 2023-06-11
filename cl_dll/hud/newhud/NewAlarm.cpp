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
// new alarm
//
#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

#include <string.h>
#include <stdio.h>
#include "draw_util.h"

#include "vgui_controls/controls.h"
#include "vgui/ILocalize.h"
#include "vgui/ISurface.h"
#include "vgui/IScheme.h"




namespace cl {
	DECLARE_MESSAGE(m_NewAlarm, ResetRound)

		vgui2::HFont m_font;
	struct AlarmNew
	{
		Alarm_Type type;
		std::string name;
		std::string sound;
		int ribbonCount;
		bool noalarm;
	};

	const AlarmNew AlarmDataInfo[] = {

		{ Alarm_Type(NULL) , "null" ,"null" ,NULL},
		{ ALARM_KNIFE , "knife" ,"vox/humililation.wav" ,20},
		{ ALARM_GRENADE , "grenade" ,"vox/gotit.wav" ,15},
		{ ALARM_HEADSHOT , "headshot" ,"vox/headshot.wav" ,25},
		{ ALARM_KILL , "kill" ,"metalarena/supply.wav"},
		{ ALARM_2KILL , "2kill" ,"vox/doublekill.wav" ,20},
		{ ALARM_3KILL , "3kill" ,"vox/triplekill.wav" ,18},
		{ ALARM_4KILL , "4kill" ,"vox/multikill.wav" ,16},
		{ ALARM_FIRSTBLOOD , "firstblood" ,"vox/firstkill.wav" ,10},
		{ ALARM_PAYBACK , "payback" ,"events/revenge.wav" ,20},
		{ ALARM_EXCELLENT , "excellent" ,"vox/excellent.wav" ,14},
		{ ALARM_INCREDIBLE , "incredible" ,"vox/incredible.wav" ,11},
		{ ALARM_CRAZY , "crazy" ,"vox/crazy.wav" ,8},
		{ ALARM_CANTBELIEVE , "cantbelieve" ,"vox/cantbelive.wav" ,5},
		{ ALARM_OUTOFWORLD , "outofworld" ,"vox/outofworld.wav" ,2},
		{ ALARM_LIBERATOR , "liberator" ,"metalarena/supply.wav" ,10},
		{ ALARM_ALLROUND , "allround" ,"metalarena/supply.wav" ,5},
		{ ALARM_INFECTOR , "infector" ,"metalarena/supply.wav" ,10},
		{ ALARM_SUPPORTER , "supporter" ,"metalarena/supply.wav" ,10},
		{ ALARM_KINGMAKER , "kingmaker" ,"metalarena/supply.wav" ,20},
		{ ALARM_ASSIST , "assist" ,"metalarena/supply.wav" ,30},
		{ ALARM_THELAST , "thelast" ,"metalarena/supply.wav" ,15},
		{ ALARM_LASTSOLDIER , "lastsoldier" ,"basic/lastsoldier.wav" ,15},
		{ ALARM_SAVIOR , "savior" ,"metalarena/supply.wav" ,15},
		{ ALARM_INVISHAND , "invisiblehand" ,"metalarena/supply.wav" ,20},
		{ ALARM_KINGMURDER , "kingmurder" ,"metalarena/supply.wav" ,15},
		{ ALARM_BACKMARKER , "backmarker" ,"metalarena/supply.wav" ,25},
		{ ALARM_WELCOME , "welcome" ,"metalarena/supply.wav" ,7},
		{ ALARM_COMEBACK , "comeback" ,"metalarena/supply.wav" ,10},
		{ ALARM_C4MANKILL , "c4mankill" ,"metalarena/supply.wav" ,20},
		{ ALARM_C4PLANT , "c4plant" ,"metalarena/supply.wav" ,20},
		{ ALARM_C4PLANTBLOCK , "c4plantblock" ,"metalarena/supply.wav" ,10},
		{ ALARM_C4DEFUSTBLOCK , "c4defuseblock" ,"metalarena/supply.wav" ,10},
		{ ALARM_ZOMBIEBOMB , "zombiebomb" ,"metalarena/supply.wav" ,15},
		{ ALARM_ZOMBIETANKER , "zombietanker" ,"metalarena/supply.wav" ,30},
		{ ALARM_INTOXICATION , "intoxication" ,"metalarena/supply.wav" ,10},
		{ ALARM_REVIVALWILL , "revivalwill" ,"metalarena/supply.wav" ,2},
		{ ALARM_MAXLEVEL , "maxlevel" ,"metalarena/supply.wav" ,1},
		{ ALARM_PROFESSIONAL , "professional" ,"metalarena/supply.wav" ,1},
		{ ALARM_UNTOUCHABLE , "untouchable" ,"metalarena/supply.wav" ,1},
		{ ALARM_DESTRUCTION , "destruction" ,"metalarena/supply.wav" ,1},
		{ ALARM_TERMINATION , "termination" ,"metalarena/supply.wav" ,1},
		{ ALARM_RESCUE , "rescue" ,"metalarena/supply.wav" ,5},
		{ ALARM_COMBO , "combo" ,"metalarena/supply.wav" ,5},
		{ ALARM_ALIVE , "alive" ,"metalarena/supply.wav" ,6},
		{ ALARM_NOPARASHUTE , "noparashute" ,"NULL" ,10 ,true},
		{ ALARM_CHICKENCHASER , "chickenchaser" ,"NULL" ,10,true},

		{ ALARM_OVERRUN , "overrun" ,"metalarena/supply.wav" ,15},
		{ ALARM_ATTACKER , "attacker" ,"metalarena/supply.wav" ,20},
		{ ALARM_BESTMOMENT , "bestmoment" ,"metalarena/supply.wav" ,15},
		{ ALARM_HOLDOUT , "holdout" ,"metalarena/supply.wav" ,18},
		{ ALARM_BATPULL , "batpull" ,"metalarena/supply.wav" ,5},
		{ ALARM_DROPWEAPON , "dropweapon" ,"metalarena/supply.wav" ,14},
		{ ALARM_HIDE , "hide" ,"metalarena/supply.wav" ,5},
		{ ALARM_HEALER , "healer" ,"metalarena/supply.wav" ,25},
		{ ALARM_LAST , "null", "null", NULL}
	};

	static CHudNewAlarm s_HudDrawAlarm;

	CHudNewAlarm& NewAlarm(void)
	{
		return s_HudDrawAlarm;
	};


	CHudNewAlarm::CHudNewAlarm()
	{
		//From Sme
		//memset(m_AlarmDefault, 0, sizeof(m_AlarmDefault));
		m_vecAlarmCustom.clear();
		//memset(&m_AlarmDisplay, 0, sizeof(AlarmDisplay_t));

		bFirstblood = false;

		m_iTextureBG = nullptr;
		m_iTextureLogoBG = nullptr;
		bTextureLoaded = false;
	}
	CHudNewAlarm::~CHudNewAlarm()
	{
		//From Sme
		std::vector<AlarmBasicdata>().swap(m_vecAlarmCustom);
		std::vector<std::pair<AlarmBasicdata, bool>>().swap(m_AlarmDisplay.m_vecAlarm);
		std::vector<int>().swap(m_AlarmDisplay.m_vecCountRibbonC);
	}
	int CHudNewAlarm::Init(void)
	{
		gHUD.AddHudElem(this);
		m_iFlags |= HUD_DRAW;
		HOOK_MESSAGE(ResetRound);

		//From Sme
		bAlarmRead = false;

		return 1;
	}

	void CHudNewAlarm::InitHUDData(void)
	{

	}
	int CHudNewAlarm::VidInit(void)
	{
		vgui2::IScheme* pClientScheme = vgui2::scheme()->GetIScheme(vgui2::scheme()->GetScheme("ClientScheme"));
		m_font = pClientScheme->GetFont("Default20");

		//From Sme
		if (!m_vecAlarmCustom.size() && !bAlarmRead)
		{
			static int iCountDefault;
			iCountDefault = 0;

			const AlarmNew* AlarmInfo = nullptr;
			AlarmInfo = AlarmDataInfo;

			for (int i = 1; i < ALARM_LAST; i++)
			{
				char szAlarm[64];
				strcpy(szAlarm, AlarmInfo[i].name.c_str());
				m_AlarmDefault[i] = InitAlarm(szAlarm, i, false);

			}

			bAlarmRead = true;
			m_AlarmDisplay.m_vecAlarm.reserve(32);
		}

		m_AlarmDisplay.m_bPlaying = false;
		m_AlarmDisplay.m_bReset = true;
		m_AlarmDisplay.m_flDisplayTime = 0.0f;
		m_AlarmDisplay.m_vecAlarm.clear();
		m_AlarmDisplay.m_vecCountRibbonC.clear();

		//memset(m_AlarmDisplay.m_iCount_Ribbon, 0, sizeof(m_AlarmDisplay.m_iCount_Ribbon));

		if (!bTextureLoaded)
		{
			R_InitTexture(m_iTextureBG, "resource/announceribbon/announceicon/alarm_bg");
			R_InitTexture(m_iTextureLogoBG, "resource/announceribbon/announceicon/alarm_logobg");
			bTextureLoaded = true;
		}

		return 1;
	}

	void CHudNewAlarm::Shutdown(void)
	{
	}

	int CHudNewAlarm::Draw(float flTime)
	{
		if ((gHUD.m_iHideHUDDisplay & HIDEHUD_ALL))
			return 1;

		RedrawAlarm(flTime);
		return 1;
	}
	//From Sme And Modified By Moe
	AlarmBasicdata CHudNewAlarm::InitAlarm(char* szName, int iType, bool bCustom)
	{
		AlarmBasicdata g_TempAlarm;
		//memset(&g_TempAlarm, 0, sizeof(AlarmBasicdata));

		char data[64];
		strcpy(data, AlarmDataInfo[iType].name.c_str());

		if (data[0] != '-')
		{
			g_TempAlarm.bInitialized = true;
			strcpy(g_TempAlarm.szName, data);

			static char szImagePath[128];
			sprintf(szImagePath, "resource/announceribbon/announceicon/alarm_%s", g_TempAlarm.szName);
			R_InitTexture(g_TempAlarm.iTexture, szImagePath);

			//HaveMessage?
			g_TempAlarm.bHasMsgBox = true;


			//CanAlarm?
			bool data2; data2 = AlarmDataInfo[iType].noalarm;
			if (data2)
				g_TempAlarm.bHasAlarm = false;
			else
				g_TempAlarm.bHasAlarm = true;

			if (g_TempAlarm.bHasAlarm)
			{
				//Sound
				strcpy(g_TempAlarm.szSound, AlarmDataInfo[iType].sound.c_str());

				//Vgui Localize
				char SzText[64]; sprintf(SzText, "CSO_Alarm_%s", g_TempAlarm.szName);
				if (vgui2::localize()->Find(SzText))
					wcscpy(g_TempAlarm.m_wcsAlarmText, vgui2::localize()->Find(SzText));
				else
				{
					vgui2::localize()->ConvertANSIToUnicode(SzText, vgui2::localize()->Find(SzText), sizeof(vgui2::localize()->Find(SzText)));
					mbstowcs(g_TempAlarm.m_wcsAlarmText, SzText, strlen(SzText));
				}

				sprintf(SzText, "CSO_Alarm_%s_desc", g_TempAlarm.szName);
				if (vgui2::localize()->Find(SzText))
					wcscpy(g_TempAlarm.m_wcsAlarmDesc, vgui2::localize()->Find(SzText));
				else
				{
					vgui2::localize()->ConvertANSIToUnicode(SzText, vgui2::localize()->Find(SzText), sizeof(vgui2::localize()->Find(SzText)));
					mbstowcs(g_TempAlarm.m_wcsAlarmDesc, SzText, strlen(SzText));
				}
			}

			//RibbonCount
			g_TempAlarm.iRibbonRequire = AlarmDataInfo[iType].ribbonCount;

			if (g_TempAlarm.iRibbonRequire)
			{
				sprintf(szImagePath, "resource/announceribbon/ribbonicon/ribbon_%s", g_TempAlarm.szName);
				R_InitTexture(g_TempAlarm.iTextureRibbon, szImagePath);

				//Vgui Localize
				char SzText[64]; sprintf(SzText, "CSO_Ribbon_%s", g_TempAlarm.szName);
				if (vgui2::localize()->Find(SzText))
					wcscpy(g_TempAlarm.m_wcsRibbonText, vgui2::localize()->Find(SzText));
				else
				{
					vgui2::localize()->ConvertANSIToUnicode(SzText, vgui2::localize()->Find(SzText), sizeof(vgui2::localize()->Find(SzText)));
					mbstowcs(g_TempAlarm.m_wcsRibbonText, SzText, strlen(SzText));
				}

				sprintf(SzText, "CSO_Ribbon_%s_desc", g_TempAlarm.szName);
				if (vgui2::localize()->Find(SzText))
					wcscpy(g_TempAlarm.m_wcsRibbonDesc, vgui2::localize()->Find(SzText));
				else
				{
					vgui2::localize()->ConvertANSIToUnicode(SzText, vgui2::localize()->Find(SzText), sizeof(vgui2::localize()->Find(SzText)));
					mbstowcs(g_TempAlarm.m_wcsRibbonDesc, SzText, strlen(SzText));
				}
			}

			if (bCustom)
				g_TempAlarm.index = m_vecAlarmCustom.size() + 1;
			else
			{
				static int iCountDefault;
				g_TempAlarm.index = iCountDefault + 1;
				iCountDefault++;
			}
		}

		return g_TempAlarm;
	}
	int CHudNewAlarm::FindAlarm(char* szName, int iType)
	{
		for (auto& iter : m_vecAlarmCustom)
		{
			if (!strcmp(iter.szName, strlwr(szName)))
				return iter.index;
		}

		AlarmBasicdata AlarmData = InitAlarm(szName, iType, true);
		if (AlarmData.bInitialized)
		{
			m_vecAlarmCustom.emplace_back(AlarmData);
			m_AlarmDisplay.m_vecCountRibbonC.emplace_back(0);

			return AlarmData.index;
		}

		return 0;
	}
	void CHudNewAlarm::SetAlarm(int iAlarm, bool bCustom)
	{
		if (gHUD.m_alarmstyle->value != 0)
			return;

		AlarmBasicdata g_TempAlarm;
		if (!bCustom)
		{
			if (iAlarm >= ALARM_LAST)
				return;

			g_TempAlarm = m_AlarmDefault[iAlarm];
		}
		else
		{
			if (iAlarm >= (int)m_vecAlarmCustom.size())
				return;

			g_TempAlarm = m_vecAlarmCustom[iAlarm];
		}


		if (!g_TempAlarm.bInitialized)
			return;

		if (g_TempAlarm.bHasAlarm)
		{
			if (IsPlaying(iAlarm, bCustom))
				goto CHECK_RIBBON;

			m_AlarmDisplay.m_vecAlarm.emplace_back(g_TempAlarm, false);

			if (!m_AlarmDisplay.m_bPlaying)
			{
				m_AlarmDisplay.m_bPlaying = true;
				m_AlarmDisplay.m_bReset = true;
				m_AlarmDisplay.m_bBackGround = true;
			}
		}

	CHECK_RIBBON:
		if (!bCustom)
			m_AlarmDisplay.m_iCount_Ribbon[iAlarm]++;
		else
		{
			m_AlarmDisplay.m_vecCountRibbonC.insert(m_AlarmDisplay.m_vecCountRibbonC.begin() + iAlarm, m_AlarmDisplay.m_vecCountRibbonC[iAlarm]++);
			//gEngfuncs.Con_Printf("Debug Custom Alarm Check: %d %d\n", iAlarm, m_AlarmDisplay.m_vecCountRibbonC[iAlarm]);
		}

		int iRibbon = (!bCustom) ? m_AlarmDisplay.m_iCount_Ribbon[iAlarm] : m_AlarmDisplay.m_vecCountRibbonC[iAlarm];

		if (iRibbon >= g_TempAlarm.iRibbonRequire && g_TempAlarm.iRibbonRequire > 0)
		{
			if (!bCustom)
				m_AlarmDisplay.m_iCount_Ribbon[iAlarm] = 0;
			else
				m_AlarmDisplay.m_vecCountRibbonC.insert(m_AlarmDisplay.m_vecCountRibbonC.begin() + iAlarm, 0);

			m_AlarmDisplay.m_vecAlarm.emplace_back(g_TempAlarm, true);

			if (!m_AlarmDisplay.m_bPlaying)
			{
				m_AlarmDisplay.m_bPlaying = true;
				m_AlarmDisplay.m_bReset = true;
				m_AlarmDisplay.m_bBackGround = false;
			}
		}
	}

	bool CHudNewAlarm::IsPlaying(int iAlarm, bool bCustom)
	{
		if (!iAlarm)
			return IsPlaying();

		if (!bCustom)
		{
			for (auto& iter : m_AlarmDisplay.m_vecAlarm)
			{
				if (iter.first.index == m_AlarmDefault[iAlarm].index)
				{

					gEngfuncs.Con_Printf("[Alarm] Local Player has %d(name: %s) alarm\n", iAlarm, m_AlarmDefault[iAlarm].szName);

					return true;
				}
			}
		}
		else
		{
			for (auto& iter : m_AlarmDisplay.m_vecAlarm)
			{
				if (iter.first.index == m_vecAlarmCustom[iAlarm].index)
				{

					gEngfuncs.Con_Printf("[Alarm] Local Player has %d(name: %s) alarm\n", iAlarm, m_vecAlarmCustom[iAlarm].szName);

					return true;
				}
			}
		}

		return false;
	}

	int CHudNewAlarm::RedrawAlarm(float flTime)
	{
		if (m_font == vgui2::INVALID_FONT)
			return 1;


		if (!m_AlarmDisplay.m_bPlaying)
			return 1;

		if (!m_AlarmDisplay.m_vecAlarm.size())
		{
			m_AlarmDisplay.m_bPlaying = false;
			return 1;
		}

		auto frontelem = m_AlarmDisplay.m_vecAlarm.front();

		if (m_AlarmDisplay.m_bReset)
		{
			m_AlarmDisplay.m_bReset = false;
			m_AlarmDisplay.m_flDisplayTime = gHUD.m_flTime + 1.0f;

			if (frontelem.second)
			{
				gEngfuncs.pfnClientCmd("spk sound/basic/assist.wav");
			}
			else if (frontelem.first.szSound[0])
			{

				static char szAlarmSound[128];
				sprintf(szAlarmSound, "spk sound/%s", frontelem.first.szSound);
				gEngfuncs.pfnClientCmd(szAlarmSound);
			}
		}

		int iX = round(ScreenWidth * 0.5f);
		int iY = ScreenHeight * 0.2f;
		int iA = 255;

		if (m_AlarmDisplay.m_flDisplayTime <= gHUD.m_flTime)
		{
			iA = (int)(255.0f * (1.0f - (gHUD.m_flTime - m_AlarmDisplay.m_flDisplayTime) / 0.25f));
			if (iA <= 0)
			{
				if (m_AlarmDisplay.m_vecAlarm.size() > 1)
				{
					for (int i = 0; i < (int)m_AlarmDisplay.m_vecAlarm.size() - 1; i++)
						m_AlarmDisplay.m_vecAlarm[i] = m_AlarmDisplay.m_vecAlarm[i + 1];
				}

				m_AlarmDisplay.m_vecAlarm.pop_back();

				m_AlarmDisplay.m_bReset = true;
				if (m_AlarmDisplay.m_bBackGround)
					m_AlarmDisplay.m_bBackGround = false;


				return 1;
			}
		}

		if (m_AlarmDisplay.m_bBackGround && !frontelem.second)
			m_iTextureBG->Draw2DQuadScaled(iX - m_iTextureBG->w() / 2, iY - m_iTextureBG->h() / 3, iX - m_iTextureBG->w() / 2 + m_iTextureBG->w(), iY - m_iTextureBG->h() / 3 + m_iTextureBG->h(), 0.0F, 0.0F, 1.0F, 1.0F, 255, 255, 255, iA);

		SharedTexture(*DrawIndex) = nullptr;
		DrawIndex = frontelem.second ? &(frontelem.first.iTextureRibbon) : &(frontelem.first.iTexture);

		if (DrawIndex)
		{
			(*DrawIndex)->Draw2DQuadScaled(iX - (*DrawIndex)->w() / 2, iY, iX - (*DrawIndex)->w() / 2 + (*DrawIndex)->w(), iY + (*DrawIndex)->h(), 0.0F, 0.0F, 1.0F, 1.0F, 255, 255, 255, iA);
			iY += (*DrawIndex)->h();
		}

		if (frontelem.first.bHasMsgBox)
		{
			wchar_t* pMsg = !frontelem.second ? frontelem.first.m_wcsAlarmText : frontelem.first.m_wcsRibbonText;
			wchar_t* pMsgDesc = !frontelem.second ? frontelem.first.m_wcsAlarmDesc : frontelem.first.m_wcsRibbonDesc;
			m_iTextureLogoBG->Draw2DQuadScaled(iX - m_iTextureLogoBG->w() / 2, iY, iX - m_iTextureLogoBG->w() / 2 + m_iTextureLogoBG->w(), iY + m_iTextureLogoBG->h(), 0.0F, 0.0F, 1.0F, 1.0F, 255, 255, 255, iA);

			int r = 255, g = 255, b = 255;
			/*DrawUtils::ScaleColors(r, g, b, iA);

			int iLength, iHeight;
			gEngfuncs.pfnDrawConsoleStringLen(HudDeathInfo().UnicodeToUTF8(pMsg), &iLength, &iHeight);
			DrawUtils::DrawHudString(iX - iLength / 2 + 2, iY + 10, ScreenWidth, HudDeathInfo().UnicodeToUTF8(pMsg), r, g, b);

			gEngfuncs.pfnDrawConsoleStringLen(HudDeathInfo().UnicodeToUTF8(pMsgDesc), &iLength, &iHeight);
			DrawUtils::DrawHudString(iX - iLength / 2 + 2, iY + 15 + iHeight, ScreenWidth, HudDeathInfo().UnicodeToUTF8(pMsgDesc), r, g, b);*/

			int iLen = GetWide(pMsg);
			DrawVguiTexts((ScreenWidth - iLen) / 2, iY + 10, r, g, b, iA, pMsg);
			//iLen = GetWide(pMsgDesc);
			//DrawVguiTexts((ScreenWidth - iLen) / 2, iY + 12 + vgui2::surface()->GetFontTall(m_font), r, g, b, iA, pMsgDesc);
		}
		return 1;
	}

	int CHudNewAlarm::GetWide(const wchar_t* str)
	{
		if (!m_font)
			return 0;

		if (!str)
			return 0;

		int width = 0;
		for (unsigned int i = 0; i < wcslen(str); i++)
			width += vgui2::surface()->GetCharacterWidth(m_font, str[i]);

		return width;
	}

	void CHudNewAlarm::DrawVguiTexts(int x, int y, int r, int g, int b, int a, const wchar_t* str)
	{
		if (!str)
			return;

		vgui2::surface()->DrawSetTextFont(m_font);

		for (unsigned int i = 0; i < wcslen(str); i++)
		{
			vgui2::surface()->DrawSetTextPos(x, y);
			vgui2::surface()->DrawSetTextColor(r, g, b, a);
			vgui2::surface()->DrawUnicodeChar(str[i]);
			x += vgui2::surface()->GetCharacterWidth(m_font, str[i]);
		}

		vgui2::surface()->DrawFlushText();
	}
	// This message handler may be better off elsewhere
	int CHudNewAlarm::MsgFunc_ResetRound(const char* pszName, int iSize, void* pbuf)
	{
		m_iFlags |= HUD_DRAW;

		BufferReader reader(pszName, pbuf, iSize);

		int iTotalRound = reader.ReadByte();
		if (iTotalRound >= 0)
		{
			for (int i = 0; i < 33; i++)
			{
				g_iDamage[i] = 0;
				g_iDamageTotal[i] = 0;

				if (!(g_PlayerInfoList[i].name && g_PlayerInfoList[i].name[0] != 0))
					continue;

				g_CWcount[i][1] = 0;
				g_CWcount[i][2] += 1;

			}

			g_iPlanter = g_iDefuser = 0;
			g_lastsoldier[0] = 0;
			g_lastsoldier[1] = 0;

			g_flDamageInAll = 0;
			gHUD.m_ZB2.m_flAliveTime = 0.0;
			gHUD.m_ZB2.m_flPlayerMoveDis = 0.0;
			gHUD.m_ZB2.m_flRecoveryAmount = 0.0;


			g_bFirstBlood = true;
			cl::gHUD.m_flZombieSelectTime = 0.0;
		}

		return 1;
	}
}