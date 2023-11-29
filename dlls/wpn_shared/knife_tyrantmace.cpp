/*
knife_tyrantmace.cpp
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

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

	class CKnifeTyrantmace : public LinkWeaponTemplate<CKnifeTyrantmace,
		TGeneralData,
		TPrecacheEvent,
		BuildTGetItemInfoFromCSW<WEAPON_KNIFE>::template type
	>
	{
	public:
		static constexpr const char* ClassName = "knife_tyrantmace";
		static constexpr const char* V_Model = "models/v_tyrantmace.mdl";
		static constexpr const char* P_Model = "models/p_tyrantmace.mdl";
		static constexpr const char* P_Model_1 = "models/p_tyrantmace2.mdl";
		static constexpr const char* W_Model = "models/w_knife.mdl";
		static constexpr const char* EventFile = "events/tyrantmace.sc";

		static constexpr int MaxClip = -1;
		static constexpr auto ItemSlot = KNIFE_SLOT;
		static constexpr const char* AnimExtension = "tomahawk";
		KnockbackData KnockBack = { 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };

	public:

#define WPNSTATE_TYRANTMACE_MODEB (1<<0)
#define TYRANTMACE_MAXAMMO	100
		enum tyrantmace_e
		{
			ANIM_IDLE = 0,
			ANIM_DRAW,
			ANIM_SLASH1,
			ANIM_SLASH2,
			ANIM_SKILL,
		};

		enum tyrantmace_bodygroup_e
		{
			GROUP_WEAPON,
			GROUP_EFFECT,
			GROUP_SKILLEFFECT,
			GROUP_SLASHEFFECT,
			GROUP_HANDS,
		};

		void Precache() override
		{
			Base::Precache();

			m_iHoleIndex = PRECACHE_MODEL("models/ef_scorpion_hole.mdl");
			m_iBModeIndex = PRECACHE_MODEL("sprites/ef_tyrantmace_bmode.spr");
			m_iHitIndex = PRECACHE_MODEL("sprites/ef_tyrantmace_hit.spr");
			PRECACHE_MODEL("sprites/ef_tyrantmace_idle.spr");
			PRECACHE_MODEL("sprites/muzzleflash305.spr");
			PRECACHE_MODEL("sprites/muzzleflash306.spr");
			PRECACHE_MODEL("sprites/muzzleflash307.spr");

			m_iModelIndex = PRECACHE_MODEL("models/v_tyrantmace.mdl");
			m_iIceIndex = PRECACHE_MODEL("models/ef_tyrantmace_ice.mdl");
			m_iIce2Index = PRECACHE_MODEL("models/ef_tyrantmace_ice2.mdl");
			m_iIceHitIndex = PRECACHE_MODEL("models/ef_tyrantmace_ice_hit.mdl");
			m_iShockWaveIndex = PRECACHE_MODEL("models/ef_tyrantmace_shockwave.mdl");

			PRECACHE_SOUND("weapons/tyrantmace_hit1.wav");
			PRECACHE_SOUND("weapons/tyrantmace_hit2.wav");
			PRECACHE_SOUND("weapons/tyrantmace_skill.wav");
			PRECACHE_SOUND("weapons/tyrantmace_skill_exp.wav");
			PRECACHE_SOUND("weapons/tyrantmace_skill_ready.wav");
			PRECACHE_SOUND("weapons/tyrantmace_slash1.wav");
			PRECACHE_SOUND("weapons/tyrantmace_slash2.wav");
		}

		BOOL Deploy() override
		{
			m_fMaxSpeed = 255;
			m_iSwing = 0;

#ifndef CLIENT_DLL
			if (m_flTimeholster != invalid_time_point)
			{

				m_iClip += (gpGlobals->time - m_flTimeholster) / 0.6s * (g_pModRunning->DamageTrack() == DT_NONE ? 1 : 6);

				if (m_iClip >= TYRANTMACE_MAXAMMO)
				{
					m_iClip = TYRANTMACE_MAXAMMO;
				}
			}

			if (!m_iWeaponState && m_iClip >= TYRANTMACE_MAXAMMO)
			{
				m_iWeaponState |= WPNSTATE_TYRANTMACE_MODEB;

				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/tyrantmace_skill_ready.wav", VOL_NORM, ATTN_NORM, 0, 94);

				m_iClip = TYRANTMACE_MAXAMMO;
			}
#endif

			if (DefaultDeploy(V_Model, P_Model, ANIM_DRAW, AnimExtension, UseDecrement() != FALSE))
			{
				if (m_iWeaponState & WPNSTATE_TYRANTMACE_MODEB)
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
		void DelayPrimaryAttack();
		float GetPrimaryAttackDamage() const
		{
			float flDamage = 65.0f;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				//flDamage = 1764.0f;
				flDamage = 1121.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 5180.0f;
#endif
			return flDamage;
		}

		float GetSecondaryAttackDamage() const
		{

			float flDamage = 60.0f;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				//flDamage = 8500.0f;
				flDamage = 1800.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 10240.0f;
#endif
			return flDamage;
		}

		float GetRadiusDamage() const
		{

			float flDamage = 20.0f;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 800.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 1500.0f;
#endif
			return flDamage;
		}

		int GetBonus() const
		{
			int iBonus = 10;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				iBonus = 45;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				iBonus = 15;
#endif
			return iBonus;
		}

		void SetToAMode(void)
		{
#ifndef CLIENT_DLL
			pev->modelindex = m_iModelIndex;

			int hands = 0;

			if (m_pPlayer && m_pPlayer->m_bIsFemale)
				hands = 1;

			SetBodygroup(GROUP_HANDS, hands);

			if (GetBodygroup(GROUP_EFFECT) == 1)
			{
				SetBodygroup(GROUP_EFFECT, 0);
			}

			pev->modelindex = 0;
#endif
		}

		void SetToBMode(void)
		{
#ifndef CLIENT_DLL
			pev->modelindex = m_iModelIndex;

			int hands = 1;	//hand 0 seems useless pev->body = 2 male hands with fullstack but make no sense

			if (m_pPlayer && m_pPlayer->m_bIsFemale)
				hands = 1;

			SetBodygroup(GROUP_HANDS, hands);

			if (GetBodygroup(GROUP_EFFECT) == 0)
			{
				SetBodygroup(GROUP_EFFECT, 1);
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

		void CreateHitFX(Vector pos);
		bool AngularCheck(CBaseEntity* pEntity, CBaseEntity* pAttacker, Vector vecSrc, Vector vecDir, Vector2D vecAngleCheckDir, float flRange, float flMinimumCosine, TraceResult* ptr);
		void Holster(int skiplocal) override;
		void ItemPostFrame() override;

		int m_iBModeIndex;
		int m_iHoleIndex;
		int m_iHitIndex;
		int m_iIceIndex;
		int m_iIce2Index;
		int m_iIceHitIndex;
		int m_iShockWaveIndex;
		int m_iModelIndex;
		time_point_t m_flTimeNextCharge;
		time_point_t m_flTimeholster;

	};

	LINK_ENTITY_TO_CLASS(knife_tyrantmace, CKnifeTyrantmace)

	

	int CKnifeTyrantmace::GetItemInfo(ItemInfo* p)
	{
		p->pszName = STRING(pev->classname);
		p->pszAmmo1 = "tyrantmaceammo";
		p->iMaxAmmo1 = -1;
		p->pszAmmo2 = NULL;
		p->iMaxAmmo2 = -1;
		p->iMaxClip = TYRANTMACE_MAXAMMO;
		p->iSlot = 2;
		p->iPosition = 1;
		p->iId = WEAPON_KNIFE;
		p->iFlags = 0;
		p->iWeight = KNIFE_WEIGHT;

		return 1;
	}

	void CKnifeTyrantmace::Spawn()
	{
		m_iKnifeID = KNIFE_TYRANTMACE;
		pev->classname = MAKE_STRING("knife_tyrantmace");
		m_iId = WEAPON_KNIFE;
		Precache();
		SET_MODEL(ENT(pev), "models/w_knife.mdl");

		m_flTimeholster = gpGlobals->time;

		m_iDefaultAmmo = TYRANTMACE_MAXAMMO;
		FallInit();
	}

	void CKnifeTyrantmace::CreateHitFX(Vector pos)
	{
#ifndef CLIENT_DLL
		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_EXPLOSION);
		WRITE_COORD(pos.x);
		WRITE_COORD(pos.y);
		WRITE_COORD(pos.z);
		WRITE_SHORT(m_iHitIndex);
		WRITE_BYTE(5);
		WRITE_BYTE(20);
		WRITE_BYTE(TE_EXPLFLAG_NODLIGHTS | TE_EXPLFLAG_NOPARTICLES | TE_EXPLFLAG_NOSOUND);
		MESSAGE_END();
#endif
	}

	bool CKnifeTyrantmace::AngularCheck(CBaseEntity* pEntity, CBaseEntity* pAttacker, Vector vecSrc, Vector vecDir, Vector2D vecAngleCheckDir, float flRange, float flMinimumCosine, TraceResult* ptr)
	{
		bool bResult = false;
#ifndef CLIENT_DLL

		if (pEntity->IsBSPModel())
		{
			UTIL_TraceLine(vecSrc, vecSrc + vecDir * flRange, ignore_monsters, pAttacker->edict(), ptr);

			if (ptr->flFraction < 1.0)
			{
				if (ptr->pHit)
				{
					CBaseEntity* pHit = CBaseEntity::Instance(ptr->pHit);

					if (pHit && pHit == pEntity)
						bResult = true;
				}
			}
		}
		else
		{
			UTIL_TraceHull(vecSrc, vecSrc + vecDir * flRange, dont_ignore_monsters, head_hull, pAttacker->edict(), ptr);

			if (ptr->flFraction < 1.0 && ptr->pHit != NULL && CBaseEntity::Instance(ptr->pHit) == pEntity)
			{
				UTIL_TraceLine(vecSrc, vecSrc + vecDir * flRange, dont_ignore_monsters, pAttacker->edict(), ptr);

				bResult = true;
			}
			else
			{
				Vector vecEnd = pEntity->BodyTarget(vecSrc);

				if (DotProduct(vecAngleCheckDir, (vecEnd.Make2D() - vecSrc.Make2D()).Normalize()) > flMinimumCosine)
				{
					edict_t* peAttacker = pAttacker->edict();

					for (int i = 5; i > 0; i--)
					{
						UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, peAttacker, ptr);

						if (!ptr->pHit || !GET_PRIVATE(ptr->pHit))
						{
							break;
						}

						CBaseEntity* pHit = CBaseEntity::Instance(ptr->pHit);

						if (ptr->flFraction < 1.0 || pHit == pEntity)
						{
							bResult = true;

							break;
						}

						if (pHit->IsBSPModel())
						{
							break;
						}

						vecSrc = ptr->vecEndPos + vecDir * 5.0;
					}
				}
			}
		}

		if (!bResult)
			return bResult;

		if ((pEntity->IsPlayer() || pEntity->Classify() == CLASS_PLAYER_ALLY) && (g_pGameRules->PlayerRelationship(m_pPlayer, pEntity) != GR_TEAMMATE))
		{
			if (!FBitSet(pEntity->pev->flags, FL_DUCKING))
				CreateHitFX(pEntity->pev->origin - Vector(0, 0, 10));
			else
				CreateHitFX(pEntity->pev->origin);
		}
#endif

		return bResult;
	}

	void CKnifeTyrantmace::PrimaryAttack(void)
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
		SendWeaponAnim(m_iSwing++ % 2 ? ANIM_SLASH1 : ANIM_SLASH2, UseDecrement() != FALSE);

		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.6s;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.2s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.7s;

		SetThink(&CKnifeTyrantmace::DelayPrimaryAttack);
		pev->nextthink = gpGlobals->time + 0.12s;
	}

	void CKnifeTyrantmace::DelayPrimaryAttack(void)
	{
		
#ifndef CLIENT_DLL
		UTIL_MakeVectors(m_pPlayer->pev->v_angle);
		pev->iuser1 = 1;
		switch (KnifeAttack(GetPrimaryAttackDamage(), 182, 230, DMG_NEVERGIB | DMG_BULLET, m_pPlayer->pev, m_pPlayer))
		{
		case HIT_NONE:
		{
			if (m_iSwing & 1)
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/tyrantmace_slash1.wav", VOL_NORM, ATTN_NORM, 0, 94);
			else
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/tyrantmace_slash2.wav", VOL_NORM, ATTN_NORM, 0, 94);
			break;
		}
		case HIT_PLAYER:
		{
			if (m_iSwing & 1)
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/tyrantmace_hit1.wav", VOL_NORM, ATTN_NORM, 0, 94);
			else
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/tyrantmace_hit2.wav", VOL_NORM, ATTN_NORM, 0, 94);
			m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;
			break;
		}
		case HIT_WALL:
		{
			if (m_iSwing & 1)
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/tyrantmace_hit1.wav", VOL_NORM, ATTN_NORM, 0, 94);
			else
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/tyrantmace_hit2.wav", VOL_NORM, ATTN_NORM, 0, 94);
			m_pPlayer->m_iWeaponVolume = KNIFE_WALLHIT_VOLUME * 0.5;
			break;
		}
		}
#endif
	}

	void CKnifeTyrantmace::SecondaryAttack(void)
	{
		if (m_iWeaponState & WPNSTATE_TYRANTMACE_MODEB)
		{
#ifndef CLIENT_DLL
			m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
			m_iClip = 0;
			SetToAMode();
			m_iWeaponState &= ~WPNSTATE_TYRANTMACE_MODEB;

			m_pPlayer->m_flNextAttack = 0.5s;
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.8s;
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0s;

			SendWeaponAnim(ANIM_SKILL, UseDecrement() != FALSE);
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;

#ifndef CLIENT_DLL
			pev->iuser1 = 0;
			struct
			{
				float RadiusDamageRadius;
				int RadiusDamageBitsDamageType;
				bool RadiusDamageCanHeadshot;
				float RadiusDamageAmount;
				float RadiusDamageHasFallOff;
			} Data;

			Vector vecSrc = m_pPlayer->GetGunPosition();

			Data.RadiusDamageRadius = 181.1;
			Data.RadiusDamageBitsDamageType = DMG_BULLET;
			Data.RadiusDamageCanHeadshot = false;
			Data.RadiusDamageAmount = GetRadiusDamage();
			Data.RadiusDamageHasFallOff = false;

			std::vector<CBaseEntity*> out;
			RadiusDamage(Data, vecSrc, m_pPlayer, m_pPlayer, std::back_inserter(out));
			for (CBaseEntity* ent : out)
			{
				CBasePlayer* player = dynamic_ent_cast<CBasePlayer*>(ent);
				if (player && player->m_bIsZombie)
				{
					Vector vecDir = (player->pev->origin - m_pPlayer->pev->origin);
					vecDir.z = 0;

					ApplyKnockbackData(player, player->Center() - vecSrc, { 1500, 1500, 1500, 1500, 1.0f });
					player->pev->velocity.z = +200.0f;
				}
			}

			Vector vecStartOrigin = m_pPlayer->pev->origin;

			if (m_pPlayer->pev->flags & FL_DUCKING)
				vecStartOrigin.z -= 16.0;
			else
				vecStartOrigin.z -= 32.0;

			UTIL_TempModel(vecStartOrigin, g_vecZero, g_vecZero, m_iHoleIndex, 2, 1, 10, 0, 0, 255, kRenderNormal, NULL, false, 0, 0, 256, 0);

			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/tyrantmace_skill.wav", VOL_NORM, ATTN_NORM, 0, 94);

			UTIL_MakeVectors(m_pPlayer->pev->v_angle);
			TraceResult tr;

			Vector vecForward = gpGlobals->v_forward;
			
			vecForward.z = 0;
			Vector vecAngle = UTIL_VecToAngles(vecForward);

			Vector vecEnd = vecStartOrigin + vecForward * 275.6;

			UTIL_TempModel(vecStartOrigin, vecAngle, g_vecZero, m_iIce2Index, 10, 0, 10, true, 16, 255, kRenderNormal, NULL, false, 0, 0, 256, 0);

			UTIL_TempModel(vecEnd, vecAngle, g_vecZero, m_iShockWaveIndex, 10, 0, 10, true, 16, 255, kRenderNormal, NULL, false, 0, 0, 256, 0);

			UTIL_TempModel(vecEnd, vecAngle, g_vecZero, m_iIceIndex, 10, 0, 10, true, 16, 255, kRenderNormal, NULL, false, 0, 0, 256, 0);

			vecAngle.y += 45;

			UTIL_TempModel(vecEnd, vecAngle, g_vecZero, m_iIceIndex, 10, 0, 10, true, 16, 255, kRenderNormal, NULL, false, 0, 0, 256, 0);

			vecAngle.y += 45;

			UTIL_TempModel(vecEnd, vecAngle, g_vecZero, m_iIceIndex, 10, 0, 10, true, 16, 255, kRenderNormal, NULL, false, 0, 0, 256, 0);

			MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
			WRITE_BYTE(TE_TEMPSPRITE);
			WRITE_COORD(vecEnd.x);
			WRITE_COORD(vecEnd.y);
			WRITE_COORD(vecEnd.z + 30.0);
			WRITE_SHORT(m_iBModeIndex);
			WRITE_BYTE(10);
			WRITE_BYTE(255);
			WRITE_BYTE(45);
			MESSAGE_END();

			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/tyrantmace_skill_exp.wav", VOL_NORM, ATTN_NORM, 0, 94);

			UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr);

			CBaseEntity* pEntity = NULL;

			while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecEnd, 204.72)) != NULL)
			{
				if (pEntity == m_pPlayer)
					continue;

				if (pEntity->IsDormant())
					continue;

				if (pEntity->pev->takedamage == DAMAGE_NO)
					continue;

				if (pEntity->IsPlayer() && bIsWallBetweenEntity(pEntity, m_pPlayer))
					continue;

				if (m_pPlayer->pev->origin.z - pEntity->pev->origin.z > 16.0f)
					continue;

				m_pPlayer->m_iWeaponVolume = 128;

				pEntity->TakeDamage(pev, m_pPlayer->pev, GetSecondaryAttackDamage(), DMG_GENERIC);

				Vector vecDirection = (pEntity->pev->origin - m_pPlayer->pev->origin).Normalize();
				UTIL_AddKickRateBoost(pEntity, vecDirection.Make2D(), 1.0, 2000.0, 0);

				if ((pEntity->IsPlayer() || pEntity->Classify() == CLASS_PLAYER_ALLY) && (g_pGameRules->PlayerRelationship(m_pPlayer, pEntity) != GR_TEAMMATE))
				{
					Vector vecPos = pEntity->pev->origin;
					if (pEntity->pev->flags & FL_DUCKING)
						vecPos.z -= 10.0;

					UTIL_TempModel(vecPos, g_vecZero, g_vecZero, m_iIceHitIndex, 10, 0, 10, true, 16, 255, kRenderTransAdd, NULL, false, 0, 0, 256, 0);
				}

				if (((CBasePlayer*)pEntity)->m_bIsZombie)
				{
					CBasePlayer* player = (CBasePlayer*)pEntity;

					auto strategy = dynamic_cast<CPlayerModStrategy_ZB1*>(player->m_pModStrategy.get());
					if (strategy)
					{
						strategy->SetStunSpeedTime(1.8s, 120.0);
						strategy->SetStunGravityTime(1.0s);
					}
				}
			}
#endif
		}
		else
		{
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.2s;
		}
	}

#ifndef CLIENT_DLL
	hit_result_t CKnifeTyrantmace::KnifeAttack(float flDamage, float flRadius, float flAngleDegrees, int bitsDamageType, entvars_t* pevInflictor, CBasePlayer* pAttackPlayer)
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

			if (!AngularCheck(pEntity, pAttackPlayer, vecSrc, vecForward, vecForward2D, flRadius, c, &tr))
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

			ClearMultiDamage();

			pEntity->TraceAttack(pAttackPlayer->pev, flDamage * flRatio, (tr.vecEndPos - vecSrc).Normalize(), &tr, bitsDamageType);

			ApplyMultiDamage(pevInflictor, pAttackPlayer->pev);

			if (!m_iWeaponState && (pEntity->IsPlayer() || pEntity->Classify() == CLASS_PLAYER_ALLY) && (g_pGameRules->PlayerRelationship(m_pPlayer, pEntity) != GR_TEAMMATE))
			{
				m_iClip += GetBonus();

				if (m_iClip >= TYRANTMACE_MAXAMMO)
				{
					m_iClip = TYRANTMACE_MAXAMMO;

					m_iWeaponState |= WPNSTATE_TYRANTMACE_MODEB;

					SetToBMode();
				}
			}

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

	void CKnifeTyrantmace::Holster(int skiplocal)
	{
		SetThink(nullptr);

		m_flTimeholster = gpGlobals->time;

		return CBasePlayerWeapon::Holster(skiplocal);
	}

	void CKnifeTyrantmace::ItemPostFrame()
	{
#ifndef CLIENT_DLL
		if (m_iWeaponState & WPNSTATE_TYRANTMACE_MODEB)
		{
			
		}
		else
		{
			if (m_iClip < TYRANTMACE_MAXAMMO)
			{
				if (m_flTimeNextCharge <= gpGlobals->time)
				{
					m_iClip += (g_pModRunning->DamageTrack() == DT_NONE ? 1 : 6);

					if (m_iClip >= TYRANTMACE_MAXAMMO)
					{
						m_iClip = TYRANTMACE_MAXAMMO;

						EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/tyrantmace_skill_ready.wav", VOL_NORM, ATTN_NORM, 0, 94);

						m_iWeaponState |= WPNSTATE_TYRANTMACE_MODEB;

						SetToBMode();
					}
						
					m_flTimeNextCharge = gpGlobals->time + 0.6s;
				}
			}
		}
#endif
		return CBasePlayerWeapon::ItemPostFrame();
	}

	void CKnifeTyrantmace::WeaponIdle(void)
	{
		ResetEmptySound();
		m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
		SendWeaponAnim(ANIM_IDLE, UseDecrement() != FALSE);
	}


}
