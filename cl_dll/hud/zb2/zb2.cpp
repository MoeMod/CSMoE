/*
zb2.cpp - CSMoE Client HUD : Zombie Mod 2
Copyright (C) 2019 Moemod Hyakuya

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
#include "followicon.h"
#include "cl_util.h"
#include "draw_util.h"
#include "triangleapi.h"

#include "parsemsg.h"

#include "zb2.h"
#include "zb2_skill.h"

#include "gamemode/zb2/zb2_const.h"

#include <vector>

class CHudZB2::impl_t
	: public THudSubDispatcher<CHudZB2_Skill>
{
public:
	SharedTexture m_pTexture_RageRetina;
	std::vector<CHudRetina::MagicNumber> m_RetinaIndexes;
};

DECLARE_MESSAGE(m_ZB2, ZB2Msg)
int CHudZB2::MsgFunc_ZB2Msg(const char *pszName, int iSize, void *pbuf)
{
	BufferReader buf(pszName, pbuf, iSize);
	ZB2MessageType type = static_cast<ZB2MessageType>(buf.ReadByte());
	switch (type)
	{
	case ZB2_MESSAGE_HEALTH_RECOVERY:
	{
		pimpl->get<CHudZB2_Skill>().OnHealthRecovery();
		break;
	}
	case ZB2_MESSAGE_SKILL_INIT:
	{
		ZombieClassType zclass = ZOMBIE_CLASS_HUMAN;
		if(!buf.Eof())
			zclass = static_cast<ZombieClassType>(buf.ReadByte());
		ZombieSkillType skills[4]{};
		for (int i = 0; i < 4 && !buf.Eof(); ++i)
			skills[i] = static_cast<ZombieSkillType>(buf.ReadByte());
		pimpl->get<CHudZB2_Skill>().OnSkillInit(zclass, skills[0], skills[1], skills[2], skills[3]);

		// remove retinas...
		for (auto x : pimpl->m_RetinaIndexes)
		{
			gHUD.m_Retina.RemoveItem(x);
		}
		pimpl->m_RetinaIndexes.clear();
		break;
	}
	case ZB2_MESSAGE_SKILL_ACTIVATE:
	{
		ZombieSkillType type = static_cast<ZombieSkillType>(buf.ReadByte());
		float flHoldTime = buf.ReadShort();
		float flFreezeTime = buf.ReadShort();
		pimpl->get<CHudZB2_Skill>().OnSkillActivate(type, flHoldTime, flFreezeTime);
		if (type == ZOMBIE_SKILL_CRAZY || type == ZOMBIE_SKILL_CRAZY2)
			pimpl->m_RetinaIndexes.push_back(gHUD.m_Retina.AddItem(pimpl->m_pTexture_RageRetina, CHudRetina::RETINA_DRAW_TYPE_BLINK, flHoldTime));
		break;
	}
		
		
	}
	
	return 1;
}

int CHudZB2::Init(void)
{
	pimpl = new CHudZB2::impl_t;

	gHUD.AddHudElem(this);

	HOOK_MESSAGE(ZB2Msg);

	return 1;
}

int CHudZB2::VidInit(void)
{
	pimpl->for_each(&IBaseHudSub::VidInit);

	if(!pimpl->m_pTexture_RageRetina)
		pimpl->m_pTexture_RageRetina = R_LoadTextureShared("resource/zombi/zombicrazy");
	return 1;
}

int CHudZB2::Draw(float time)
{
	pimpl->for_each(&IBaseHudSub::Draw, time);
	return 1;
}

void CHudZB2::Think(void)
{
	pimpl->for_each(&IBaseHudSub::Think);
}

void CHudZB2::Reset(void)
{
	pimpl->for_each(&IBaseHudSub::Reset);
}

void CHudZB2::InitHUDData(void)
{
	pimpl->for_each(&IBaseHudSub::InitHUDData);
}

void CHudZB2::Shutdown(void)
{
	delete pimpl;
	pimpl = nullptr;
}