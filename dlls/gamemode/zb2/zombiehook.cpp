#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "client.h"
#include "cbase/cbase_hash.h"

#include "zombiehook.h"
#include "gamemode/mod_zb2.h"

namespace sv {

LINK_ENTITY_TO_CLASS(zombiehook, CZombieHook);

void CZombieHook::Spawn(void)
{
	Precache();

	if (pev->classname)
	{
		RemoveEntityHashValue(pev, STRING(pev->classname), CLASSNAME);
	}

	MAKE_STRING_CLASS("zombiehook", pev);
	AddEntityHashValue(pev, STRING(pev->classname), CLASSNAME);

	SetTouch(&CZombieHook::TailTouch);
	SetThink(&CZombieHook::Pulling);

	m_nPullingPlayerIndex = 0;
	m_nOwnerIndex = 0;

	pev->movetype = MOVETYPE_BOUNCE;
	pev->solid = SOLID_BBOX;
	pev->gravity = 0.15;

	SET_MODEL(ENT(pev), "models/monster/bat_witch.mdl");
	pev->sequence = 0;

	ResetSequenceInfo();

	UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));

	pev->nextthink = gpGlobals->time + 0.1s;
	m_flPullingSpeed = 400;
}

void CZombieHook::Precache(void)
{
	m_iWitchBatModelIndex = PRECACHE_MODEL("models/monster/bat_witch.mdl");
	m_iBatEffectModelIndex = PRECACHE_MODEL("sprites/ef_bat.spr");
}

void EXPORT CZombieHook::Kill(void)
{
	EMIT_SOUND_DYN(ENT(pev), CHAN_ITEM, "zombi/Zombi_Banshee_Pulling_Fire.wav", 0.0, 0.0, 1 << 5, PITCH_NORM);

	CBasePlayer* pPlayer = (CBasePlayer*)UTIL_PlayerByIndex(m_nOwnerIndex);

	if (pPlayer)
	{
		pPlayer->m_flCustomSequenceUntilTime = invalid_time_point;

		pPlayer->ResetMaxSpeed();
		pPlayer->SendFOV(90);

		CBasePlayerWeapon* pWeapon = (CBasePlayerWeapon*)pPlayer->m_pActiveItem;

		if (pWeapon)
		{
			pWeapon->m_flTimeWeaponIdle = UTIL_WeaponTimeBase();
			pWeapon->m_flNextPrimaryAttack = UTIL_WeaponTimeBase();
			pWeapon->m_flNextSecondaryAttack = UTIL_WeaponTimeBase();
		}

		pPlayer->m_iCustomSequence = -1;
	}

	MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
	WRITE_BYTE(TE_EXPLOSION);
	WRITE_COORD(pev->origin.x);
	WRITE_COORD(pev->origin.y);
	WRITE_COORD(pev->origin.z + 20);
	WRITE_SHORT(m_iBatEffectModelIndex);
	WRITE_BYTE(15);
	WRITE_BYTE(30);
	WRITE_BYTE(TE_EXPLFLAG_NONE);
	MESSAGE_END();

	SetThink(NULL);
	SetTouch(NULL);

	m_nPullingPlayerIndex = 0;
	m_nOwnerIndex = 0;

	UTIL_Remove(this);
}

void EXPORT CZombieHook::Pulling()
{
	pev->nextthink = gpGlobals->time + 0.1s;

	CBasePlayer* pOwner = (CBasePlayer*)UTIL_PlayerByIndex(m_nOwnerIndex);

	if (pOwner && pOwner->pev->deadflag == DEAD_NO)
	{
		int iSkillSeq = pOwner->LookupSequence("skill1");
		int iSkillLoopSeq = pOwner->LookupSequence("skill1_loop");

		if (pOwner->pev->sequence != iSkillLoopSeq)
		{
			if (pOwner->pev->sequence != iSkillSeq || pOwner->m_flCustomSequenceUntilTime - gpGlobals->time < 1.3s)
			{
				pOwner->pev->sequence = iSkillLoopSeq;
				pOwner->pev->frame = 0;
				pOwner->ResetSequenceInfo();

				pOwner->m_flCustomSequenceUntilTime = m_fKillTime;
			}
		}
	}

	if (!m_nPullingPlayerIndex)
		return;

	CBasePlayer* pPlayer = (CBasePlayer*)UTIL_PlayerByIndex(m_nPullingPlayerIndex);

	if (!pOwner || !pPlayer)
	{
		Kill();

		return;
	}

	Vector vecDelta = pOwner->pev->origin - pPlayer->pev->origin;
	if (vecDelta.LengthSquared() < 80 * 80)
	{
		Kill();

		return;
	}

	pPlayer->pev->velocity = vecDelta.Normalize() * m_flPullingSpeed;
	pev->origin = pPlayer->pev->origin;

	if (gpGlobals->time > m_fKillTime)
	{
		Kill();

		return;
	}
}

void EXPORT CZombieHook::TailTouch(CBaseEntity* pOther)
{
	if (!pOther || !pOther->IsPlayer() || !pOther->IsAlive())
	{
		EMIT_SOUND_DYN(ENT(pev), CHAN_ITEM, "zombi/Zombi_Banshee_Pulling_Fail.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

		Kill();
		return;
	}

	SetTouch(NULL);
	m_nPullingPlayerIndex = pOther->entindex();

	CBasePlayer* pPlayer = (CBasePlayer*)UTIL_PlayerByIndex(m_nPullingPlayerIndex);

	if (pPlayer)
	{
		EMIT_SOUND_DYN(pPlayer->edict(), CHAN_ITEM, "zombi/Human_Female_Surprise.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

		MESSAGE_BEGIN(MSG_ONE, gmsgShake, NULL, pPlayer->pev);
		WRITE_SHORT(0xA000);
		WRITE_SHORT(0x2000);
		WRITE_SHORT(0x6400);
		MESSAGE_END();

		pev->movetype = MOVETYPE_NONE;
		pev->solid = SOLID_NOT;
		pev->sequence = 1;

		ResetSequenceInfo();
	}
	else
	{
		EMIT_SOUND_DYN(ENT(pev), CHAN_ITEM, "zombi/Zombi_Banshee_Pulling_Fail.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

		Kill();
	}
}

void CZombieHook::SetLifespan(duration_t flLifespawn)
{
	m_fKillTime = gpGlobals->time + flLifespawn;
}

void CZombieHook::Init(void)
{
	m_nOwnerIndex = ENTINDEX(pev->owner);

	CBasePlayer* pPlayer = (CBasePlayer*)UTIL_PlayerByIndex(m_nOwnerIndex);

	if (!pPlayer)
		return;

	pPlayer->ResetMaxSpeed();
	pPlayer->SendFOV(120);

	EMIT_SOUND_DYN(ENT(pev), CHAN_ITEM, "zombi/Zombi_Banshee_Pulling_Fire.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
}


}