/*
RadiusDamage.h
Copyright (C) 2019 Moemod Haoyuan

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

#include "util/u_range.hpp"

namespace sv {

#include "WeaponTemplateDetails.hpp"

#if 0
template<class T> concept HasRadiusDamageRadius =
	requires(T data){ { data.RadiusDamageRadius } -> float; } || requires(T data){ { data.GetRadiusDamageRadius() } -> float; };

template<class T> concept HasRadiusDamageAmount =
	requires(T data){ { data.RadiusDamageAmount } -> float; } || requires(T data){ { data.GetRadiusDamageAmount() } -> float; };

template<class T> concept RadiusDamageArgs = HasRadiusDamageRadius<T> && HasRadiusDamageAmount<T>;
#endif

struct DynamicRadiusDamageArgs
{
	float RadiusDamageRadius;
	float RadiusDamageAmount;
	int RadiusDamageBitsDamageType = DMG_BULLET;
	KnockbackData RadiusDamageKnockback = { 0.0, 0.0, 0.0, 0.0, 1.0 };
	bool RadiusDamageCanHeadshot = false;
	bool RadiusDamageHasFallOff = true;
};

namespace df {
DF_GENERATE_TEMPLATE_GETTER(RadiusDamageRadius)
DF_GENERATE_TEMPLATE_GETTER(RadiusDamageAmount)
DF_GENERATE_TEMPLATE_GETTER_WITH_DEFAULT(RadiusDamageBitsDamageType, DMG_BULLET)
DF_GENERATE_TEMPLATE_GETTER_WITH_DEFAULT(RadiusDamageKnockback, nullptr)
DF_GENERATE_TEMPLATE_GETTER_WITH_DEFAULT(RadiusDamageCanHeadshot, false)
DF_GENERATE_TEMPLATE_GETTER_WITH_DEFAULT(RadiusDamageHasFallOff, true)
}

namespace detail {

template<class OutIter> auto GetOutIterAcceptType(const OutIter &, priority_tag<4>) -> typename OutIter::container_type::value_type;
template<class OutIter> auto GetOutIterAcceptType(const OutIter &iter, priority_tag<3>) -> typename std::decay<decltype(*iter = (entvars_t *)nullptr)>::type;
template<class OutIter> auto GetOutIterAcceptType(const OutIter &iter, priority_tag<2>) -> typename std::decay<decltype(*iter = (edict_t *)nullptr)>::type;
template<class OutIter> auto GetOutIterAcceptType(const OutIter &iter, priority_tag<1>) -> typename std::decay<decltype(*iter = (CBaseEntity *)nullptr)>::type;
template<class OutIter> auto GetOutIterAcceptType(const OutIter &, priority_tag<0>) -> CBaseEntity *; // fallbacks

template<class OutIter> void RadiusDamage_WriteToOutIter(OutIter &out, CBaseEntity *pEntity)
{
	*(out++) = static_ent_cast<decltype(GetOutIterAcceptType(out, priority_tag<5>()))>(pEntity);
}
// nothing need for nullptr
inline void RadiusDamage_WriteToOutIter(std::nullptr_t out, CBaseEntity *pEntity) {}


// we can knockback by KnockbackData
inline void RadiusDamage_TryKnockback(const KnockbackData &knockback, CBaseEntity *pEntity, const Vector &dir)
{
	CBasePlayer *pVictim = dynamic_cast<CBasePlayer *>(pEntity);
	if (pVictim && pVictim->m_bIsZombie) // Zombie Knockback...
	{
		ApplyKnockbackData(pVictim, dir, knockback);
	}
}
// nothing to knockback with nullptr (by default)
inline void RadiusDamage_TryKnockback(std::nullptr_t, CBaseEntity *pEntity, const Vector &dir) {}

}

template<class DamageData, class InflictorType = entvars_t *, class AttackerType = entvars_t *, class OutIter = std::nullptr_t>
void RadiusDamage(DamageData &&data, Vector vecSrc, InflictorType Inflictor, AttackerType Attacker, OutIter out = OutIter())
{
	const float flRadius = df::RadiusDamageRadius::Get(data);
	const float flDamage = df::RadiusDamageAmount::Get(data);
	const auto bitsDamageType = df::RadiusDamageBitsDamageType::Get(data);
	const auto knockback = df::RadiusDamageKnockback::Get(data); // KnockbackData or std::nullptr_t ?
	const bool canheadshot = df::RadiusDamageCanHeadshot::Get(data); // must be bool
	const bool hasfalloff = df::RadiusDamageHasFallOff::Get(data); // must be bool
	entvars_t* const pevAttacker = static_ent_cast<entvars_t *>(std::move(Attacker));
	entvars_t* const pevInflictor = static_ent_cast<entvars_t *>(std::move(Inflictor));

	const float falloff = hasfalloff ? (flRadius ? flDamage / flRadius : 1) : 0;
	const int bInWater = (UTIL_PointContents(vecSrc) == CONTENTS_WATER);
	// in case grenade is lying on the ground
	vecSrc.z += 1;

	for(CBaseEntity* pEntity : moe::range::EntityList<moe::Enumer_InSphere<>>(vecSrc, flRadius))
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

			const Vector vecSpot = pEntity->BodyTarget(vecSrc);
			TraceResult tr;
			UTIL_TraceLine(vecSrc, vecSpot, missile, ENT(pevInflictor), &tr);

			if (tr.flFraction == 1.0f || tr.pHit == pEntity->edict())
			{
				if (tr.fStartSolid)
				{
					tr.vecEndPos = vecSrc;
					tr.flFraction = 0;
				}
				const float flAdjustedDamage = max(0, flDamage - (vecSrc - pEntity->pev->origin).Length() * falloff);

				if (tr.flFraction == 1.0f)
				{
					pEntity->TakeDamage(pevInflictor, pevAttacker, flAdjustedDamage, bitsDamageType);
				}
				else
				{
					if(!canheadshot)
						tr.iHitgroup = HITGROUP_CHEST;
					ClearMultiDamage();
					pEntity->TraceAttack(pevInflictor, flAdjustedDamage, (tr.vecEndPos - vecSrc).Normalize(), &tr, bitsDamageType);
					ApplyMultiDamage(pevInflictor, pevAttacker);
				}

				detail::RadiusDamage_TryKnockback(knockback, pEntity, vecSpot - vecSrc);
				detail::RadiusDamage_WriteToOutIter(out, pEntity);
			}
		}
	}
}

}
#else
template<class DamageData, class InflictorType = entvars_t *, class AttackerType = entvars_t *, class OutIter = std::nullptr_t>
void RadiusDamage(DamageData &&data, Vector vecSrc, const InflictorType &Inflictor, const AttackerType &Attacker, OutIter out = OutIter())
{
	// nothing to do for client
}
#endif
