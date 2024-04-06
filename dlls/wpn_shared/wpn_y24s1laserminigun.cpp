/*
laserminigun.cpp
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


#ifndef CLIENT_DLL
#include "gamemode/mod_zb1.h"

namespace sv {
	class CY24S1LasermingunCharge : public CBaseEntity
	{
	public:

		KnockbackData GetKnockBackData() override { return { 350.0f, 250.0f, 300.0f, 100.0f, 0.6f }; }
		float GetArmorRatioModifier() { return 1.65; }
		int GetWeaponsId(void) { return WEAPON_Y24S1LASERMINIGUN; }

		static CY24S1LasermingunCharge* Create(edict_t* pentOwner)
		{
			edict_t* pent = CREATE_NAMED_ENTITY(MAKE_STRING("y24s1laserminiguncharge"));

			if (FNullEnt(pent))
			{
				ALERT(at_console, "NULL Ent in Create!\n");
				return NULL;
			}

			CY24S1LasermingunCharge* pAmmo = (CY24S1LasermingunCharge*)Instance(pent);

			if (pAmmo)
			{
				pAmmo->pev->owner = pentOwner;
				pAmmo->Spawn();
			}

			return pAmmo;
		}

		void Spawn() override
		{
			Precache();

			SetThink(&CY24S1LasermingunCharge::EnergyBallThink);
			
			SET_MODEL(this->edict(), "sprites/ef_y24s1laserminigun_charge_blue.spr");

			pev->rendermode = kRenderTransAdd;
			pev->renderamt = 255;
			pev->solid = SOLID_NOT;
			pev->movetype = MOVETYPE_FOLLOW;
			pev->aiment = pev->owner;
			pev->body = 1;

			pev->animtime = gpGlobals->time;
			pev->framerate = 30.0;
			pev->scale = EnergyBallSprScale * 0.001;
			pev->frame = 0;
			m_flNumFrames = 10.0;

			UTIL_SetSize(pev, g_vecZero, g_vecZero);

			pev->nextthink = gpGlobals->time + 0.1s;
		}

		void SetState(int iState)
		{
			if (iState == MaxConsumeLevel)
			{
				SET_MODEL(this->edict(), "sprites/ef_y24s1laserminigun_charge_purple.spr");
		
				pev->scale = FullEnergyBallSprScale * 0.001;

			}
			else
			{
				SET_MODEL(this->edict(), "sprites/ef_y24s1laserminigun_charge_blue.spr");

				pev->scale = EnergyBallSprScale * 0.001;
			}

			UTIL_SetSize(pev, g_vecZero, g_vecZero);
		}

		void EXPORT EnergyBallThink()
		{
			pev->nextthink = gpGlobals->time + 0.02s;

			pev->frame += pev->framerate * gpGlobals->frametime / 1s;

			if (pev->frame > m_flNumFrames)
				pev->frame = fmod(pev->frame, m_flNumFrames);
		}

		void Precache() override
		{
			PRECACHE_MODEL("sprites/ef_y24s1laserminigun_charge_purple.spr");
			PRECACHE_MODEL("sprites/ef_y24s1laserminigun_charge_blue.spr");
		}

		CBasePlayer* m_pOwner;
		int m_iChargeTime;
		float m_flNumFrames;

	private:
		static constexpr int MaxConsumeLevel = 4;
		static constexpr int EnergyBallSprScale = 130;
		static constexpr int FullEnergyBallSprScale = 140;

	};

	LINK_ENTITY_TO_CLASS(y24s1laserminiguncharge, CY24S1LasermingunCharge)

	class CY24S1LasermingunRange : public CBaseEntity
	{
	public:
		CY24S1LasermingunRange()
		{
			m_iState = 0;
		}

		KnockbackData GetKnockBackData() override { return { 350.0f, 250.0f, 300.0f, 100.0f, 0.6f }; }
		float GetArmorRatioModifier() { return 1.65; }
		int GetWeaponsId(void) { return WEAPON_Y24S1LASERMINIGUN; }

		void Spawn() override
		{
			Precache();

			SET_MODEL(this->edict(), "models/ef_y24s1laserminigun_range.mdl");

			pev->rendermode = kRenderTransAdd;
			pev->renderamt = 255;
			pev->solid = SOLID_NOT;
			pev->movetype = MOVETYPE_NOCLIP;
	
			UTIL_SetSize(pev, g_vecZero, g_vecZero);

			SetThink(&CY24S1LasermingunRange::RangeThink);

			pev->nextthink = gpGlobals->time + 0.1s;
		}

		void EXPORT RangeThink()
		{
			pev->nextthink = gpGlobals->time + 0.1s;

			if (!m_pOwner || (!m_pOwner->IsAlive() || m_pOwner->m_bIsZombie))
				Explode();
		
		}

		void SetOrigin(Vector origin)
		{
			pev->origin = origin;
		}

		void Precache() override
		{
			PRECACHE_MODEL("models/ef_y24s1laserminigun_range.mdl");

			m_iExpSpr[0] = PRECACHE_MODEL("sprites/ef_y24s1laserminigun_rainbow.spr");
			m_iExpSpr[1] = PRECACHE_MODEL("sprites/ef_y24s1laserminigun_explo.spr");
		}

		void Explode()
		{
			Vector vecSrc = pev->origin;
			Vector vecDirection;
			if (m_iState == MaxConsumeLevel)
			{
				MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
				WRITE_BYTE(TE_TEMPSPRITE);
				WRITE_COORD(vecSrc.x);
				WRITE_COORD(vecSrc.y);
				WRITE_COORD(vecSrc.z + FullExploSprZoffset);
				WRITE_SHORT(m_iExpSpr[0]);
				WRITE_BYTE(FullExploSprScale);
				WRITE_BYTE(255);
				WRITE_BYTE(FullExploSprFrameRate);
				MESSAGE_END();

				UTIL_EMIT_SOUND(this->edict(), CHAN_ITEM, "weapons/y24s1laserminigun-charge_exp2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

			}
			else
			{
				MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
				WRITE_BYTE(TE_TEMPSPRITE);
				WRITE_COORD(vecSrc.x);
				WRITE_COORD(vecSrc.y);
				WRITE_COORD(vecSrc.z + ExploSprZoffset);
				WRITE_SHORT(m_iExpSpr[1]);
				WRITE_BYTE(ExploSprScale);
				WRITE_BYTE(255);
				WRITE_BYTE(ExploSprFrameRate);
				MESSAGE_END();

				UTIL_EMIT_SOUND(this->edict(), CHAN_ITEM, "weapons/y24s1laserminigun-charge_exp1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

			}

			entvars_t* pevOwner = VARS(pev->owner);
			TraceResult tr;
			CBaseEntity* pEntity = NULL;
			float flRadius;
			
			flRadius = ChargingAttackRadius * 39.37;
	
			while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, flRadius)) != NULL)
			{
				if (pEntity->pev->takedamage == DAMAGE_NO)
					continue;

				if (pEntity->pev->health <= 0)
					continue;

				if (ENT(pEntity->pev) == pev->owner)
					continue;

				if (g_pGameRules->PlayerRelationship(m_pOwner, pEntity) == GR_TEAMMATE)
					continue;

				UTIL_TraceLine(vecSrc, pEntity->BodyTarget(vecSrc), dont_ignore_monsters, edict(), &tr);

				if (tr.flFraction < 1.0 && tr.pHit != pEntity->edict())
					continue;

				pEntity->m_LastHitGroup = HITGROUP_CHEST;
				pEntity->TakeDamage(pev, pevOwner, GetExpDamage(), DMG_GENERIC);

				if (pEntity->IsPlayer())
				{
					CBasePlayer* player = dynamic_cast<CBasePlayer*>(pEntity);

					if (player->m_bIsZombie)
					{
						auto strategy = dynamic_cast<CPlayerModStrategy_ZB1*>(player->m_pModStrategy.get());
						if (strategy)
						{
							if(m_iState == MaxConsumeLevel)
								strategy->SetStunSpeedTime(FullChargingAttackFixedTime, FullChargingAttackFixedSpeed);
							else
								strategy->SetStunSpeedTime(ChargingAttackFixedTime, ChargingAttackFixedSpeed);
						}
					}
					else
					{
						//support other mode
						if (m_iState == MaxConsumeLevel)
						{
							player->m_tLockBuffTime = gpGlobals->time + FullChargingAttackFixedTime;
							player->m_flLockSpeed = FullChargingAttackFixedSpeed;
							player->ResetMaxSpeed();
						}
						else
						{
							player->m_tLockBuffTime = gpGlobals->time + ChargingAttackFixedTime;
							player->m_flLockSpeed = ChargingAttackFixedSpeed;
							player->ResetMaxSpeed();
						}
					}
				}

				vecDirection = (pEntity->pev->origin - pevOwner->origin).Normalize();
				if (m_iState == MaxConsumeLevel)
				{
					if (g_pModRunning->DamageTrack() == DT_ZBS)
						UTIL_AddKickRateBoost(pEntity, vecDirection.Make2D(), 1.0, FullChargingAttackKnockBackXYScenario, FullChargingAttackKnockBackZScenario);
					else
						UTIL_AddKickRateBoost(pEntity, vecDirection.Make2D(), 1.0, FullChargingAttackKnockBackXYZombie, FullChargingAttackKnockBackZZombie);
				}
			}


			UTIL_Remove(this);
		}
	
		CBasePlayer* m_pOwner;
		int m_iState;

	private:
		int m_iExpSpr[2];

		static constexpr int MaxConsumeLevel = 4;
		static constexpr float ChargingAttackDamage = 25.0;
		static constexpr float ChargingAttackDamageZombie = 2960.0;
		static constexpr float ChargingAttackDamageScenario = 1000.0;
		static constexpr float ChargingAttackFixedSpeed = 150.0;
		static constexpr const auto& ChargingAttackFixedTime = 1.0s;
		static constexpr float FullChargingAttackDamage = 25.0;
		static constexpr float FullChargingAttackDamageZombie = 5600.0;
		static constexpr float FullChargingAttackDamageScenario = 2100.0;
		static constexpr float FullChargingAttackFixedSpeed = 140.0;
		static constexpr const auto& FullChargingAttackFixedTime = 2.0s;
		static constexpr float FullChargingAttackKnockBackXYZombie = 545.0;
		static constexpr float FullChargingAttackKnockBackZZombie = 345.0;
		static constexpr float FullChargingAttackKnockBackXYScenario = 145.0;
		static constexpr float FullChargingAttackKnockBackZScenario = 145.0;
		static constexpr float ChargingAttackRadius = 80.0;
		static constexpr float ChargingAttackScaleRatio = 0.35;

		static constexpr int ExploSprScale = 17;
		static constexpr float ExploSprZoffset = 38.0;
		static constexpr int ExploSprFrameRate = 60;
		

		static constexpr int FullExploSprScale = 24;
		static constexpr float FullExploSprZoffset = 35.0;
		static constexpr int FullExploSprFrameRate = 60;


		float GetExpDamage()
		{
			if (m_iState == MaxConsumeLevel)
			{
				if (g_pModRunning->DamageTrack() == DT_ZB)
					return FullChargingAttackDamageZombie;
				else if (g_pModRunning->DamageTrack() == DT_ZBS)
					return FullChargingAttackDamageScenario;
				return FullChargingAttackDamage;
			}
			else
			{	
				if (g_pModRunning->DamageTrack() == DT_ZB)
					return ChargingAttackDamageZombie;
				else if (g_pModRunning->DamageTrack() == DT_ZBS)
					return ChargingAttackDamageScenario;
				return ChargingAttackDamage;
			}
		}

		float GetRadius()
		{
			float flRadius;
			if (m_iState > 1)
			{
				flRadius = 1.5f;
				if (g_pModRunning->DamageTrack() == DT_ZB)
					flRadius = 2.0f;
				else if (g_pModRunning->DamageTrack() == DT_ZBS)
					flRadius = 4.0f;
			}
			else if (m_iState > 0)
			{
				flRadius = 1.25f;
				if (g_pModRunning->DamageTrack() == DT_ZB)
					flRadius = 1.5f;
				else if (g_pModRunning->DamageTrack() == DT_ZBS)
					flRadius = 2.5f;
			}
			else
			{
				flRadius = 1.0f;
				if (g_pModRunning->DamageTrack() == DT_ZB)
					flRadius = 1.0f;
				else if (g_pModRunning->DamageTrack() == DT_ZBS)
					flRadius = 1.5f;
			}
			return flRadius;
		}
	};

	LINK_ENTITY_TO_CLASS(y24s1laserminigunrange, CY24S1LasermingunRange)
}
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif
    class CY24S1LaserMiniGun : public LinkWeaponTemplate< CY24S1LaserMiniGun,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_M249>::template type,
            TPrecacheEvent,
            TDeployDefault,
            TReloadDefault,
            TGetDamageDefault
    >
    {
    public:
	static constexpr const auto& ReloadTime = 3.5s;
	static constexpr const auto& DeployTime = 2.0s;
	//static constexpr const auto& DrawAttackTime = 2.0s;
	static constexpr const auto& DefaultDeployTime = DeployTime;
	static constexpr const auto& DefaultReloadTime = ReloadTime;
    static constexpr const char *V_Model = "models/v_y24s1laserminigun.mdl";
    static constexpr const char *P_Model = "models/p_y24s1laserminigunA.mdl";
	static constexpr const char* P_Model_2 = "models/p_y24s1laserminigunB.mdl";
    static constexpr const char *W_Model = "models/w_y24s1laserminigun.mdl";
    static constexpr const char *EventFile = "events/y24s1laserminigun.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_y24s1laserminigun";
    static constexpr const char *AnimExtension = "m249";
    static constexpr int MaxClip = 200;
	enum
	{
		ANIM_IDLE1 = 0,
        ANIM_CHARGE_ORIGIN,
        ANIM_CHARGE_START,
        ANIM_CHARGE_LOOP,
        ANIM_CHARGE_SHOOT,
        ANIM_RELOAD,
        ANIM_DRAW,
		ANIM_SHOOT1,
		ANIM_SHOOT2,
        ANIM_SHOOT3,
	};

	static constexpr int DamageDefault = 25;
	static constexpr int DamageZB = 99;
	static constexpr int DamageZBS = 220;
	static constexpr float RangeMod = 0.97f;
	static constexpr const auto& CycleTime = 0.07s;
	static constexpr int Penetration = 1;
	
	static constexpr float AccuracyDefault = 0.2;
	static constexpr float AccuracyDiv = 210.0;
	static constexpr float AccuracyAdd = 0.4;
	static constexpr float AccuracyLimit = 0.9;

	static constexpr float SpreadBase = 0.0;
	static constexpr float SpreadMod = 0.01;
	static constexpr float SpreadBaseJump = 0.03;
	static constexpr float SpreadModJump = 0.2;
	static constexpr float SpreadBaseMove = 0.02;
	static constexpr float SpreadModMove = 0.04;

	static constexpr int ChargingCount = 5;
	static constexpr int MaxChargingCount = 12;
	static constexpr const auto& ChargingAnimStartToLoop = 1.5s;
	static constexpr const auto& ChargingShootTime = 0.8s;
	static constexpr const auto& NextConsumeTime = 0.5s;
	static constexpr int RequireChargingCount = 1;
	static constexpr int ConsumeCount = 1;
	static constexpr int MaxConsumeLevel = 4;
	static constexpr float ChargingAttackDistance = 1500.0;
	
	static constexpr const auto& SoundIdleLoop = 2.1s;
	static constexpr const auto& SoundOriginLoop = 1.7s;
	static constexpr const auto& SoundChargingLoop = 1.7s;


    static constexpr float PlayerMaxSpeed = 230;
    static constexpr float ArmorRatioModifier = 1.65f;

    KnockbackData KnockBack = { 350.0f, 250.0f, 300.0f, 100.0f, 0.6f };

#ifndef CLIENT_DLL
	EntityHandle <CY24S1LasermingunCharge> m_pChargeAmmo;
	EntityHandle<CY24S1LasermingunRange> m_pRange;
#endif

	time_point_t m_flTimeCheckCharge;
	int m_iChargeState;
	int m_iConsumeLevel;
	int m_iChargeShotsFired;
	time_point_t m_flTimeCheckAmmo2;
	time_point_t m_flSoundIdleLoop;

	void Spawn()
	{
		m_iChargeShotsFired = 0;
		m_iChargeState = 0;
		m_iConsumeLevel = 0;
		m_flTimeCheckCharge = invalid_time_point;
		m_flSoundIdleLoop = invalid_time_point;
		m_flTimeCheckAmmo2 = gpGlobals->time;
		Base::Spawn();
	}

	int ExtractAmmo(CBasePlayerWeapon* pWeapon) override
	{
		m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = m_iWeaponState;

		return CBasePlayerWeapon::ExtractAmmo(pWeapon);
	}

    void Precache(void)
    {
        Base::Precache();
        PRECACHE_SOUND("weapons/laserminigun-charge_start.wav");
        PRECACHE_SOUND("weapons/laserminigun-charge_origin.wav");
        PRECACHE_SOUND("weapons/laserminigun-charge_shoot.wav");

        PRECACHE_SOUND("weapons/laserminigun-1.wav");

		PRECACHE_MODEL("models/p_laserminigunb.mdl");
		PRECACHE_MODEL("sprites/muzzleflash37.spr");
		PRECACHE_MODEL("sprites/muzzleflash38.spr");
		PRECACHE_MODEL("sprites/muzzleflash39.spr");

		PRECACHE_MODEL("sprites/ef_laser1.spr");
		PRECACHE_MODEL("sprites/ef_laser2.spr");
		PRECACHE_MODEL("sprites/ef_laser3.spr");
    }


    int GetItemInfo(ItemInfo* p) override
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "laserminigun";
        p->iMaxAmmo1 = MAX_AMMO_556NATOBOX;
        p->pszAmmo2 = "y24s1laserminigun2";
        p->iMaxAmmo2 = -1;
        p->iMaxClip = 200;
        p->iSlot = 0;
        p->iPosition = 4;
        p->iId = m_iId = WEAPON_Y24S1LASERMINIGUN;
        p->iFlags = 0;
        p->iWeight = M249_WEIGHT;
        return 1;
    }

    bool HasSecondaryAttack() override { return true; }
#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_laserminigun" , 80 }; }
#endif
    float GetArmorRatioModifier() override { return 1.65; }
    WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 6, 3, 0 }; }

	BOOL Deploy() override
	{
#ifndef CLIENT_DLL
		if (m_pChargeAmmo)
		{
			m_pChargeAmmo->SUB_Remove();
		}

		if (m_pRange)
		{
			m_pRange->SUB_Remove();
		}
#endif
		m_flTimeCheckAmmo2 = invalid_time_point;
		m_iChargeState = 0;
		m_iConsumeLevel = 0;
		if (Base::Deploy())
		{
			pev->skin = 0;
			return TRUE;
		}
			
		return FALSE;
	}

	void Holster(int skiplocal) override
	{
#ifndef CLIENT_DLL
		if (m_pChargeAmmo)
		{
			m_pChargeAmmo->SUB_Remove();
		}

		if (m_pRange)
		{
			m_pRange->SUB_Remove();
		}
#endif
		SetThink(nullptr);
		return CBasePlayerWeapon::Holster(skiplocal);
	}

	void ItemPostFrame()
	{
#ifndef CLIENT_DLL
		if (m_pRange && m_iChargeState)
		{
			UTIL_MakeVectors(m_pPlayer->pev->v_angle);

			m_pPlayer->FireBullets3(m_pPlayer->GetGunPosition(), gpGlobals->v_forward, 0.0, ChargingAttackDistance, 1, 0, 0, 0.88, m_pPlayer->pev, true, m_pPlayer->random_seed);
		}
#endif

		if (m_flTimeCheckAmmo2 < gpGlobals->time && m_flTimeCheckAmmo2 != invalid_time_point)
		{	
			if (m_iWeaponState > 0 && m_iConsumeLevel < MaxConsumeLevel)
			{
				m_iWeaponState -= ConsumeCount;
				m_iConsumeLevel += ConsumeCount;

				//2 1
				//5 4
				m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = m_iWeaponState;
#ifndef CLIENT_DLL
				if (m_pChargeAmmo)
				{
					if (m_iConsumeLevel >= MaxConsumeLevel)
					{
						//red
						m_pChargeAmmo->SetState(m_iConsumeLevel);
					}			
				}

				if (m_pRange)
				{
					m_pRange->pev->scale += 0.1 * m_iConsumeLevel;
					m_pRange->m_iState = m_iConsumeLevel;
				}
#endif
			}
				
			m_flTimeCheckAmmo2 = gpGlobals->time + NextConsumeTime;
		}

		CBasePlayerWeapon::ItemPostFrame();
	}

	void PrimaryAttack(void)
	{	
		if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			LaserMiniGunFire(SpreadBaseJump + SpreadModJump * m_flAccuracy, CycleTime, FALSE);
		else if (m_pPlayer->pev->velocity.Length2D() > 0)
			LaserMiniGunFire(SpreadBaseMove + SpreadModMove * m_flAccuracy, CycleTime, FALSE);
		else
			LaserMiniGunFire(SpreadBase + SpreadMod * m_flAccuracy, CycleTime, FALSE);
	}


	void LaserMiniGunFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
	{
		m_bDelayFire = true;
		m_iShotsFired++;
		m_flAccuracy = ((m_iShotsFired * m_iShotsFired * m_iShotsFired) / AccuracyDiv) + AccuracyAdd;

		if (m_flAccuracy > AccuracyLimit)
			m_flAccuracy = AccuracyLimit;

		if (m_iClip <= 0)
		{
			if (m_fFireOnEmpty)
			{
				PlayEmptySound();
				m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2s;
			}

			return;
		}

		m_iChargeShotsFired++;

		if (m_iChargeShotsFired >= ChargingCount && m_iWeaponState < MaxChargingCount)
		{
			m_iWeaponState += RequireChargingCount;
			m_iChargeShotsFired = 0;
		}

		m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = m_iWeaponState;

		m_iClip--;
		m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

		UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

		m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
		m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

		Vector vecSrc = m_pPlayer->GetGunPosition();
		Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, Penetration, BULLET_PLAYER_556MM, GetDamage(), RangeMod, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);

		int flags;
#ifdef CLIENT_WEAPONS
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif

		PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), m_iWeaponState, FALSE);
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

#ifndef CLIENT_DLL
		if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.6s;

		if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			KickBackCustom(1, 0.4, 0.2, 0.15, 2.8, 1.8, 0);
		else if (m_pPlayer->pev->velocity.Length2D() > 0)
			KickBackCustom(0.3, 0.075, 0.05, 0.02, 2.3, 1.3, 0);
		else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
			KickBackCustom(0.175, 0.04, 0.03, 0.001, 1.3, 0.8, 0);
		else
			KickBackCustom(0.2, 0.05, 0.035, 0.015, 1.8, 1.05, 0);
		
	}


	void KickBackCustom(float up_base, float lateral_base, float up_modifier, float lateral_modifier, float up_max, float lateral_max, int direction_change)
	{
		float up, lateral;
		int iRan;

		up = ((rand() % 10) * up_base) * 0.1;
		lateral = ((rand() % 10) * lateral_base) * 0.1;

		if (m_iShotsFired != 1)
		{
			up += (rand() % (m_iShotsFired * 2)) * up_modifier;
			lateral += (rand() % (m_iShotsFired * 2)) * lateral_modifier;
		}

		iRan = rand();

		if (!iRan || iRan % 2)
		{
			m_pPlayer->pev->punchangle.x += up;

			if (m_pPlayer->pev->punchangle.x > up_max)
				m_pPlayer->pev->punchangle.x = up_max;
		}
		else
		{
			m_pPlayer->pev->punchangle.x -= up;

			if (m_pPlayer->pev->punchangle.x < -up_max)
				m_pPlayer->pev->punchangle.x = -up_max;
		}

		if (rand() % 2 == 1)
		{
			m_pPlayer->pev->punchangle.y += lateral;

			if (m_pPlayer->pev->punchangle.y > lateral_max)
				m_pPlayer->pev->punchangle.y = lateral_max;
		}
		else
		{
			m_pPlayer->pev->punchangle.y -= lateral;

			if (m_pPlayer->pev->punchangle.y < -lateral_max)
				m_pPlayer->pev->punchangle.y = -lateral_max;
		}
	}



	void WeaponIdle(void)
	{
		ResetEmptySound();

		if (m_iChargeState && !(m_pPlayer->pev->button & IN_ATTACK2))
		{
			OnAttack2Released();
		}

		if (pev->weaponanim == ANIM_IDLE1 && m_flSoundIdleLoop < gpGlobals->time)
		{
			UTIL_EMIT_SOUND(this->edict(), CHAN_WEAPON, "weapons/y24s1laserminigun_idle.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM, 1);

			m_flSoundIdleLoop = gpGlobals->time + SoundIdleLoop;
		}

		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;

		SendWeaponAnim(ANIM_IDLE1);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20.0s;
	}

	void OnAttack2Released()
	{
		m_pPlayer->pev->weaponmodel = MAKE_STRING(P_Model);

		if (m_iChargeState)
		{
			pev->skin = 0;
			SendWeaponAnim(ANIM_CHARGE_SHOOT);

			UTIL_EMIT_SOUND(this->edict(), CHAN_WEAPON, "weapons/y24s1laserminigun_charge_shoot.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM, 1);

#ifndef CLIENT_DLL
			if (m_pChargeAmmo)
			{
				m_pChargeAmmo->SUB_Remove();
			}

			if (m_pRange)
			{
				m_pRange->Explode();
			}

#endif
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + ChargingShootTime;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + ChargingShootTime;
		}

		m_flTimeCheckAmmo2 = invalid_time_point;
		m_iChargeState = 0;
	}

    void SecondaryAttack(void)
    {
		if (m_iWeaponState <= 0)
			return;

		if (m_iChargeState)
		{
			if (gpGlobals->time > m_flTimeCheckCharge)
			{
				switch (m_iChargeState)
				{
				case 1:
				{
					SendWeaponAnim(ANIM_CHARGE_START);

					m_iChargeState = 2;
					m_flTimeCheckCharge = gpGlobals->time + ChargingAnimStartToLoop;
					m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.0s;
					m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 3.0s;
					break;
				}
				case 2:
				{
					//red
					UTIL_EMIT_SOUND(this->edict(), CHAN_STATIC, "weapons/y24s1laserminigun-charge_ready.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM, 1);
					UTIL_EMIT_SOUND(this->edict(), CHAN_WEAPON, "weapons/y24s1laserminigun-charge_loop.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM, 1);

					SendWeaponAnim(ANIM_CHARGE_LOOP);
					m_iChargeState = 3;
					m_flTimeCheckCharge = gpGlobals->time + 9999.0s;
					m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 9999.0s;
					m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 9999.0s;
					break;
				}
				case 3:
				{
					m_flTimeCheckCharge = gpGlobals->time + SoundChargingLoop;

					UTIL_EMIT_SOUND(this->edict(), CHAN_WEAPON, "weapons/y24s1laserminigun-charge_loop.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM, 1);

				}

				default:
					break;
				}
			}
		}
		else
		{
			pev->skin = 1;

			m_pPlayer->pev->weaponmodel = MAKE_STRING(P_Model_2);

			UTIL_EMIT_SOUND(this->edict(), CHAN_WEAPON, "weapons/y24s1laserminigun-charge_origin.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM, 1);

			
			m_flTimeCheckAmmo2 = gpGlobals->time + NextConsumeTime;
			m_iWeaponState -= ConsumeCount;
			m_iConsumeLevel = 0;
			m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = m_iWeaponState;

			SendWeaponAnim(ANIM_CHARGE_ORIGIN);

			m_flTimeCheckCharge = gpGlobals->time + NextConsumeTime;
			m_iChargeState = 1;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.0s;
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 3.0s;

#ifndef CLIENT_DLL
			m_pChargeAmmo = CY24S1LasermingunCharge::Create(ENT(m_pPlayer->pev));
			if (m_pChargeAmmo)
				m_pChargeAmmo->m_pOwner = m_pPlayer;

			UTIL_MakeVectors(m_pPlayer->pev->v_angle);

			Vector vecDir = m_pPlayer->FireBullets3(m_pPlayer->GetGunPosition(), gpGlobals->v_forward, 0.0, ChargingAttackDistance, Penetration, 0, 0, RangeMod, m_pPlayer->pev, true, m_pPlayer->random_seed);
#endif
		}
    }


#ifndef CLIENT_DLL
	void FireBullet3CallBack(int iPenetration, Vector vecDir, TraceResult* pTrace)
	{
		if (m_iChargeState)
		{
			if (Penetration == iPenetration)
			{
				Vector vecSrc = pTrace->vecEndPos;

				if (!m_pRange)
				{
					m_pRange = static_cast<CY24S1LasermingunRange*>(CBaseEntity::Create("y24s1laserminigunrange", vecSrc, m_pPlayer->pev->v_angle, ENT(m_pPlayer->pev)));
					m_pRange->m_pOwner = m_pPlayer;
				}
					
				m_pRange->SetOrigin(vecSrc);	
			}
		}

	}
#endif
    		 	
};
LINK_ENTITY_TO_CLASS(weapon_y24s1laserminigun, CY24S1LaserMiniGun)
}