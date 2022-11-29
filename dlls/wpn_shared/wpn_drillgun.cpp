/*
drillgun.cpp
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
namespace sv {
	class CDrillAmmo : public CBaseEntity
	{
	public:
		KnockbackData GetKnockBackData() override { return { 700.0f, 450.0f, 600.0f, 450.0f, 0.9f }; }
		int GetWeaponsId(void) override { return WEAPON_DRILLGUN; }
		float GetArmorRatioModifier() override { return 1.7; }

		static CDrillAmmo* CreateBolt(void)
		{
			edict_t* pent = CREATE_NAMED_ENTITY(MAKE_STRING("drill"));

			if (FNullEnt(pent))
			{
				ALERT(at_console, "NULL Ent in Create!\n");
				return NULL;
			}

			CDrillAmmo* pDrill = (CDrillAmmo*)Instance(pent);

			pDrill->Spawn();

			return pDrill;
		}

		void Spawn() override
		{
			Precache();

			pev->movetype = MOVETYPE_FLY;
			pev->solid = SOLID_BBOX;
			pev->gravity = 0.5;

			SET_MODEL(ENT(pev), "models/drillgun_nail.mdl");
			UTIL_SetOrigin(pev, pev->origin);
			UTIL_SetSize(pev, g_vecZero, g_vecZero);

			SetTouch(&CDrillAmmo::DrillTouch);
			IgniteThink();

			//pev->nextthink = gpGlobals->time + 0.1s;
			m_DirectDmg = TEAM_UNASSIGNED;

			m_bLockingTarget = FALSE;
		}

		void Precache(void)
		{
			PRECACHE_MODEL("models/drillgun_nail.mdl");
			PRECACHE_SOUND("weapons/xbow_hitbod1.wav");
			PRECACHE_SOUND("weapons/xbow_hit1.wav");

			PRECACHE_SOUND("weapons/knife_hitwall1.wav");

			m_iTrail = PRECACHE_MODEL("sprites/smoke.spr");
		}

		void EXPORT DrillTouch(CBaseEntity* pOther)
		{
			if (pOther->IsBSPModel())
			{
				SetThink(NULL);
				SetTouch(NULL);

				EMIT_SOUND_DYN(ENT(pev), CHAN_BODY, "weapons/xbow_hit1.wav", RANDOM_FLOAT(0.95, 1.0), ATTN_NORM, 0, 98 + RANDOM_LONG(0, 7));

				SetThink(&CBaseEntity::SUB_Remove);
				pev->nextthink = gpGlobals->time;

				if (FClassnameIs(pOther->pev, "worldspawn"))
				{
					Vector vecDir = pev->velocity.Normalize();
					Vector vecSrc = pev->origin;
					UTIL_SetOrigin(pev, vecSrc);

					pev->angles = UTIL_VecToAngles(vecSrc);
					pev->solid = SOLID_NOT;
					pev->movetype = MOVETYPE_FLY;
					pev->velocity = g_vecZero;
					pev->avelocity.z = 0.0;
					pev->angles.z = RANDOM_LONG(0, 360);

					pev->nextthink = gpGlobals->time + 10.0s;
				}

				if (UTIL_PointContents(pev->origin) != CONTENTS_WATER)
				{
					TraceResult tr = UTIL_GetGlobalTrace();

					UTIL_GunshotDecalTrace(&tr, DamageDecal(DMG_BULLET | DMG_NEVERGIB), false, NULL);
				}
			}
			else
			{
				TraceResult tr = UTIL_GetGlobalTrace();
				entvars_t* pevOwner = VARS(pev->owner);

				ClearMultiDamage();

				pOther->TraceAttack(pevOwner, m_DirectDmg, pev->velocity.Normalize(), &tr, DMG_BULLET | DMG_NEVERGIB);

				ApplyMultiDamage(pev, pevOwner);

				EMIT_SOUND_DYN(ENT(pev), CHAN_BODY, "weapons/knife_hitwall1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
				
				if (m_iPenetration && (pOther->IsPlayer() || pOther->Classify() == CLASS_PLAYER_ALLY))
				{
					pev->origin += m_vecDirection * 0.5 * 39.37;

					m_vecLockingPos = pev->origin;
					m_bLockingTarget = TRUE;

					pev->nextthink = gpGlobals->time;

					m_iPenetration--;
				}

				if (!m_iPenetration)
				{
					SetThink(NULL);
					SetTouch(NULL);

					MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
					WRITE_BYTE(TE_KILLBEAM);
					WRITE_SHORT(this->entindex());
					MESSAGE_END();

					UTIL_Remove(this);

					return;
				}
			}
		}

		void EXPORT BubbleThink(void)
		{
			pev->velocity = m_vecVelocity;

			if (m_bLockingTarget)
			{
				pev->origin = m_vecLockingPos;

				m_bLockingTarget = FALSE;
			}

			UTIL_BubbleTrail(pev->origin - pev->velocity * 0.1, pev->origin, 1);

			if (pev->waterlevel == 0)
				SetThink(&CDrillAmmo::FollowThink);

			pev->nextthink = gpGlobals->time + 0.1s;
		}

		void EXPORT FollowThink(void)
		{
			pev->velocity = m_vecVelocity;

			if (m_bLockingTarget)
			{
				pev->origin = m_vecLockingPos;

				m_bLockingTarget = FALSE;
			}

			if (pev->waterlevel != 0)
				SetThink(&CDrillAmmo::BubbleThink);
	
			pev->nextthink = gpGlobals->time + 0.1s;
		}

		void EXPORT IgniteThink(void)
		{
			if (pev->waterlevel == 0)
			{
				MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
				WRITE_BYTE(TE_BEAMFOLLOW);
				WRITE_SHORT(entindex());
				WRITE_SHORT(m_iTrail);
				WRITE_BYTE(2);
				WRITE_BYTE(2);
				WRITE_BYTE(255);
				WRITE_BYTE(255);
				WRITE_BYTE(255);
				WRITE_BYTE(200);
				MESSAGE_END();

				SetThink(&CDrillAmmo::FollowThink);
			}
			else
			{
				SetThink(&CDrillAmmo::BubbleThink);
			}

			pev->nextthink = gpGlobals->time + 0.1s;
		}

	public:
		BOOL m_bLockingTarget;
		Vector m_vecLockingPos;
		Vector m_vecVelocity;
		Vector m_vecDirection;
		int m_iPenetration;
		int m_iTrail;
		float m_DirectDmg;
	};

	LINK_ENTITY_TO_CLASS(drill, CDrillAmmo);
}
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif
    class CDrillGun : public LinkWeaponTemplate< CDrillGun,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_P90>::template type,
			TPrecacheEvent,
			TGetDamageDefault
    >
    {
    public:
    static constexpr const char *V_Model = "models/v_drillgun.mdl";
    static constexpr const char *P_Model = "models/p_drillgun.mdl";
    static constexpr const char *W_Model = "models/w_drillgun.mdl";
    static constexpr const char *EventFile = "events/drillgun.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_drillgun";
    static constexpr const char *AnimExtension = "carbine";
    static constexpr int MaxClip = 20;
	enum
	{
		ANIM_IDLE1 = 0,	
		ANIM_SHOOT1,
		ANIM_RELOAD,
		ANIM_DRAW,
		ANIM_DRAW_EMPTY,
		ANIM_IDLE_EMPTY,
	};
	static constexpr float ArmorRatioModifier = 1.7f;

    static constexpr int DamageDefault = 133;
	static constexpr int DamageZB = 2750;
    static constexpr int DamageZBS = 3750;

    KnockbackData KnockBack = { 700.0f, 450.0f, 600.0f, 450.0f, 0.9f };

	int GetItemInfo(ItemInfo* p)
	{
		p->pszName = STRING(pev->classname);
		p->pszAmmo1 = "drillgun";
		p->iMaxAmmo1 = 20;
		p->pszAmmo2 = NULL;
		p->iMaxAmmo2 = -1;
		p->iMaxClip = -1;
		p->iSlot = 0;
		p->iPosition = 16;
		p->iId = m_iId = WEAPON_DRILLGUN;
		p->iFlags = 0;
		p->iWeight = P90_WEIGHT;

		return 1;
	}
#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_drill" , 180 }; }
#endif
	WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 4, 4, 0 }; }

	float GetArmorRatioModifier() override { return 1.7; }

	float GetMaxSpeed(void) override
	{
		return 215;
	}

	time_point_t m_flTimeReloadDrill;

	void Spawn(void)
	{
		Base::Spawn();
		m_iClip = WEAPON_NOCLIP;
		m_iId = WEAPON_DRILLGUN;
		m_iDefaultAmmo = 20;
	}

	int ExtractAmmo(CBasePlayerWeapon* pWeapon)
	{
		if (m_iDefaultAmmo)
		{
			if (m_pPlayer)
				m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] = m_iDefaultAmmo;
			m_iClip = WEAPON_NOCLIP;
			m_iDefaultAmmo = 0;
			return TRUE;
		}
		return CBasePlayerWeapon::ExtractAmmo(pWeapon);
	}


	BOOL Deploy(void) override
	{
		m_flAccuracy = 0.05;
		m_iShotsFired = 0;
		m_bDelayFire = false;

		m_flTimeReloadDrill = invalid_time_point;
		
		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] > 0)
			return DefaultDeploy("models/v_drillgun.mdl", "models/p_drillgun.mdl", ANIM_DRAW, "carbine", UseDecrement() != FALSE);
		else
			return DefaultDeploy("models/v_drillgun.mdl", "models/p_drillgun.mdl", ANIM_DRAW_EMPTY, "carbine", UseDecrement() != FALSE);
	}


	float GetSpread(void)
	{
		if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			return m_flAccuracy * 0.035 + 0.1;
		else if (m_pPlayer->pev->velocity.Length2D() > 120)
			return m_flAccuracy * 0.006 + 0.02;
		else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
			return m_flAccuracy * 0.001 + 0.0075;
		else
			return m_flAccuracy * 0.001 + 0.01;
	}

	void PrimaryAttack() override
	{
		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			return;

		m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;

		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;


		m_pPlayer->pev->effects &= ~EF_MUZZLEFLASH;

		m_flTimeReloadDrill = gpGlobals->time + 1.0s;

		UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

		Vector vecSrc = m_pPlayer->GetGunPosition();
		Vector vecDir = m_pPlayer->FireBullets3(vecSrc - gpGlobals->v_up * 2.0, gpGlobals->v_forward, GetSpread(), 8192, 0, BULLET_PLAYER_556MM, 0, 0, m_pPlayer->pev, false, m_pPlayer->random_seed);
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);

		Vector vecDrillDir = gpGlobals->v_forward + gpGlobals->v_right * vecDir.x + gpGlobals->v_up * vecDir.y;
		Vector vecAngles = UTIL_VecToAngles(vecDrillDir);

		CDrillAmmo* pDrill = CDrillAmmo::CreateBolt();

		pDrill->pev->origin = vecSrc;
		pDrill->pev->angles = vecAngles;
		pDrill->pev->owner = m_pPlayer->edict();
		pDrill->m_iTeam = m_pPlayer->m_iTeam;
		pDrill->m_DirectDmg = GetDamage();
		pDrill->m_iPenetration = 5;

		float flSpeed;
		if (m_pPlayer->pev->waterlevel == 3)
			flSpeed = 1500.0;
		else
			flSpeed = 2000.0;

		pDrill->m_vecVelocity = pDrill->pev->velocity = vecDrillDir * flSpeed;
		pDrill->pev->avelocity.z = 5.0;
		pDrill->m_vecDirection = vecDrillDir;
#endif

		int flags;

#ifdef CLIENT_WEAPONS
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif

		PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, 0, 0, 0, m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType], FALSE, FALSE);

		m_flNextPrimaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.05s;

		if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			KickBack(15.0, 10.0, 0.6, 0.35, 9.0, 6.0, 5);
		else if (m_pPlayer->pev->velocity.Length2D() > 0)
			KickBack(15.0, 10.0, 0.225, 0.05, 6.5, 2.5, 7);
		else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
			KickBack(4.0, 3.0, 0.125, 0.02, 5.0, 1.35, 9);
		else
			KickBack(10.0, 7.0, 0.22, 0.038, 5.9, 1.9, 8);
	}


	void ItemPostFrame(void)
	{
		if (m_flTimeReloadDrill != invalid_time_point && gpGlobals->time > m_flTimeReloadDrill && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] > 0)
		{
			SendWeaponAnim(ANIM_RELOAD);

			m_flTimeReloadDrill = invalid_time_point;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0s;
		}
		else
		{
			CBasePlayerWeapon::ItemPostFrame();
		}
	}

	void WeaponIdle(void)
	{
		ResetEmptySound();

		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;

		int iAnim;

		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			iAnim = ANIM_IDLE_EMPTY;
		else
			iAnim = ANIM_IDLE1;

		SendWeaponAnim(iAnim);

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20.0s;
	}

	
};

LINK_ENTITY_TO_CLASS(weapon_drillgun, CDrillGun)
}