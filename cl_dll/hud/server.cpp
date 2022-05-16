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
#include "hud.h"
#include "com_model.h"
#include "r_studioint.h"
#include "cl_util.h"
#include "parsemsg.h"
#include <string.h>
#include <studio.h>
#include "events.h"
#include "r_efx.h"
#include "gamemode/mods_const.h"
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "weapons.h"
#include "cs_wpn/bte_weapons.h"

namespace cl {
	extern engine_studio_api_t IEngineStudio;

enum
{
	HUMAN_SKILL_KNIFE2X,
	HUMAN_SKILL_HEADSHOT,
	ZOMBIE_SKILL_HEAL,
	ZOMBIE_SKILL_HEAL_HEAD,
	CANNON_FLAME_BURN,
	HUNTBOW_DMGREITERATION,
	HUNTBOW_MARKZOMBIE,
	TELEPORT_MARKEF,
	HOLYBOMB_BURN,
	LANCE_HIT,
	ZOMBIE_SKILL_PILE,
};

void CreateAttachedEntitiesToPlayer(int entity, int type);
void R_AttachTentToEntity(int entity, int modelIndex, vec3_t offset, float life, int additive, int flags, float scale, int rendermode, float framerate);
void CreateBalrog11CannonFollowEnt(TEMPENTITY* prev, vec3_t origin, int iAimingEntity, int iType);
void CreateFollowEnt(TEMPENTITY* prev, vec3_t origin, int iAimingEntity, int iType);
void CreateBalrog11CannonSingleProjectile(TEMPENTITY* prev, vec3_t origin, int iAimingEntity, int iType);

int CHud::MsgFunc_MPToCL(const char* pszName, int iSize, void* pbuf)
{
	int iType;
	BufferReader reader(pszName, pbuf, iSize);
	int arg1;
	int arg2;
	Vector pos;
	iType = reader.ReadByte();
	switch (iType)
	{
	case 0:
	{
		int length = reader.ReadShort();
		for (int i = 0; i < length; i++)
		{
			// {"weapon_usp", "USP45", 500, PISTOL_SLOT, UNASSIGNED, BUY_PISTOL, 1},
			MoEWeaponBuyInfo_s info;
			info.pszClassName = reader.ReadString();
			info.pszDisplayName = reader.ReadString();
			info.iCost = reader.ReadShort();
			info.iSlot = (InventorySlotType)reader.ReadByte();
			info.iTeam = (TeamName)reader.ReadByte();
			info.iMenu = (MoEWeaponBuyType)reader.ReadByte();
			info.iScore = 1;
			gHUD.m_WeaponBuyInfo.push_back(info);
		}
		break;
	}
	case 1:
	{
		CBasePlayerWeapon* pActiveBTEWeapon = BTEClientWeapons().GetActiveWeaponEntity();
		if (pActiveBTEWeapon && pActiveBTEWeapon->m_iId == WEAPON_M95TIGER)
			pActiveBTEWeapon->pev->iuser1 = reader.ReadByte();
		break;
	}
	case 2:
	{
		pos.x = reader.ReadCoord();
		pos.y = reader.ReadCoord();
		pos.z = reader.ReadCoord();
		arg1 = reader.ReadShort();
		arg2 = reader.ReadByte();
		CreateBalrog11CannonSingleProjectile(NULL, pos, arg1, arg2);
		//CreateBalrog11CannonFollowEnt(NULL, pos, arg1, arg2);
		break;
	}
	case 3:
	{
		pos.x = reader.ReadCoord();
		pos.y = reader.ReadCoord();
		pos.z = reader.ReadCoord();
		arg1 = reader.ReadShort();
		arg2 = reader.ReadByte();
		CreateFollowEnt(NULL, pos, arg1, arg2);
		break;
	}
	case 4:
	{
		arg1 = reader.ReadShort();
		arg2 = reader.ReadByte();

		CreateAttachedEntitiesToPlayer(arg1, arg2);

		break;

	}
	case 15:
		switch (reader.ReadByte())
		{
		case 1:
			gEngfuncs.pfnPlaySoundByNameAtPitch("zombi/TD_Buff.wav", 1.0, 100);
			break;

		case 44:
			gEngfuncs.pfnPlaySoundByNameAtLocation("weapons/sgmissile_reload.wav", VOL_NORM, gHUD.m_vecOrigin);
			break;
		}

		break;
	case 79:	// stop sound
		arg1 = reader.ReadShort();
		arg2 = reader.ReadShort();

		gEngfuncs.pEventAPI->EV_StopSound(arg1, arg2, reader.ReadString());

		break;
	}
	return 1;
}


void FollowThink(tempent_s* ent, float frametime, float currenttime)
{
	if (ent->entity.baseline.fuser1 > ent->entity.curstate.scale)
		ent->entity.curstate.scale += 0.03;

	static cl_entity_t* pEntity;
	pEntity = gEngfuncs.GetEntityByIndex(ent->entity.baseline.iuser1);

	if (!pEntity)	//preent
	{
		ent->die = currenttime;
		return;
	}

	if (ent->die - currenttime <= frametime * 100 && !ent->entity.curstate.iuser1)
	{
		CreateBalrog11CannonSingleProjectile(ent, pEntity->origin, pEntity->index, ent->entity.curstate.iuser2);
		ent->entity.curstate.iuser1 = 1;
	}
}

void DefaultFramerateTouch(tempent_s* ent, pmtrace_t* tr)
{
	ent->entity.baseline.origin *= 0.01f;
	ent->entity.curstate.framerate = 60;
}

void CreateBalrog11CannonSingleProjectile(TEMPENTITY* prev, vec3_t origin, int iAimingEntity, int iType)
{
	cl_entity_t* ent = gEngfuncs.GetEntityByIndex(iAimingEntity);	//the msgent
	if (!ent)
		return;
	if (prev && (prev->entity.origin - origin).Length() < 5)	// Why not other numbers??	//2nd
		return;
	static int iModelIndex;

	switch (iType)
	{
	case 0:
		iModelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/flame_puff01.spr"); break;
	case 1:
		iModelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/ef_hwater.spr"); break;
	case 2:
		iModelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/ef_sgmissile_line.spr"); break;	
	case 3:
		iModelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/ef_sgmissilem_line.spr"); break;
	case 4:
		iModelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/ef_sgmissileex_line.spr"); break;
	}

	TEMPENTITY* te = gEngfuncs.pEfxAPI->R_DefaultSprite(origin, iModelIndex, 48);
	if (te)
	{
		te->flags |= FTENT_CLIENTCUSTOM | FTENT_COLLIDEKILL;
		te->entity.angles.y = gEngfuncs.pfnRandomFloat(-512, 511);
		te->entity.angles.x = gEngfuncs.pfnRandomFloat(-256, 255);
		te->entity.angles.z = gEngfuncs.pfnRandomFloat(-256, 255);
		te->flags |= FTENT_ROTATE;
		te->entity.curstate.scale = 0.01;
		te->entity.baseline.fuser1 = 0.3;
		te->entity.curstate.iuser2 = iType;
		te->die = gHUD.m_flTime + 1.0;	//0.75 default
		te->callback = FollowThink;
		te->hitcallback = DefaultFramerateTouch;	
		te->entity.curstate.rendermode = kRenderTransAdd;
		te->entity.curstate.renderamt = 40.0f;
		te->entity.curstate.renderfx = kRenderFxNone;

		te->entity.baseline.origin = Vector(0, 0, 0);
		te->entity.baseline.iuser1 = ent->index;
		te->entity.curstate.iuser1 = 0;
		te->clientIndex = ent->curstate.owner;
	}
}

void CreateBalrog11CannonFollowEnt(TEMPENTITY* prev, vec3_t origin, int iAimingEntity, int iType)
{
	cl_entity_t* ent = gEngfuncs.GetEntityByIndex(iAimingEntity);
	if (!ent)
		return;
	static int iModelIndex;
	switch (iType)
	{
	case 0:
		iModelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/flame_puff01.spr"); break;
	case 1:
		iModelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/ef_hwater.spr"); break;
	case 2:
		iModelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/ef_sgmissile_line.spr"); break;
	}

	TEMPENTITY* pEnt = gEngfuncs.pEfxAPI->R_DefaultSprite(origin, iModelIndex, 48);
	if (pEnt)
	{
		pEnt->flags |= FTENT_CLIENTCUSTOM | FTENT_COLLIDEKILL;
		pEnt->entity.angles.y = gEngfuncs.pfnRandomFloat(-512, 511);
		pEnt->entity.angles.x = gEngfuncs.pfnRandomFloat(-256, 255);
		pEnt->entity.angles.z = gEngfuncs.pfnRandomFloat(-256, 255);
		pEnt->flags |= FTENT_ROTATE;
		pEnt->entity.curstate.scale = 0.01;
		pEnt->entity.baseline.fuser1 = 0.3;
		pEnt->entity.curstate.iuser2 = iType;

		pEnt->entity.curstate.rendermode = kRenderTransAdd;
		pEnt->entity.curstate.renderamt = 40.0f;
		pEnt->entity.curstate.renderfx = kRenderFxNone;

		pEnt->entity.baseline.origin = Vector(0, 0, 0);
		pEnt->entity.baseline.iuser1 = ent->index;
		pEnt->entity.curstate.iuser1 = 0;
		pEnt->clientIndex = ent->curstate.owner;

		pEnt->callback = [](tempent_s* ent, float frametime, float currenttime) {
			if (ent->entity.baseline.fuser1 > ent->entity.curstate.scale)
				ent->entity.curstate.scale += 0.03;

			cl_entity_t* pEntity;
			pEntity = gEngfuncs.GetEntityByIndex(ent->entity.baseline.iuser1);

			if (!pEntity)
			{
				ent->die = currenttime;
				return;
			}

			if (ent->die - currenttime <= frametime * 100 && !ent->entity.curstate.iuser1)
			{
				if ((ent->entity.origin - pEntity->origin).Length() < 5)
					return;
				CreateBalrog11CannonFollowEnt(ent, pEntity->origin, pEntity->index, ent->entity.curstate.iuser2);
				ent->entity.curstate.iuser1 = 1;
			}
		};
		pEnt->hitcallback = [](tempent_s* ent, pmtrace_s* ptr) {
			ent->entity.baseline.origin *= 0.01f;
			ent->entity.curstate.framerate = 60;

		};
	}
}


void CreateFollowEnt(TEMPENTITY* prev, vec3_t origin, int iAimingEntity, int iType)
{
	cl_entity_t* ent = gEngfuncs.GetEntityByIndex(iAimingEntity);
	if (!ent)
		return;
	static int iModelIndex;
	if (iType)
		iModelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/ef_hwater.spr");
	else
		iModelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/flame_puff01.spr");

	TEMPENTITY* pEnt = gEngfuncs.pEfxAPI->R_DefaultSprite(origin, iModelIndex, 48);
	if (pEnt)
	{
		pEnt->flags |= FTENT_CLIENTCUSTOM | FTENT_COLLIDEKILL;
		pEnt->entity.angles.y = gEngfuncs.pfnRandomFloat(-512, 511);
		pEnt->entity.angles.x = gEngfuncs.pfnRandomFloat(-256, 255);
		pEnt->entity.angles.z = gEngfuncs.pfnRandomFloat(-256, 255);
		pEnt->flags |= FTENT_ROTATE;
		pEnt->entity.curstate.scale = 0.01;
		pEnt->entity.baseline.fuser1 = 0.3;
		pEnt->entity.curstate.iuser2 = iType;

		pEnt->entity.curstate.rendermode = kRenderTransAdd;
		pEnt->entity.curstate.renderamt = 40.0f;
		pEnt->entity.curstate.renderfx = kRenderFxNone;

		pEnt->entity.baseline.origin = Vector(0, 0, 0);
		pEnt->entity.baseline.iuser1 = ent->index;
		pEnt->entity.curstate.iuser1 = 0;
		pEnt->clientIndex = ent->curstate.owner;

		pEnt->callback = [](tempent_s* ent, float frametime, float currenttime) {
			if (ent->entity.baseline.fuser1 > ent->entity.curstate.scale)
				ent->entity.curstate.scale += 0.03;

			cl_entity_t* pEntity;
			pEntity = gEngfuncs.GetEntityByIndex(ent->entity.baseline.iuser1);

			if (!pEntity)
			{
				ent->die = currenttime;
				return;
			}

			if (ent->die - currenttime <= frametime * 100 && !ent->entity.curstate.iuser1)
			{
				if ((ent->entity.origin - pEntity->origin).Length() < 5)
					return;
				CreateFollowEnt(ent, pEntity->origin, pEntity->index, ent->entity.curstate.iuser2);
				ent->entity.curstate.iuser1 = 1;
			}
		};
		pEnt->hitcallback = [](tempent_s* ent, pmtrace_s* ptr) {
			ent->entity.baseline.origin *= 0.01f;
			ent->entity.curstate.framerate = 60;

		};
	}
}

void CreateAttachedEntitiesToPlayer(int entity, int type)
{
	int flags = FTENT_PERSIST | FTENT_FADEOUT | FTENT_SPRANIMATELOOP;
	switch (type)
	{
	case HUMAN_SKILL_KNIFE2X:
	{
		R_AttachTentToEntity(entity, gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/zb_meleeup.spr"),
			Vector(0.0, 0.0, 30.0), 10.0, TRUE, flags, 1.0, kRenderTransAdd, 1.0);
		break;
	}
	case HUMAN_SKILL_HEADSHOT:
	{
		R_AttachTentToEntity(entity, gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/zb_skill_headshot.spr"),
			Vector(0.0, 0.0, 30.0), 4.5, TRUE, flags, 1.0, kRenderTransAdd, 1.0);
		break;
	}
	case ZOMBIE_SKILL_HEAL:
	{
		R_AttachTentToEntity(entity, gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/zombihealer.spr"),
			Vector(0.0, 0.0, 25.0), 1.0, TRUE, flags, 1.0, kRenderTransAdd, 10.0);
		break;
	}
	case ZOMBIE_SKILL_HEAL_HEAD:
	{
		R_AttachTentToEntity(entity, gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/zombiheal_head.spr"),
			Vector(0.0, 0.0, 25.0), 1.0, TRUE, flags, 1.0, kRenderTransAdd, 10.0);
		break;
	}
	case CANNON_FLAME_BURN:
	{
		R_AttachTentToEntity(entity, gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/flame_burn01.spr"),
			Vector(Com_RandomFloat(-5.0, 5.0), Com_RandomFloat(-5.0, 5.0), 0.0), 3, TRUE, flags, 0.3, kRenderTransAdd, 10.0);
		break;
	}
	case HOLYBOMB_BURN:
	{
		R_AttachTentToEntity(entity, gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/holybomb_burn.spr"),
			Vector(0.0, 0.0, 30.0), 5.0, TRUE, flags, 0.9, kRenderTransAdd, 10.0);
		break;
	}
	case LANCE_HIT:
	{
		flags = FTENT_PERSIST;
		R_AttachTentToEntity(entity, gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/ef_lance_hit.spr"),
			Vector(0.0, 0.0, 0.0), 0.18, TRUE, flags, 1.0, kRenderTransAdd, 10.0);
		break;
	}
	case ZOMBIE_SKILL_PILE:
	{
		R_AttachTentToEntity(entity, gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/zbt_slow.spr"),
			Vector(0.0, 0.0, 40.0), 3.0, TRUE, flags, 1.0, kRenderTransAdd, 1.0);
		break;
	}
	default:
		break;
	}
}

void R_AttachTentToEntity(int entity, int modelIndex, vec3_t offset, float life, int additive, int flags, float scale, int rendermode, float framerate)
{
	if (!modelIndex)
	{
		gEngfuncs.Con_Printf("No model %d!\n", modelIndex);
		return;
	}

	cl_entity_t* pEnity = gEngfuncs.GetEntityByIndex(entity);
	if (!pEnity)
	{
		gEngfuncs.Con_Printf("Couldn't get ClientEntity for %i\n", entity);
		return;
	}

	if (!pEnity->index)	//avoid sent to world origin
		return;

	TEMPENTITY* pTemp = gEngfuncs.pEfxAPI->R_DefaultSprite(pEnity->origin + offset, modelIndex, 1.0);
	if (!pTemp)
	{
		gEngfuncs.Con_Printf("No temp ent.\n");

		return;
	}

	pTemp->entity.curstate.framerate = framerate;
	pTemp->entity.baseline.renderamt = pTemp->entity.curstate.renderamt = 255;
	pTemp->tentOffset = offset;
	pTemp->flags = FTENT_PLYRATTACHMENT | FTENT_SPRANIMATE | flags;
	pTemp->clientIndex = entity;
	pTemp->entity.curstate.renderfx = kRenderFxNoDissipation;
	pTemp->entity.curstate.scale = scale;
	pTemp->entity.curstate.rendermode = rendermode;
	if (additive)
		pTemp->entity.curstate.rendermode = kRenderTransAdd;
	pTemp->die = gHUD.m_flTime + life;

}

}