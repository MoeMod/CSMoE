#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif

namespace sv {
LINK_ENTITY_TO_CLASS(holysword_cannon, CHolySwordCannon)

void CHolySwordCannon::Spawn()
{
	Precache();

	SetThink(&CHolySwordCannon::FlyThink);
	SetTouch(&CHolySwordCannon::OnTouch);
	SET_MODEL(this->edict(), "models/ef_holysword_chargecannon.mdl");

	pev->rendermode = kRenderTransAdd;
	pev->renderfx = kRenderFxNone;
	pev->renderamt = 180;
	pev->framerate = 1.0;
	pev->solid = SOLID_TRIGGER;
	pev->movetype = MOVETYPE_FLY;

	m_flRemoveTime = gpGlobals->time + 1.0s;
	UTIL_SetOrigin(pev, pev->origin);
	UTIL_SetSize(pev, Vector(-4, -4, -4), Vector(4, 4, 4));

	pev->classname = MAKE_STRING("holysword_cannon");

	pev->nextthink = gpGlobals->time + 0.01s;
}

void CHolySwordCannon::Precache()
{
	PRECACHE_MODEL("models/ef_holysword_chargecannon.mdl");

	m_ExpIndex = PRECACHE_MODEL("sprites/muzzleflash78.spr");
	m_Exp2Index = PRECACHE_MODEL("sprites/muzzleflash65.spr");


	PRECACHE_SOUND("weapons/holysword_cannon_exp.wav");
}

void EXPORT CHolySwordCannon::OnTouch(CBaseEntity* pOther)
{
	if (pev->owner == pOther->edict())
		return;

	CBaseEntity* pAttacker = CBaseEntity::Instance(pev->owner);
	if (!pOther->IsPlayer() || !pAttacker->IsPlayer()) {
		TouchWall();
		Remove();
		return;
	}

	CBasePlayer* pAttackePlayer = nullptr;
	if (pAttacker && pAttacker->IsPlayer())
		pAttackePlayer = static_cast<CBasePlayer*>(pAttacker);

	if (pAttackePlayer)
	{
		if (pOther->pev->takedamage != DAMAGE_NO && pOther->IsAlive())
		{
			Vector vecDirection = (pOther->pev->origin - pev->origin).Normalize();

			TraceResult tr;
			UTIL_TraceLine(pev->origin, pOther->pev->origin, missile, ENT(pAttackePlayer->pev), &tr);

			ClearMultiDamage();
			pOther->TraceAttack(pAttackePlayer->pev, m_flTouchDamage, vecDirection, &tr, DMG_BULLET);
			ApplyMultiDamage(pAttackePlayer->pev, pAttackePlayer->pev);
		}
		RadiusDamage();
	}

	PenetrateStart();
}

void CHolySwordCannon::CreateEffect()
{
	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
	WRITE_BYTE(TE_EXPLOSION);
	WRITE_COORD(pev->origin[0]);
	WRITE_COORD(pev->origin[1]);
	WRITE_COORD(pev->origin[2]);
	WRITE_SHORT(m_ExpIndex);
	WRITE_BYTE(9);
	WRITE_BYTE(15);
	WRITE_BYTE(TE_EXPLFLAG_NOPARTICLES | TE_EXPLFLAG_NODLIGHTS | TE_EXPLFLAG_NOSOUND);
	MESSAGE_END();

	EMIT_SOUND_DYN(ENT(this->pev), CHAN_ITEM, "weapons/holysword_cannon_exp.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
}

void CHolySwordCannon::RadiusDamage()
{
	CBaseEntity* pAttacker = CBaseEntity::Instance(pev->owner);
	CBasePlayer* pAttackePlayer = nullptr;
	if (pAttacker && pAttacker->IsPlayer())
		pAttackePlayer = static_cast<CBasePlayer*>(pAttacker);

	CreateEffect();
	TraceResult tr;
	CBaseEntity* pEntity = NULL;

	Vector vecEnd;
	int bInWater = (UTIL_PointContents(pev->origin) == CONTENTS_WATER);

	while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, 90.0)) != NULL)
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

		int falloff = RadiusDamageAmount / 90.0;

		float flCurrentDamage = max(0, RadiusDamageAmount - (pev->origin - pEntity->pev->origin).Length() * falloff);

		pEntity->TakeDamage(pev, pAttackePlayer->pev, flCurrentDamage, DMG_EXPLOSION);

		if (pEntity->IsPlayer())
		{
			CBasePlayer* player = (CBasePlayer*)pEntity;

			if (player->m_bIsZombie)
			{
				Vector vecDir = (player->pev->origin - pAttackePlayer->pev->origin);
				vecDir.z = 0;

				ApplyKnockbackData(player, vecDir, { 1200, 1200, 1200, 1200, 0.85f });
				player->pev->velocity.z = +200.0f;
			}
		}
	}
}

void CHolySwordCannon::TouchWall()
{
	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
	WRITE_BYTE(TE_EXPLOSION);
	WRITE_COORD(pev->origin[0]);
	WRITE_COORD(pev->origin[1]);
	WRITE_COORD(pev->origin[2]);
	WRITE_SHORT(m_Exp2Index);
	WRITE_BYTE(6);
	WRITE_BYTE(15);
	WRITE_BYTE(TE_EXPLFLAG_NOPARTICLES | TE_EXPLFLAG_NODLIGHTS | TE_EXPLFLAG_NOSOUND);
	MESSAGE_END();

	pev->movetype = MOVETYPE_NONE;
	RadiusDamage();
}

void CHolySwordCannon::PenetrateStart()
{
	pev->origin = pev->origin + m_vecStartVelocity.Normalize() * 42;
	pev->velocity = {};
	pev->solid = SOLID_NOT;
	pev->nextthink = gpGlobals->time + 0.05s;
}

void CHolySwordCannon::PenetrateEnd()
{
	pev->velocity = m_vecStartVelocity;
	pev->solid = SOLID_CUSTOM;
	pev->nextthink = gpGlobals->time + 0.001s;
}

void EXPORT CHolySwordCannon::FlyThink()
{
	if (m_flRemoveTime <= gpGlobals->time)
	{
		TouchWall();
		Remove();
		return;
	}

	pev->nextthink = gpGlobals->time + 0.01s;
	if (pev->solid == SOLID_NOT)
	{
		PenetrateEnd();
	}
}

void CHolySwordCannon::Init(Vector vecVelocity, float flTouchDamage, float flExplodeDamage)
{
	std::tie(m_flTouchDamage, RadiusDamageAmount) = std::make_tuple(flTouchDamage, flExplodeDamage);
	m_vecStartVelocity = pev->velocity = std::move(vecVelocity);
}

void CHolySwordCannon::Remove()
{
	SetThink(nullptr);
	SetTouch(nullptr);
	pev->effects |= EF_NODRAW; // 0x80u
	return UTIL_Remove(this);
}
}
