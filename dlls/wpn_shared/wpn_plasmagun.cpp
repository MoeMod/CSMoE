/*
z4b_m4a1mw.cpp
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

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

using WeaponTemplate::Varibles::A;
using WeaponTemplate::Varibles::N;

struct PlasmaGun_WeaponData
{
	static constexpr const char* V_Model = "models/v_plasmagun.mdl";
	static constexpr const char* P_Model = "models/p_plasmagun.mdl";
	static constexpr const char* W_Model = "models/w_plasmagun.mdl";
	static constexpr const char *EventFile = "events/plasmagun.sc";
	static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
	static constexpr const char *ClassName = "weapon_plasmagun";
	static constexpr const char *AnimExtension = "ak47";
	static constexpr int MaxClip = 45;
	enum
	{
		ANIM_IDLE1 = 0,
		ANIM_RELOAD,
		ANIM_DRAW,
		ANIM_SHOOT1,
		ANIM_SHOOT2,
		ANIM_SHOOT3
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

#ifndef CLIENT_DLL
	class CPlasmaGrenade : public CBaseEntity, public PlasmaGun_WeaponData
	{
	public:
		float GetArmorRatioModifier() { return 1.85; }
		int GetWeaponsId(void) { return WEAPON_PLASMAGUN; }
		void Spawn() override
		{
			Precache();

			m_fSequenceLoops = 0;
			//ph26 = 0;
			SetThink(&CPlasmaGrenade::FollowThink);
			SetTouch(&CPlasmaGrenade::OnTouch);
			SET_MODEL(this->edict(), "sprites/plasmaball.spr");

			//ph32 = ?
			pev->rendermode = kRenderTransAdd;
			pev->renderfx = kRenderFxNone;
			pev->renderamt = 180;
			pev->framerate = 10;
			pev->scale = 0.25;
			pev->solid = SOLID_BBOX;
			pev->movetype = MOVETYPE_FLYMISSILE;
			pev->nextthink = gpGlobals->time + 0.1s;

			m_flMaxFrames = float(MODEL_FRAMES(pev->modelindex) - 1);

			UTIL_SetOrigin(pev, pev->origin);
			UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));

			pev->classname = MAKE_STRING("plasma_grenade");
		}

		void Precache() override
		{
			PRECACHE_MODEL("sprites/plasmabomb.spr");
			PRECACHE_MODEL("sprites/plasmaball.spr");
			m_iSprExplo = PRECACHE_MODEL("sprites/plasmabomb.spr");
		}

		void EXPORT OnTouch(CBaseEntity* pOther)
		{
			MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
			WRITE_BYTE(TE_SPRITE);
			WRITE_COORD(pev->origin.x);
			WRITE_COORD(pev->origin.y);
			WRITE_COORD(pev->origin.z);
			WRITE_SHORT(m_iSprExplo);	// sprite index
			WRITE_BYTE(5);			// scale in 0.1's
			WRITE_BYTE(255);			// brightness
			MESSAGE_END();

			EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "weapons/plasmagun_exp.wav", VOL_NORM, 0.25, 0, PITCH_NORM);

			if (pev->owner == pOther->edict())
			{
				SetTouch(NULL);
				SetThink(NULL);
				Remove();
				return;
			}
				

			CBaseEntity* pAttacker = CBaseEntity::Instance(pev->owner);
			CBasePlayer* pAttackePlayer = nullptr;
			if (pAttacker && pAttacker->IsPlayer())
				pAttackePlayer = static_cast<CBasePlayer*>(pAttacker);

			if (pAttackePlayer)
			{
				if (pOther->pev->takedamage != DAMAGE_NO &&pOther->IsAlive())
				{
					Vector vecDirection = (pOther->pev->origin - pev->origin).Normalize();

					TraceResult tr;
					UTIL_TraceLine(pev->origin, pOther->pev->origin, missile, ENT(pAttackePlayer->pev), &tr);
					tr.iHitgroup = HITGROUP_CHEST; // ...

					ClearMultiDamage();
					pOther->TraceAttack(pAttackePlayer->pev, m_flTouchDamage, vecDirection, &tr, DMG_GENERIC);
					ApplyMultiDamage(this->pev, pAttackePlayer->pev);
				}

				TraceResult tr;
				CBaseEntity* pEntity = NULL;

				Vector vecEnd;
				int bInWater = (UTIL_PointContents(pev->origin) == CONTENTS_WATER);

				while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, 1.75 * 39.37)) != NULL)
				{
					if (pEntity->edict() == pev->owner)
						continue;

					if (pEntity->IsDormant())
						continue;

					if (pEntity->pev->takedamage == DAMAGE_NO)
						continue;

					if (pEntity->Classify() == CLASS_NONE)
						continue;

					if (pEntity->IsPlayer())
					{
						CBasePlayer* target = static_cast<CBasePlayer*>(pEntity);

						if (pAttackePlayer->m_iTeam == target->m_iTeam)
								continue;
					}

					if ((bInWater && pEntity->pev->waterlevel == 0) || (!bInWater && pEntity->pev->waterlevel == 3))
						continue;

					vecEnd = pEntity->BodyTarget(pev->origin);

					UTIL_TraceLine(pev->origin, vecEnd, ignore_monsters, pev->owner, &tr);

					if (tr.flFraction < 1.0 && tr.pHit != pEntity->edict())
						continue;

					int falloff = RadiusDamageAmount / 90.0;

					float flCurrentDamage = max(0, RadiusDamageAmount - (pev->origin - pEntity->pev->origin).Length() * falloff);

					pEntity->TakeDamage(pev, pAttackePlayer->pev, flCurrentDamage, DMG_GENERIC);

					/*if (pEntity->IsPlayer())
					{
						CBasePlayer* player = (CBasePlayer*)pEntity;

						if (player->m_bIsZombie)
						{
							Vector vecDir = (pev->origin - pAttackePlayer->pev->origin).Normalize();

							player->pev->velocity.x = vecDir.x * 1.0 * 1200.0;
							player->pev->velocity.y = vecDir.y * 1.0 * 1200.0;
							player->pev->velocity.z = 1.0 * 100.0;
						}
					}*/
				}
			}

			SetTouch(NULL);
			SetThink(NULL);
			Remove();
		}

		void EXPORT FollowThink()
		{
			pev->frame += pev->framerate * gpGlobals->frametime / 1s;

			if (pev->frame > m_flMaxFrames)
				pev->frame = fmod(pev->frame, m_flMaxFrames);

			pev->nextthink = gpGlobals->time + 0.01s;
		}

		void Init(Vector vecVelocity, float flTouchDamage, float flExplodeDamage, int iTeam, int type = 0)
		{
			std::tie(m_flTouchDamage, RadiusDamageAmount, m_iTeam) = std::make_tuple(flTouchDamage, flExplodeDamage, iTeam);
			m_vecStartVelocity = pev->velocity = std::move(vecVelocity);
			m_iType = type;
		}

		KnockbackData GetKnockBackData() override { return { 500.0f, 250.0f, 450.0f, 300.0f, 0.7f }; }

		int m_fSequenceLoops;
		time_point_t m_flRemoveTime;
		float m_flMaxFrames;
		float m_flTouchDamage;
		float RadiusDamageAmount;
		int m_iSprEffect;
		int m_iSprExplo;
		Vector m_vecStartVelocity;
		int m_iType;

	protected:
		void Remove()
		{
			SetThink(nullptr);
			SetTouch(nullptr);
			pev->effects |= EF_NODRAW; // 0x80u
			return UTIL_Remove(this);
		}
	};
	LINK_ENTITY_TO_CLASS(plasma_grenade, CPlasmaGrenade)
#endif

	
    class CPlasmaGun : public LinkWeaponTemplate< CPlasmaGun,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_AK47>::template type,
            TPrecacheEvent,
            TDeployDefault,
            TWeaponIdleDefault
    >, public PlasmaGun_WeaponData
    {
    public:

	void Precache(void) override
	{
		Base::Precache();

		PRECACHE_MODEL("sprites/muzzleflash27.spr");

	}

#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_plasma" , 200 }; }
#endif

	void PrimaryAttack(void) override
	{
		duration_t flCycleTime;
		if (m_pPlayer->m_iFOV != 90)
			flCycleTime = 0.165s;
		else
			flCycleTime = 0.15s;

		if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			FireLaserBomb((0.4) * m_flAccuracy + 0.04, flCycleTime, FALSE);
		else if (m_pPlayer->pev->velocity.Length2D() > 120)
			FireLaserBomb((0.07) * m_flAccuracy + 0.04, flCycleTime, FALSE);
		else if (m_pPlayer->pev->fov != 90)
			FireLaserBomb((0.0) * m_flAccuracy + 0.0, flCycleTime, FALSE);
		else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
			FireLaserBomb((0.001) * m_flAccuracy + 0.001, flCycleTime, FALSE);
		else
			FireLaserBomb((0.03) * m_flAccuracy + 0.002, flCycleTime, FALSE);

	}


	void Spawn(void) override
	{
		pev->classname = MAKE_STRING("weapon_plasmagun");

		Precache();
		m_iId = WEAPON_PLASMAGUN;
		SET_MODEL(ENT(pev), "models/w_plasmagun.mdl");
		m_flAccuracy = 0.25;
		m_iShotsFired = 0;
		m_iDefaultAmmo = 45;

		FallInit();
	}

	int GetItemInfo(ItemInfo* p) override
	{
		p->pszName = STRING(pev->classname);
		p->pszAmmo1 = "plasma";
		p->iMaxAmmo1 = 90;
		p->pszAmmo2 = NULL;
		p->iMaxAmmo2 = -1;
		p->iMaxClip = 45;
		p->iSlot = 0;
		p->iPosition = 2;
		p->iId = m_iId = WEAPON_PLASMAGUN;
		p->iFlags = 0;
		p->iWeight = AUG_WEIGHT;

		return 1;
	}

	WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 4, 4, 140 }; }

	void Reload(void) override
	{
		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			return;

		if (DefaultReload(45, ANIM_RELOAD, 3.3s))
		{
#ifndef CLIENT_DLL
			m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
			if (m_pPlayer->m_iFOV != 90)
				SecondaryAttack();

			m_flAccuracy = 0.25;
			m_iShotsFired = 0;
			m_bDelayFire = false;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0s;
		}
	}


	void SecondaryAttack(void) override
	{
		switch (m_pPlayer->m_iFOV)
		{
		case 90: m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 65; break;
		default: m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 90; break;
		}

		m_pPlayer->ResetMaxSpeed();
		EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_ITEM, "weapons/zoom.wav", 0.2, 2.4, 0, PITCH_NORM);

		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3s;
	}


	float GetMaxSpeed(void) override
	{
		if (m_pPlayer->pev->fov == 90)
			return 220;

		return 195;
	}
    	
	void FireLaserBomb(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
	{
		m_iShotsFired++;
		m_flAccuracy = AccuracyCalc(N = m_flAccuracy);

		if (m_flAccuracy > AccuracyMax)
			m_flAccuracy = AccuracyMax;

		if (m_iClip <= 0)
		{
			if (m_fFireOnEmpty)
			{
				PlayEmptySound();
				m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2s;
			}

			return;
		}

		--m_iClip;

#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
		
		UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

		m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;	// 1000
		m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;	// 512

		Vector vecDir;

#ifndef CLIENT_DLL
		UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
		Vector vecSrc = m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 16 + gpGlobals->v_right * 8 + gpGlobals->v_up * -8;
		CPlasmaGrenade* pEnt = static_cast<CPlasmaGrenade*>(CBaseEntity::Create("plasma_grenade", vecSrc, m_pPlayer->pev->v_angle, ENT(m_pPlayer->pev)));
		if (pEnt)
		{
			float flTouchDamage = g_pModRunning->DamageTrack() == DT_NONE ? 51 : 242;	//119
			float flExplodeDamage = g_pModRunning->DamageTrack() == DT_NONE ? 30 : 105;	//55
			pEnt->Init(gpGlobals->v_forward * 2000, flTouchDamage, flExplodeDamage, m_pPlayer->m_iTeam);
		}
#endif

		m_pPlayer->pev->effects |= EF_MUZZLEFLASH;

		int flags;
#ifdef CLIENT_WEAPONS
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif

		PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), TRUE, FALSE);

		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
#ifndef CLIENT_DLL
		if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + flCycleTime + 0.2s;

		if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			KickBack(1.2, 0.5, 0.23, 0.15, 5.5, 3.5, 6);
		else if (m_pPlayer->pev->velocity.Length2D() > 0)
			KickBack(1.0, 0.45, 0.28, 0.045, 3.75, 3.0, 7);
		else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
			KickBack(0.6, 0.3, 0.2, 0.0125, 3.25, 2.0, 7);
		else
			KickBack(0.65, 0.35, 0.25, 0.015, 3.5, 2.25, 7);
	}
};
LINK_ENTITY_TO_CLASS(weapon_plasmagun, CPlasmaGun)
}