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
#include <vector>
#include <array>
#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#include "effects.h"
#endif
#include "wpn_revivegun.h"


#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

LINK_ENTITY_TO_CLASS(weapon_revivegun, CReviveGun)

void CReviveGun::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_revivegun");

	Precache();
	m_iId = WEAPON_AUG;
	SET_MODEL(ENT(pev), "models/w_revivegun.mdl");

	m_iDefaultAmmo = 60;
	m_iDefaultAmmo2 = 6;
	m_flAccuracy = 0.2;
	m_iShotsFired = 0;
	m_iThinkType = 0;
	m_iChargeShotsFired = 0;
	m_tNextPlayEvent = invalid_time_point;
	m_tBeamThink = m_tNextThink = invalid_time_point;

	FallInit();
}

void CReviveGun::Precache(void)
{
	PRECACHE_MODEL("models/v_revivegun.mdl");
	PRECACHE_MODEL("models/w_revivegun.mdl");
	PRECACHE_MODEL("models/p_revivegun.mdl");
	PRECACHE_MODEL("models/ef_revivegun_field.mdl");

	PRECACHE_MODEL("sprites/ef_revivegun_idle1.spr");
	PRECACHE_MODEL("sprites/ef_revivegun_idle1_left.spr");
	PRECACHE_MODEL("sprites/ef_revivegun_idle2.spr");
	PRECACHE_MODEL("sprites/ef_revivegun_idle2_left.spr");

	PRECACHE_MODEL("sprites/ef_revivegun_laser.spr");

	PRECACHE_MODEL("sprites/ef_revivegun_draw1_left.spr");
	PRECACHE_MODEL("sprites/ef_revivegun_draw1.spr");
	PRECACHE_MODEL("sprites/ef_revivegun_draw2_left.spr");
	PRECACHE_MODEL("sprites/ef_revivegun_draw2.spr");

	PRECACHE_MODEL("sprites/ef_revivegun_expA.spr");
	PRECACHE_MODEL("sprites/ef_revivegun_expB.spr");

	PRECACHE_MODEL("sprites/muzzleflash223.spr");
	PRECACHE_MODEL("sprites/muzzleflash224.spr");
	PRECACHE_MODEL("sprites/muzzleflash225.spr");
	PRECACHE_MODEL("sprites/muzzleflash226.spr");

	PRECACHE_SOUND("weapons/revivegun-1.wav");
	PRECACHE_SOUND("weapons/revivegun-2.wav");
	PRECACHE_SOUND("weapons/revivegun-1_exp.wav");

	PRECACHE_SOUND("weapons/revivegun_revive_in1.wav");
	PRECACHE_SOUND("weapons/revivegun_revive_out1.wav");

	PRECACHE_SOUND("weapons/revivegun_idle1.wav");
	PRECACHE_SOUND("weapons/revivegun_idle2.wav");

	m_usFireReviveGun = PRECACHE_EVENT(1, "events/revivegun.sc");
}

int CReviveGun::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "556NatoStar";
	p->iMaxAmmo1 = 150;
	p->pszAmmo2 = "ReviveAmmo";
	p->iMaxAmmo2 = 6;
	p->iMaxClip = REVIVEGUN_MAXCLIP;
	p->iSlot = 0;
	p->iPosition = 14;
	p->iId = m_iId = WEAPON_AUG;
	p->iFlags = 0;
	p->iWeight = AUG_WEIGHT;

	return 1;
}

BOOL CReviveGun::Deploy(void)
{
	m_flAccuracy = 0.2;
	m_iShotsFired = 0;

	m_iThinkType = 0;
	m_tBeamThink = m_tNextThink = invalid_time_point;
	m_tNextPlayEvent = gpGlobals->time + 1.0s;
	CreateEffect();



	if (m_flTimeHostler != invalid_time_point)
	{
		m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] += (gpGlobals->time - m_flTimeHostler) / 3.5s;
	}
	if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] >= 6)
		m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 6;

	if (DefaultDeploy("models/v_revivegun.mdl", "models/p_revivegun.mdl", IsSignal() ? REVIVEGUN_REVIVE_DRAW : REVIVEGUN_DRAW, "m249", UseDecrement() != FALSE, 0.1s))
	{
#ifndef CLIENT_DLL
		PLAYBACK_EVENT_FULL(FEV_HOSTONLY, m_pPlayer->edict(), PRECACHE_EVENT(1, "events/wpneffects.sc"), 0, g_vecZero, g_vecZero, 0.0, 0.0, 18, 1, IsSignal(), TRUE);
#endif
		return TRUE;
	}
}


int CReviveGun::ExtractAmmo(CBasePlayerWeapon* pWeapon)
{
	if (m_iDefaultAmmo2)
	{
		m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = m_iDefaultAmmo2;
		m_iDefaultAmmo2 = 0;
	}
	return CBasePlayerWeapon::ExtractAmmo(pWeapon);
}

void CReviveGun::SecondaryAttack(void)
{
	if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] <= 0)
		return;

	m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]--;
	
	if (!m_iThinkType)
	{
		SendWeaponAnim(IsSignal() ? REVIVEGUN_REVIVE_SHOOTB_START : REVIVEGUN_SHOOTB_START, UseDecrement() != FALSE);
		m_tNextThink = gpGlobals->time + 0.37s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.37s;
		m_iThinkType = 1;
	}
	else if (m_iThinkType == 2)
	{
#ifndef CLIENT_DLL
		PLAYBACK_EVENT_FULL(0, m_pPlayer->edict(), m_usFireReviveGun, 0, g_vecZero, g_vecZero, 0.0, 0.0, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), IsSignal(), TRUE);
#endif

		EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "weapons/revivegun-2.wav", VOL_NORM, ATTN_NORM, 0, 94);

		SecondaryAttack_FindTargets();
		SecondaryAttack_InstantDamage();

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.7s;
		m_tNextThink = gpGlobals->time + 1.7s;
	}

	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.84s;
}


bool CReviveGun::SecondaryAttack_CheckTargetAvailable(CBaseEntity* a2, Vector vecAngleDirection)
{
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

	if (DotProduct(gpGlobals->v_forward, vecDelta) < 0.7)
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

void CReviveGun::SecondaryAttack_FindTargets()
{
	m_pEnemyList.clear();
#ifndef CLIENT_DLL
	CBaseEntity* pEntity = NULL;
	while ((pEntity = UTIL_FindEntityInSphere(pEntity, m_pPlayer->pev->origin, SECNDARYATTACK_RADIUS)) != nullptr)
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


void CReviveGun::SecondaryAttack_InstantDamage()
{
	int iAttachment = 2;
	size_t iBeamTarget = 0;
	m_tBeamThink = gpGlobals->time + 0.4s;

	if (m_pEnemyList.empty())
	{		
		Vector vecSrc = m_pPlayer->GetGunPosition();
		UTIL_MakeVectors(m_pPlayer->pev->v_angle);
		TraceResult tr;
		Vector vecEnd = vecSrc + gpGlobals->v_forward * 4096;
		UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr);

		if (tr.flFraction != 1.0f)
			vecEnd = tr.vecEndPos;

#ifndef CLIENT_DLL
		int iTime;
		for (iTime = 0; iTime < 7; iTime++)
		{
			//Has Problem need fix
			CBeam* pBeam = m_pBeamList[iBeamTarget];
			if (pBeam)
			{
				pBeam->SetType(BEAM_ENTPOINT);
				pBeam->SetStartPos(vecEnd);
				pBeam->SetEndEntity(ENTINDEX(m_pPlayer->edict()));
				pBeam->SetStartAttachment(0);
				pBeam->SetEndAttachment(iAttachment);
				pBeam->SetBrightness(230);
				pBeam->RelinkBeam();
				pBeam->pev->effects &= ~EF_NODRAW;
			}
			++iBeamTarget;
			iAttachment++;
			if (iAttachment >= 6)
				iAttachment = 2;

		}
#endif
		if (tr.flFraction != 1.0f)
		{
			CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);
			if (!pEntity)
				return;
			Vector vecDirection = (pEntity->pev->origin - m_pPlayer->pev->origin).Normalize();
#ifndef CLIENT_DLL
			tr.iHitgroup = HITGROUP_CHEST;
			ClearMultiDamage();
			pEntity->TraceAttack(m_pPlayer->pev, 7 * GetSecondaryAttackDamage(), vecDirection, &tr, DMG_BULLET);
			ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);
#endif
		}
		return;
	}

	auto mod = MAXTARGET % m_pEnemyList.size();
	int iAveShoot = (MAXTARGET - mod) / m_pEnemyList.size();

	for (CBaseEntity* pEntity : m_pEnemyList)
	{
		if (m_pEnemyList.size() == 1)
		{
			if (iBeamTarget >= 7)
				break;
		}
		else
		{
			if (iBeamTarget >= MAXTARGET)
				break;
		}

		if (!pEntity)
			continue;
		Vector vecDirection = (pEntity->pev->origin - m_pPlayer->pev->origin).Normalize();

		TraceResult tr;
		UTIL_TraceLine(m_pPlayer->pev->origin, pEntity->pev->origin, missile, ENT(m_pPlayer->pev), &tr);

		if (m_pEnemyList.size() == 1)
		{
#ifndef CLIENT_DLL
			tr.iHitgroup = HITGROUP_CHEST;
			ClearMultiDamage();
			pEntity->TraceAttack(m_pPlayer->pev, 7 * GetSecondaryAttackDamage(), vecDirection, &tr, DMG_BULLET);
			ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);
#endif
			int iTime;
			for (iTime = 0; iTime < 7; iTime++)
			{
#ifndef CLIENT_DLL
				CBeam* pBeam = m_pBeamList[iBeamTarget];
				if (pBeam)
				{
					//pBeam->EntsInit(ENTINDEX(m_pPlayer->edict()), ENTINDEX(pEntity->edict()));
					pBeam->SetType(BEAM_ENTS);
					pBeam->SetStartEntity(ENTINDEX(m_pPlayer->edict()));
					pBeam->SetEndEntity(ENTINDEX(pEntity->edict()));
					pBeam->SetStartAttachment(iAttachment);
					pBeam->SetEndAttachment(0);
					pBeam->SetBrightness(230);
					pBeam->RelinkBeam();
					pBeam->pev->effects &= ~EF_NODRAW;

				}
#endif
				iBeamTarget++;
				iAttachment++;
				if (iAttachment >= 6)
					iAttachment = 2;

			}
		}
		else
		{
			int iAddShoot = 1;
			if (mod > 0)
				mod--;
			else
			{
				mod = 0;
				iAddShoot = 0;
			}

#ifndef CLIENT_DLL
			tr.iHitgroup = HITGROUP_CHEST;
			ClearMultiDamage();
			pEntity->TraceAttack(m_pPlayer->pev, iAveShoot * GetSecondaryAttackDamage() + iAddShoot * GetSecondaryAttackDamage(), vecDirection, &tr, DMG_BULLET);
			ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);
#endif
			int iTime;
			for (iTime = 0; iTime < iAveShoot + iAddShoot; iTime++)
			{
#ifndef CLIENT_DLL
				CBeam* pBeam = m_pBeamList[iBeamTarget];
				if (pBeam)
				{
					//pBeam->EntsInit(ENTINDEX(m_pPlayer->edict()), ENTINDEX(pEntity->edict()));
					pBeam->SetType(BEAM_ENTS);
					pBeam->SetStartEntity(ENTINDEX(m_pPlayer->edict()));
					pBeam->SetEndEntity(ENTINDEX(pEntity->edict()));
					pBeam->SetStartAttachment(iAttachment);
					pBeam->SetEndAttachment(0);
					pBeam->SetBrightness(230);
					pBeam->RelinkBeam();
					pBeam->pev->effects &= ~EF_NODRAW;

				}
#endif
				iBeamTarget++;
				iAttachment++;
				if (iAttachment >= 6)
					iAttachment = 2;

			}
		}

#ifndef CLIENT_DLL
		MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pEntity->pev->origin);
		WRITE_BYTE(TE_EXPLOSION);
		WRITE_COORD(pEntity->pev->origin.x);
		WRITE_COORD(pEntity->pev->origin.y);
		WRITE_COORD(pEntity->pev->origin.z);
		WRITE_SHORT(MODEL_INDEX("sprites/ef_revivegun_expB.spr"));
		WRITE_BYTE(5);
		WRITE_BYTE(30);
		WRITE_BYTE(TE_EXPLFLAG_NODLIGHTS | TE_EXPLFLAG_NOPARTICLES | TE_EXPLFLAG_NOSOUND);
		MESSAGE_END();
#endif
	}

}

void CReviveGun::CreateEffect()
{
#ifndef CLIENT_DLL
	for (size_t i = 0; i < MAXTARGET; ++i)
	{
		CBeam* pBeam = CBeam::BeamCreate("sprites/ef_revivegun_laser.spr", 125);
		pBeam->SetColor(255, 255, 255);
		pBeam->SetScrollRate(30);
		pBeam->SetNoise(12);
		pBeam->SetWidth(125);
		pBeam->pev->effects |= EF_NODRAW;

		m_pBeamList[i] = pBeam;
	}
#endif
}

void CReviveGun::DestroyEffect()
{
#ifndef CLIENT_DLL
	for (CBeam* p : m_pBeamList)
	{
		if (p)
			p->SUB_Remove();
	}
#endif
}

void CReviveGun::ClearEffect()
{
#ifndef CLIENT_DLL
	for (CBeam* pBeam : m_pBeamList)
	{
		if (pBeam)
		{
			pBeam->SetBrightness(0);
			pBeam->pev->effects |= EF_NODRAW; // 0x80
		}
	}
#endif
	m_pEnemyList.clear();
}

void CReviveGun::PrimaryAttack(void)
{
	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		ReviveGunFire(0.035 + (0.4) * m_flAccuracy, 0.3s, FALSE);
	else if (m_pPlayer->pev->velocity.Length2D() > 140)
		ReviveGunFire(0.035 + (0.07) * m_flAccuracy, 0.3s, FALSE);
	else
		ReviveGunFire((0.02) * m_flAccuracy, 0.3s, FALSE);

}

bool CReviveGun::IsSignal(void)
{
	return m_iChargeShotsFired > 20 ? true : false;
}

void CReviveGun::ReviveGunFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
{
	m_bDelayFire = true;
	m_iShotsFired++;
	m_flAccuracy = ((float)(m_iShotsFired * m_iShotsFired * m_iShotsFired) / 215.0) + 0.3;

	if (m_flAccuracy > 1)
		m_flAccuracy = 1;

	if (m_iClip <= 0)
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2s;
		}

		return;
	}

	m_iThinkType = 0;
	m_iChargeShotsFired++;
	m_iClip--;
	m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecDir;

#ifndef CLIENT_DLL
	vecDir = FireBullets(vecSrc, gpGlobals->v_forward, flSpread, 8192, 3, BULLET_PLAYER_762MM, GetDamage(), 0.96, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);
#else
	vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 3, BULLET_PLAYER_762MM, GetDamage(), 0.96, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);
#endif

	int flags;

#ifndef CLIENT_DLL
	PLAYBACK_EVENT_FULL(0, m_pPlayer->edict(), m_usFireReviveGun, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), IsSignal(), FALSE);
#endif

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.9s;

	if (m_pPlayer->pev->velocity.Length2D() > 0)
		KickBack(1.0, 0.45, 0.275, 0.05, 4.0, 2.5, 7);
	else if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		KickBack(1.25, 0.45, 0.22, 0.18, 5.5, 4.0, 5);
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		KickBack(0.575, 0.325, 0.2, 0.011, 3.25, 2.0, 8);
	else
		KickBack(0.625, 0.375, 0.25, 0.0125, 3.5, 2.25, 8);
}

void CReviveGun::ItemPostFrame()
{
	if (gpGlobals->time > m_tNextPlayEvent && m_tNextPlayEvent != invalid_time_point)
	{
#ifndef CLIENT_DLL
		PLAYBACK_EVENT_FULL(FEV_HOSTONLY, m_pPlayer->edict(), PRECACHE_EVENT(1, "events/wpneffects.sc"), 0, g_vecZero, g_vecZero, 0.0, 0.0, 18, 0, IsSignal(), TRUE);
		m_tNextPlayEvent = gpGlobals->time + (IsSignal() ? 1.57s : 1.5s);
#endif
		//m_tNextPlayEvent = gpGlobals->time + 10s;
	}

	if (m_tNextThink != invalid_time_point && m_tNextThink < gpGlobals->time)
	{
		switch (m_iThinkType)
		{
			case 1:
			{
#ifndef CLIENT_DLL
				PLAYBACK_EVENT_FULL(0, m_pPlayer->edict(), m_usFireReviveGun, 0, g_vecZero, g_vecZero, 0.0, 0.0, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), IsSignal(), TRUE);
#endif

				EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "weapons/revivegun-2.wav", VOL_NORM, ATTN_NORM, 0, 94);

				SecondaryAttack_FindTargets();
				SecondaryAttack_InstantDamage();

				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.7s;
				m_tNextThink = gpGlobals->time + 1.7s;
				m_iThinkType = 2;
				break;
			}
			case 2:
			{
				m_iThinkType = 0;
				m_tNextThink = invalid_time_point;
			}
		}
	}

#ifndef CLIENT_DLL
	if (m_tBeamThink != invalid_time_point && m_tBeamThink < gpGlobals->time)
	{
		for (CBeam* pBeam : m_pBeamList)
		{
			if (pBeam)
			{
				pBeam->pev->renderamt -= 40.0;
				if (pBeam->pev->renderamt <= 0.0)
				{
					pBeam->SetBrightness(0);
					pBeam->pev->effects |= EF_NODRAW;
				}
			}

		}
		m_pEnemyList.clear();
		m_tBeamThink = gpGlobals->time + 0.1s;
	}
#endif

	if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] < 6)
	{
		if (m_flTimeNextCharge <= gpGlobals->time)
		{
			m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]++;
			m_flTimeNextCharge = gpGlobals->time + 3.5s;
		}
	}
	return CBasePlayerWeapon::ItemPostFrame();
}


void CReviveGun::Reload(void)
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		return;

	if (DefaultReload(60, IsSignal() ? REVIVEGUN_REVIVE_RELOAD : REVIVEGUN_RELOAD, 2s))
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif	
		m_flAccuracy = 0;
		m_iShotsFired = 0;
		m_bDelayFire = false;
	}
}

#ifndef CLIENT_DLL
Vector CReviveGun::FireBullets(Vector vecSrc, Vector vecDirShooting, float vecSpread, float flDistance, int iPenetration, int iBulletType, int iDamage, float flRangeModifier, entvars_t* pevAttacker, bool bPistol, int shared_rand)
{
	int iOriginalPenetration = iPenetration;
	int iPenetrationPower;
	float flPenetrationDistance;
	int iCurrentDamage = iDamage;
	float flCurrentDistance;

	TraceResult tr, tr2;
	Vector vecRight, vecUp;

	bool bHitMetal = false;
	int iSparksAmount = 1;

	vecRight = gpGlobals->v_right;
	vecUp = gpGlobals->v_up;

	iPenetrationPower = 39;
	flPenetrationDistance = 5000;

	if (!pevAttacker) {
		// the default attacker is ourselves
		pevAttacker = pev;
	}

	gMultiDamage.type = (DMG_BULLET | DMG_NEVERGIB);

	float x, y, z;

	if (IsPlayer()) {
		// Use player's random seed.
		// get circular gaussian spread
		x = UTIL_SharedRandomFloat(shared_rand, -0.5, 0.5) + UTIL_SharedRandomFloat(shared_rand + 1, -0.5, 0.5);
		y = UTIL_SharedRandomFloat(shared_rand + 2, -0.5, 0.5) + UTIL_SharedRandomFloat(shared_rand + 3, -0.5, 0.5);
	}
	else {
		do {
			x = RANDOM_FLOAT(-0.5, 0.5) + RANDOM_FLOAT(-0.5, 0.5);
			y = RANDOM_FLOAT(-0.5, 0.5) + RANDOM_FLOAT(-0.5, 0.5);
			z = x * x + y * y;
		} while (z > 1);
	}

	Vector vecDir, vecEnd;
	Vector vecOldSrc, vecNewSrc;

	vecDir = vecDirShooting + x * vecSpread * vecRight + y * vecSpread * vecUp;
	vecEnd = vecSrc + vecDir * flDistance;

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
			if (iPenetration == iOriginalPenetration)
			{
				MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
				WRITE_BYTE(TE_EXPLOSION);
				WRITE_COORD(tr.vecEndPos[0]);
				WRITE_COORD(tr.vecEndPos[1]);
				WRITE_COORD(tr.vecEndPos[2]);
				WRITE_SHORT(MODEL_INDEX("sprites/ef_revivegun_expA.spr"));
				WRITE_BYTE(6);
				WRITE_BYTE(25);
				WRITE_BYTE(TE_EXPLFLAG_NOPARTICLES | TE_EXPLFLAG_NODLIGHTS | TE_EXPLFLAG_NOSOUND);
				MESSAGE_END();

				EMIT_SOUND_DYN(this->edict(), CHAN_STATIC, "weapons/revivegun-1_exp.wav", VOL_NORM, ATTN_NORM, 0, 94);

				float RadiusDamageAmount = 150;
				CBaseEntity* pEntity = NULL;

				Vector vecEnd;
				int bInWater = (UTIL_PointContents(pev->origin) == CONTENTS_WATER);

				while ((pEntity = UTIL_FindEntityInSphere(pEntity, tr.vecEndPos, 90.0)) != NULL)
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
						CBasePlayer* Player = GetClassPtr<CBasePlayer>(pevAttacker);
						CBasePlayer* target = static_cast<CBasePlayer*>(pEntity);

						if (Player->m_iTeam == target->m_iTeam)
							continue;
					}

					if ((bInWater && pEntity->pev->waterlevel == 0) || (!bInWater && pEntity->pev->waterlevel == 3))
						continue;


					vecEnd = pEntity->BodyTarget(tr.vecEndPos);

					UTIL_TraceLine(tr.vecEndPos, vecEnd, ignore_monsters, pev->owner, &tr2);

					if (tr2.flFraction < 1.0 && tr2.pHit != pEntity->edict())
						continue;

					UTIL_TraceLine(vecSrc, pEntity->pev->origin, dont_ignore_monsters, pev->owner, &tr2);

					int falloff = RadiusDamageAmount / 90.0;

					float flCurrentDamage = max(0, RadiusDamageAmount - (tr.vecEndPos - pEntity->pev->origin).Length() * falloff);

					ClearMultiDamage();
					pEntity->TraceAttack(pevAttacker, flCurrentDamage, pEntity->pev->origin - pevAttacker->origin, &tr2, DMG_BULLET);
					ApplyMultiDamage(pev, pevAttacker);
				}
			}
			iPenetration--;

			flCurrentDistance = tr.flFraction * flDistance;
			iCurrentDamage *= pow(flRangeModifier, flCurrentDistance / 500);

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

			DecalGunshot(&tr, iBulletType, (!bPistol && RANDOM_LONG(0, 3)), pev, bHitMetal);

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

			pEntity->TraceAttack(pevAttacker, iCurrentDamage, vecDir, &tr, (DMG_BULLET | DMG_NEVERGIB));
			iCurrentDamage *= flDamageModifier;
		}
		else
			iPenetration = 0;

		ApplyMultiDamage(pev, pevAttacker);
	}

	return Vector(x * vecSpread, y * vecSpread, 0);
}
#endif

void CReviveGun::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
	SendWeaponAnim(IsSignal() ? REVIVEGUN_REVIVE_IDLE : REVIVEGUN_IDLE, UseDecrement() != FALSE);

}

void CReviveGun::Holster(int skiplocal)
{
	// clear target list ?
	m_flTimeHostler = gpGlobals->time;

	ClearEffect();
	DestroyEffect();
	return CBasePlayerWeapon::Holster();
}

float CReviveGun::GetDamage() const
{
	float flDamage = 25.0f;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flDamage = 180.0f;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 146.0f;
#endif
	return flDamage;
}

float CReviveGun::GetSecondaryAttackDamage() const
{
	float flDamage = 25.0f;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flDamage = 615.0f;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 146.0f;
#endif
	return flDamage;
}
}
