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
#pragma once
#include "hud_sub.h"
namespace cl {
extern vec3_t g_velocity;

class CHudNewHud : public CHudBase
{
public:
	CHudNewHud();
	int Init(void);
	int VidInit(void);
	int Draw(float flTime);
	int DrawNewHudCharacterBG(float flTime);

	void InitHUDData(void);
	CHudMsgFunc(NewHudMsg);

private:
	UniqueTexture m_iCharacterBG;
	UniqueTexture m_iCharacterBG_New_Bottom;
	UniqueTexture m_iCharacterBG_New_Top;

	SharedTexture m_iNum_Character;

	wrect_t m_iNum_CharacterC[10];

	UniqueTexture m_iIcon_Speed;
	UniqueTexture m_iIcon_Damage;

	UniqueTexture m_iWeaponBG;


	UniqueTexture m_iCharacter;
	std::string m_szLastModel;
};
}