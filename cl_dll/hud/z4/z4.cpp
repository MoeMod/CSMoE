/*
z4.cpp - CSMoE Client HUD : Zombie 4
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

#include "hud_sub_impl.h"

#include "z4.h"
#include "z4_energy.h"
#include "z4_scoreboard.h"

#include "gamemode/z4/z4_const.h"

#include <vector>

class CHudZ4::impl_t
	: public THudSubDispatcher<CHudZ4Energy, CHudZ4Scoreboard>
{
public:
};

DECLARE_MESSAGE(m_ZB4, Z4Msg)
int CHudZ4::MsgFunc_Z4Msg(const char *pszName, int iSize, void *pbuf)
{
	BufferReader buf(pszName, pbuf, iSize);
	auto type = static_cast<Z4MessageType>(buf.ReadByte());
	switch (type)
	{
	/*
	case 4:
	{
		int slot = READ_BYTE();
		int id = READ_BYTE();
		int status = READ_BYTE();
		g_CHudZB4UI.m_Icon.AddIcon(slot, id, status);
		break;
	}
	case 5:
	{
		int start = READ_BYTE();
		int end = READ_BYTE();
		g_CHudZB4UI.m_Tip.SetNightTime(start, end);
		break;
	}*/
	case Z4_MESSAGE_ENERGY:
	{
		int power = buf.ReadByte();
		float flashtime = buf.ReadByte() / 10.0f;
		//g_CHudZB4UI.m_Icon.SetPower(power);
		pimpl->get<CHudZ4Energy>().SetPower(power, flashtime);
		break;
	}
	case Z4_MESSAGE_DAMAGE:
	{
		int a = buf.ReadByte();
		int b = buf.ReadByte();
		pimpl->get<CHudZ4Scoreboard>().SetDamage(a * 255 + b);
		break;
	}
	case Z4_MESSAGE_ZCLASS:
	{
		int zclassid = buf.ReadByte();
		int iSkillConsume = buf.ReadByte();
		pimpl->get<CHudZ4Energy>().SetZbSkill(zclassid, iSkillConsume);
	}
	}
	
	return 1;
}

int CHudZ4::Init(void)
{
	pimpl = new CHudZ4::impl_t;

	gHUD.AddHudElem(this);

	HOOK_MESSAGE(Z4Msg);

	return 1;
}

int CHudZ4::VidInit(void)
{
	pimpl->for_each(&IBaseHudSub::VidInit);

	return 1;
}

int CHudZ4::Draw(float time)
{
	pimpl->for_each(&IBaseHudSub::Draw, time);
	return 1;
}

void CHudZ4::Think(void)
{
	pimpl->for_each(&IBaseHudSub::Think);
}

void CHudZ4::Reset(void)
{
	pimpl->for_each(&IBaseHudSub::Reset);
}

void CHudZ4::InitHUDData(void)
{
	pimpl->for_each(&IBaseHudSub::InitHUDData);
}

void CHudZ4::Shutdown(void)
{
	delete pimpl;
	pimpl = nullptr;
}

bool CHudZ4::ActivateSkill(int iSlot)
{
	if (iSlot == 5)
	{
		gEngfuncs.pfnClientCmd("z4_skill5");
		return true;
	}
	else if (iSlot == 6)
	{
		gEngfuncs.pfnClientCmd("z4_skill6");
		return true;
	}

	return false;
}
