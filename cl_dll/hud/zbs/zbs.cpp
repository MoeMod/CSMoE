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

#include "parsemsg.h"

#include "hud_sub_impl.h"

#include "zbs.h"
#include "zbs_level.h"
#include "zbs_scoreboard.h"
#include "zbs_roundclear.h"

#include "zbs_kill.h"

#include "gamemode/zbs/zbs_const.h"

namespace cl {

class CHudZBS::impl_t
	: public THudSubDispatcher<CHudZBSLevel, CHudZBSScoreBoard, CHudZBSKill, CHudZBSRoundClear> {};

DECLARE_MESSAGE(m_ZBS, ZBSTip)
DECLARE_MESSAGE(m_ZBS, ZBSLevel)

int CHudZBS::MsgFunc_ZBSTip(const char *pszName, int iSize, void *pbuf)
{
	BufferReader buf(pszName, pbuf, iSize);
	ZBSTipType type = static_cast<ZBSTipType>(buf.ReadByte());
	switch (type)
	{
	case ZBS_TIP_KILL:
		pimpl->get<CHudZBSKill>().OnKillMessage();
		break;
	case ZBS_TIP_ROUNDCLEAR:
		pimpl->get<CHudZBSRoundClear>().OnRoundClear();
		break;
	case ZBS_TIP_ROUNDFAIL:
		pimpl->get<CHudZBSRoundClear>().OnRoundFail();
		break;
	}

	return 1;
}

int CHudZBS::MsgFunc_ZBSLevel(const char *pszName, int iSize, void *pbuf)
{
	BufferReader buf(pszName, pbuf, iSize);
	int type = buf.ReadByte(); // reserved.
	int hp = buf.ReadByte();
	int att = buf.ReadByte();
	int wall = buf.ReadByte();

	pimpl->get<CHudZBSLevel>().UpdateLevel(hp, att, wall);

	return 1;
}

int CHudZBS::Init(void)
{
	pimpl = new CHudZBS::impl_t;

	gHUD.AddHudElem(this);

	HOOK_MESSAGE(ZBSTip);
	HOOK_MESSAGE(ZBSLevel);

	return 1;
}

int CHudZBS::VidInit(void)
{
	pimpl->for_each(&IBaseHudSub::VidInit);
	return 1;
}

int CHudZBS::Draw(float time)
{
	pimpl->for_each(&IBaseHudSub::Draw, time);
	return 1;
}

void CHudZBS::Think(void)
{
	pimpl->for_each(&IBaseHudSub::Think);
}

void CHudZBS::Reset(void)
{
	pimpl->for_each(&IBaseHudSub::Reset);
}

void CHudZBS::InitHUDData(void)
{
	pimpl->for_each(&IBaseHudSub::InitHUDData);
}

void CHudZBS::Shutdown(void)
{
	delete pimpl;
	pimpl = nullptr;
}

}
