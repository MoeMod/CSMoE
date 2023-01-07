#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "client.h"
#include "cbase/cbase_hash.h"

#include "deimostail.h"
#include "gamemode/mods.h"
#include "player/player_mod_strategy.h"

#include <utility>

namespace sv {

LINK_ENTITY_TO_CLASS(deimostail, CDeimosTail);

void CDeimosTail::Spawn(void)
{
	Precache();

	pev->movetype = MOVETYPE_FLY;
	pev->solid = SOLID_BBOX;

	SET_MODEL(ENT(pev), "models/w_hiddenTail.mdl");
	SetTouch(&CDeimosTail::TailTouch);
	SetThink(NULL);

	m_bIsZombieSkill = true;
	m_bHitHuman = false;

	UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));
}

void CDeimosTail::Precache(void)
{
	PRECACHE_MODEL("models/w_hiddenTail.mdl");
	PRECACHE_MODEL("sprites/deimosexp.spr");

	m_iSmokeTrail = PRECACHE_MODEL("sprites/smoke.spr");
}

void EXPORT CDeimosTail::Kill(void)
{
	MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
	WRITE_BYTE(TE_EXPLOSION);
	WRITE_COORD(pev->origin.x);
	WRITE_COORD(pev->origin.y);
	WRITE_COORD(pev->origin.z - 20);
	WRITE_SHORT(MODEL_INDEX("sprites/deimosexp.spr"));
	WRITE_BYTE(m_bHitHuman ? 100 : 70);
	WRITE_BYTE(30);
	WRITE_BYTE(TE_EXPLFLAG_NOPARTICLES | TE_EXPLFLAG_NODLIGHTS | TE_EXPLFLAG_NOSOUND);
	MESSAGE_END();

	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
	WRITE_BYTE(TE_KILLBEAM);
	WRITE_SHORT(this->entindex());
	MESSAGE_END();

	SetThink(NULL);
	SetTouch(NULL);

	UTIL_Remove(this);
}

void EXPORT CDeimosTail::TailTouch(CBaseEntity* pOther)
{
	if (!pOther || !pOther->IsPlayer() || !pOther->IsAlive())
	{
		Kill();

		return;
	}

	bool bDropWeapon = false;
	if (m_bIsZombieSkill)
		bDropWeapon = true;

	SetTouch(NULL);

	pev->solid = SOLID_NOT;
	EMIT_SOUND_DYN(pOther->edict(), CHAN_ITEM, "zombi/deimos_skill_hit.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

	if (m_bIsZombieSkill)
	{
		pOther->TakeDamage(pOther->pev, pOther->pev, 0, DMG_BULLET);

		if (!((CBasePlayer*)pOther)->m_bIsZombie)
		{
			MESSAGE_BEGIN(MSG_ONE, gmsgShake, NULL, pOther->pev);
			WRITE_SHORT(0xA000);
			WRITE_SHORT(0x2000);
			WRITE_SHORT(0x6400);
			MESSAGE_END();
		}
	}		
	else
	{
		float flDamage;
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 800.0f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 250.0f;
		else
			flDamage = 30.0f;

		CBaseEntity* pAttacker = CBaseEntity::Instance(pev->owner);
		CBasePlayer* pAttackePlayer = nullptr;
		CBaseEntity* pInflictor = pAttacker;

		if (pAttacker && pAttacker->IsPlayer())
		{
			pAttackePlayer = static_cast<CBasePlayer*>(pAttacker);
			if (pAttackePlayer->m_pActiveItem)
			{
				pInflictor = pAttackePlayer->m_pActiveItem;
			}
		}

		if (pOther->IsPlayer() && pAttackePlayer)
		{
			Vector vecDirection = (pev->velocity).Normalize();
			TraceResult tr;
			UTIL_TraceLine(pev->origin, pev->origin + vecDirection * 36.0f, dont_ignore_monsters, ENT(pAttackePlayer->pev), &tr);

			ClearMultiDamage();
			pOther->TraceAttack(pAttackePlayer->pev, flDamage, vecDirection, &tr, DMG_BULLET | DMG_NEVERGIB);
			ApplyMultiDamage(pAttackePlayer->pev, pAttackePlayer->pev);
		}		
	}
		
	if (bDropWeapon)
		((CBasePlayer*)pOther)->ReactToDeimosTail();

	m_bHitHuman = true;
	CBaseEntity* pAttacker = CBaseEntity::Instance(pev->owner);
	CBasePlayer* pAttackePlayer = nullptr;
	if (pAttacker && pAttacker->IsPlayer())
		pAttackePlayer = static_cast<CBasePlayer*>(pAttacker);


	if (m_bHitHuman && pAttackePlayer)
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgZB2Msg, nullptr, pAttackePlayer->pev);
		WRITE_BYTE(8U);//ZB2_MESSAGE_ALARM
		WRITE_BYTE(8);
		WRITE_COORD(float(pAttackePlayer->entindex()));
		MESSAGE_END();
	}

	Kill();
}

void CDeimosTail::Initialize(duration_t tLifespan)
{
	SetThink(&CDeimosTail::Kill);
	SetThink(&CDeimosTail::Kill);
	pev->nextthink = gpGlobals->time + tLifespan;

	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
	WRITE_BYTE(TE_BEAMFOLLOW);
	WRITE_SHORT(entindex());
	WRITE_SHORT(m_iSmokeTrail);
	WRITE_BYTE(20);
	WRITE_BYTE(1);
	WRITE_BYTE(m_bIsZombieSkill ? 250 : 209);
	WRITE_BYTE(m_bIsZombieSkill ? 200 : 120);
	WRITE_BYTE(m_bIsZombieSkill ? 0 : 9);
	WRITE_BYTE(255);
	MESSAGE_END();
}

}
