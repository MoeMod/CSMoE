/*
zb3.cpp - CSMoE Client HUD : Zombie Hero
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

#include "zb3.h"
#include "zb3_morale.h"
#include "zb3_rage.h"

#include "gamemode/zb3/zb3_const.h"
#include "player/player_zombie.h"

#include <vector>

class CHudZB3::impl_t
	: public THudSubDispatcher<CHudZB3Morale, CHudZB3Rage>
{
public:
};

DECLARE_MESSAGE(m_ZB3, ZB3Msg)
int CHudZB3::MsgFunc_ZB3Msg(const char *pszName, int iSize, void *pbuf)
{
	BufferReader buf(pszName, pbuf, iSize);
	auto type = static_cast<ZB3MessageType>(buf.ReadByte());
	switch (type)
	{
	case ZB3_MESSAGE_MORALE:
	{
		auto morale_type = static_cast<ZB3HumanMoraleType_e>(buf.ReadByte());
		int morale_level = buf.ReadByte();
		pimpl->get<CHudZB3Morale>().UpdateLevel(morale_type, morale_level);
		break;
	}
	case ZB3_MESSAGE_RAGE:
	{
		auto zombie_level = static_cast<ZombieLevel>(buf.ReadByte());
		int percent = buf.ReadByte();
		pimpl->get<CHudZB3Rage>().SetZombieLevel(zombie_level);
		pimpl->get<CHudZB3Rage>().SetPercent(percent);
		break;
	}
	}
	
	return 1;
}

int CHudZB3::Init(void)
{
	pimpl = new CHudZB3::impl_t;

	gHUD.AddHudElem(this);

	HOOK_MESSAGE(ZB3Msg);

	return 1;
}

int CHudZB3::VidInit(void)
{
	pimpl->for_each(&IBaseHudSub::VidInit);

	return 1;
}

int CHudZB3::Draw(float time)
{
	pimpl->for_each(&IBaseHudSub::Draw, time);
	return 1;
}

void CHudZB3::Think(void)
{
	pimpl->for_each(&IBaseHudSub::Think);
}

void CHudZB3::Reset(void)
{
	pimpl->for_each(&IBaseHudSub::Reset);
}

void CHudZB3::InitHUDData(void)
{
	pimpl->for_each(&IBaseHudSub::InitHUDData);
}

void CHudZB3::Shutdown(void)
{
	delete pimpl;
	pimpl = nullptr;
}