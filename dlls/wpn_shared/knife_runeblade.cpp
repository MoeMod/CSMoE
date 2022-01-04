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
		static constexpr const char* AnimExtension = "dragonsword";
		KnockbackData KnockBack = { 1.0f, 1.0f, 1.0f, 1.0f, 0.5f };

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

			PRECACHE_MODEL("sprites/runeblade_ef.spr");
			PRECACHE_MODEL("sprites/runeblade_ef02.spr");
			PRECACHE_MODEL("sprites/runeblade_eye.spr");
			PRECACHE_MODEL("sprites/ef_runeblade_swing.spr");

			PRECACHE_SOUND("weapons/runeblade-slash1.wav");
			PRECACHE_SOUND("weapons/runeblade-slash2.wav");
			PRECACHE_SOUND("weapons/runeblade_v_charge_attack1.wav");
			PRECACHE_SOUND("weapons/runeblade_v_charge_attack2.wav");
		}
		
		BOOL Deploy(void) override
		{
			m_fMaxSpeed = 250;
			m_iState = 0;
		
			m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;
			
			m_tTimeRestMaxSpeed = m_tTimeSoundEmit = m_flTimeCheckCharge = invalid_time_point;
			m_pPlayer->m_bShieldDrawn = false;

			if (DefaultDeploy(V_Model, P_Model, ANIM_DRAW, "dragonsword", UseDecrement() != FALSE))
			{
				m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.1s;
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.2s;
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
#ifndef CLIENT_DLL
		void MaterialSound();
# endif
		BOOL CanDrop() override { return false; }
		void ItemPostFrame() override;

	public:
		float GetPrimaryAttackDamage() const
		{
			float flDamage = 30;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 1836.0f;
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
				flDamage = 1785.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage *= 3600.0f;
#endif
			return flDamage;
		}

		float  GetChargeAttackDamage() const
		{
			float flDamage = 35;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 2465.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 1280.0f;
#endif
			return flDamage;
		}

		time_point_t m_flTimeCheckCharge;
		time_point_t m_tTimeSoundEmit;
		time_point_t m_tTimeRestMaxSpeed;
		int m_iDamageType;
		int m_iState;	//pev_iuser1
		int m_iState2;	//pev_iuser4
		int m_iActSlash3;
		int m_iCheck;	//g_iCheck
		int m_iFrozen;	//g_Frozen
		int m_iCheckSpecial;	//CheckSpecial
	};

	LINK_ENTITY_TO_CLASS(knife_runeblade, CKnifeRuneBlade)

	void CKnifeRuneBlade::Spawn()
	{
		pev->classname = MAKE_STRING("knife_runeblade");
		m_iId = WEAPON_KNIFE;
		Precache();
		SET_MODEL(ENT(pev), "models/w_knife.mdl");
		m_tTimeRestMaxSpeed = m_tTimeSoundEmit = m_flTimeCheckCharge = invalid_time_point;
		m_iClip = WEAPON_NOCLIP;
		m_iState = 0;
		m_iState2 = 0;
		m_iCheck = 0;
		m_iActSlash3 = 0;
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

#ifndef CLIENT_DLL
	void CKnifeRuneBlade::MaterialSound()
	{
		UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
		Vector vecSrc = m_pPlayer->GetGunPosition();
		Vector vecEnd = vecSrc + gpGlobals->v_forward * 8192;
		TraceResult tr;
		UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr);
		char chTextureType = 0;
		char szbuffer[64];
		if (tr.flFraction < 1)
		{
			const char* pTextureName;
			pTextureName = TRACE_TEXTURE(tr.pHit, vecSrc, vecEnd);

			if (pTextureName)
			{
				// strip leading '-0' or '+0~' or '{' or '!'
				if (*pTextureName == '-' || *pTextureName == '+')
					pTextureName += 2;

				if (*pTextureName == '{' || *pTextureName == '!' || *pTextureName == '~' || *pTextureName == ' ')
					pTextureName++;

				// '}}'
				Q_strcpy(szbuffer, pTextureName);
				szbuffer[CBTEXTURENAMEMAX - 1] = '\0';

				// get texture type
				chTextureType = TEXTURETYPE_Find(szbuffer);
			}

			switch (chTextureType)
			{
				case CHAR_TEX_METAL:
				case CHAR_TEX_VENT:
				case CHAR_TEX_COMPUTER:
					{
						if(RANDOM_LONG(0,1))
							EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_AUTO, "weapons/runeblade_wall_metal1.wav", VOL_NORM, ATTN_NORM, 0, 94);
						else
							EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_AUTO, "weapons/runeblade_wall_metal2.wav", VOL_NORM, ATTN_NORM, 0, 94);					
						break;
					}
				case CHAR_TEX_WOOD:
				case CHAR_TEX_SLOSH:
					{
						if (RANDOM_LONG(0, 1))
							EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_AUTO, "weapons/runeblade_wall_stone1.wav", VOL_NORM, ATTN_NORM, 0, 94);
						else
							EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_AUTO, "weapons/runeblade_wall_stone1.wav", VOL_NORM, ATTN_NORM, 0, 94);
						break;
					}
				default:
				{
					if (RANDOM_LONG(0, 1))
						EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_AUTO, "weapons/runeblade_wall_metal1.wav", VOL_NORM, ATTN_NORM, 0, 94);
					else
						EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_AUTO, "weapons/runeblade_wall_metal2.wav", VOL_NORM, ATTN_NORM, 0, 94);
					break;
				}
			}
		}
	}
#endif

	void CKnifeRuneBlade::DelayPrimaryAttack(void)
	{
#ifndef CLIENT_DLL		
	
#endif

		SetThink(nullptr);
	}

	void CKnifeRuneBlade::PrimaryAttack(void)
	{

		SendWeaponAnim(ANIM_SLASH, UseDecrement() != FALSE);
		SetThink(&CKnifeRuneBlade::DelayPrimaryAttack);
		pev->nextthink = gpGlobals->time + 0.35s;

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.3s;				
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.85s;
	}

	void CKnifeRuneBlade::SecondaryAttack(void)
	{
		if (!m_iState)
		{
			SendWeaponAnim(ANIM_STAB, UseDecrement() != FALSE);
			m_flTimeCheckCharge = gpGlobals->time + 0.5s;
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
		case 0:
		{
#ifndef CLIENT_DLL	
			m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

			m_flNextSecondaryAttack = m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.5s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;

			SetThink(&CKnifeRuneBlade::DelaySecondaryAttack);
			pev->nextthink = gpGlobals->time + 1.0s;
			break;
		}
		case 1:
		case 2:
		case 3:
		{
			SendWeaponAnim(ANIM_CHARGE_ATTACK1, UseDecrement() != FALSE);

			SetThink(&CKnifeRuneBlade::DelaySecondaryAttack);
			pev->nextthink = gpGlobals->time + 0.35s;
			break;
		}
		case 4:
		case 5:
		{
			SendWeaponAnim(ANIM_CHARGE_ATTACK2, UseDecrement() != FALSE);

			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.86s;
			m_iState2 = 6;


			SetThink(nullptr);
			SetThink(&CKnifeRuneBlade::DelaySecondaryAttack);
			pev->nextthink = gpGlobals->time + 0.35s;
			break;
		}
		}
	}

	void CKnifeRuneBlade::DelaySecondaryAttack(void)
	{
#ifndef CLIENT_DLL		
		UTIL_MakeVectors(m_pPlayer->pev->v_angle);
		Vector vecSrc = m_pPlayer->GetGunPosition();
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);

		switch (KnifeAttack4(vecSrc, gpGlobals->v_forward, GetSecondaryAttackDamage(), 150, 90, DMG_NEVERGIB | DMG_BULLET, m_pPlayer->pev, m_pPlayer->pev , 0, 2))
		{
		case HIT_PLAYER:
		{
			
			break;
		}
		case HIT_WALL:
		{
			MaterialSound();
			break;
		}
		}
#endif

		SetThink(nullptr);
	}

	float CKnifeRuneBlade::GetMaxSpeed(void)
	{
			return 250;
	}

	void CKnifeRuneBlade::Holster(int skiplocal)
	{
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
		
	}

	void CKnifeRuneBlade::ItemPostFrame()
	{
		return CBasePlayerWeapon::ItemPostFrame();
	}

}
