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
#include "wpn_janus7xmas.h"

#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

	class CBeam;

	LINK_ENTITY_TO_CLASS(weapon_janus7xmas, CJanus7xmas)

		static const int MG3_AMMO_GIVE = 200;

	void CJanus7xmas::Spawn(void)
	{
		pev->classname = MAKE_STRING("weapon_janus7xmas");

		Precache();
		m_iId = WEAPON_P90;
		SET_MODEL(ENT(pev), "models/w_janus7xmas.mdl");

		m_iDefaultAmmo = MG3_AMMO_GIVE;
		m_flAccuracy = 0.2;
		m_iShotsCount = 0;
		m_iShotsFired = 0;

		FallInit();
	}

	void CJanus7xmas::Precache(void)
	{
		PRECACHE_MODEL("models/v_janus7xmas.mdl");
		PRECACHE_MODEL("models/w_janus7xmas.mdl");
		PRECACHE_MODEL("models/p_janus7xmas.mdl");
		PRECACHE_SOUND("weapons/janus7-1.wav");
		PRECACHE_SOUND("weapons/janus7-2.wav");
		PRECACHE_SOUND("weapons/janus7xmas_clipout.wav");
		PRECACHE_SOUND("weapons/janus7xmas_clipin.wav");
		PRECACHE_SOUND("weapons/janus7xmas_cliplock.wav");
		PRECACHE_SOUND("weapons/janus7xmas_open.wav");
		PRECACHE_SOUND("weapons/janus7xmas_close.wav");

		m_iShell = PRECACHE_MODEL("models/rshell.mdl");
		m_usFireJanus7xmas = PRECACHE_EVENT(1, "events/janus7xmas.sc");
		m_iSprSmokePuff = PRECACHE_MODEL("sprites/wall_puff1.spr");
		m_iSprBeam = PRECACHE_MODEL("sprites/ef_laserfist_laserbeam.spr");
	}

	int CJanus7xmas::GetItemInfo(ItemInfo *p)
	{
		p->pszName = STRING(pev->classname);
		p->pszAmmo1 = "556NatoBox";
		p->iMaxAmmo1 = MAX_AMMO_556NATOBOX;
		p->pszAmmo2 = NULL;
		p->iMaxAmmo2 = -1;
		p->iMaxClip = MG3_AMMO_GIVE;
		p->iSlot = 0;
		p->iPosition = 4;
		p->iId = m_iId = WEAPON_M249;
		p->iFlags = 0;
		p->iWeight = M249_WEIGHT;

		return 1;
	}


	BOOL CJanus7xmas::Deploy(void)
	{
		iShellOn = 1;
		if (m_iWeaponState & WPNSTATE_M4A1_SILENCED)
		{
			return DefaultDeploy("models/v_janus7xmas.mdl", "models/p_janus7xmas.mdl", ANIM_DRAWB, "m249", UseDecrement() != FALSE);
		}
		else if (IsSignal())
		{
			return DefaultDeploy("models/v_janus7xmas.mdl", "models/p_janus7xmas.mdl", ANIM_DRAW_SIGNAL, "m249", UseDecrement() != FALSE);
		}
		else
			return DefaultDeploy("models/v_janus7xmas.mdl", "models/p_janus7xmas.mdl", ANIM_DRAW, "m249", UseDecrement() != FALSE);
	}

	void CJanus7xmas::ItemPostFrame(void)
	{
		if (m_pPlayer->pev->button & IN_ATTACK2 && m_pPlayer->m_flNextAttack <= 0.0s && !m_fInReload)
		{
			m_pPlayer->pev->button &= ~IN_ATTACK2;
			SecondaryAttack();
		}
		if (IsSignal() && (gpGlobals->time > m_flLastFire + 9.0s))
		{
			m_iShotsCount = 0;
			m_flTimeWeaponIdle = 0.0s;
		}
		else if (m_iWeaponState & WPNSTATE_M4A1_SILENCED && (gpGlobals->time > m_flLastFire + 15.0s))
		{
			m_iShotsCount = 0;
			SendWeaponAnim(ANIM_CHANGEA, UseDecrement() != FALSE);
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.5s;
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.5s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0s;
			m_iWeaponState &= ~WPNSTATE_M4A1_SILENCED;
		}
		return CBasePlayerWeapon::ItemPostFrame();
	}

	void CJanus7xmas::PrimaryAttack(void)
	{

		if (m_iWeaponState & WPNSTATE_M4A1_SILENCED)
		{
			Janus7Lighting();
		}
		else
		{
			if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
				Janus7xmasFire(0.045 + (0.5) * m_flAccuracy, 0.095s, FALSE);
			else if (m_pPlayer->pev->velocity.Length2D() > 140)
				Janus7xmasFire(0.045 + (0.095) * m_flAccuracy, 0.095s, FALSE);
			else
				Janus7xmasFire((0.03) * m_flAccuracy, 0.095s, FALSE);

			m_iShotsCount++;
			if (m_iShotsCount == SIGNAL_SHOTS_COUNT_ZB)
			{
				m_flLastFire = gpGlobals->time;
			}
		}

	}

	void CJanus7xmas::SecondaryAttack(void)
	{

		if (IsSignal() && !(m_iWeaponState & WPNSTATE_M4A1_SILENCED))
		{
			SendWeaponAnim(ANIM_CHANGEB, UseDecrement() != FALSE);
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 2.0s;
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 2.0s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0s;
			m_iWeaponState |= WPNSTATE_M4A1_SILENCED;
			m_flLastFire = gpGlobals->time;
		}

	}

	void CJanus7xmas::Janus7xmasFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
	{
		m_bDelayFire = true;
		m_iShotsFired++;
		m_flAccuracy = ((m_iShotsFired * m_iShotsFired * m_iShotsFired) / 190.0) + 0.3;

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

/*		int flags;
#ifdef CLIENT_WEAPONS
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif
*/
#ifndef CLIENT_DLL
		PLAYBACK_EVENT_FULL(0, m_pPlayer->edict(), m_usFireJanus7xmas, 0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), IsSignal(), FALSE);
#endif		
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

#ifndef CLIENT_DLL
		if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.9s;

		if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			KickBack(1.5, 0.55, 0.3, 0.3, 6.0, 5.0, 5);
		else if (m_pPlayer->pev->velocity.Length2D() > 0)
			KickBack(1.1, 0.3, 0.2, 0.06, 4.0, 2.5, 8);
		else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
			KickBack(0.75, 0.1, 0.1, 0.018, 3.5, 1.2, 9);
		else
			KickBack(0.8, 0.2, 0.18, 0.02, 3.2, 2.25, 7);
	}

	void CJanus7xmas::Janus7Lighting(void)
	{

#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
		SendWeaponAnim(RANDOM_FLOAT(0, 1) ? ANIM_SHOOTB1 : ANIM_SHOOTB2, UseDecrement() != FALSE);

		UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);


		m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
		m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

		Vector vecSrc = m_pPlayer->GetGunPosition();

#ifndef CLIENT_DLL
		CBaseEntity *pEntity = NULL;
		while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, 400.0)) != nullptr)
		{
			if (IsTargetAvailable(m_pPlayer, pEntity, vecSrc))
				break;
		}
		Vector VecEnd;
		if (pEntity != nullptr && pEntity->IsAlive())
		{
			VecEnd = pEntity->pev->origin;
			entvars_t * const pevAttacker = VARS(pev->owner);
			entvars_t * const pevInflictor = this->pev;
			//pEntity->m_LastHitGroup = HITGROUP_CHEST;
			pEntity->TakeDamage(pevInflictor, pevAttacker, GetDamageB(), DMG_BULLET);
		}
		else
		{
			Vector VecForward;
			VecForward = gpGlobals->v_forward;
			VecForward *= 400;
			VecEnd = vecSrc + VecForward;
		}

		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_BEAMENTPOINT);
		WRITE_SHORT(ENTINDEX(m_pPlayer->edict()) | 0x1000);
		WRITE_COORD(VecEnd.x);
		WRITE_COORD(VecEnd.y);
		WRITE_COORD(VecEnd.z);
		WRITE_SHORT(m_iSprBeam);
		WRITE_BYTE(0); // FRAMERATE
		WRITE_BYTE(0); // FRAMERATE
		WRITE_BYTE(1); // LIFE
		WRITE_BYTE(30); // WIDTH
		WRITE_BYTE(15);   // NOISE
		WRITE_BYTE(255);   // R, G, B
		WRITE_BYTE(0);  // R, G, B
		WRITE_BYTE(0);   // R, G, B
		WRITE_BYTE(255);	// BRIGHTNESS
		WRITE_BYTE(25);		// SPEED
		MESSAGE_END();

		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_BEAMENTPOINT);
		WRITE_SHORT(ENTINDEX(m_pPlayer->edict()) | 0x1000);
		WRITE_COORD(VecEnd.x);
		WRITE_COORD(VecEnd.y);
		WRITE_COORD(VecEnd.z);
		WRITE_SHORT(m_iSprBeam);
		WRITE_BYTE(0); // FRAMERATE
		WRITE_BYTE(0); // FRAMERATE
		WRITE_BYTE(1);// LIFE
		WRITE_BYTE(20);  // WIDTH
		WRITE_BYTE(50);   // NOISE
		WRITE_BYTE(250);   // R, G, B
		WRITE_BYTE(50);   // R, G, B
		WRITE_BYTE(0);  // R, G, B
		WRITE_BYTE(255);	// BRIGHTNESS
		WRITE_BYTE(25);		// SPEED
		MESSAGE_END();
#endif

		m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME; // 600
		m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH; // 512

		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.075s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;

		if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			KickBack(1.5, 0.55, 0.3, 0.3, 6.0, 5.0, 5);
		else if (m_pPlayer->pev->velocity.Length2D() > 0)
			KickBack(1.1, 0.3, 0.2, 0.06, 4.0, 2.5, 8);
		else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
			KickBack(0.75, 0.1, 0.1, 0.018, 3.5, 1.2, 9);
		else
			KickBack(0.8, 0.2, 0.18, 0.02, 3.2, 2.25, 7);

		EMIT_SOUND_DYN(this->edict(), CHAN_WEAPON, "weapons/janus7-2.wav", 0.8, ATTN_NORM, 0, PITCH_NORM);

	}

	BOOL CJanus7xmas::IsTargetAvailable(CBasePlayer *m_pPlayer, CBaseEntity *pEntity, Vector vecSrc)
	{

		if (!pEntity->IsAlive())
			return false;
		
#ifndef CLIENT_DLL
		if (g_pGameRules->PlayerRelationship(m_pPlayer, pEntity) == GR_TEAMMATE)
			return false;
#endif

		if (pEntity->IsBSPModel())
			return false;

		if (pEntity->pev == m_pPlayer->pev)
			return false;

		Vector vecGunPostion = m_pPlayer->GetGunPosition();
		Vector vecDelta = (pEntity->Center() - vecGunPostion).Normalize();

		UTIL_MakeVectors(m_pPlayer->pev->v_angle);

		if (DotProduct(gpGlobals->v_forward, vecDelta) < 0.5)
			return false;

		return true;
	}



	void CJanus7xmas::Reload(void)
	{
		if (m_pPlayer->ammo_556natobox <= 0)
			return;

		if (m_iWeaponState & WPNSTATE_M4A1_SILENCED)
			return;

		if (IsSignal())
		{
			if (DefaultReload(WEAPON_CLIP, ANIM_RELOAD_SIGNAL, 4.8s))
			{
#ifndef CLIENT_DLL
				m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
				m_flAccuracy = 0.22;
				m_bDelayFire = false;
				m_iShotsFired = 0;
			}
		}
		else
		{
			if (DefaultReload(WEAPON_CLIP, ANIM_RELOAD, 4.8s))
			{
#ifndef CLIENT_DLL
				m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
				m_flAccuracy = 0.22;
				m_bDelayFire = false;
				m_iShotsFired = 0;
			}
		}

	}

	BOOL CJanus7xmas::IsSignal()
	{
		return m_iShotsCount >= SIGNAL_SHOTS_COUNT_ZB;
	}

	void CJanus7xmas::WeaponIdle(void)
	{
		ResetEmptySound();
		m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;
		if (m_iWeaponState & WPNSTATE_M4A1_SILENCED)
		{
			SendWeaponAnim(ANIM_IDLEB, UseDecrement() != FALSE);
			EMIT_SOUND_DYN(this->edict(), CHAN_WEAPON, "weapons/null.wav", 0.8, ATTN_NORM, 0, PITCH_NORM);
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
		}
		else if (IsSignal())
		{
			SendWeaponAnim(ANIM_IDLE_SIGNAL, UseDecrement() != FALSE);
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
		}
		else
		{
			SendWeaponAnim(ANIM_IDLE, UseDecrement() != FALSE);
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
		}
	}


	float CJanus7xmas::GetDamage() const
	{
		float flDamage = 33.0f;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 35.0f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 35.0f;
#endif
		return flDamage;
	}

	float CJanus7xmas::GetDamageB() const
	{
		float flDamage = 33.0f;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 35.0f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 35.0f;
#endif
		return flDamage;
	}

}