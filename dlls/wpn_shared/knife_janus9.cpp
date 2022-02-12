/*
knife_janus9.cpp
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

	class CKnifeJanus9 : public LinkWeaponTemplate<CKnifeJanus9,
		TGeneralData,
		BuildTGetItemInfoFromCSW<WEAPON_KNIFE>::template type
	>
	{
	public:
		static constexpr const char* ClassName = "knife_janus9";
		static constexpr const char* V_Model = "models/v_janus9.mdl";
		static constexpr const char* P_Model = "models/p_janus9_a.mdl";
		static constexpr const char* P_Model_B = "models/p_janus9_b.mdl";
		static constexpr const char* W_Model = "models/w_knife.mdl";

		static constexpr int MaxClip = 100;
		static constexpr auto ItemSlot = KNIFE_SLOT;
		static constexpr const char* AnimExtension = "sfsword";
		KnockbackData KnockBack = { 0.0f, 1.0f, 1.0f, 0.0f, 1.0f };

	public:

		enum Janus9_e
		{
			ANIM_IDLE = 0,
			ANIM_IDLE_SIGNAL,
			ANIM_ENDSIGNAL,
			ANIM_SLASH1,
			ANIM_SLASH1_STARTSIGNAL,
			ANIM_SLASH1_SIGNAL,
			ANIM_SLASH2,
			ANIM_SLASH2_STARTSIGNAL,
			ANIM_SLASH2_SIGNAL,
			ANIM_DRAW,
			ANIM_DRAW_SIGNAL,
			ANIM_STAB1,
			ANIM_STAB2
		};

		void Precache() override
		{
			PRECACHE_MODEL("models/v_janus9.mdl");
			PRECACHE_MODEL("models/p_janus9_a.mdl");
			PRECACHE_MODEL("models/p_janus9_b.mdl");
		
			PRECACHE_SOUND("weapons/janus9_draw.wav");
			PRECACHE_SOUND("weapons/janus9_endsignal.wav");
			PRECACHE_SOUND("weapons/janus9_hit1.wav");
			PRECACHE_SOUND("weapons/janus9_hit2.wav");
			PRECACHE_SOUND("weapons/janus9_slash1.wav");
			PRECACHE_SOUND("weapons/janus9_slash2_signal.wav");
			PRECACHE_SOUND("weapons/janus9_stab1.wav");
			PRECACHE_SOUND("weapons/janus9_stab2.wav");
			PRECACHE_SOUND("weapons/janus9_metal1.wav");
			PRECACHE_SOUND("weapons/janus9_metal2.wav");
			PRECACHE_SOUND("weapons/janus9_stone1.wav");
			PRECACHE_SOUND("weapons/janus9_stone2.wav");
			PRECACHE_SOUND("weapons/janus9_wood1.wav");
			PRECACHE_SOUND("weapons/janus9_wood2.wav");
		}
		
		BOOL Deploy(void) override
		{
			m_fMaxSpeed = 250;

			m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;
			
			m_pPlayer->m_bShieldDrawn = false;

			if (m_flTimeSignalOver != invalid_time_point && m_flTimeSignalOver > gpGlobals->time)
				return DefaultDeploy(V_Model, P_Model_B, ANIM_DRAW_SIGNAL, "tomahawk", UseDecrement() != FALSE, 0.64s);
			return DefaultDeploy(V_Model, P_Model, ANIM_DRAW, "tomahawk", UseDecrement() != FALSE, 0.64s);
	
		}

		int GetItemInfo(ItemInfo* p) override;
		void PrimaryAttack() override;	
		void SecondaryAttack() override;
		void Spawn() override;
		void WeaponIdle() override;
		float GetMaxSpeed() override;
		void MaterialSound(float flRadius);
		BOOL CanDrop() override { return false; }
		void ItemPostFrame() override;

	public:
		float GetPrimaryAttackDamage() const
		{
			float flDamage = 48.0f;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 447.0f;	//original 1836
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 1341.0f;
#endif
			return flDamage;
		}
		float GetSecondaryAttackDamage() const
		{
			float flDamage = 110.0f;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 1045.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 3135.0f;
#endif
			return flDamage;
		}

		float  GetSignalAttackDamage() const
		{
			float flDamage = 52.0f;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 492.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 1476.0f;
#endif
			return flDamage;
		}

		time_point_t m_flTimeSignalOver;
		//pev->iuser1 Signal
	};

	LINK_ENTITY_TO_CLASS(knife_janus9, CKnifeJanus9)

	void CKnifeJanus9::Spawn()
	{
		pev->classname = MAKE_STRING("knife_janus9");
		m_iId = WEAPON_KNIFE;
		Precache();
		SET_MODEL(ENT(pev), "models/w_knife.mdl");
		m_flTimeSignalOver = invalid_time_point;
		m_iClip = WEAPON_NOCLIP;

		FallInit();
	}

	int CKnifeJanus9::GetItemInfo(ItemInfo* p)
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


	void CKnifeJanus9::MaterialSound(float flRadius)
	{
#ifndef CLIENT_DLL
		UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
		Vector vecStart = m_pPlayer->GetGunPosition();
		Vector vecEnd = vecStart + gpGlobals->v_forward * flRadius;
		TraceResult ptr;
		UTIL_TraceLine(vecStart, vecEnd, dont_ignore_monsters, ENT(m_pPlayer->pev), &ptr);

		const char* rgsz[2];

		switch (UTIL_TextureHit(&ptr, vecStart, vecEnd))
		{
		case CHAR_TEX_WOOD:
			rgsz[0] = "weapons/janus9_wood1.wav";
			rgsz[1] = "weapons/janus9_wood2.wav";

			break;

		case CHAR_TEX_GRATE:
		case CHAR_TEX_METAL:
		case CHAR_TEX_COMPUTER:
			rgsz[0] = "weapons/janus9_metal1.wav";
			rgsz[1] = "weapons/janus9_metal2.wav";

			break;

		default:
			rgsz[0] = "weapons/janus9_stone1.wav";
			rgsz[1] = "weapons/janus9_stone2.wav";

			break;
		}

		UTIL_EmitAmbientSound(ENT(0), ptr.vecEndPos, rgsz[RANDOM_LONG(0, 1)], 0.9, ATTN_NORM, 0, 96 + RANDOM_LONG(0, 15));
#endif
	}

	void CKnifeJanus9::PrimaryAttack(void)
	{
		UTIL_MakeVectors(m_pPlayer->pev->v_angle);
		Vector vecSrc = m_pPlayer->GetGunPosition();
#ifndef CLIENT_DLL	
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
		bool fDidHit = FALSE;

		m_iSwing++;
		pev->iuser1++;

		if (pev->iuser1 >= 3)
			pev->iuser1 = 3;
#ifndef CLIENT_DLL
		if (m_iSwing & 1)
		{
			if (pev->iuser1 == 3)
			{
				SendWeaponAnim(ANIM_SLASH1_SIGNAL, UseDecrement() != FALSE);
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/janus9_slash1.wav", VOL_NORM, ATTN_NORM, 0, 94);
			}
			else if (pev->iuser1 == 2)
			{
				SendWeaponAnim(ANIM_SLASH1_STARTSIGNAL, UseDecrement() != FALSE);
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/janus9_slash2_signal.wav", VOL_NORM, ATTN_NORM, 0, 94);
				m_flTimeSignalOver = gpGlobals->time + 3.0s;
			}
			else
			{
				SendWeaponAnim(ANIM_SLASH1, UseDecrement() != FALSE);
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/janus9_slash1.wav", VOL_NORM, ATTN_NORM, 0, 94);
			}
		}
		else
		{
			if (pev->iuser1 == 3)
			{
				SendWeaponAnim(ANIM_SLASH2_SIGNAL, UseDecrement() != FALSE);
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/janus9_slash1.wav", VOL_NORM, ATTN_NORM, 0, 94);
			}
			else if (pev->iuser1 == 2)
			{
				SendWeaponAnim(ANIM_SLASH2_STARTSIGNAL, UseDecrement() != FALSE);
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/janus9_slash2_signal.wav", VOL_NORM, ATTN_NORM, 0, 94);			
				m_flTimeSignalOver = gpGlobals->time + 3.0s;
			}
			else
			{
				SendWeaponAnim(ANIM_SLASH2, UseDecrement() != FALSE);
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/janus9_slash1.wav", VOL_NORM, ATTN_NORM, 0, 94);
			}
		}
		

		switch (KnifeAttack(vecSrc, gpGlobals->v_forward, m_iSwing ? GetSignalAttackDamage() : GetPrimaryAttackDamage(), 90, DMG_NEVERGIB | DMG_BULLET, m_pPlayer->pev, m_pPlayer->pev))
		{
			case HIT_NONE:
			{
				break;
			}
			case HIT_PLAYER:
			{
				if (m_iSwing & 1)
					EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/janus9_hit1.wav", VOL_NORM, ATTN_NORM, 0, 94);
				else
					EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/janus9_hit2.wav", VOL_NORM, ATTN_NORM, 0, 94);
				m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;
				fDidHit = TRUE;
				break;
			}
			case HIT_WALL:
			{
				
				m_pPlayer->m_iWeaponVolume = KNIFE_WALLHIT_VOLUME * 0.5;
				MaterialSound(90.0);
				fDidHit = TRUE;
				break;
			}
		}
		
#endif
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.25s;				
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.61s;
	}

	void CKnifeJanus9::SecondaryAttack(void)
	{
		if (pev->iuser1 == 2 || pev->iuser1 == 3)
		{
			m_iSwing++;
#ifndef CLIENT_DLL	
			if (m_iSwing & 1)
			{
				SendWeaponAnim(ANIM_STAB1, UseDecrement() != FALSE);
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/janus9_stab1.wav", VOL_NORM, ATTN_NORM, 0, 94);
			}
			else
			{
				SendWeaponAnim(ANIM_STAB2, UseDecrement() != FALSE);
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/janus9_stab2.wav", VOL_NORM, ATTN_NORM, 0, 94);
			}
			m_pPlayer->SetAnimation(PLAYER_ATTACK1);

			switch (KnifeAttack5(GetSecondaryAttackDamage(), 114, 180, DMG_NEVERGIB | DMG_BULLET, this->pev, m_pPlayer))
			{
			case HIT_NONE:
			{
				break;
			}
			case HIT_PLAYER:
			{
				if (RANDOM_LONG(0, 1))
					EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/janus9_hit1.wav", VOL_NORM, ATTN_NORM, 0, 94);
				else
					EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/janus9_hit2.wav", VOL_NORM, ATTN_NORM, 0, 94);
				m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;
				break;
			}
			case HIT_WALL:
			{
				m_pPlayer->m_iWeaponVolume = KNIFE_WALLHIT_VOLUME * 0.5;
				MaterialSound(90.0);
				break;
			}
			}
#endif
			pev->iuser1 = 0;
			m_flTimeSignalOver = invalid_time_point;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.9s;
			m_flNextSecondaryAttack = m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.84s;
		}
		else
			return;

	}


	float CKnifeJanus9::GetMaxSpeed(void)
	{
		return 250;
	}

	void CKnifeJanus9::WeaponIdle(void)
	{
		ResetEmptySound();
		m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;
	
		int Anim = ANIM_IDLE;
		if (m_flTimeSignalOver != invalid_time_point && m_flTimeSignalOver > gpGlobals->time)
			Anim = ANIM_IDLE_SIGNAL;

		SendWeaponAnim(Anim, UseDecrement() != FALSE);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
		
	}

	void CKnifeJanus9::ItemPostFrame()
	{
		if (m_flTimeSignalOver != invalid_time_point && m_flTimeSignalOver < gpGlobals->time)
		{
			pev->iuser1 = 0;
			m_flTimeSignalOver = invalid_time_point;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.27s;
#ifndef CLIENT_DLL	
			SendWeaponAnim(ANIM_ENDSIGNAL, UseDecrement() != FALSE);
#endif
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/janus9_endsignal.wav", VOL_NORM, ATTN_NORM, 0, 94);	
		}

		return CBasePlayerWeapon::ItemPostFrame();
	}

}
