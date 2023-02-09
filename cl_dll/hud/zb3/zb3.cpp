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
#include "cl_util.h"

#include "parsemsg.h"

#include "hud_sub_impl.h"

#include "zb3.h"
#include "zb3_morale.h"
#include "zb3_rage.h"
#include "draw_util.h"
#include "calcscreen.h"
#include "gamemode/mods_const.h"
#include "gamemode/zb3/zb3_const.h"

#include <vector>
namespace cl {

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
		if (gHUD.m_iModRunning == MOD_ZBZ && morale_level == 255)
			pimpl->get<CHudZB3Morale>().SetEnabled(true);
		else
			pimpl->get<CHudZB3Morale>().UpdateLevel(morale_type, morale_level);

		break;
	}
	case ZB3_MESSAGE_RAGE:
	{
		auto zombie_level = static_cast<ZombieLevel>(buf.ReadByte());
		int percent = buf.ReadByte();
		if (gHUD.m_iModRunning == MOD_ZBZ && percent == 255)
		{
			pimpl->get<CHudZB3Rage>().SetEnabled(true);
		}
		else
		{
			pimpl->get<CHudZB3Rage>().SetZombieLevel(zombie_level);
			pimpl->get<CHudZB3Rage>().SetPercent(percent);
		}
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

	R_InitTexture(m_iHero, "resource/helperhud/hero_s");

	return 1;
}

int CHudZB3::Draw(float time)
{
	pimpl->for_each(&IBaseHudSub::Draw, time);
	DrawHeroIcon();

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

int CHudZB3::DrawHeroIcon()
{
	if (!gHUD.m_pCvarDraw->value)
		return 0;

	if (gHUD.m_iIntermission || gEngfuncs.IsSpectateOnly())
		return 0;

	if (g_iUser1)
		return 0;

	if (!(gHUD.m_iWeaponBits & (1 << (WEAPON_SUIT))))
		return 0;

	int x = 0, y = 0;

	for (int i = 0; i < 33; i++)
	{
		if (!g_PlayerExtraInfo[i].vip)
			continue;

		if (g_PlayerExtraInfo[i].zombie || g_PlayerExtraInfo[i].dead)
			continue;

		cl_entity_t* ent = gEngfuncs.GetEntityByIndex(i);

		cl_entity_t* pLocal = gEngfuncs.GetLocalPlayer();

		if (!ent)
			continue;

		int iDistance = (ent->origin - pLocal->origin).Length() * 0.0254f;
	
		float xyScreen[2];
		if (CalcScreen(ent->origin, xyScreen))
		{
			if (iDistance > 10 && iDistance < 30)
			{
				m_iHero->Draw2DQuadScaled(xyScreen[0] - 18, xyScreen[1] - 18, xyScreen[0] + 19, xyScreen[1] + 19);

				char szBuffer[16];
				sprintf(szBuffer, "[%im]", iDistance);

				int textlen = DrawUtils::HudStringLen(szBuffer);
				int r = 182, g = 167, b = 254;
				DrawUtils::ScaleColors(r, g, b, 255);
				DrawUtils::DrawHudString(xyScreen[0] - textlen * 0.5f, xyScreen[1] + 25, gHUD.m_scrinfo.iWidth, szBuffer, r, g, b);
			}
		}
	}

	return 1;


}
}