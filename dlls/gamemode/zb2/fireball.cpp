#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "client.h"
#include "cbase/cbase_hash.h"

#include "fireball.h"
#include "gamemode/mods.h"
#include "player/player_mod_strategy.h"

#include <utility>

namespace sv {

LINK_ENTITY_TO_CLASS(aksha_fireball, CAkshaFireBall);

void CAkshaFireBall::Spawn(void)
{
	Precache();

	pev->movetype = MOVETYPE_TOSS;
	pev->solid = SOLID_BBOX;
	pev->gravity = 1.0;
	pev->sequence = 0;
	pev->framerate = 1.0;

	SET_MODEL(ENT(pev), "models/ef_fireball2.mdl");
	SetTouch(&CAkshaFireBall::OnTouch);
	SetThink(&CAkshaFireBall::Onthink);
	UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));
	m_flRemoveTime = gpGlobals->time + GetAliveTime();

	PLAYBACK_EVENT_FULL(FEV_GLOBAL, this->edict(), PRECACHE_EVENT(1, "events/wpneffects.sc"), 0, g_vecZero, g_vecZero, 0, 0, 20, 0, TRUE, FALSE);
	pev->nextthink = gpGlobals->time + 0.1s;
}

void CAkshaFireBall::Precache(void)
{
	PRECACHE_SOUND("zombi/akshazombi_skill_exp.wav");
	PRECACHE_MODEL("models/ef_fireball2.mdl");
	PRECACHE_MODEL("sprites/ef_aksha_fireball.spr");
	m_iFireBallDestroy = PRECACHE_MODEL("sprites/ef_aksha_fireballdestroy.spr");
	m_iFireBallExplosion = PRECACHE_MODEL("sprites/ef_aksha_fireballexplosion.spr");
}

void CAkshaFireBall::Onthink()
{
	CBaseEntity* pAttacker = CBaseEntity::Instance(pev->owner);
	CBasePlayer* pAttackePlayer = nullptr;
	if (pAttacker && pAttacker->IsPlayer())
		pAttackePlayer = static_cast<CBasePlayer*>(pAttacker);

	if (pAttackePlayer)
	{
		if (!pAttackePlayer->IsAlive())
		{
			MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
			WRITE_BYTE(TE_EXPLOSION);
			WRITE_COORD(pev->origin.x);
			WRITE_COORD(pev->origin.y);
			WRITE_COORD(pev->origin.z + 25);
			WRITE_SHORT(m_iFireBallDestroy);
			WRITE_BYTE(15);
			WRITE_BYTE(30);
			WRITE_BYTE(TE_EXPLFLAG_NOPARTICLES | TE_EXPLFLAG_NODLIGHTS | TE_EXPLFLAG_NOSOUND);
			MESSAGE_END();

			Remove();
		}
	}

	if (m_flRemoveTime < gpGlobals->time)
	{
		Explosion();
		RadiusDamage();
		Remove();
	}
}

void CAkshaFireBall::Remove(void)
{
	SetThink(NULL);
	SetTouch(NULL);

	UTIL_Remove(this);
}

void CAkshaFireBall::Explosion()
{
	EMIT_SOUND(ENT(pev), CHAN_ITEM, "zombi/akshazombi_skill_exp.wav", VOL_NORM, ATTN_NORM);

	MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
	WRITE_BYTE(TE_EXPLOSION);
	WRITE_COORD(pev->origin.x);
	WRITE_COORD(pev->origin.y);
	WRITE_COORD(pev->origin.z + 25);
	WRITE_SHORT(m_iFireBallExplosion);
	WRITE_BYTE(15);
	WRITE_BYTE(30);
	WRITE_BYTE(TE_EXPLFLAG_NOPARTICLES | TE_EXPLFLAG_NODLIGHTS | TE_EXPLFLAG_NOSOUND);
	MESSAGE_END();
}

void CAkshaFireBall::RadiusDamage()
{
	float flDamage = GetDamage();
	float flRadius = GetRange();
	float flDist;

	TraceResult tr;
	CBaseEntity* pEntity = NULL;
	Vector vecVelocityAdd;

	Vector vecEnd;
	int bInWater = (UTIL_PointContents(pev->origin) == CONTENTS_WATER);

	CBaseEntity* pAttacker = CBaseEntity::Instance(pev->owner);
	CBasePlayer* pAttackePlayer = nullptr;
	if (pAttacker && pAttacker->IsPlayer())
		pAttackePlayer = static_cast<CBasePlayer*>(pAttacker);

	if (pAttackePlayer)
	{
		while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, flRadius)) != NULL)
		{
			if (pEntity->edict() == pev->owner)
				continue;

			if (pEntity->IsDormant())
				continue;

			if (pEntity->pev->takedamage == DAMAGE_NO)
				continue;

			if (pEntity->Classify() == CLASS_NONE)
				continue;

			if (pEntity->IsPlayer())
			{
				CBasePlayer* target = static_cast<CBasePlayer*>(pEntity);

				if (pAttackePlayer->m_iTeam == target->m_iTeam)
					continue;
			}

			if ((bInWater && pEntity->pev->waterlevel == 0) || (!bInWater && pEntity->pev->waterlevel == 3))
				continue;

			vecEnd = pEntity->BodyTarget(pev->origin);

			UTIL_TraceLine(pev->origin, vecEnd, ignore_monsters, pev->owner, &tr);

			if (tr.flFraction < 1.0 && tr.pHit != pEntity->edict())
				continue;

			int falloff = flDamage / flRadius;

			float flCurrentDamage = max(0, flDamage - (pev->origin - pEntity->pev->origin).Length() * falloff);

			pEntity->TakeDamage(pev, pAttackePlayer->pev, flCurrentDamage, DMG_EXPLOSION);

			if (pEntity->IsPlayer())
			{
				CBasePlayer* player = (CBasePlayer*)pEntity;

				vecVelocityAdd = player->pev->origin - pev->origin;
				flDist = vecVelocityAdd.Length();
				vecVelocityAdd *= 1 / (flDist / GetKnockBack());
				player->pev->velocity += vecVelocityAdd;
			}
		}
	}
}

void EXPORT CAkshaFireBall::OnTouch(CBaseEntity* pOther)
{
	Explosion();

	if (pev->owner == pOther->edict())
	{
		Remove();
		return;
	}

	float flDamage = GetDamage();
	float flRadius = GetRange();
	float flDist;
	Vector vecVelocityAdd;

	CBaseEntity* pAttacker = CBaseEntity::Instance(pev->owner);
	CBasePlayer* pAttackePlayer = nullptr;
	if (pAttacker && pAttacker->IsPlayer())
		pAttackePlayer = static_cast<CBasePlayer*>(pAttacker);

	if (pAttackePlayer)
	{
		if (pOther->pev->takedamage != DAMAGE_NO && pOther->IsAlive())
		{
			Vector vecDirection = (pOther->pev->origin - pev->origin).Normalize();
			pOther->TakeDamage(pev, pAttackePlayer->pev, flDamage, DMG_EXPLOSION);

			if (pOther->IsPlayer())
			{
				vecVelocityAdd = pOther->pev->origin - pev->origin;
				flDist = vecVelocityAdd.Length();
				vecVelocityAdd *= 1 / (flDist / GetKnockBack());
				pOther->pev->velocity += vecVelocityAdd;
			}
		}

		RadiusDamage();
	}

	Remove();
}

}
