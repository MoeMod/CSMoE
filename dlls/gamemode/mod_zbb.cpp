/*
mob_zbb.cpp - CSMoE Gameplay server : Zombie Mod Base-builder
Copyright (C) 2018 Moemod Hyakuya

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "game.h"

#include "mod_zbb.h"

CMod_ZombieBaseBuilder::CMod_ZombieBaseBuilder() // precache
{

}

void CMod_ZombieBaseBuilder::UpdateGameMode(CBasePlayer *pPlayer)
{
	MESSAGE_BEGIN(MSG_ONE, gmsgGameMode, nullptr, pPlayer->edict());
	WRITE_BYTE(MOD_ZB1);
	WRITE_BYTE(0); // Reserved. (weapon restriction? )
	WRITE_BYTE(static_cast<int>(maxrounds.value)); // MaxRound (mp_roundlimit)
	WRITE_BYTE(0); // Reserved. (MaxTime?)

	MESSAGE_END();
}

class CPlayerModStrategy_ZBB : public CPlayerModStrategy_ZB1
{
public:
	using CPlayerModStrategy_ZB1::CPlayerModStrategy_ZB1;
	
private:
	bool m_bCanBuild = true;
	bool m_bIsBuilding = false;
	float m_flBuildDistance;
	CBaseEntity* m_pPointingTarget;
	Vector m_vecOffset;
	float m_flBuildDelay;

	static constexpr auto BUILD_DELAY = 0.75;
	static constexpr auto BUILD_MAXDIST = 720;
	static constexpr auto BUILD_MINDIST = 30;
	static constexpr auto BUILD_SETDIST = 64;

	void Build_Start()
	{
		if (!m_bCanBuild)
			return;

		const Vector vecStart = m_pPlayer->GetGunPosition();

		UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
		Vector vecDir = gpGlobals->v_forward;
		Vector vecEnd = vecStart + vecDir * BUILD_MAXDIST;

		TraceResult tr{};
		UTIL_TraceLine(vecStart, vecEnd, dont_ignore_monsters, m_pPlayer->edict(), &tr);

		CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);

		if (!pEntity || pEntity->IsAlive() /* pHit->IsMoving()*/)
			return;

		m_vecOffset = pEntity->pev->origin - tr.vecEndPos;
		m_flBuildDistance = (tr.vecEndPos - vecStart).Length();

		if (m_flBuildDistance < BUILD_MINDIST)
			m_flBuildDistance = BUILD_SETDIST;

		//MovingEnt(pEntity);
		//SetEntMover(pEntity, id);
		m_bIsBuilding = true;
		m_pPointingTarget = pEntity;

		pEntity->pev->solid = SOLID_BBOX;
		pEntity->pev->movetype = MOVETYPE_FLY;
		pEntity->pev->rendermode = kRenderTransColor;
		pEntity->pev->rendercolor = { 90.0, 190.0, 90.0 };
		pEntity->pev->renderamt = 125;
	}

	void Build_Process()
	{

	}

	void Build_End()
	{
		if (!m_bIsBuilding)
			return;

		CBaseEntity * pEntity = m_pPointingTarget;

		const Vector vecAngles = pEntity->pev->angles;
		DispatchSpawn(pEntity->edict());
		pEntity->pev->angles = vecAngles;

		pEntity->pev->rendermode = kRenderNormal;
		pEntity->pev->rendercolor = { 255, 255, 255 };
		pEntity->pev->renderamt = 255;

		m_bIsBuilding = false;
		//UnsetEntMover(iEntity);
		m_pPointingTarget = nullptr;
		//UnmovingEnt(iEntity);
	}
};

void CMod_ZombieBaseBuilder::InstallPlayerModStrategy(CBasePlayer *player)
{
	std::unique_ptr<CPlayerModStrategy_ZBB> up(new CPlayerModStrategy_ZBB(player, this));
	player->m_pModStrategy = std::move(up);
}

#if 0
// Building (Thanks to BaseBuilder by Tirant)
#define BUILD_DELAY 0.75
#define BUILD_PUSHPULLRATE 4.0
#define BUILD_ROTATERATE 2.5
#define BUILD_MAXDIST 720.0
#define BUILD_MINDIST 30.0
#define BUILD_SETDIST 64.0

#define MovingEnt(%1)		 ( set_pev(%1, pev_iuser4, 1) )
#define UnmovingEnt(%1)	   ( set_pev(%1, pev_iuser4, 0) )
#define IsMovingEnt(%1)	   ( pev(%1, pev_iuser4) == 1)

#define SetEntMover(%1,%2)	  ( set_pev( %1, pev_iuser3, %2) )
#define UnsetEntMover(%1)	   ( set_pev( %1, pev_iuser3, 0) )
#define GetEntMover(%1)	   ( pev( %1, pev_iuser3) )

new const cfg_iColorEntSelect[3] = { 90, 90, 90 }
new const cfg_iColorEntMoving[3] = { 90, 190, 90 }
new const cfg_iColorEntChecking[3] = { 190, 190, 90 }
new const cfg_iColorEntStucked[3] = { 190, 90, 90 }
new const Float : cfg_flColorEntMoving[3] = { 90.0, 190.0, 90.0 }

new g_bitsCanBuild
new g_bitsBuilding, g_pPointingTarget[33]
new Float:g_vecOffset[33][3]
new Float : g_flBuildDistance[33]
new Float : g_fBuildDelay[33]

public plugin_init()
{
	register_plugin(PLUGIN, VERSION, AUTHOR)

		register_forward(FM_CmdStart, "fw_CmdStart")
		register_forward(FM_PlayerPostThink, "fw_PlayerPostThink")
		register_forward(FM_TraceLine, "fw_TraceLine", 1)
		register_forward(FM_AddToFullPack, "fw_AddToFullPack_Post", 1)

		register_event("HLTV", "Event_NewRound", "a", "1=0", "2=0")
		//register_event("CurWeapon", "Event_CurWeapon", "be", "1=1")

		RegisterHam(Ham_Touch, "func_wall", "fw_BuildWallTouch")

		// Build Entities
		//ResetEntity()

		g_bitsCanBuild = 0;
}

public Native_SetPlayerCanBuild(id, bSet)
{
	if (bSet)
		BitsSet(g_bitsCanBuild, id)
	else
		Reset_Player(id)
}

public Native_GetPlayerCanBuild(id)
{
	return !!BitsGet(g_bitsCanBuild, id)
}

public Native_SetEntityBuilding(pEntity, bSet)
{
	set_pev(pEntity, pev_iuser2, bSet ? 998 : 0);
	set_pev(pEntity, pev_iuser4, 0);
	set_pev(pEntity, pev_rendermode, kRenderNormal);
	engfunc(EngFunc_SetOrigin, pEntity, Float:{ 0.0, 0.0, 0.0 });
	for (new i = 0; i < 33; i++)
	{
		if (g_pPointingTarget[i] == pEntity)
			Build_End(i);
	}
}

public Native_GetEntityBuilding(pEntity)
{
	return pev(pEntity, pev_iuser2);
}

public Event_NewRound()
{
	//ResetEntity()
}

public Reset_Player(id)
{
	BitsUnSet(g_bitsCanBuild, id);

	Build_End(id)
}

public client_disconnect(id)
{
	Reset_Player(id)
}

public fw_CmdStart(id, uc_handle, seed)
{
	static bitsCurButton; bitsCurButton = get_uc(uc_handle, UC_Buttons)
		static bitsOldButton; bitsOldButton = pev(id, pev_oldbuttons)

		if (BitsGet(g_bitsBuilding, id))
		{
			if (bitsCurButton & IN_ATTACK)
			{
				g_flBuildDistance[id] += BUILD_PUSHPULLRATE;
				if (g_flBuildDistance[id] > BUILD_MAXDIST)
				{
					g_flBuildDistance[id] = BUILD_MAXDIST
				}
				bitsCurButton &= ~IN_ATTACK;
			}
			else if (bitsCurButton & IN_ATTACK2)
			{
				g_flBuildDistance[id] -= BUILD_PUSHPULLRATE;
				if (g_flBuildDistance[id] < BUILD_SETDIST)
				{
					g_flBuildDistance[id] = BUILD_SETDIST
				}
				bitsCurButton &= ~IN_ATTACK2;
			}
			set_uc(uc_handle, UC_Buttons, bitsCurButton)
		}

	if (bitsCurButton & ~bitsOldButton & IN_USE)
	{
		Build_Start(id)
	}
	else if (BitsGet(g_bitsBuilding, id) && !(bitsCurButton & IN_USE))
	{
		Build_End(id)
	}
}

public fw_TraceLine(Float:v1[3], Float : v2[3], noMonsters, pentToSkip)
{
	new id = pentToSkip;
	if (!is_user_alive(id))
		return;

	if (!BitsGet(g_bitsCanBuild, id))
		return;
	if (BitsGet(g_bitsBuilding, id))
		return;

	new iEntity = get_tr(TR_pHit)

		// 旧的目标失效
		if (!pev_valid(g_pPointingTarget[id]))
		{
			g_pPointingTarget[id] = 0;
		}

	// 更换了目标
	if (iEntity != g_pPointingTarget[id])
	{
		if (FCanEntityBuilt(iEntity))
		{
			g_pPointingTarget[id] = iEntity;
		}

	}

	return;
}

FCanEntityBuilt(iEntity)
{
	if (!pev_valid(iEntity) || is_user_alive(iEntity) || IsMovingEnt(iEntity))
		return 0;

	if (!pev(iEntity, pev_iuser2))
		return 0;

	return 1;
}

public fw_PlayerPostThink(id)
{
	Build_Check_Moving(id)
}

public fw_AddToFullPack_Post(es_handle, e, iEntity, host, hostflags, player, pset)
{
	if (!is_user_connected(host))
		return

		if (iEntity == g_pPointingTarget[host])
		{
			if (BitsGet(g_bitsBuilding, host))
			{
				if (get_es(es_handle, ES_Solid) == SOLID_NOT)
				{
					set_es(es_handle, ES_RenderMode, kRenderTransColor)
						set_es(es_handle, ES_RenderAmt, 125)
						set_es(es_handle, ES_RenderFx, kRenderFxGlowShell)
						set_es(es_handle, ES_RenderColor, cfg_iColorEntMoving)
				}
				else if (get_es(es_handle, ES_Solid) == SOLID_BBOX)
				{
					set_es(es_handle, ES_RenderMode, kRenderTransColor)
						set_es(es_handle, ES_RenderAmt, 125)
						set_es(es_handle, ES_RenderFx, kRenderFxGlowShell)
						set_es(es_handle, ES_RenderColor, cfg_iColorEntChecking)
				}
				else if (get_es(es_handle, ES_Solid) == SOLID_SLIDEBOX)
				{
					set_es(es_handle, ES_RenderMode, kRenderTransColor)
						set_es(es_handle, ES_RenderAmt, 125)
						set_es(es_handle, ES_RenderFx, kRenderFxGlowShell)
						set_es(es_handle, ES_RenderColor, cfg_iColorEntStucked)
				}
			}
			else
			{
				set_es(es_handle, ES_RenderMode, kRenderTransColor)
					set_es(es_handle, ES_RenderAmt, 125)
					set_es(es_handle, ES_RenderFx, kRenderFxGlowShell)
					set_es(es_handle, ES_RenderColor, cfg_iColorEntSelect)
			}
		}
}

Build_Check_Moving(id)
{
	if (!BitsGet(g_bitsCanBuild, id) || !is_user_alive(id))
	{
		Build_End(id)
			return PLUGIN_HANDLED
	}

	if (!BitsGet(g_bitsBuilding, id))
		return PLUGIN_HANDLED

		static Float:vecOrigin[3]; pev(g_pPointingTarget[id], pev_origin, vecOrigin)
		static Float:vecAngles[3]; pev(g_pPointingTarget[id], pev_angles, vecAngles)

		new iOrigin[3], iLook[3], Float:fOrigin[3], Float : fLook[3], Float : fLength

		get_user_origin(id, iOrigin, 1);
	IVecFVec(iOrigin, fOrigin);
	get_user_origin(id, iLook, 3);
	IVecFVec(iLook, fLook);

	fLength = get_distance_f(fLook, fOrigin);
	if (fLength == 0.0) fLength = 1.0;

	vecOrigin[0] = (fOrigin[0] + (fLook[0] - fOrigin[0]) * g_flBuildDistance[id] / fLength);
	vecOrigin[1] = (fOrigin[1] + (fLook[1] - fOrigin[1]) * g_flBuildDistance[id] / fLength);
	vecOrigin[2] = (fOrigin[2] + (fLook[2] - fOrigin[2]) * g_flBuildDistance[id] / fLength);
	vecOrigin[2] = float(floatround(vecOrigin[2], floatround_floor));
	xs_vec_add(vecOrigin, g_vecOffset[id], vecOrigin)

		if (pev(g_pPointingTarget[id], pev_solid) == SOLID_SLIDEBOX)
		{
			static Float:vecPos[3]; pev(g_pPointingTarget[id], pev_origin, vecPos)
				if (vector_distance(vecOrigin, vecPos) < 30.0)
				{
					return PLUGIN_CONTINUE
				}
			set_pev(g_pPointingTarget[id], pev_solid, SOLID_BBOX)
		}

	engfunc(EngFunc_SetOrigin, g_pPointingTarget[id], vecOrigin)
		set_pev(g_pPointingTarget[id], pev_angles, vecAngles)

		return PLUGIN_CONTINUE
}

Build_Start(id)
{
	if (!is_user_alive(id) || !BitsGet(g_bitsCanBuild, id))
		return PLUGIN_HANDLED;
	if (g_fBuildDelay[id] > get_gametime())
		return PLUGIN_HANDLED;
	else
		g_fBuildDelay[id] = get_gametime() + BUILD_DELAY

		static Float:vecStart[3], Float : vecViewOfs[3]
		pev(id, pev_origin, vecStart)
		pev(id, pev_view_ofs, vecViewOfs)
		xs_vec_add(vecStart, vecViewOfs, vecStart)

		static Float : vecViewAngle[3], Float : vecEnd[3]
		pev(id, pev_v_angle, vecViewAngle)
		engfunc(EngFunc_MakeVectors, vecViewAngle)
		global_get(glb_v_forward, vecEnd)
		xs_vec_mul_scalar(vecEnd, BUILD_MAXDIST, vecEnd)
		xs_vec_add(vecStart, vecEnd, vecEnd)

		new ptr = create_tr2()
		engfunc(EngFunc_TraceLine, vecStart, vecEnd, 0, id, ptr)

		static Float:vecEndPos[3], Float : flFraction
		get_tr2(ptr, TR_vecEndPos, vecEndPos)
		get_tr2(ptr, TR_flFraction, flFraction)
		if (flFraction >= 1.0)
			engfunc(EngFunc_TraceHull, vecStart, vecEnd, 0, HULL_HEAD, id, ptr)
			new iEntity = get_tr2(ptr, TR_pHit)
			free_tr2(ptr)


			if (!pev_valid(iEntity) || is_user_alive(iEntity) || IsMovingEnt(iEntity))
				return PLUGIN_HANDLED;
	if (!pev(iEntity, pev_iuser2))
		return PLUGIN_HANDLED;

	new Float:vecOrigin[3]
		pev(iEntity, pev_origin, vecOrigin)

		xs_vec_sub(vecOrigin, vecEndPos, g_vecOffset[id])

		g_flBuildDistance[id] = get_distance_f(vecStart, vecEndPos)

		if (g_flBuildDistance[id] < BUILD_MINDIST)
			g_flBuildDistance[id] = BUILD_SETDIST;

	MovingEnt(iEntity);
	SetEntMover(iEntity, id);
	BitsSet(g_bitsBuilding, id)
		g_pPointingTarget[id] = iEntity

		set_pev(iEntity, pev_solid, SOLID_BBOX)
		set_pev(iEntity, pev_movetype, MOVETYPE_FLY)

		set_pev(iEntity, pev_rendermode, kRenderTransColor)
		set_pev(iEntity, pev_renderfx, kRenderFxGlowShell)
		set_pev(iEntity, pev_rendercolor, cfg_flColorEntMoving)
		set_pev(iEntity, pev_renderamt, 125.0)
		return PLUGIN_CONTINUE;
}

Build_End(id)
{
	if (!BitsGet(g_bitsBuilding, id))
		return PLUGIN_HANDLED

		new iEntity = g_pPointingTarget[id]

		new Float:vecAngles[3]
		pev(iEntity, pev_angles, vecAngles)
		dllfunc(DLLFunc_Spawn, iEntity)
		set_pev(iEntity, pev_angles, vecAngles)

		fm_set_rendering(iEntity)

		BitsUnSet(g_bitsBuilding, id);
	UnsetEntMover(iEntity);
	g_pPointingTarget[id] = 0;
	UnmovingEnt(iEntity);

	return PLUGIN_CONTINUE;
}

public fw_BuildWallTouch(iEntity, pHit)
{
	if (pev(iEntity, pev_solid) != SOLID_BBOX || !IsMovingEnt(iEntity))
		return
		set_pev(iEntity, pev_solid, SOLID_SLIDEBOX)
		set_pev(iEntity, pev_fuser1, get_gametime() + 0.3)
}
#endif