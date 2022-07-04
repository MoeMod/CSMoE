/*
sfpistol.cpp
Copyright (C) 2019 Moemod Haoyuan

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

#include "weapons/WeaponTemplate.hpp"

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif
	class CSFPistol : public LinkWeaponTemplate< CSFPistol,
		TGeneralData,
		BuildTGetItemInfoFromCSW<WEAPON_GLOCK18>::template type,
		TPrecacheEvent,
		TDeployDefault,
		TFirePistol,
		TRecoilKickBack,
		TWeaponIdleDefault
	>
	{

	public:
		static constexpr const char* V_Model = "models/v_sfpistol.mdl";
		static constexpr const char* P_Model = "models/p_sfpistol.mdl";
		static constexpr const char* W_Model = "models/w_sfpistol.mdl";
		static constexpr const char* EventFile = "events/sfpistol.sc";
		static constexpr InventorySlotType ItemSlot = PISTOL_SLOT;
		static constexpr const char* ClassName = "weapon_sfpistol";
		static constexpr const char* AnimExtension = "onehanded";
		static constexpr int MaxClip = 50;

		enum e_sfpistol
		{
			ANIM_IDLE1,
			ANIM_SHOOT1,
			ANIM_SHOOT_END,
			ANIM_RELOAD,
			ANIM_DRAW
		};
		static constexpr float PlayerMaxSpeed = 250;
		static constexpr float ArmorRatioModifier = 1.0f;
		
		static constexpr int DamageDefault = 21;
		static constexpr int DamageZB = 29;
		static constexpr int DamageZBS = 57;

		static constexpr float AccuracyDefault = 0.9;
		/*static constexpr*/ int Penetration = 1;
		KnockbackData KnockBack = { 85.0f, 100.0f, 100.0f, 80.0f, 0.8f };

#ifndef CLIENT_DLL
		WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_energycell" , 100 }; }
#endif
		float GetArmorRatioModifier() override { return 1.2; }
		WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED | ACCURACY_DUCK, 8, 3, 0 }; }

		void Precache(void) override
		{
			PRECACHE_SOUND("weapons/sfpistol_shoot1.wav");
			PRECACHE_SOUND("weapons/sfpistol_clipin.wav");
			PRECACHE_SOUND("weapons/sfpistol_clipout.wav");

			PRECACHE_MODEL("sprites/smoke.spr");
			PRECACHE_MODEL("sprites/smoke_ia.spr");
			PRECACHE_MODEL("sprites/muzzleflash27.spr");
			return Base::Precache();
		}
		int GetItemInfo(ItemInfo* p)
		{
			p->pszName = STRING(pev->classname);
			p->pszAmmo1 = "energycell";
			p->iMaxAmmo1 = 100;
			p->pszAmmo2 = NULL;
			p->iMaxAmmo2 = -1;
			p->iMaxClip = 50;
			p->iSlot = 1;
			p->iPosition = 2;
			p->iId = m_iId = WEAPON_SFPISTOL;
			p->iFlags = 0;
			p->iWeight = GLOCK18_WEIGHT;

			return 1;
		}
		void Reload(void)
		{
			if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
				return;

			if (DefaultReload(50, ANIM_RELOAD, 2.33s))
			{
				StopIdleSound();

#ifndef CLIENT_DLL
				m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
			}
		}

		void WeaponIdle(void)
		{
			ResetEmptySound();
			m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

			if (m_pPlayer->pev->weaponanim == ANIM_SHOOT1)
			{
				StopShootSound();

				SendWeaponAnim(ANIM_SHOOT_END, UseDecrement() != FALSE);

				m_flNextPrimaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.528s;
			}

			if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
				return;

			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 60s;

			SendWeaponAnim(ANIM_IDLE1, UseDecrement() != FALSE);
		}

		void PrimaryAttack() override
		{
			if (m_iClip <= 0)
			{
				StopShootSound();

				if (m_fFireOnEmpty)
				{
					PlayEmptySound();

					m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2s;
				}

#ifndef CLIENT_DLL
				if (TheBots)
				{
					TheBots->OnEvent(EVENT_WEAPON_FIRED_ON_EMPTY, m_pPlayer);
				}
#endif

				return;
			}

			StopIdleSound();

			m_iClip--;
			m_flNextPrimaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.066s;

			m_pPlayer->m_iWeaponVolume = BIG_EXPLOSION_VOLUME;
			m_pPlayer->m_iWeaponFlash = DIM_GUN_FLASH;

			SendWeaponAnim(ANIM_SHOOT1, UseDecrement() != FALSE);

			UTIL_MakeVectors(m_pPlayer->pev->v_angle);

			Vector vecDir = m_pPlayer->FireBullets3(m_pPlayer->GetGunPosition(), gpGlobals->v_forward, 0.0, 4098, 1, 50, GetDamage(), 0.81, m_pPlayer->pev, true, m_pPlayer->random_seed);

			int flags;

#ifdef CLIENT_WEAPONS
			flags = FEV_NOTHOST;
#else
			flags = 0;
#endif

			PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, int(m_pPlayer->pev->punchangle.x * 100), 0, m_iClip == 0, FALSE);

#ifndef CLIENT_DLL
			if (!m_iClip && !m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
				m_pPlayer->SetSuitUpdate("!HEV_AMO0", 0, 0);
#endif
			if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
				KickBack(1.2f, 0.8f, 0.5f, 0.5f, 7.0f, 5.0f, 4);
			else if (m_pPlayer->pev->velocity.Length2D() > 0)
				KickBack(0.7f, 0.5f, 0.325f, 0.065f, 4.0f, 3.0f, 5);
			else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
				KickBack(0.55f, 0.3f, 0.25f, 0.035f, 3.0f, 2.0f, 6);
			else
				KickBack(0.6f, 0.35f, 0.3f, 0.05f, 3.25f, 2.0f, 5);
		}

		void StopIdleSound(void)
		{
#ifndef CLIENT_DLL
			MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->pev);
			WRITE_BYTE(79);
			WRITE_SHORT(m_pPlayer->entindex());
			WRITE_SHORT(CHAN_AUTO);
			WRITE_STRING("weapons/sfpistol_idle.wav");
			MESSAGE_END();
#endif
		}

		void StopShootSound(void)
		{
#ifndef CLIENT_DLL
			MESSAGE_BEGIN(MSG_ALL, gmsgMPToCL);
			WRITE_BYTE(79);
			WRITE_SHORT(m_pPlayer->entindex());
			WRITE_SHORT(CHAN_WEAPON);
			WRITE_STRING("weapons/sfpistol_shoot1.wav");
			MESSAGE_END();
#endif
		}

		void Holster(int skiplocal)
		{
			StopIdleSound();
			StopShootSound();

			CBasePlayerWeapon::Holster(skiplocal);
		}


		float GetDamage() const
		{
			float flDamage = 22.0;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 49.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 57.0f;
#endif
			return flDamage;
		}

	};

	LINK_ENTITY_TO_CLASS(weapon_sfpistol, CSFPistol)
};
