/*
pianogunex.cpp
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

	struct PianoGunEx_WeaponData
	{
		static constexpr const char* V_Model = "models/v_pianogunex.mdl";
		static constexpr const char* P_Model = "models/p_pianogunex.mdl";
		static constexpr const char* W_Model = "models/w_pianogunex.mdl";
		static constexpr const char* EventFile = "events/pianogunex.sc";
		static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
		static constexpr const char* ClassName = "weapon_pianogunex";
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
			ANIM_SHOOTC3,
			ANIM_RELOAD,
			ANIM_DRAW
		};
		static constexpr float ArmorRatioModifier = 1.85f;
		static constexpr const auto& AccuracyCalc = (N * N * N / 220.0) + 0.3;
		static constexpr float AccuracyDefault = 0.25;
		static constexpr float AccuracyMax = 0.94;
		static constexpr float RangeModifier = 0.82;
		static constexpr auto BulletType = BULLET_PLAYER_762MM;
		static constexpr int Penetration = 2;
		KnockbackData KnockBack = { 500.0f, 250.0f, 450.0f, 300.0f, 0.7f };
	};
	class CPianoGunEx : public LinkWeaponTemplate< CPianoGunEx,
		TGeneralData,
		BuildTGetItemInfoFromCSW<WEAPON_M249>::template type,
		TPrecacheEvent,
		TDeployDefault
	>, public PianoGunEx_WeaponData
	{
	public:



#ifndef CLIENT_DLL
		WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_pianogun_energycell" , 100 }; }
#endif
		WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 4, 4, 140 }; }
		enum pianogunex_crosshairstate_e
		{
			PIANOGUNEX_CRITICAL = 16,
			PIANOGUNEX_HEAL,
			PIANOGUNEX_NULL,
		};

		
		void Precache(void) override
		{
			Base::Precache();
			PRECACHE_MODEL("sprites/ef_pianogun_note1.spr");
			PRECACHE_MODEL("sprites/ef_pianogun_note2.spr");
			PRECACHE_MODEL("sprites/ef_pianogun_note3.spr");
			PRECACHE_MODEL("sprites/ef_pianogun_note4.spr");

			PRECACHE_MODEL("sprites/muzzleflash218.spr");
			PRECACHE_MODEL("sprites/muzzleflash219.spr");
			PRECACHE_MODEL("sprites/muzzleflash220.spr");
			PRECACHE_MODEL("sprites/muzzleflash221.spr");
			PRECACHE_MODEL("sprites/muzzleflash222.spr");
			PRECACHE_MODEL("sprites/muzzleflash223.spr");
		}

		void Spawn(void) override
		{
			pev->classname = MAKE_STRING("weapon_pianogunex");

			Precache();
			m_iId = WEAPON_PIANOGUNEX;
			SET_MODEL(ENT(pev), "models/w_pianogunex.mdl");
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
			p->iId = m_iId = WEAPON_PIANOGUNEX;
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
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 18s;
				SendWeaponAnim(ANIM_IDLE2, UseDecrement() != FALSE);
				pev->iuser1 = 0;
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
			bool IsSkill = (m_pPlayer->m_flPlayerBuffData && m_flSkillTime < gpGlobals->time) ? TRUE : FALSE;
			return IsSkill?(220 + m_pPlayer->m_flPlayerBuffData):220;
		}

		BOOL Deploy(void) override
		{
			pev->iuser1 = 0;
			m_iSaveUserButton = 0;
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
				case 2:
				{
					m_iSaveUserButton = 0;
					m_bShouldDoSkills = true;
					m_iSkillType = 7;
					//CriticalAttack
					m_flNextSkillTime = gpGlobals->time + PianoGunPreparationBeforeSkills;
					sprintf(SzMsg, "Press Button:Left & -Left-Right-Left");
					break;
				}
				case 3:
				{
					m_iSaveUserButton = 4;
					sprintf(SzMsg, "Press Button:Left & -Right-Left");
					break;
				}
				case 4:
				{
					m_iSaveUserButton = 0;
					m_bShouldDoSkills = true;
					m_iSkillType = 6;
					//Recovery
					m_flNextSkillTime = gpGlobals->time + PianoGunPreparationBeforeSkills;
					sprintf(SzMsg, "Press Button:Left & -Right-Left-Left");
					break;
				}
				default:
				{
					m_iSaveUserButton = 0;
					sprintf(SzMsg, "Invalid Button Combination.");
					break;
				}
				}
				ClientPrint(m_pPlayer->pev, HUD_PRINTTALK, "%s", SzMsg);
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

			ShootWave(8);

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
			PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), TRUE, FALSE);
			SendWeaponAnim(ANIM_SHOOTB, UseDecrement() != FALSE);
			pev->iuser1 = 0;

			char SzMsg[128]{};
			switch (m_iSaveUserButton)
			{
			case 0:
			{
				m_iSaveUserButton = 3;
				sprintf(SzMsg, "Press Button:Right & -Right");
				break;
			}
			case 1:
			{
				m_iSaveUserButton = 2;
				sprintf(SzMsg, "Press Button:Right & -Left-Right");
				break;
			}
			case 2:
			{
				m_iSaveUserButton = 0;
				m_bShouldDoSkills = true;
				m_iSkillType = 5;
				//Speedup
				m_flNextSkillTime = gpGlobals->time + PianoGunPreparationBeforeSkills;
				sprintf(SzMsg, "Press Button:Right & -Left-Right-Right");
				break;
			}
			default:
			{
				m_iSaveUserButton = 0;
				sprintf(SzMsg, "Invalid Button Combination.");
				break;
			}
			}
			ClientPrint(m_pPlayer->pev, HUD_PRINTTALK, "%s", SzMsg);


		}


		void ItemPostFrame(void) override
		{
			if (m_iPrimaryAttackInterval > invalid_time_point && m_iPrimaryAttackInterval < gpGlobals->time)
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
						ShootNote(4, m_iPrimaryAttackCount);
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

					if (m_iSkillType == 5)
					{
						ShootNote(m_iSkillType, 0);

						pev->nextthink = gpGlobals->time + 0.1s;
						SetThink(&CPianoGunEx::BuffThink);
						m_flSkillTime = gpGlobals->time + 0.5s;

						SendWeaponAnim(ANIM_SHOOTC1, UseDecrement() != FALSE);
						pev->iuser1 = 0;

						m_CrossHairState = PIANOGUNEX_NULL;

					}
					else if (m_iSkillType == 6)
					{

						ShootNote(m_iSkillType, 0);

						pev->nextthink = gpGlobals->time + 0.1s;
						SetThink(&CPianoGunEx::BuffThink);
						m_flSkillTime = gpGlobals->time + 1.5s;

						SendWeaponAnim(ANIM_SHOOTC2, UseDecrement() != FALSE);
						pev->iuser1 = 0;

						m_CrossHairState = PIANOGUNEX_HEAL;
#ifdef CLINET_DLL
						CPianoGunWave* pEnt = CPianoGunWave::Create(1, 10, m_pPlayer->pev->origin, m_pPlayer->pev->v_angle, ENT(m_pPlayer->pev), m_pPlayer->m_iTeam);
#endif

					}
					else if (m_iSkillType == 7)
					{

						ShootNote(m_iSkillType, 0);

						pev->nextthink = gpGlobals->time + 0.1s;
						SetThink(&CPianoGunEx::BuffThink);
						m_flSkillTime = gpGlobals->time + 3.0s;

						SendWeaponAnim(ANIM_SHOOTC3, UseDecrement() != FALSE);
						pev->iuser1 = 0;

						m_CrossHairState = PIANOGUNEX_CRITICAL;
					}
					m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + PianoGunAttackAnimTime;
					m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0s;
#ifndef CLIENT_DLL
					if (m_CrossHairState != m_CrossHairStateStored)
					{
						MESSAGE_BEGIN(MSG_ONE, gmsgSpecialCrossHair, NULL, m_pPlayer->pev);
						WRITE_BYTE(5);
						WRITE_BYTE(m_CrossHairState);
						MESSAGE_END();

						m_CrossHairStateStored = m_CrossHairState;
					}
#endif
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
			ShootNote(4, m_iPrimaryAttackCount);
			--m_iClip;

			int flags;
#ifdef CLIENT_WEAPONS
			flags = FEV_NOTHOST;
#else
			flags = 0;
#endif

			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
			PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), TRUE, FALSE);

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
			Vector vecSrc = m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 20 + gpGlobals->v_right * 8 + gpGlobals->v_up * -10;
			vecSrc.z -= iCount * 8;


			CPianoGunWave* pEnt = CPianoGunWave::Create(1, iType, vecSrc, m_pPlayer->pev->v_angle, ENT(m_pPlayer->pev), m_pPlayer->m_iTeam);

			if (pEnt)
			{
				pEnt->pev->velocity = gpGlobals->v_forward * 1200;
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

			CPianoGunWave* pEnt = CPianoGunWave::Create(1, iType, vecSrc, m_pPlayer->pev->v_angle, ENT(m_pPlayer->pev), m_pPlayer->m_iTeam);
			RadiusAttack(iType, flRadius, flAngle, DMG_BULLET | DMG_NEVERGIB, m_pPlayer->pev, m_pPlayer);
#endif
		}
		void EXPORT BuffThink(void)
		{
			pev->nextthink = gpGlobals->time + 0.5s;

#ifndef CLIENT_DLL
			if (m_flSkillTime < gpGlobals->time)
			{
				m_CrossHairState = m_CrossHairStateStored = PIANOGUNEX_NULL;
				MESSAGE_BEGIN(MSG_ONE, gmsgSpecialCrossHair, NULL, m_pPlayer->pev);
				WRITE_BYTE(5);
				WRITE_BYTE(m_CrossHairState);
				MESSAGE_END();

				SetThink(NULL);
				return;
			}

			CBaseEntity* pOwner = CBaseEntity::Instance(pev->owner);
			CBasePlayer* pOwnerPlayer = nullptr;
			if (pOwner && pOwner->IsPlayer())
				pOwnerPlayer = static_cast<CBasePlayer*>(pOwner);

			if (!pOwnerPlayer)
				return;

			char SzMsg[64];
			CBaseEntity* pEntity = NULL;


			while ((pEntity = UTIL_FindEntityInSphere(pEntity, pOwnerPlayer->pev->origin, GetSkillData(m_iSkillType - 5, 0))) != NULL)
			{
				if (pEntity->IsDormant())
					continue;

				if (!pEntity->IsPlayer())
					continue;

				CBasePlayer* Teammate = (CBasePlayer*)pEntity;

				if (Teammate->m_bIsZombie)
					continue;

				if (g_pGameRules->PlayerRelationship(pOwnerPlayer, Teammate) != GR_TEAMMATE)
					continue;

				switch (m_iSkillType)
				{
					//加速
				case 5:
				{
					Teammate->m_flPlayerBuffData = GetSkillData(0, 1);
					Teammate->ResetMaxSpeed();

					//sprintf(SzMsg, "%s 给您恢复了%.1f 的Speed", STRING(pOwnerPlayer->pev->netname), GetSkillData(0, 1));
					//ClientPrint(Teammate->pev, HUD_PRINTTALK, "%s", SzMsg);
					break;

				}
				//回血
				case 6:
				{

					if (Teammate->pev->health + GetSkillData(1, 1) >= Teammate->pev->max_health)
						Teammate->pev->health = Teammate->pev->max_health;
					else
						Teammate->pev->health += GetSkillData(1, 1);

					//sprintf(SzMsg, "%s 给您恢复了%.1f 的血量", STRING(pOwnerPlayer->pev->netname), GetSkillData(1, 1));
					//ClientPrint(Teammate->pev, HUD_PRINTTALK, "%s", SzMsg);
					break;

				}
				//暴击
				case 7:
				{
					Teammate->m_flPlayerBuffData = GetSkillData(2, 1);

					//sprintf(SzMsg, "%s 为您提升了%.2f 的伤害", STRING(pOwnerPlayer->pev->netname), GetSkillData(2, 1));
					//ClientPrint(Teammate->pev, HUD_PRINTTALK, "%s", SzMsg);
					break;
				}
				default:break;
				}
			}
#endif
		}
		float GetSkillData(int iSkill, int iType)
		{
#ifndef CLIENT_DLL
			//data
			float flVar1 = 0.0f;
			float flVar2 = 0.0f;
			//range

			//Speedup
			if (!iSkill)
			{
				flVar1 = 0.0f;
				if (g_pModRunning->DamageTrack() == DT_ZB)
					flVar1 = 20.0f;
				else if (g_pModRunning->DamageTrack() == DT_ZBS)
					flVar1 = 35.0f;

				flVar2 = 175.0f;
			}
			//Recovery
			else if (iSkill == 1)
			{
				flVar1 = 5.0f;
				if (g_pModRunning->DamageTrack() == DT_ZB)
					flVar1 = 50.0f;
				else if (g_pModRunning->DamageTrack() == DT_ZBS)
					flVar1 = 100.0f;

				flVar2 = 150.0f;
			}
			//Powerup
			else if (iSkill == 2)
			{
				flVar1 = 1.0f;
				if (g_pModRunning->DamageTrack() == DT_ZB)
					flVar1 = 1.5f;
				else if (g_pModRunning->DamageTrack() == DT_ZBS)
					flVar1 = 2.5f;

				flVar2 = 120.0f;
			}
			if (iType)
				return flVar1;
			else
				return flVar2;
#endif
		}


#ifndef CLIENT_DLL
		int TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
		{
			auto pAttack = dynamic_cast<CBasePlayer*>(CBasePlayer::Instance(pevAttacker));

			if (!pAttack)
				return 0;

			if (pAttack->IsPlayer() && !pAttack->m_bIsZombie/* && bitsDamageType & DMG_BULLET*/)
			{
				float flDamageRate = pAttack->m_flPlayerBuffData;
				if (RANDOM_LONG(0, 100) < flDamageRate * 10)
				{
					flDamage *= flDamageRate;
				}	
			}

			return CBaseEntity::TakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType);
		}
		void RadiusAttack(int iType, float flRadius, float flAngleDegrees, int bitsDamageType, entvars_t* pevInflictor, CBasePlayer* pAttackPlayer)
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

					if (player->m_bIsZombie && player->m_ZombieClass)
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
			else if (m_iType == 8)
			{
				flDamage = 60;
				if (g_pModRunning->DamageTrack() == DT_ZB)
					flDamage = 432;
				else if (g_pModRunning->DamageTrack() == DT_ZBS)
					flDamage = 600;
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
		int m_iActiveSkill;

		time_point_t m_iPrimaryAttackInterval;
		time_point_t m_flNextSkillTime;
		time_point_t m_flSkillTime;
		pianogunex_crosshairstate_e m_CrossHairState;
		pianogunex_crosshairstate_e m_CrossHairStateStored;
	};
	LINK_ENTITY_TO_CLASS(weapon_pianogunex, CPianoGunEx)
}