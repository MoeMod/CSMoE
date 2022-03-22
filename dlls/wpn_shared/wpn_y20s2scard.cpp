/*
y20s2scard.cpp
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

struct Y20s2SCARD_WeaponData
{

	static constexpr const auto &DefaultReloadTime = 2.0s;
	static constexpr int ZoomFOV = 90;
	static constexpr const char* V_Model = "models/v_y20s2scard.mdl";
	static constexpr const char* P_Model = "models/p_y20s2scard.mdl";
	static constexpr const char* W_Model = "models/w_y20s2scard.mdl";
	static constexpr const char *EventFile = "events/y20s2scard.sc";
	static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
	static constexpr const char *ClassName = "weapon_y20s2scard";
	static constexpr const char *AnimExtension = "rifle";
	static constexpr int MaxClip = 45;
	enum
	{
		ANIM_IDLE1 = 0,
		ANIM_RELOAD,
		ANIM_DRAW,
		ANIM_SHOOT1,
		ANIM_SHOOT2,
		ANIM_SHOOT3,
		ANIM_SHOOTB,
	};
	static constexpr float PlayerMaxSpeed = 240;
	static constexpr float ArmorRatioModifier = 1.45f;
	static constexpr const auto& SpreadCalcNotOnGround = 0.4 * A + 0.035;
	static constexpr const auto& SpreadCalcWalking = 0.025 * A + 0.07;
	static constexpr const auto& SpreadCalcDucking = 0.0 * A;
	static constexpr const auto& SpreadCalcDefault = 0.02 * A;
	static constexpr const auto &CycleTime = 0.0875s;
	static constexpr int DamageDefault = 21;
	static constexpr int DamageZB = 120;
	static constexpr int DamageZBS = 298;
	static constexpr const auto& AccuracyCalc = (N * N * N / 200.0) + 0.35;
	static constexpr float AccuracyDefault = 0.19;
	static constexpr float AccuracyMax = 1.25;
	static constexpr float RangeModifier = 0.96;
	static constexpr auto BulletType = BULLET_PLAYER_556MM;
	static constexpr int Penetration = 1;
	KnockbackData KnockBack = { 370.0f, 270.0f, 300.0f, 100.0f, 0.55f };

	KickBackData KickBackWalking = { 1.0f, 0.45f, 0.28f, 0.045f, 3.75f, 3.0f, 7 };
	KickBackData KickBackNotOnGround = { 1.2f, 0.5f, 0.23f, 0.15f, 5.5f, 3.5f, 6 };
	KickBackData KickBackDucking = { 0.6f, 0.3f, 0.2f, 0.0125f, 3.25f, 2.0f, 7 };
	KickBackData KickBackDefault = { 0.65f, 0.35f, 0.25f, 0.015f, 3.5f, 2.25f, 7 };


	static constexpr auto RadiusDamageRadius = 90;
	KnockbackData RadiusDamageKnockback = { 350.0f, 250.0f, 300.0f, 100.0f, 0.6f };
};

#ifndef CLIENT_DLL
	class CY20S2SCARDGrenade : public CBaseEntity, public Y20s2SCARD_WeaponData
	{
	public:
		void Spawn() override
		{
			Precache();

			SetThink(&CY20S2SCARDGrenade::FlyThink);
			SetTouch(&CY20S2SCARDGrenade::OnTouch);
			SET_MODEL(this->edict(), "models/y20s2sacr4_grenade.mdl");

			pev->rendermode = kRenderNormal;
			pev->renderfx = kRenderFxNone;
			pev->renderamt = 255;
			pev->framerate = 1.0;
			pev->solid = SOLID_TRIGGER;
			pev->movetype = MOVETYPE_FLY;

			m_flRemoveTime = gpGlobals->time + 4.0s;
			UTIL_SetSize(pev, g_vecZero, g_vecZero);

			pev->classname = MAKE_STRING("weapon_y21s1jetgunmd");

			pev->nextthink = gpGlobals->time + 0.01s;
		}

		void Precache() override
		{
			PRECACHE_MODEL("models/y20s2sacr4_grenade.mdl");

			m_ExpIndex = PRECACHE_MODEL("sprites/ef_y20s2scar.spr");

			PRECACHE_SOUND("weapons/y20s2scard-2_exp.wav");
		}

		bool CanAttack(CBaseEntity* pOther)
		{
			if (pOther->IsBSPModel())
				return false;

			if (FNullEnt(pOther->edict()))
				return false;

			if (pOther->IsDormant())
				return false;

			if (pOther->pev->takedamage == DAMAGE_NO)
				return false;

			if (pOther->IsPlayer())
				return true;

			return false;
		}

		void EXPORT OnTouch(CBaseEntity* pOther)
		{
			if (pev->owner == pOther->edict())
				return;

			if (CanAttack(pOther))
			{
				CBaseEntity* pAttacker = CBaseEntity::Instance(pev->owner);
				CBasePlayer* pAttackePlayer = nullptr;
				if (pAttacker && pAttacker->IsPlayer())
					pAttackePlayer = static_cast<CBasePlayer*>(pAttacker);

				if (pAttackePlayer)
				{
					if (pOther->pev->takedamage != DAMAGE_NO && pOther->IsAlive())
					{
						Vector vecDirection = (pOther->pev->origin - pev->origin).Normalize();

						TraceResult tr;
						UTIL_TraceLine(pev->origin, pOther->pev->origin, missile, ENT(pAttackePlayer->pev), &tr);

						ClearMultiDamage();
						pOther->TraceAttack(pAttackePlayer->pev, m_flTouchDamage, vecDirection, &tr, DMG_BULLET);
						ApplyMultiDamage(pAttackePlayer->pev, pAttackePlayer->pev);

						CBasePlayer* player = (CBasePlayer*)pOther;

						if (player->m_bIsZombie)
						{
							Vector vecDir = (player->pev->origin - pAttackePlayer->pev->origin);
							vecDir.z = 0;

							ApplyKnockbackData(player, vecDir, { 350.0f, 250.0f, 300.0f, 100.0f, 0.6f });
						}
					}
					RadiusDamage();

					if (pOther->IsPlayer())
					{
						if (m_iPenetrationTime > -1)
						{
							CY20S2SCARDGrenade* pEnt = static_cast<CY20S2SCARDGrenade*>(CBaseEntity::Create("y20s2scard_grenade", pOther->pev->origin, g_vecZero, pev->owner));
							if (pEnt)
							{
								float flExplodeDamage = GetGrenadeExpDamage();
								pEnt->Init(0, 0, flExplodeDamage, -1, pOther->edict());
							}

						}
					}
				}

				if (m_iPenetrationTime < 5)
				{
					m_iPenetrationTime++;
				}
			}
			else
			{
				RadiusDamage();
				Remove();
				return;
			}				
		}

		void CreateEffect()
		{
			MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
			WRITE_BYTE(TE_EXPLOSION);
			WRITE_COORD(pev->origin[0]);
			WRITE_COORD(pev->origin[1]);
			WRITE_COORD(pev->origin[2]);
			WRITE_SHORT(m_ExpIndex);
			WRITE_BYTE(6);
			WRITE_BYTE(15);
			WRITE_BYTE(TE_EXPLFLAG_NOPARTICLES | TE_EXPLFLAG_NODLIGHTS | TE_EXPLFLAG_NOSOUND);
			MESSAGE_END();

			EMIT_SOUND_DYN(ENT(this->pev), CHAN_ITEM, "weapons/y20s2scard-2_exp.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		}

		void RadiusDamage()
		{
			CBaseEntity* pAttacker = CBaseEntity::Instance(pev->owner);
			CBasePlayer* pAttackePlayer = nullptr;
			if (pAttacker && pAttacker->IsPlayer())
				pAttackePlayer = static_cast<CBasePlayer*>(pAttacker);

			CreateEffect();
			TraceResult tr;
			CBaseEntity* pEntity = NULL;

			Vector vecEnd;
			int bInWater = (UTIL_PointContents(pev->origin) == CONTENTS_WATER);

			while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, 90.0)) != NULL)
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

				pEntity->TakeDamage(pev, pAttackePlayer->pev, flCurrentDamage, DMG_EXPLOSION);

				if (pEntity->IsPlayer())
				{
					CBasePlayer* player = (CBasePlayer*)pEntity;

					if (player->m_bIsZombie)
					{
						Vector vecDir = (player->pev->origin - pAttackePlayer->pev->origin);
						vecDir.z = 0;

						ApplyKnockbackData(player, vecDir, { 800.0f, 250.0f, 300.0f, 100.0f, 0.6f });
					}
				}
			}
		}


		void EXPORT FlyThink()
		{
			if (m_iPenetrationTime == -1)
			{
				//exp in 1 miniute
				if (m_pAttachedEnt)
				{
					pev->movetype = MOVETYPE_FOLLOW;
					pev->solid = SOLID_NOT;
					pev->effects |= EF_NODRAW;
					pev->aiment = m_pAttachedEnt;
					m_flRemoveTime = gpGlobals->time + 0.8s;
					m_iPenetrationTime = -2;
				}	
			}
			
			if (m_flRemoveTime <= gpGlobals->time || m_iPenetrationTime >= 5)
			{
				RadiusDamage();
				Remove();
				return;
			}

			pev->nextthink = gpGlobals->time + 0.1s;
		}

		float GetGrenadeExpDamage() const
		{
			float flDamage = 20;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 415;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 1000;
#endif
			return flDamage;
		}

		void Init(Vector vecVelocity, float flTouchDamage, float flExplodeDamage, int iPenetrationTime, edict_t *pAttachedEnt)
		{
			m_flTouchDamage = flTouchDamage;
			RadiusDamageAmount = flExplodeDamage;
			pev->velocity = vecVelocity;
			m_iPenetrationTime = iPenetrationTime;
			m_pAttachedEnt = pAttachedEnt;
		}

		KnockbackData GetKnockBackData() override { return { 0.0f, 0.0f, 0.0f, 0.0f, 1.0f }; }

		edict_t* m_pAttachedEnt;
		int m_iPenetrationTime;
		time_point_t m_flRemoveTime;
		float m_flTouchDamage;
		float RadiusDamageAmount;
		int m_ExpIndex;

	protected:
		void Remove()
		{
			SetThink(nullptr);
			SetTouch(nullptr);
			pev->effects |= EF_NODRAW; // 0x80u
			return UTIL_Remove(this);
		}
	};
	LINK_ENTITY_TO_CLASS(y20s2scard_grenade, CY20S2SCARDGrenade)
#endif

	
    class CY20S2SCARD : public LinkWeaponTemplate< CY20S2SCARD,
            TGeneralData,
			BuildTGetItemInfoFromCSW<WEAPON_AUG>::template type,
            TPrecacheEvent,
            TDeployDefault,
            TReloadDefault,
            TSecondaryAttackZoom,
            TPrimaryAttackRifle,
            TFireRifle,
			TRecoilKickBack,
            TWeaponIdleDefault,
            TGetDamageDefault
    >, public Y20s2SCARD_WeaponData
    {
    public:

	int m_iDefaultAmmo2;
	int m_iSprEffect;
	int m_iShootCount;

	float GetGrenadeTouchDamage() const
	{
		float flDamage = 10;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 800;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 1000;
#endif
		return flDamage;
	}

	float GetGrenadeExpDamage() const
	{
		float flDamage = 30;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 950;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 1500;
#endif
		return flDamage;
	}

	int GetItemInfo(ItemInfo* p) override
	{
		p->pszName = STRING(pev->classname);
		p->pszAmmo1 = "762Nato";
		p->iMaxAmmo1 = MAX_AMMO_762NATO;
		p->pszAmmo2 = "SCARDGrenade";
		p->iMaxAmmo2 = 10;
		p->iMaxClip = 45;
		p->iSlot = 0;
		p->iPosition = 14;
		p->iId = m_iId = WEAPON_AUG;
		p->iFlags = 0;
		p->iWeight = AUG_WEIGHT;

		return 1;
	}

	void Spawn(void)
	{
		pev->classname = MAKE_STRING("weapon_y20s2scard");

		Precache();
		m_iId = WEAPON_AUG;
		SET_MODEL(ENT(pev), W_Model);

		m_iDefaultAmmo = 45;
		m_flAccuracy = 0.2;
		m_iShotsFired = 0;
		m_iShootCount = 0;
		m_iDefaultAmmo2 = 10;

		FallInit();
	}

	void AddHitEffectsToLineTargets(Vector vecSrc, Vector vecDir)
	{
#ifndef CLIENT_DLL
		TraceResult tr;

		UTIL_TraceLine(vecSrc, vecSrc + vecDir * 8192.0, dont_ignore_monsters, m_pPlayer->edict(), &tr);

		if (tr.flFraction < 1.0 && !FNullEnt(tr.pHit))
		{
			CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);
			if (pEntity && g_pGameRules->PlayerRelationship(m_pPlayer, pEntity) != GR_TEAMMATE)
			{
				MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
				WRITE_BYTE(TE_EXPLOSION);
				WRITE_COORD(pEntity->pev->origin[0]);
				WRITE_COORD(pEntity->pev->origin[1]);
				WRITE_COORD(pEntity->pev->origin[2]);
				WRITE_SHORT(m_iSprEffect);
				WRITE_BYTE(3);
				WRITE_BYTE(15);
				WRITE_BYTE(TE_EXPLFLAG_NOPARTICLES | TE_EXPLFLAG_NODLIGHTS | TE_EXPLFLAG_NOSOUND);
				MESSAGE_END();
			}
		}
#endif
	}

	void PrimaryAttack(void) override
	{
		Base::PrimaryAttack();

		m_iShootCount++;
		if (m_iShootCount >= 15 && m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] < 10)
		{
			m_iShootCount = 0;
			m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]++;
		}

		UTIL_MakeVectors(m_pPlayer->pev->punchangle + m_pPlayer->pev->v_angle);

		Vector vecSrc = m_pPlayer->GetGunPosition();
		Vector vecDirShooting = gpGlobals->v_forward;

		AddHitEffectsToLineTargets(vecSrc, vecDirShooting);
	}

	void SecondaryAttack(void) override
	{
		FireGrenade(0, 0.7s, FALSE);
	}

	void FireEvent(const Vector& vecDir)
	{
		int flags = 0;
#ifdef CLIENT_WEAPONS
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif
		PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, static_cast<int>(m_pPlayer->pev->punchangle.x * 100), static_cast<int>(m_pPlayer->pev->punchangle.y * 100), TRUE, FALSE);
	}

	void Precache() override
	{
		Base::Precache();

		m_iSprEffect = PRECACHE_MODEL("sprites/ef_y20s2scarhit.spr");
#ifndef CLIENT_DLL
		PRECACHE_MODEL("sprites/muzzleflash135.spr");
#endif
	}

	int ExtractAmmo(CBasePlayerWeapon* pWeapon) override
	{
		if (m_iDefaultAmmo2)
		{
			m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = m_iDefaultAmmo2;
			m_iDefaultAmmo2 = 0;
		}
		return CBasePlayerWeapon::ExtractAmmo(pWeapon);
	}

	void FireGrenade(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
	{
		m_bDelayFire = true;
		m_iShotsFired++;
		m_flAccuracy = AccuracyCalc(N = m_flAccuracy);

		if (m_flAccuracy > AccuracyMax)
			m_flAccuracy = AccuracyMax;

		if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] <= 0)
		{
			if (m_fFireOnEmpty)
			{
				PlayEmptySound();
				m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.2s;
			}

			return;
		}

		m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]--;

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
		CY20S2SCARDGrenade* pEnt = static_cast<CY20S2SCARDGrenade*>(CBaseEntity::Create("y20s2scard_grenade", vecSrc, m_pPlayer->pev->v_angle, ENT(m_pPlayer->pev)));
		if (pEnt)
		{
			float flTouchDamage = GetGrenadeTouchDamage();
			float flExplodeDamage = GetGrenadeExpDamage();
			pEnt->Init(gpGlobals->v_forward * 900, flTouchDamage, flExplodeDamage, 0, NULL);
		}
#endif

		m_pPlayer->pev->effects |= EF_MUZZLEFLASH;

		PLAYBACK_EVENT_FULL(0, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), FALSE, FALSE);

		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + flCycleTime + 0.2s;

		if (m_pPlayer->pev->velocity.Length2D() > 0)
			KickBack(0.7, 0.275, 0.425, 0.0335, 3.5, 2.5, 8);
		else if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			KickBack(1.5, 0.9, 0.4, 0.3, 7.0, 4.5, 8);
		else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
			KickBack(0.5, 0.12, 0.24, 0.001, 3.15, 2.1, 6);
		else
			KickBack(0.595, 0.225, 0.32, 0.0115, 3.25, 2.1, 7);
	}
};
LINK_ENTITY_TO_CLASS(weapon_y20s2scard, CY20S2SCARD)
}