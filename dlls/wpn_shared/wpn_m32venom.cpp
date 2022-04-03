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
#include "wpn_m32venom.h"

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

#ifndef CLIENT_DLL
void CM32VenomMark::Spawn(void)
{
	Precache();

	pev->movetype = MOVETYPE_NOCLIP;
	pev->solid = SOLID_NOT;
	pev->rendermode = kRenderTransAdd;
	pev->renderamt = 180;
	pev->frame = 0;

	pev->framerate = 1.0;
	m_flNumFrames = (float)(MODEL_FRAMES(pev->modelindex) - 1);

	SET_MODEL(ENT(pev), "sprites/head_poison.spr");
	UTIL_SetSize(pev, g_vecZero, g_vecZero);
	UTIL_SetOrigin(pev, pev->origin);

	pev->classname = MAKE_STRING("m32venom");
	SetThink(&CM32VenomMark::VenomThink);

	m_flVenomDamageTime = gpGlobals->time + 0.1s;
	m_flSprEffectTime = gpGlobals->time + 0.1s;
	m_flRemoveTime = gpGlobals->time + 8.0s;
	pev->nextthink = gpGlobals->time + 0.1s;
}

void CM32VenomMark::Precache(void)
{
	PRECACHE_MODEL("sprites/head_poison.spr");
}

float CM32VenomMark::GetVenomDamage() const
{
	float flDamage = 30;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
	{
		flDamage = 300;	//600
	}
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
	{
		flDamage = 1300;
	}
#endif
	return flDamage;
}

float CM32VenomMark::GetRadius() const
{
	float flRadius = 4 * 39.37;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
		{
			flRadius = 4 * 39.37;
		}
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
		{
			flRadius = 4 * 39.37;
		}
#endif
	return flRadius;
}

void EXPORT CM32VenomMark::VenomThink(void)
{
	pev->nextthink = gpGlobals->time + 0.1s;

	pev->frame += pev->framerate * gpGlobals->frametime / 1s;

	if (pev->frame > m_flNumFrames)
		pev->frame = fmod(pev->frame, m_flNumFrames);

	if (!m_pEnemy || !m_pAttacker)
	{
		Remove();
		return;
	}

	if (!m_pEnemy->IsAlive() || m_pAttacker->m_bIsZombie || !m_pAttacker->IsAlive())
	{
		Remove();
		return;
	}

	UTIL_SetOrigin(pev, m_pEnemy->pev->origin + Vector(0, 0, 60));

	if (m_flSprEffectTime < gpGlobals->time)
	{
		m_flSprEffectTime = gpGlobals->time + 1.0s;

		PLAYBACK_EVENT_FULL(FEV_GLOBAL, this->edict(), PRECACHE_EVENT(1, "events/wpneffects.sc"), 0.0, pev->origin, g_vecZero, 1.0, 0.0, 22, m_pEnemy->entindex(), FALSE, TRUE);
	}

	float flRadius = GetRadius();
	CBaseEntity* pEntity = NULL;
	if (m_flVenomDamageTime < gpGlobals->time)
	{
		m_flVenomDamageTime = gpGlobals->time + 1.0s;

		while ((pEntity = UTIL_FindEntityInSphere(pEntity, m_pEnemy->pev->origin, flRadius)) != NULL)
		{
			if (pEntity->IsDormant())
				continue;

			if (pEntity->pev->takedamage == DAMAGE_NO)
				continue;

			if (pEntity->pev == m_pAttacker->pev)
				continue;

			float flDamage = GetVenomDamage();

			pEntity->TakeDamage(pev, m_pAttacker->pev, flDamage, DMG_NEVERGIB | DMG_EXPLOSION);
		}

	}

	if (m_flRemoveTime < gpGlobals->time)
	{
		Remove();
	}
}

void CM32VenomMark::Remove()
{
	m_pEnemy->m_iBuff &= ~BUFF_M32VENOM;
	SetThink(NULL);
	SetThink(&CBaseEntity::SUB_StartFadeOut);
}

	//KnockbackData GetKnockBackData() override { return { 500.0f, 600.0f, 600.f, 300.0f, 1.0f }; }

LINK_ENTITY_TO_CLASS(m32venom_mark, CM32VenomMark);

void CM32VenomRocket::Spawn(void)
{
	Precache();

	pev->movetype = MOVETYPE_BOUNCE;
	pev->solid = SOLID_BBOX;

	SET_MODEL(ENT(pev), "models/s_grenade.mdl");
	UTIL_SetSize(pev, { -8, -8, -2 }, { 8, 8, 1.3 });
	UTIL_SetOrigin(pev, pev->origin);

	pev->classname = MAKE_STRING("d_m32venom");
	SetThink(&CM32VenomRocket::IgniteThink);
	SetTouch(&CM32VenomRocket::RocketTouch);

	pev->angles.x -= 30.0;
	UTIL_MakeVectors(pev->angles);
	pev->angles.x = -(pev->angles.x + 30.0);

	pev->velocity = gpGlobals->v_forward * 300;
	pev->gravity = 0.5;
	pev->nextthink = gpGlobals->time + 0.1s;
}

void CM32VenomRocket::Precache(void)
{
	PRECACHE_MODEL("models/s_grenade.mdl");

	m_iTrail = PRECACHE_MODEL("sprites/smoke.spr");
}

void CM32VenomRocket::FollowThink(void)
{
	UTIL_MakeAimVectors(pev->angles);

	pev->angles = UTIL_VecToAngles(gpGlobals->v_forward);
	pev->nextthink = gpGlobals->time + 0.1s;
}

void CM32VenomRocket::IgniteThink(void)
{
	pev->movetype = MOVETYPE_BOUNCE;

	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
	WRITE_BYTE(TE_BEAMFOLLOW);
	WRITE_SHORT(entindex());
	WRITE_SHORT(m_iTrail);
	WRITE_BYTE(40);
	WRITE_BYTE(5);
	WRITE_BYTE(224);
	WRITE_BYTE(224);
	WRITE_BYTE(255);
	WRITE_BYTE(255);
	MESSAGE_END();

	m_flIgniteTime = gpGlobals->time;

	SetThink(&CM32VenomRocket::FollowThink);
	pev->nextthink = gpGlobals->time + 0.1s;
}

void CM32VenomRocket::VenomThink(void)
{
	pev->nextthink = gpGlobals->time + 0.1s;

	CBaseEntity* pAttacker = CBaseEntity::Instance(pev->owner);
	CBasePlayer* pAttackPlayer = nullptr;
	if (pAttacker && pAttacker->IsPlayer())
		pAttackPlayer = static_cast<CBasePlayer*>(pAttacker);

	if (!pAttackPlayer || pAttackPlayer->m_bIsZombie)
	{
		SetTouch(NULL);
		SetThink(NULL);

		UTIL_Remove(this);
		return;
	}

	if (m_flSprEffectTime < gpGlobals->time)
	{
		m_flSprEffectTime = gpGlobals->time + 999s;

		PLAYBACK_EVENT_FULL(FEV_GLOBAL, this->edict(), PRECACHE_EVENT(1, "events/wpneffects.sc"), 0.0, pev->origin, g_vecZero, 2.0, 0.0, 22, 0, FALSE, FALSE);
	}
	
	if (m_flVenomDamageTime < gpGlobals->time)
	{
		m_flVenomDamageTime = gpGlobals->time + 0.5s;

		float flDamage = GetVenomDamage();
		float flRadius = GetRadius();
		CBaseEntity* pEntity = NULL;

		while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, flRadius)) != NULL)
		{
			if (pEntity->IsDormant())
				continue;

			if (pEntity->pev->takedamage == DAMAGE_NO)
				continue;

			if (pEntity == pAttackPlayer)
				continue;

			if (pEntity->pev == pAttackPlayer->pev)
				continue;

			pEntity->TakeDamage(pev, pAttackPlayer->pev, GetVenomDamage(), DMG_NEVERGIB | DMG_EXPLOSION);
		}

	}

	if (m_flRemoveTime < gpGlobals->time)
	{
		SetTouch(NULL);
		SetThink(NULL);

		UTIL_Remove(this);
	}
}

void CM32VenomRocket::RocketTouch(CBaseEntity* pOther)
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

	EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "weapons/m32venom_exp.wav", VOL_NORM, 0.25, 0, PITCH_NORM);

	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
	WRITE_BYTE(TE_KILLBEAM);
	WRITE_SHORT(this->entindex());
	MESSAGE_END();

	SetTouch(NULL);
	SetThink(&CM32VenomRocket::VenomThink);
	pev->movetype = MOVETYPE_TOSS;
	pev->velocity = g_vecZero;
	pev->gravity = 0.2;
	m_flRemoveTime = gpGlobals->time + 3.0s;
	m_flSprEffectTime = gpGlobals->time + 1.1s;
	pev->nextthink = gpGlobals->time + 0.1s;
}

//KnockbackData GetKnockBackData() override { return { 500.0f, 600.0f, 600.f, 300.0f, 1.0f }; }

LINK_ENTITY_TO_CLASS(m32venom_rocket, CM32VenomRocket);
#endif

LINK_ENTITY_TO_CLASS(weapon_m32venom, CM32Venom)

void CM32Venom::Spawn(void)
{
	Precache();
	m_iId = WEAPON_XM1014;
	pev->classname = MAKE_STRING("weapon_m32venom");
	SET_MODEL(ENT(pev), "models/@m32venom_w_m32.mdl");
	m_iDefaultAmmo = 18;

	FallInit();
}

void CM32Venom::Precache(void)
{
	PRECACHE_MODEL("models/@m32venom_v_m32.mdl");
	PRECACHE_MODEL("models/@m32venom_w_m32.mdl");
	PRECACHE_MODEL("models/@m32venom_p_m32.mdl");
#ifndef CLIENT_DLL
	PRECACHE_MODEL("sprites/muzzleflash17.spr");
#endif
	m_iShellId = PRECACHE_MODEL("models/shotgunshell.mdl");

	PRECACHE_SOUND("weapons/m32venom_exp.wav");

	m_usFireM32Venom = PRECACHE_EVENT(1, "events/m32.sc");
}

int CM32Venom::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "M32VenomGrenade";
	p->iMaxAmmo1 = 36;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = 6;
	p->iSlot = 0;
	p->iPosition = 12;
	p->iId = m_iId = WEAPON_XM1014;
	p->iFlags = 0;
	p->iWeight = XM1014_WEIGHT;

	return 1;
}

BOOL CM32Venom::Deploy(void)
{
#ifndef CLIENT_DLL
	m_pPlayer->UpdateShieldCrosshair(false);
#endif

	return DefaultDeploy("models/@m32venom_v_m32.mdl", "models/@m32venom_p_m32.mdl", M32VENOM_DRAW, "m249", UseDecrement() != FALSE);
}

void CM32Venom::SecondaryAttack(void)
{
	switch (m_pPlayer->m_iFOV)
	{
	case 90: m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 65; break;
	default: m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 90; break;
	}

	m_pPlayer->ResetMaxSpeed();
	EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/zoom.wav", 0.2, 2.4);
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3s;
}

void CM32Venom::PrimaryAttack(void)
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

		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.88s;
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
	Vector vecSrc = m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 13 + gpGlobals->v_right * -1.5 + gpGlobals->v_up * -1.5;
	CM32VenomRocket* pEnt = static_cast<CM32VenomRocket*>(CBaseEntity::Create("m32venom_rocket", vecSrc, m_pPlayer->pev->v_angle, m_pPlayer->edict()));
	if (pEnt)
	{
		pEnt->m_iTeam = m_pPlayer->m_iTeam;
		UTIL_MakeVectors(m_pPlayer->pev->v_angle);
		if (m_pPlayer->m_iFOV == 90)
			pEnt->pev->velocity += gpGlobals->v_forward * 800;
		else
			pEnt->pev->velocity += gpGlobals->v_forward * 1000;
	}
#endif
	float flVol = VOL_NORM;

	PLAYBACK_EVENT_FULL(0, ENT(m_pPlayer->pev), m_usFireM32Venom, 0, g_vecZero, g_vecZero, flVol, 0.0735, 7, m_vVecAiming.x * 100, FALSE, TRUE);
	
#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
	if (m_iClip)
		m_flPumpTime = UTIL_WeaponTimeBase() + 0.125s;

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.76s;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.76s;

	if (m_iClip)
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;
	else
		m_flTimeWeaponIdle = 0.75s;

	m_fInSpecialReload = 0;

	if (m_pPlayer->pev->flags & FL_ONGROUND)
		m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 3, 4);
	else
		m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 7, 10);
}

void CM32Venom::Reload(void)
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 || m_iClip == 6)
		return;

	if (m_flNextPrimaryAttack > UTIL_WeaponTimeBase())
		return;

	if (!m_fInSpecialReload)
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
		SendWeaponAnim(M32VENOM_START_RELOAD, UseDecrement() != FALSE);

		m_fInSpecialReload = 1;
		m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.88s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.88s;
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.88s;
	}
	else if (m_fInSpecialReload == 1)
	{
		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;

		m_fInSpecialReload = 2;

		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/m32venom_insert.wav", VOL_NORM, ATTN_NORM, 0, 85 + RANDOM_LONG(0, 31));

		SendWeaponAnim(M32VENOM_INSERT, UseDecrement());

		m_flNextReload = UTIL_WeaponTimeBase() + 0.68s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.68s;
	}
	else
	{
		m_iClip++;
		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;
		m_fInSpecialReload = 1;
		m_pPlayer->ammo_buckshot--;
	}
}

void CM32Venom::WeaponIdle(void)
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
			if (m_iClip != 6 && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
			{
				Reload();
			}
			else
			{
				SendWeaponAnim(M32VENOM_AFTER_RELOAD, UseDecrement() != FALSE);

				m_fInSpecialReload = 0;
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.77s;
			}
		}
		else
		{
			SendWeaponAnim(M32VENOM_IDLE, UseDecrement() != FALSE);

			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20.0s;
		}
	}
}

float CM32Venom::GetDamage() const
{
	float flDamage = 20.0f;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flDamage = 20.0f;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 42.0f;
#endif
	return flDamage;
}

}
