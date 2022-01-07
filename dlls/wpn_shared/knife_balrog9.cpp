/*
knife_runeblade.cpp
Copyright (C) 2021 Original Author BTE Team


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

	class CKnifeRuneBlade : public LinkWeaponTemplate<CKnifeRuneBlade,
		TGeneralData,
		BuildTGetItemInfoFromCSW<WEAPON_KNIFE>::template type
	>
	{
	public:
		static constexpr const char* ClassName = "knife_runeblade";
		static constexpr const char* V_Model = "models/v_runeblade.mdl";
		static constexpr const char* P_Model = "models/p_runeblade.mdl";
		static constexpr const char* W_Model = "models/w_knife.mdl";

		static constexpr int MaxClip = 100;
		static constexpr auto ItemSlot = KNIFE_SLOT;
		static constexpr const char* AnimExtension = "sfsword";
		KnockbackData KnockBack = { 1.0f, 1.0f, 1.0f, 1.0f, 0.8f };

	public:

		enum runeblade_e
		{
			ANIM_IDLE = 0,
			ANIM_SLASH,
			ANIM_DRAW,
			ANIM_CHARGE_START1,
			ANIM_CHARGE_FINISH1,
			ANIM_CHARGE_IDLE1,
			ANIM_CHARGE_IDLE2,
			ANIM_CHARGE_ATTACK1,
			ANIM_CHARGE_ATTACK2,
			ANIM_STAB,
		};

		void Precache() override
		{
			PRECACHE_MODEL("models/v_runeblade.mdl");
			PRECACHE_MODEL("models/p_runeblade.mdl");
			PRECACHE_MODEL("models/ef_runeblade_swing.mdl");

			m_iSecondarySprEffect = PRECACHE_MODEL("sprites/runeblade_ef.spr");
			m_iPrimarySprEffect = PRECACHE_MODEL("sprites/runeblade_ef02.spr");
			PRECACHE_MODEL("sprites/runeblade_eye.spr");
			PRECACHE_MODEL("sprites/ef_runeblade_swing.spr");

			PRECACHE_SOUND("weapons/runeblade-slash1.wav");
			PRECACHE_SOUND("weapons/runeblade-slash2.wav");
			PRECACHE_SOUND("weapons/runeblade_v_charge_attack1.wav");
			PRECACHE_SOUND("weapons/runeblade_v_charge_attack2.wav");
			PRECACHE_SOUND("weapons/tomahawk_slash1_hit.wav");
			PRECACHE_SOUND("weapons/laserminigun_exp2.wav");
		}
		
		BOOL Deploy(void) override
		{
			m_fMaxSpeed = 250;
			m_iState = 0;
		
			m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;
			
			m_flTimeCheckCharge = invalid_time_point;
			m_pPlayer->m_bShieldDrawn = false;

			if (DefaultDeploy(V_Model, P_Model, ANIM_DRAW, "sfsword", UseDecrement() != FALSE))
			{
				m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0s;
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;
				return TRUE;
			}
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
				flDamage = 2618.0f;	//original 1836
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
				flDamage = 3298.0f;	//1785
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
				flDamage = 4573.0f;	//2465
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 4573.0f;
#endif
			return flDamage;
		}

		time_point_t m_flTimeCheckCharge;

		int m_iPrimarySprEffect;
		int m_iSecondarySprEffect;
		int m_iState;	//pev_iuser1
	};

	LINK_ENTITY_TO_CLASS(knife_runeblade, CKnifeRuneBlade)

	void CKnifeRuneBlade::Spawn()
	{
		pev->classname = MAKE_STRING("knife_runeblade");
		m_iId = WEAPON_KNIFE;
		Precache();
		SET_MODEL(ENT(pev), "models/w_knife.mdl");
		m_flTimeCheckCharge = invalid_time_point;
		m_iClip = WEAPON_NOCLIP;
		m_iState = 0;
		FallInit();
	}

	int CKnifeRuneBlade::GetItemInfo(ItemInfo* p)
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


	bool CKnifeRuneBlade::AngularCheck(CBaseEntity* pEntity, CBaseEntity* pAttacker, Vector vecSrc, Vector vecDir, Vector2D vecAngleCheckDir, float flRange, float flMinimumCosine, TraceResult* ptr, int iType)
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
					CKnifeRuneBlade::CreateHitFX(pEntity->pev->origin + Vector(0, 0, 10), iType);
				else
					CKnifeRuneBlade::CreateHitFX(pEntity->pev->origin + Vector(0, 0, -10), iType);
			}
			else
			{
				if (FBitSet(pEntity->pev->flags, FL_DUCKING))
					CKnifeRuneBlade::CreateHitFX(pEntity->pev->origin + Vector(0, 0, 10), iType);
				else
					CKnifeRuneBlade::CreateHitFX(pEntity->pev->origin + Vector(0, 0, -10), iType);
			}
		}
#endif

		return bResult;
}


	void CKnifeRuneBlade::CreateHitFX(Vector pos, int iType)
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

	void CKnifeRuneBlade::DelayPrimaryAttack(void)
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

		float c = cos(200.0 * 0.5 * M_PI / 180.0);

		while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, 110.0)) != NULL)
		{
			if (pEntity == m_pPlayer)
				continue;

			if (pEntity->IsDormant())
				continue;

			if (pEntity->pev->takedamage == DAMAGE_NO)
				continue;

			if (!AngularCheck(pEntity, m_pPlayer, vecSrc, vecForward, vecForward2D, 110.0, c, &tr, 0))
				continue;

			float flRatio = 1.0;

			if (pEntity && pEntity->IsPlayer())
			{
				UTIL_MakeVectors(pEntity->pev->angles);

				if (DotProduct((pEntity->pev->origin.Make2D() - m_pPlayer->pev->origin.Make2D()).Normalize(), gpGlobals->v_forward.Make2D()) > 0.8)
					flRatio = 3.0;
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
			vecEnd = vecSrc + vecForward * 110.0;

			UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, m_pPlayer->edict(), &tr);

			if (tr.flFraction < 1.0f)
			{
				//MaterialSound(&tr, vecSrc, vecEnd);
				CKnifeRuneBlade::CreateHitFX(tr.vecEndPos, 0);
				EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_ITEM, "weapons/tomahawk_wall.wav", VOL_NORM, ATTN_NORM, 0, 94);
			}
		}
		else
#endif
		SetThink(nullptr);
	}

	void CKnifeRuneBlade::PrimaryAttack(void)
	{

		SendWeaponAnim(ANIM_SLASH, UseDecrement() != FALSE);
#ifndef CLIENT_DLL	
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
		SetThink(&CKnifeRuneBlade::DelayPrimaryAttack);
		pev->nextthink = gpGlobals->time + 0.35s;

		EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "weapons/runeblade-slash1.wav", VOL_NORM, ATTN_NORM, 0, 94);

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.03s;				
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.2s;
	}

	void CKnifeRuneBlade::SecondaryAttack(void)
	{
		if (!m_iState)
		{
			SendWeaponAnim(ANIM_STAB, UseDecrement() != FALSE);

			m_flTimeCheckCharge = gpGlobals->time + 0.5s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.7s;
			m_iState = 1;
		}
		else
		{
			if (m_flTimeCheckCharge < gpGlobals->time)
			{
				switch (m_iState)
				{
				case 1:
				{
					SendWeaponAnim(ANIM_CHARGE_START1, UseDecrement() != FALSE);

					m_iState = 2;

					m_flTimeCheckCharge = gpGlobals->time + 0.37s;
					m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.37s;
					m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.85s;
					break;
				}
				case 2:
				{
					SendWeaponAnim(ANIM_CHARGE_IDLE1, UseDecrement() != FALSE);

					m_iState = 3;

					m_flTimeCheckCharge = gpGlobals->time + 1.23s;
					m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 999999s;
					m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.23s;
					break;
				}
				case 3:
				{
					SendWeaponAnim(ANIM_CHARGE_FINISH1, UseDecrement() != FALSE);

					m_iState = 4;

					m_flTimeCheckCharge = gpGlobals->time + 0.35s;
					m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 999999s;
					m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.33s;
					break;
				}
				case 4:
				{
					SendWeaponAnim(ANIM_CHARGE_IDLE2, UseDecrement() != FALSE);

					m_iState = 5;

					m_flTimeCheckCharge = gpGlobals->time + 0.1s;
					m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 999999s;
					m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.1s;
					break;	
				}
				case 5:
				{
					m_flTimeCheckCharge = gpGlobals->time + 999999s;
					break;
				}
				}
			}
		}
	}

	void CKnifeRuneBlade::OnAttack2Released(void)
	{
		switch (m_iState)
		{
		case 1:
		{
#ifndef CLIENT_DLL	
			m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
			m_iSwing = 0;
			m_flNextSecondaryAttack = m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.3s;

			EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "weapons/runeblade-slash2.wav", VOL_NORM, ATTN_NORM, 0, 94);
			
			SetThink(&CKnifeRuneBlade::DelaySecondaryAttack);
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
			m_iSwing = 0;
			SendWeaponAnim(ANIM_CHARGE_ATTACK1, UseDecrement() != FALSE);

			m_flNextSecondaryAttack = m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.3s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0s;

			EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "weapons/runeblade_v_charge_attack1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

			SetThink(&CKnifeRuneBlade::DelaySecondaryAttack);
			pev->nextthink = gpGlobals->time + 0.35s;
			break;
		}
		case 5:
		{
#ifndef CLIENT_DLL	
			m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
			m_iSwing = 1;
			SendWeaponAnim(ANIM_CHARGE_ATTACK2, UseDecrement() != FALSE);

			m_flNextSecondaryAttack = m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.3s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0s;

			EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "weapons/runeblade_v_charge_attack2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

			SetThink(&CKnifeRuneBlade::DelaySecondaryAttack);
			pev->nextthink = gpGlobals->time + 0.35s;

			break;
		}
		}
		m_iState = 0;
	}

	void CKnifeRuneBlade::DelaySecondaryAttack(void)
	{
#ifndef CLIENT_DLL
		float flDamage, flRadius, flAngleDegrees;
		flAngleDegrees = m_iSwing ? 120 : 60;
		flDamage = m_iSwing ? GetChargeAttackDamage() : GetSecondaryAttackDamage();
		flRadius = m_iSwing ? 210 : 120;
		
		CBaseEntity* pEntity = NULL;
		BOOL fDidHit = FALSE;
		TraceResult tr;
		UTIL_MakeVectors(m_pPlayer->pev->v_angle);
		Vector vecEnd;
		Vector vecSrc = m_pPlayer->GetGunPosition();

		Vector vecForward = gpGlobals->v_forward;
		Vector2D vecForward2D = vecForward.Make2D().Normalize();

		if (m_iSwing)
		{
			PLAYBACK_EVENT_FULL(FEV_HOSTONLY, m_pPlayer->edict(), PRECACHE_EVENT(1, "events/wpneffects.sc"), 0.0, (float*)&g_vecZero, (float*)&g_vecZero, 0.0, (m_pPlayer->pev->animtime - gpGlobals->time) / 1s, 13, 0, FALSE, FALSE);
			CKnifeRuneBlade::CreateHitFX(vecSrc + vecForward * 40, m_iSwing);
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
					flRatio = 3.0;
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

			pEntity->TraceAttack(m_pPlayer->pev, flDamage * flRatio, (tr.vecEndPos - vecSrc).Normalize(), &tr, DMG_BULLET | DMG_NEVERGIB);

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
					CKnifeRuneBlade::CreateHitFX(tr.vecEndPos, m_iSwing);
					EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_ITEM, "weapons/tomahawk_wall.wav", VOL_NORM, ATTN_NORM, 0, 94);
				}
			}
			else
			{
				if (m_iSwing)
				{
					CKnifeRuneBlade::CreateHitFX(vecSrc + vecForward * 55.0, m_iSwing);
					EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "weapons/laserminigun_exp2.wav", VOL_NORM, ATTN_NORM, 0, 94);
				}
			}
		}
#endif
		m_iState = 0;
		m_iSwing = 0;
		SetThink(nullptr);
	}

	float CKnifeRuneBlade::GetMaxSpeed(void)
	{
			return 250;
	}

	void CKnifeRuneBlade::Holster(int skiplocal)
	{
		m_iState = 0;
		SetThink(nullptr);

		return CBasePlayerWeapon::Holster(skiplocal);
	}

	void CKnifeRuneBlade::WeaponIdle(void)
	{
		ResetEmptySound();
		m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

		if (m_iState && !(m_pPlayer->pev->button & IN_ATTACK2))
			OnAttack2Released();

		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
		SendWeaponAnim(ANIM_IDLE, UseDecrement() != FALSE);
		PLAYBACK_EVENT_FULL(FEV_HOSTONLY, m_pPlayer->edict(), PRECACHE_EVENT(1, "events/wpneffects.sc"), 0.0, (float*)&g_vecZero, (float*)&g_vecZero, 0.0, 0.0, 14, 0, TRUE, FALSE);
		
	}

	void CKnifeRuneBlade::ItemPostFrame()
	{
		return CBasePlayerWeapon::ItemPostFrame();
	}

}
