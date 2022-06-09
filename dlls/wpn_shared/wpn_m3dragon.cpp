/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "wpn_m3dragon.h"

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

#ifndef CLIENT_DLL
class CM3DragonGrenade : public CGrenade
{
public:
	void Spawn() override
	{
		Precache();

		SetThink(&CM3DragonGrenade::FlyThink);
		SetTouch(&CM3DragonGrenade::OnTouch);
		SET_MODEL(this->edict(), "models/y20s2sacr4_grenade.mdl");

		pev->rendermode = kRenderNormal;
		pev->renderfx = kRenderFxNone;
		pev->renderamt = 255;
		pev->framerate = 1.0;
		pev->solid = SOLID_TRIGGER;
		pev->movetype = MOVETYPE_FLY;

		m_flRemoveTime = gpGlobals->time + 4.0s;
		UTIL_SetSize(pev, g_vecZero, g_vecZero);

		pev->classname = MAKE_STRING("d_y20s2scard");

		pev->nextthink = gpGlobals->time + 0.01s;
	}

	void Precache() override
	{
		PRECACHE_MODEL("models/ef_fireball2.mdl");

		m_ExpIndex = PRECACHE_MODEL("sprites/ef_y20s2scar.spr");

		PRECACHE_SOUND("weapons/m3dragon_exp.wav");
	}

	void EXPORT Ontouch(CBaseEntity* pOther)
	{
		if (!pOther)
			return;

		if (pOther->edict() == pev->owner)
			return;

		float flExplodeDamage = GetExplodeDamage();
		float flRadius = GetRadius();
		entvars_t* pevOwner = VARS(pev->owner);

		CBaseEntity* pAttacker = CBaseEntity::Instance(pev->owner);
		CBasePlayer* pAttackPlayer = nullptr;
		if (pAttacker && pAttacker->IsPlayer())
			pAttackPlayer = static_cast<CBasePlayer*>(pAttacker);

		if (!pAttackPlayer)
			return;

		if (!pOther->IsDormant())
		{
			if (pOther->pev->takedamage != DAMAGE_NO)
			{
				pOther->TakeDamage(pev, pevOwner, flExplodeDamage, DMG_NEVERGIB | DMG_EXPLOSION);
			}
		}

		CBaseEntity* pEntity = NULL;

		while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, flRadius)) != NULL)
		{
			if (pEntity->IsDormant())
				continue;

			if (pEntity->pev->takedamage == DAMAGE_NO)
				continue;

			if (pEntity == pOther)
				continue;

			if (pEntity->edict() == pev->owner)
				flExplodeDamage *= 0.35;

			int falloff = flExplodeDamage / flRadius;

			float flCurrentDamage = max(0, flExplodeDamage - (pev->origin - pEntity->pev->origin).Length() * falloff);

			pEntity->TakeDamage(pev, pevOwner, flCurrentDamage, DMG_NEVERGIB | DMG_EXPLOSION);
		}

		PLAYBACK_EVENT_FULL(FEV_GLOBAL, this->edict(), PRECACHE_EVENT(1, "events/wpneffects.sc"), 0.0, pev->origin, g_vecZero, 2.0, 0.0, 22, 0, TRUE, FALSE);

		EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "weapons/m3dragon_exp.wav", VOL_NORM, 0.25, 0, PITCH_NORM);

		SetTouch(NULL);
		SetThink(&CM32VenomRocket::VenomThink);
		pev->movetype = MOVETYPE_TOSS;
		pev->velocity = g_vecZero;
		pev->gravity = 0.2;
		m_flRemoveTime = gpGlobals->time + 3.0s;
		m_flSprEffectTime = gpGlobals->time + 1.1s;
		pev->nextthink = gpGlobals->time + 0.1s;
	}

	void CreateEffect()
	{
		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_EXPLOSION);
		WRITE_COORD(pev->origin[0]);
		WRITE_COORD(pev->origin[1]);
		WRITE_COORD(pev->origin[2]);
		WRITE_SHORT(m_ExpIndex);
		WRITE_BYTE(6);
		WRITE_BYTE(15);
		WRITE_BYTE(TE_EXPLFLAG_NOPARTICLES | TE_EXPLFLAG_NODLIGHTS | TE_EXPLFLAG_NOSOUND);
		MESSAGE_END();

		EMIT_SOUND_DYN(ENT(this->pev), CHAN_ITEM, "weapons/y20s2scard-2_exp.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
	}

	void RadiusDamage()
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
				}
			}
		}
	}

	void EXPORT FlyThink()
	{
		if (m_flRemoveTime <= gpGlobals->time)
		{
			RadiusDamage();
			Remove();
			return;
		}

		pev->nextthink = gpGlobals->time + 0.1s;
	}

	float GetGrenadeExpDamage() const
	{
		float flDamage = 20;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 415;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 1000;
#endif
		return flDamage;
	}

	void Init(Vector vecVelocity, float flTouchDamage, float flExplodeDamage, int iPenetrationTime, edict_t* pAttachedEnt)
	{
		m_flTouchDamage = flTouchDamage;
		RadiusDamageAmount = flExplodeDamage;
		m_vecStartVelocity = pev->velocity = vecVelocity;
		m_iPenetrationTime = iPenetrationTime;
		m_pAttachedEnt = pAttachedEnt;
	}

	KnockbackData GetKnockBackData() override { return { 0.0f, 0.0f, 0.0f, 0.0f, 1.0f }; }

	edict_t* m_pAttachedEnt;
	int m_iPenetrationTime;
	time_point_t m_flRemoveTime;
	float m_flTouchDamage;
	float RadiusDamageAmount;
	int m_ExpIndex;
	TeamName m_iTeam;
	Vector m_vecStartVelocity;

protected:
	void Remove()
	{
		SetThink(nullptr);
		SetTouch(nullptr);
		pev->effects |= EF_NODRAW; // 0x80u
		return UTIL_Remove(this);
	}
};
LINK_ENTITY_TO_CLASS(m3dragon_grenade, CM3DragonGrenade)

#endif
LINK_ENTITY_TO_CLASS(weapon_m3dragon, CM3Dragon)

void CM3Dragon::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_m3dragon");

	Precache();
	m_iId = WEAPON_m3dragon;
	SET_MODEL(ENT(pev), "models/w_m3dragon.mdl");
	m_iDefaultAmmo = m3dragon_DEFAULT_GIVE;

	FallInit();
}

void CM3Dragon::Precache(void)
{
	PRECACHE_MODEL("models/v_m3dragon.mdl");
	PRECACHE_MODEL("models/w_m3dragon.mdl");

	m_iShellId = m_iShell = PRECACHE_MODEL("models/shotgunshell.mdl");

	PRECACHE_SOUND("weapons/m3dragon-1.wav");
	PRECACHE_SOUND("weapons/m3dragon_insertshell.wav");
	PRECACHE_SOUND("weapons/m3dragon_pump.wav");
	PRECACHE_SOUND("weapons/reload1.wav");
	PRECACHE_SOUND("weapons/reload3.wav");

	m_usFirem3dragon = PRECACHE_EVENT(1, "events/m3dragon.sc");
}

int CM3Dragon::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "buckshot";
	p->iMaxAmmo1 = MAX_AMMO_BUCKSHOT;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = m3dragon_MAX_CLIP;
	p->iSlot = 0;
	p->iPosition = 5;
	p->iId = m_iId = WEAPON_m3dragon;
	p->iFlags = 0;
	p->iWeight = m3dragon_WEIGHT;

	return 1;
}

BOOL CM3Dragon::Deploy(void)
{

	return DefaultDeploy("models/v_m3dragon.mdl", "models/p_m3dragon.mdl", m3dragon_DRAW, "shotgun", UseDecrement() != FALSE);
}

void CM3Dragon::SecondaryAttack(void)
{
	
}

void CM3Dragon::PrimaryAttack(void)
{
	if (m_pPlayer->pev->waterlevel == 3)
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15s;
		return;
	}

	if (m_iClip <= 0)
	{
		Reload();

		if (m_iClip == 0)
			PlayEmptySound();

		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.0s;
		return;
	}

	m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	m_iClip--;
	m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
#ifndef CLIENT_DLL
	m_pPlayer->FireBullets(9, m_pPlayer->GetGunPosition(), gpGlobals->v_forward, Vector(0.0675, 0.0675, 0), 3000, BULLET_PLAYER_BUCKSHOT, 0);
#endif

	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, ENT(m_pPlayer->pev), m_usFirem3dragon, 0, g_vecZero, g_vecZero, 0, 0, 0, 0, FALSE, FALSE);

	if (m_iClip)
		m_flPumpTime = UTIL_WeaponTimeBase() + 0.5s;

#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
	if (m_iClip)
		m_flPumpTime = UTIL_WeaponTimeBase() + 0.5s;

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.875s;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.875s;

	if (m_iClip)
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.5s;
	else
		m_flTimeWeaponIdle = 0.875s;

	m_fInSpecialReload = 0;

	if (m_pPlayer->pev->flags & FL_ONGROUND)
		m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 4, 6);
	else
		m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 8, 11);

	m_pPlayer->m_flEjectBrass = gpGlobals->time + 0.45s;
}

void CM3Dragon::Reload(void)
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 || m_iClip == m3dragon_MAX_CLIP)
		return;

	if (m_flNextPrimaryAttack > UTIL_WeaponTimeBase())
		return;

	if (!m_fInSpecialReload)
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
		SendWeaponAnim(m3dragon_START_RELOAD, UseDecrement() != FALSE);

		m_fInSpecialReload = 1;
		m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.55s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.55s;
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.55s;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.55s;
	}
	else if (m_fInSpecialReload == 1)
	{
		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;

		m_fInSpecialReload = 2;
		SendWeaponAnim(m3dragon_RELOAD, UseDecrement() != FALSE);

		m_flNextReload = UTIL_WeaponTimeBase() + 0.45s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.45s;
	}
	else
	{
		m_iClip++;
		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;
		m_fInSpecialReload = 1;
		m_pPlayer->ammo_buckshot--;
	}
}

void CM3Dragon::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);

	if (m_flPumpTime != zero_duration && m_flPumpTime < UTIL_WeaponTimeBase())
		m_flPumpTime = zero_duration;

	if (m_flTimeWeaponIdle < UTIL_WeaponTimeBase())
	{
		if (m_iClip == 0 && m_fInSpecialReload == 0 && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
		{
			Reload();
		}
		else if (m_fInSpecialReload != 0)
		{
			if (m_iClip != m3dragon_MAX_CLIP && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
			{
				Reload();
			}
			else
			{
				SendWeaponAnim(m3dragon_PUMP, UseDecrement() != FALSE);

				m_fInSpecialReload = 0;
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;
			}
		}
		else
			SendWeaponAnim(m3dragon_IDLE, UseDecrement() != FALSE);
	}
}

}
