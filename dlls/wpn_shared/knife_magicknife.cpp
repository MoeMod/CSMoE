/*
knife_magicknife.cpp
Copyright (C) 2021 JustANoobQAQ


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

#include "weapons/WeaponTemplate.hpp"
#include "weapons/KnifeAttack.h"

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif
	
#define KNIFE_BODYHIT_VOLUME 128
#define KNIFE_WALLHIT_VOLUME 512
	class CMagicCircle;
	class CKnifeMagicKnife : public LinkWeaponTemplate<CKnifeMagicKnife,
		TGeneralData,
		BuildTGetItemInfoFromCSW<WEAPON_KNIFE>::template type
	>
	{
	public:
		static constexpr const char* ClassName = "knife_magicknife";
		static constexpr const char* V_Model = "models/v_magicknife.mdl";
		static constexpr const char* P_Model = "models/p_magicknife.mdl";
		static constexpr const char* W_Model = "models/w_knife.mdl";

		static constexpr int MaxClip = -1;
		static constexpr auto ItemSlot = KNIFE_SLOT;
		static constexpr const char* AnimExtension = "sfsword";
		KnockbackData KnockBack = { 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };

	public:

		enum magicknife_e
		{
			ANIM_IDLE = 0,
			ANIM_SLASH1,
			ANIM_SLASH2,
			ANIM_SLASH3,
			ANIM_SLASH4,
			ANIM_SLASH_END,
			ANIM_DRAW,
			ANIM_CHARGE_START,
			ANIM_CHARGE_FINISH,
			ANIM_CHARGE_IDLE1,
			ANIM_CHARGE_IDLE2,
			ANIM_CHARGE_ATTACK1,
			ANIM_CHARGE_ATTACK2,
			ANIM_STAB,
		};

		void Precache() override
		{
			PRECACHE_MODEL("models/v_magicknife.mdl");
			PRECACHE_MODEL("models/p_magicknife.mdl");

			PRECACHE_SOUND("weapons/magicknife_charge_attack2.wav");
			PRECACHE_SOUND("weapons/magicknife_charge_finish.wav");
			PRECACHE_SOUND("weapons/magicknife_charge_idle2_1.wav");
			PRECACHE_SOUND("weapons/magicknife_charge_start.wav");
			PRECACHE_SOUND("weapons/magicknife_magic_end.wav");
			PRECACHE_SOUND("weapons/magicknife_magic_exp.wav");
			PRECACHE_SOUND("weapons/magicknife_magic_hit.wav");
			PRECACHE_SOUND("weapons/magicknife_magic_start.wav");
			PRECACHE_SOUND("weapons/magicknife_slash1.wav");
			PRECACHE_SOUND("weapons/magicknife_slash2.wav");
			PRECACHE_SOUND("weapons/magicknife_slash3.wav");
			PRECACHE_SOUND("weapons/magicknife_slash4.wav");

			PRECACHE_SOUND("weapons/tomahawk_slash1_hit.wav");
		}
		
		BOOL Deploy(void) override
		{
			m_fMaxSpeed = 255;
			m_iState = 0;
			m_iState2 = 0;
			m_iThinkType = 0;
		
			m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;
			
			m_tNextThink = invalid_time_point;
			m_flTimeCheckCharge = invalid_time_point;
#ifndef CLIENT_DLL	
			m_flNextFindTarget = invalid_time_point;
			m_iLastAnim = ANIM_DRAW;
#endif
			m_pPlayer->m_bShieldDrawn = false;
			

			DefaultDeploy(V_Model, P_Model, ANIM_DRAW, "sfsword", UseDecrement() != FALSE, 0.4s, 1.0s);
			return TRUE;
		}

		int GetItemInfo(ItemInfo* p) override;
		void PrimaryAttack() override;	
		void SecondaryAttack() override;
		void Spawn() override;
		void WeaponIdle() override;
		float GetMaxSpeed() override;
		void Holster(int skiplocal) override;
		void DelayPrimaryAttack();
		void DelaySecondaryAttack();
		void DelayChargeAttack();
		void OnAttack2Released();

		BOOL CanDrop() override { return false; }
		void ItemPostFrame() override;
#ifndef CLIENT_DLL
		hit_result_t KnifeAttack(float flDamage, float flRadius, float flAngleDegrees, int bitsDamageType, entvars_t* pevInflictor, CBasePlayer* pAttackPlayer, bool IsPrimaryAttack);
		void SecondaryAttack_FindTargets();
		bool SecondaryAttack_CheckTargetAvailable(CBaseEntity* a2);
		time_point_t m_flNextFindTarget;
#endif
	public:
		float GetPrimaryAttackDamage() const
		{
			float flDamage = 50;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 850.0f;	//original 1836
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 1000.0f;
#endif
			return flDamage;
		}
		float GetSecondaryAttackDamage() const
		{
			float flDamage = 115;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 1750.0f;	//2990.0
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 3450.0;
#endif
			return flDamage;
		}

		float  GetChargeAttackDamage() const
		{
			float flDamage = 130;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 1865.0f;	//4573
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 4573.0f;
#endif
			return flDamage;
		}

		time_point_t m_flTimeCheckCharge;

#ifndef CLIENT_DLL
		EntityHandle <CMagicCircle> m_pMagicCircle[10];
		EHANDLE m_pEnemyList[10];
		
#endif
		int m_iLastAnim;
		int m_iState;
		int m_iState2;
		
		int m_iThinkType;
		time_point_t m_tNextThink;
	};


#ifndef CLIENT_DLL
	class CMagicCircle : public CBaseEntity
	{
	public:
		CMagicCircle()
		{
			m_flAnimTime = invalid_time_point;
		}

		void Spawn(void);
		void EXPORT OnThink();
		void Explode();
		void Init(CBasePlayer* pOwner, CBaseEntity* pAttachedEnt, CKnifeMagicKnife* pWeapon);

		void Precache(void)
		{
			//PRECACHE_MODEL("sprites/ef_mgknife_end.spr");
			PRECACHE_MODEL("sprites/ef_mgknife_exp01.spr");
			PRECACHE_MODEL("sprites/ef_mgknife_exp02.spr");
			PRECACHE_MODEL("sprites/ef_mgknife_loop.spr");
			PRECACHE_MODEL("sprites/ef_mgknife_start.spr");
		}

		float  GetDamage() const
		{
			float flDamage = 130;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 1700.0f;	//3000
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 3000.0f;
#endif
			return flDamage;
		}

		CBasePlayer* m_pOwner;
		CBaseEntity* m_pAttachedEnt;
		CKnifeMagicKnife* m_pWeapon;
		time_point_t m_flAnimTime;
		Vector vecDir;
		float m_flMaxFrames;

		void Remove()
		{
			if(m_pWeapon)
				m_pWeapon->pev->iuser1 &= ~(1 << m_pAttachedEnt->entindex());

			EMIT_SOUND_DYN(ENT(pev), CHAN_ITEM, "weapons/magicknife_magic_end.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

			PLAYBACK_EVENT_FULL(FEV_GLOBAL, ENT(pev), PRECACHE_EVENT(1, "events/wpneffects.sc"), 0.0, pev->origin, g_vecZero, 0.0, 0.0, 25, m_pOwner->entindex(), TRUE, FALSE);

			SetThink(nullptr);
			pev->effects |= EF_NODRAW; // 0x80u
			return UTIL_Remove(this);
		}
	};

	LINK_ENTITY_TO_CLASS(magicknife_magiccircle, CMagicCircle)
#endif

	LINK_ENTITY_TO_CLASS(knife_magicknife, CKnifeMagicKnife)

	void CKnifeMagicKnife::Spawn()
	{
		pev->classname = MAKE_STRING("knife_magicknife");
		m_iId = WEAPON_KNIFE;
		Precache();
		SET_MODEL(ENT(pev), "models/w_knife.mdl");
		m_tNextThink = invalid_time_point;
		m_flTimeCheckCharge = invalid_time_point;
#ifndef CLIENT_DLL	
		m_flNextFindTarget = invalid_time_point;
#endif
		m_iClip = WEAPON_NOCLIP;
		m_iState = 0;
		m_iState2 = 0;
		m_iThinkType = 0;
		FallInit();
	}

	int CKnifeMagicKnife::GetItemInfo(ItemInfo* p)
	{
		p->pszName = STRING(pev->classname);
		p->pszAmmo1 = NULL;
		p->iMaxAmmo1 = -1;
		p->pszAmmo2 = NULL;
		p->iMaxAmmo2 = -1;
		p->iMaxClip = WEAPON_NOCLIP;
		p->iSlot = 2;
		p->iPosition = 1;
		p->iId = WEAPON_KNIFE;
		p->iFlags = 0;
		p->iWeight = KNIFE_WEIGHT;

		return 1;
	}
#ifndef CLIENT_DLL	
	hit_result_t CKnifeMagicKnife::KnifeAttack(float flDamage, float flRadius, float flAngleDegrees, int bitsDamageType, entvars_t* pevInflictor, CBasePlayer* pAttackPlayer, bool IsPrimaryAttack)
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

			ClearMultiDamage();

			pEntity->TraceAttack(pAttackPlayer->pev, flDamage * flRatio, (tr.vecEndPos - vecSrc).Normalize(), &tr, bitsDamageType);

			ApplyMultiDamage(pevInflictor, pAttackPlayer->pev);

			result = HIT_PLAYER;

			if (pEntity->IsPlayer())
			{
				CBasePlayer* player = (CBasePlayer*)pEntity;

				if (player->m_bIsZombie)
				{
					Vector vecDir = (player->pev->origin - m_pPlayer->pev->origin).Normalize();
					vecDir.z = 0;

					if (!IsPrimaryAttack)
					{
						if (player->m_bIsZombie && !(player->m_iBuff & BUFF_IGNORE_KNOCKBACK))
						{
							player->pev->velocity.x = vecDir.x * 600;
							player->pev->velocity.y = vecDir.y * 600;
							player->pev->velocity.z = 400;

							auto strategy = dynamic_cast<CPlayerModStrategy_ZB1*>(player->m_pModStrategy.get());
							if (strategy)
							{
								strategy->SetStunSpeedTime(1.0s);
								strategy->SetStunGravityTime(1.0s);
							}
						}
					}
					else
					{
						ApplyKnockbackData(player, vecDir, { 0.0, 0.0, 0.0, 0.0, 0.2f });
					}
				}
			}
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
	void CKnifeMagicKnife::DelayPrimaryAttack(void)
	{
#ifndef CLIENT_DLL		
		UTIL_MakeVectors(m_pPlayer->pev->v_angle);
		Vector vecSrc = m_pPlayer->GetGunPosition();

		float flDamage = GetPrimaryAttackDamage();
		float flDistance = 115;
		float flAngle = 180.0;

		switch (KnifeAttack(flDamage, flDistance, flAngle, DMG_NEVERGIB | DMG_BULLET, this->pev, m_pPlayer, true))
		{
		case HIT_PLAYER:
		{
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/tomahawk_slash1_hit.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			break;
		}
		case HIT_WALL:
		{
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/combatknife_wall.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			break;
		}
		}
#endif

		SetThink(nullptr);
	}

#ifndef CLIENT_DLL
	bool CKnifeMagicKnife::SecondaryAttack_CheckTargetAvailable(CBaseEntity* a2)
	{
		if (!a2->IsAlive())
			return false;

		if (g_pGameRules->PlayerRelationship(m_pPlayer, a2) == GR_TEAMMATE)
			return false;

		if (a2->pev == m_pPlayer->pev)
			return false;

		if (!a2->IsPlayer())
			return false;

		if (bIsWallBetweenEntity(a2, m_pPlayer))
			return false;

		if ((pev->iuser1 & (1 << a2->entindex())))
			return false;

		return true;
	}

	void CKnifeMagicKnife::SecondaryAttack_FindTargets()
	{
		CBaseEntity* pEntity = NULL;
		CBaseEntity* pNearest = NULL;
		while ((pEntity = UTIL_FindEntityInSphere(pEntity, m_pPlayer->pev->origin, 39.19 * 13.6)) != nullptr)
		{
			if (SecondaryAttack_CheckTargetAvailable(pEntity))
			{
				if(!pNearest)
					pNearest = pEntity;

				if ((pNearest->pev->origin - m_pPlayer->pev->origin).Length() > (pEntity->pev->origin - m_pPlayer->pev->origin).Length())
					pNearest = pEntity;
			}
		}
		if (pNearest)
		{
			for (int i = 0; i < 10; i++)
			{
				if (!m_pEnemyList[i])
				{
					pev->iuser1 |= (1 << pNearest->entindex());

					if (!(pev->iuser2 & (1 << pNearest->entindex())))
					{
						if (pNearest->IsPlayer())
						{
							CBasePlayer* player = (CBasePlayer*)pNearest;
							if (player->m_bIsZombie && !(player->m_iBuff & BUFF_IGNORE_KNOCKBACK))
							{
								auto strategy = dynamic_cast<CPlayerModStrategy_ZB1*>(player->m_pModStrategy.get());
								if (strategy)
								{
									strategy->SetStunSpeedTime(3.0s);
									strategy->SetStunGravityTime(3.0s);
								}
							}
						}
					}
					pev->iuser2 |= (1 << pNearest->entindex());

					Vector vecDir = (pNearest->pev->origin - m_pPlayer->pev->origin).Normalize();
					Vector angle = UTIL_VecToAngles(vecDir);
					angle[0] *= -1.0;

					EMIT_SOUND_DYN(ENT(pev), CHAN_ITEM, "weapons/magicknife_magic_start.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

					m_pEnemyList[i] = pNearest;
					m_pMagicCircle[i] = static_cast<CMagicCircle*>(CBaseEntity::Create("magicknife_magiccircle", pNearest->pev->origin, angle, pev->owner));
					if (m_pMagicCircle[i])
						m_pMagicCircle[i]->Init(m_pPlayer, pNearest, this);
					return;
				}
			}
		}
	}
#endif
	void CKnifeMagicKnife::PrimaryAttack(void)
	{
		switch (m_iState)
		{
			case 0:
			{
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/magicknife_slash1.wav", VOL_NORM, ATTN_NORM, 0, 94);

				m_iState = 1;
				SendWeaponAnim(ANIM_SLASH1, UseDecrement() != FALSE);
				m_iLastAnim = ANIM_SLASH1;
				
				break;
			}
			case 1:
			{
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/magicknife_slash2.wav", VOL_NORM, ATTN_NORM, 0, 94);

				m_iState = 2;
				SendWeaponAnim(ANIM_SLASH2, UseDecrement() != FALSE);
				m_iLastAnim = ANIM_SLASH2;
				
				break;
			}
			case 2:
			{
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/magicknife_slash3.wav", VOL_NORM, ATTN_NORM, 0, 94);

				m_iState = 3;
				SendWeaponAnim(ANIM_SLASH3, UseDecrement() != FALSE);
				m_iLastAnim = ANIM_SLASH3;
				
				break;
			}
			case 3:
			{
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/magicknife_slash4.wav", VOL_NORM, ATTN_NORM, 0, 94);

				m_iState = 0;
				SendWeaponAnim(ANIM_SLASH4, UseDecrement() != FALSE);
				m_iLastAnim = ANIM_SLASH4;
				
				break;
			}
		}

#ifndef CLIENT_DLL	
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

		SetThink(&CKnifeMagicKnife::DelayPrimaryAttack);
		pev->nextthink = gpGlobals->time + 0.1s;

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.53s;
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.36s;
		m_tNextThink = gpGlobals->time + 0.5s;
		m_iThinkType = 1;
	}

	void CKnifeMagicKnife::SecondaryAttack(void)
	{
		m_iState = 0;
		m_iThinkType = 0;
		if (!m_iState2)
		{
			SendWeaponAnim(ANIM_STAB, UseDecrement() != FALSE);
			m_iLastAnim = ANIM_STAB;
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 9999s;
			m_flTimeCheckCharge = gpGlobals->time + 0.67s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.7s;
			m_iState2 = 1;
		}
		else
		{
			if (m_flTimeCheckCharge < gpGlobals->time)
			{
				switch (m_iState2)
				{
				case 1:
				{
					SendWeaponAnim(ANIM_CHARGE_START, UseDecrement() != FALSE);
					m_iLastAnim = ANIM_CHARGE_START;
					m_iState2 = 2;

					m_flTimeCheckCharge = gpGlobals->time + 0.33s;			
					m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.85s;
					break;
				}
				case 2:
				{
					SendWeaponAnim(ANIM_CHARGE_IDLE1, UseDecrement() != FALSE);
					m_iLastAnim = ANIM_CHARGE_IDLE1;
					m_iState2 = 3;

					m_flTimeCheckCharge = gpGlobals->time + 0.47s;
					m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.23s;
					break;
				}
				case 3:
				{
					SendWeaponAnim(ANIM_CHARGE_FINISH, UseDecrement() != FALSE);
					m_iLastAnim = ANIM_CHARGE_FINISH;
					m_iState2 = 4;

					m_flTimeCheckCharge = gpGlobals->time + 0.33s;
					m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.33s;
					break;
				}
				case 4:
				{
					SendWeaponAnim(ANIM_CHARGE_IDLE2, UseDecrement() != FALSE);
					m_iLastAnim = ANIM_CHARGE_IDLE2;
					m_iState2 = 5;

					m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.0s;
					m_flTimeCheckCharge = gpGlobals->time + 0.36s;
#ifndef CLIENT_DLL	
					m_flNextFindTarget = gpGlobals->time + 0.36s;
#endif			
					break;	
				}
				case 5:
				{
#ifndef CLIENT_DLL
					if (m_flNextFindTarget != invalid_time_point && m_flNextFindTarget < gpGlobals->time)
					{
						SecondaryAttack_FindTargets();
						m_flNextFindTarget = gpGlobals->time + 0.36s;
					}

					for (int i = 0; i < 10; i++)
					{
						if (!m_pMagicCircle[i])
						{
							m_pEnemyList[i] = NULL;
						}
							
					}
					break;
#endif
				}
				}
			}
		}
	}

	void CKnifeMagicKnife::OnAttack2Released(void)
	{
		switch (m_iState2)
		{
		case 1:
		{
#ifndef CLIENT_DLL	
			m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
			m_flNextSecondaryAttack = m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.56s;
			
			SetThink(&CKnifeMagicKnife::DelaySecondaryAttack);
			pev->nextthink = gpGlobals->time + 1.0s;
			break;
		}
		case 2:
		case 3:
		case 4:
		{
#ifndef CLIENT_DLL	
			m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
			SendWeaponAnim(ANIM_CHARGE_ATTACK1, UseDecrement() != FALSE);
			m_iLastAnim = ANIM_CHARGE_ATTACK1;
			m_flNextSecondaryAttack = m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.0s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0s;

			SetThink(&CKnifeMagicKnife::DelaySecondaryAttack);
			pev->nextthink = gpGlobals->time + 0.3s;
			break;
		}
		case 5:
		{
#ifndef CLIENT_DLL	
			m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
			SendWeaponAnim(ANIM_CHARGE_ATTACK2, UseDecrement() != FALSE);
			m_iLastAnim = ANIM_CHARGE_ATTACK2;

			m_flNextSecondaryAttack = m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.0s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0s;

			EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "weapons/magicknife_charge_attack2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

			SetThink(&CKnifeMagicKnife::DelayChargeAttack);
			pev->nextthink = gpGlobals->time + 0.3s;

			break;
		}
		}
		m_iState2 = 0;
	}

	void CKnifeMagicKnife::DelayChargeAttack(void)
	{
		UTIL_MakeVectors(m_pPlayer->pev->v_angle);
		Vector vecSrc = m_pPlayer->GetGunPosition();

#ifndef CLIENT_DLL	
		float flDamage = GetSecondaryAttackDamage();
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
		switch (KnifeAttack(flDamage, 125, 120, DMG_NEVERGIB | DMG_BULLET, this->pev, m_pPlayer, false))
		{
		case HIT_PLAYER:
		{
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/tomahawk_slash1_hit.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			break;
		}
		case HIT_WALL:
		{
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/combatknife_wall.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			break;
		}
		}

		for (int i = 0; i < 10; i++)
		{
			if (m_pMagicCircle[i])
			{		
				m_pMagicCircle[i]->Explode();
				m_pMagicCircle[i] = NULL;
				m_pEnemyList[i] = NULL;
			}
		}
		pev->iuser1 = 0;
		pev->iuser2 = 0;
#endif
		SetThink(nullptr);

	}


	void CKnifeMagicKnife::DelaySecondaryAttack(void)
	{
		UTIL_MakeVectors(m_pPlayer->pev->v_angle);
		Vector vecSrc = m_pPlayer->GetGunPosition();
		
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/magicknife_slash4.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
#ifndef CLIENT_DLL	
		float flDamage = GetSecondaryAttackDamage();
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
		switch (KnifeAttack(flDamage, 125, 120, DMG_NEVERGIB | DMG_BULLET, this->pev, m_pPlayer, false))
		{
		case HIT_PLAYER:
		{
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_AUTO, "weapons/tomahawk_slash1_hit.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			break;
		}
		case HIT_WALL:
		{
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_AUTO, "weapons/combatknife_wall.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			break;
		}
		}
#endif

		SetThink(nullptr);
	}

	float CKnifeMagicKnife::GetMaxSpeed(void)
	{
			return 255;
	}

	void CKnifeMagicKnife::Holster(int skiplocal)
	{
		m_iState = 0;
		m_iState2 = 0;
		SetThink(nullptr);
#ifndef CLIENT_DLL
		for (int i = 0; i < 10; i++)
		{
			if (m_pMagicCircle[i])
			{
				m_pMagicCircle[i]->Remove();
				m_pEnemyList[i] = NULL;
				m_pMagicCircle[i] = NULL;
			}
			if(m_pEnemyList[i])
				m_pEnemyList[i] = NULL;
		}
		pev->iuser1 = 0;
		pev->iuser2 = 0;
#endif
		return CBasePlayerWeapon::Holster(skiplocal);
	}

	void CKnifeMagicKnife::WeaponIdle(void)
	{
		ResetEmptySound();
		m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

		if (m_iState2 && !(m_pPlayer->pev->button & IN_ATTACK2))
			OnAttack2Released();

		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
		SendWeaponAnim(ANIM_IDLE, UseDecrement() != FALSE);

		
	}

	void CKnifeMagicKnife::ItemPostFrame()
	{
		if (m_tNextThink != invalid_time_point && m_tNextThink < gpGlobals->time)
		{
			switch (m_iThinkType)
			{
			case 1:
			{
				SendWeaponAnim(ANIM_SLASH_END, UseDecrement() != FALSE);
				m_iLastAnim = ANIM_SLASH_END;
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.87s;
				m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.75s;
				m_iState = 0;
				m_tNextThink = invalid_time_point;
				m_iThinkType = 0;
				break;
			}
			default:
			{
				m_tNextThink = invalid_time_point;
				break;
			}
			}
		}

		return CBasePlayerWeapon::ItemPostFrame();
	}

#ifndef CLIENT_DLL
	void CMagicCircle::Spawn(void)
	{
		Precache();
		pev->classname = MAKE_STRING("magicknife");
		SetThink(&CMagicCircle::OnThink);

		pev->movetype = MOVETYPE_NOCLIP;
		pev->solid = SOLID_NOT;
		m_flMaxFrames = 45;
		pev->frame = 0;
		m_flAnimTime = gpGlobals->time + 0.5s;
		pev->nextthink = gpGlobals->time + 0.1s;
		SET_MODEL(ENT(pev), "sprites/ef_mgknife_start.spr");
		pev->scale = 0.5;
		pev->animtime = gpGlobals->time;
		pev->framerate = 30.0;
		pev->rendermode = kRenderTransAdd; // 5;
		pev->renderamt = 255.0;
	}

	void EXPORT CMagicCircle::OnThink()
	{
		pev->nextthink = gpGlobals->time + 0.01s;

		this->pev->frame = (float)(this->pev->framerate * gpGlobals->frametime / 1s) + this->pev->frame;
		if (pev->frame > m_flMaxFrames)
		{
			pev->frame = fmod(pev->frame, m_flMaxFrames);
			if (m_flAnimTime < gpGlobals->time)
			{
				SET_MODEL(ENT(pev), "sprites/ef_mgknife_loop.spr");
				m_flMaxFrames = 15;
				pev->frame = 0;
				m_flAnimTime = gpGlobals->time + 9999s;
			}
		}

		if (!m_pOwner || m_pOwner->m_bIsZombie || !m_pAttachedEnt || !m_pAttachedEnt->IsAlive())
			Remove();

		if (bIsWallBetweenEntity(m_pAttachedEnt, m_pOwner))
			Remove();

		if((m_pOwner->pev->origin - m_pAttachedEnt->pev->origin).Length() > 13.6 * 39.19)
			Remove();

		if(!m_pWeapon)
			Remove();

		vecDir =(m_pAttachedEnt->pev->origin - m_pOwner->pev->origin).Normalize();
		Vector angle = UTIL_VecToAngles(vecDir);
		angle[0] *= -1.0;
		pev->angles = angle;
		pev->origin = m_pAttachedEnt->pev->origin - vecDir * 20;
	}

	void CMagicCircle::Explode()
	{
		if (m_pOwner && m_pAttachedEnt)
		{
			if (m_pOwner->pev->takedamage != DAMAGE_NO && m_pAttachedEnt->IsAlive())
			{
				Vector vecDirection = (m_pAttachedEnt->pev->origin - m_pOwner->pev->origin).Normalize();

				TraceResult tr;

				UTIL_TraceLine(m_pOwner->pev->origin, m_pAttachedEnt->pev->origin, dont_ignore_monsters, m_pOwner->edict(), &tr);

				ClearMultiDamage();
				tr.iHitgroup = HITGROUP_CHEST;
				m_pAttachedEnt->TraceAttack(m_pOwner->pev, GetDamage(), vecDirection, &tr, DMG_NEVERGIB | DMG_BULLET);
				ApplyMultiDamage(this->pev, m_pOwner->pev);

				//m_pAttachedEnt->TakeDamage(this->pev, m_pOwner->pev, GetDamage(), DMG_NEVERGIB | DMG_BULLET);
			}
		}

		EMIT_SOUND_DYN(ENT(pev), CHAN_AUTO, "weapons/magicknife_magic_hit.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

		PLAYBACK_EVENT_FULL(FEV_GLOBAL, ENT(pev), PRECACHE_EVENT(1, "events/wpneffects.sc"), 0.0, pev->origin, g_vecZero, 0.0, 0.0, 25, m_pOwner->entindex(), FALSE, FALSE);

		SetThink(nullptr);
		pev->effects |= EF_NODRAW; // 0x80u
		return UTIL_Remove(this);
	}
	void CMagicCircle::Init(CBasePlayer* pOwner, CBaseEntity* pAttachedEnt, CKnifeMagicKnife* pWeapon)
	{
		m_pOwner = pOwner;
		m_pAttachedEnt = pAttachedEnt;
		m_pWeapon = pWeapon;
	}
#endif

}
