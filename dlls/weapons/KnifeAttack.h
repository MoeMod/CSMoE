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

#ifndef PROJECT_KNIFEATTACK_H
#define PROJECT_KNIFEATTACK_H


#ifndef CLIENT_DLL
namespace sv {

void FindHullIntersection(const Vector &vecSrc, TraceResult &tr, float *pflMins, float *pfkMaxs, edict_t *pEntity);

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
}
#endif


#endif //PROJECT_KNIFEATTACK_H
