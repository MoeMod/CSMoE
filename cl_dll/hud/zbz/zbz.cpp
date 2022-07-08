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
#include "r_efx.h"
#include "r_studioint.h"
#include "event_api.h"

#include "parsemsg.h"

#include "hud_sub_impl.h"

#include "zbz.h"
#include "zbz_skill_info.h"
#include "zbz_ghosthunter.h"
#include "gamemode/mods_const.h"

namespace cl {
	extern engine_studio_api_t IEngineStudio;

class CHudZBZ::impl_t
	: public THudSubDispatcher<CHudZBZ_Skill, CHudZBZ_GhostHunter> {};

DECLARE_MESSAGE(m_ZBZ, ZBZMsg)

int CHudZBZ::MsgFunc_ZBZMsg(const char* pszName, int iSize, void* pbuf)
{
	BufferReader buf(pszName, pbuf, iSize);
	auto type = static_cast<ZBZMessageType>(buf.ReadByte());
	switch (type)
	{
	case ZBZ_MESSAGE_LEVEL_INFO:
	{
		int lv = buf.ReadByte();
		int exp = buf.ReadByte();
		int point = buf.ReadByte();
		pimpl->get<CHudZBZ_Skill>().UpdateLevel(lv, exp, point);
		break;
	}
	case ZBZ_MESSAGE_SKILL_ADD:
	{
		ZombieZSkillSkillId id = static_cast<ZombieZSkillSkillId>(buf.ReadByte());
		pimpl->get<CHudZBZ_Skill>().AddSkill(id);
		break;
	}
	case ZBZ_MESSAGE_SKILL_REMOVE:
	{
		ZombieZSkillSkillId id = static_cast<ZombieZSkillSkillId>(buf.ReadByte());
		pimpl->get<CHudZBZ_Skill>().RemoveSkill(id);
		break;
	}
	case ZBZ_MESSAGE_BURN:
	{
		int player = buf.ReadByte();
		float time = buf.ReadByte() / 10.0f;
		gHUD.m_HeadIcon.R_AttachTentToPlayer(player, gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/flame_burn01.spr"), Vector(0.0, 0.0, 0.0), time, TRUE, FTENT_PERSIST | FTENT_FADEOUT | FTENT_SPRANIMATELOOP, 1.0, kRenderTransAdd, 10.0f);
		break;
	}
	case ZBZ_MESSAGE_CONTACT_INFECTION_RING:
	{
		struct model_s* pModel = IEngineStudio.Mod_ForName("models/zombiez_contactinfection_ring.mdl", false);
		if (!pModel)
			break;

		int player = buf.ReadByte();
		cl_entity_t* pPlayer = gEngfuncs.GetEntityByIndex(player);
		if (!pPlayer)
		{
			gEngfuncs.Con_Printf("Couldn't get ClientEntity for %i\n", player);
			break;
		}
		TEMPENTITY* pEnt = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(pPlayer->origin, pModel);
		if (!pEnt)
			break;

		pEnt->entity.curstate.sequence = 0;
		pEnt->entity.curstate.frame = 0;
		pEnt->entity.curstate.framerate = 1.0;
		pEnt->clientIndex = player;
		pEnt->die = gHUD.m_flTime + 9999.9f;
		pEnt->flags |= FTENT_PLYRATTACHMENT | FTENT_CLIENTCUSTOM | FTENT_PERSIST;
		pEnt->callback = [](tempent_s* ent, float frametime, float currenttime)
		{
			if (!g_PlayerExtraInfo[ent->clientIndex].zombie || g_PlayerExtraInfo[ent->clientIndex].dead)
				ent->die = currenttime;
		};
		break;
	}
	case ZBZ_MESSAGE_CLAW:
	{
		struct model_s* pModel = IEngineStudio.Mod_ForName("models/zombiezclaw.mdl", false);
		if (!pModel)
			break;

		int player = buf.ReadByte();
		cl_entity_t* pPlayer = gEngfuncs.GetEntityByIndex(player);
		if (!pPlayer)
		{
			gEngfuncs.Con_Printf("Couldn't get ClientEntity for %i\n", player);
			break;
		}
		TEMPENTITY* pEnt = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(pPlayer->origin, pModel);
		if (!pEnt)
			break;

		pEnt->entity.curstate.sequence = 0;
		pEnt->entity.curstate.frame = 0;
		pEnt->entity.curstate.framerate = 1.0;
		pEnt->entity.curstate.animtime = gHUD.m_flTime;
		pEnt->entity.angles = pPlayer->angles;
		pEnt->die = gHUD.m_flTime + 0.6667f;
		pEnt->flags |= FTENT_FADEOUT;
		break;
	}
	case ZBZ_MESSAGE_WING:
	{
		struct model_s* pModel = IEngineStudio.Mod_ForName("models/zombiezwingfx.mdl", false);
		if (!pModel)
			break;

		int player = buf.ReadByte();
		cl_entity_t* pPlayer = gEngfuncs.GetEntityByIndex(player);
		if (!pPlayer)
		{
			gEngfuncs.Con_Printf("Couldn't get ClientEntity for %i\n", player);
			break;
		}
		TEMPENTITY* pEnt = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(pPlayer->origin, pModel);
		if (!pEnt)
			break;

		pEnt->entity.curstate.sequence = 0;
		pEnt->entity.curstate.frame = 0;
		pEnt->entity.curstate.framerate = 1.0;
		pEnt->entity.curstate.animtime = gHUD.m_flTime;
		pEnt->entity.angles = pPlayer->angles;
		pEnt->clientIndex = player;
		pEnt->die = gHUD.m_flTime + 2.0f;
		pEnt->flags |= FTENT_PLYRATTACHMENT | FTENT_CLIENTCUSTOM | FTENT_PERSIST;
		pEnt->callback = [](tempent_s* ent, float frametime, float currenttime)
		{
			if (g_PlayerExtraInfo[ent->clientIndex].dead)
			{
				ent->die = currenttime;
				return;
			}

			cl_entity_t* pPlayer = gEngfuncs.GetEntityByIndex(ent->clientIndex);
			if (pPlayer)
			{
				ent->entity.origin = pPlayer->origin;
				ent->entity.angles = pPlayer->angles;
			}
		};
		break;
	}
	case ZBZ_MESSAGE_GHOSTHUNTER:
	{
		int type = buf.ReadByte();
		float time = buf.ReadByte();
		pimpl->get<CHudZBZ_GhostHunter>().SetGhostHunterClass(type);
		pimpl->get<CHudZBZ_GhostHunter>().SetImageDrawTime(time);
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


bool CHudZBZ::CheckHasSkill(ZombieZSkillSkillId id)
{
	return pimpl->get<CHudZBZ_Skill>().CheckHasSkill(id);
}

}