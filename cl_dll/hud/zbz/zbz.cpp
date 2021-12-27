/*
zbs.cpp - CSMoE Client HUD : Zombie Scenerio
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
#include "draw_util.h"
#include "triangleapi.h"

#include "parsemsg.h"

#include "hud_sub_impl.h"

#include "zbz.h"
#include "zbz_skill_info.h"

class CHudZBZ::impl_t
	: public THudSubDispatcher<CHudZBZ_Skill> {};

DECLARE_MESSAGE(m_ZBZ, ZBZMsg)

int CHudZBZ::MsgFunc_ZBZMsg(const char* pszName, int iSize, void* pbuf)
{
	BufferReader buf(pszName, pbuf, iSize);
	auto type = static_cast<ZBZMessageType>(buf.ReadByte());
	switch (type)
	{
	case ZB3_MESSAGE_LEVEL_INFO:
	{
		int lv = buf.ReadByte();
		int exp = buf.ReadByte();
		int point = buf.ReadByte();
		pimpl->get<CHudZBZ_Skill>().UpdateLevel(lv, exp, point);
		break;
	}
	case ZB3_MESSAGE_SKILL_ADD:
	{
		ZombieZSkillSkillId id = static_cast<ZombieZSkillSkillId>(buf.ReadByte());
		pimpl->get<CHudZBZ_Skill>().AddSkill(id);
		break;
	}
	case ZB3_MESSAGE_SKILL_REMOVE:
	{
		ZombieZSkillSkillId id = static_cast<ZombieZSkillSkillId>(buf.ReadByte());
		pimpl->get<CHudZBZ_Skill>().RemoveSkill(id);
		break;
	}
	}

	return 1;
}

int CHudZBZ::Init(void)
{
	pimpl = new CHudZBZ::impl_t;

	gHUD.AddHudElem(this);

	HOOK_MESSAGE(ZBZMsg);

	return 1;
}

int CHudZBZ::VidInit(void)
{
	pimpl->for_each(&IBaseHudSub::VidInit);
	return 1;
}

int CHudZBZ::Draw(float time)
{
	pimpl->for_each(&IBaseHudSub::Draw, time);
	return 1;
}

void CHudZBZ::Think(void)
{
	pimpl->for_each(&IBaseHudSub::Think);
}

void CHudZBZ::Reset(void)
{
	pimpl->for_each(&IBaseHudSub::Reset);
}

void CHudZBZ::InitHUDData(void)
{
	pimpl->for_each(&IBaseHudSub::InitHUDData);
}

void CHudZBZ::Shutdown(void)
{
	delete pimpl;
	pimpl = nullptr;
}

