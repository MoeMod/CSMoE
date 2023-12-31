/*
petrolboomer.cpp
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
#include "decals.h"


#ifndef CLIENT_DLL
namespace sv {

	class CPetrolMine : public CBaseEntity
	{
	public:
		int GetWeaponsId(void) override { return WEAPON_PETROLBOOMER; }
		KnockbackData GetKnockBackData() override { return  { 375.0f, 270.0f, 300.0f, 120.0f, 0.5f }; }
		float GetArmorRatioModifier() override { return 1.5; }

		void Spawn() override
		{
			pev->movetype = MOVETYPE_TOSS;
			pev->solid = SOLID_BBOX;
			pev->gravity = 0.5;
			pev->friction = 0.0;
			pev->takedamage = DAMAGE_NO;
			pev->max_health = 100;
			pev->health = 100;

			SET_MODEL(ENT(pev), "models/null.mdl");
			UTIL_SetSize(pev, Vector(0.0, 0.0, 0.0), Vector(0.0, 0.0, 0.0));
			UTIL_SetOrigin(pev, pev->origin);

			SetThink(&CPetrolMine::PetrolMine_Think);
			pev->nextthink = gpGlobals->time + 0.1s;

			SetTouch(NULL);
			m_fLaunchTime = gpGlobals->time;
			m_iPartEmitCount = 0;
		}

	public:
		void EXPORT PetrolMine_Think(void)
		{
			if (gpGlobals->time >= m_fLaunchTime + 5.5s)
			{
				SetThink(NULL);
				SetTouch(NULL);

				UTIL_Remove(this);

				return;
			}

			int flags = PARTICLE_ANIMATE | PARTICLE_ANIMATELOOP | PARTICLE_RANDOMANGLE;

			switch (m_iPartEmitCount)
			{
			case 0:
				MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
				WRITE_BYTE(TE_SMOKE);
				WRITE_COORD(pev->origin.x);
				WRITE_COORD(pev->origin.y);
				WRITE_COORD(pev->origin.z - 5.0);
				WRITE_SHORT(g_sModelIndexSmoke);
				WRITE_BYTE(int(35.0 + RANDOM_FLOAT(0.0, 10.0)));
				WRITE_BYTE(5);
				MESSAGE_END();

				UTIL_TempEmit(
					"sprites/burn7.spr",
					-1,
					20,
					3,
					flags,
					0,
					15,
					kRenderTransAdd,
					150.0,
					255.0,
					255.0,
					255.0,
					Vector(0.0, 0.0, 0.0),
					Vector(0.0, 0.0, 0.0),
					RANDOM_LONG(700, 1200),
					RANDOM_LONG(7, 9),
					0,
					RANDOM_LONG(5, 7),
					0,
					entindex(),
					0,
					0
				);

				break;

			case 1:
				UTIL_TempEmit(
					"sprites/burn5.spr",
					-1,
					20,
					3,
					flags,
					0,
					15,
					kRenderTransAdd,
					150.0,
					255.0,
					255.0,
					255.0,
					Vector(-50.0, -50.0, 0.0),
					Vector(0.0, 0.0, 10.0),
					RANDOM_LONG(700, 1200),
					RANDOM_LONG(7, 9),
					0,
					RANDOM_LONG(5, 7),
					0,
					entindex(),
					0,
					0
				);

				break;

			case 2:
				UTIL_TempEmit(
					"sprites/burn6.spr",
					-1,
					20,
					3,
					flags,
					0,
					15,
					kRenderTransAdd,
					150.0,
					255.0,
					255.0,
					255.0,
					Vector(50.0, -50.0, 0.0),
					Vector(0.0, 0.0, 10.0),
					RANDOM_LONG(700, 1200),
					RANDOM_LONG(7, 9),
					0,
					RANDOM_LONG(5, 7),
					0,
					entindex(),
					0,
					0
				);

				break;

			case 3:
				UTIL_TempEmit(
					"sprites/burn5.spr",
					-1,
					20,
					3,
					flags,
					0,
					15,
					kRenderTransAdd,
					150.0,
					255.0,
					255.0,
					255.0,
					Vector(-50.0, 50.0, 0.0),
					Vector(0.0, 0.0, 10.0),
					RANDOM_LONG(700, 1200),
					RANDOM_LONG(7, 9),
					0,
					RANDOM_LONG(5, 7),
					0,
					entindex(),
					0,
					0
				);

				break;

			case 4:
				UTIL_TempEmit(
					"sprites/burn6.spr",
					-1,
					20,
					3,
					flags,
					0,
					15,
					kRenderTransAdd,
					150.0,
					255.0,
					255.0,
					255.0,
					Vector(50.0, 50.0, 0.0),
					Vector(0.0, 0.0, 10.0),
					RANDOM_LONG(700, 1200),
					RANDOM_LONG(7, 9),
					0,
					RANDOM_LONG(5, 7),
					0,
					entindex(),
					0,
					0
				);

				break;

			default:
				break;
			}

			SetThink(&CPetrolMine::PetrolMine_Think);
			if (++m_iPartEmitCount > 5)
			{
				pev->nextthink = gpGlobals->time + 0.5s;

				Attack(false);
			}
			else
			{
				pev->nextthink = gpGlobals->time + 0.1s;
			}
		}

	public:
		void Attack(bool bExplode)
		{
			TraceResult tr;
			float flDamage;
			CBaseEntity* pList[100];
			Vector vecStart;

			vecStart = pev->origin + Vector(0.0, 0.0, 1.0);

			const float flRange = 2.5 * 39.37;
			int iCount = UTIL_EntitiesInBox(pList, 100, vecStart - Vector(flRange, flRange, flRange), vecStart + Vector(flRange, flRange, flRange), FL_CLIENT | FL_MONSTER);

			if (g_pModRunning->DamageTrack() == DT_ZBS)
			{
				flDamage = bExplode ? 800.0 : 115.0;
			}
			else if (g_pModRunning->DamageTrack() == DT_ZB)
			{
				flDamage = bExplode ? 150.0 : 125.0;
			}
			else
			{
				flDamage = 10.0;
			}

			CBasePlayer* pOwner = (CBasePlayer*)CBaseEntity::Instance(pev->owner);

			if (pOwner)
			{
				if (pOwner->IsPlayer() && pOwner->m_bIsZombie)
					flDamage = 0.0;

				for (int i = 0; i < iCount; i++)
				{
					CBaseEntity* pEntity = pList[i];

					if (pEntity == this)
						continue;

					if (pEntity->pev->takedamage == DAMAGE_NO)
						continue;

					if (pEntity->edict() == pev->owner)
						continue;

					if (g_pGameRules->PlayerRelationship(pOwner, pEntity) == GR_TEAMMATE)
						continue;

					UTIL_TraceLine(vecStart, pEntity->BodyTarget(vecStart), dont_ignore_monsters, edict(), &tr);

					if (tr.flFraction < 1.0 && tr.pHit != pEntity->edict())
						continue;

					pEntity->TakeDamage(pev, VARS(pev->owner), flDamage, DMG_GENERIC);
				}
			}
		}

	public:
		time_point_t m_fLaunchTime;
		int m_iPartEmitCount;

	};

	LINK_ENTITY_TO_CLASS(petrol_mine, CPetrolMine);

	class CPetrolRocket : public CBaseEntity
	{
	public:
		KnockbackData GetKnockBackData() override { return  { 375.0f, 270.0f, 300.0f, 120.0f, 0.5f }; }
		int GetWeaponsId(void) override { return WEAPON_PETROLBOOMER; }
		float GetArmorRatioModifier() override { return 1.5; }

		void Spawn() override
		{
			Precache();

			pev->movetype = MOVETYPE_BOUNCE;
			pev->solid = SOLID_BBOX;
			pev->gravity = 0.5;

			SET_MODEL(ENT(pev), "models/petrol.mdl");
			UTIL_SetOrigin(pev, pev->origin);
			UTIL_SetSize(pev, Vector(0.0, 0.0, 0.0), Vector(0.0, 0.0, 0.0));

			SetThink(&CPetrolRocket::IgniteThink);
			pev->nextthink = gpGlobals->time + 0.1s;

			SetTouch(&CPetrolRocket::RocketTouch);
			pev->gravity = 0.5;
		}

		void Precache(void)
		{
			PRECACHE_MODEL("models/petrol.mdl");
			m_iTrail = PRECACHE_MODEL("sprites/smoke.spr");
			m_iAttach = PRECACHE_MODEL("sprites/burn5.spr");
		}

		void Setup(CBasePlayer* pPlayer)
		{
			pev->owner = pPlayer->edict();
			m_iTeam = pPlayer->m_iTeam;
			pev->angles.x -= 30.0;

			UTIL_MakeVectors(pev->angles);
			pev->angles.x = -(pev->angles.x + 30.0);
			pev->velocity = gpGlobals->v_forward * 300.0;

			UTIL_MakeVectors(pPlayer->pev->v_angle);
			pev->velocity = pev->velocity + gpGlobals->v_forward * 550.0;

			EMIT_SOUND_DYN(pPlayer->edict(), CHAN_ITEM, "weapons/petrolboomer-1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		}

		void EXPORT RocketTouch(CBaseEntity* pOther)
		{
			if (!pOther)
				return;

			float flDamage = 85.0f;

			if (g_pModRunning->DamageTrack() == DT_ZBS)
			{
				flDamage = 800.0f;
			}
			else if (g_pModRunning->DamageTrack() == DT_ZB)
			{
				flDamage = 190.0f;
			}

			entvars_t* pevOwner;
			if (pev->owner)
			{
				pevOwner = VARS(pev->owner);

				CBasePlayer* pOwner = (CBasePlayer*)CBaseEntity::Instance(pev->owner);

				if (pOwner)
				{
					if (pOwner->IsPlayer() && pOwner->m_bIsZombie)
						flDamage = 0.0;

					if (pOther->pev != pevOwner && !pOther->IsDormant())
					{
						if (pOther->pev->takedamage != DAMAGE_NO)
						{
							pOther->TakeDamage(pev, pevOwner, flDamage, DMG_GENERIC);
						}
					}
				}		
			}
			else
			{
				pevOwner = NULL;
			}

			Explode();

			if (pevOwner)
			{
				CPetrolMine* pMine = (CPetrolMine*)CBaseEntity::Create("petrol_mine", pev->origin, Vector(0.0, 0.0, 0.0), pev->owner);

				pMine->m_iTeam = m_iTeam;
				pMine->Attack(true);
			}

			MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
			WRITE_BYTE(TE_KILLBEAM);
			WRITE_SHORT(this->entindex());
			MESSAGE_END();

			UTIL_Remove(this);

			SetThink(NULL);
			SetTouch(NULL);
		}

		void Explode(void)
		{
			MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
			WRITE_BYTE(TE_EXPLOSION);
			WRITE_COORD(pev->origin.x);
			WRITE_COORD(pev->origin.y);
			WRITE_COORD(pev->origin.z);
			WRITE_SHORT(g_sModelIndexFireball3);
			WRITE_BYTE(25);
			WRITE_BYTE(30);
			WRITE_BYTE(0);
			MESSAGE_END();

			MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
			WRITE_BYTE(TE_EXPLOSION);
			WRITE_COORD(pev->origin.x + RANDOM_FLOAT(-64.0, 64.0));
			WRITE_COORD(pev->origin.y + RANDOM_FLOAT(-64.0, 64.0));
			WRITE_COORD(pev->origin.z + RANDOM_FLOAT(30.0, 35.0));
			WRITE_SHORT(g_sModelIndexFireball2);
			WRITE_BYTE(30);
			WRITE_BYTE(30);
			WRITE_BYTE(0);
			MESSAGE_END();

			TraceResult tr = UTIL_GetGlobalTrace();

			if (RANDOM_FLOAT(0.0, 1.0) < 0.5)
				UTIL_DecalTrace(&tr, DECAL_SCORCH1);
			else
				UTIL_DecalTrace(&tr, DECAL_SCORCH2);

			EMIT_SOUND_DYN(ENT(pev), CHAN_ITEM, "weapons/petrolboomer_explosion.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

			switch (RANDOM_LONG(0, 2))
			{
			case 0: EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "weapons/debris1.wav", 0.55, ATTN_NORM, 0, PITCH_NORM); break;
			case 1: EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "weapons/debris2.wav", 0.55, ATTN_NORM, 0, PITCH_NORM); break;
			case 2: EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "weapons/debris3.wav", 0.55, ATTN_NORM, 0, PITCH_NORM); break;
			}
		}


		void EXPORT FollowThink(void)
		{
			pev->angles = UTIL_VecToAngles(pev->velocity);
			pev->nextthink = gpGlobals->time + 0.1s;
		}

		void EXPORT IgniteThink(void)
		{
			pev->movetype = MOVETYPE_BOUNCE;

			MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
			WRITE_BYTE(TE_BEAMFOLLOW);
			WRITE_SHORT(entindex());
			WRITE_SHORT(m_iTrail);
			WRITE_BYTE(10);
			WRITE_BYTE(5);
			WRITE_BYTE(224);
			WRITE_BYTE(224);
			WRITE_BYTE(255);
			WRITE_BYTE(255);
			MESSAGE_END();

			MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
			WRITE_BYTE(TE_BEAMPARTICLE); //defined as 32
			WRITE_SHORT(this->entindex());  // short (entity:attachment to follow)
			WRITE_SHORT(m_iAttach);// short (sprite index)
			WRITE_SHORT(m_iAttach);// short (sprite index)
			WRITE_SHORT(m_iAttach);// short (sprite index)
			WRITE_BYTE(255); //r
			WRITE_BYTE(255); //g
			WRITE_BYTE(255); //b
			WRITE_BYTE(0); //randomcolor
			WRITE_BYTE(20); //scale in 0.01's
			WRITE_BYTE(4); //freq(time of one cycle) in 0.01's
			WRITE_BYTE(3); //fadetime in 0.1s
			WRITE_BYTE(2);  //count
			WRITE_BYTE(0);  //random in org
			WRITE_BYTE(0);  //speed (if>0 do slow gravity)
			MESSAGE_END();

			SetThink(&CPetrolRocket::FollowThink);
			pev->nextthink = gpGlobals->time + 0.1s;
		}

	public:
		BOOL m_bLockingTarget;
		Vector m_vecLockingPos;
		Vector m_vecVelocity;
		Vector m_vecDirection;
		int m_iPenetration;
		int m_iTrail;
		int m_iAttach;
	};

	LINK_ENTITY_TO_CLASS(petrol_rocket, CPetrolRocket);
}
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif
    class CPetrolBoomer : public LinkWeaponTemplate< CPetrolBoomer,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_P90>::template type,
			TPrecacheEvent,
			TGetDamageDefault
    >
    {
    public:
    static constexpr const char *V_Model = "models/v_petrolboomer.mdl";
    static constexpr const char *P_Model = "models/p_petrolboomer.mdl";
    static constexpr const char *W_Model = "models/w_petrolboomer.mdl";
    static constexpr const char *EventFile = "events/petrolboomer.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_petrolboomer";
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
	static constexpr float ArmorRatioModifier = 1.5f;

    static constexpr int DamageDefault = 133;
	static constexpr int DamageZB = 2750;
    static constexpr int DamageZBS = 3750;

    KnockbackData KnockBack = { 375.0f, 270.0f, 300.0f, 120.0f, 0.5f };

	int GetItemInfo(ItemInfo* p)
	{
		p->pszName = STRING(pev->classname);
		p->pszAmmo1 = "petrolboomer";
		p->iMaxAmmo1 = -1;
		p->pszAmmo2 = NULL;
		p->iMaxAmmo2 = -1;
		p->iMaxClip = 20;
		p->iSlot = 0;
		p->iPosition = 16;
		p->iId = m_iId = WEAPON_PETROLBOOMER;
		p->iFlags = 0;
		p->iWeight = AK47_WEIGHT;

		return 1;
	}
#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_petrolboomer" , 200 }; }
#endif
	WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 6, 3, 0 }; }

	float GetArmorRatioModifier() override { return 1.5; }

	float GetMaxSpeed(void) override
	{
		return 215;
	}

	time_point_t m_flReloadTime;

	void Spawn(void)
	{
		Base::Spawn();
		m_iId = WEAPON_PETROLBOOMER;
		m_iDefaultAmmo = 20;
	}

	BOOL Deploy(void) override
	{
		m_flAccuracy = 0;
		m_iShotsFired = 0;
		m_bDelayFire = false;
		m_flReloadTime = invalid_time_point;

#ifndef CLIENT_DLL
		m_pPlayer->UpdateShieldCrosshair(false);
#endif

		if (m_iClip > 0)
			return DefaultDeploy(V_Model, P_Model, ANIM_DRAW, "carbine", UseDecrement() != FALSE);
		else
			return DefaultDeploy(V_Model, P_Model, ANIM_DRAW_EMPTY, "carbine", UseDecrement() != FALSE);
	}

	void PrimaryAttack() override
	{
		if (m_iClip <= 0)
			return;

		m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
		m_iClip--;
		
		m_flReloadTime = gpGlobals->time + 1.0s;

		UTIL_MakeVectors(m_pPlayer->pev->v_angle);

#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);

		CPetrolRocket* pRocket = (CPetrolRocket*)CBaseEntity::Create("petrol_rocket",
			m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 16.0 + gpGlobals->v_right * 8.0 + gpGlobals->v_up * -8.0,
			m_pPlayer->pev->v_angle,
			m_pPlayer->edict());

		if (pRocket)
		{
			pRocket->pev->iuser4 = m_uiWeaponPaintType;
			pRocket->Setup(m_pPlayer);
		}
			
#endif

		int flags;

#ifdef CLIENT_WEAPONS
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif

		PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0.0, g_vecZero, g_vecZero, 0.0, 0.0, m_iClip, m_iClip, FALSE, FALSE);

		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 4.7s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 4.7s;

		if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			KickBack(1.0, 1.0, 0.5, 0.5, 5.0, 5.0, 5);
		else if (m_pPlayer->pev->velocity.Length2D() > 0)
			KickBack(0.3, 0.3, 0.07, 0.07, 2.0, 2.0, 4);
		else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
			KickBack(0.2, 0.2, 0.01, 0.01, 0.5, 0.5, 4);
		else
			KickBack(0.2, 0.2, 0.01, 0.01, 0.7, 0.7, 3);
	}


	void ItemPostFrame(void)
	{
		if (m_flReloadTime != invalid_time_point && gpGlobals->time > m_flReloadTime)
		{
			SendWeaponAnim(ANIM_RELOAD);

			m_flReloadTime = invalid_time_point;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.7s;
		}
		else
			CBasePlayerWeapon::ItemPostFrame();
	}

	void WeaponIdle(void)
	{
		ResetEmptySound();

		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;

		int iAnim;

		if (m_iClip <= 0)
			iAnim = ANIM_IDLE_EMPTY;
		else
			iAnim = ANIM_IDLE1;

		SendWeaponAnim(iAnim);

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20.0s;
	}

	
};

LINK_ENTITY_TO_CLASS(weapon_petrolboomer, CPetrolBoomer)
}