/*
voidpistol.cpp
Copyright (C) 2022 JustANoobQAQ

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
	class CVoidPistol : public LinkWeaponTemplate< CVoidPistol,
		TGeneralData,
		BuildTGetItemInfoFromCSW<WEAPON_GLOCK18>::template type,
		TPrecacheEvent
	>
	{
	public:

#define WPNSTATE_VOIDPISTOL_MODEB	 (1 << 0)
#define WPNSTATE_VOIDPISTOL_MODEC	 (1 << 1)
#define WPNSTATE_VOIDPISTOL_SCAN_ON (1 << 2)

#define VOIDPISTOL_MAXTARGET	10
#define VOIDPISTOL_RELOAD_TIME	2.7s
#define VOIDPISTOL_MAXCLIP 50
#define VOIDPISTOL_AUTOAIM_RADIUS 14.8 * 39.37;
#define VOIDPISTOL_NEXT_SCAN 0.5s

		static constexpr const auto& DefaultReloadTime = 3.75s;
		static constexpr const char* V_Model = "models/v_voidpistol.mdl";
		static constexpr const char* P_Model = "models/p_voidpistol.mdl";
		static constexpr const char* W_Model = "models/w_voidpistol.mdl";
		static constexpr const char* EventFile = "events/voidpistol.sc";
		static constexpr InventorySlotType ItemSlot = PISTOL_SLOT;
		static constexpr const char* ClassName = "weapon_voidpistol";
		static constexpr const char* AnimExtension = "onehanded";
		static constexpr int MaxClip = 50;
		enum
		{
			VOIDPISTOL_IDLEA,
			VOIDPISTOL_IDLEB,
			VOIDPISTOL_IDLEC,
			VOIDPISTOL_SHOOTA,
			VOIDPISTOL_SHOOTB,
			VOIDPISTOL_SHOOTC,
			VOIDPISTOL_SHOOT_BLACKHOLE_A,
			VOIDPISTOL_SHOOT_BLACKHOLE_B,
			VOIDPISTOL_RELOADA,
			VOIDPISTOL_RELOADB,
			VOIDPISTOL_RELOADC,
			VOIDPISTOL_SCANNING_ON,
			VOIDPISTOL_SCANNING_OFF,
			VOIDPISTOL_CHANGEAC,
			VOIDPISTOL_CHANGEBC,
			VOIDPISTOL_DRAWA,
			VOIDPISTOL_DRAWB,
			VOIDPISTOL_DRAWC
		};

		enum voidpistol_bodygroup_e
		{
			GROUP_WEAPON1,
			GROUP_WEAPON2,
			GROUP_WEAPON3,
			GROUP_WEAPON4,
			GROUP_BLACKHOLE,
			GROUP_HANDS,
		};

		static constexpr float PlayerMaxSpeed = 260;
		static constexpr float ArmorRatioModifier = 1.5f;

		static constexpr float AccuracyDefault = 0.9;
		/*static constexpr*/ int Penetration = 1;
		KnockbackData KnockBack = { 450.0f, 350.0f, 400.0f, 300.0f, 0.3f };

#ifndef CLIENT_DLL
		WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_voidpistol" , 200 }; }
#endif
		float GetArmorRatioModifier() override { return 1.5; }
		WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED | ACCURACY_DUCK, 8, 3, 0 }; }

		int m_iModelIndex;
		int m_iDefaultAmmo2;

#ifndef CLIENT_DLL
		std::vector<EHANDLE> m_pEnemylist;

		time_point_t m_flNextTimeScan;
		time_point_t m_flNextThink;

		bool m_bHit;

		int m_iShotsRequired;
		int m_iChargeShotsFired;
#endif

#ifndef CLIENT_DLL
		int GetShotsRequired()
		{
			int iShotsRequired = 70;
			if (g_pModRunning->DamageTrack() == DT_ZB)
				iShotsRequired = 70;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				iShotsRequired = 70;
			return iShotsRequired;
		}
#endif

		void Spawn() override
		{
#ifndef CLIENT_DLL
			m_iChargeShotsFired = 0;
			m_flNextTimeScan = gpGlobals->time;
			m_iShotsRequired = GetShotsRequired();
#endif
			m_iDefaultAmmo2 = 0;
			Base::Spawn();
		}

		void Precache() override
		{
			m_iModelIndex = PRECACHE_MODEL(V_Model);

			PRECACHE_MODEL("models/ef_blackhole.mdl");

			PRECACHE_SOUND("weapons/voidpistol_beep.wav");
			PRECACHE_SOUND("weapons/voidpistol-1.wav");
			PRECACHE_SOUND("weapons/voidpistol-2.wav");

			PRECACHE_MODEL("sprites/ef_blackhole_star.spr");

			//PRECACHE_MODEL("sprites/ef_blackhole01.spr"); //charing spr attached to model
			//PRECACHE_MODEL("sprites/ef_blackhole02.spr");
			//PRECACHE_MODEL("sprites/ef_blackhole03.spr");
			PRECACHE_MODEL("sprites/ef_blackhole04.spr");//infront of the gun charing finished

			PRECACHE_MODEL("sprites/muzzleflash140.spr");
			PRECACHE_MODEL("sprites/muzzleflash141.spr");

			Base::Precache();
		}


		int GetItemInfo(ItemInfo* p)
		{
			p->pszName = STRING(pev->classname);
			p->pszAmmo1 = "voidpistolammo";
			p->iMaxAmmo1 = 100;
			p->pszAmmo2 = "voidpistolammo2";
			p->iMaxAmmo2 = 0;
			p->iMaxClip = VOIDPISTOL_MAXCLIP;
			p->iSlot = 1;
			p->iPosition = 1;
			p->iId = m_iId = WEAPON_VOIDPISTOL;
			p->iFlags = 0;
			p->iWeight = DEAGLE_WEIGHT;

			return 1;
		}

#ifndef CLIENT_DLL
		void SetSubModel()
		{
			pev->modelindex = m_iModelIndex;

			int iSubModel;

			if (m_iChargeShotsFired > 40)
				iSubModel = 2;
			else if (m_iChargeShotsFired > 15)
				iSubModel = 1;
			else
				iSubModel = 0;

			if (GetBodygroup(GROUP_BLACKHOLE) != iSubModel)
			{
				SetBodygroup(GROUP_BLACKHOLE, iSubModel);
			}

			int hands = 0;

			if (m_pPlayer && m_pPlayer->m_bIsFemale)
				hands = 1;

			SetBodygroup(GROUP_HANDS, hands);

			pev->modelindex = 0;
		}
#endif

		int ExtractAmmo(CBasePlayerWeapon* pWeapon)
		{
			if (m_iDefaultAmmo2)
			{
				m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = m_iDefaultAmmo2;
				m_iDefaultAmmo2 = 0;
			}
			return CBasePlayerWeapon::ExtractAmmo(pWeapon);
		}

		BOOL Deploy() override
		{
			m_flAccuracy = 0.9;

			m_fMaxSpeed = 260;

			int iAnim;
			if (m_iWeaponState & WPNSTATE_VOIDPISTOL_MODEC)
				iAnim = VOIDPISTOL_DRAWC;
			else
				iAnim = VOIDPISTOL_DRAWA;

#ifndef CLIENT_DLL
			m_pEnemylist.clear();

			m_iWeaponState &= ~WPNSTATE_VOIDPISTOL_SCAN_ON;
			if (m_iWeaponState & WPNSTATE_VOIDPISTOL_MODEB)
				m_iWeaponState &= ~WPNSTATE_VOIDPISTOL_MODEB;
#endif
			if (DefaultDeploy(V_Model, P_Model, iAnim, "onehanded", UseDecrement() != FALSE, 0.2s))
			{
#ifndef CLIENT_DLL
				m_flNextTimeScan = gpGlobals->time + 2.0s;

				SetSubModel();
#endif
				return TRUE;
			}
			return FALSE;

		}

		void Holster(int skiplocal)
		{
#ifndef CLIENT_DLL
			m_pEnemylist.clear();
#endif
			CBasePlayerWeapon::Holster();
		}
		void SecondaryAttack() override
		{
			if (!(m_iWeaponState & WPNSTATE_VOIDPISTOL_MODEC))
				return;

			m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
			m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;
			m_pPlayer->pev->effects |= EF_MUZZLEFLASH;

#ifndef CLIENT_DLL
			m_iChargeShotsFired = 0;
			SetSubModel();
			m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] -= 1;

			m_pPlayer->SetAnimation(PLAYER_ATTACK1);
			SendWeaponAnim(m_pEnemylist.empty() ? VOIDPISTOL_SHOOT_BLACKHOLE_A : VOIDPISTOL_SHOOT_BLACKHOLE_B, UseDecrement() != FALSE);

			EMIT_SOUND_DYN(ENT(this->pev), CHAN_WEAPON, "weapons/voidpistol-2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

			if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] <= 0)
				m_iWeaponState &= ~WPNSTATE_VOIDPISTOL_MODEC;

			if (!m_pEnemylist.empty())
				m_iWeaponState |= (WPNSTATE_VOIDPISTOL_MODEB | WPNSTATE_VOIDPISTOL_SCAN_ON);

			UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
			Vector vecSrc = m_pPlayer->GetGunPosition();
			CVoidPistolBlackHole* pProjectile = CVoidPistolBlackHole::Create(0, vecSrc, m_pPlayer->pev->v_angle, ENT(m_pPlayer->pev), m_pPlayer->m_iTeam);

			if (pProjectile)
			{
				pProjectile->pev->velocity = gpGlobals->v_forward * 600;
				pProjectile->m_pOwner = m_pPlayer;
			}
#endif		
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;

		}

		void PrimaryAttack() override
		{
			if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
				Fire(1.2 * m_flAccuracy, 0.225s, FALSE);
			else if (m_pPlayer->pev->velocity.Length2D() > 0)
				Fire(0.05 * m_flAccuracy, 0.225s, FALSE);
			else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
				Fire(0.01 * m_flAccuracy, 0.225s, FALSE);
			else
				Fire(0.02 * m_flAccuracy, 0.225s, FALSE);
		}

#ifndef CLIENT_DLL
		void FireBullet3CallBack(int iPenetration, Vector vecDir, TraceResult* pTrace)
		{
			if (!m_bHit && !(m_iWeaponState & WPNSTATE_VOIDPISTOL_MODEC))
			{
				if (CBaseEntity* pEntity = gMultiDamage.pEntity)
				{
					if (pEntity->IsPlayer() || pEntity->Classify() == CLASS_PLAYER_ALLY)
					{
						if (g_pGameRules->PlayerRelationship(m_pPlayer, pEntity) != GR_TEAMMATE)
						{
							m_iChargeShotsFired++;
							m_bHit = true;
						}
					}
				}
			}
		}

		void FindTargets()
		{
			const float flRadius = VOIDPISTOL_AUTOAIM_RADIUS;

			m_pEnemylist.erase(std::remove_if(m_pEnemylist.begin(), m_pEnemylist.end(), [=](CBaseEntity* pEntity) {

				if (!pEntity->IsAlive() || UTIL_IsWallBetweenEntity(pEntity, m_pPlayer))
				return true;

			Vector vecDelta = m_pPlayer->pev->origin - pEntity->pev->origin;

			if (vecDelta.IsLengthGreaterThan(flRadius))
				return true;

			//UTIL_MakeVectors(m_pPlayer->pev->v_angle);

			//Vector vecSrc = m_pPlayer->GetGunPosition();
			//Vector vecDelta = (pEntity->Center() - vecSrc).Normalize();

			//if (DotProduct(gpGlobals->v_forward, vecDelta) < 0.7)
			//	return true;

			return false;
				}), m_pEnemylist.end());


			CBaseEntity* pEntity = NULL;
			while ((pEntity = UTIL_FindEntityInSphere(pEntity, m_pPlayer->pev->origin, flRadius)) != nullptr)
			{
				EHANDLE eh;
				eh.Set(pEntity->edict());

				if (std::find(m_pEnemylist.begin(), m_pEnemylist.end(), eh) != m_pEnemylist.end())
					continue;

				if (m_pEnemylist.size() > VOIDPISTOL_MAXTARGET)
					break;

				if (CheckTargetAvailable(pEntity, m_pPlayer->pev->v_angle))
				{
					m_pEnemylist.push_back(eh);
				}
			}
		}

		bool CheckTargetAvailable(CBaseEntity* a2, Vector vecAngleDirection)
		{
			const float flRadius = VOIDPISTOL_AUTOAIM_RADIUS;

			if (!a2->IsPlayer() && a2->Classify() != CLASS_PLAYER_ALLY)
				return false;

			if (!a2->IsAlive())
				return false;

			if (g_pGameRules->PlayerRelationship(m_pPlayer, a2) == GR_TEAMMATE)
				return false;

			if (a2->pev == m_pPlayer->pev)
				return false;

			/*Vector vecSrc = m_pPlayer->GetGunPosition();
			Vector vecDelta = (a2->Center() - vecSrc).Normalize();

			UTIL_MakeVectors(vecAngleDirection);

			if (DotProduct(gpGlobals->v_forward, vecDelta) < 0.7)
				return false;*/

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


		void PrimaryAttack_InstantDamage()
		{
			for (CBaseEntity* pEntity : m_pEnemylist)
			{
				if (m_iClip <= 0)
					break;

				if (!pEntity)
					continue;

				m_iClip--;
				m_iChargeShotsFired++;

				if (m_iChargeShotsFired == m_iShotsRequired)
				{
					m_iChargeShotsFired++;
					m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]++;

					MESSAGE_BEGIN(MSG_ONE, gmsgAmmoPickup, NULL, m_pPlayer->pev);
					WRITE_BYTE(CBasePlayer::GetAmmoIndex("voidpistolammo2"));
					WRITE_BYTE(1);
					MESSAGE_END();

					m_flNextThink = gpGlobals->time + 0.1s;
				}

				Vector vecDirection = (pEntity->pev->origin - m_pPlayer->pev->origin).Normalize();
				Vector vecSrc = m_pPlayer->GetGunPosition();

				TraceResult tr;
				UTIL_TraceLine(vecSrc, vecDirection * 8192, missile, ENT(m_pPlayer->pev), &tr);

				float flDamage = GetDamage();

				float flCurrentDistance = tr.flFraction * 8192;
				flDamage *= pow(0.88, flCurrentDistance / 500);

				ClearMultiDamage();
				pEntity->TraceAttack(m_pPlayer->pev, flDamage, vecDirection, &tr, DMG_BULLET);
				ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);
			}

			if (!m_bHit && !(m_iWeaponState & WPNSTATE_VOIDPISTOL_MODEC))
				m_bHit = true;
		}
#endif

		void Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
		{
			if (m_flLastFire != invalid_time_point)
			{
				m_flAccuracy -= (0.3 - ((gpGlobals->time - m_flLastFire) / 1s)) * 0.325;

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


			m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
			m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

			UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
			m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;

			Vector vecSrc = m_pPlayer->GetGunPosition();
			Vector vecDir;

#ifndef CLIENT_DLL
			m_bHit = m_iWeaponState & WPNSTATE_VOIDPISTOL_MODEC ? true : false;
#endif

			if (m_iWeaponState & WPNSTATE_VOIDPISTOL_SCAN_ON)
			{

#ifndef CLIENT_DLL
				PrimaryAttack_InstantDamage();
#endif
			}
			else
			{
				m_iClip--;
				vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 3, BULLET_PLAYER_9MM, GetDamage(), 0.88, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);
			}

			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
#ifndef CLIENT_DLL
			SetSubModel();

			PLAYBACK_EVENT_FULL(FEV_GLOBAL, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), m_iWeaponState & WPNSTATE_VOIDPISTOL_SCAN_ON, m_iWeaponState);

			if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
				m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif

			m_pPlayer->pev->punchangle.x -= 2;
		}

		void ItemPostFrame() override
		{
#ifndef CLIENT_DLL
			if (m_iChargeShotsFired == m_iShotsRequired)
			{
				m_iChargeShotsFired++;
				m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]++;

				MESSAGE_BEGIN(MSG_ONE, gmsgAmmoPickup, NULL, m_pPlayer->pev);
				WRITE_BYTE(CBasePlayer::GetAmmoIndex("voidpistolammo2"));
				WRITE_BYTE(1);
				MESSAGE_END();

				m_flNextThink = gpGlobals->time + 0.1s;
			}

			if (m_flNextTimeScan < gpGlobals->time)
			{
				m_flNextTimeScan = gpGlobals->time + VOIDPISTOL_NEXT_SCAN;
				if (g_pModRunning->DamageTrack() != DT_NONE)
					FindTargets();
			}

			if (m_pEnemylist.empty())
			{
				m_iWeaponState &= ~WPNSTATE_VOIDPISTOL_SCAN_ON;
				if (m_iWeaponState & WPNSTATE_VOIDPISTOL_MODEB)
				{
					m_iWeaponState &= ~WPNSTATE_VOIDPISTOL_MODEB;

					SendWeaponAnim(VOIDPISTOL_SCANNING_OFF, 0);
					m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.8s;
				}
			}
			else
			{
				if (!(m_iWeaponState & (WPNSTATE_VOIDPISTOL_MODEC | WPNSTATE_VOIDPISTOL_MODEB)))
				{
					m_iWeaponState |= WPNSTATE_VOIDPISTOL_MODEB;

					SendWeaponAnim(VOIDPISTOL_SCANNING_ON, 0);
					m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.8s;
				}

				m_iWeaponState |= WPNSTATE_VOIDPISTOL_SCAN_ON;
			}

			if (m_flNextThink != invalid_time_point && m_flNextThink < gpGlobals->time)
			{
				if (m_iWeaponState & WPNSTATE_VOIDPISTOL_MODEB)
				{
					m_iWeaponState &= ~WPNSTATE_VOIDPISTOL_MODEB;
					SendWeaponAnim(VOIDPISTOL_CHANGEBC, UseDecrement() != FALSE);
					m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.875s;
					m_iWeaponState |= WPNSTATE_VOIDPISTOL_MODEC;
					m_flNextThink = invalid_time_point;
				}
				else
				{
					SendWeaponAnim(VOIDPISTOL_CHANGEAC, UseDecrement() != FALSE);
					m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.875s;
					m_iWeaponState |= WPNSTATE_VOIDPISTOL_MODEC;
					m_flNextThink = invalid_time_point;
				}
			}

#endif
			return CBasePlayerWeapon::ItemPostFrame();
		}

		void Reload() override
		{
			if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
				return;

			int iAnim;
			if (m_iWeaponState & WPNSTATE_VOIDPISTOL_MODEC)
				iAnim = VOIDPISTOL_RELOADC;
			else if (m_iWeaponState & WPNSTATE_VOIDPISTOL_MODEB)
				iAnim = VOIDPISTOL_RELOADB;
			else
				iAnim = VOIDPISTOL_RELOADA;


			if (DefaultReload(MaxClip, iAnim, VOIDPISTOL_RELOAD_TIME))
			{
#ifndef CLIENT_DLL

				m_pEnemylist.clear();
				m_flNextTimeScan = gpGlobals->time + +VOIDPISTOL_RELOAD_TIME + 0.5s;

				m_iWeaponState &= ~WPNSTATE_VOIDPISTOL_SCAN_ON;
				if (m_iWeaponState & WPNSTATE_VOIDPISTOL_MODEB)
					m_iWeaponState &= ~WPNSTATE_VOIDPISTOL_MODEB;

				m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
				m_flAccuracy = 0.9;
				m_iShotsFired = 0;

				m_bDelayFire = false;
			}
		}

		void WeaponIdle(void) override
		{
			ResetEmptySound();
			m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

			if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
				return;

			int iAnim;
			if (m_iWeaponState & WPNSTATE_VOIDPISTOL_MODEB)
				iAnim = VOIDPISTOL_IDLEB;
			else if (m_iWeaponState & WPNSTATE_VOIDPISTOL_MODEC)
				iAnim = VOIDPISTOL_IDLEC;
			else
				iAnim = VOIDPISTOL_IDLEA;

			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
			SendWeaponAnim(iAnim, UseDecrement() != FALSE);
		}

		float GetDamage() const
		{
			float flDamage = 34.0f;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 300.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 520.0f;
#endif
			return flDamage;
		}
	};
	LINK_ENTITY_TO_CLASS(weapon_voidpistol, CVoidPistol)
}