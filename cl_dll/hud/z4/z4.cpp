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
#include "z4_status.h"
#include "z4_tips.h"

#include <vector>

class CHudZ4::impl_t
	: public THudSubDispatcher<CHudZ4Energy, CHudZ4Scoreboard, CHudZ4Status, CHudZ4Tips>
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
	
	case Z4_MESSAGE_NIGHTTIME:
	{
		int start = buf.ReadByte();
		int end = buf.ReadByte();
		pimpl->get<CHudZ4Tips>().SetNightTime(start, end);
		pimpl->get<CHudZ4Status>().HideAll();
		break;
	}
	case Z4_MESSAGE_ENERGY:
	{
		int power = buf.ReadByte();
		float flashtime = buf.ReadByte() / 10.0f;
		pimpl->get<CHudZ4Energy>().SetPower(power, flashtime);
		pimpl->get<CHudZ4Status>().SetPower(power);
		break;
	}
	case Z4_MESSAGE_DAMAGE:
	{
		int a = buf.ReadByte();
		int b = buf.ReadByte();
		pimpl->get<CHudZ4Scoreboard>().SetDamage(a * 255 + b);
		break;
	}
	case Z4_MESSAGE_STATUS:
	{
		int slot = buf.ReadByte();
		Z4Status id = static_cast<Z4Status>(buf.ReadByte());
		Z4StatusIconDraw status = static_cast<Z4StatusIconDraw>(buf.ReadByte());
		pimpl->get<CHudZ4Status>().AddIcon(slot, id, status);
		break;
	}
	case Z4_MESSAGE_ZCLASS:
	{
		Z4ZClassType zclass = static_cast<Z4ZClassType>(buf.ReadByte());
		pimpl->get<CHudZ4Energy>().SetZClass(zclass);
		break;
	}
	case Z4_MESSAGE_SKILL:
	{
		Z4ZSkillType skill = static_cast<Z4ZSkillType>(buf.ReadByte());
		int activate = buf.ReadByte();
		if (skill == Z4_SKILL_DASH)
		{
			if (g_PlayerExtraInfo[gEngfuncs.GetLocalPlayer()->index].zombie)
			{
				switch (pimpl->get<CHudZ4Energy>().GetZClass())
				{
				case Z4_CLASS_NORMAL:
					pimpl->get<CHudZ4Status>().AddIcon(7, Z4_SPEEDUP, activate ? Z4_ICON_SHOW : Z4_ICON_FLASH2);
					break;
				case Z4_CLASS_LIGHT:
					pimpl->get<CHudZ4Status>().AddIcon(7, Z4_SPEEDUP, activate ? Z4_ICON_SHOW : Z4_ICON_FLASH2);
					pimpl->get<CHudZ4Status>().AddIcon(9, Z4_DEFENSEDOWN, activate ? Z4_ICON_SHOW : Z4_ICON_FLASH2);
					break;
				case Z4_CLASS_HEAVY:
					pimpl->get<CHudZ4Status>().AddIcon(7, Z4_SPEEDDOWN, activate ? Z4_ICON_SHOW : Z4_ICON_FLASH2);
					pimpl->get<CHudZ4Status>().AddIcon(9, Z4_DEFENSEUP, activate ? Z4_ICON_SHOW : Z4_ICON_FLASH2);
					break;
				case Z4_CLASS_HIDE:
					pimpl->get<CHudZ4Status>().AddIcon(7, Z4_SPEEDUP, activate ? Z4_ICON_SHOW : Z4_ICON_FLASH2);
					break;
				case Z4_CLASS_HUMPBACK:
					pimpl->get<CHudZ4Status>().AddIcon(7, Z4_SPEEDUP, activate ? Z4_ICON_SHOW : Z4_ICON_FLASH2);
					pimpl->get<CHudZ4Status>().AddIcon(9, Z4_DEFENSEUP, activate ? Z4_ICON_SHOW : Z4_ICON_FLASH2);
					break;
				default:
					break;
				}
			}
			else
			{
				pimpl->get<CHudZ4Status>().AddIcon(7, Z4_SPEEDUP, activate ? Z4_ICON_SHOW : Z4_ICON_FLASH2);

			}
		}
		else if (skill == Z4_SKILL_ACCSHOOT)
		{
			pimpl->get<CHudZ4Status>().AddIcon(1, Z4_CRITICALSHOT, activate ? Z4_ICON_SHOW : Z4_ICON_FLASH2);
		}
		break;
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

void CHudZ4::SetStatus(int slot, Z4Status id, Z4StatusIconDraw status)
{
	pimpl->get<CHudZ4Status>().AddIcon(slot, id, status);
}