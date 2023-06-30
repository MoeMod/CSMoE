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
#include "wpn_ozwpnset1.h"

#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#include "gamemode/mod_zb1.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

LINK_ENTITY_TO_CLASS(weapon_ignitemg, CIgniteMG)

void CIgniteMG::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_ignitemg");

	Precache();
	m_iId = WEAPON_IGNITEMG;
	SET_MODEL(ENT(pev), "models/w_ignitemg.mdl");
	m_tNextPlayEvent = invalid_time_point;
	m_iDefaultAmmo = 150;
	m_iDefaultAmmo2 = 0;
	m_flAccuracy = 0.2;
	m_iShotsFired = 0;
	m_flTimeNextCharge = m_flAttackLoop = invalid_time_point;
	FallInit();
}

void CIgniteMG::Precache(void)
{
	PRECACHE_MODEL("models/v_ignitemg.mdl");
	PRECACHE_MODEL("models/w_ignitemg.mdl");
	PRECACHE_MODEL("models/p_ignitemg.mdl");

	PRECACHE_SOUND("weapons/ignitemg-1.wav");

	m_iHitIndex = PRECACHE_MODEL("sprites/ef_ignitemg_hit.spr");

	PRECACHE_MODEL("sprites/muzzleflash318.spr");
	PRECACHE_MODEL("sprites/muzzleflash319.spr");
	PRECACHE_MODEL("sprites/muzzleflash320.spr");
	PRECACHE_MODEL("sprites/muzzleflash321.spr");
	PRECACHE_MODEL("sprites/muzzleflash322.spr");
	PRECACHE_MODEL("sprites/muzzleflash323.spr");

	m_iBeamIndex = PRECACHE_MODEL("sprites/ef_gungnir_lightline1ex.spr");


	m_iShell = PRECACHE_MODEL("models/rshell.mdl");
	m_usFire = PRECACHE_EVENT(1, "events/ignitemg.sc");
}

int CIgniteMG::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "556NatoBox";
	p->iMaxAmmo1 = MAX_AMMO_556NATOBOX;
	p->pszAmmo2 = "ignitemg_b";
	p->iMaxAmmo2 = 25;
	p->iMaxClip = 150;
	p->iSlot = 0;
	p->iPosition = 4;
	p->iId = m_iId = WEAPON_IGNITEMG;
	p->iFlags = 0;
	p->iWeight = M249_WEIGHT;

	return 1;
}

int CIgniteMG::ExtractAmmo(CBasePlayerWeapon* pWeapon)
{
	if (m_iDefaultAmmo2)
	{
		if (m_pPlayer)
			m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = m_iDefaultAmmo2;
		m_iDefaultAmmo2 = 0;
	}
	return CBasePlayerWeapon::ExtractAmmo(pWeapon);
}

BOOL CIgniteMG::Deploy(void)
{
	ResetChargeTimer();
	ChargeAmmo();
	m_flNextReload = invalid_time_point;
	m_flAccuracy = 0.2;
	m_iShotsFired = 0;

	m_flAttackLoop = invalid_time_point;

	return DefaultDeploy("models/v_ignitemg.mdl", "models/p_ignitemg.mdl", ANIM_DRAW, "m134", UseDecrement() != FALSE);
}

void CIgniteMG::PrimaryAttack(void)
{
	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		IgniteMGFire(0.045 + 0.5 * m_flAccuracy, 0.12s, FALSE);
	else if (m_pPlayer->pev->velocity.Length2D() > 140)
		IgniteMGFire(0.045 + 0.095 * m_flAccuracy, 0.12s, FALSE);
	else
		IgniteMGFire(0.0275 * m_flAccuracy, 0.12s, FALSE);
	
}

void CIgniteMG::IgniteMGFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
{
	m_bDelayFire = true;
	m_iShotsFired++;
	m_flAccuracy = ((m_iShotsFired * m_iShotsFired * m_iShotsFired) / 175.0) + 0.35;

	if (m_flAccuracy > 0.9)
		m_flAccuracy = 0.9;

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

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 2, BULLET_PLAYER_556MM, GetDamage(), 0.97, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);

	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), FALSE, FALSE);
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5s;

	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		KickBack(1.8, 0.65, 0.45, 0.125, 5, 3.5, 8);
	else if (m_pPlayer->pev->velocity.Length2D() > 0)
		KickBack(1, 0.4, 0.3, 0.06, 4, 3, 8);
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		KickBack(0.65, 0.125, 0.25, 0.025, 3.5, 2.5, 9);
	else
		KickBack(0.8, 0.2, 0.25, 0.038, 3.75, 3, 10);
}

void CIgniteMG::Reload(void)
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		return;

	if (DefaultReload(GetReloadClip(), ANIM_RELOAD, 1.1s))
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif

		m_flNextReload = gpGlobals->time + 1.1s;
		m_flAccuracy = 0.2;
		m_bDelayFire = false;
		m_iShotsFired = 0;
	}
}

void CIgniteMG::Holster(int skiplocal)
{
	m_flNextPrimaryAttack = 0s;
	ResetChargeTimer();
	return CBasePlayerWeapon::Holster(skiplocal);
}

void CIgniteMG::ResetChargeTimer(void)
{
	m_flTimeNextCharge = gpGlobals->time + 1.0s;
}

void CIgniteMG::ChargeAmmo()
{
	if (m_flTimeNextCharge != invalid_time_point && m_flTimeNextCharge <= gpGlobals->time)
	{
		m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] += int((gpGlobals->time - m_flTimeNextCharge) / 1.0s + 1.0);

		if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] >= 25)
			m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 25;


		m_flTimeNextCharge = gpGlobals->time + 1.0s;

		if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] == 25)
			m_flTimeNextCharge = invalid_time_point;
	}
}

void CIgniteMG::CreateEffect()
{
#ifndef CLIENT_DLL
	for (size_t i = 0; i < 3; ++i)
	{
		CBeam* pBeam = CBeam::BeamCreate("sprites/ef_gungnir_lightline1ex.spr", 30);
		pBeam->SetColor(0, 100, 255);
		pBeam->SetScrollRate(30);
		pBeam->SetBrightness(0);
		pBeam->SetNoise(30);
		pev->effects |= EF_NODRAW;

		m_pBeamList[i] = pBeam;
	}
#endif
}

void CIgniteMG::ClearEffect()
{
#ifndef CLIENT_DLL
	for (CBeam* pBeam : m_pBeamList)
	{
		if (pBeam)
		{
			pBeam->SetBrightness(0);
			pev->effects |= EF_NODRAW; // 0x80
		}
	}
	m_pEnemyList.clear();
#endif
}


void CIgniteMG::DestroyEffect()
{
#ifndef CLIENT_DLL
	for (CBeam* p : m_pBeamList)
	{
		if (p)
			UTIL_Remove(p);
	}
	m_pBeamList[0] = NULL;
	m_pBeamList[1] = NULL;
	m_pBeamList[2] = NULL;
#endif
}

void CIgniteMG::SecondaryAttack_FindTargets()
{
	const float flRadius = 320;
#ifndef CLIENT_DLL
	m_pEnemyList.clear();

	CBaseEntity* pEntity = NULL;
	while ((pEntity = UTIL_FindEntityInSphere(pEntity, m_pPlayer->pev->origin, flRadius)) != nullptr)
	{
		if (SecondaryAttack_CheckTargetAvailable(pEntity, m_pPlayer->pev->v_angle))
		{
			EHANDLE eh;
			eh.Set(pEntity->edict());
			m_pEnemyList.push_back(eh);
		}
	}
#endif
}

bool CIgniteMG::SecondaryAttack_CheckTargetAvailable(CBaseEntity* a2, Vector vecAngleDirection)
{
	const float flRadius = 255;

	if (!a2->IsPlayer() && a2->Classify() != CLASS_PLAYER_ALLY)
		return false;

	if (!a2->IsAlive())
		return false;

#ifndef CLIENT_DLL
	if (g_pGameRules->PlayerRelationship(m_pPlayer, a2) == GR_TEAMMATE)
		return false;
#endif

	if (a2->pev == m_pPlayer->pev)
		return false;

	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecDelta = (a2->Center() - vecSrc).Normalize();

	UTIL_MakeVectors(vecAngleDirection);

	if (DotProduct(gpGlobals->v_forward, vecDelta) < 0.5)
		return false;

	TraceResult tr;
	UTIL_TraceLine(m_pPlayer->pev->origin, a2->pev->origin, missile, m_pPlayer->edict(), &tr);

	if (tr.flFraction < 1.0)
	{
		if (!tr.pHit)
			return false;

		if (tr.pHit != a2->edict())
			return false;
	}

	return true;
}


void CIgniteMG::SecondaryAttack_InstantDamage()
{
#ifndef CLIENT_DLL
	if (m_pEnemyList.empty())	//m_pEnemyList
		return;

	size_t v8 = 0;
	for (CBaseEntity* pEntity : m_pEnemyList)
	{
		if (v8 >= 3)
			break;
		if (!pEntity)
			continue;

		pEntity->m_LastHitGroup = HITGROUP_CHEST;

		pEntity->TakeDamage(pev, m_pPlayer->pev, GetDamageB(), DMG_BULLET);

		if (v8 < 3)
		{

			CBeam* pBeam = m_pBeamList[v8];
			if (pBeam)
			{
				pBeam->SetType(BEAM_ENTS);
				pBeam->SetStartEntity(ENTINDEX(m_pPlayer->edict()));
				pBeam->SetEndEntity(ENTINDEX(pEntity->edict()));
				pBeam->SetStartAttachment(2);
				pBeam->SetEndAttachment(0);
				pBeam->RelinkBeam();
				pBeam->SetBrightness(230);
				pBeam->pev->effects &= ~EF_NODRAW;
			}

			UTIL_TempModel(Vector(0, 0, 0), g_vecZero, g_vecZero, m_iHitIndex, 30, 0, 255, 0, 0, 210, kRenderTransAdd, pEntity, false, 0, 6, 16, 0);

			++v8;
		}
	}
#endif
}

void CIgniteMG::CreateBeamEffect()
{
	pev->nextthink = gpGlobals->time + 0.1s;

	int iStoredAmmo = m_iWeaponState;

	if (m_iWeaponState)
	{
		if (!m_fInReload)
		{
			ClearEffect();
			pev->nextthink = gpGlobals->time;
			SetThink(nullptr);
			return;
		}
		else
		{
			/*
			if (m_flTimeNextCharge2 < gpGlobals->time)
			{
				m_flTimeNextCharge2 = gpGlobals->time + 0.5s;
				m_iWeaponState--;
				SetChargeSkin();
			}

			PrimaryAttack_FindTargets();
			PrimaryAttack_InstantDamage();*/
		}
	}

	if (!m_iWeaponState)
	{
		//remove beam
		pev->nextthink = gpGlobals->time;
		if (iStoredAmmo != m_iWeaponState)
		{
			int flags = 0;
#ifdef CLIENT_WEAPONS
			flags = FEV_NOTHOST;
#else
			flags = 0;
#endif

			PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, 0, 0, 0, 0, 0, TRUE);
		}
		ClearEffect();
		SetThink(nullptr);
	}
}

void CIgniteMG::ItemPostFrame(void)
{
	ChargeAmmo();

	if (m_fInReload && m_flNextReload != invalid_time_point && m_flNextReload < gpGlobals->time)
	{
		// complete the reload.
		int j = Q_min(Q_min(GetMaxClip() - m_iClip, GetReloadClip()), m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]);
#ifndef CLIENT_DLL
		j = m_pPlayer->m_pModStrategy->AdjustDefaultReloadClip(this, j);
#endif

		// Add them to the clip
		m_iClip += j;
		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] -= j;
#ifndef CLIENT_DLL
		m_pPlayer->TabulateAmmo();
#endif
		m_fInReload = FALSE;
		m_flNextReload = invalid_time_point;
	}

	if (m_fInReload)
		return;

	return CBasePlayerWeapon::ItemPostFrame();
}

void CIgniteMG::SecondaryAttack()
{
	if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] <= 0)
	{
#ifndef CLIENT_DLL
		if (TheBots)
			TheBots->OnEvent(EVENT_WEAPON_FIRED_ON_EMPTY, m_pPlayer, NULL);
#endif

		if (m_flAttackLoop != invalid_time_point)
		{
			SendWeaponAnim(ANIM_SHOOT2_END, UseDecrement() != FALSE);

			m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.7s;
			m_flAttackLoop = invalid_time_point;
		}

		return;
	}

	ResetChargeTimer();

	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	if (m_flAttackLoop == invalid_time_point)
	{
		SendWeaponAnim(ANIM_SHOOT2_START, UseDecrement() != FALSE);

		m_flNextPrimaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.37s;
		m_flAttackLoop = gpGlobals->time + 0.37s;

		EMIT_SOUND_DYN(this->edict(), CHAN_WEAPON, "weapons/ignitemg_shoot2_start.wav", 0.8, ATTN_NORM, 0, PITCH_NORM);
		return;
	}

	if (m_flAttackLoop > gpGlobals->time)
		return;

	m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]--;

	Attack();

#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
	SendWeaponAnim(ANIM_SHOOT2_LOOP, UseDecrement() != FALSE);

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 999s;
	m_flTimeWeaponIdle = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.25s;
}

void CIgniteMG::Attack()
{

}

void CIgniteMG::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	
	if (m_flAttackLoop != invalid_time_point)
	{
		SendWeaponAnim(ANIM_SHOOT2_END, UseDecrement() != FALSE);

		m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.7s;
		m_flAttackLoop = invalid_time_point;
	}
	else
	{
		SendWeaponAnim(ANIM_IDLE1, UseDecrement() != FALSE);

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
	}
		
}

float CIgniteMG::GetDamage() const
{
	float flDamage = 36.0f;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flDamage = 180.0f;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 400.0f;
#endif
	return flDamage;
}

float CIgniteMG::GetDamageB() const
{
	float flDamage = 36.0f;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flDamage = 180.0f;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 400.0f;
#endif
	return flDamage;
}

KnockbackData CIgniteMG::GetKnockBackData()
{
	return { 350.0f, 250.0f, 300.0f, 100.0f, 0.6f };
}

float CIgniteMG::GetMaxSpeed(void)
{
	return 210;
}

}
