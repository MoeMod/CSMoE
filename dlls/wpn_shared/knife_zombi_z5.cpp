/*
knife_zombi.cpp
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
#include "wpn_knife.h"
#include "weapons/knifeattack.h"
#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#include "player/player_mod_strategy.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

	class CKnifeZombiZ5 : public CKnife
	{
		enum knife_e
		{
			KNIFE_IDLE,
			KNIFE_ATTACK1HIT,
			KNIFE_ATTACK2HIT,
			KNIFE_DRAW,
			KNIFE_STABHIT,
			KNIFE_STABMISS,
			KNIFE_MIDATTACK1HIT,
			KNIFE_MIDATTACK2HIT
		};
	public:
		void Precache() override;
		int GetItemInfo(ItemInfo* p) override;
		BOOL Deploy() override;
		float GetMaxSpeed() override 
		{ 
			if (m_iState >= 2)
				return 240;
			else
				return 290;
		}
		int iItemSlot() override { return KNIFE_SLOT; }
		void PrimaryAttack() override;
		void SecondaryAttack() override;
		void OnAttack2Released();

		void Holster(int skiplocal) override
		{
			PLAYBACK_EVENT_FULL(FEV_HOSTONLY, ENT(m_pPlayer->pev), m_usKnife, 0, g_vecZero, g_vecZero, 0, 0, 2, 0, TRUE, 4);
	
			return CBasePlayerWeapon::Holster(skiplocal);
		}

		BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
			return TRUE;
#else
			return FALSE;
#endif
		}
		void WeaponIdle() override;

	public:
		int Stab(int fFirst);
		int Swing(int fFirst);
		int m_iState;
		time_point_t m_flTimeCheckCharge;
	};

	LINK_ENTITY_TO_CLASS(knife_zombi_z5, CKnifeZombiZ5)

	void CKnifeZombiZ5::Precache(void)
	{
#ifndef CLIENT_DLL
		PRECACHE_MODEL("sprites/muzzleflash164.spr");
#endif
		PRECACHE_SOUND("zombi/deathknight_slash_wall.wav");
		PRECACHE_SOUND("zombi/deathknight_skill_charge_start.wav");
		PRECACHE_SOUND("zombi/deathknight_skill_charge_fx.wav");
		PRECACHE_SOUND("zombi/deathknight_skill_charge_end.wav");
		PRECACHE_SOUND("zombi/deathknight_skill_guard_start.wav");
		PRECACHE_SOUND("zombi/deathknight_skill_guard_end.wav");
		PRECACHE_SOUND("zombi/deathknight_skill_flash_start.wav");
		PRECACHE_SOUND("zombi/deathknight_skill_flash_end.wav");
		PRECACHE_SOUND("zombi/deathknight_slash.wav");
		PRECACHE_SOUND("zombi/deathknight_slash2.wav");
		PRECACHE_SOUND("zombi/deathknight_stab.wav");
		
		m_usKnife = PRECACHE_EVENT(1, "events/knife.sc");
	}

	int CKnifeZombiZ5::GetItemInfo(ItemInfo* p)
	{
		CKnife::GetItemInfo(p);
		p->pszName = "weapon_knife";
		p->iWeight = 5;
		p->iId = WEAPON_KNIFE;

		return 1;
	}

	BOOL CKnifeZombiZ5::Deploy(void)
	{
		m_fMaxSpeed = 290;
		m_iSwing = 0;
		m_iState = 0;
		m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;
		m_pPlayer->m_bShieldDrawn = false;

		PLAYBACK_EVENT_FULL(FEV_HOSTONLY, ENT(m_pPlayer->pev), m_usKnife, 0, g_vecZero, g_vecZero, 0, 0, 2, 0, TRUE, 4);

		return DefaultDeploy("models/v_knife_zombi.mdl", "", KNIFE_DRAW, "knife", UseDecrement() != FALSE);
	}

	void CKnifeZombiZ5::PrimaryAttack(void)
	{
		Swing(TRUE);
	}

	void CKnifeZombiZ5::SecondaryAttack(void)
	{	
		if (m_pPlayer->IsBot())
		{
			m_iState = 1;
			OnAttack2Released();
			return;
		}

		if (!m_iState)
		{
			SendWeaponAnim(12, UseDecrement() != FALSE);

			EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "zombi/deathknight_skill_charge_start.wav", VOL_NORM, ATTN_NORM);

			m_flTimeCheckCharge = gpGlobals->time + 0.2s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;
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
					

					SendWeaponAnim(13, UseDecrement() != FALSE);
					m_iState = 2;

					m_pPlayer->ResetMaxSpeed();
					m_flTimeCheckCharge = gpGlobals->time + 0.5s;
					m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 9999s;

					break;
				}
				case 2:
				{
					if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
						strcpy(m_pPlayer->m_szCustomSequence, "skill_charge_crouch_idle");
					else
						strcpy(m_pPlayer->m_szCustomSequence, "skill_charge");

					m_pPlayer->m_flCustomSequenceUntilTime = gpGlobals->time + 9999s;

					//charge 1
					m_iState = 3;

					EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "zombi/deathknight_skill_charge_fx.wav", VOL_NORM, ATTN_NORM);

					m_flTimeCheckCharge = gpGlobals->time + 0.5s;

					PLAYBACK_EVENT_FULL(FEV_HOSTONLY, ENT(m_pPlayer->pev), m_usKnife, 0, g_vecZero, g_vecZero, 0, 0, m_iState, 0, TRUE, 4);

					break;
				}
				case 3:
				{
					//charge 2
					m_iState = 4;

					m_flTimeCheckCharge = gpGlobals->time + 0.5s;

					PLAYBACK_EVENT_FULL(FEV_HOSTONLY, ENT(m_pPlayer->pev), m_usKnife, 0, g_vecZero, g_vecZero, 0, 0, m_iState, 0, TRUE, 4);

					break;
				}
				case 4:
				{
					//charge 3
					m_iState = 5;

					m_flTimeCheckCharge = gpGlobals->time + 9999s;

					PLAYBACK_EVENT_FULL(FEV_HOSTONLY, ENT(m_pPlayer->pev), m_usKnife, 0, g_vecZero, g_vecZero, 0, 0, m_iState, 0, TRUE, 4);

					break;
				}
				}
			}
		}

	}

	void CKnifeZombiZ5::OnAttack2Released(void)
	{
		switch (m_iState)
		{
		case 1:
		{
			Stab(TRUE);
			pev->nextthink = invalid_time_point + UTIL_WeaponTimeBase() + 0.35s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;
			break;
		}
		case 2:
		case 3:
		case 4:
		case 5:
		{
			EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "zombi/deathknight_skill_charge_end.wav", VOL_NORM, ATTN_NORM);

			PLAYBACK_EVENT_FULL(FEV_HOSTONLY, ENT(m_pPlayer->pev), m_usKnife, 0, g_vecZero, g_vecZero, 0, 0, 2, 0, FALSE, 4);

			if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
				strcpy(m_pPlayer->m_szCustomSequence, "skill_charge_crouch_shoot");
			else
				strcpy(m_pPlayer->m_szCustomSequence, "skill_charge_shoot");

			m_pPlayer->m_flCustomSequenceUntilTime = gpGlobals->time + 1s;

			m_iSwing = 0;
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0s;
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.0s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.7s;

			SendWeaponAnim(14, UseDecrement() != FALSE);

			BOOL fDidHit = FALSE;
			UTIL_MakeVectors(m_pPlayer->pev->v_angle);
			Vector vecSrc = m_pPlayer->GetGunPosition();
			
#ifndef CLIENT_DLL
			m_pPlayer->SetAnimation(PLAYER_ATTACK1);
			float flRange = 45.0f;
			flRange = m_pPlayer->m_pModStrategy->AdjustZombieKnifeRange(true, flRange);

			flRange += (m_iState - 2) * 10.0;
			Vector vecEnd = vecSrc + gpGlobals->v_forward * flRange;

			float flDamage = 1000 + (m_iState - 2) * 300.0;
			m_iState = 0;

			switch (KnifeAttack5(flDamage, flRange, 90, DMG_NEVERGIB | DMG_EXPLOSION, this->pev, m_pPlayer))
			{
			case HIT_NONE:
			{
				break;
			}
			case HIT_PLAYER:
			{
				m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;
				EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "zombi/zombi_attack_3.wav", VOL_NORM, ATTN_NORM);	
				break;
			}
			case HIT_WALL:
			{
				m_pPlayer->m_iWeaponVolume = KNIFE_WALLHIT_VOLUME * 0.5;
				EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "zombi/deathknight_slash_wall.wav", VOL_NORM, ATTN_NORM);
				break;
			}
			}
#endif
			break;
		}
		}
		m_iState = 0;
		m_pPlayer->ResetMaxSpeed();
	}

	void CKnifeZombiZ5::WeaponIdle(void)
	{
		ResetEmptySound();
		m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

		if (m_iState && !(m_pPlayer->pev->button & IN_ATTACK2))
			OnAttack2Released();
		
		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;

		if (m_pPlayer->m_bShieldDrawn != true)
		{
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
			SendWeaponAnim(KNIFE_IDLE, UseDecrement() != FALSE);
		}
	}

	int CKnifeZombiZ5::Swing(int fFirst)
	{

#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
		BOOL fDidHit = FALSE;
		UTIL_MakeVectors(m_pPlayer->pev->v_angle);
		Vector vecSrc = m_pPlayer->GetGunPosition();
		float flRange = 32.0f;
#ifndef CLIENT_DLL
		flRange = m_pPlayer->m_pModStrategy->AdjustZombieKnifeRange(false, flRange);
#endif
		Vector vecEnd = vecSrc + gpGlobals->v_forward * flRange;

		switch (RANDOM_LONG(0, 1))
		{
		case 0: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "zombi/deathknight_slash.wav", VOL_NORM, ATTN_NORM); break;
		case 1: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "zombi/deathknight_slash2.wav", VOL_NORM, ATTN_NORM); break;
		}

		TraceResult tr;
		UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr);

		if (tr.flFraction >= 1)
		{
			UTIL_TraceHull(vecSrc, vecEnd, dont_ignore_monsters, head_hull, ENT(m_pPlayer->pev), &tr);

			if (tr.flFraction < 1)
			{
				CBaseEntity* pHit = CBaseEntity::Instance(tr.pHit);

				if (!pHit || pHit->IsBSPModel())
					FindHullIntersection(vecSrc, tr, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX, ENT(m_pPlayer->pev));

				vecEnd = tr.vecEndPos;
			}
		}

		if (tr.flFraction >= 1)
		{
			if (fFirst)
			{
				switch ((m_iSwing++) % 2)
				{
				case 0: SendWeaponAnim(KNIFE_MIDATTACK1HIT, UseDecrement() != FALSE); break;
				case 1: SendWeaponAnim(KNIFE_MIDATTACK2HIT, UseDecrement() != FALSE); break;
				}

				m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.5s;
				m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.5s;

				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;
			}
		}
		else
		{
			fDidHit = TRUE;

			switch ((m_iSwing++) % 2)
			{
			case 0: SendWeaponAnim(KNIFE_MIDATTACK1HIT, UseDecrement() != FALSE); break;
			case 1: SendWeaponAnim(KNIFE_MIDATTACK2HIT, UseDecrement() != FALSE); break;
			}

			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.5s;
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.5s;

			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;

			CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);
			SetPlayerShieldAnim();

			ClearMultiDamage();
			if (pEntity)
			{
				pEntity->TraceAttack(m_pPlayer->pev, 300, gpGlobals->v_forward, &tr, DMG_NEVERGIB | DMG_BULLET);
			}
			ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);

			float flVol = 1;
#ifndef CLIENT_DLL
			int fHitWorld = TRUE;
#endif
			if (pEntity)
			{
				if (pEntity->Classify() != CLASS_NONE && pEntity->Classify() != CLASS_MACHINE)
				{
					switch (RANDOM_LONG(0, 1))
					{
					case 0: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "zombi/zombi_attack_1.wav", VOL_NORM, ATTN_NORM); break;
					case 1: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "zombi/zombi_attack_2.wav", VOL_NORM, ATTN_NORM); break;
					}

					m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;

					if (!pEntity->IsAlive())
						return TRUE;

					flVol = 0.1;
#ifndef CLIENT_DLL
					fHitWorld = FALSE;
#endif
				}
			}

#ifndef CLIENT_DLL
			if (fHitWorld)
			{
				TEXTURETYPE_PlaySound(&tr, vecSrc, vecSrc + (vecEnd - vecSrc) * 2, BULLET_PLAYER_CROWBAR);

				EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "zombi/deathknight_slash_wall.wav", VOL_NORM, ATTN_NORM);
			}
#endif

			m_trHit = tr;
			m_pPlayer->m_iWeaponVolume = flVol * KNIFE_WALLHIT_VOLUME;

			SetThink(&CKnifeZombiZ5::Smack);
			pev->nextthink = invalid_time_point + UTIL_WeaponTimeBase() + 0.2s;
			SetPlayerShieldAnim();
		}

		return fDidHit;
	}

	int CKnifeZombiZ5::Stab(int fFirst)
	{
		BOOL fDidHit = FALSE;
		UTIL_MakeVectors(m_pPlayer->pev->v_angle);
		Vector vecSrc = m_pPlayer->GetGunPosition();
		float flRange = 45.0f;
#ifndef CLIENT_DLL
		flRange = m_pPlayer->m_pModStrategy->AdjustZombieKnifeRange(true, flRange);
#endif
		Vector vecEnd = vecSrc + gpGlobals->v_forward * flRange;

		TraceResult tr;
		UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr);

		EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "zombi/deathknight_stab.wav", VOL_NORM, ATTN_NORM);

		if (tr.flFraction >= 1)
		{
			UTIL_TraceHull(vecSrc, vecEnd, dont_ignore_monsters, head_hull, ENT(m_pPlayer->pev), &tr);

			if (tr.flFraction < 1)
			{
				CBaseEntity* pHit = CBaseEntity::Instance(tr.pHit);

				if (!pHit || pHit->IsBSPModel())
					FindHullIntersection(vecSrc, tr, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX, ENT(m_pPlayer->pev));

				vecEnd = tr.vecEndPos;
			}
		}

		if (tr.flFraction >= 1)
		{
			if (fFirst)
			{
				SendWeaponAnim(KNIFE_STABMISS, UseDecrement() != FALSE);

				m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1s;
				m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1s;

#ifndef CLIENT_DLL
				m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
			}
		}
		else
		{
			fDidHit = TRUE;

#ifndef CLIENT_DLL
			SendWeaponAnim(KNIFE_STABHIT, UseDecrement() != FALSE);
#endif
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1s;
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1s;

			CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);

#ifndef CLIENT_DLL
			m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
			float flDamage = 200.0;

			if (pEntity && pEntity->IsPlayer())
			{
				Vector2D vec2LOS;
				Vector vecForward = gpGlobals->v_forward;

				UTIL_MakeVectors(m_pPlayer->pev->angles);

				vec2LOS = vecForward.Make2D();
				vec2LOS = vec2LOS.Normalize();

				if (DotProduct(vec2LOS, gpGlobals->v_forward.Make2D()) > 0.8)
					flDamage *= 3.0;
			}

			UTIL_MakeVectors(m_pPlayer->pev->v_angle);
			ClearMultiDamage();
			if (pEntity)
				pEntity->TraceAttack(m_pPlayer->pev, flDamage, gpGlobals->v_forward, &tr, DMG_NEVERGIB | DMG_BULLET);
			ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);

			float flVol = 1;
#ifndef CLIENT_DLL
			int fHitWorld = TRUE;
#endif
			if (pEntity)
			{
				if (pEntity->Classify() != CLASS_NONE && pEntity->Classify() != CLASS_MACHINE)
				{
					EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "zombi/zombi_attack_3.wav", VOL_NORM, ATTN_NORM);
					m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;

					if (!pEntity->IsAlive())
						return TRUE;

					flVol = 0.1;
#ifndef CLIENT_DLL
					fHitWorld = FALSE;
#endif
				}
			}

#ifndef CLIENT_DLL
			if (fHitWorld)
			{
				TEXTURETYPE_PlaySound(&tr, vecSrc, vecSrc + (vecEnd - vecSrc) * 2, BULLET_PLAYER_CROWBAR);

				EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "zombi/deathknight_slash_wall.wav", VOL_NORM, ATTN_NORM);
			}
#endif

			m_trHit = tr;
			m_pPlayer->m_iWeaponVolume = flVol * KNIFE_WALLHIT_VOLUME;

			SetThink(&CKnifeZombiZ5::Smack);
			pev->nextthink = invalid_time_point + UTIL_WeaponTimeBase() + 0.2s;
		}

		return fDidHit;
	}

}
