/*
knife_dvhammer.cpp
Copyright (C) 2019 Moemod Hyakuya

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
#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif
#include "weapons/RadiusDamage.hpp"
#include "weapons/WeaponTemplate.hpp"
#include "weapons/KnifeAttack.h"

#ifndef CLIENT_DLL
namespace sv {

#define SPIKE_TIME	 6s

	class CCKnifeDvhammer_Spike : public CBaseEntity
	{
	public:
		void Spawn(void);
		int GetWeaponsId(void) { return WEAPON_KNIFE; }
		float GetArmorRatioModifier() { return 1.0; }


		void Precache()
		{
			m_iEf2Index = PRECACHE_MODEL("models/ef_dvhammer2.mdl");
			PRECACHE_MODEL("models/dvhammer_spike.mdl");
		
			m_iExploIndex = PRECACHE_MODEL("sprites/ef_dvhammer_explo.spr");

			m_iSpikeExploIndex = PRECACHE_MODEL("sprites/ef_dvhammer_spikeexplo.spr");

			PRECACHE_MODEL("sprites/ef_dvhammer_explo_b1.spr");
			PRECACHE_MODEL("sprites/ef_dvhammer_lightning_b1.spr");

			PRECACHE_SOUND("weapons/dvhammer_skill_exp.wav");
			PRECACHE_SOUND("weapons/dvhammer_skill_exp1.wav");
		};

		KnockbackData GetKnockBackData() override
		{
			return { -1.0f,  -1.0f,  -1.0f,  -1.0f, 1.0f };
		}


	public:
		void EXPORT OnThink(void);
		void Explosion(bool bManual);
		virtual int TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType);
		virtual void Killed(entvars_t* pevAttacker, int iGib);
		

		void Remove()
		{
			SetThink(NULL);
			SetTouch(NULL);

			UTIL_Remove(this);
		}

		time_point_t m_fLaunchTime;
		int m_iEf2Index;
		int m_iExploIndex;
		int m_iSpikeExploIndex;

	private:
		float GetRadiusDamage(bool bManual) const
		{
			float flDamage = bManual ? 60.0f : 20.0f;
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = bManual ? 7400.0f : 2500.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = bManual ? 8100.0f : 5400.0f;
			return flDamage;
		}

		float GetRadius(bool bManual) const
		{
			return bManual ? 500.0f : 100.0f;
		}
	};

	LINK_ENTITY_TO_CLASS(knifedvhammer_spike, CCKnifeDvhammer_Spike)


	int CCKnifeDvhammer_Spike::TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
	{
		auto pAttack = dynamic_cast<CBasePlayer*>(CBasePlayer::Instance(pevAttacker));

		if (!pAttack)
			return 0;

		if (pAttack->IsPlayer() || !pAttack->m_bIsZombie)
		{
			if (pAttack->m_pActiveItem != nullptr && !Q_strcmp(STRING(pAttack->m_pActiveItem->pev->classname), "knife_dvhammer"))
			{
				Explosion(true);
				return 1;
			}
			return 0;
		}
		else
			flDamage = 0;

		return CBaseEntity::TakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType);
	}

	void CCKnifeDvhammer_Spike::Killed(entvars_t* pevAttacker, int iGib)
	{
		Explosion(true);
	}

	void CCKnifeDvhammer_Spike::Explosion(bool bManual)
	{
		CBasePlayer* pOwner = (CBasePlayer*)CBaseEntity::Instance(pev->owner);

		if (!pOwner)
		{
			SetThink(NULL);
			SetTouch(NULL);

			UTIL_Remove(this);
		}

		if (bManual)
		{
			EMIT_SOUND_DYN(ENT(pev), CHAN_ITEM, "weapons/dvhammer_skill_exp.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

			MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
			WRITE_BYTE(TE_TEMPSPRITE);
			WRITE_COORD(pev->origin.x);
			WRITE_COORD(pev->origin.y);
			WRITE_COORD(pev->origin.z);
			WRITE_SHORT(m_iSpikeExploIndex);
			WRITE_BYTE(10);
			WRITE_BYTE(255);
			WRITE_BYTE(30);
			MESSAGE_END();

			Vector vecDir = pOwner->pev->origin - pev->origin;
			vecDir.z = 0;
			Vector vecAngle = UTIL_VecToAngles(vecDir);

			UTIL_TempModel(pev->origin, vecAngle, g_vecZero, m_iEf2Index, 10, 0, 10, true, 16, 255, kRenderTransTexture, NULL, false, 0, 0, 256, 0);
		}
		else
		{
			EMIT_SOUND_DYN(ENT(pev), CHAN_ITEM, "weapons/dvhammer_skill_exp1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

			MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
			WRITE_BYTE(TE_TEMPSPRITE);
			WRITE_COORD(pev->origin.x);
			WRITE_COORD(pev->origin.y);
			WRITE_COORD(pev->origin.z);
			WRITE_SHORT(m_iExploIndex);
			WRITE_BYTE(10);
			WRITE_BYTE(255);
			WRITE_BYTE(30);
			MESSAGE_END();
		}

		CBaseEntity* pEntity = NULL;
		float flRadius = GetRadius(bManual);
		float flDamage = GetRadiusDamage(bManual);

		while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, flRadius)) != NULL)
		{
			if (!pEntity->IsAlive())
				continue;

			if (pEntity->edict() == pev->owner)
				continue;

			if (pev->origin.z - pEntity->pev->origin.z < 16.0f)
				continue;

			CBasePlayer* pPlayer = (CBasePlayer*)pEntity;
			if ((pEntity->IsPlayer() || pEntity->Classify() == CLASS_PLAYER_ALLY) && g_pGameRules->PlayerRelationship(pOwner, pPlayer) != GR_TEAMMATE)
			{
				UTIL_AddKickRateBoost(pEntity, (pEntity->pev->origin.Make2D() - pev->origin.Make2D()).Normalize(), 1.0, 1000, 200.0);

				pEntity->TakeDamage(pev, VARS(pev->owner), flDamage, DMG_EXPLOSION | DMG_NEVERGIB);
			}
		}

		pev->takedamage = DAMAGE_NO;
		pev->effects = EF_NODRAW;
		pev->deadflag = DEAD_DEAD;

		SetThink(NULL);
		SetTouch(NULL);

		UTIL_Remove(this);
	}

	void CCKnifeDvhammer_Spike::OnThink(void)
	{
		pev->nextthink = gpGlobals->time + 0.1s;

		if (pev->owner)
		{
			CBasePlayer* pOwner = (CBasePlayer*)CBaseEntity::Instance(pev->owner);

			if (pOwner)
			{
				if (pOwner->IsPlayer() && (pOwner->m_bIsZombie || !pOwner->IsAlive()))
					Remove();
			}
		}
		else
			Remove();

		if (gpGlobals->time > m_fLaunchTime + SPIKE_TIME)
			Explosion(false);
	}

	void CCKnifeDvhammer_Spike::Spawn(void)
	{
		pev->movetype = MOVETYPE_TOSS;
		pev->solid = SOLID_NOT;
		pev->friction = 0.0;
		pev->deadflag = DEAD_NO;
		pev->takedamage = DAMAGE_YES;
		pev->health = 10.0f;
		pev->gravity = 1.0;

		Precache();

		SET_MODEL(ENT(pev), "models/dvhammer_spike.mdl");
		UTIL_SetSize(pev, Vector(-4.0, -4.0, -44.0), Vector(4.0, 4.0, 4.0));
		UTIL_SetOrigin(pev, pev->origin);

		SetThink(&CCKnifeDvhammer_Spike::OnThink);
		pev->nextthink = gpGlobals->time + 0.1s;

		SetTouch(NULL);
		m_fLaunchTime = gpGlobals->time;
	}
}
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

	class CKnifeDVHammer : public LinkWeaponTemplate<CKnifeDVHammer,
		TGeneralData,
		TPrecacheEvent,
		BuildTGetItemInfoFromCSW<WEAPON_KNIFE>::template type
	>
	{
	public:
		static constexpr const char* ClassName = "knife_dvhammer";
		static constexpr const char* V_Model = "models/v_dvhammer.mdl";
		static constexpr const char* P_Model = "models/p_dvhammer.mdl";
		static constexpr const char* W_Model = "models/w_knife.mdl";
		static constexpr const char* EventFile = "events/dvhammer.sc";

		static constexpr int MaxClip = -1;
		static constexpr auto ItemSlot = KNIFE_SLOT;
		static constexpr const char* AnimExtension = "hammer";
		KnockbackData KnockBack = { 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };

	public:

#define WPNSTATE_DVHAMMER_MODEB (1<<0)
#define DVHAMMER_MAXAMMO	100
#define DVHAMMER_TARGET_SHOT	5

		enum dvhammer_e
		{
			ANIM_IDLE = 0,
			ANIM_DRAW,
			ANIM_SLASH1,
			ANIM_SLASH2,
			ANIM_CHANGE,
			ANIM_IDLEB,
			ANIM_DRAWB,
			ANIM_SLASH1B,
			ANIM_SLASH2B,
			ANIM_SKILL1,
			ANIM_SKILL2,	//USE THIS
		};

		enum dvhammer_bodygroup_e
		{
			GROUP_WEAPON_1,
			GROUP_WEAPON_2,
			GROUP_WEAPON_3,
			GROUP_WEAPON_4,
			GROUP_WEAPON_5,
			GROUP_HANDS,
		};

		void Precache() override
		{
			Base::Precache();

			m_iEfIndex = PRECACHE_MODEL("models/ef_dvhammer.mdl");
			
			PRECACHE_MODEL("sprites/ef_dvhammer_blue.spr");
			PRECACHE_MODEL("sprites/ef_dvhammer_red.spr");


			m_iModelIndex = PRECACHE_MODEL("models/v_dvhammer.mdl");

			PRECACHE_SOUND("weapons/dvhammer_idle1.wav");
			PRECACHE_SOUND("weapons/dvhammer_idle2.wav");
			PRECACHE_SOUND("weapons/dvhammer_pin.wav");

			PRECACHE_SOUND("weapons/dvhammer_slash_hit.wav");
			PRECACHE_SOUND("weapons/dvhammer_slash1.wav");
			PRECACHE_SOUND("weapons/dvhammer_slash1_b.wav");
			PRECACHE_SOUND("weapons/dvhammer_slash2.wav");
			PRECACHE_SOUND("weapons/dvhammer_slash2_b.wav");
		}

		BOOL Deploy() override
		{
			m_fMaxSpeed = 255;
			m_iChargeShotsFired = 0;

#ifndef CLIENT_DLL
			if (m_flTimeholster != invalid_time_point)
			{

				m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] += (gpGlobals->time - m_flTimeholster) / 0.5s * 7;

				if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] >= DVHAMMER_MAXAMMO)
				{
					m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] = DVHAMMER_MAXAMMO;
				}
			}

			if (!m_iWeaponState && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] >= DVHAMMER_MAXAMMO)
			{
				m_iWeaponState |= WPNSTATE_DVHAMMER_MODEB;

				m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] = DVHAMMER_MAXAMMO;
			}
#endif

			if (DefaultDeploy(V_Model, P_Model, m_iWeaponState & WPNSTATE_DVHAMMER_MODEB ? ANIM_DRAWB :  ANIM_DRAW, AnimExtension, UseDecrement() != FALSE))
			{
				if (m_iWeaponState & WPNSTATE_DVHAMMER_MODEB)
					SetToBMode();
				else
					SetToAMode();
				return true;
			}

			return false;
		}

		void Spawn() override;
		float GetMaxSpeed() override { return m_fMaxSpeed; }
		void WeaponIdle();
		void PrimaryAttack() override;
		void SecondaryAttack() override;
		int GetItemInfo(ItemInfo* p) override;
		BOOL CanDrop() override { return false; }

	public:

#ifndef CLIENT_DLL
		hit_result_t KnifeAttack(float flDamage, float flRadius, float flAngleDegrees, int bitsDamageType, entvars_t* pevInflictor, CBasePlayer* pAttackPlayer);
#endif

		void DelaySecondaryAttack();
		void DelayPrimaryAttack();
		float GetPrimaryAttackDamage() const
		{
			float flDamage;

			if (m_iWeaponState & WPNSTATE_DVHAMMER_MODEB)
				flDamage = m_iChargeShotsFired == DVHAMMER_TARGET_SHOT ? 60 : 60;
			else
				flDamage = m_iChargeShotsFired == DVHAMMER_TARGET_SHOT ? 50 : 50;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
			{
				if (m_iWeaponState & WPNSTATE_DVHAMMER_MODEB)
					flDamage = m_iChargeShotsFired == DVHAMMER_TARGET_SHOT ? 2500 : 2300;
				else
					flDamage = m_iChargeShotsFired == DVHAMMER_TARGET_SHOT ? 2350 : 2100;
			}
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
			{
				if (m_iWeaponState & WPNSTATE_DVHAMMER_MODEB)
					flDamage = m_iChargeShotsFired == DVHAMMER_TARGET_SHOT ? 7220 : 7020;
				else
					flDamage = m_iChargeShotsFired == DVHAMMER_TARGET_SHOT ? 6700 : 6500;
			}
#endif
			return flDamage;
		}

		float GetSecondaryAttackDamage() const
		{

			float flDamage = 60.0f;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 6500.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 7700.0f;
#endif
			return flDamage;
		}

		void SetToAMode(void)
		{
			pev->skin = 0;
#ifndef CLIENT_DLL
			pev->modelindex = m_iModelIndex;

			int hands = 0;

			if (m_pPlayer && m_pPlayer->m_bIsFemale)
				hands = 1;

			SetBodygroup(GROUP_HANDS, hands);

			if (GetBodygroup(GROUP_WEAPON_1) == 0)
			{
				SetBodygroup(GROUP_WEAPON_1, 1);
			}

			pev->modelindex = 0;
#endif
		}

		void SetToBMode(void)
		{
			pev->skin = 1;
#ifndef CLIENT_DLL
			pev->modelindex = m_iModelIndex;

			int hands = 1;	//hand 0 seems useless pev->body = 2 male hands with fullstack but make no sense

			if (m_pPlayer && m_pPlayer->m_bIsFemale)
				hands = 1;

			SetBodygroup(GROUP_HANDS, hands);

			if (GetBodygroup(GROUP_WEAPON_1) == 1)
			{
				SetBodygroup(GROUP_WEAPON_1, 0);
			}

			pev->modelindex = 0;
#endif
		}

		KnockbackData GetKnockBackData() override
		{
			if (pev->iuser1)
				return { 0.0f, 0.0f, 0.0f, 0.0f, 0.3f };
			else
				return { 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };
		}

		void Holster(int skiplocal) override;
		int ExtractAmmo(CBasePlayerWeapon* pWeapon) override;
		void ItemPostFrame() override;

		int m_iChargeShotsFired;

		int m_iEfIndex;
		int m_iModelIndex;
		time_point_t m_flTimeNextCharge;
		time_point_t m_flTimeholster;

	};

	LINK_ENTITY_TO_CLASS(knife_dvhammer, CKnifeDVHammer)

#ifndef CLIENT_DLL
	hit_result_t CKnifeDVHammer::KnifeAttack(float flDamage, float flRadius, float flAngleDegrees, int bitsDamageType, entvars_t* pevInflictor, CBasePlayer* pAttackPlayer)
	{
		CBaseEntity* pEntity = NULL;
		hit_result_t result = HIT_NONE;
		TraceResult tr;
		UTIL_MakeVectors(pAttackPlayer->pev->v_angle);
		Vector vecEnd;
		Vector vecSrc = pAttackPlayer->GetGunPosition();

		Vector vecForward = gpGlobals->v_forward;
		Vector2D vecForward2D = vecForward.Make2D().Normalize();

		float c = cos(flAngleDegrees * 0.5 * M_PI / 180.0);

		while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, flRadius)) != NULL)
		{
			if (pEntity == pAttackPlayer)
				continue;

			if (pEntity->IsDormant())
				continue;

			if (pEntity->pev->takedamage == DAMAGE_NO)
				continue;

			if (pEntity->IsPlayer() && bIsWallBetweenEntity(pEntity, pAttackPlayer))
				continue;

			if (!AngleCheck(pEntity, pAttackPlayer, vecSrc, vecForward, vecForward2D, flRadius, c, &tr))
				continue;

			float flRatio = 1.0;

			if (pEntity && pEntity->IsPlayer())
			{
				UTIL_MakeVectors(pEntity->pev->angles);

				if (DotProduct((pEntity->pev->origin.Make2D() - pAttackPlayer->pev->origin.Make2D()).Normalize(), gpGlobals->v_forward.Make2D()) > 0.8)
				{
					flRatio = 3.0;
					bitsDamageType |= DMG_BACKATK;
				}
			}

			pAttackPlayer->m_iWeaponVolume = 128;

			CBasePlayer* pPlayer = (CBasePlayer*)pEntity;
			if (pPlayer->IsPlayer() && pPlayer->m_bIsZombie)
			{
				Vector2D vecDir2D = (pPlayer->pev->origin.Make2D() - m_pPlayer->pev->origin.Make2D()).Normalize();

				pPlayer->pev->velocity.x = vecDir2D.x * 600;
				pPlayer->pev->velocity.y = vecDir2D.y * 600;
				pPlayer->pev->velocity.z = 200;
			}

			ClearMultiDamage();

			pEntity->TraceAttack(pAttackPlayer->pev, flDamage * flRatio, (tr.vecEndPos - vecSrc).Normalize(), &tr, bitsDamageType);

			ApplyMultiDamage(pevInflictor, pAttackPlayer->pev);

			result = HIT_PLAYER;
		}

		if (!result)
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

			if (tr.flFraction < 1.0f)
				result = HIT_WALL;
		}

		return result;
	}
#endif

	int CKnifeDVHammer::ExtractAmmo(CBasePlayerWeapon* pWeapon)
	{
		if (m_iDefaultAmmo)
		{
			m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] = m_iDefaultAmmo;
			m_iClip = WEAPON_NOCLIP;
			m_iDefaultAmmo = 0;
			return TRUE;
		}
		return CBasePlayerWeapon::ExtractAmmo(pWeapon);
	}

	int CKnifeDVHammer::GetItemInfo(ItemInfo* p)
	{
		p->pszName = STRING(pev->classname);
		p->pszAmmo1 = "dvhammerammo";
		p->iMaxAmmo1 = DVHAMMER_MAXAMMO;
		p->pszAmmo2 = NULL;
		p->iMaxAmmo2 = -1;
		p->iMaxClip = DVHAMMER_MAXAMMO;
		p->iSlot = 2;
		p->iPosition = 1;
		p->iId = WEAPON_KNIFE;
		p->iFlags = 0;
		p->iWeight = KNIFE_WEIGHT;

		return 1;
	}

	void CKnifeDVHammer::Spawn()
	{
		pev->classname = MAKE_STRING("knife_dvhammer");
		m_iId = WEAPON_KNIFE;
		Precache();
		SET_MODEL(ENT(pev), "models/w_knife.mdl");
		m_iClip = WEAPON_NOCLIP;
		m_flTimeholster = gpGlobals->time;

		m_iDefaultAmmo = DVHAMMER_MAXAMMO;
		FallInit();
	}

	void CKnifeDVHammer::PrimaryAttack(void)
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
		m_iChargeShotsFired++;

		if (m_iWeaponState & WPNSTATE_DVHAMMER_MODEB)
			SendWeaponAnim(m_iChargeShotsFired == DVHAMMER_TARGET_SHOT ? ANIM_SLASH2B : ANIM_SLASH1B, UseDecrement() != FALSE);		
		else
			SendWeaponAnim(m_iChargeShotsFired == DVHAMMER_TARGET_SHOT ? ANIM_SLASH2 : ANIM_SLASH1, UseDecrement() != FALSE);
		
		if (m_iChargeShotsFired == DVHAMMER_TARGET_SHOT)
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/dvhammer_slash1_b.wav", VOL_NORM, ATTN_NORM, 0, 94);
		else
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/dvhammer_slash1.wav", VOL_NORM, ATTN_NORM, 0, 94);

		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.9s;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.4s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0s;

		SetThink(&CKnifeDVHammer::DelayPrimaryAttack);
		pev->nextthink = gpGlobals->time + 0.55s;
	}

	void CKnifeDVHammer::DelayPrimaryAttack(void)
	{
#ifndef CLIENT_DLL
		switch (KnifeAttack(GetPrimaryAttackDamage(), 184, 260, DMG_NEVERGIB | DMG_BULLET, m_pPlayer->pev, m_pPlayer))
		{
		case HIT_NONE:
		{
			break;
		}
		case HIT_PLAYER:
		{

			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/dvhammer_slash_hit.wav", VOL_NORM, ATTN_NORM, 0, 94);

			m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;
			break;
		}
		case HIT_WALL:
		{
			
			break;
		}
		}

		if (m_iChargeShotsFired == 5)
			m_iChargeShotsFired = 0;
#endif
	}

	void CKnifeDVHammer::DelaySecondaryAttack(void)
	{
#ifndef CLIENT_DLL
		switch (KnifeAttack(GetPrimaryAttackDamage(), 270, 360, DMG_NEVERGIB | DMG_BULLET, m_pPlayer->pev, m_pPlayer))
		{
		case HIT_NONE:
		{
			break;
		}
		case HIT_PLAYER:
		{

			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/dvhammer_slash_hit.wav", VOL_NORM, ATTN_NORM, 0, 94);

			m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;
			break;
		}
		case HIT_WALL:
		{

			break;
		}
		}

		Vector vecStartOrigin = m_pPlayer->pev->origin;

		if (m_pPlayer->pev->flags & FL_DUCKING)
			vecStartOrigin.z -= 16.0;
		else
			vecStartOrigin.z -= 32.0;

		UTIL_MakeVectors(m_pPlayer->pev->v_angle);
		TraceResult tr;

		Vector vecForward = gpGlobals->v_forward;

		vecForward.z = 0;
		Vector vecAngle = UTIL_VecToAngles(vecForward);

		Vector vecEnd = vecStartOrigin + vecForward * 90.0;

		UTIL_TempModel(vecEnd, vecAngle, g_vecZero, m_iEfIndex, 5, 0, 10, false, 0, 255, kRenderNormal, NULL, false, 0, 0, 256, 0);

		CCKnifeDvhammer_Spike* pSpike = static_cast<CCKnifeDvhammer_Spike*>(CBaseEntity::Create("knifedvhammer_spike", vecEnd, vecAngle, m_pPlayer->edict()));
		if (pSpike)
		{
			pSpike->m_iTeam = m_pPlayer->m_iTeam;
		}
#endif
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_AUTO, "weapons/dvhammer_pin.wav", VOL_NORM, ATTN_NORM, 0, 94);
	}

	void CKnifeDVHammer::SecondaryAttack(void)
	{
		if (m_iWeaponState & WPNSTATE_DVHAMMER_MODEB)
		{
#ifndef CLIENT_DLL
			m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
			m_iChargeShotsFired = 0;
			m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] -= 100;

			if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			{
				SetToAMode();
				m_iWeaponState &= ~WPNSTATE_DVHAMMER_MODEB;

				SendWeaponAnim(ANIM_SKILL2, UseDecrement() != FALSE);
			}
			else
				SendWeaponAnim(ANIM_SKILL1, UseDecrement() != FALSE);

			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/dvhammer_skill.wav", VOL_NORM, ATTN_NORM, 0, 94);

			m_pPlayer->m_flNextAttack = 0.2s;
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.9s;
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3s;

			SetThink(&CKnifeDVHammer::DelaySecondaryAttack);
			pev->nextthink = gpGlobals->time + 0.9s;
		}
		else
		{
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.2s;
		}
	}


	void CKnifeDVHammer::Holster(int skiplocal)
	{
		SetThink(nullptr);

		m_flTimeholster = gpGlobals->time;

		return CBasePlayerWeapon::Holster(skiplocal);
	}

	void CKnifeDVHammer::ItemPostFrame()
	{
#ifndef CLIENT_DLL
		if (m_iWeaponState & WPNSTATE_DVHAMMER_MODEB)
		{
			
		}
		else
		{
			if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] < DVHAMMER_MAXAMMO)
			{
				if (m_flTimeNextCharge <= gpGlobals->time)
				{
					m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] += 7;

					if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] >= DVHAMMER_MAXAMMO)
					{
						m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] = DVHAMMER_MAXAMMO;

						if (m_pPlayer->pev->weaponanim == ANIM_IDLE)
						{
							SendWeaponAnim(ANIM_CHANGE, UseDecrement() != FALSE);
							m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1s;
						}
						
						m_iWeaponState |= WPNSTATE_DVHAMMER_MODEB;

						SetToBMode();
					}
						
					m_flTimeNextCharge = gpGlobals->time + 0.5s;
				}
			}
		}
#endif
		return CBasePlayerWeapon::ItemPostFrame();
	}

	void CKnifeDVHammer::WeaponIdle(void)
	{
		ResetEmptySound();
		m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
		m_iChargeShotsFired = 0;
		SendWeaponAnim(m_iWeaponState & WPNSTATE_DVHAMMER_MODEB ? ANIM_IDLEB : ANIM_IDLE, UseDecrement() != FALSE);
	}


}
