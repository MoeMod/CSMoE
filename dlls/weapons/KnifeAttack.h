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

namespace sv {

void FindHullIntersection(const Vector &vecSrc, TraceResult &tr, const float *pflMins, const float *pfkMaxs, edict_t *pEntity);

enum hit_result_t
{
	HIT_NONE, HIT_WALL, HIT_PLAYER,
};
inline hit_result_t
KnifeAttack(Vector vecSrc, Vector vecDir, float flDamage, float flRadius, int bitsDamageType,
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
		if (pHit && pHit->pev->takedamage != DAMAGE_NO) {
			const float flAdjustedDamage = flDamage;

			UTIL_MakeVectors(pHit->pev->angles);
			if (DotProduct((tr.vecEndPos - vecSrc).Normalize().Make2D(), gpGlobals->v_forward.Make2D()) > 0.8)
				flDamage *= 3.0;

			ClearMultiDamage();
			pHit->TraceAttack(pevInflictor, flAdjustedDamage, (tr.vecEndPos - vecSrc).Normalize(), &tr, bitsDamageType);
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

				Vector vecRealDir = (tr.vecEndPos - vecSrc).Normalize();

				UTIL_MakeVectors(pEntity->pev->angles);
				if (DotProduct(vecRealDir.Make2D(), gpGlobals->v_forward.Make2D()) > 0.8)
					flDamage *= 3.0;

				ClearMultiDamage();
				pEntity->TraceAttack(pevInflictor, flDamage, vecRealDir, &tr, bitsDamageType);
				ApplyMultiDamage(pevInflictor, pevAttacker);

				result = HIT_PLAYER;
			}
		}
	}

	return result;
}

inline hit_result_t
KnifeAttack4(Vector vecSrc, Vector vecDir, float flDamage, float flRadius, float flAngleDegrees, int bitsDamageType,
	entvars_t* pevInflictor, entvars_t* pevAttacker, bool IsPrimaryAttack)
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
			CBaseEntity* pHit = CBaseEntity::Instance(tr.pHit);

			if (!pHit || pHit->IsBSPModel()) {
				FindHullIntersection(vecSrc, tr, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX, ENT(pevAttacker));
			}

			vecEnd = tr.vecEndPos;
		}
	}

	if (tr.flFraction < 1) {
		CBaseEntity* pHit = CBaseEntity::Instance(tr.pHit);
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
			if(IsPrimaryAttack)
			{ 
				PLAYBACK_EVENT_FULL(FEV_GLOBAL, ENT(pHit->pev), PRECACHE_EVENT(1, "events/wpneffects.sc"), 0.0, tr.vecEndPos, (float*)&g_vecZero, 0.0, 0.0, 1, 0, TRUE, TRUE);
				PLAYBACK_EVENT_FULL(FEV_GLOBAL, ENT(pHit->pev), PRECACHE_EVENT(1, "events/wpneffects.sc"), 0.0, tr.vecEndPos, (float*)&g_vecZero, 0.0, 0.0, 1, 0, TRUE, FALSE);
			}
			else
			{
				PLAYBACK_EVENT_FULL(FEV_GLOBAL, ENT(pHit->pev), PRECACHE_EVENT(1, "events/wpneffects.sc"), 0.0, tr.vecEndPos, (float*)&g_vecZero, 0.0, 0.0, 1, 0, FALSE, TRUE);
				PLAYBACK_EVENT_FULL(FEV_GLOBAL, ENT(pHit->pev), PRECACHE_EVENT(1, "events/wpneffects.sc"), 0.0, tr.vecEndPos, (float*)&g_vecZero, 0.0, 0.0, 1, 0, FALSE, FALSE);
			}
			result = HIT_WALL;
		}
	}

	CBaseEntity* pEntity = nullptr;
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

				Vector vecRealDir = (tr.vecEndPos - vecSrc).Normalize();

				UTIL_MakeVectors(pEntity->pev->angles);
				if (DotProduct(vecRealDir.Make2D(), gpGlobals->v_forward.Make2D()) > 0.8)
					flDamage *= 3.0;

				ClearMultiDamage();
				pEntity->TraceAttack(pevInflictor, flDamage, vecRealDir, &tr, bitsDamageType);
				ApplyMultiDamage(pevInflictor, pevAttacker);

				if (IsPrimaryAttack)
				{
					PLAYBACK_EVENT_FULL(FEV_GLOBAL, ENT(pEntity->pev), PRECACHE_EVENT(1, "events/wpneffects.sc"), 0.0, pEntity->pev->origin, (float*)&g_vecZero, 0.0, 0.0, 1, 0, TRUE, TRUE);
					PLAYBACK_EVENT_FULL(FEV_GLOBAL, ENT(pEntity->pev), PRECACHE_EVENT(1, "events/wpneffects.sc"), 0.0, pEntity->pev->origin, (float*)&g_vecZero, 0.0, 0.0, 1, 0, TRUE, FALSE);
				}
				else
				{
					PLAYBACK_EVENT_FULL(FEV_GLOBAL, ENT(pEntity->pev), PRECACHE_EVENT(1, "events/wpneffects.sc"), 0.0, pEntity->pev->origin, (float*)&g_vecZero, 0.0, 0.0, 1, 0, FALSE, TRUE);
					PLAYBACK_EVENT_FULL(FEV_GLOBAL, ENT(pEntity->pev), PRECACHE_EVENT(1, "events/wpneffects.sc"), 0.0, pEntity->pev->origin, (float*)&g_vecZero, 0.0, 0.0, 1, 0, FALSE, FALSE);
				}
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
				}
			}
		}
	}
	return vecDir;
}
}
#endif

