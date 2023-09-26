/*
pianogun.cpp
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
#include "weapons/RadiusDamage.hpp"
#include "weapons/KnifeAttack.h"

//PrimaryAttackInterval.
#define PianoGunPrimaryAttackInterval1 0.81s
//When doing primaryattack,the interval between three automatic attacks.
#define PianoGunPrimaryAttackInterval2 0.05s
//When triggering the skill,time to delay.
#define PianoGunPreparationBeforeSkills 0.875s
//SecondaryAttackInterval.
#define PianoGunSecondaryAttackInterval 1.0s
//Animation Time for all attacks.
#define PianoGunAttackAnimTime 1.36s

//Skills
#define PianoGunRecovery 0
#define PianoGunRemoveDebuff 1

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

using WeaponTemplate::Varibles::A;
using WeaponTemplate::Varibles::N;

struct PianoGun_WeaponData
{
		static constexpr const char* V_Model = "models/v_pianogun.mdl";
		static constexpr const char* P_Model = "models/p_pianogun.mdl";
		static constexpr const char* W_Model = "models/w_pianogun.mdl";
		static constexpr const char* EventFile = "events/pianogun.sc";
		static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
		static constexpr const char* ClassName = "weapon_pianogun";
		static constexpr const char* AnimExtension = "pianogun";
		static constexpr int MaxClip = 180;
		enum
		{
			ANIM_IDLE1 = 0,
			ANIM_IDLE2,
			ANIM_SHOOTA,
			ANIM_SHOOTB,
			ANIM_SHOOTC1,
			ANIM_SHOOTC2,
			ANIM_RELOAD,
			ANIM_DRAW
		};
		static constexpr float ArmorRatioModifier = 1.85f;
		static constexpr const auto& AccuracyCalc = (N * N * N / 220.0) + 0.3;
		static constexpr float AccuracyDefault = 0.25;
		static constexpr float AccuracyMax = 1.0;
		static constexpr float RangeModifier = 0.98;
		static constexpr auto BulletType = BULLET_PLAYER_762MM;
		static constexpr int Penetration = 2;
		KnockbackData KnockBack = { 500.0f, 250.0f, 450.0f, 300.0f, 0.7f };
};
	class CPianoGun : public LinkWeaponTemplate< CPianoGun,
		TGeneralData,
		BuildTGetItemInfoFromCSW<WEAPON_M249>::template type,
		TPrecacheEvent,
		TDeployDefault
	>, public PianoGun_WeaponData
	{
	public:
	


#ifndef CLIENT_DLL
		WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_pianogun_energycell" , 100 }; }
#endif
		WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 4, 4, 140 }; }

		void Precache(void) override
		{
			Base::Precache();
			PRECACHE_MODEL("sprites/ef_pianogun_note1.spr");
			PRECACHE_MODEL("sprites/ef_pianogun_note2.spr");
			PRECACHE_MODEL("sprites/ef_pianogun_note3.spr");
			PRECACHE_MODEL("sprites/ef_pianogun_note4.spr");

			PRECACHE_MODEL("sprites/muzzleflash130.spr");
			PRECACHE_MODEL("sprites/muzzleflash131.spr");
			PRECACHE_MODEL("sprites/muzzleflash133.spr");
			PRECACHE_MODEL("sprites/muzzleflash134.spr");
			PRECACHE_MODEL("sprites/muzzleflash136.spr");
			PRECACHE_MODEL("sprites/muzzleflash137.spr");

			PRECACHE_SOUND("weapons/pianogun_shoot_a.wav");
			PRECACHE_SOUND("weapons/pianogun_shoot_b.wav");
			PRECACHE_SOUND("weapons/pianogun_shootc1.wav");
			PRECACHE_SOUND("weapons/pianogun_shootc2.wav");
			PRECACHE_SOUND("weapons/pianogun-1.wav");
			PRECACHE_SOUND("weapons/pianogun-2.wav");
			PRECACHE_SOUND("weapons/pianogun-3_1.wav");
			PRECACHE_SOUND("weapons/pianogun-3_2.wav");
		}

		void Spawn(void) override
		{
			pev->classname = MAKE_STRING("weapon_pianogun");

			Precache();
			m_iId = WEAPON_PIANOGUN;
			SET_MODEL(ENT(pev), "models/w_pianogun.mdl");
			m_flAccuracy = 0.25;
			m_iShotsFired = 0;
			m_iDefaultAmmo = 180;

			m_bDoingPrimaryAttack = false;
			m_iPrimaryAttackCount = 0;
			m_iPrimaryAttackInterval = invalid_time_point;

			m_iSaveUserButton = 0;

			FallInit();
		}

		int GetItemInfo(ItemInfo* p) override
		{
			p->pszName = STRING(pev->classname);
			p->pszAmmo1 = "pianogun_energycell";
			p->iMaxAmmo1 = 360;
			p->pszAmmo2 = NULL;
			p->iMaxAmmo2 = -1;
			p->iMaxClip = 180;
			p->iSlot = 0;
			p->iPosition = 2;
			p->iId = m_iId = WEAPON_PIANOGUN;
			p->iFlags = 0;
			p->iWeight = M249_WEIGHT;

			return 1;
		}
		bool HasSecondaryAttack() override { return true; }

		void Reload(void) override
		{
			if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
				return;

			if (DefaultReload(180, ANIM_RELOAD, 2.0s))
			{
#ifndef CLIENT_DLL
				m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif

				m_flAccuracy = 0.25;
				m_iShotsFired = 0;
				m_bDelayFire = false;
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0s;

				m_bDoingPrimaryAttack = false;
				m_iPrimaryAttackCount = 0;
				m_iPrimaryAttackInterval = invalid_time_point;

				m_iSaveUserButton = 0;
			}
		}
		void WeaponIdle(void) override
		{
			ResetEmptySound();
			m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

			if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
				return;

			m_iSaveUserButton = 0;

			if (pev->iuser1)
			{
				if (m_flNextIdleThink <= gpGlobals->time)
				{
					m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 18s;
					SendWeaponAnim(ANIM_IDLE2, UseDecrement() != FALSE);
					pev->iuser1 = 0;
					m_flNextIdleThink = gpGlobals->time + 18.0s;

				}
			}
			else
			{

				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 10s;
				SendWeaponAnim(ANIM_IDLE1, UseDecrement() != FALSE);
				pev->iuser1 = 1;

			}

		}

		float GetMaxSpeed(void) override
		{
			return 220;
		}

		BOOL Deploy(void) override
		{
			pev->iuser1 = 0;
			m_iSaveUserButton = 0;
			m_flNextIdleThink = invalid_time_point;
			m_flNextIdleThink = gpGlobals->time + 10.0s;
			return Base::Deploy();
		}

		void PrimaryAttack(void) override
		{
			if (m_iClip <= 0)
			{
				if (m_fFireOnEmpty)
				{
					PlayEmptySound();
					m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2s;
				}

				return;
			}

			if (!m_bDoingPrimaryAttack && m_iPrimaryAttackInterval == invalid_time_point)
			{
				m_bDoingPrimaryAttack = true;
				m_iPrimaryAttackCount = 1;
				m_iPrimaryAttackInterval = gpGlobals->time + PianoGunPrimaryAttackInterval2;

				SendWeaponAnim(ANIM_SHOOTA, UseDecrement() != FALSE);
				pev->iuser1 = 0;
				m_flNextIdleThink = invalid_time_point;
				m_flNextIdleThink = gpGlobals->time + 10.0s;

				duration_t flCycleTime;
				flCycleTime = PianoGunPrimaryAttackInterval1;

				if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
					SetFireNoteStart((0.4) * m_flAccuracy + 0.04, flCycleTime, FALSE);
				else if (m_pPlayer->pev->velocity.Length2D() > 120)
					SetFireNoteStart((0.07) * m_flAccuracy + 0.04, flCycleTime, FALSE);
				else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
					SetFireNoteStart((0.001) * m_flAccuracy + 0.001, flCycleTime, FALSE);
				else
					SetFireNoteStart((0.03) * m_flAccuracy + 0.002, flCycleTime, FALSE);

				char SzMsg[128]{};
				switch (m_iSaveUserButton)
				{
				case 0:
				{
					m_iSaveUserButton = 1;
					sprintf(SzMsg, "Press Button:Left & -Left");
					break;
				}
				case 1:
				{
					m_iSaveUserButton = 2;
					sprintf(SzMsg, "Press Button:Left & -Left-Left");
					break;
				}
				case 3:
				{
					m_iSaveUserButton = 0;
					m_bShouldDoSkills = true;
					m_iSkillType = 1;
					m_flNextSkillTime = gpGlobals->time + PianoGunPreparationBeforeSkills;
					sprintf(SzMsg, "Press Button:Left & -Left-Right-Left");
					break;
				}
				default:
				{
					m_iSaveUserButton = 0;
					sprintf(SzMsg, "Invalid Button Combination.");
					break;
				}
				}
				//ClientPrint(m_pPlayer->pev, HUD_PRINTTALK, "%s", SzMsg);
			}

		}

		void SecondaryAttack(void) override
		{
			if (m_iClip <= 0)
			{
				if (m_fFireOnEmpty)
				{
					PlayEmptySound();
					m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2s;
				}

				return;
			}

			if (m_iClip >= 12)
				m_iClip -= 12;
			else
				return;

			ShootWave(2);
			
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + PianoGunSecondaryAttackInterval;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + PianoGunAttackAnimTime;

			m_pPlayer->pev->effects |= EF_MUZZLEFLASH;

			Vector vecDir;

			int flags;
#ifdef CLIENT_WEAPONS
			flags = FEV_NOTHOST;
#else
			flags = 0;
#endif
			PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, 0, 0, TRUE, FALSE);

			SendWeaponAnim(ANIM_SHOOTB, UseDecrement() != FALSE);
			pev->iuser1 = 0;
			m_flNextIdleThink = invalid_time_point;
			m_flNextIdleThink = gpGlobals->time + 10.0s;

			char SzMsg[128]{};
			switch (m_iSaveUserButton)
			{
			case 1:
			{
				m_iSaveUserButton = 3;
				sprintf(SzMsg, "Press Button:Right & -Left-Right");
				break;
			}
			case 2:
			{
				m_iSaveUserButton = 0;
				m_bShouldDoSkills = true;
				m_iSkillType = 3;
				m_flNextSkillTime = gpGlobals->time + PianoGunPreparationBeforeSkills;
				sprintf(SzMsg, "Press Button:Right & -Left-Left-Right");
				break;
			}
			default:
			{
				m_iSaveUserButton = 0;
				sprintf(SzMsg, "Invalid Button Combination.");
				break;
			}
			}
			//ClientPrint(m_pPlayer->pev, HUD_PRINTTALK, "%s", SzMsg);


		}


		void ItemPostFrame(void) override
		{
			if (m_iPrimaryAttackInterval > invalid_time_point && m_iPrimaryAttackInterval< gpGlobals->time)
			{
				if (m_iPrimaryAttackCount < 3)
				{
					if (m_bDoingPrimaryAttack && this->m_iClip > 0)
					{
						int iClipBeforeAttack = m_iClip;
						--m_iClip;
						++m_iPrimaryAttackCount;
						m_iPrimaryAttackInterval = gpGlobals->time + PianoGunPrimaryAttackInterval2;
#ifndef CLIENT_DLL
						m_pPlayer->m_pModStrategy->OnAttack_Post(this, iClipBeforeAttack, m_iClip, m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]);
#endif
						ShootNote(0, m_iPrimaryAttackCount);
					}
				}
				else
				{
					m_bDoingPrimaryAttack = false;
					m_iPrimaryAttackInterval = invalid_time_point;
					m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + PianoGunAttackAnimTime;

					//return CBasePlayerWeapon::ItemPostFrame();
				}
			}
			if (m_bShouldDoSkills)
			{
				if (m_flNextSkillTime != invalid_time_point && m_flNextSkillTime < gpGlobals->time)
				{
					m_bShouldDoSkills = false;
					m_flNextSkillTime = invalid_time_point;

					if (m_iSkillType == 1)
					{

						ShootNote(m_iSkillType, 0);
						SendWeaponAnim(ANIM_SHOOTC1, UseDecrement() != FALSE);
						pev->iuser1 = 0;

						EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "weapons/pianogun_shootc1.wav", VOL_NORM, ATTN_NORM, 0, 94);
						int flags;
#ifdef CLIENT_WEAPONS
						flags = FEV_NOTHOST;
#else
						flags = 0;
#endif
						PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, 0, 0, 0, 0, TRUE, TRUE);

					}
					else if (m_iSkillType == 3)
					{
						ShootWave(m_iSkillType);
#ifdef CLINT_DLL
						DoRemoveDebuff(PianoGunRecovery, m_pPlayer->origin, m_pPlayer);
						DoRemoveDebuff(PianoGunRemoveDebuff, m_pPlayer->origin, m_pPlayer);
#endif
						SendWeaponAnim(ANIM_SHOOTC2, UseDecrement() != FALSE);
						pev->iuser1 = 0;
						EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "weapons/pianogun_shootc2.wav", VOL_NORM, ATTN_NORM, 0, 94);
						int flags;
#ifdef CLIENT_WEAPONS
						flags = FEV_NOTHOST;
#else
						flags = 0;
#endif
						PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, 0, 0, 0, 1, TRUE, TRUE);

					}
					m_flNextIdleThink = invalid_time_point;
					m_flNextIdleThink = gpGlobals->time + 10.0s;
					m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + PianoGunAttackAnimTime;
					m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0s;
				}
			}
			return CBasePlayerWeapon::ItemPostFrame();
		}


		void SetFireNoteStart(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
		{
			m_iShotsFired++;
			m_flAccuracy = AccuracyCalc(N = m_flAccuracy);

			if (m_flAccuracy > AccuracyMax)
				m_flAccuracy = AccuracyMax;

#ifndef CLIENT_DLL
			m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

			UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

			m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;	// 1000
			m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;	// 512
			m_pPlayer->pev->effects |= EF_MUZZLEFLASH;

			Vector vecDir;
			ShootNote(0, m_iPrimaryAttackCount);
			--m_iClip;

			int flags;
#ifdef CLIENT_WEAPONS
			flags = FEV_NOTHOST;
#else
			flags = 0;
#endif

			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
			PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, 0, 0, FALSE, TRUE);

#ifndef CLIENT_DLL
			if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
				m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + PianoGunAttackAnimTime;

			if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
				KickBack(1.2, 0.5, 0.23, 0.15, 5.5, 3.5, 6);
			else if (m_pPlayer->pev->velocity.Length2D() > 0)
				KickBack(1.0, 0.45, 0.28, 0.045, 3.75, 3.0, 7);
			else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
				KickBack(0.6, 0.3, 0.2, 0.0125, 3.25, 2.0, 7);
			else
				KickBack(0.65, 0.35, 0.25, 0.015, 3.5, 2.25, 7);
		}

		void ShootNote(int iType, int iCount)
		{
#ifndef CLIENT_DLL
			UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
			Vector vecSrc = m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 50;
			//跳起来
			if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			{
				if (iCount < 2)
					vecSrc.z -= iCount * 5;
				else vecSrc.z -= iCount * 10;
				vecSrc.y += iCount * 8;
				vecSrc.x += iCount * 8;
			}
			else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
			{
				if (iCount < 2)
					vecSrc.z -= iCount * 5;
				else vecSrc.z -= iCount * 8;
				vecSrc.y -= iCount * 4;
				vecSrc.x += iCount * 4;
			}
			else
			{
				if (iCount < 2)
					vecSrc.z -= iCount * 4;
				else vecSrc.z -= iCount * 8;
				vecSrc.y += iCount * 4;
				vecSrc.x += iCount * 4;
			}


			CPianoGunWave* pEnt = CPianoGunWave::Create(0, iType, vecSrc, m_pPlayer->pev->v_angle, ENT(m_pPlayer->pev), m_pPlayer->m_iTeam);

			if (pEnt)
			{
				pEnt->pev->velocity = gpGlobals->v_forward * 500;
			}
#endif
		}

		void ShootWave(int iType)
		{
#ifndef CLIENT_DLL
			const float flRadius = 360;
			const float flAngle = 360;

			UTIL_MakeVectors(m_pPlayer->pev->v_angle);
			Vector vecSrc = m_pPlayer->GetGunPosition();

			CPianoGunWave* pEnt = CPianoGunWave::Create(0, iType, vecSrc, m_pPlayer->pev->v_angle, ENT(m_pPlayer->pev), m_pPlayer->m_iTeam);
			RadiusAttack(iType,flRadius, flAngle, DMG_BULLET | DMG_NEVERGIB, m_pPlayer->pev, m_pPlayer);
#endif
		}

#ifndef CLIENT_DLL
		void RadiusAttack(int iType,float flRadius, float flAngleDegrees, int bitsDamageType, entvars_t* pevInflictor, CBasePlayer* pAttackPlayer)
		{
			CBaseEntity* pEntity = NULL;
			BOOL fHit = FALSE;

			TraceResult tr;
			UTIL_MakeVectors(pAttackPlayer->pev->v_angle);
			Vector vecEnd;
			Vector vecSrc = pAttackPlayer->GetGunPosition();

			Vector vecForward = gpGlobals->v_forward;
			Vector2D vecForward2D = vecForward.Make2D().Normalize();

			float c = cos(flAngleDegrees * 0.5 * M_PI / 180.0);
			bool bHit = false;

			while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, flRadius)) != NULL)
			{
				if (pEntity == pAttackPlayer)
					continue;

				if (pEntity->IsDormant())
					continue;

				if (pEntity->pev->takedamage == DAMAGE_NO)
					continue;

				if (!AngleCheck(pEntity, pAttackPlayer, vecSrc, vecForward, vecForward2D, flRadius, c, &tr))
					continue;

				pAttackPlayer->m_iWeaponVolume = 128;

				ClearMultiDamage();

				tr.iHitgroup = HITGROUP_CHEST;
				pEntity->TraceAttack(pAttackPlayer->pev, GetRadiusDamage(iType), (tr.vecEndPos - vecSrc).Normalize(), &tr, bitsDamageType);

				ApplyMultiDamage(pevInflictor, pAttackPlayer->pev);

				fHit = TRUE;

				if (pEntity->IsPlayer())
				{
					CBasePlayer* player = (CBasePlayer*)pEntity;

					if (player->m_bIsZombie)
					{
						CBasePlayer* player = dynamic_ent_cast<CBasePlayer*>(pEntity);
						if (player && player->m_bIsZombie)
						{

							player->m_flVelocityModifier = 1.0;
							if (!(player->m_iBuff & BUFF_IGNORE_KNOCKBACK))
							{
								Vector2D vecDir2D = (player->pev->origin.Make2D() - m_pPlayer->pev->origin.Make2D()).Normalize();

								player->pev->velocity.x = vecDir2D.x * 800;
								player->pev->velocity.y = vecDir2D.y * 800;
								player->pev->velocity.z = 300;
							}
						}
					}
				}
			}

			if (!fHit)
			{
				static int calced = 0;
				static float calcRes = 0.0f;

				if (!(calced & 1))
				{
					calced |= 1;
					calcRes = sqrt(512.0);
				}

				vecEnd = vecSrc + vecForward * (flRadius - calcRes);

				UTIL_TraceHull(vecSrc, vecEnd, dont_ignore_monsters, human_hull, pAttackPlayer->edict(), &tr);
			}
			//return fHit;
		}
#ifdef CLENT_DLL
		void DoRemoveDebuff(int iType, vec3_t flOrigin, CBasePlayer* iOwner)
		{
			CBaseEntity* pEntity = NULL;
			while ((pEntity = UTIL_FindEntityInSphere(pEntity, flOrigin, GetSkillsData(1))) != NULL)
			{
				if (pEntity->IsDormant())
					continue;
				if(!pEntity->IsPlayer())
					continue;

				CBasePlayer* player = (CBasePlayer*)pEntity;
				if (player->m_bIsZombie)
					continue;

				if (g_pGameRules->PlayerRelationship(iOwner, pEntity) != GR_TEAMMATE)
					continue;

				if (!iType)
				{
					if (player->pev->health + GetSkillsData(0) >= player->pev->max_health)
						player->pev->health = player->pev->max_health;
					else
						player->pev->health += GetSkillsData(0);
					char SzMsg[64];sprintf(SzMsg,"%s 给您恢复了%.1f 的血量", GetPlayerName(iOwner), GetSkillsData(0));
					ClientPrint(player->pev, HUD_PRINTTALK, "%s", SzMsg);
				}
				else
				{
					m_pPlayer->m_tLockBuffTime = invalid_time_point;
					m_pPlayer->ResetMaxSpeed();
				}

				



			}

		}
#endif

		float GetSkillsData(int m_iType)
		{
			float SzBuff;
			if (!m_iType)
			{
				float flRecoveryAmount;
				flRecoveryAmount = 30;
				if (g_pModRunning->DamageTrack() == DT_ZB)
					flRecoveryAmount = 100;
				else if (g_pModRunning->DamageTrack() == DT_ZBS)
					flRecoveryAmount = 300;
				SzBuff = flRecoveryAmount;

			}
			else
			{
				float flRemoverRadius;
				flRemoverRadius = 30;
				if (g_pModRunning->DamageTrack() == DT_ZB)
					flRemoverRadius = 100;
				else if (g_pModRunning->DamageTrack() == DT_ZBS)
					flRemoverRadius = 300;
				SzBuff = flRemoverRadius;
			}

			return SzBuff;
		}

		float GetRadiusDamage(int m_iType)
		{
			float flDamage;
			if (m_iType == 2)
			{
				flDamage = 60;
				if (g_pModRunning->DamageTrack() == DT_ZB)
					flDamage = 432;
				else if (g_pModRunning->DamageTrack() == DT_ZBS)
					flDamage = 600;
			}
			else if (m_iType == 3)
			{
				flDamage = 100;
				if (g_pModRunning->DamageTrack() == DT_ZB)
					flDamage = 700;
				else if (g_pModRunning->DamageTrack() == DT_ZBS)
					flDamage = 1000;
			}
			return flDamage;
		}
#endif
		private:
			bool m_bShouldDoSkills;
			bool m_bDoingPrimaryAttack;

			int m_iPrimaryAttackCount;
			int m_iSaveUserButton;
			int m_iSkillType;

			time_point_t m_iPrimaryAttackInterval;
			time_point_t m_flNextSkillTime;
			time_point_t m_flNextIdleThink;
};
LINK_ENTITY_TO_CLASS(weapon_pianogun, CPianoGun)
}