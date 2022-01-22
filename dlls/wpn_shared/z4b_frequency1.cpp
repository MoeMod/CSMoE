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
#include "z4b_frequency1.h"

#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

LINK_ENTITY_TO_CLASS(z4b_frequency1, CFrequency1)

void CFrequency1::Spawn(void)
{
	pev->classname = MAKE_STRING("z4b_frequency1");

	Precache();
	m_iId = WEAPON_DEAGLE;
	SET_MODEL(ENT(pev), "models/z4b/pw_mccree.mdl");

	m_iDefaultAmmo = FREQUENCY1_MAX_CLIP;
	m_flAccuracy = 0.9;
	m_iDefaultAmmo2 = 0;
	m_flTimeNextCharge = m_tAimTime = m_flTimeHostler = invalid_time_point;
	m_iUsingSkill = 0;
	m_iHolsterAmmo = 0;
	m_iIndex = 1;
	m_fMaxSpeed = 250;

	FallInit();
}

void CFrequency1::Precache(void)
{
	PRECACHE_MODEL("models/z4b/pw_mccree.mdl");
#ifdef ENABLE_SHIELD
	PRECACHE_MODEL("models/shield/v_shield_deagle.mdl");
#endif
	PRECACHE_MODEL("models/z4b/v_mccree.mdl");

	PRECACHE_SOUND("weapons/skull1-1.wav");
	PRECACHE_SOUND("weapons/skull1-2.wav");
	PRECACHE_SOUND("z4/z4_mccree_skill.wav");

	m_iShell = PRECACHE_MODEL("models/pshell.mdl");
	m_iSprBeam = PRECACHE_MODEL("sprites/laserbeam.spr");
	m_usFireFrequency1 = PRECACHE_EVENT(1, "events/frequency1.sc");
}

int CFrequency1::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "50AE";
	p->iMaxAmmo1 = MAX_AMMO_50AE;
	p->pszAmmo2 = "SVDEXGrenade";
	p->iMaxAmmo2 = -1;
	p->iMaxClip = FREQUENCY1_MAX_CLIP;
	p->iSlot = 1;
	p->iPosition = 1;
	p->iId = m_iId = WEAPON_DEAGLE;
	p->iFlags = 0;
	p->iWeight = DEAGLE_WEIGHT;

	return 1;
}

int CFrequency1::ExtractAmmo(CBasePlayerWeapon* pWeapon)
{
	if (m_iDefaultAmmo2)
	{
		m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = m_iDefaultAmmo2;
		m_iDefaultAmmo2 = 0;
	}
	return CBasePlayerWeapon::ExtractAmmo(pWeapon);
}

BOOL CFrequency1::Deploy(void)
{
	m_flAccuracy = 0.9;
	m_pPlayer->m_bShieldDrawn = false;
	m_fMaxSpeed = 250;
	m_tAimTime = invalid_time_point;
	m_iIndex = 1;
	m_iUsingSkill = 0;

	if (m_flTimeHostler != invalid_time_point)
	{
		m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] += (gpGlobals->time - m_flTimeHostler) / 0.3s;
	}
	if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] >= 100)
		m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 100;

	return DefaultDeploy("models/z4b/v_mccree.mdl", "models/z4b/pw_mccree.mdl", ANIM_DRAW, "onehanded", UseDecrement() != FALSE);
}


void CFrequency1::Holster(int skiplocal)
{
	SetThink(nullptr);
	m_iHolsterAmmo = m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType];
	m_flTimeHostler = gpGlobals->time;
	m_iUsingSkill = 0;
	m_iIndex = 1;
	m_tAimTime = invalid_time_point;
	return CBasePlayerWeapon::Holster(skiplocal);
}

void CFrequency1::PrimaryAttack(void)
{
	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		Frequency1Fire((1.5) * (1 - m_flAccuracy), 0.5s, FALSE);
	else if (m_pPlayer->pev->velocity.Length2D() > 0)
		Frequency1Fire((0.25) * (1 - m_flAccuracy), 0.5s, FALSE);
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		Frequency1Fire((0.115) * (1 - m_flAccuracy), 0.5s, FALSE);
	else
		Frequency1Fire((0.13) * (1 - m_flAccuracy), 0.5s, FALSE);
}

void CFrequency1::SecondaryAttack(void)
{
	if (CanUseSkill() && m_iClip == 6)
	{		
		SendWeaponAnim(ANIM_HIGHNOON_START, UseDecrement() != FALSE);
		m_iUsingSkill = 1;
		m_pPlayer->ResetMaxSpeed();

		EMIT_SOUND_DYN(pev->owner, CHAN_WEAPON, "z4/z4_mccree_skill.wav", VOL_NORM, ATTN_NORM, 0, 94);

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5s;
		m_tAimTime = gpGlobals->time + 0.33s;
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 4.0s;

		return;
	}

#ifndef CLIENT_DLL
	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		Frequency1FireSP((1.5) * (1 - m_flAccuracy), 0.17s, FALSE);
	else if (m_pPlayer->pev->velocity.Length2D() > 0)
		Frequency1FireSP((0.25) * (1 - m_flAccuracy), 0.17s, FALSE);
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		Frequency1FireSP((0.115) * (1 - m_flAccuracy), 0.17s, FALSE);
	else
		Frequency1FireSP((0.13) * (1 - m_flAccuracy), 0.17s, FALSE);
#endif
}


void CFrequency1::Frequency1FireSP(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
{
	flCycleTime -= 0.075s;
	m_iShotsFired++;

	if (m_flLastFire != invalid_time_point)
	{
		m_flAccuracy -= (0.4 - ((gpGlobals->time - m_flLastFire) / 1s)) * 0.35;

		if (m_flAccuracy > 0.9)
			m_flAccuracy = 0.9;
		else if (m_flAccuracy < 0.0)
			m_flAccuracy = 0.0;
	}

	m_flLastFire = gpGlobals->time;

	if (m_iClip <= 0)
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2s;
		}

		return;
	}

	m_iClip--;
	m_pPlayer->pev->effects |= EF_MUZZLEFLASH;

#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	m_pPlayer->m_iWeaponVolume = BIG_EXPLOSION_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 4096, 10, BULLET_PLAYER_50AE, GetDamage2(), 0.89, m_pPlayer->pev, TRUE, m_pPlayer->random_seed);

	PLAYBACK_EVENT_FULL(0, m_pPlayer->edict(), m_usFireFrequency1, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), FALSE, FALSE);

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.8s;

	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		KickBack(1.8, 0.75, 0.35, 0.075, 6.5, 3.5, 6);
	else if (m_pPlayer->pev->velocity.Length2D() > 0)
		KickBack(2.0, 0.9, 0.5, 0.1, 7.0, 5.0, 5);
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		KickBack(1.5, 0.7, 0.3, 0.05, 6.0, 3.0, 7);
	else
		KickBack(3.0, 2.0, 1.0, 0.7, 8.0, 12.0, 4);

#if 0
	Vector vecEnd = vecSrc + gpGlobals->v_forward * 2048;
	Effect_Beam(vecSrc, vecEnd, 7, 50, 75, 150, 150, 4, 998);
#endif
}

void CFrequency1::Frequency1Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
{
	flCycleTime -= 0.075s;
	m_iShotsFired++;

	if (m_iShotsFired > 1)
		return;

	if (m_flLastFire != invalid_time_point)
	{
		m_flAccuracy -= (0.4 - ((gpGlobals->time - m_flLastFire) / 1s)) * 0.35;

		if (m_flAccuracy > 0.9)
			m_flAccuracy = 0.9;
		else if (m_flAccuracy < 0.55)
			m_flAccuracy = 0.55;
	}

	m_flLastFire = gpGlobals->time;

	if (m_iClip <= 0)
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2s;
		}

		return;
	}

	m_iClip--;
	m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
	SetPlayerShieldAnim();
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	m_pPlayer->m_iWeaponVolume = BIG_EXPLOSION_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 4096, 10, BULLET_PLAYER_50AE, GetDamage1(), 0.89, m_pPlayer->pev, TRUE, m_pPlayer->random_seed);

	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFireFrequency1, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), FALSE , TRUE);

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.8s;
	m_pPlayer->pev->punchangle.x -= 2;
#if 0
	Vector vecEnd = vecSrc + gpGlobals->v_forward * 2048;
	Effect_Beam(vecSrc, vecEnd, 7, 50, 75, 150, 150, 4, 998);
#endif

}

#ifndef CLIENT_DLL
void CFrequency1::Effect_Beam(Vector vecStart, Vector vecEnd, int iLife, int iBrightness, int r, int g, int b, int iWidth, int iScollSpeed)
{
	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
	WRITE_BYTE(TE_BEAMPOINTS);
	WRITE_COORD(vecStart.x);
	WRITE_COORD(vecStart.y);
	WRITE_COORD(vecStart.z);
	WRITE_COORD(vecEnd.x);
	WRITE_COORD(vecEnd.y);
	WRITE_COORD(vecEnd.z); // coord coord coord (end position) 
	WRITE_SHORT(m_iSprBeam); // short (sprite index) 
	WRITE_BYTE(0); // byte (starting frame) 
	WRITE_BYTE(1); // byte (frame rate in 0.1's) 
	WRITE_BYTE(iLife); // byte (life in 0.1's) 
	WRITE_BYTE(iWidth); // byte (line width in 0.1's) 
	WRITE_BYTE(0); // byte (noise amplitude in 0.01's) 
	WRITE_BYTE(r); // byte,byte,byte (color)
	WRITE_BYTE(g);
	WRITE_BYTE(b);
	WRITE_BYTE(iBrightness); // byte (brightness)
	WRITE_BYTE(iScollSpeed); // byte (scroll speed in 0.1's)
	MESSAGE_END();
}
#endif

void CFrequency1::Reload(void)
{
	if (m_pPlayer->ammo_50ae <= 0)
		return;

	m_tAimTime = invalid_time_point;
	m_iUsingSkill = 0;
	m_iIndex = 1;
	m_tAimTime = invalid_time_point;

	if (DefaultReload(FREQUENCY1_MAX_CLIP, ANIM_RELOAD, FREQUENCY1_RELOAD_TIME))
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
		m_flAccuracy = 0.9;
	}
}

bool CFrequency1::CanUseSkill()
{
	return m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] >= 100;
}

#ifndef CLIENT_DLL
void CFrequency1::CalcIsTarget()
{
	for (int iIndex = 1; iIndex <= gpGlobals->maxClients; ++iIndex)
	{
		CBaseEntity* entity = UTIL_PlayerByIndex(iIndex);
		m_iIsTarget[iIndex] = 0;
		if (!entity)
			continue;

		CBasePlayer* TargetPlayer = static_cast<CBasePlayer*>(entity);

		if (FNullEnt(TargetPlayer->pev))
			continue;

		if (TargetPlayer == m_pPlayer)
			continue;

		if (TargetPlayer->IsDormant())
			continue;

		if (!TargetPlayer->IsAlive())
			continue;

		if (g_pGameRules->PlayerRelationship(m_pPlayer, TargetPlayer) == GR_TEAMMATE)
			continue;

		if (!is_target_avaliable(iIndex))
			continue;

		m_iIsTarget[iIndex] = 1;
	}
}


bool CFrequency1::is_target_avaliable(int iIndex)
{
	CBasePlayer* pPlayer = static_cast<CBasePlayer*>(UTIL_PlayerByIndex(iIndex));

	if (!pPlayer)
		return false;

	TraceResult tr;
	UTIL_TraceLine(m_pPlayer->pev->origin, pPlayer->pev->origin, ignore_monsters, m_pPlayer->edict(), &tr);
	if (tr.flFraction < 1.0)
		return false;

	Vector vecDelta = (pPlayer->pev->origin - m_pPlayer->pev->origin).Normalize();
	UTIL_MakeVectors(m_pPlayer->pev->v_angle);
	if (DotProduct(gpGlobals->v_forward, vecDelta) < 0)
		return false;

	return true;
}



void CFrequency1::Frequency1HighNoonFire(int iIndex)
{
	m_iShotsFired++;
	duration_t flCycleTime = 0.5s;
	if (m_flLastFire != invalid_time_point)
	{
		m_flAccuracy -= (0.4 - ((gpGlobals->time - m_flLastFire) / 1s)) * 0.35;

		if (m_flAccuracy > 0.9)
			m_flAccuracy = 0.9;
		else if (m_flAccuracy < 0.0)
			m_flAccuracy = 0.0;
	}

	m_flLastFire = gpGlobals->time;

	if (m_iClip <= 0)
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2s;
		}

		return;
	}

	CBaseEntity* entity = UTIL_PlayerByIndex(iIndex);

	if (!entity)
		return;

	CBasePlayer* TargetPlayer = static_cast<CBasePlayer*>(entity);

	m_iClip--;

	m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] -= 17;
	if(m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] <= 0)
		m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 0;

	m_pPlayer->pev->effects |= EF_MUZZLEFLASH;

	Vector vecEnd, vecDir;

	float flDistance = 4096.0;

	Vector vecSrc = m_pPlayer->GetGunPosition();

	vecDir = (TargetPlayer->pev->origin - vecSrc).Normalize();

	vecEnd = vecSrc + vecDir * flDistance;

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	m_pPlayer->m_iWeaponVolume = BIG_EXPLOSION_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	Effect_Beam(vecSrc, vecEnd, 7, 50, 75, 150, 150, 4, 998);

#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	int iPenetration = FREQUENCY1_PENETRATION;
	int iPenetrationPower = 30;
	float flPenetrationDistance = 1000;
	int iCurrentDamage = GetDamage3();
	float flCurrentDistance;

	TraceResult tr;

	bool bHitMetal = false;

	float flDamageModifier = 0.5;

	while (iPenetration != 0) {
		ClearMultiDamage();
		UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(pev), &tr);

		if (TheBots != NULL && tr.flFraction != 1.0f) {
			TheBots->OnEvent(EVENT_BULLET_IMPACT, this, (CBaseEntity*)&tr.vecEndPos);
		}

		char cTextureType = UTIL_TextureHit(&tr, vecSrc, vecEnd);
		bool bSparks = false;

		switch (cTextureType) {
		case CHAR_TEX_METAL:
			bHitMetal = true;
			bSparks = true;

			iPenetrationPower *= 0.15;
			flDamageModifier = 0.2;
			break;
		case CHAR_TEX_CONCRETE:
			iPenetrationPower *= 0.25;
			break;
		case CHAR_TEX_GRATE:
			bHitMetal = true;
			bSparks = true;

			iPenetrationPower *= 0.5;
			flDamageModifier = 0.4;
			break;
		case CHAR_TEX_VENT:
			bHitMetal = true;
			bSparks = true;

			iPenetrationPower *= 0.5;
			flDamageModifier = 0.45;
			break;
		case CHAR_TEX_TILE:
			iPenetrationPower *= 0.65;
			flDamageModifier = 0.3;
			break;
		case CHAR_TEX_COMPUTER:
			bHitMetal = true;
			bSparks = true;

			iPenetrationPower *= 0.4;
			flDamageModifier = 0.45;
			break;
		case CHAR_TEX_WOOD:
			flDamageModifier = 0.6;
			break;
		default:
			break;
		}
		if (tr.flFraction != 1.0f) {
			CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);
			iPenetration--;

			flCurrentDistance = tr.flFraction * flDistance;
			iCurrentDamage *= pow(FREQUENCY1_RANGE_MODIFER, flCurrentDistance / 500);

			if (flCurrentDistance > flPenetrationDistance) {
				iPenetration = 0;
			}

			if (tr.iHitgroup == HITGROUP_SHIELD) {
				EMIT_SOUND(pEntity->edict(), CHAN_VOICE,
					(RANDOM_LONG(0, 1) == 1) ? "weapons/ric_metal-1.wav" : "weapons/ric_metal-2.wav", VOL_NORM,
					ATTN_NORM);
				UTIL_Sparks(tr.vecEndPos);

				pEntity->pev->punchangle.x = iCurrentDamage * RANDOM_FLOAT(-0.15, 0.15);
				pEntity->pev->punchangle.z = iCurrentDamage * RANDOM_FLOAT(-0.15, 0.15);

				if (pEntity->pev->punchangle.x < 4) {
					pEntity->pev->punchangle.x = -4;
				}

				if (pEntity->pev->punchangle.z < -5) {
					pEntity->pev->punchangle.z = -5;
				}
				else if (pEntity->pev->punchangle.z > 5) {
					pEntity->pev->punchangle.z = 5;
				}

				break;
			}

			float flDistanceModifier;
			if (VARS(tr.pHit)->solid != SOLID_BSP || !iPenetration) {
				iPenetrationPower = 42;
				flDamageModifier = 0.75;
				flDistanceModifier = 0.75;
			}
			else
				flDistanceModifier = 0.5;

			if (!Q_strcmp(STRING(pEntity->pev->classname), "zombiepile")) {
				EMIT_SOUND(pEntity->edict(), CHAN_VOICE,
					(RANDOM_LONG(0, 1) == 1) ? "weapons/ric_metal-1.wav" : "weapons/ric_metal-2.wav", VOL_NORM,
					ATTN_NORM);
				UTIL_Sparks(tr.vecEndPos);
				iPenetration = 0;
				flDistanceModifier = 0.0;
			}

			DecalGunshot(&tr, BULLET_PLAYER_50AE, (1 && RANDOM_LONG(0, 3)), pev, bHitMetal);

#ifdef TRACE_BULLETS
			Vector vecEndPos2 = tr.vecEndPos - (vecDir * 3);
			MESSAGE_BEGIN(MSG_ALL, SVC_TEMPENTITY);
			WRITE_BYTE(TE_LINE);
			WRITE_COORD(vecEndPos2.x);
			WRITE_COORD(vecEndPos2.y);
			WRITE_COORD(vecEndPos2.z);

			WRITE_COORD(tr.vecEndPos.x);
			WRITE_COORD(tr.vecEndPos.y);
			WRITE_COORD(tr.vecEndPos.z);
			WRITE_SHORT(300);
			WRITE_BYTE(0);
			WRITE_BYTE(0);
			WRITE_BYTE(255);
			MESSAGE_END();
#endif


			vecSrc = tr.vecEndPos + (vecDir * iPenetrationPower);
			flDistance = (flDistance - flCurrentDistance) * flDistanceModifier;
			vecEnd = vecSrc + (vecDir * flDistance);
			tr.iHitgroup = HITGROUP_HEAD;
			pEntity->TraceAttack(m_pPlayer->pev, iCurrentDamage, vecDir, &tr, (DMG_BULLET | DMG_NEVERGIB));
			iCurrentDamage *= flDamageModifier;
		}
		else
			iPenetration = 0;

		ApplyMultiDamage(pev, m_pPlayer->pev);
	}
#endif

	int flags;

	PLAYBACK_EVENT_FULL(0, m_pPlayer->edict(), m_usFireFrequency1, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), TRUE, FALSE);

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.8s;

	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		KickBack(1.8, 0.75, 0.35, 0.075, 6.5, 3.5, 6);
	else if (m_pPlayer->pev->velocity.Length2D() > 0)
		KickBack(2.0, 0.9, 0.5, 0.1, 7.0, 5.0, 5);
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		KickBack(1.5, 0.7, 0.3, 0.05, 6.0, 3.0, 7);
	else
		KickBack(3.0, 2.0, 1.0, 0.7, 8.0, 12.0, 4);

	
}
#endif
void CFrequency1::ItemPostFrame()
{
#ifndef CLIENT_DLL
	if (m_iUsingSkill)
	{
		if (m_tAimTime < gpGlobals->time)
		{
			if (m_iUsingSkill > 1)
			{
				if (m_iUsingSkill > 2)
				{
					if (m_iUsingSkill > 3)
					{
						SendWeaponAnim(ANIM_HIGHNOON_END, UseDecrement() != FALSE);
						m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5s;
						m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5s;
						m_iIndex = 0;
						m_iUsingSkill = 0;
						m_pPlayer->ResetMaxSpeed();
						m_tAimTime = invalid_time_point;
					}
					else
					{
						for (; m_iIndex <= gpGlobals->maxClients; ++m_iIndex)
						{
							if (!m_iClip)
							{
								m_iUsingSkill = 0;
								m_pPlayer->ResetMaxSpeed();
								m_tAimTime = invalid_time_point;
								m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5s;
								m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5s;
								break;
							}
							
							if (m_iIndex >= gpGlobals->maxClients)
							{
								if (m_iClip == 6)
								{
									m_tAimTime = gpGlobals->time + 0.5s;
									m_iIndex = 0;
									m_iUsingSkill = 4;
									break;
								}
								else
								{
									m_iUsingSkill = 0;
									m_pPlayer->ResetMaxSpeed();
									m_tAimTime = invalid_time_point;
									m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5s;
									m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5s;
									break;
								}
							}

							if (m_iIsTarget[m_iIndex])
							{
								Frequency1HighNoonFire(m_iIndex);
								m_tAimTime = gpGlobals->time + 0.17s;
								m_iIsTarget[m_iIndex] = 0;
								break;
							}					
						}
					}
				}
				else
				{
					CalcIsTarget();

					for (int i = 1; i <= gpGlobals->maxClients; ++i)
					{
						if (m_iIsTarget[i])
						{
							SendWeaponAnim(ANIM_HIGHNOON_STARTSHOOT, UseDecrement() != FALSE);
							break;
						}
					}
				
					m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 9999s;
					m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.4s;
					m_tAimTime = gpGlobals->time + 0.2s;
					m_iUsingSkill = 3;
				}
			}
			else
			{
				SendWeaponAnim(ANIM_HIGHNOON_LOOP, UseDecrement() != FALSE);
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 9999s;
				m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 3.0s;
				m_tAimTime = gpGlobals->time + 3.0s;
				m_iUsingSkill = 2;
			}
		}
	}

	if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] < 100)
	{
		if (m_flTimeNextCharge <= gpGlobals->time)
		{
			m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]++;
			m_flTimeNextCharge = gpGlobals->time + 0.3s;

			if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] == 100)
				EMIT_SOUND_DYN(pev->owner, CHAN_WEAPON, "weapons/skullt9_fx.wav", VOL_NORM, ATTN_NORM, 0, 94);
		}
	}
#endif

	return CBasePlayerWeapon::ItemPostFrame();
}


void CFrequency1::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	if (m_iUsingSkill)
		return;

	SendWeaponAnim(ANIM_IDLE, UseDecrement() != FALSE);
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
}

float CFrequency1::GetDamage1() const
{
	float flDamage = 56.0f;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flDamage = 280.0f;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 350.0f;
#endif
	return flDamage;
}

float CFrequency1::GetDamage2() const
{
	float flDamage = 40.0f;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flDamage = 190.0f;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 280.0f;
#endif
	return flDamage;
}

float CFrequency1::GetDamage3() const
{
	float flDamage = 70.0f;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flDamage = 480.0f;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 280.0f;
#endif
	return flDamage;
}

float CFrequency1::GetMaxSpeed(void)
{
	if (m_iUsingSkill)
		return 200.0;

	return 250.0;
}

}
