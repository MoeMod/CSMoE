/*
wpn_cannonEX.cpp - Black Dragon CannonEX from CSO
Copyright (C) 2019 Moemod Hyakuya

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "wpn_cannonex.h"
#include "weapons/KnifeAttack.h"

#ifndef CLIENT_DLL
#include "soundent.h"
#include "gamemode/mods.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

LINK_ENTITY_TO_CLASS(weapon_cannonex, CCannonEX)

enum cannonexdragon_effect_state_e
{
	EFFECT_NONE,
	EFFECT_FADEIN,
	EFFECT_FADEOUT
};

void CCannonEX::Spawn(void)
{
	Base::Spawn();
	m_iClip = m_iDefaultAmmo = ItemInfoData_t::iMaxAmmo1;
	m_iId = WEAPON_CANNONEX;
#ifndef CLIENT_DLL
	m_pDragon = NULL;
#endif
	m_iWeaponState = 0;

	InitializeData();

}

void CCannonEX::Precache(void)
{
	Base::Precache();
	PRECACHE_MODEL("sprites/flame_puff01.spr");
	PRECACHE_MODEL("sprites/flame_burn01.spr");
	PRECACHE_SOUND("weapons/cannonex-1.wav");

	m_usFireCannonEx = PRECACHE_EVENT(1, "events/cannonex.sc");

	PRECACHE_MODEL("models/p_cannonexb.mdl");
	PRECACHE_MODEL("models/w_cannonexb.mdl");
	PRECACHE_SOUND("weapons/cannonexplo.wav");

	m_iDragonFXModelIndex = PRECACHE_MODEL("models/p_cannonexdragonfx.mdl");
	m_iExplodeModelIndex = PRECACHE_MODEL("models/p_cannonexplo.mdl");
	m_iExplodeEffectModelIndex = PRECACHE_MODEL("sprites/ef_cannonex.spr");
}

void CCannonEX::InitializeData(void)
{
	m_flExplodeRadius = 200.0;
	m_flIdleTime = 3.0s;
	m_flCycleTime = 3.1s;
	m_flRange = 410;
	m_flAngleCheck1 = m_flAngleCheck2 = 35.0;
	m_flRangeModifier = 0.98;
	m_flTransformTime = 3.2s;
	m_flDragonFireTime = 0.6s;
	m_flReloadDelay = 2.5s;
	m_flReloadTime = 2.47s;

	m_strWeaponModel = "models/p_cannonex.mdl";
	m_strWeaponModelB = "models/p_cannonexb.mdl";
}

void CCannonEX::PrimaryAttack(void)
{
	if (m_bDragonReloading || m_bDragonTransforming)
		return;

	CannonEXFire(0.0, m_flIdleTime, FALSE);
}


void CCannonEX::SecondaryAttack(void)
{
	if (m_bDragonReloading || m_bDragonTransforming)
		return;

	if (m_iWeaponState)
		return;

	SendWeaponAnim(CANNONEX_DRAGON_TRANSFORM, UseDecrement() != FALSE);

	strcpy(m_pPlayer->m_szAnimExtention, "skullaxe");
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
	UTIL_TempModel(g_vecZero, m_pPlayer->pev->angles, g_vecZero, m_iDragonFXModelIndex, (m_flTransformTime.count() * 10.0), 0, 10, 0, 0, 255, kRenderTransAdd, m_pPlayer, false, 0, 0, 255, 0);
#endif
	strcpy(m_pPlayer->m_szAnimExtention, "carbine");

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + m_flTransformTime;

	m_bDragonTransforming = true;
	m_iWeaponState2 = 1;
	m_flTimeCheckState = gpGlobals->time + m_flTransformTime;
}

void CCannonEX::CannonEXFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
{
	if (m_iClip <= 0)
	{
		PlayEmptySound();

		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.4s;

		return;
	}

	m_iClip--;
	m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
	

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	Vector vecSrc = m_pPlayer->GetGunPosition();
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	Vector vecDir = KnifeAttack2(vecSrc, gpGlobals->v_forward, GetDamageAmount(), 405.0f, 45.0f, DMG_BULLET, this->pev, m_pPlayer->pev);
	m_pPlayer->m_iExtraSoundTypes = bits_SOUND_DANGER;

	if (!m_iClip)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", 0, 0);
#endif
	m_pPlayer->m_iWeaponVolume = BIG_EXPLOSION_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	

	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		KickBack(13.0, 5.0, 1.85, 0.55, 15.0, 5.7, 2);
	else if (m_pPlayer->pev->velocity.Length2D() > 0)
		KickBack(13.0, 2.25, 1.45, 0.7, 12.0, 10.0, 1);
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		KickBack(9.0, 2.1, 1.25, 0.5, 15.0, 5.5, 1);
	else
		KickBack(13.0, 3.2, 1.5, 0.5, 15.0, 10.0, 2);

	switch (m_iWeaponState)
	{
	case 0:
	case 1:
	case 2:
		if (m_iWeaponState == 1)
		{
			CreateDragon();

			m_iWeaponState2 = 4;
			m_iWeaponState = 2;
			m_flTimeCheckState = gpGlobals->time + GetDragonRemovalTime();
		}
#ifndef CLIENT_DLL
		PLAYBACK_EVENT_FULL(0, m_pPlayer->edict(), m_usFireCannonEx, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, int(m_pPlayer->pev->punchangle.x * 100.0), int(m_pPlayer->pev->punchangle.y * 100.0), TRUE, FALSE);
#endif
		SendWeaponAnim(!m_iWeaponState ? CANNONEX_SHOOTA : CANNONEX_SHOOTB, UseDecrement() != FALSE);

		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + m_flCycleTime;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + m_flIdleTime;
		return;

	case 3:
#ifndef CLIENT_DLL
		PLAYBACK_EVENT_FULL(0, m_pPlayer->edict(), m_usFireCannonEx, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, int(m_pPlayer->pev->punchangle.x * 100.0), int(m_pPlayer->pev->punchangle.y * 100.0), FALSE, FALSE);
#endif
		SendWeaponAnim(CANNONEX_DRAGON_RELOAD1, UseDecrement() != FALSE);

		m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + m_flReloadDelay + m_flReloadTime;
		m_bDragonReloading = true;
		m_iWeaponState2 = 2;
		m_flTimeCheckState = gpGlobals->time + m_flReloadDelay;

		return;

	default:
		return;
	}
}


void CCannonEX::CreateDragon(void)
{
#ifndef CLIENT_DLL
	if (m_pDragon != NULL)
		return;

	m_pDragon = (CCannonEXDragon*)Create("cannonex_dragon", m_pPlayer->pev->origin + Vector(0, 0, 35), g_vecZero, m_pPlayer->edict());

	if (m_pDragon)
		m_pDragon->DragonInit(m_pPlayer, this);
#endif
}

BOOL CCannonEX::Deploy(void)
{
	InitializeData();

	m_flAccuracy = 0;
	m_iShotsFired = 0;
	m_bDelayFire = false;
	m_bDragonReloading = false;
	m_bDragonTransforming = false;
	ResetState();

	if (m_iWeaponState != 0)
		m_iWeaponState = 3;

	RemoveDragon();

	m_pPlayer->ResetMaxSpeed();

	return DefaultDeploy(V_Model, m_iWeaponState ? "models/p_cannonexb.mdl" : P_Model, m_iWeaponState ? CANNONEX_DRAWB : CANNONEX_DRAW, "carbine", UseDecrement() != 0, 0.75s, 1.5s);
}


void CCannonEX::Explode(void)
{
#ifndef CLIENT_DLL
	Vector vecSrc;
	Vector vecDir;
	Vector2D vecAngleCheckDir;
	CBaseEntity* pEntity = NULL;
	TraceResult tr;

	UTIL_MakeVectors(m_pPlayer->pev->v_angle);

	vecDir = gpGlobals->v_forward;
	vecAngleCheckDir = gpGlobals->v_forward.Make2D().Normalize();
	vecSrc = m_pPlayer->GetGunPosition();

	while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, m_flExplodeRadius)) != NULL)
	{
		if (pEntity == m_pPlayer)
			continue;

		if (pEntity->IsDormant())
			continue;

		if (pEntity->pev->takedamage == DAMAGE_NO)
			continue;

		if (UTIL_AngularCheck(pEntity, m_pPlayer, vecSrc, vecDir, vecAngleCheckDir, m_flExplodeRadius, -1.0, &tr) == false)
			continue;

		ClearMultiDamage();

		pEntity->TraceAttack(m_pPlayer->pev, GetExplodeDamage(), (tr.vecEndPos - vecSrc).Normalize(), &tr, DMG_BULLET | DMG_NEVERGIB);

		ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);
	}

	UTIL_TempModel(m_pPlayer->pev->origin, m_pPlayer->pev->angles, g_vecZero, m_iExplodeModelIndex, 1, 0, 5, true, 16, 255, kRenderTransAdd, NULL, false, 0, 0, 255, 0);
	UTIL_TempModel(m_pPlayer->pev->origin, m_pPlayer->pev->angles, g_vecZero, m_iExplodeEffectModelIndex, 6, 0, 255, 0, 0, 255, kRenderTransAdd, NULL, false, 0, 0, 255, 0);

	EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "weapons/cannonexplo.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
#endif
}

duration_t CCannonEX::GetDragonRemovalTime(void)
{
	return 6.0s;
}

float CCannonEX::GetDamageAmount(void)
{
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		return 3550;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		return 20850;
#endif
	return 410;
}

float CCannonEX::GetExplodeDamage(void)
{
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		return 2150;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		return 19950;
#endif
	return 410;
}

void CCannonEX::Holster(int skiplocal)
{
	RemoveDragon();
	StopIdleSound();

	CBasePlayerWeapon::Holster();
}


void CCannonEX::ItemPostFrame(void)
{
	if (m_flTimeCheckState != invalid_time_point && gpGlobals->time >= m_flTimeCheckState)
		CheckWeaponState();

	CBasePlayerWeapon::ItemPostFrame();
}


void CCannonEX::CheckWeaponState(void)
{
	const char* modelName;

	switch (m_iWeaponState2)
	{
	case 1:
		modelName = "models/p_cannonexb.mdl";

		m_pPlayer->pev->weaponmodel = MAKE_STRING(modelName);

		m_bDragonTransforming = false;
		m_iWeaponState = 1;
		m_iWeaponState2 = 4;
		m_flTimeCheckState = gpGlobals->time + m_flDragonFireTime;

		Explode();
		return;

	case 2:
		SendWeaponAnim(CANNONEX_DRAGON_RELOAD2, UseDecrement() != FALSE);

		m_iWeaponState2 = 3;
		m_flTimeCheckState = gpGlobals->time + m_flReloadTime;
		return;

	case 3:
		modelName ="models/p_cannonex.mdl";

		m_pPlayer->pev->weaponmodel = MAKE_STRING(modelName);

		m_bDragonReloading = false;
		m_iWeaponState = 0;

		ResetState();
		return;

	case 4:
		RemoveDragon();

		m_iWeaponState = 3;

		ResetState();
		return;

	default:
		return;
	}
}

void CCannonEX::ResetState(void)
{
	m_flTimeCheckState = invalid_time_point;
	m_iWeaponState2 = 0;
}

void CCannonEX::RemoveDragon(void)
{
#ifndef CLIENT_DLL
	if (m_pDragon && m_pDragon->m_iEffectState != EFFECT_FADEOUT)
	{
		m_pDragon->RemoveDragon();
	}
#endif
}


void CCannonEX::StopIdleSound(void)
{
#ifndef CLIENT_DLL
	MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->pev);
	WRITE_BYTE(79);
	WRITE_SHORT(m_pPlayer->entindex());
	WRITE_SHORT(CHAN_AUTO);
	WRITE_STRING("weapons/cannonex_Dtransform.wav");
	MESSAGE_END();

	MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->pev);
	WRITE_BYTE(79);
	WRITE_SHORT(m_pPlayer->entindex());
	WRITE_SHORT(CHAN_AUTO);
	WRITE_STRING("weapons/cannonex_D_reload2.wav");
	MESSAGE_END();
#endif
}


void CCannonEX::UpdateOnRemove(void)
{
#ifndef CLIENT_DLL
	if (m_pDragon)
	{
		if (m_pDragon->m_pWeapon)
			m_pDragon->m_pWeapon = NULL;

		RemoveDragon();
	}

	CBaseEntity::UpdateOnRemove();
#endif
}


void CCannonEX::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_2DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 10.0s;

	if (m_iWeaponState == 0)
	{
		SendWeaponAnim(CANNONEX_IDLE, UseDecrement() != FALSE);
	}
	else
	{
		SendWeaponAnim(CANNONEX_IDLEB, UseDecrement() != FALSE);
	}
}

}
