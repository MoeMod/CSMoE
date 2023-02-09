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
// battery.cpp
//
// implementation of CHudBattery class
//

#include "hud.h"
#include "parsemsg.h"
#include "cl_util.h"
#include "draw_util.h"
#include "legacy/hud_scoreboard_legacy.h"
#include "player/player_model.h"

namespace cl {

DECLARE_MESSAGE(m_NewHud, NewHudMsg)

CHudNewHud::CHudNewHud()
{
	CHudScoreBoardLegacy::BuildNumberRC(m_iNum_CharacterC, 7, 16);
}

int CHudNewHud::Init(void)
{
	HOOK_MESSAGE(NewHudMsg);

	gHUD.AddHudElem(this);
	m_iFlags |= HUD_DRAW;
	return 1;
}

int CHudNewHud::VidInit(void)
{
	if (!m_iCharacterBG)
		m_iCharacterBG = R_LoadTextureUnique("resource/hud/hud_character_bg");

	if (!m_iCharacterBG_New_Bottom)
		m_iCharacterBG_New_Bottom = R_LoadTextureUnique("resource/hud/hud_character_bg_new_bottom");

	if (!m_iCharacterBG_New_Top)
		m_iCharacterBG_New_Top = R_LoadTextureUnique("resource/hud/hud_character_bg_new_top");

	if (!m_iNum_Character)
		m_iNum_Character = R_LoadTextureShared("resource/hud/hud_character_num");

	if (!m_iIcon_Speed)
		m_iIcon_Speed = R_LoadTextureUnique("resource/hud/hud_icon_speed");

	if (!m_iIcon_Damage)
		m_iIcon_Damage = R_LoadTextureUnique("resource/hud/hud_icon_damage");

	if(!m_iWeaponBG)
		m_iWeaponBG = R_LoadTextureUnique("resource/hud/hud_weapon_bg");

	return 1;
}

void CHudNewHud::InitHUDData(void)
{
	m_szLastModel = "";
}

int CHudNewHud::Draw(float flTime)
{
	if (!gHUD.m_pCvarDraw->value)
		return 0;

	if (gHUD.m_iHideHUDDisplay & HIDEHUD_ALL || gHUD.m_iHideHUDDisplay & HIDEHUD_HEALTH)
		return 0;

	if (gHUD.m_iIntermission || gEngfuncs.IsSpectateOnly())
		return 0;

	if (!(gHUD.m_iWeaponBits & (1 << (WEAPON_SUIT))))
		return 0;

	if (g_iUser1)
		return 0;

	if (gHUD.m_hudstyle->value != 2)
		return 0;

	DrawNewHudCharacterBG(flTime);
	return 1;
}

int CHudNewHud::DrawNewHudCharacterBG(float flTime)
{
	int iX = 0;
	int iY = ScreenHeight - 5;

	int iW = m_iCharacterBG_New_Bottom->w();
	int iH = m_iCharacterBG_New_Bottom->h();

	if (gHUD.m_iModRunning == MOD_ZB1 || gHUD.m_iModRunning == MOD_ZB2 || gHUD.m_iModRunning == MOD_ZB3 || gHUD.m_iModRunning == MOD_ZBZ)
	{
		int iSpeed;

		iW = m_iCharacterBG->w();
		iH = m_iCharacterBG->h();
		m_iCharacterBG->Draw2DQuadScaled(iX, iY - iH, iX + iW, iY - iH + iH);

		iSpeed = g_velocity.Length();

		int bitsShowState = PlayerClassManager().GetPlayerClass(gEngfuncs.GetLocalPlayer()->index).m_iBitsShowState;

		if ((bitsShowState & SHOW_SPEED) && (bitsShowState & SHOW_DAMAGE))
		{
			iX = 120;
			iY = iY - iH;
			iW = m_iIcon_Speed->w();
			iH = m_iIcon_Speed->h();
			m_iIcon_Speed->Draw2DQuadScaled(iX, iY + 5, iX + iW, iY + 5 + iH);

			iX = 180 - 30;

			CHudScoreBoardLegacy::DrawTexturedNumbers(*m_iNum_Character, m_iNum_CharacterC, iSpeed, iX, iY + 5, 0, 1, 1.0, 138, 193, 222);
			iX = 120;
			iY = iY + 5 + iH;
			iW = m_iIcon_Damage->w();
			iH = m_iIcon_Damage->h();
			m_iIcon_Damage->Draw2DQuadScaled(iX, iY + 5, iX + iW, iY + 5 + iH);

			iX = 180 - 30;

			int idx = gEngfuncs.GetLocalPlayer()->index;
			cl_entity_t* ent = gEngfuncs.GetEntityByIndex(idx);

			CHudScoreBoardLegacy::DrawTexturedNumbers(*m_iNum_Character, m_iNum_CharacterC, g_iDamage[idx] ? g_iDamage[idx] : 0, iX, iY + 5, 0, 1, 1.0, 255, 214, 110);
			CHudScoreBoardLegacy::DrawTexturedNumbers(*m_iNum_Character, m_iNum_CharacterC, g_iDamageTotal[idx] ? g_iDamageTotal[idx] : 0, iX + 45, iY + 5, 0, 1, 1.0, 218, 120, 120);

		}
		else if (bitsShowState & SHOW_SPEED)
		{
			iX = 120;
			iY = iY - iH;
			iW = m_iIcon_Speed->w();
			iH = m_iIcon_Speed->h();
			m_iIcon_Speed->Draw2DQuadScaled(iX, iY + 5, iX + iW, iY + 5 + iH);

			iX = 180 - 30;

			CHudScoreBoardLegacy::DrawTexturedNumbers(*m_iNum_Character, m_iNum_CharacterC, iSpeed, iX, iY + 5, 0, 1, 1.0, 138, 193, 222);
		}
		else if (bitsShowState & SHOW_DAMAGE)
		{
			iX = 120;
			iY = iY - iH;
			iW = m_iIcon_Damage->w();
			iH = m_iIcon_Damage->h();
			m_iIcon_Damage->Draw2DQuadScaled(iX, iY + 5, iX + iW, iY + 5 + iH);

			iX = 180 - 30;

			int idx = gEngfuncs.GetLocalPlayer()->index;
			cl_entity_t* ent = gEngfuncs.GetEntityByIndex(idx);

			CHudScoreBoardLegacy::DrawTexturedNumbers(*m_iNum_Character, m_iNum_CharacterC, g_iDamage[idx] ? g_iDamage[idx] : 0, iX, iY + 5, 0, 1, 1.0, 255, 214, 110);
			CHudScoreBoardLegacy::DrawTexturedNumbers(*m_iNum_Character, m_iNum_CharacterC, g_iDamageTotal[idx] ? g_iDamageTotal[idx] : 0, iX + 45, iY + 5, 0, 1, 1.0, 218, 120, 120);

		}
	}
	else
	{
		m_iCharacterBG_New_Bottom->Draw2DQuadScaled(iX, iY - iH, iX + iW, iY - iH + iH);
	}

	iX = ScreenWidth;
	iY = ScreenHeight - 5;

	iW = m_iCharacterBG_New_Bottom->w();
	iH = m_iCharacterBG_New_Bottom->h();


	m_iWeaponBG->Draw2DQuadScaled(iX - iW, iY - iH, iX, iY);

	int idx = gEngfuncs.GetLocalPlayer()->index;
	if (!g_PlayerInfoList[idx].model) {
		return 1;
	}

	if (m_szLastModel != g_PlayerInfoList[idx].model) {
		m_szLastModel = g_PlayerInfoList[idx].model;
		if (m_iCharacter)
			m_iCharacter = nullptr;

		std::string tmp = "resource/hud/portrait/hud_" + m_szLastModel;
		m_iCharacter = R_LoadTextureUnique(tmp.c_str());
	}

	if (m_iCharacter) {
		iW = m_iCharacter->w();
		iH = m_iCharacter->h();
		iX = 0;
		iY = ScreenHeight - 44;

		m_iCharacter->Draw2DQuadScaled(iX, iY - iH, iX + iW, iY - iH + iH);
	}
	return 1;
}

int CHudNewHud::MsgFunc_NewHudMsg(const char* pszName, int iSize, void* pbuf)
{
	BufferReader reader(pszName, pbuf, iSize);


	return 1;
}


}