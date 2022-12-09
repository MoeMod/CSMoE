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
#include "wpn_magicsg.h"

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

	LINK_ENTITY_TO_CLASS(weapon_magicsg, CMagicSg)

		void CMagicSg::Spawn(void)
	{
		Precache();
		m_iId = WEAPON_MAGICSG;
		SET_MODEL(ENT(pev), "models/w_magicsg.mdl");
		m_iDefaultAmmo = 144;

		FallInit();
	}

	void CMagicSg::Precache(void)
	{
		PRECACHE_MODEL("models/v_magicsg.mdl");
		PRECACHE_MODEL("models/p_magicsg.mdl");
		PRECACHE_MODEL("models/w_magicsg.mdl");

		PRECACHE_MODEL("sprites/muzzleflash107.spr");
		PRECACHE_MODEL("sprites/muzzleflash108.spr");
		PRECACHE_MODEL("sprites/muzzleflash110.spr");

		PRECACHE_MODEL("sprites/ef_magicsg_star.spr");
		PRECACHE_MODEL("sprites/ef_magicsg_click.spr");
		PRECACHE_MODEL("sprites/ef_magicsg_glitter.spr");

		PRECACHE_SOUND("weapons/magicsg_idle_charge_loop.wav");
		PRECACHE_SOUND("weapons/magicsg_idle_charge1.wav");
		PRECACHE_SOUND("weapons/magicsg_idle_charge2.wav");
		PRECACHE_SOUND("weapons/magicsg_idle_charge3.wav");
		PRECACHE_SOUND("weapons/magicsg_idle_charge4.wav");
		PRECACHE_SOUND("weapons/magicsg_idle_charge5.wav");

		PRECACHE_SOUND("weapons/magicsg_reload.wav");
		PRECACHE_SOUND("weapons/magicsg_shoot2_exp.wav");

		m_iShellId = PRECACHE_MODEL("models/shell_sgdrill.mdl");
		m_usFireMagicSg = PRECACHE_EVENT(1, "events/magicsg.sc");
	}

	int CMagicSg::GetItemInfo(ItemInfo* p)
	{
		p->pszName = STRING(pev->classname);
		p->pszAmmo1 = "magicsg_buckshot";
		p->iMaxAmmo1 = 48;
		p->pszAmmo2 = "magicsgbcs";
		p->iMaxAmmo2 = 5;
		p->iMaxClip = 20;
		p->iSlot = 0;
		p->iPosition = 12;
		p->iId = m_iId = WEAPON_MAGICSG;
		p->iFlags = 0;
		p->iWeight = XM1014_WEIGHT;

		return 1;
	}

	BOOL CMagicSg::Deploy(void)
	{
		m_fMaxSpeed = 245;

		m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 0;
		m_iChargeState = 0;
		m_fChargeTime = invalid_time_point;

		m_bStartedCharge = false;
		m_bIsCharging = false;

		return DefaultDeploy("models/v_magicsg.mdl", "models/p_magicsg.mdl", MAGICSG_DRAW, "xm1014", UseDecrement() != FALSE);
	}

	int CMagicSg::ExtractAmmo(CBasePlayerWeapon* pWeapon)
	{
		m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 0;
		return CBasePlayerWeapon::ExtractAmmo(pWeapon);
	}

	void CMagicSg::Holster(int skiplocal)
	{
		m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 0;
		CBasePlayerWeapon::Holster();
	}



	void CMagicSg::ItemPreFrame(void)
	{

		if (m_fInReload > 0)
			return;

		if (m_bIsCharging)
		{

		}
	}

	void CMagicSg::PrimaryAttack(void)
	{
		if (m_bStartedCharge || m_bIsCharging)
			return;

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
		m_pPlayer->FireBullets(3, m_pPlayer->GetGunPosition(), gpGlobals->v_forward, Vector(0.04, 0.04, 0), 3048, BULLET_PLAYER_MAGICSG_BUCKSHOT, 0, GetDamage());
#endif
		EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "weapons/magicsg_shoot1.wav", VOL_NORM, 0.52, 0, 94 + RANDOM_LONG(0, 15));

		int flags;
#ifdef CLIENT_WEAPONS
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif

		PLAYBACK_EVENT_FULL(flags, ENT(m_pPlayer->pev), m_usFireMagicSg, 0, gpGlobals->v_forward, g_vecZero, m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType], 0.1, 3, m_vVecAiming.x * 100, FALSE, 3);

		SendWeaponAnim(MAGICSG_SHOOT1, UseDecrement() != FALSE);

#ifndef CLIENT_DLL
		if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif


		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2575s;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.2575s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.7s;
		m_fInSpecialReload = 0;


		if (m_pPlayer->pev->velocity.Length2D() > 0)
			m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomFloat(m_pPlayer->random_seed + 1, 3.0, 4.0);
		else if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 5.0, 6.0);
		else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
			m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 1.5, 2.0);
		else
			m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 2.0, 3.0);
	}

	void CMagicSg::Reload(void)
	{
		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			return;

		if (m_bStartedCharge || m_bIsCharging)
			return;


		if (DefaultReload(30, MAGICSG_RELOAD, 3.6s))
		{
#ifndef CLIENT_DLL
			m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
		}
	}


	void CMagicSg::SecondaryAttack(void)
	{
		if (m_iClip <= 0)
			return;

		if (!m_bStartedCharge)
		{
			if (!m_bIsCharging)
			{
				m_bStartedCharge = true;
				m_iChargeState = 1;
				m_fChargeTime = gpGlobals->time + 1.5s;

				SendWeaponAnim(MAGICSG_IDLE_CHARGE, UseDecrement() != FALSE);
				m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.5s;
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 10.0s;

				EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "weapons/magicsg_idle_charge_loop.wav", VOL_NORM, 0.25, 0, PITCH_NORM);
			}
		}
		else
		{
			if (gpGlobals->time >= m_fChargeTime)
			{
				if (m_bStartedCharge)
				{
					m_bIsCharging = true;
					m_bStartedCharge = false;

					m_iChargeState = 2;

					m_fChargeTime = invalid_time_point;
					m_fChargeTime = gpGlobals->time + 0.5s;

					m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5s;
					m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;

					EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "weapons/magicsg_idle_charge1.wav", VOL_NORM, 0.25, 0, PITCH_NORM);

					m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 1;
#ifndef CLIENT_DLL
					MESSAGE_BEGIN(MSG_ONE, gmsgAmmoPickup, NULL, m_pPlayer->pev);
					WRITE_BYTE(CBasePlayer::GetAmmoIndex("magicsgbcs"));
					WRITE_BYTE(1);
					MESSAGE_END();
#endif
				}
			}
		}

		if (!m_bStartedCharge && m_bIsCharging)
		{
			if (m_iChargeState < (2 + min(4, m_iClip - 1)))
			{
				if (gpGlobals->time >= m_fChargeTime)
				{
					m_iChargeState++;
					m_fChargeTime = gpGlobals->time + 0.5s;

					std::string str("weapons/magicsg_idle_charge");
					char SzTemp[16]; sprintf(SzTemp, "%d.wav", (m_iChargeState - 1));
					str.append(SzTemp);

					EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, str.c_str(), VOL_NORM, 0.25, 0, PITCH_NORM);
					EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "weapons/magicsg_idle_charge_loop.wav", VOL_NORM, 0.25, 0, PITCH_NORM);

					m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]++;
					//char SzText[64]; sprintf(SzText, "Charging Now :%d", (int)(m_iChargeState - 1));
					//ClientPrint(m_pPlayer->pev, HUD_PRINTTALK, "%s", SzText);
#ifndef CLIENT_DLL
					MESSAGE_BEGIN(MSG_ONE, gmsgAmmoPickup, NULL, m_pPlayer->pev);
					WRITE_BYTE(CBasePlayer::GetAmmoIndex("magicsgbcs"));
					WRITE_BYTE(1);
					MESSAGE_END();
#endif
				}
			}
		}

		m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
		m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;
		m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
	}


	void CMagicSg::WeaponIdle(void)
	{
		ResetEmptySound();

		m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

		if (m_bStartedCharge)
		{
			m_bStartedCharge = false;

			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0s;
			SendWeaponAnim(MAGICSG_IDLE, UseDecrement() != FALSE);

			m_pPlayer->pev->effects &= ~EF_MUZZLEFLASH;

			m_iChargeState = 0;
			m_fChargeTime = invalid_time_point;

#ifndef CLIENT_DLL
			MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->edict());
			WRITE_BYTE(79);
			WRITE_SHORT(m_pPlayer->entindex());
			WRITE_SHORT(CHAN_WEAPON);
			WRITE_STRING("weapons/magicsg_idle_charge_loop.wav");
			MESSAGE_END();
#endif

			//ClientPrint(m_pPlayer->pev, HUD_PRINTTALK, "Stage 1 Cancelled");
		}
		else if (m_bIsCharging)
		{
			m_bIsCharging = false;

			m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 0;

			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;
			SendWeaponAnim(MAGICSG_SHOOT2, UseDecrement() != FALSE);

			DoShoot2((int)(m_iChargeState - 1));
			m_iClip -= (m_iChargeState - 1);

			m_iChargeState = 0;
			m_fChargeTime = invalid_time_point;

#ifndef CLIENT_DLL
			MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->edict());
			WRITE_BYTE(79);
			WRITE_SHORT(m_pPlayer->entindex());
			WRITE_SHORT(CHAN_WEAPON);
			WRITE_STRING("weapons/magicsg_idle_charge_loop.wav");
			MESSAGE_END();
#endif

			EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "weapons/magicsg_shoot2.wav", VOL_NORM, 0.25, 0, PITCH_NORM);

			m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
			m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;
			m_pPlayer->pev->effects |= EF_MUZZLEFLASH;

		}
		else
		{
			if (m_flTimeWeaponIdle <= UTIL_WeaponTimeBase())
			{
				SendWeaponAnim(MAGICSG_IDLE, UseDecrement() != FALSE);
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.0s;
			}
		}
	}
	void CMagicSg::DoShoot2(int iCount)
	{
#ifndef CLIENT_DLL

		CUtlVector<CBaseEntity*>* pList = new CUtlVector<CBaseEntity*>();
		CUtlVector<CBaseEntity*>* pGroupList = new CUtlVector<CBaseEntity*>();

		//Init as -1
		int iColor = MAGICSG_NONE;
		int iBuffType = MAGICSG_BUFF_NONE;

		switch ((iCount))
		{
		case 1:
		{
			//SME = YELLOW? WHY?
			MagicSgBcsFire(0, pList, pGroupList, MAGICSG_GREEN, MAGICSG_BUFF_KNOCKBACK);
			break;
		}
		case 2:case 3:case 4:
		{
			if (iCount == 2)
			{
				iColor = MAGICSG_YELLOW;
				iBuffType = MAGICSG_BUFF_FORCESTOP;
			}
			else if (iCount == 3)
			{
				iColor = MAGICSG_BLUE;
				iBuffType = MAGICSG_BUFF_BANSKILLS;
			}
			else if (iCount == 4)
			{
				iColor = MAGICSG_PINK;
				iBuffType = MAGICSG_BUFF_RECOVERY;
			}

			MagicSgBcsFire(6.0, pList, pGroupList, iColor, iBuffType);
			MagicSgBcsFire(0, pList, pGroupList, iColor, iBuffType);
			MagicSgBcsFire(-6.0, pList, pGroupList, iColor, iBuffType);
			break;
		}
		case 5:
		{
			//maybe 4/5 to trigger headshot,i am not sure.
			int ToTriggerBuff;
			MagicSgBcsFire(0, pList, pGroupList, MAGICSG_GREEN, MAGICSG_BUFF_HEADSHOT);

			for (int i = 1; i <= 2; i++)
			{
				MagicSgBcsFire(i * 6.0, pList, pGroupList, i <= 1 ? MAGICSG_YELLOW : MAGICSG_BLUE, (i <= 1 ? ((RANDOM_LONG(0, 1) ? MAGICSG_BUFF_HEADSHOT : (RANDOM_LONG(MAGICSG_BUFF_BANSKILLS, MAGICSG_BUFF_BANSKILLS + 1)))) : MAGICSG_BUFF_HEADSHOT));
				MagicSgBcsFire(-i * 6.0, pList, pGroupList, i <= 1 ? MAGICSG_RED : MAGICSG_PINK, (i <= 1 ? ((RANDOM_LONG(0, 1) ? MAGICSG_BUFF_HEADSHOT : (RANDOM_LONG(MAGICSG_BUFF_BANSKILLS, MAGICSG_BUFF_BANSKILLS + 1)))) : MAGICSG_BUFF_HEADSHOT));
			}
			break;
		}
		}
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);

#endif
	}

#ifndef CLIENT_DLL
	void CMagicSg::MagicSgBcsFire(float fYaw, CUtlVector<CBaseEntity*>* pList, CUtlVector<CBaseEntity*>* pGroupList, int iColor, int iBuffType)
	{
		UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

		Vector vecSrc = m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 25 - gpGlobals->v_up * 2;

		float sinA = sin(fYaw * M_PI / 180);
		float cosA = cos(fYaw * M_PI / 180);
		Vector vecDir = gpGlobals->v_forward * cosA + gpGlobals->v_right * sinA;
		Vector vecAngle;
		VectorAngles(vecDir, vecAngle);
		CMagicSgCannon* pEnt = static_cast<CMagicSgCannon*>(CMagicSgCannon::Create("magicsg_cannon", vecSrc, vecAngle, ENT(m_pPlayer->pev)));
		if (pEnt)
		{
			pEnt->Init(vecDir, 700, 10s, GetDamageBcs() * max((iBuffType + 1), 1) * 0.5, pList, pGroupList, GetExplodeDamage() * max((iBuffType + 1), 1), GetExplodeRadius(), GetDirectDamageForce(), iColor, iBuffType);

			PLAYBACK_EVENT_FULL(FEV_GLOBAL, ENT(pEnt->pev), PRECACHE_EVENT(1, "events/wpneffects.sc"), 0.1, pEnt->pev->origin, vecDir, iColor, 1.0, 27, 0, TRUE, TRUE);
			//PLAYBACK_EVENT_FULL(FEV_GLOBAL, ENT(m_pPlayer->pev), m_usFireMagicSg, 0.0, pEnt->pev->origin, vecDir, float(pEnt->entindex()), 0.0, iColor, 1, TRUE, FALSE);
		}

	}
#endif

	float CMagicSg::GetDamage() const
	{
		float flDamage = 16.0;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 120.0f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 120.0f;
#endif
		return flDamage;
	}

	float CMagicSg::GetDamageBcs() const
	{
		float flDamage = 35.0f;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 800.0f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 2350.0f;
#endif
		return flDamage;
	}

	float CMagicSg::GetExplodeDamage(void)
	{
		float flDamage = 105.0f;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 400.0f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 300.0f;
#endif
		return flDamage;
	}

	float CMagicSg::GetDirectDamageForce(void)
	{
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			return 5000;
#endif
		return 100;
	}

	float CMagicSg::GetExplodeRadius(void)
	{
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			return 100;
#endif
		return 75;
	}



}
