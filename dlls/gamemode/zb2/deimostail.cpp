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
	m_iBossBig00Index = PRECACHE_MODEL("models/monster/zbs_bossl_big00.mdl"); 
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

	const char* mod_str = CVAR_GET_STRING("mp_gamemode");
	bool bIsZbzMode = (!strcmp(mod_str, "zbz")) ? true : false;

	if (m_bIsZombieSkill && !RANDOM_LONG(0, 8) && bIsZbzMode)
	{
		SetTouch(NULL);

		pev->velocity = g_vecZero;

		UTIL_TempModel(pev->origin, g_vecZero, g_vecZero, m_iBossBig00Index, 30, 9, 10, true, 64, 255, kRenderNormal, NULL, false, 0, 0, 256, 0);

		SetThink(&CDeimosTail::DropGunEX);

		pev->nextthink = gpGlobals->time + 2.1s;
	}
	else
	{
		SetThink(NULL);
		SetTouch(NULL);

		UTIL_Remove(this);
	}
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

void CDeimosTail::DropGunEX()
{
	SetThink(NULL);

	TraceResult tr;
	float flDamage;
	CBaseEntity* pList[100];
	Vector vecStart;

	vecStart = pev->origin + Vector(0.0, 0.0, 5.0);

	const float flRange = 2.5 * 39.37;
	int iCount = UTIL_EntitiesInBox(pList, 100, vecStart - Vector(flRange, flRange, flRange), vecStart + Vector(flRange, flRange, flRange), FL_CLIENT | FL_MONSTER);

	flDamage = 200.0;

	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, vecStart);
	WRITE_BYTE(TE_BEAMCYLINDER);
	WRITE_COORD(vecStart[0]);
	WRITE_COORD(vecStart[1]);
	WRITE_COORD(vecStart[2]);
	WRITE_COORD(vecStart[0]);
	WRITE_COORD(vecStart[1]);
	WRITE_COORD(vecStart[2] + 200);
	WRITE_SHORT(g_sModelIndexShockWave);
	WRITE_BYTE(0); // Start Frame
	WRITE_BYTE(0); // Framerate
	WRITE_BYTE(5); // Live Time
	WRITE_BYTE(60); // Width
	WRITE_BYTE(0); // Noise
	WRITE_BYTE(130); // R
	WRITE_BYTE(139); // G
	WRITE_BYTE(253); // B
	WRITE_BYTE(150); // Bright
	WRITE_BYTE(0); // Speed
	MESSAGE_END();

	for (int i = 0; i < iCount; i++)
	{
		CBaseEntity* pEntity = pList[i];

		if (pEntity == this)
			continue;

		if (!pEntity->IsPlayer())
			continue;

		if (pEntity->pev->takedamage == DAMAGE_NO)
			continue;

		if (pEntity->edict() == pev->owner)
			continue;

		UTIL_TraceLine(vecStart, pEntity->BodyTarget(vecStart), dont_ignore_monsters, edict(), &tr);

		if (tr.flFraction < 1.0 && tr.pHit != pEntity->edict())
			continue;

		pEntity->TakeDamage(pev, VARS(pev->owner), flDamage, DMG_EXPLOSION);

		((CBasePlayer*)pEntity)->ReactToDeimosTail();
	}

	UTIL_Remove(this);
}

}
