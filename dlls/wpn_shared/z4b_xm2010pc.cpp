/*
z4b_xm2010pc.cpp
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

#ifndef CLIENT_DLL
namespace sv {
class CXM2010PC_Crystal : public CBaseEntity
{
public:
	void Spawn() override
	{
		Precache();

		//ph26 = 0;
		SetThink(&CXM2010PC_Crystal::FlyThink);
		SetTouch(&CXM2010PC_Crystal::OnTouch);
		SET_MODEL(this->edict(), "sprites/s_xm2010pc.mdl");

		//ph32 = ?
		pev->framerate = 10;
		pev->scale = 0.2;
		pev->solid = SOLID_BBOX; // 2
		pev->movetype = MOVETYPE_FLYMISSILE; // 9
		pev->nextthink = gpGlobals->time + 0.01s;
		m_flReturnTime = gpGlobals->time + 3s;
		/*
		v9 = 0x40800000;
		v10 = 0x40800000;
		v11 = 0x40800000;
		v6 = 80000000800000008000000080000000h ^ 0x40800000;
		v7 = 80000000800000008000000080000000h ^ 0x40800000;
		v8 = 80000000800000008000000080000000h ^ 0x40800000;
		return UTIL_SetSize((int)v4, (int)&v6, (int)&v9);
		*/
		UTIL_SetSize(pev, { -0.5, -0.5, -2.0 }, { 0.5, 0.5, 2.0 });

		int id = 0;
		CBasePlayer* owner = nullptr;
		if((owner = dynamic_ent_cast<CBasePlayer *>(pev->owner)) != nullptr)
		{
			id = owner->entindex();
		}
		
		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_BEAMENTS);
		WRITE_SHORT(id | 0x1000);
		WRITE_SHORT(this->entindex());
		WRITE_SHORT(m_iSprBeam);
		WRITE_BYTE(0); // framerate
		WRITE_BYTE(0); // framerate
		WRITE_BYTE(30); // life
		WRITE_BYTE(20);  // width
		WRITE_BYTE(7);   // noise
		WRITE_BYTE(50);
		WRITE_BYTE(20);
		WRITE_BYTE(255);
		WRITE_BYTE(255);	// brightness
		WRITE_BYTE(25);		// speed
		MESSAGE_END();

		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_BEAMENTS);
		WRITE_SHORT(id | 0x1000);
		WRITE_SHORT(this->entindex());
		WRITE_SHORT(m_iSprBeam);
		WRITE_BYTE(0); // framerate
		WRITE_BYTE(0); // framerate
		WRITE_BYTE(30); // life
		WRITE_BYTE(13); // width
		WRITE_BYTE(30);   // noise
		WRITE_BYTE(50);
		WRITE_BYTE(20);
		WRITE_BYTE(255);
		WRITE_BYTE(255);	// brightness
		WRITE_BYTE(25);		// speed
		MESSAGE_END();
	}

	void PenetrateStart()
	{
		pev->origin = pev->origin + m_vecStartVelocity.Normalize() * 42;
		pev->velocity = {};

		pev->solid = SOLID_NOT;

		pev->nextthink = gpGlobals->time + 0.05s;
	}

	void PenetrateEnd()
	{
		pev->velocity = m_vecStartVelocity;
		pev->solid = SOLID_CUSTOM;

		pev->nextthink = gpGlobals->time + 0.001s;
	}
	
	void EXPORT FlyThink()
	{
		if (gpGlobals->time < m_flReturnTime)
		{
			this->pev->nextthink = gpGlobals->time + 0.0099999998s;

			if (pev->solid == SOLID_NOT)
			{
				PenetrateEnd();
			}
		}
		else
		{
			Remove();
		}
	}
	

	void Precache() override
	{
		PRECACHE_MODEL("sprites/z4b/s_xm2010pc.mdl");
		m_iSprBeam = PRECACHE_MODEL("sprites/zbeam1.spr");
		m_iModelShell = PRECACHE_MODEL("models/z4b/s2_xm2010pc.mdl");
		m_iSprEffect2 = PRECACHE_MODEL("sprites/z4b/ef_xm2010pc.spr");
	}

	void EXPORT OnTouch(CBaseEntity* pOther)
	{
		if (TouchEntity(pOther))
		{
			std::vector<edict_t *> vecEntDamaged;
			RadiusDamage(*this, pev->origin, this, pev->owner, std::back_inserter(vecEntDamaged));
			MakeExplosionEffect();
			Remove();
		}
	}

	bool TouchEntity(CBaseEntity* pOther)
	{
		if (pev->owner == pOther->edict())
			return false;

		CBaseEntity* pAttacker = CBaseEntity::Instance(pev->owner);
		CBasePlayer* pAttackePlayer = nullptr;
		if (pAttacker && pAttacker->IsPlayer())
			pAttackePlayer = static_cast<CBasePlayer*>(pAttacker);

		if (pAttackePlayer &&
			pOther->pev->takedamage != DAMAGE_NO &&
			pOther->IsAlive()
			)
		{
			Vector vecDirection = (pOther->pev->origin - pev->origin).Normalize();

			TraceResult tr;
			UTIL_TraceLine(pev->origin, pOther->pev->origin, missile, ENT(pAttackePlayer->pev), &tr);
			tr.iHitgroup = HITGROUP_CHEST; // ...

			ClearMultiDamage();
			pOther->TraceAttack(pAttackePlayer->pev, m_flTouchDamage, vecDirection, &tr, DMG_BULLET);
			ApplyMultiDamage(pAttackePlayer->pev, pAttackePlayer->pev);

			PenetrateStart();
		}
		pev->velocity = {};

		return pOther->IsBSPModel() || pOther->pev->team != m_iTeam;
	}

	void MakeExplosionEffect()
	{
		MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
		WRITE_BYTE(TE_EXPLODEMODEL);
		WRITE_COORD(pev->origin.x);
		WRITE_COORD(pev->origin.y);
		WRITE_COORD(pev->origin.z);
		WRITE_SHORT(390);
		WRITE_SHORT(m_iModelShell);
		WRITE_SHORT(20);
		WRITE_BYTE(5);
		MESSAGE_END();

		MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
		WRITE_BYTE(TE_PARTICLEBURST); // TE id
		WRITE_COORD(pev->origin.x);
		WRITE_COORD(pev->origin.y);
		WRITE_COORD(pev->origin.z);
		WRITE_SHORT(30); // radius
		WRITE_BYTE(47); // color
		WRITE_BYTE(1); // duration (will be randomized a bit)
		MESSAGE_END();

		MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
		WRITE_BYTE(TE_EXPLOSION);
		WRITE_COORD(pev->origin.x);
		WRITE_COORD(pev->origin.y);
		WRITE_COORD(pev->origin.z);
		WRITE_SHORT(m_iSprEffect2);
		WRITE_BYTE(20);
		WRITE_BYTE(200);
		WRITE_BYTE(TE_EXPLFLAG_NOPARTICLES);
		MESSAGE_END();
	}

	void Init(Vector vecVelocity, float flTouchDamage, CBasePlayer *player)
	{
		std::tie(m_flTouchDamage, m_iTeam) = std::make_tuple(flTouchDamage, player->m_iTeam);
		pev->velocity = std::move(vecVelocity);
		m_vecStartVelocity = pev->velocity = std::move(vecVelocity);
		m_vecStartOrigin = pev->origin;
		VEC_TO_ANGLES(vecVelocity.Normalize(), pev->angles);
		pev->owner = player->edict();
	}

	float m_flTouchDamage;
	float m_flExplodeDamage;
	float m_flExplodeRadius;
	TeamName m_iTeam;
	int m_iSprBeam;
	int m_iModelShell;
	int m_iSprEffect2;
	time_point_t m_flReturnTime;
	Vector m_vecStartOrigin;
	Vector m_vecStartVelocity;


	float GetRadiusDamageAmount() const {
		float flDamage = 50;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage *= 9.5f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage *= 5.5f;
#endif
		return flDamage;
	}
	static constexpr auto RadiusDamageBitsDamageType = DMG_BULLET;
	static constexpr auto RadiusDamageRadius = 233;
	const KnockbackData RadiusDamageKnockback = { 0.0, 0.0, 0.0, 0.0, 1.0 };

protected:
	void Remove()
	{
		SetThink(nullptr);
		SetTouch(nullptr);
		pev->effects |= EF_NODRAW; // 0x80u

		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_KILLBEAM);
		WRITE_SHORT(this->entindex());
		MESSAGE_END();
		
		return UTIL_Remove(this);
	}
};
LINK_ENTITY_TO_CLASS(z4b_xm2010pc_crystal, CXM2010PC_Crystal)
}
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

    class CXM2010PC : public LinkWeaponTemplate< CXM2010PC,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_AWP>::template type,
            TPrecacheEvent,
            TDeployDefault,
            TReloadDefault,
			TSecondaryAttackSniperZoom2,
            TPrimaryAttackRifle,
			TRecoilPunch,
            TWeaponIdleDefault,
            TGetDamageDefault
    >
    {
    public:
    static constexpr const auto &DefaultReloadTime = 3.2s;
    static constexpr int ZoomFOV1 = 40;
    static constexpr int ZoomFOV2 = 10;
    static constexpr const char *V_Model = "models/z4b/v_xm2010pc.mdl";
    static constexpr const char *P_Model = "models/z4b/p_xm2010pc.mdl";
    static constexpr const char *W_Model = "models/z4b/w_xm2010pc.mdl";
    static constexpr const char *EventFile = "events/xm2010pc.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "z4b_xm2010pc";
    static constexpr const char *AnimExtension = "rifle";
    static constexpr int MaxClip = 5;
	enum
	{
		ANIM_IDLE1 = 0,
		ANIM_SHOOT1,
		ANIM_SHOOT2,
		ANIM_SHOOT3,
		ANIM_RELOAD,
		ANIM_DRAW,
	};
    static constexpr float MaxSpeed = 260;
    static constexpr float ArmorRatioModifier = 1.6f;
    static constexpr const auto &SpreadCalcNotOnGround = 0.05*A + 0.05;
    static constexpr const auto &SpreadCalcWalking = 0.042*A + 0.02;
    static constexpr const auto &SpreadCalcDefault = 0.037*A + 0.017;
    static constexpr const auto &CycleTime = 1.6s;
    static constexpr int DamageDefault = 286;
    static constexpr int DamageZB = 386;
    static constexpr int DamageZBS = 686;
    static constexpr float RangeModifier = 0.94;
    static constexpr auto BulletType = BULLET_PLAYER_556MM;
    static constexpr int Penetration = 1;
	std::array<float, 3> RecoilPunchAngleDelta = { -2, 0, 0 };
    KnockbackData KnockBack = { 4000.0f, 600.0f, 1000.0f, 600.0f, 0.4f };

	public:
	void Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
	{
		if (m_pPlayer->pev->fov != 90)
		{
			m_pPlayer->m_bResumeZoom = true;
			m_pPlayer->m_iLastZoom = m_pPlayer->m_iFOV;
			m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 90;
		}
		else // not zoomed, so bullets diverts more
			flSpread += 0.08;
		
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
		m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

		const float flDistance = 8192;
		const int iPenetration = Penetration;
		const Bullet iBulletType = BulletType;
		const int iDamage = GetDamage();
		const float flRangeModifier = RangeModifier;
		const BOOL bPistol = ItemSlot == PISTOL_SLOT;
		
		const Vector vecSrc = m_pPlayer->GetGunPosition();
		//Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, flDistance, iPenetration, iBulletType, iDamage, flRangeModifier, m_pPlayer->pev, bPistol, m_pPlayer->random_seed);

		float x, y, z;
		do
		{
			x = RANDOM_FLOAT(-0.5, 0.5) + RANDOM_FLOAT(-0.5, 0.5);
			y = RANDOM_FLOAT(-0.5, 0.5) + RANDOM_FLOAT(-0.5, 0.5);
			z = x * x + y * y;
		} while (z > 1);
		const Vector vecDir = gpGlobals->v_forward + x * flSpread * gpGlobals->v_right + y * flSpread * gpGlobals->v_up;
		
#ifndef CLIENT_DLL
		UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
		Vector vecSrcA = m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 10 + gpGlobals->v_right * 5;
		Vector vecSrcB = m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 10 + -gpGlobals->v_right * (-5);
		
		CXM2010PC_Crystal* pEnt = static_cast<CXM2010PC_Crystal*>(CBaseEntity::Create("z4b_xm2010pc_crystal", vecSrcA, m_pPlayer->pev->v_angle, ENT(m_pPlayer->pev)));
		if (pEnt)
		{
			pEnt->Init(gpGlobals->v_forward * 800, GetDamage(), m_pPlayer);
		}
#endif
		
		FireEvent(vecDir);

		m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
		m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

#ifndef CLIENT_DLL
		if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + flCycleTime + 1.9s;

		Recoil();
	}

	void FireEvent(const Vector& vecDir)
	{
		int flags = 0;
#ifdef CLIENT_WEAPONS
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif

		PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, (float*)&g_vecZero, (float*)&g_vecZero, vecDir.x, vecDir.y, static_cast<int>(m_pPlayer->pev->punchangle.x * 100), static_cast<int>(m_pPlayer->pev->punchangle.y * 100), FALSE, FALSE);
	}
};
LINK_ENTITY_TO_CLASS(z4b_xm2010pc, CXM2010PC)
}