/*
KnifeAttack.h
Copyright (C) 2019 Moemod Hymei

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#pragma once

#ifndef CLIENT_DLL
#include "monsters.h"
#include <dlls/gamemode/mod_zb1.h>

namespace sv {

void FindHullIntersection(const Vector &vecSrc, TraceResult &tr, const vec3_t pflMins, const vec3_t pfkMaxs, edict_t *pEntity);

enum hit_result_t
{
	HIT_NONE, HIT_WALL, HIT_PLAYER,
};
inline hit_result_t
KnifeAttack(Vector vecSrc, Vector vecDir, float flDamage, float flRadius, int bitsDamageType,
             entvars_t *pevInflictor, entvars_t *pevAttacker, bool IsPrimaryAttack = false)
{
	TraceResult tr;
	hit_result_t result = HIT_NONE;

	vecSrc.z += 1;

	if (!pevAttacker)
		pevAttacker = pevInflictor;

	Vector vecEnd = vecSrc + vecDir.Normalize() * flRadius;
	UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(pevAttacker), &tr);

	if (tr.flFraction >= 1) {
		UTIL_TraceHull(vecSrc, vecEnd, dont_ignore_monsters, head_hull, ENT(pevAttacker), &tr);

		if (tr.flFraction < 1) {
			CBaseEntity *pHit = CBaseEntity::Instance(tr.pHit);

			if (!pHit || pHit->IsBSPModel()) {
				FindHullIntersection(vecSrc, tr, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX, ENT(pevAttacker));
			}

			vecEnd = tr.vecEndPos;
		}
	}

	if (tr.flFraction < 1) {
		CBaseEntity *pHit = CBaseEntity::Instance(tr.pHit);
		if (pHit && pHit->pev->takedamage != DAMAGE_NO) {

			if (!IsPrimaryAttack)
			{
				UTIL_MakeVectors(pHit->pev->angles);
				if (DotProduct((pHit->pev->origin - pevAttacker->origin).Normalize().Make2D(), gpGlobals->v_forward.Make2D()) > 0.8)
				{
					flDamage *= 3.0;
					bitsDamageType |= DMG_BACKATK;
				}
			}
			
			ClearMultiDamage();
			pHit->TraceAttack(pevInflictor, flDamage, (tr.vecEndPos - vecSrc).Normalize(), &tr, bitsDamageType);
			ApplyMultiDamage(pevInflictor, pevAttacker);
			result = HIT_PLAYER;
		}

		float flVol = 1;
		BOOL fHitWorld = TRUE;
		if (pHit && pHit->Classify() != CLASS_NONE && pHit->Classify() != CLASS_MACHINE) {
			flVol = 0.1f;
			fHitWorld = FALSE;
		}

		if (fHitWorld) {
			TEXTURETYPE_PlaySound(&tr, vecSrc, vecSrc + (vecEnd - vecSrc) * 2, BULLET_PLAYER_CROWBAR);
			result = HIT_WALL;
		}
	}

	return result;
}
inline hit_result_t
KnifeAttack3(Vector vecSrc, Vector vecDir, float flDamage, float flRadius, float flAngleDegrees, int bitsDamageType,
             entvars_t *pevInflictor, entvars_t *pevAttacker)
{
	TraceResult tr;
	hit_result_t result = HIT_NONE;

	vecSrc.z += 1;

	if (!pevAttacker)
		pevAttacker = pevInflictor;

	Vector vecEnd = vecSrc + vecDir.Normalize() * flRadius;
	UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(pevAttacker), &tr);

	if (tr.flFraction >= 1) {
		UTIL_TraceHull(vecSrc, vecEnd, dont_ignore_monsters, head_hull, ENT(pevAttacker), &tr);

		if (tr.flFraction < 1) {
			CBaseEntity *pHit = CBaseEntity::Instance(tr.pHit);

			if (!pHit || pHit->IsBSPModel()) {
				FindHullIntersection(vecSrc, tr, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX, ENT(pevAttacker));
			}

			vecEnd = tr.vecEndPos;
		}
	}

	if (tr.flFraction < 1) {
		CBaseEntity *pHit = CBaseEntity::Instance(tr.pHit);
		if (pHit && pHit->IsBSPModel() && pHit->pev->takedamage != DAMAGE_NO) {
			const float flAdjustedDamage = flDamage;
			ClearMultiDamage();
			pHit->TraceAttack(pevInflictor, flAdjustedDamage, (tr.vecEndPos - vecSrc).Normalize(), &tr, bitsDamageType);
			ApplyMultiDamage(pevInflictor, pevAttacker);
		}

		float flVol = 1;
		BOOL fHitWorld = TRUE;
		if (pHit && pHit->Classify() != CLASS_NONE && pHit->Classify() != CLASS_MACHINE) {
			flVol = 0.1f;
			fHitWorld = FALSE;
		}

		if (fHitWorld) {
			TEXTURETYPE_PlaySound(&tr, vecSrc, vecSrc + (vecEnd - vecSrc) * 2, BULLET_PLAYER_CROWBAR);
			result = HIT_WALL;
		}
	}

	CBaseEntity *pEntity = nullptr;
	while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, flRadius)) != nullptr) {
		if (pEntity->pev->takedamage != DAMAGE_NO) {
			if (pEntity->IsBSPModel())
				continue;

			if (pEntity->pev == pevAttacker)
				continue;

			Vector vecSpot = pEntity->BodyTarget(vecSrc);
			vecSpot.z = vecEnd.z;
			UTIL_TraceLine(vecSrc, vecSpot, missile, ENT(pevInflictor), &tr);

			if (AngleBetweenVectors(tr.vecEndPos - vecSrc, vecDir) > flAngleDegrees)
				continue;

			if (tr.flFraction == 1.0f || tr.pHit == pEntity->edict()) {
				if (tr.fStartSolid) {
					tr.vecEndPos = vecSrc;
					tr.flFraction = 0;
				}

				if (tr.flFraction == 1.0f) {
					pEntity->TakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType);
				}

				Vector vecRealDir = (pEntity->pev->origin - pevAttacker->origin).Normalize();

				UTIL_MakeVectors(pEntity->pev->angles);
				if (DotProduct(vecRealDir.Make2D(), gpGlobals->v_forward.Make2D()) > 0.8)
				{
					flDamage *= 3.0;
					bitsDamageType |= DMG_BACKATK;
				}

				ClearMultiDamage();
				pEntity->TraceAttack(pevInflictor, flDamage, vecRealDir, &tr, bitsDamageType);
				ApplyMultiDamage(pevInflictor, pevAttacker);

				result = HIT_PLAYER;
			}
		}
	}

	return result;
}

inline Vector KnifeAttack2(Vector vecSrc, Vector vecDir, float flDamage, float flRadius, float flAngleDegrees, int bitsDamageType, entvars_t* pevInflictor, entvars_t* pevAttacker)
{
	TraceResult tr;

	const float falloff = flRadius ? flDamage / flRadius : 1;
	const int bInWater = (UTIL_PointContents(vecSrc) == CONTENTS_WATER);

	vecSrc.z += 1;

	if (!pevAttacker)
		pevAttacker = pevInflictor;

	Vector vecEnd = vecSrc + vecDir.Normalize() * flAngleDegrees;
	UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(pevAttacker), &tr);

	if (tr.flFraction >= 1)
	{
		UTIL_TraceHull(vecSrc, vecEnd, dont_ignore_monsters, head_hull, ENT(pevAttacker), &tr);

		if (tr.flFraction < 1)
		{
			CBaseEntity* pHit = CBaseEntity::Instance(tr.pHit);

			if (pHit && pHit->IsBSPModel() && pHit->pev->takedamage != DAMAGE_NO)
			{
				float flAdjustedDamage = flDamage - (tr.vecEndPos - vecSrc).Length() * falloff;
				ClearMultiDamage();
				pHit->TraceAttack(pevInflictor, flAdjustedDamage, (tr.vecEndPos - vecSrc).Normalize(), &tr, bitsDamageType);
				ApplyMultiDamage(pevInflictor, pevAttacker);
			}
			vecEnd = tr.vecEndPos;
		}
	}

	CBaseEntity* pEntity = NULL;
	while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, flRadius)) != NULL)
	{
		if (pEntity->pev->takedamage != DAMAGE_NO)
		{
			if (bInWater && !pEntity->pev->waterlevel)
				continue;

			if (!bInWater && pEntity->pev->waterlevel == 3)
				continue;

			if (pEntity->IsBSPModel())
				continue;

			if (pEntity->pev == pevAttacker)
				continue;

			Vector vecSpot = pEntity->BodyTarget(vecSrc);
			UTIL_TraceLine(vecSrc, vecSpot, missile, ENT(pevInflictor), &tr);

			if (AngleBetweenVectors(tr.vecEndPos - vecSrc, vecDir) > flAngleDegrees)
				continue;

			if (tr.flFraction == 1.0f || tr.pHit == pEntity->edict())
			{
				if (tr.fStartSolid)
				{
					tr.vecEndPos = vecSrc;
					tr.flFraction = 0;
				}

				float flAdjustedDamage = flDamage - (tr.vecEndPos - vecSrc).Length() * falloff;
				flAdjustedDamage = Q_max(0, flAdjustedDamage);

				if (tr.flFraction == 1.0f)
				{
					pEntity->TakeDamage(pevInflictor, pevAttacker, flAdjustedDamage, bitsDamageType);
				}
				else
				{
					tr.iHitgroup = HITGROUP_CHEST;
					ClearMultiDamage();
					pEntity->TraceAttack(pevAttacker, flAdjustedDamage, (tr.vecEndPos - vecSrc).Normalize(), &tr, bitsDamageType);
					ApplyMultiDamage(pevInflictor, pevAttacker);
					if (pEntity->IsPlayer())
					{
						CBasePlayer* pVictim = dynamic_cast<CBasePlayer*>(pEntity);
						if (pVictim->m_bIsZombie) // fire burn
						{
							MESSAGE_BEGIN(MSG_ALL, gmsgMPToCL, nullptr, pVictim->pev);
							WRITE_BYTE(4);
							WRITE_SHORT(pVictim->entindex());
							WRITE_BYTE(4);
							MESSAGE_END();
						}
					}

					
				}
			}
		}
	}
	return vecDir;
}

inline bool bIsWallBetweenEntity(CBaseEntity* pEntity, CBasePlayer* pAttackPlayer)
{
	TraceResult tr;
	Vector vecSrc = pAttackPlayer->pev->origin;
	Vector vecEnd = pEntity->pev->origin;
	UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, pAttackPlayer->edict(), &tr);
	
	if (tr.flFraction < 1.0)
	{
		if (!tr.pHit)
			return false;

		CBaseEntity* pHit = CBaseEntity::Instance(tr.pHit);

		if (pHit)
		{
			if(pHit == pEntity)
				return false;

			if (!Q_strcmp(STRING(pHit->pev->classname), "holysword_parray") || !Q_strcmp(STRING(pHit->pev->classname), "y22s1holyswordmb_parray"))
				return false;
		}
			

		return true;
	}
	return false;
}

inline
bool AngleCheck(CBaseEntity* pEntity, CBaseEntity* pAttacker, Vector vecSrc, Vector vecDir, Vector2D vecAngleCheckDir, float flRange, float flMinimumCosine, TraceResult* ptr)
{
#ifndef CLIENT_DLL
	if (pEntity->IsBSPModel())
	{
		UTIL_TraceLine(vecSrc, vecSrc + vecDir * flRange, ignore_monsters, pAttacker->edict(), ptr);

		if (ptr->flFraction < 1.0)
		{
			if (ptr->pHit)
			{
				CBaseEntity* pHit = CBaseEntity::Instance(ptr->pHit);

				if (pHit && pHit == pEntity)
					return true;
			}
		}

	}
	else
	{
		UTIL_TraceHull(vecSrc, vecSrc + vecDir * flRange, dont_ignore_monsters, head_hull, pAttacker->edict(), ptr);

		if (ptr->flFraction < 1.0 && ptr->pHit != NULL && CBaseEntity::Instance(ptr->pHit) == pEntity)
		{
			UTIL_TraceLine(vecSrc, vecSrc + vecDir * flRange, dont_ignore_monsters, pAttacker->edict(), ptr);

			return true;
		}
		else
		{
			Vector vecEnd = pEntity->BodyTarget(vecSrc);

			if (DotProduct(vecAngleCheckDir, (vecEnd.Make2D() - vecSrc.Make2D()).Normalize()) > flMinimumCosine)
			{
				edict_t* peAttacker = pAttacker->edict();

				for (int i = 5; i > 0; i--)
				{
					UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, peAttacker, ptr);

					if (!ptr->pHit || !GET_PRIVATE(ptr->pHit))
						return false;

					CBaseEntity* pHit = CBaseEntity::Instance(ptr->pHit);

					if (ptr->flFraction < 1.0 || pHit == pEntity)
						return true;

					if (pHit->IsBSPModel())
						return false;

					vecSrc = ptr->vecEndPos + vecDir * 5.0;
				}
			}
		}
	}
#endif
	return false;
}

inline hit_result_t
 KnifeAttack5(float flDamage, float flRadius, float flAngleDegrees, int bitsDamageType, entvars_t* pevInflictor, CBasePlayer* pAttackPlayer, bool IsPrimaryAttack = false)
{
	CBaseEntity* pEntity = NULL;
	hit_result_t result = HIT_NONE;
	TraceResult tr;
	UTIL_MakeVectors(pAttackPlayer->pev->v_angle);
	Vector vecEnd;
	Vector vecSrc = pAttackPlayer->GetGunPosition();

	Vector vecForward = gpGlobals->v_forward;
	Vector2D vecForward2D = vecForward.Make2D().Normalize();

	float c = cos(flAngleDegrees * 0.5 * M_PI / 180.0);

	while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, flRadius)) != NULL)
	{
		if (pEntity == pAttackPlayer)
			continue;

		if (pEntity->IsDormant())
			continue;

		if (pEntity->pev->takedamage == DAMAGE_NO)
			continue;

		if(bIsWallBetweenEntity(pEntity, pAttackPlayer))
			continue;

		if (!AngleCheck(pEntity, pAttackPlayer, vecSrc, vecForward, vecForward2D, flRadius, c, &tr))
			continue;

		float flRatio = 1.0;

		if (pEntity && pEntity->IsPlayer() && !IsPrimaryAttack)
		{
			UTIL_MakeVectors(pEntity->pev->angles);

			if (DotProduct((pEntity->pev->origin.Make2D() - pAttackPlayer->pev->origin.Make2D()).Normalize(), gpGlobals->v_forward.Make2D()) > 0.8)
				flRatio = 3.0;
		}

		pAttackPlayer->m_iWeaponVolume = 128;

		ClearMultiDamage();
		if (flRatio == 3.0) bitsDamageType |= DMG_BACKATK;
		pEntity->TraceAttack(pAttackPlayer->pev, flDamage * flRatio, (tr.vecEndPos - vecSrc).Normalize(), &tr, bitsDamageType);

		ApplyMultiDamage(pevInflictor, pAttackPlayer->pev);

		result = HIT_PLAYER;
	}

	if(!result)
	{
		static int calced = 0;
		static float calcRes = 0.0f;

		if (!(calced & 1))
		{
			calced |= 1;
			calcRes = sqrt(512.0);
		}

		vecEnd = vecSrc + vecForward * (flRadius - calcRes);

		UTIL_TraceHull(vecSrc, vecEnd, dont_ignore_monsters, human_hull, pAttackPlayer->edict(), &tr);

		if (tr.flFraction < 1.0f)
			result = HIT_WALL;
	}

	return result;
}

}
#endif

