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

		static constexpr int MaxClip = 100;
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

			PRECACHE_SOUND("weapons/combatknife_hit_2.wav");
			PRECACHE_SOUND("weapons/combatknife_wall.wav");
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
			m_pPlayer->m_bShieldDrawn = false;
			m_iLastAnim = ANIM_DRAW;

			if (DefaultDeploy(V_Model, P_Model, ANIM_DRAW, "sfsword", UseDecrement() != FALSE, 0.3s, 1.0s))
			{
				m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0s;
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;
			}
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
		void OnAttack2Released();
		void CreateHitFX(Vector pos, int iType);
		bool AngularCheck(CBaseEntity* pEntity, CBaseEntity* pAttacker, Vector vecSrc, Vector vecDir, Vector2D vecAngleCheckDir, float flRange, float flMinimumCosine, TraceResult* ptr, int iType);

		BOOL CanDrop() override { return false; }
		void ItemPostFrame() override;

	public:
		float GetPrimaryAttackDamage() const
		{
			float flDamage = 55;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 1240.0f;	//original 1836
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 1240.0f;
#endif
			return flDamage;
		}
		float GetSecondaryAttackDamage() const
		{
			float flDamage = 70;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 2065.0f;	//3298
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 3600.0f;
#endif
			return flDamage;
		}

		float  GetChargeAttackDamage() const
		{
			float flDamage = 130;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 2865.0f;	//4573
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 4573.0f;
#endif
			return flDamage;
		}

		time_point_t m_flTimeCheckCharge;

#ifndef CLIENT_DLL
		int m_ExpIndex;
		int m_Exp2Index;
		EntityHandle <CMagicCircle> m_pMagicCircle;
		std::vector<EHANDLE> m_pEnemyList;
#endif
		

		int m_iPrimarySprEffect;
		int m_iSecondarySprEffect;
		int m_iState;
		int m_iState2;	//pev_iuser1
		int m_iLastAnim;
		int m_iThinkType;
		time_point_t m_tNextThink;
	};


#ifndef CLIENT_DLL
	class CMagicCircle : public CBaseEntity
	{
	public:
		CMagicCircle()
		{
			m_flRemoveTime = invalid_time_point;
		}

		void Spawn(void);
		void EXPORT OnThink();

		void Init(CBasePlayer* pOwner, CKnifeHolySword* pWeapon);

		void Precache(void)
		{
			PRECACHE_MODEL("sprites/ef_mgknife_end.spr");
			PRECACHE_MODEL("sprites/ef_mgknife_exp01.spr");
			PRECACHE_MODEL("sprites/ef_mgknife_exp02.spr");
			PRECACHE_MODEL("sprites/ef_mgknife_loop.spr");
			PRECACHE_MODEL("sprites/ef_mgknife_start.spr");
		}

		EntityHandle <CBasePlayer> m_pOwner;
		EntityHandle <CKnifeMagicKnife> m_pWeapon;
		time_point_t m_flRemoveTime;

		void Remove()
		{
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


	bool CKnifeMagicKnife::AngularCheck(CBaseEntity* pEntity, CBaseEntity* pAttacker, Vector vecSrc, Vector vecDir, Vector2D vecAngleCheckDir, float flRange, float flMinimumCosine, TraceResult* ptr, int iType)
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

		//if ((!friendlyfire.value && pEntity->m_iTeam == pAttacker->m_iTeam) || !bResult)
		if (!bResult)
			return bResult;

		if (!m_iSwing)
		{
			if (pEntity->IsPlayer())
			{
				if (FBitSet(pEntity->pev->flags, FL_DUCKING))
					CKnifeMagicKnife::CreateHitFX(pEntity->pev->origin + Vector(0, 0, 10), iType);
				else
					CKnifeMagicKnife::CreateHitFX(pEntity->pev->origin + Vector(0, 0, -10), iType);
			}
			else
			{
				if (FBitSet(pEntity->pev->flags, FL_DUCKING))
					CKnifeMagicKnife::CreateHitFX(pEntity->pev->origin + Vector(0, 0, 10), iType);
				else
					CKnifeMagicKnife::CreateHitFX(pEntity->pev->origin + Vector(0, 0, -10), iType);
			}
		}
#endif

		return bResult;
}


	void CKnifeMagicKnife::CreateHitFX(Vector pos, int iType)
	{
#ifndef CLIENT_DLL
		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_EXPLOSION);
		WRITE_COORD(pos.x);
		WRITE_COORD(pos.y);
		WRITE_COORD(pos.z);
		WRITE_SHORT(iType ? m_iSecondarySprEffect : m_iPrimarySprEffect);
		WRITE_BYTE(iType ? 10 : 3);
		WRITE_BYTE(30);
		WRITE_BYTE(TE_EXPLFLAG_NODLIGHTS | TE_EXPLFLAG_NOPARTICLES | TE_EXPLFLAG_NOSOUND);
		MESSAGE_END();
#endif
	}

	void CKnifeMagicKnife::DelayPrimaryAttack(void)
	{
#ifndef CLIENT_DLL		
		m_iSwing = 0;
		CBaseEntity* pEntity = NULL;
		BOOL fDidHit = FALSE;
		TraceResult tr;
		UTIL_MakeVectors(m_pPlayer->pev->v_angle);
		Vector vecEnd;
		Vector vecSrc = m_pPlayer->GetGunPosition();

		Vector vecForward = gpGlobals->v_forward;
		Vector2D vecForward2D = vecForward.Make2D().Normalize();

		float c = cos(180.0 * 0.5 * M_PI / 180.0);

		while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, 115.0)) != NULL)
		{
			if (pEntity == m_pPlayer)
				continue;

			if (pEntity->IsDormant())
				continue;

			if (pEntity->pev->takedamage == DAMAGE_NO)
				continue;

			if (bIsWallBetweenEntity(pEntity, pAttackPlayer))
				continue;

			if (!AngularCheck(pEntity, m_pPlayer, vecSrc, vecForward, vecForward2D, 110.0, c, &tr, 0))
				continue;

			float flRatio = 1.0;

			if (pEntity && pEntity->IsPlayer())
			{
				UTIL_MakeVectors(pEntity->pev->angles);

				if (DotProduct((pEntity->pev->origin.Make2D() - m_pPlayer->pev->origin.Make2D()).Normalize(), gpGlobals->v_forward.Make2D()) > 0.8)
				{
					flRatio = 3.0;
					bitsDamageType |= DMG_BACKATK;
				}
			}

			m_pPlayer->m_iWeaponVolume = 128;

			EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_ITEM, "weapons/tomahawk_slash1_hit.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

			ClearMultiDamage();

			pEntity->TraceAttack(m_pPlayer->pev, GetPrimaryAttackDamage() * flRatio, (tr.vecEndPos - vecSrc).Normalize(), &tr, DMG_BULLET | DMG_NEVERGIB);

			ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);

			fDidHit = TRUE;
		}

		if (!fDidHit)
		{
			vecEnd = vecSrc + vecForward * 115.0;

			UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, m_pPlayer->edict(), &tr);

			if (tr.flFraction < 1.0f)
			{
				//MaterialSound(&tr, vecSrc, vecEnd);
				CKnifeMagicKnife::CreateHitFX(tr.vecEndPos, 0);
				EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_ITEM, "weapons/tomahawk_wall.wav", VOL_NORM, ATTN_NORM, 0, 94);
			}
		}
		else
#endif
		SetThink(nullptr);
	}

	void CKnifeMagicKnife::SecondaryAttack_FindTargets()
	{
		m_pEnemyList.clear();
#ifndef CLIENT_DLL
		CBaseEntity* pEntity = NULL;
		while ((pEntity = UTIL_FindEntityInSphere(pEntity, m_pPlayer->pev->origin, SECNDARYATTACK_RADIUS)) != nullptr)
		{
			if (SecondaryAttack_CheckTargetAvailable(pEntity, m_pPlayer->pev->v_angle))
			{
				EHANDLE eh;
				eh.Set(pEntity->edict());
				m_pEnemyList.push_back(eh);
			}
		}
#endif
	}

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
			}
			case 1:
			{
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/magicknife_slash2.wav", VOL_NORM, ATTN_NORM, 0, 94);

				m_iState = 2;
				SendWeaponAnim(ANIM_SLASH2, UseDecrement() != FALSE);
				m_iLastAnim = ANIM_SLASH2;
			}
			case 2:
			{
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/magicknife_slash3.wav", VOL_NORM, ATTN_NORM, 0, 94);

				m_iState = 3;
				SendWeaponAnim(ANIM_SLASH3, UseDecrement() != FALSE);
				m_iLastAnim = ANIM_SLASH3;
			}
			case 3:
			{
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/magicknife_slash4.wav", VOL_NORM, ATTN_NORM, 0, 94);

				m_iState = 0;
				SendWeaponAnim(ANIM_SLASH4, UseDecrement() != FALSE);
				m_iLastAnim = ANIM_SLASH4;
			}
		}

		SetThink(&CKnifeSummonKnife::DelayPrimaryAttack);
		pev->nextthink = gpGlobals->time + 0.1s;

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.53s;
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.36s;
		m_tNextThink = gpGlobals->time + 0.5s;
		m_iThinkType = 1;
	}

	void CKnifeMagicKnife::SecondaryAttack(void)
	{
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
					m_flTimeCheckCharge = gpGlobals->time + 999999s;
					break;	
				}
				case 5:
				{
#ifndef CLIENT_DLL
					m_pMagicCircle = static_cast<CMagicCircle*>(CBaseEntity::Create("holysword_parray", m_pPlayer->pev->origin + vecForward * 35, vecAngle, ENT(m_pPlayer->pev)));
					if (m_pParray)
					{
						m_pParray->Init(m_pPlayer, this);
					}
#endif
					m_flTimeCheckCharge = gpGlobals->time + 0.2s;
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
			m_iSwing = 0;
			m_flNextSecondaryAttack = m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.56s;

			EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "weapons/knife_magicknife-slash2.wav", VOL_NORM, ATTN_NORM, 0, 94);
			
			SetThink(&CKnifeMagicKnife::DelaySecondaryAttack);
			pev->nextthink = gpGlobals->time + 1.33s;
			break;
		}
		case 2:
		case 3:
		case 4:
		{
#ifndef CLIENT_DLL	
			m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
			m_iSwing = 0;
			SendWeaponAnim(ANIM_CHARGE_ATTACK1, UseDecrement() != FALSE);
			m_iLastAnim = ANIM_CHARGE_ATTACK1;

			m_flNextSecondaryAttack = m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.6s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.3s;

			EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "weapons/knife_magicknife.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

			SetThink(&CKnifeMagicKnife::DelaySecondaryAttack);
			pev->nextthink = gpGlobals->time + 0.5s;
			break;
		}
		case 5:
		{
#ifndef CLIENT_DLL	
			m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
			m_iSwing = 1;
			SendWeaponAnim(ANIM_CHARGE_ATTACK2, UseDecrement() != FALSE);
			m_iLastAnim = ANIM_CHARGE_ATTACK2;

			m_flNextSecondaryAttack = m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.8s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.3s;

			EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "weapons/knife_magicknife.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

			SetThink(&CKnifeMagicKnife::DelaySecondaryAttack);
			pev->nextthink = gpGlobals->time + 0.5s;

			break;
		}
		}
		m_iState2 = 0;
	}

	void CKnifeMagicKnife::DelaySecondaryAttack(void)
	{
#ifndef CLIENT_DLL
		float flDamage, flRadius, flAngleDegrees;
		int bitsDamageType = DMG_BULLET | DMG_NEVERGIB
		flAngleDegrees = 180;
		flDamage = m_iSwing ? GetChargeAttackDamage() : GetSecondaryAttackDamage();
		flRadius = m_iSwing ? 120 : 120;
		
		CBaseEntity* pEntity = NULL;
		BOOL fDidHit = FALSE;
		TraceResult tr;
		
		Vector vecEnd, vecAngle;
		vecAngle = m_pPlayer->pev->v_angle;
		UTIL_MakeVectors(vecAngle);
		Vector vecSrc = m_pPlayer->GetGunPosition();

		Vector vecForward = gpGlobals->v_forward;
		Vector2D vecForward2D = vecForward.Make2D().Normalize();

		if (m_iSwing)
		{
			vecAngle[0] = 360.0 - vecAngle[0];
			CKnifeMagicKnife::CreateHitFX(vecSrc + vecForward * 40, m_iSwing);
		}
		float c = cos(flAngleDegrees * 0.5 * M_PI / flRadius);

		while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, flRadius)) != NULL)
		{
			if (pEntity == m_pPlayer)
				continue;

			if (pEntity->IsDormant())
				continue;

			if (pEntity->pev->takedamage == DAMAGE_NO)
				continue;

			if (!AngularCheck(pEntity, m_pPlayer, vecSrc, vecForward, vecForward2D, flRadius, c, &tr, m_iSwing))
				continue;

			float flRatio = 1.0;

			if (pEntity && pEntity->IsPlayer())
			{
				UTIL_MakeVectors(pEntity->pev->angles);

				if (DotProduct((pEntity->pev->origin.Make2D() - m_pPlayer->pev->origin.Make2D()).Normalize(), gpGlobals->v_forward.Make2D()) > 0.8)
				{
					flRatio = 3.0;
					bitsDamageType |= DMG_BACKATK;
				}
			}

			m_pPlayer->m_iWeaponVolume = 128;

			if (m_iSwing)
			{
				EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "weapons/laserminigun_exp2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
				EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_ITEM, "weapons/tomahawk_slash1_hit.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			}
			else
			{
				EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_ITEM, "weapons/tomahawk_slash1_hit.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			}

			ClearMultiDamage();

			pEntity->TraceAttack(m_pPlayer->pev, flDamage * flRatio, (tr.vecEndPos - vecSrc).Normalize(), &tr, bitsDamageType);

			ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);

			CBasePlayer* pPlayer = (CBasePlayer*)pEntity;
			if (pPlayer->IsPlayer() && pPlayer->m_bIsZombie)
			{
				Vector2D vecDir2D = (pPlayer->pev->origin.Make2D() - m_pPlayer->pev->origin.Make2D()).Normalize();

				pPlayer->pev->velocity.x = vecDir2D.x * 300;
				pPlayer->pev->velocity.y = vecDir2D.y * 300;
				pPlayer->pev->velocity.z = 400;

				pPlayer->m_flVelocityModifier = 0.9;
			}

			fDidHit = TRUE;
		}

		if (!fDidHit)
		{
			vecEnd = vecSrc + vecForward * 110.0;

			UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, m_pPlayer->edict(), &tr);

			if (tr.flFraction < 1.0f)
			{
				if (m_iSwing)
				{
					EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "weapons/laserminigun_exp2.wav", VOL_NORM, ATTN_NORM, 0, 94);
					EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_ITEM, "weapons/tomahawk_slash1_hit.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
				}
				else
				{
					CKnifeMagicKnife::CreateHitFX(tr.vecEndPos, m_iSwing);
					EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_ITEM, "weapons/tomahawk_wall.wav", VOL_NORM, ATTN_NORM, 0, 94);
				}
			}
			else
			{
				if (m_iSwing)
				{
					CKnifeMagicKnife::CreateHitFX(vecSrc + vecForward * 55.0, m_iSwing);
					EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "weapons/laserminigun_exp2.wav", VOL_NORM, ATTN_NORM, 0, 94);
				}
			}
		}

		if (m_iState2 == 5)
		{

			m_pEnemyList.clear();
		}
#endif
		m_iState2 = 0;
		m_iSwing = 0;
		SetThink(nullptr);
	}

	float CKnifeMagicKnife::GetMaxSpeed(void)
	{
			return 250;
	}

	void CKnifeMagicKnife::Holster(int skiplocal)
	{
		m_iState = 0;
		m_iState2 = 0;
		SetThink(nullptr);

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
		PLAYBACK_EVENT_FULL(FEV_HOSTONLY, m_pPlayer->edict(), PRECACHE_EVENT(1, "events/wpneffects.sc"), 0.0, g_vecZero, g_vecZero, 0.0, 0.0, 14, 0, TRUE, FALSE);
		
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
				m_fl
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

		SetThink(&CHolySwordParray::OnThink);

		pev->deadflag = DEAD_NO;
		pev->takedamage = DAMAGE_YES;

		pev->solid = SOLID_TRIGGER;
		pev->movetype = MOVETYPE_NOCLIP;

		UTIL_SetSize(pev, Vector(-45.0, -55.0, -80.0), Vector(45.0, 55.0, 64.0));

		m_flRemoveTime = gpGlobals->time + 3.0s;
		pev->nextthink = gpGlobals->time + 0.1s;
#if DEBUG_DRAWBOX
		//SET_MODEL(ENT(pev), "models/summoner.mdl");
		m_flDrawTime = gpGlobals->time + 0.2s;
		DrawBeam();
#endif
	}

	void EXPORT CMagicCircle::OnThink()
	{
		pev->nextthink = gpGlobals->time + 0.01s;

		if (!m_pOwner || m_pOwner->m_bIsZombie)
		{
			Remove();
		}

		if (bIsWallBetweenEntity(pEntity, m_pOwner))

		UTIL_MakeVectors(m_pOwner->pev->v_angle);

		Vector vecForward = gpGlobals->v_forward;
		vecForward.z = 0;
		vecForward = vecForward.Normalize();
		UTIL_SetOrigin(pev, m_pOwner->pev->origin + vecForward * 35);

		pev->angles = m_pOwner->pev->angles;
		pev->angles.x = 0;

#if DEBUG_DRAWBOX
		if (m_flDrawTime < gpGlobals->time)
		{
			DrawBeam();
			m_flDrawTime = gpGlobals->time + 0.2s;
		}
#endif
		if (m_flRemoveTime < gpGlobals->time)
		{
			Remove();
		}
	}

	void CMagicCircle::Init(CBasePlayer* pOwner, CKnifeMagicKnife* pWeapon)
	{
		m_pOwner = pOwner;
		m_pWeapon = pWeapon;
	}
#endif

}
