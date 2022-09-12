/*
bloodhunter.cpp
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
#include "cbase/cbase_hash.h"
#include "weapons/WeaponTemplate.hpp"


#ifndef CLIENT_DLL
namespace sv {
	class CBloodGrenade : public CGrenade
	{
	public:
		int GetWeaponsId(void)
		{
			return WEAPON_BLOODHUNTER;
		}
		
		static CBloodGrenade* ShootGrenade(entvars_t* pevOwner, Vector vecStart, Vector vecVelocity, int iTeam, int iType, float flDamage)
		{
			CBloodGrenade* pGrenade = CreateClassPtr<CBloodGrenade>();
			pGrenade->Spawn();
			UTIL_SetOrigin(pGrenade->pev, vecStart);
			pGrenade->pev->velocity = vecVelocity;
			pGrenade->pev->angles = UTIL_VecToAngles(-pGrenade->pev->velocity);
			pGrenade->pev->owner = ENT(pevOwner);
			pGrenade->pev->sequence = 1;
			pGrenade->pev->frame = 0;
			pGrenade->ResetSequenceInfo();
			pGrenade->m_fSequenceLoops = TRUE;
			pGrenade->pev->framerate = 1.0f;
			pGrenade->pev->gravity = 0.55f;
			pGrenade->pev->friction = 0.7f;
			pGrenade->pev->dmg = flDamage;
			pGrenade->pev->dmgtime = gpGlobals->time + 1.0s;
			pGrenade->pev->nextthink = gpGlobals->time + 0.1s;
			pGrenade->m_iTeam = iTeam;
			pGrenade->m_iGrenadeID = GRENADE_BLOODHUNTER;
			pGrenade->SetTouch(&CBloodGrenade::BounceTouch);
			pGrenade->SetThink(&CBloodGrenade::TumbleThink);
			pGrenade->m_iType = iType;
			return pGrenade;
		}

		void Precache(void)
		{
			m_sModelIndexExp = PRECACHE_MODEL("sprites/ef_bloodhunter3.spr");
			PRECACHE_MODEL("models/w_bloodhunter_left.mdl");
		}

		void Spawn()
		{
			Precache();
			pev->movetype = MOVETYPE_PUSHSTEP;

			if (pev->classname)
			{
				RemoveEntityHashValue(pev, STRING(pev->classname), CLASSNAME);
			}

			MAKE_STRING_CLASS("bloodgrenade", pev);
			AddEntityHashValue(pev, STRING(pev->classname), CLASSNAME);
			pev->solid = SOLID_BBOX;

			SET_MODEL(ENT(pev), "models/w_bloodhunter_left.mdl");
			UTIL_SetSize(pev, Vector(-1.0, -1.0, -1.0), Vector(1.0, 1.0, 1.0));
		}

	
		void BounceTouch(CBaseEntity* pOther)
		{
			if (pOther->edict() == pev->owner)
				return;

			if (pOther->IsPlayer() || pOther->Classify() == CLASS_PLAYER_ALLY)
			{
				SetThink(&CBloodGrenade::Detonate);
				pev->nextthink = gpGlobals->time + 0.1s;
				return;
			}

			if (FClassnameIs(pOther->pev, "func_breakable") && pOther->pev->rendermode != kRenderNormal)
			{
				pev->velocity = pev->velocity * -2.0f;
				return;
			}

			Vector vecTestVelocity;
			vecTestVelocity = pev->velocity;
			vecTestVelocity.z *= 0.7f;

			if (pev->flags & FL_ONGROUND)
			{
				pev->velocity = pev->velocity * 0.965f;
			}
		}

		void TumbleThink()
		{
			if (!IsInWorld())
			{
				UTIL_Remove(this);
				return;
			}

			if (pev->velocity.LengthSquared() > 1.0f)
			{
				auto vecAngles = UTIL_VecToAngles(-pev->velocity);
				vecAngles[0] = vecAngles[2] = 0.0f;
				pev->angles = vecAngles;
			}
			else
				pev->angles.x = 0.0f;

			if (pev->waterlevel != 0)
			{
				pev->velocity = pev->velocity * 0.5f;
				pev->framerate = 0.2f;
			}
			else
			{
				pev->framerate = pev->velocity.Length() / 200.0f;
			}

			StudioFrameAdvance();
			pev->nextthink = gpGlobals->time + 0.1s;

			if (pev->dmgtime <= gpGlobals->time)
			{
				SetThink(&CBloodGrenade::Detonate);
			}
		}

		void Detonate()
		{
			TraceResult tr;
			Vector vecSpot;

			vecSpot = pev->origin + Vector(0, 0, 8);
			UTIL_TraceLine(vecSpot, vecSpot + Vector(0, 0, -40), ignore_monsters, ENT(pev), &tr);
			Explode(&tr, DMG_EXPLOSION);
		}

		void Explode(TraceResult* pTrace, int bitsDamageType)
		{
			float flRadius = GetRadius();
			float flKnockbackRatio = GetKnockBackRatio();
			Vector vecSrc = pev->origin;
		

			pev->model = iStringNull; // invisible
			pev->solid = SOLID_NOT; // intangible
			pev->takedamage = DAMAGE_NO;

			MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
			WRITE_BYTE(TE_EXPLOSION);	// This makes a dynamic light and the explosion sprites/sound
			WRITE_COORD(pev->origin.x);	// Send to PAS because of the sound
			WRITE_COORD(pev->origin.y);
			WRITE_COORD(pev->origin.z + 20.0);
			WRITE_SHORT(m_sModelIndexExp);
			WRITE_BYTE(round(flRadius / 3.0));			// scale * 10
			WRITE_BYTE(30);		// framerate
			WRITE_BYTE(TE_EXPLFLAG_NONE);	// flags
			MESSAGE_END();

			entvars_t* pevOwner;
			if (pev->owner)
				pevOwner = VARS(pev->owner);
			else
				pevOwner = NULL;

#ifndef CLIENT_DLL
			//MH_SpecialEvent(id, 50 + 3);

			MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, pevOwner);
			WRITE_BYTE(26);
			WRITE_BYTE(0);
			MESSAGE_END();
#endif

			if (TheBots != NULL)
			{
				TheBots->OnEvent(EVENT_HE_GRENADE_EXPLODED, CBaseEntity::Instance(pev->owner));
			}

			CBaseEntity* pEntity = NULL;
			float flAdjustedDamage = pev->dmg;
			while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, flRadius)) != NULL) 
			{
				if (pEntity->pev->takedamage == DAMAGE_NO)
					continue;

				if (!pEntity->IsAlive())
					continue;

				Vector vecEnd, vecDelta;
				vecEnd = pevOwner->origin;
				vecDelta = vecSrc - vecEnd;

				flAdjustedDamage *= 1.0 - (vecDelta.Length() / flRadius);

				if (pEntity->edict() == pev->owner)
					flAdjustedDamage *= 0.05;

				if (flAdjustedDamage < 0.0)
					flAdjustedDamage = 0.0;

				pEntity->TakeDamage(this->pev, pevOwner, flAdjustedDamage, bitsDamageType);

				if (pEntity->IsPlayer() || pEntity->Classify() == CLASS_PLAYER_ALLY)
				{
					UTIL_AddKickRateBoost(pEntity, vecDelta.Make2D().Normalize(), 1.0, 1000 * GetKnockBackRatio(), 200 * GetKnockBackRatio());
				}
				
			}


			switch (RANDOM_LONG(0, 2))
			{
			case 0: EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/debris1.wav", 0.55, ATTN_NORM); break;
			case 1: EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/debris2.wav", 0.55, ATTN_NORM); break;
			case 2: EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/debris3.wav", 0.55, ATTN_NORM); break;
			}

			pev->effects |= EF_NODRAW;
			pev->velocity = g_vecZero;

			SetThink(nullptr);
			SetTouch(nullptr);
			UTIL_Remove(this);
		}

		private:
			float GetKnockBackRatio() const
			{
				if (g_pModRunning->DamageTrack() == DT_ZB)
				{
					if (m_iType == 0)
						return 1.0;
					else if (m_iType == 1)
						return 1.1;
					else
						return 1.3;
				}
				else if (g_pModRunning->DamageTrack() == DT_ZBS)
				{
					if (m_iType == 0)
						return 1.0;
					else if (m_iType == 1)
						return 1.1;
					else
						return 1.3;
				}
				else
				{
					if (m_iType == 0)
						return 0.3;
					else if (m_iType == 1)
						return 0.4;
					else
						return 0.5;
				}
				return 0.0;
			}

			float GetRadius() const 
			{
				if (g_pModRunning->DamageTrack() == DT_ZB)
				{
					if (m_iType == 0)
						return 350.0;
					else if(m_iType == 1)
						return 370.0;
					else
						return 400.0;
				}
				else if (g_pModRunning->DamageTrack() == DT_ZBS)
				{
					if (m_iType == 0)
						return 350.0;
					else if (m_iType == 1)
						return 370.0;
					else
						return 400.0;
				}
				else
				{
					if (m_iType == 0)
						return 100.0;
					else if (m_iType == 1)
						return 110.0;
					else
						return 120.0;
				}
				return 0.0;
			}

		private:
			int m_iType;
			short m_sModelIndexExp;
	};

	LINK_ENTITY_TO_CLASS(bloodgrenade, CBloodGrenade);
}
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif
    class CBloodHunter : public LinkWeaponTemplate< CBloodHunter,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_DEAGLE>::template type,
            TPrecacheEvent,
            TPrimaryAttackRifle,
            TFirePistol,
			TGetDamageDefault
    >
    {
    public:
#define BLOODHUNTER_CHARGE_REQUIRED 4

    static constexpr const char *V_Model = "models/v_bloodhunter.mdl";
    static constexpr const char *P_Model = "models/p_bloodhunter.mdl";
    static constexpr const char *W_Model = "models/w_bloodhunter.mdl";
    static constexpr const char *EventFile = "events/bloodhunter.sc";
    static constexpr InventorySlotType ItemSlot = PISTOL_SLOT;
    static constexpr const char *ClassName = "weapon_bloodhunter";
    static constexpr const char *AnimExtension = "onehanded";
    static constexpr int MaxClip = 30;


	enum
	{
		ANIM_IDLE_RIGHT = 0,
		ANIM_SHOOT_RIGHT,
		ANIM_SHOOT_RIGHT_EF,
		ANIM_SHOOT,
		ANIM_SHOOTA,
		ANIM_SHOOTB,
		ANIM_SHOOTC,
		ANIM_THROW,
        ANIM_THROWA,
		ANIM_THROWB,
		ANIM_THROWC,
		ANIM_RELOAD,
		ANIM_RELOADA,
		ANIM_RELOADB,
		ANIM_RELOADC,
		ANIM_DRAW,
		ANIM_DRAWA,
		ANIM_DRAWB,
		ANIM_DRAWC,
		ANIM_IDLE_LEFT,
		ANIM_IDLE_LEFTA,
		ANIM_IDLE_LEFTB,
		ANIM_IDLE_LEFTC,
		ANIM_CHANGEA,
		ANIM_CHANGEB,
		ANIM_CHANGEC,
	};
    static constexpr float PlayerMaxSpeed = 250;
    static constexpr float ArmorRatioModifier = 1.6f;
    static constexpr const auto &SpreadCalcNotOnGround = 0.14*(1-A);
    static constexpr const auto &SpreadCalcWalking = 0.055*(1-A);
    static constexpr const auto& SpreadCalcDucking = 0.03*(1-A);
    static constexpr const auto &SpreadCalcDefault = 0.035*(1-A);

	static constexpr const auto& CycleTime = 0.2s;
    static constexpr int DamageDefault = 37;
    static constexpr int DamageZB = 85;
    static constexpr int DamageZBS = 210;

    static constexpr float AccuracyDefault = 0.8;
    /*static constexpr*/ int Penetration = 3;
    KnockbackData KnockBack = { 450.0f, 400.0f, 400.0f, 250.0f, 0.7f };

#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_bloodhunter" , 25 }; }
#endif
	float GetArmorRatioModifier() override { return 1.6; }
	WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED | ACCURACY_DUCK, 8, 3, 0 }; }

	int m_iChargedFired;
	int m_iExpIndex;
	bool m_bHit;

	int GetItemInfo(ItemInfo* p)
	{
		p->pszName = STRING(pev->classname);
		p->pszAmmo1 = "bloodhunter";
		p->iMaxAmmo1 = 100;
		p->pszAmmo2 = NULL;
		p->iMaxAmmo2 = -1;
		p->iMaxClip = 30;
		p->iSlot = 1;
		p->iPosition = 2;
		p->iId = m_iId = WEAPON_BLOODHUNTER;
		p->iFlags = 0;
		p->iWeight = DEAGLE_WEIGHT;

		return 1;
	}

	void Precache(void)
	{
		PRECACHE_MODEL("sprites/ef_bloodhunter1.spr");
		PRECACHE_MODEL("sprites/ef_bloodhunter2.spr");
		m_iExpIndex = PRECACHE_MODEL("sprites/ef_bloodhunter3.spr");

		PRECACHE_SOUND("weapons/bloodhunter_throwa.wav");

		Base::Precache();
	}

	void PlayWeaponAnim(int iAnim)
	{
		SendWeaponAnim(iAnim, UseDecrement() != FALSE);
#ifndef CLIENT_DLL
		MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->pev);
		WRITE_BYTE(25);
		WRITE_BYTE(iAnim);
		WRITE_BYTE(m_iWeaponState);
		MESSAGE_END();
#endif
	}

    void ItemPostFrame(void)
    {
        if (pev->iuser3 && m_flNextSecondaryAttack < 0s)
        {
			ChargeAttack();
        }
        return CBasePlayerWeapon::ItemPostFrame();
    }

    void ChargeAttack()
    {
        m_iWeaponState = 0;
        m_iChargedFired = 0;
        pev->iuser3 = 0;

		PlayWeaponAnim(ANIM_DRAW);

        //MH_SpecialEvent(id, 50 + iType);
#ifndef CLIENT_DLL
		MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->pev);
		WRITE_BYTE(26);
		WRITE_BYTE(m_iWeaponState);
		MESSAGE_END();
#endif

		Vector angThrow = m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle;

		if (angThrow.x < 0)
			angThrow.x = -10 + angThrow.x * ((90 - 10) / 90.0);
		else
			angThrow.x = -10 + angThrow.x * ((90 + 10) / 90.0);

		float flVel = (90 - angThrow.x) * 6;

		if (flVel > 750)
			flVel = 750;

		UTIL_MakeVectors(angThrow);
		Vector vecSrc = m_pPlayer->pev->origin + m_pPlayer->pev->view_ofs + gpGlobals->v_forward * 16;
		Vector vecThrow = gpGlobals->v_forward * flVel + m_pPlayer->pev->velocity;

#ifndef CLIENT_DLL
		CBloodGrenade::ShootGrenade(m_pPlayer->pev, vecSrc, vecThrow, m_pPlayer->m_iTeam, m_iWeaponState, GetGrenadeDamage());
#endif
        m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.03s;
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;
    }

	void SecondaryAttack(void)
	{
        if (!m_iWeaponState)
            return;

		EMIT_SOUND_DYN(this->edict(), CHAN_WEAPON, "weapons/bloodhunter_throwa.wav", 0.8, ATTN_NORM, 0, PITCH_NORM);

		PlayWeaponAnim(ANIM_THROW + m_iWeaponState);

        m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3s;
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;
        pev->iuser3 = 1;
	}

	BOOL Deploy()
	{
#ifndef CLIENT_DLL
		MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->pev);
		WRITE_BYTE(26);
		WRITE_BYTE(m_iWeaponState);
		MESSAGE_END();

		MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->pev);
		WRITE_BYTE(25);
		WRITE_BYTE(m_iWeaponState + 15);
		WRITE_BYTE(m_iWeaponState);
		MESSAGE_END();
#endif
		m_flAccuracy = 1.0;
        pev->iuser3 = 0;
		return DefaultDeploy("models/v_bloodhunter.mdl", "models/p_bloodhunter.mdl", m_iWeaponState + 15, m_pPlayer->GetAnimExtDualPistols(), UseDecrement() != FALSE);
	}

	void Reload(void)
	{
		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			return;

		if (DefaultReload(10, m_iWeaponState + 11, 3.2s))
		{
#ifndef CLIENT_DLL
			MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->pev);
			WRITE_BYTE(25);
			WRITE_BYTE(m_iWeaponState + 11);
			WRITE_BYTE(m_iWeaponState);
			MESSAGE_END();

			m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
			m_flAccuracy = 1.0;
		}
	}

    void Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
    {
		m_iShotsFired++;

		if (m_iShotsFired > 1)
			return;

		if (m_flLastFire != invalid_time_point)
		{
			m_flAccuracy -= (0.52 - ((gpGlobals->time - m_flLastFire) / 1s)) * 0.3;

			if (m_flAccuracy > 1.0)
				m_flAccuracy = 1.0;
			else if (m_flAccuracy < 0.6)
				m_flAccuracy = 0.6;
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

		m_iClip--;
		m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

		UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
		m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;

		Vector vecSrc = m_pPlayer->GetGunPosition();
		Vector vecDir;

		m_bHit = false;

		vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 4096, 3, BULLET_PLAYER_50AE, GetDamage(), 0.8, m_pPlayer->pev, TRUE, m_pPlayer->random_seed);

#ifndef CLIENT_DLL
		int bParam2 = (m_iWeaponState < 3 && m_iChargedFired == BLOODHUNTER_CHARGE_REQUIRED) ? TRUE : FALSE;
		PLAYBACK_EVENT_FULL(FEV_GLOBAL, m_pPlayer->edict(), m_usFire, 0.0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, int(m_pPlayer->pev->punchangle.x * 100), int(m_pPlayer->pev->punchangle.y * 100), FALSE, bParam2);
#endif
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;
#ifndef CLIENT_DLL
		if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
		
		if (m_pPlayer->pev->velocity.Length2D() > 0)
			KickBack(0.5, 0.6, 0.3, 0.15, 2.5, 3.5, 4);
		else if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			KickBack(1.0, 1.0, 0.8, 0.8, 5.0, 5.0, 5);
		else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
			KickBack(0.3, 0.45, 0.12, 0.12, 2.0, 3.0, 2);
		else
			KickBack(0.4, 0.55, 0.2, 0.3, 2.0, 2.5, 3.5);

		Fire_Post();
    }

	void Fire_Post()
	{
		if (m_iChargedFired >= BLOODHUNTER_CHARGE_REQUIRED && m_iWeaponState < 3)
		{
			m_iWeaponState++;
			m_iChargedFired = 0;
			//MH_SpecialEvent(id, 50 + iType);
#ifndef CLIENT_DLL
			MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->pev);
			WRITE_BYTE(26);
			WRITE_BYTE(m_iWeaponState);
			MESSAGE_END();
#endif
		}
	}

	void WeaponIdle(void)
	{
		ResetEmptySound();
		m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;

		PlayWeaponAnim(ANIM_IDLE_RIGHT);
	}

	float GetGrenadeDamage() const
	{
		float flDamage = 30.0;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
		{
			if (m_iWeaponState == 0)
				return 400.0;
			else if (m_iWeaponState == 1)
				return 700.0;
			else
				return 1000.0;
		}
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
		{
			if (m_iWeaponState == 0)
				return 700.0;
			else if (m_iWeaponState == 1)
				return 1000.0;
			else
				return 1300.0;
		}
		else
		{
			if (m_iWeaponState == 0)
				return 30.0;
			else if (m_iWeaponState == 1)
				return 40.0;
			else
				return 50.0;
		}
#endif
		return flDamage;
	}


#ifndef CLIENT_DLL
	void FireBullet3CallBack(int iPenetration, Vector vecDir, TraceResult* pTrace)
	{
		if (!m_bHit)
		{
			if (CBaseEntity* pEntity = gMultiDamage.pEntity)
			{
				if (pEntity && (pEntity->IsPlayer() || pEntity->Classify() == CLASS_PLAYER_ALLY))
				{
					if (pEntity->IsPlayer())
					{
						if (g_pGameRules->PlayerRelationship(m_pPlayer, pEntity) != GR_TEAMMATE)
						{
							m_iChargedFired++;
							if (m_iWeaponState < 3)
								PLAYBACK_EVENT_FULL(FEV_GLOBAL, m_pPlayer->edict(), m_usFire, 0.0, g_vecZero, g_vecZero, 0.0, 0.0, 0, 0, TRUE, FALSE);
						}
					}
					else
					{
						m_iChargedFired++;
						if (m_iWeaponState < 3)
							PLAYBACK_EVENT_FULL(FEV_GLOBAL, m_pPlayer->edict(), m_usFire, 0.0, g_vecZero, g_vecZero, 0.0, 0.0, 0, 0, TRUE, FALSE);
					}
				}

				m_bHit = true;
			}
		}

	}
#endif
};
LINK_ENTITY_TO_CLASS(weapon_bloodhunter, CBloodHunter)
}