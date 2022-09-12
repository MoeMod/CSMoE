/*
mgsm.cpp
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
//#include "gamemode/mod_zb1.h"
#include "weapons/KnifeAttack.h"
#include "weapons/WeaponTemplate.hpp"
#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif


#ifndef CLIENT_DLL
	class CMGSMMissile : public CGrenade
	{
	public:
		int GetWeaponsId(void) { return WEAPON_MGSM; }
		void Spawn() override
		{
			Precache();

			pev->movetype = MOVETYPE_FLY;
			pev->solid = SOLID_TRIGGER;

			SET_MODEL(ENT(pev), "models/mgsm_missile.mdl");

			UTIL_SetSize(pev, g_vecZero, g_vecZero);
			UTIL_SetOrigin(pev, pev->origin);

			SetThink(&CMGSMMissile::MissileThink);
			SetTouch(&CMGSMMissile::MissileTouch);

			pev->nextthink = gpGlobals->time + 0.1s;

			MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
			WRITE_BYTE(TE_BEAMPARTICLE); //defined as 32
			WRITE_SHORT(this->entindex());  // short (entity:attachment to follow)
			WRITE_SHORT(m_iSmoke);// short (sprite index)
			WRITE_SHORT(m_iSmoke);// short (sprite index)
			WRITE_SHORT(m_iSmoke);// short (sprite index)
			WRITE_BYTE(255); //r
			WRITE_BYTE(255); //g
			WRITE_BYTE(255); //b
			WRITE_BYTE(0); //randomcolor
			WRITE_BYTE(100); //scale in 0.01's
			WRITE_BYTE(6); //freq(time of one cycle) in 0.01's
			WRITE_BYTE(3); //fadetime in 0.1s
			WRITE_BYTE(1);  //count
			WRITE_BYTE(0);  //random in org
			WRITE_BYTE(0);  //speed (if>0 do slow gravity)
			MESSAGE_END();
		}

		void Precache() override
		{
			PRECACHE_SOUND("weapons/mgsm_exp.wav");
			
			PRECACHE_MODEL("models/mgsm_missile.mdl");
			m_iExpEffect = PRECACHE_MODEL("sprites/ef_mgsmexplo.spr");
			m_iSmoke = PRECACHE_MODEL("sprites/smoke_ia.spr");
		}

		KnockbackData GetKnockBackData() override
		{
			return { 2000.0f, 1250.0f, 1000.0f, 1750.0f, 1.0f };
		}

		float GetDirectDamage() const
		{
			float flDamage = 60;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 6400;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 22450;
#endif
			return flDamage;
		}

		float GetExplodeDamage() const
		{
			float flDamage = 20;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 2000;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 2860;
#endif
			return flDamage;
		}

		void EXPORT MissileThink(void)
		{
			if (!IsInWorld())
			{
				SetThink(NULL);
				SetTouch(NULL);

				UTIL_Remove(this);
				return;
			}

			float flDamage = GetDirectDamage();

			entvars_t* pevOwner = VARS(pev->owner);
			TraceResult tr;

			pev->velocity = m_vecVelocity;

			if (m_bLockingTarget)
			{
				pev->origin = m_vecLockingPos;

				m_bLockingTarget = FALSE;
			}

			CBaseEntity* pEntity = NULL;
			while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, 2.5 * 39.37)) != NULL)
			{
				if (std::find(m_iTouchID.begin(), m_iTouchID.end(), pEntity->entindex()) != m_iTouchID.end())
					continue;

				if (pEntity->IsDormant())
					continue;

				if (pEntity->pev->takedamage == DAMAGE_NO)
					continue;

				if (pEntity->pev == VARS(pev->owner))
					continue;

				if (pEntity->IsPlayer())
				{
					UTIL_TraceLine(pev->origin, pev->origin + m_vecDirection * 50, missile, pev->owner, &tr);

					ClearMultiDamage();

					pEntity->TraceAttack(pevOwner, flDamage, m_vecDirection, &tr, DMG_BULLET | DMG_NEVERGIB);

					ApplyMultiDamage(pev, pevOwner);

					if (tr.pHit && tr.pHit == pEntity->edict())
					{
						RadiusDamage(pEntity);
					}

					m_iTouchID.push_back(pEntity->entindex());
					Vector vecDelta = pev->origin - pEntity->pev->origin;
					float yaw = g_engfuncs.pfnVecToYaw(vecDelta);
					g_engfuncs.pfnWalkMove(pEntity->edict(), yaw, vecDelta.Length(), WALKMOVE_NORMAL);
					break;
				}
			}

			pev->nextthink = gpGlobals->time + 0.1s;
		}

		void EXPORT MissileTouch(CBaseEntity* pOther)
		{
			if (!pOther)
				return;

			if (pOther->edict() == pev->owner)
				return;

			m_vecOrigin = pev->origin;

			Vector vecEnd = m_vecOrigin + pev->velocity.Normalize() * 80;
			TraceResult tr;

			UTIL_TraceLine(m_vecOrigin, vecEnd, dont_ignore_monsters, ENT(pev), &tr);

			if (tr.fStartSolid)
				return;

			pev->origin += m_vecDirection * 0.5 * 39.37;

			m_vecLockingPos = pev->origin;
			m_bLockingTarget = TRUE;

			pev->nextthink = gpGlobals->time;

			if (std::find(m_iTouchID.begin(), m_iTouchID.end(), pOther->entindex()) == m_iTouchID.end())
				RadiusDamage(pOther);
		}

		void RadiusDamage(CBaseEntity* pEnt)
		{
			float flDamage = GetDirectDamage();
			float flExplodeDamage = GetExplodeDamage();

			entvars_t* pevOwner = VARS(pev->owner);

			CBaseEntity* pEntity = NULL;

			while ((pEntity = UTIL_FindEntityInSphere(pEntity, m_vecOrigin, 1.75 * 39.37)) != NULL)
			{
				if (pEntity->IsDormant())
					continue;

				if (pEntity->pev->takedamage == DAMAGE_NO)
					continue;

				if (pEntity == pEnt)
					continue;

				if (pEntity->pev == pevOwner)
					continue;

				float flNewDamage = UTIL_CalcDamage(pev->origin, pEntity->pev->origin, flDamage, flExplodeDamage, 1.75 * 39.37);

				if (flNewDamage != 0.0f)
					pEntity->TakeDamage(pev, pevOwner, flNewDamage, DMG_GENERIC | DMG_EXPLOSION);
			}

			MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY, pev->origin);
			WRITE_BYTE(TE_TEMPSPRITE);
			WRITE_COORD(m_vecOrigin.x);
			WRITE_COORD(m_vecOrigin.y);
			WRITE_COORD(m_vecOrigin.z + 50.0);
			WRITE_SHORT(m_iExpEffect);
			WRITE_BYTE(15);
			WRITE_BYTE(255);
			WRITE_BYTE(30);
			MESSAGE_END();

			EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "weapons/mgsm_exp.wav", VOL_NORM, 0.25, 0, PITCH_NORM);
		}

		std::vector<int> m_iTouchID;
		BOOL m_bLockingTarget;
		Vector m_vecVelocity;
		Vector m_vecDirection;
		Vector m_vecLockingPos;
		Vector m_vecOrigin;
		int m_iSmoke;
		int m_iExpEffect;
	};
	LINK_ENTITY_TO_CLASS(mgsm_missile, CMGSMMissile)
#endif


    class CMGSM : public LinkWeaponTemplate< CMGSM,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_M249>::template type,
            TPrecacheEvent,
            TGetDamageDefault,
            TPrimaryAttackRifle,
            TFireRifle
    >
    {
    public:
    static constexpr const char *V_Model = "models/v_mgsm.mdl";
    static constexpr const char *P_Model = "models/p_mgsm.mdl";
    static constexpr const char *W_Model = "models/w_mgsm.mdl";
    static constexpr const char *EventFile = "events/mgsm.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_mgsm";
    static constexpr const char *AnimExtension = "m134";
    static constexpr int MaxClip = 350;
	enum
	{
		ANIM_IDLE1 = 0,
		ANIM_DRAW,
		ANIM_RELOAD,
		ANIM_SHOOT1,
		ANIM_SHOOT2,
	};
    static constexpr float PlayerMaxSpeed = 225;
    static constexpr float ArmorRatioModifier = 1.5f;
	static constexpr float PrimaryAttackWalkingMiniumSpeed = 140;
    static constexpr const auto &SpreadCalcNotOnGround = 0.5*A + 0.0365;
    static constexpr const auto &SpreadCalcWalking = 0.0775*A + 0.045;
    static constexpr const auto &SpreadCalcDefault = 0.03*A;

	static constexpr const auto& CycleTime = 0.1s;
    static constexpr int DamageDefault = 25;
    static constexpr int DamageZB = 250;
    static constexpr int DamageZBS = 500;
    static constexpr const auto &AccuracyCalc = (N * N * N / 190.0) + 0.385;
    static constexpr float AccuracyDefault = 0.2;
    static constexpr float AccuracyMax = 0.92;
    static constexpr float RangeModifier = 0.99;
    static constexpr auto BulletType = BULLET_PLAYER_556MM;
    /*static constexpr*/ int Penetration = 2;
    KnockbackData KnockBack = { 350.0f, 250.0f, 300.0f, 100.0f, 0.6f };

	KickBackData KickBackWalking = { 1.1f, 0.5f, 0.3f, 0.06f, 4.0f, 3.0f, 8 };
	KickBackData KickBackNotOnGround = { 1.8f, 0.65f, 0.45f, 0.125f, 5.0f, 3.5f, 8 };
	KickBackData KickBackDucking = { 0.75f, 0.325f, 0.25f, 0.025f, 3.5f, 2.5f, 9 };
	KickBackData KickBackDefault = { 0.8f, 0.35f, 0.3f, 0.03f, 3.85f, 2.72f, 7 };

#define WPNSTATE_MGSM_MISSILE_ON	1

	enum mgsm_launcher_e
	{
		ANIM_IDLE_A,
		ANIM_IDLE_B,
		ANIM_LAUNCHER_ON,
		ANIM_LAUNCHER_SHOOT,
	};

	int m_iEfHit;
	duration_t m_flChargeTime;
	time_point_t m_flTimeChargeStart;
	time_point_t m_flNextReload;

	void Spawn()
	{
		m_flNextReload = invalid_time_point;
		m_flTimeChargeStart = gpGlobals->time;
		m_flChargeTime = GetChargeTime();
		Base::Spawn();
	}

    int GetItemInfo(ItemInfo* p)
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "mgsm";
        p->iMaxAmmo1 = 350;
        p->pszAmmo2 = NULL;
        p->iMaxAmmo2 = -1;
        p->iMaxClip = 350;
        p->iSlot = 0;
        p->iPosition = 4;
        p->iId = m_iId = WEAPON_MGSM;
        p->iFlags = 0;
        p->iWeight = M249_WEIGHT;

        return 1;
    }

	void Precache()
	{
		PRECACHE_MODEL("sprites/muzzleflash192.spr");
		PRECACHE_MODEL("sprites/muzzleflash193.spr");
		PRECACHE_MODEL("sprites/muzzleflash194.spr");
		m_iEfHit = PRECACHE_MODEL("sprites/ef_mgsm_hit.spr");
		PRECACHE_MODEL("models/ef_mgsm_launcher.mdl");
	
		PRECACHE_SOUND("weapons/mgsm_gauge.wav");
		PRECACHE_SOUND("weapons/mgsm-2.wav");
		PRECACHE_SOUND("weapons/mgsm-1_exp1.wav");
		PRECACHE_SOUND("weapons/mgsm-1_exp2.wav");
		PRECACHE_SOUND("weapons/mgsm-1_exp3.wav");

		Base::Precache();
	}

	void PlayWeaponAnim(int iAnim, int iAnim2)
	{
		SendWeaponAnim(iAnim, UseDecrement() != FALSE);
#ifndef CLIENT_DLL
		MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->edict());
		WRITE_BYTE(28);
		WRITE_BYTE(m_iWeaponState);
		WRITE_BYTE(iAnim2);
		MESSAGE_END();
#endif
	}

	void UpdateItemInfo()
	{
		CBasePlayerWeapon::UpdateItemInfo();
#ifndef CLIENT_DLL
		MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->edict());
		WRITE_BYTE(28);
		WRITE_BYTE(m_iWeaponState);
		WRITE_BYTE(ANIM_LAUNCHER_ON);
		MESSAGE_END();
#endif
	}

	BOOL Deploy()
	{
		m_flNextReload = invalid_time_point;
		m_flAccuracy = 0.24;
		m_iShotsFired = 0;

		if (!m_iWeaponState && gpGlobals->time - m_flTimeChargeStart >= m_flChargeTime)
		{
			m_flTimeChargeStart = gpGlobals->time + 9999s;
			m_iWeaponState = WPNSTATE_MGSM_MISSILE_ON;
		}
		else
		{
			float delta = (gpGlobals->time - m_flTimeChargeStart) / 1s;
#ifndef CLIENT_DLL
			MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->edict());
			WRITE_BYTE(27);
			WRITE_COORD(delta);
			WRITE_SHORT(m_flChargeTime / 1s);
			MESSAGE_END();
#endif
		}

		if(m_iWeaponState == WPNSTATE_MGSM_MISSILE_ON)
			EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "weapons/mgsm_gauge.wav", VOL_NORM, 0.25, 0, PITCH_NORM);
			
		if (DefaultDeploy("models/v_mgsm.mdl", "models/p_mgsm.mdl", ANIM_DRAW, "m134", UseDecrement() != FALSE))
		{

			return TRUE;
		}
		return FALSE;
	}

	void ItemPostFrame()
	{
		if (m_fInReload && m_flNextReload != invalid_time_point && m_flNextReload < gpGlobals->time)
		{
			// complete the reload.
			int j = Q_min(350 - m_iClip, m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]);
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

		if (!m_iWeaponState && gpGlobals->time - m_flTimeChargeStart >= m_flChargeTime)
		{
			m_flTimeChargeStart = gpGlobals->time + 9999s;
			m_iWeaponState = WPNSTATE_MGSM_MISSILE_ON;

			EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "weapons/mgsm_gauge.wav", VOL_NORM, 0.25, 0, PITCH_NORM);

#ifndef CLIENT_DLL
			MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->edict());
			WRITE_BYTE(28);
			WRITE_BYTE(m_iWeaponState);
			WRITE_BYTE(ANIM_LAUNCHER_ON);
			MESSAGE_END();
#endif

		}

		if (m_fInReload)
			return;

		return CBasePlayerWeapon::ItemPostFrame();
	}


#ifndef CLIENT_DLL
	void FireBullet3CallBack(int iPenetration, Vector vecDir, TraceResult* pTrace)
	{
		int iOriginalPenetration = 2;

		if (iOriginalPenetration == iPenetration)
		{
			if (CBaseEntity* pEntity = gMultiDamage.pEntity)
			{
				if (pEntity->IsPlayer() || pEntity->Classify() == CLASS_PLAYER_ALLY)
				{
					float flBuffDamage = GetBuffDamage();
					float flExplodeDamage = GetExplodeDamage();
					switch (pEntity->m_LastHitGroup) {
					case HITGROUP_GENERIC:
						break;
					case HITGROUP_HEAD:
						flBuffDamage *= 3;
						break;
					case HITGROUP_CHEST:
					case HITGROUP_STOMACH:
						flBuffDamage *= 1.5;
						break;
					case HITGROUP_LEFTARM:
					case HITGROUP_RIGHTARM:
						flBuffDamage *= 1.0;
						break;
					case HITGROUP_LEFTLEG:
					case HITGROUP_RIGHTLEG:
						flBuffDamage *= 0.75;
						break;
					case HITGROUP_SHIELD:
						flBuffDamage = 0;
						break;
					default:
						break;
					}

					if (pEntity->IsPlayer())
					{
						if (g_pGameRules->PlayerRelationship(m_pPlayer, pEntity) != GR_TEAMMATE)
						{
							pEntity->m_pBuffAttacker = m_pPlayer->pev;
							pEntity->m_flHealthDecreaseAmount = flBuffDamage;
							pEntity->m_iHealthDecreaseCount = 6;
							pEntity->m_flHealthDecreaseInterval = 1.0s;
						}
					}
					else
					{
						pEntity->m_pBuffAttacker = m_pPlayer->pev;
						pEntity->m_flHealthDecreaseAmount = flBuffDamage;
						pEntity->m_iHealthDecreaseCount = 6;
						pEntity->m_flHealthDecreaseInterval = 1.0s;
					}

					MESSAGE_BEGIN(MSG_ALL, gmsgMPToCL, NULL, pEntity->pev);
					WRITE_BYTE(4);
					WRITE_SHORT(pEntity->entindex());
					WRITE_BYTE(4);
					MESSAGE_END();

					MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY, pEntity->pev->origin);
					WRITE_BYTE(TE_TEMPSPRITE);
					WRITE_COORD(pEntity->pev->origin.x);
					WRITE_COORD(pEntity->pev->origin.y);
					WRITE_COORD(pEntity->pev->origin.z);
					WRITE_SHORT(m_iEfHit);
					WRITE_BYTE(6);
					WRITE_BYTE(255);
					WRITE_BYTE(30);
					MESSAGE_END();

					pEntity->TakeDamage(this->pev, m_pPlayer->pev, flExplodeDamage, DMG_EXPLOSION);
				}
			}
		}

	}
#endif

	void Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
	{
		m_bDelayFire = true;
		m_iShotsFired++;
		m_flAccuracy = (m_iShotsFired * m_iShotsFired * m_iShotsFired) / 190.0 + 0.385;

		if (m_flAccuracy > 0.92)
			m_flAccuracy = 0.92;

		if (m_iClip <= 0)
		{
			if (m_fFireOnEmpty)
			{
				PlayEmptySound();

				m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2s;
			}

#ifndef CLIENT_DLL
			if (TheBots)
				TheBots->OnEvent(EVENT_WEAPON_FIRED_ON_EMPTY, m_pPlayer);
#endif

			return;
		}

		m_iClip--;
		m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

		UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

		m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
		m_pPlayer->m_iWeaponVolume = BRIGHT_GUN_FLASH;

		Vector vecSrc = m_pPlayer->GetGunPosition();
		Vector vecDir;

		float flDamage = GetDamage();
		vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 2, BULLET_PLAYER_556MM, flDamage, 0.97, m_pPlayer->pev, false, m_pPlayer->random_seed);

#ifndef CLIENT_DLL
		PLAYBACK_EVENT_FULL(FEV_GLOBAL, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, static_cast<int>(m_pPlayer->pev->punchangle.x * 100), static_cast<int>(m_pPlayer->pev->punchangle.y * 100), FALSE, FALSE);
#endif

		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
#ifndef CLIENT_DLL
		if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", 0, 0);
#endif
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;

		if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			KickBack(1.8f, 0.65f, 0.45f, 0.125f, 5.0f, 3.5f, 8);
		else if (m_pPlayer->pev->velocity.Length2D() > 0)
			KickBack(1.1f, 0.5f, 0.3f, 0.06f, 4.0f, 3.0f, 8);
		else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
			KickBack(0.75f, 0.325f, 0.25f, 0.025f, 3.5f, 2.5f, 9);
		else
			KickBack(0.8f, 0.35f, 0.3f, 0.03f, 3.85f, 2.72f, 7);
	}

	void SecondaryAttack(void) override
	{
		if (m_iWeaponState != WPNSTATE_MGSM_MISSILE_ON)
			return;

#ifndef CLIENT_DLL
		MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->edict());
		WRITE_BYTE(28);
		WRITE_BYTE(m_iWeaponState);
		WRITE_BYTE(ANIM_LAUNCHER_SHOOT);
		MESSAGE_END();
#endif

		m_iWeaponState = 0;
		m_flTimeChargeStart = gpGlobals->time;

#ifndef CLIENT_DLL
		MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->edict());
		WRITE_BYTE(27);
		WRITE_COORD(0.0);
		WRITE_SHORT(m_flChargeTime / 1s);
		MESSAGE_END();
#endif

		EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "weapons/mgsm-2.wav", VOL_NORM, 0.25, 0, PITCH_NORM);


#ifndef CLIENT_DLL
		UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
		Vector vecSrc = m_pPlayer->GetGunPosition();
		Vector vecAngles = UTIL_VecToAngles(gpGlobals->v_forward);
		CMGSMMissile* pMissile = static_cast<CMGSMMissile*>(CBaseEntity::Create("mgsm_missile", vecSrc, vecAngles, ENT(m_pPlayer->pev)));
		if (pMissile)
		{
			pMissile->m_vecVelocity = pMissile->pev->velocity = gpGlobals->v_forward * 1000;
			pMissile->m_iTeam = m_pPlayer->m_iTeam;
			pMissile->m_vecDirection = pMissile->pev->velocity.Normalize();
		}
#endif

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;

		if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			KickBack(0.2, 0.0, 0.05, 0.0, 0.5, 0.0, 0);
		else if (m_pPlayer->pev->velocity.Length2D() > 0)
			KickBack(0.1, 0.0, 0.05, 0.0, 0.5, 0.0, 0);
		else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
			KickBack(0.1, 0.0, 0.05, 0.0, 0.5, 0.0, 0);
		else
			KickBack(0.1, 0.0, 0.05, 0.0, 0.5, 0.0, 0);
	}

	void Reload(void)
	{
		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			return;

		if (DefaultReload(350, ANIM_RELOAD, 2.5s, 1))
		{
#ifndef CLIENT_DLL         
			m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
			m_flNextReload = gpGlobals->time + 2.5s;
		}
	}

	void WeaponIdle(void)
	{
		ResetEmptySound();
		m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;

		PlayWeaponAnim(ANIM_IDLE1, ANIM_IDLE_B);
	}

	float GetBuffDamage() const
	{
		float flDamage = 1;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 25;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 50;
#endif
		return flDamage;
	}

	float GetExplodeDamage() const
	{
		float flDamage = 4;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 90;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 180;
#endif
		return flDamage;
	}

	duration_t GetChargeTime()
	{
		duration_t time = 5s;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			time = 10s;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			time = 10s;
#endif
		return time;
	}

#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_mgsm" , 200 }; }
#endif
    float GetArmorRatioModifier() override { return 1.5; }
    WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 6, 3, 140 }; }

};
LINK_ENTITY_TO_CLASS(weapon_mgsm, CMGSM)
}