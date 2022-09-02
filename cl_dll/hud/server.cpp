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

	extern float g_flBloodhunterAnimTime;
	extern int g_iBloodhunterSecAnim;
	extern int g_iBloodhunterState;

	extern float g_flMGSMAnimTime;
	extern int g_iMGSMSecAnim;
	extern int g_iMGSMState;

	extern float g_flM1887xmasAnimTime;
	extern int g_iM1887xmasAnim;

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
	ZOMBIE_SKILL_HPBUFF_HEAD,
	ZOMBIE_SKILL_ARMORRECOVERY_HEAD,
	HUMAN_DEBUFF_SHOOTINGDOWN_HEAD,
	HOLYFIST_GLITCH,
	HOLYFIST_GLITCH_RING,
	WINGGUN_WING,
	DIVINETITAN_PARTICLE,
};

TEMPENTITY *iHolyFistRingEffect[33];
TEMPENTITY *iWingGunEffect[33];
TEMPENTITY* iDivinetitanParticle[33];

void CreateAttachedEntitiesToPlayer(int entity, int type);
void R_AttachTentToEntity(int entity, int modelIndex, vec3_t offset, float life, int additive, int flags, float scale, int rendermode, float framerate);
void CreateFollowEnt(TEMPENTITY* prev, vec3_t origin, int iAimingEntity, int iType);
void CreateBalrog11CannonSingleProjectile(TEMPENTITY* prev, vec3_t origin, int iAimingEntity, int iType);
TEMPENTITY* AttachTentToEntity(int entity, int modelIndex, vec3_t offset, float life, int additive, int flags, float scale, int rendermode, float framerate);
//void EnableWallHack(int enable, int iTarget);

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
		int iBanType = reader.ReadByte();
		int iSlot = iBanType / 10; // 1-wpn,2-knife,3-grenad
		int iDataType = iBanType % 10; // 0-all, 1-add, 2-remove
		int length = reader.ReadByte();
		if (iSlot == 1) {
			if (iDataType == 0) {
				gHUD.m_setBanWeapon.clear();
			}
			for (int i = 0; i < length; i++)
			{
				int id = reader.ReadShort();
				if(iDataType <= 1)
					gHUD.m_setBanWeapon.insert(id);
				else if(iDataType == 2)
					gHUD.m_setBanWeapon.erase(id);
			}
		}else if (iSlot == 2) {
			if (iDataType == 0) {
				gHUD.m_setBanKnife.clear();
			}
			for (int i = 0; i < length; i++)
			{
				int id = reader.ReadByte();
				if (iDataType <= 1)
					gHUD.m_setBanKnife.insert(id);
				else if (iDataType == 2)
					gHUD.m_setBanKnife.erase(id);
			}
		}else if (iSlot == 3) {
			if (iDataType == 0) {
				gHUD.m_setBanGrenade.clear();
			}
			for (int i = 0; i < length; i++)
			{
				int id = reader.ReadByte();
				if (iDataType <= 1)
					gHUD.m_setBanGrenade.insert(id);
				else if (iDataType == 2)
					gHUD.m_setBanGrenade.erase(id);
			}
		}
		break;
	}
	case 1:
	{
		CBasePlayerWeapon* pActiveBTEWeapon = BTEClientWeapons().GetActiveWeaponEntity();
		if (pActiveBTEWeapon)
		{
			switch (pActiveBTEWeapon->m_iId)
			{
			case WEAPON_M95TIGER:
			case WEAPON_M3DRAGON:
			case WEAPON_M3DRAGONM:
			case WEAPON_KRONOS12:
				pActiveBTEWeapon->pev->iuser1 = reader.ReadByte();
			default:
				break;
			}
		}
			
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
		case 2:
			gEngfuncs.pfnPlaySoundByNameAtPitch("zombi/siren_scream.wav", 1.0, 100);
			break;
		case 44:
			gEngfuncs.pfnPlaySoundByNameAtLocation("weapons/sgmissile_reload.wav", VOL_NORM, gHUD.m_vecOrigin);
			break;
		case 80:
			gEngfuncs.pfnPlaySoundByNameAtLocation("weapons/divinetitan_charge.wav", VOL_NORM, gHUD.m_vecOrigin);
			break;
		case 81:
			gEngfuncs.pfnPlaySoundByNameAtLocation("weapons/bunkerbuster_gauge.wav", VOL_NORM, gHUD.m_vecOrigin);
			break;
		}

		break;
	case 16:		
		arg1 = reader.ReadShort();
		arg2 = reader.ReadByte();

		switch (arg2)
		{
		case DIVINETITAN_PARTICLE:
		{
			if (iDivinetitanParticle[arg1])
			{
				iDivinetitanParticle[arg1]->die = gHUD.m_flTime;
				iDivinetitanParticle[arg1] = nullptr;
				break;
			}

			if (!iDivinetitanParticle[arg1])
				iDivinetitanParticle[arg1] = AttachTentToEntity(arg1, gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/ef_winggun_particle.spr"),
					Vector(0.0, 0.0, 5.0), 5.0, TRUE, FTENT_PERSIST | FTENT_SPRANIMATELOOP, 0.8, kRenderTransAdd, 30.0);
			break;
		}
		case WINGGUN_WING:
		{
			if (iWingGunEffect[arg1])
			{
				iWingGunEffect[arg1]->die = gHUD.m_flTime;
				iWingGunEffect[arg1] = nullptr;
				break;
			}

			if (!iWingGunEffect[arg1])
				iWingGunEffect[arg1] = AttachTentToEntity(arg1, gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/ef_winggun_idle.spr"),
					Vector(0.0, 0.0, 50.0), 15.0, TRUE, FTENT_PERSIST | FTENT_SPRANIMATELOOP, 0.5, kRenderTransAdd, 30.0);
			break;
		}
		case HOLYFIST_GLITCH_RING:
		{
			if (iHolyFistRingEffect[arg1])
			{
				iHolyFistRingEffect[arg1]->die = gHUD.m_flTime;
				iHolyFistRingEffect[arg1] = nullptr;
				break;
			}

			if (!iHolyFistRingEffect[arg1])
				iHolyFistRingEffect[arg1] = AttachTentToEntity(arg1, gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/terminator_ring.spr"),
					Vector(0.0, 0.0, 0.0), 5.0, TRUE, FTENT_PERSIST | FTENT_SPRANIMATELOOP, 0.7, kRenderTransAdd, 1.0);
			break;
		}
		default:
			break;
		}	
		break;
	case 17:
	{
		arg1 = reader.ReadByte();
		arg2 = reader.ReadByte();


		cl_entity_t* pPlayer = gEngfuncs.GetEntityByIndex(arg1);


		break;
	}
	case 18:
	{
		arg1 = reader.ReadByte();
		arg2 = reader.ReadShort();
		
		switch (arg1)
		{
		case 0:
		{
			TEMPENTITY* pEnt = gEngfuncs.pEfxAPI->R_TempCustomModel({ 0, 0, 10 }, { 0, 0, 0 }, { 0, 0, 0 }, 999, gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/ef_wondercannon_bomb_set.spr"), 0, 15, false, 255, kRenderTransAdd, arg2, 0, false, 0, 0.2, 21, FTENT_PERSIST | FTENT_SPRANIMATELOOP | FTENT_PLYRATTACHMENT);
			break;
		}
		case 1:
		{
			TEMPENTITY* pEnt = gEngfuncs.pEfxAPI->R_TempCustomModel({ 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, 999, gEngfuncs.pEventAPI->EV_FindModelIndex("models/ef_wondercannon_area.mdl"), 0, 1.0, false, 255, kRenderTransAdd, arg2, 0, false, 0, 1.0, 200, FTENT_PERSIST | FTENT_PLYRATTACHMENT);
			break;
		}
		default:
			break;
		}
		
		break;
	}
	case 19:
	{
		arg1 = reader.ReadShort();
		arg2 = reader.ReadShort();

		cl_entity_t* pEnt = gEngfuncs.GetEntityByIndex(arg1);
		cl_entity_t* pLinkEnt = gEngfuncs.GetEntityByIndex(arg2);

		vec3_t vecBeamStart, vecBeamEnd;
		vecBeamStart = pEnt->origin;
		vecBeamEnd = pLinkEnt->origin;

		int iBeamIndex = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/ef_wondercannon_chain.spr");

		BEAM* pBeam = gEngfuncs.pEfxAPI->R_BeamPoints_Stretch(vecBeamStart, vecBeamEnd, iBeamIndex, 1.0, 30.0, 255, 0, 30.0, 255, 255, 255);

		if (pBeam)
		{
			pBeam->startEntity = arg1;
			pBeam->endEntity = arg2;
			pBeam->flags = FBEAM_STARTENTITY | FBEAM_ENDENTITY;
		}

		break;
	}
	case 20:
	{
		arg1 = reader.ReadShort();

		m_SniperScope.SetKronosTime(arg1);
		break;
	}
	case 21:
	{
		arg1 = reader.ReadByte();
		arg2 = reader.ReadByte();
		int arg3 = reader.ReadByte();
		m_SniperScope.SetLockOnData(arg1, arg2, arg3);
		break;
	}
	case 22:
	{
		m_SniperScope.ClearAllLockOnData();
		break;
	}
	case 23:
	{
		int length = reader.ReadByte();

		for (int i = 0; i < length; i++)
		{
			arg1 = reader.ReadShort();
			arg2 = reader.ReadByte();
			m_SniperScope.InsertPatrolDroneData(i, arg1, arg2);
		}
		
		m_SniperScope.SetPatrolDroneDeployTime();
		break;
	}
	case 24:
	{
		int Slot = reader.ReadByte();
		arg1 = reader.ReadShort();
		arg2 = reader.ReadByte();
		m_SniperScope.InsertPatrolDroneData(Slot, arg1, arg2);
		break;
	}
	case 25:
	{
		//bloodhunter
		arg1 = reader.ReadByte();
		arg2 = reader.ReadByte();
		CBasePlayerWeapon* pActiveBTEWeapon = BTEClientWeapons().GetActiveWeaponEntity();
		if (pActiveBTEWeapon)
		{
			if (pActiveBTEWeapon->m_iId == WEAPON_BLOODHUNTER)
			{
				if (arg1 < 3)	//iAnim
				{
					if (!g_flBloodhunterAnimTime)
						g_flBloodhunterAnimTime = gHUD.m_flTime;
					g_iBloodhunterSecAnim = 19 + arg2;
				}
				else
				{
					g_flBloodhunterAnimTime = 0.0;
					g_iBloodhunterSecAnim = 0;
				}
			}
			else
			{
				g_flBloodhunterAnimTime = 0.0;
				g_iBloodhunterSecAnim = 0;
			}
		}
		break;
	}
	case 26:
	{
		//bloodhunter
		arg1 = reader.ReadByte();
		g_iBloodhunterState = arg1;
		break;
	}
	case 27:
	{
		//mgsm
		float arg3 = reader.ReadCoord();
		float arg4 = reader.ReadShort();
		m_SniperScope.SetMGSMAmmo(arg3, arg4);
		break;
	}
	case 28:
	{
		arg1 = reader.ReadByte();
		arg2 = reader.ReadByte();
		CBasePlayerWeapon* pActiveBTEWeapon = BTEClientWeapons().GetActiveWeaponEntity();
		if (pActiveBTEWeapon)
		{
			if (pActiveBTEWeapon->m_iId == WEAPON_MGSM)
			{
				if (arg1)	//iWpnState
				{
					//if (!g_flMGSMAnimTime)
					g_flMGSMAnimTime = gHUD.m_flTime;
					g_iMGSMSecAnim = arg2;
				}
				else
				{
					g_flMGSMAnimTime = 0.0;
					g_iMGSMSecAnim = 0;
				}
			}
			else
			{
				g_flMGSMAnimTime = 0.0;
				g_iMGSMSecAnim = 0;
			}
		}
		break;
	}
	case 29:
	{
		//bunkerbuster
		arg2 = reader.ReadByte();
		float arg3 = reader.ReadCoord();
		float arg4 = reader.ReadCoord();

		m_SniperScope.InsertBunkerBusterData(arg2,arg3, arg4);
		break;
	}
	case 30:
	{
		float arg3 = reader.ReadCoord();

		m_SniperScope.InsertBunkerBusterData2(arg3);
		break;


	}
	case 31:
	{
		g_iM1887xmasAnim = -1;
		g_flM1887xmasAnimTime = 0.0;
		break;
	}
	case 42:
	{
		arg1 = reader.ReadShort();
		pos.x = reader.ReadCoord();
		pos.y = reader.ReadCoord();
		pos.z = reader.ReadCoord();

		R_AttachTentToEntity(arg1, gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/flame_burn01.spr"),
			pos, reader.ReadShort() * 0.1, TRUE, FTENT_FADEOUT | FTENT_SPRANIMATE | FTENT_PERSIST | FTENT_PLYRATTACHMENT | FTENT_SPRANIMATELOOP, 0.3, kRenderTransAdd, 1.0);

		break;
	}
	case 79:	// stop sound
		arg1 = reader.ReadShort();
		arg2 = reader.ReadShort();

		gEngfuncs.pEventAPI->EV_StopSound(arg1, arg2, reader.ReadString());

		break;
	}
	return 1;
}

//useless
//void EnableWallHack(int enable, int iTarget)
//{
//	int idx = gEngfuncs.GetLocalPlayer()->index;
//	cl_entity_t* pPlayer = gEngfuncs.GetEntityByIndex(idx);
//
//	if (!enable)
//	{
//		for (int i = 1; i < 33; i++)
//		{
//			if (i == idx)
//				continue;
//
//			cl_entity_t* pEnt = gEngfuncs.GetEntityByIndex(i);
//			if (!pEnt)
//				continue;
//
//			pEnt->curstate.renderfx = kRenderFxNone;
//		}
//	}
//	else
//	{
//		cl_entity_t* pEnt = gEngfuncs.GetEntityByIndex(iTarget);
//		if (!pEnt)
//			return;
//
//		pEnt->curstate.renderfx = kRenderFxWallHack;
//	}
//}

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
	case ZOMBIE_SKILL_HPBUFF_HEAD:
	{
		R_AttachTentToEntity(entity, gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/zb_skill_hpbuff.spr"),
			Vector(0.0, 0.0, 25.0), 1.0, TRUE, flags, 1.0, kRenderTransAdd, 10.0);
		break;
	}
	case ZOMBIE_SKILL_ARMORRECOVERY_HEAD:
	{
		R_AttachTentToEntity(entity, gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/armorrecovery_head.spr"),
			Vector(0.0, 0.0, 25.0), 1.0, TRUE, flags, 1.0, kRenderTransAdd, 10.0);
		break;
	}
	case HUMAN_DEBUFF_SHOOTINGDOWN_HEAD:
	{
		R_AttachTentToEntity(entity, gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/zbt_shootingdown.spr"),
			Vector(0.0, 0.0, 25.0), 5.0, TRUE, flags, 1.0, kRenderTransAdd, 10.0);
		break;
	}
	case HOLYFIST_GLITCH:
	{
		flags = FTENT_PERSIST | FTENT_FADEOUT;
		R_AttachTentToEntity(entity, gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/ef_lance_hit.spr"),
			Vector(0.0, 0.0, 0.0), 0.18, TRUE, flags, 1.0, kRenderTransAdd, 10.0);
		break;
	}
	case HOLYFIST_GLITCH_RING:
	{
		flags = FTENT_PERSIST | FTENT_FADEOUT;
		R_AttachTentToEntity(entity, gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/ef_lance_hit.spr"),
			Vector(0.0, 0.0, 0.0), 0.18, TRUE, flags, 1.0, kRenderTransAdd, 10.0);
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


TEMPENTITY *AttachTentToEntity(int entity, int modelIndex, vec3_t offset, float life, int additive, int flags, float scale, int rendermode, float framerate)
{
	if (!modelIndex)
	{
		gEngfuncs.Con_Printf("No model %d!\n", modelIndex);
		return nullptr;
	}

	cl_entity_t* pEnity = gEngfuncs.GetEntityByIndex(entity);
	if (!pEnity)
	{
		gEngfuncs.Con_Printf("Couldn't get ClientEntity for %i\n", entity);
		return nullptr;
	}

	if (!pEnity->index)	//avoid sent to world origin
		return nullptr;

	TEMPENTITY* pTemp = gEngfuncs.pEfxAPI->R_DefaultSprite(pEnity->origin + offset, modelIndex, 1.0);
	if (!pTemp)
	{
		gEngfuncs.Con_Printf("No temp ent.\n");

		return nullptr;
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

	return pTemp;
}

}