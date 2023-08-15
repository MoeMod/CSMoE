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
#include "zbs_message.h"
#include "zbs_kill.h"
#include "zbs_msg.h"
#include "zbs_healthbar.h"
#include "zbs_rank.h"

#include "gamemode/zbs/zbs_const.h"

namespace cl {
	class CHudZBS::impl_t
		: public THudSubDispatcher<CHudZBSLevel, CHudZBSScoreBoard, CHudZBSKill, CHudZBSRoundClear, CHudZBSMsg, CHudZBSHealthBar, CHudZBSRank> {};

	DECLARE_MESSAGE(m_ZBS, ZBSTip)
	DECLARE_MESSAGE(m_ZBS, ZBSLevel)
	DECLARE_MESSAGE(m_ZBS, ZBSMsg)

	int CHudZBS::MsgFunc_ZBSMsg(const char* pszName, int iSize, void* pbuf)
	{
		BufferReader buf(pszName, pbuf, iSize);
		int iType = buf.ReadByte();
		switch (iType)
		{
		case 0:
		{
			int iID = buf.ReadShort();

			pimpl->get<CHudZBSMsg>().PushMsg(iID);
			break;
		}
		case 1:
		{
			int iClass = buf.ReadByte();
			int flPercent = buf.ReadShort();

			pimpl->get<CHudZBSHealthBar>().UpdateHealth(flPercent / 1000.0f, iClass);
			break;
		}
		case 2:
		{
			int iIndex = buf.ReadByte();
			int flData = buf.ReadShort();

			pimpl->get<CHudZBSRank>().PushRankData(iIndex, flData);
			break;
		}
		case 3:
		{
			pimpl->get<CHudZBSHealthBar>().UpdateHealth(0.0, -1);
			pimpl->get<CHudZBSRank>().PushRankData(-1, -1);

			break;
		}
		}


		return 1;
	}

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
	HOOK_MESSAGE(ZBSMsg);

	ZBS_LoadMessageData();

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
