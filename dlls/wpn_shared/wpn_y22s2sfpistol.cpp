/*
y22s2sfpistol.cpp
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
	class CY22S2SFPistol : public LinkWeaponTemplate< CY22S2SFPistol,
		TGeneralData,
		BuildTGetItemInfoFromCSW<WEAPON_GLOCK18>::template type,
		TPrecacheEvent,
		TDeployDefault,
		TFirePistol,
		TRecoilKickBack,
		TWeaponIdleDefault
	>
	{
	private:
		int m_iDefaultAmmo2;
		int m_iSaveShoots;

	public:
		static constexpr const char* V_Model = "models/v_y22s2sfpistol.mdl";
		static constexpr const char* P_Model = "models/p_y22s2sfpistol.mdl";
		static constexpr const char* W_Model = "models/w_y22s2sfpistol.mdl";
		static constexpr const char* EventFile = "events/y22s2sfpistol.sc";
		static constexpr InventorySlotType ItemSlot = PISTOL_SLOT;
		static constexpr const char* ClassName = "weapon_y22s2sfpistol";
		static constexpr const char* AnimExtension = "onehanded";
		static constexpr int MaxClip = 50;

		

		enum e_y22s2sfpistol
		{
			ANIM_IDLE1,
			ANIM_SHOOT1,
			ANIM_SHOOT_END,
			ANIM_RELOAD,
			ANIM_DRAW,
			ANIM_SHOOT2
		};
		static constexpr float PlayerMaxSpeed = 250;
		static constexpr float ArmorRatioModifier = 1.0f;

		static constexpr int DamageDefault = 51;
		static constexpr int DamageZB = 139;
		static constexpr int DamageZBS = 500;

		static constexpr float AccuracyDefault = 0.9;
		/*static constexpr*/ int Penetration = 1;
		KnockbackData KnockBack = { 85.0f, 100.0f, 100.0f, 80.0f, 0.8f };

#ifndef CLIENT_DLL
		WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_energycell" , 100 }; }
#endif
		float GetArmorRatioModifier() override { return 1.2; }
		WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED | ACCURACY_DUCK, 8, 3, 0 }; }

		void Spawn() override
		{
			m_iWeaponState = 0;
			m_iDefaultAmmo2 = 0;
			m_iSaveShoots = 0;
			pev->skin = 0;
			Base::Spawn();
		}
		void Precache(void) override
		{
			PRECACHE_SOUND("weapons/y22s2sfpistol-1.wav");
			PRECACHE_SOUND("weapons/y22s2sfpistol_shoot-2.wav");
			PRECACHE_SOUND("weapons/y22s2sfpistol_shoot_end.wav");
			PRECACHE_SOUND("weapons/y22s2sfpistol_idle.wav");
			
			PRECACHE_MODEL("sprites/ef_y22s2sfpistol_laser.spr");
			PRECACHE_MODEL("sprites/ef_y22s2sfpistol_star.spr");
			//PRECACHE_MODEL("sprites/ef_starchasersr_star.spr");
			PRECACHE_MODEL("sprites/ef_y20s3plasmaex_line.spr");
			PRECACHE_MODEL("sprites/ef_y20s3plasmaex_star.spr");
			
			PRECACHE_MODEL("sprites/muzzleflash240.spr");
			PRECACHE_MODEL("sprites/muzzleflash241.spr");
			PRECACHE_MODEL("models/ef_y22s2sfpistol.mdl");

			
			return Base::Precache();
		}
		int GetItemInfo(ItemInfo* p)
		{
			p->pszName = STRING(pev->classname);
			p->pszAmmo1 = "energycell";
			p->iMaxAmmo1 = 100;
			p->pszAmmo2 = "y22s2sfpbcs";
			p->iMaxAmmo2 = 3;
			p->iMaxClip = 50;
			p->iSlot = 1;
			p->iPosition = 2;
			p->iId = m_iId = WEAPON_Y22S2SFPISTOL;
			p->iFlags = 0;
			p->iWeight = GLOCK18_WEIGHT;

			return 1;
		}
		int ExtractAmmo(CBasePlayerWeapon* pWeapon)
		{
			if (m_iDefaultAmmo2)
			{
				m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = m_iDefaultAmmo2;
				m_iDefaultAmmo2 = 0;
			}
			return CBasePlayerWeapon::ExtractAmmo(pWeapon);
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
				KickBack(0.8f, 0.8f, 0.5f, 0.5f, 7.0f, 5.0f, 4);
			else if (m_pPlayer->pev->velocity.Length2D() > 0)
				KickBack(0.7f, 0.5f, 0.325f, 0.065f, 4.0f, 3.0f, 5);
			else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
				KickBack(0.55f, 0.3f, 0.25f, 0.035f, 3.0f, 2.0f, 6);
			else
				KickBack(0.6f, 0.35f, 0.3f, 0.05f, 3.25f, 2.0f, 5);

			if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]< 3 )
			{
				if (m_iSaveShoots < 25)
					++m_iSaveShoots;
				else
				{
					
					m_iSaveShoots = 0;
					++m_iWeaponState;
					m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = m_iWeaponState;
					pev->skin = 1;
#ifndef CLIENT_DLL

					MESSAGE_BEGIN(MSG_ONE, gmsgAmmoPickup, NULL, m_pPlayer->pev);
					WRITE_BYTE(CBasePlayer::GetAmmoIndex("y22s2sfpbcs"));
					WRITE_BYTE(1);
					MESSAGE_END();
#endif
				}
			}
		}
		void SecondaryAttack(void)
		{
			if (m_iWeaponState && m_flNextSecondaryAttack<= UTIL_WeaponTimeBase())
			{
				SendWeaponAnim(ANIM_SHOOT2, UseDecrement() != FALSE);
#ifndef CLIENT_DLL
				m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;
				m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0s;
				StopIdleSound();

#ifndef CLIENT_DLL

				CY22s2SFpistolField::Create(1, m_pPlayer->pev->origin, m_pPlayer->pev->v_angle, ENT(m_pPlayer->pev), m_pPlayer->m_iTeam);
#endif

				--m_iWeaponState;
				m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = m_iWeaponState;

				if (!m_iWeaponState)
				{
					pev->skin = 0;
				}
			}
		}

		void StopIdleSound(void)
		{
#ifndef CLIENT_DLL
			MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->pev);
			WRITE_BYTE(79);
			WRITE_SHORT(m_pPlayer->entindex());
			WRITE_SHORT(CHAN_AUTO);
			WRITE_STRING("weapons/y22s2sfpistol_idle.wav");
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
			WRITE_STRING("weapons/y22s2sfpistol-1.wav");
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
			float flDamage = 51.0;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 139.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 500.0f;
#endif
			return flDamage;
		}

	};

	LINK_ENTITY_TO_CLASS(weapon_y22s2sfpistol, CY22S2SFPistol)
};
