/*
knife_skullaxe.cpp
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
#include "util/u_range.hpp"
#endif

#include <random>
#include "weapons/KnifeAttack.h"
#include "weapons/WeaponTemplate.hpp"


#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

class CKnifeWhipSword: public LinkWeaponTemplate<CKnifeWhipSword,
	TGeneralData,
	TPrecacheEvent,
	BuildTGetItemInfoFromCSW<WEAPON_KNIFE>::template type
>
{
public:
	enum whipsword_e
	{
		ANIM_IDLE,
		ANIM_SLASH1,
		ANIM_SLASH1_END,
		ANIM_SLASH_LOOP,
		ANIM_SLASH_END,
		ANIM_SLASH_SKILL,
		ANIM_STAB1,
		ANIM_STAB2,
		ANIM_STAB12_END,
		ANIM_STAB_LOOP,
		ANIM_STAB_END,
		ANIM_STAB_SKILL_START,
		ANIM_STAB_SKILL_LOOP,
		ANIM_STAB_SKILL_END,
		ANIM_DRAW,
	};

	enum whipsword_crosshairstate_e
	{
		WHIPSWORD_AIM_OFF = 13,
		WHIPSWORD_AIM_ON,
	};

	static constexpr const char *ClassName = "knife_whipsword";
	static constexpr const char *V_Model = "models/v_whipsword.mdl";
	static constexpr const char *P_Model = "models/p_whipsword.mdl";
	static constexpr const char *W_Model = "models/w_knife.mdl";
	static constexpr const char* EventFile = "events/whipsword.sc";

	static constexpr int MaxClip = -1;
	static constexpr auto ItemSlot = KNIFE_SLOT;
	static constexpr float ArmorRatioModifier = 1.7f;
	static constexpr const char *AnimExtension = "katana";
	KnockbackData KnockBack = { 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };

private:
	int m_iPrimaryAttackRecord; // iPrimaryAttackRecord
	int m_iSecondaryAttackRecord; // iPrimaryAttackRecord
	int m_iStabRecord;
	int m_iThinkType; // iThinkType
	time_point_t m_flNextTimeCanHolster; // flNextTimeCanHolster
	time_point_t m_tNextThink;
	time_point_t m_flTimeNextMessage;
	time_point_t m_tNextAttack;
	whipsword_crosshairstate_e m_CrossHairState;
	whipsword_crosshairstate_e m_CrossHairStateStored;
	Vector m_vecPlayerOrigin;
	Vector m_vecHookOrigin;

public:
	

	BOOL CanHolster() override
	{
		return !pev->iuser2;
	}

	void Precache() override
	{
		Base::Precache();

		PRECACHE_SOUND("weapons/whipsword_slash_end.wav");
		PRECACHE_SOUND("weapons/whipsword_slash_loop_start.wav");
		PRECACHE_SOUND("weapons/whipsword_slash_loop1.wav");
		PRECACHE_SOUND("weapons/whipsword_slash_loop2.wav");
		PRECACHE_SOUND("weapons/whipsword_slash_loop3.wav");
		PRECACHE_SOUND("weapons/whipsword_slash_skill.wav");
		PRECACHE_SOUND("weapons/whipsword_slash1.wav");
		PRECACHE_SOUND("weapons/whipsword_slash1_end.wav");
		PRECACHE_SOUND("weapons/whipsword_slash2.wav");
		PRECACHE_SOUND("weapons/whipsword_slash3.wav");

		PRECACHE_SOUND("weapons/whipsword_stab_end.wav");
		PRECACHE_SOUND("weapons/whipsword_stab_loop_all.wav");
		//PRECACHE_SOUND("weapons/whipsword_stab_loop_all1.wav");
		//PRECACHE_SOUND("weapons/whipsword_stab_loop_all2.wav");
		PRECACHE_SOUND("weapons/whipsword_stab_loop_end.wav");
		//PRECACHE_SOUND("weapons/whipsword_stab_loop1.wav");
		//PRECACHE_SOUND("weapons/whipsword_stab_loop2.wav");
		//PRECACHE_SOUND("weapons/whipsword_stab_loop3.wav");
		PRECACHE_SOUND("weapons/whipsword_stab_skill_flying.wav");
		PRECACHE_SOUND("weapons/whipsword_stab_skill_start.wav");
		PRECACHE_SOUND("weapons/whipsword_stab1.wav");
		PRECACHE_SOUND("weapons/whipsword_stab2.wav");
		PRECACHE_SOUND("weapons/whipsword_stab3.wav");
		PRECACHE_SOUND("weapons/whipsword_stab12_end.wav");
		
		
		PRECACHE_MODEL("models/p_whipsword.mdl");
		PRECACHE_MODEL("models/v_whipsword.mdl");
		PRECACHE_MODEL("models/v_whipsword_1.mdl");

#ifndef CLIENT_DLL
		PRECACHE_MODEL("sprites/muzzleflash230.spr");
		PRECACHE_MODEL("sprites/muzzleflash231.spr");
#endif

		
	}

	BOOL Deploy() override
	{
		m_iPrimaryAttackRecord = 1;
		m_iSecondaryAttackRecord = 1;
		m_iStabRecord = 0;	//stab1
		m_iThinkType = 0;
		m_flNextTimeCanHolster = invalid_time_point;

		m_fMaxSpeed = 260;
		m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;
		m_pPlayer->m_bShieldDrawn = false;

#ifndef CLIENT_DLL
		MESSAGE_BEGIN(MSG_ONE, gmsgSpecialCrossHair, NULL, m_pPlayer->pev);
		WRITE_BYTE(3);
		WRITE_BYTE(m_CrossHairState);
		MESSAGE_END();
		m_pPlayer->UpdateShieldCrosshair(false);
#endif

		return DefaultDeploy(V_Model, P_Model, ANIM_DRAW, AnimExtension, UseDecrement() != FALSE, 0.53s, 1.0s);
	}

	void Spawn() override;
	void WeaponIdle() override;
	float GetMaxSpeed() override { return m_fMaxSpeed; }
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	BOOL CanDrop() override { return false; }

#ifndef CLIENT_DLL
	hit_result_t KnifeAttack(float flDamage, float flRadius, float flAngleDegrees, int bitsDamageType, entvars_t* pevInflictor, CBasePlayer* pAttackPlayer, bool IsPrimaryAttack);
	bool AngularCheck(CBaseEntity* pEntity, CBaseEntity* pAttacker, Vector vecSrc, Vector vecDir, Vector2D vecAngleCheckDir, float flRange, float flMinimumCosine, TraceResult* ptr, bool IsPrimaryAttack);
	
#endif
	void ItemPostFrame() override;
	float GetPrimaryAttackDamage(int iType) const
	{
		float flDamage = iType ? 30 : 40;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			//flDamage = iType ? 1067 : 1333;
			flDamage = iType ? 768 : 850;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = iType ? 3267 : 4900;
#endif
		return flDamage;
	}
	float GetSecondaryAttackDamage(int iType) const
	{
		float flDamage = iType ? 50 : 60;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			//flDamage = iType ? 640 : 1666;
			flDamage = iType ? 540 : 970;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = iType ? 1960 : 5554;
#endif
		return flDamage;
	}

	float GetSlashComboAttackDamage() const
	{
		float flDamage = 100;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			//flDamage = 3530;
			flDamage = 1333;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 12978;
#endif
		return flDamage;
	}

	Vector GetShootPosition(int* bHit = 0)
	{
		Vector end;
		Vector Start = m_pPlayer->GetGunPosition();
		end = m_pPlayer->pev->v_angle;
		UTIL_MakeVectors(end);
		end = gpGlobals->v_forward;
		end = end * 15 * 39.37;
		end = Start + end;

		TraceResult tr;
		UTIL_TraceLine(Start, end, dont_ignore_monsters, m_pPlayer->edict(), &tr);
		end = tr.vecEndPos;
		if (tr.flFraction != 1.0f)
			*bHit = 1;
		return end;
	}

	void Holster(int skiplocal) override
	{
#ifndef CLIENT_DLL
		MESSAGE_BEGIN(MSG_ONE, gmsgSpecialCrossHair, NULL, m_pPlayer->pev);
		WRITE_BYTE(3);
		WRITE_BYTE(3);
		MESSAGE_END();

		MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->pev);
		WRITE_BYTE(79);
		WRITE_SHORT(m_pPlayer->entindex());
		WRITE_SHORT(CHAN_VOICE);
		WRITE_STRING("weapons/whipsword_stab_loop_all.wav");
		MESSAGE_END();
#endif
		return CBasePlayerWeapon::Holster(skiplocal);
	}

};

LINK_ENTITY_TO_CLASS(knife_whipsword, CKnifeWhipSword)

void CKnifeWhipSword::Spawn()
{
	pev->classname = MAKE_STRING("knife_whipsword");
	m_iId = WEAPON_KNIFE;
	m_CrossHairState = m_CrossHairStateStored = WHIPSWORD_AIM_OFF;
	m_flTimeNextMessage = invalid_time_point;
	Precache();
	SET_MODEL(ENT(pev), "models/w_knife.mdl");
	m_iClip = WEAPON_NOCLIP;
	FallInit();
}

void CKnifeWhipSword::WeaponIdle()
{
	if (m_flTimeWeaponIdle > 0.0s)
		return;

	ResetEmptySound();

	m_iPrimaryAttackRecord = 1;
	m_iSecondaryAttackRecord = 1;
#ifndef CLIENT_DLL
	SendWeaponAnim(ANIM_IDLE);
#endif
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20.0s;

}

void CKnifeWhipSword::ItemPostFrame()
{

	if (m_tNextThink != invalid_time_point && m_tNextThink < gpGlobals->time)
	{
		switch (m_iThinkType)
		{
		case 1:
		{
			SendWeaponAnim(ANIM_SLASH1_END, UseDecrement() != FALSE);
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.8s;
			m_tNextThink = gpGlobals->time + 0.3s;
			m_iThinkType = 7;

			EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_VOICE, "weapons/whipsword_slash1_end.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

			break;
		}
		case 2:
		{
			SendWeaponAnim(ANIM_STAB12_END, UseDecrement() != FALSE);
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.8s;
			m_tNextThink = invalid_time_point;
			m_tNextThink = gpGlobals->time + 0.3s;
			m_iThinkType = 7;

			EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_VOICE, "weapons/whipsword_stab12_end.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

			break;
		}
		case 3:
		{
			//slash1_loop_end
			SendWeaponAnim(ANIM_SLASH_END, UseDecrement() != FALSE);
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;
			m_iPrimaryAttackRecord = 1;
			m_iThinkType = 6;
			m_tNextThink = gpGlobals->time + 1.0s;

			EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "weapons/whipsword_slash_end.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

			break;
		}
		case 4:
		{
			//slash1_stab_skill_end
			SendWeaponAnim(ANIM_STAB_END, UseDecrement() != FALSE);
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;
			m_iSecondaryAttackRecord = 1;
			m_iThinkType = 6;
			m_tNextThink = gpGlobals->time + 1.0s;

			EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "weapons/whipsword_stab_loop_end.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

			break;
		}
		case 5:
		{
			SendWeaponAnim(ANIM_STAB_SKILL_LOOP, UseDecrement() != FALSE);
			EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "weapons/whipsword_stab_skill_flying.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 9999s;
			m_tNextThink = invalid_time_point;
			break;
		}
		case 6:
		{
			//can holster
			pev->iuser2 = 0;
			m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_whipsword.mdl");
			m_tNextThink = invalid_time_point;
			break;
		}
		case 7:
		{
			//reset record
			m_iPrimaryAttackRecord = 1;
			m_iSecondaryAttackRecord = 1;
			m_tNextThink = invalid_time_point;
			m_iThinkType = 0;
			break;
		}
		}
	}

	if (m_pPlayer->pev->velocity.z <= 0 && m_iThinkType == 5)
	{
#ifndef CLIENT_DLL
		SendWeaponAnim(ANIM_STAB_SKILL_END, UseDecrement() != FALSE);
#endif
		m_iThinkType = 6;
		m_tNextThink = gpGlobals->time + 0.7s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.7s;
		return CBasePlayerWeapon::ItemPostFrame();
	}

	if (m_iThinkType == 5 && pev->iuser1)
	{
		Vector vecDis = (m_vecHookOrigin - m_pPlayer->pev->origin);
		if (vecDis.Length() < 39.37 * 2)
		{
			pev->iuser1 = 0;
			Vector vecDir = (m_vecHookOrigin - m_vecPlayerOrigin).Normalize();
			vecDir *= 500;
			m_pPlayer->pev->velocity = vecDir;
			m_pPlayer->pev->velocity.z += 50;
		}
		else
		{
			Vector vecDir = (m_vecHookOrigin - m_vecPlayerOrigin).Normalize();
			vecDir *= 500;
			m_pPlayer->pev->velocity = vecDir;
		}
	}
	
	if (m_iThinkType == 3 || m_iThinkType == 4)
	{
		int bHit = 0;

		m_vecHookOrigin = GetShootPosition(&bHit);
		if (bHit)
		{
			m_CrossHairState = WHIPSWORD_AIM_ON;
		}
		else
			m_CrossHairState = WHIPSWORD_AIM_OFF;

		if (m_tNextAttack != invalid_time_point && m_tNextAttack < gpGlobals->time)
		{
#ifndef CLIENT_DLL
			m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
			if (m_iThinkType == 3)
			{
				switch (RANDOM_LONG(0, 3))
				{
				case 1:
					EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/whipsword_slash_loop1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM); break;
				case 2:
					EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/whipsword_slash_loop2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM); break;
				case 3:
					EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/whipsword_slash_loop3.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM); break;
				}
			}

			m_tNextAttack = gpGlobals->time + (m_iThinkType == 3 ? 0.5s : 0.25s);

#ifndef CLIENT_DLL
			const float flDamage = m_iThinkType == 3 ? GetPrimaryAttackDamage(m_iThinkType) : GetSecondaryAttackDamage(m_iThinkType);
			const float flRadius = m_iThinkType == 3 ? 200 : 140;
			const float flAngle = 360;

			switch (KnifeAttack(flDamage, flRadius, flAngle, DMG_NEVERGIB | DMG_BULLET, this->pev, m_pPlayer, false))
			{
			case HIT_NONE:
			{
				break;
			}
			case HIT_PLAYER:
			{
				if (RANDOM_LONG(0, 1))
					EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/dualsword_stab1_hit.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
				else
					EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/dualsword_stab2_hit.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
				m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;
				break;
			}
			case HIT_WALL:
			{
				//EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/mastercombat_wall.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
				m_pPlayer->m_iWeaponVolume = KNIFE_WALLHIT_VOLUME * 0.5;
				break;
			}
			}	
#endif
		}
	}
	else if (m_iThinkType == 1 || m_iThinkType == 2)
	{
		m_CrossHairState = WHIPSWORD_AIM_OFF;
		if (m_tNextAttack != invalid_time_point && m_tNextAttack < gpGlobals->time)
		{
			m_tNextAttack = invalid_time_point;
#ifndef CLIENT_DLL
			const float flDamage = m_iThinkType == 1 ? GetPrimaryAttackDamage(0) : GetSecondaryAttackDamage(0);
			const float flRadius = m_iThinkType == 1 ? 180 : 131;
			const float flAngle = 360;

			switch (KnifeAttack(flDamage, flRadius, flAngle, DMG_NEVERGIB | DMG_BULLET, this->pev, m_pPlayer, m_iThinkType == 1 ? true : false))
			{
			case HIT_NONE:
			{
				break;
			}
			case HIT_PLAYER:
			{
				if (RANDOM_LONG(0, 1))
					EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/dualsword_stab1_hit.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
				else
					EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/dualsword_stab2_hit.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
				m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;
				break;
			}
			case HIT_WALL:
			{
				//EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/mastercombat_wall.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
				m_pPlayer->m_iWeaponVolume = KNIFE_WALLHIT_VOLUME * 0.5;
				break;
			}
			}
#endif
		}
	}
	else
		m_CrossHairState = WHIPSWORD_AIM_OFF;

	if (m_flTimeNextMessage < gpGlobals->time)
	{
		if (m_CrossHairState != m_CrossHairStateStored)
		{
#ifndef CLIENT_DLL
			MESSAGE_BEGIN(MSG_ONE_UNRELIABLE, gmsgSpecialCrossHair, NULL, m_pPlayer->pev);
			WRITE_BYTE(3);
			WRITE_BYTE(m_CrossHairState);
			MESSAGE_END();
#endif
		}

		m_flTimeNextMessage = gpGlobals->time + 0.1s;
		m_CrossHairStateStored = m_CrossHairState;
	}
	return CBasePlayerWeapon::ItemPostFrame();
}

void CKnifeWhipSword::PrimaryAttack(void)
{
	if (!m_iSecondaryAttackRecord)
	{
		m_vecPlayerOrigin = m_pPlayer->pev->origin;

		if (m_CrossHairState == WHIPSWORD_AIM_ON)
		{
			pev->iuser1 = 1;
			pev->iuser2 = 1;
			Vector vecDir = (m_vecHookOrigin - m_vecPlayerOrigin).Normalize();
			vecDir *= 500;
			m_pPlayer->pev->velocity = vecDir;
			m_pPlayer->pev->velocity.z += 250;
			
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0s;
			m_tNextThink = gpGlobals->time + 1.0s;
			m_iThinkType = 5;
			m_iPrimaryAttackRecord = 1;
			m_iSecondaryAttackRecord = 1;
#ifndef CLIENT_DLL
			SendWeaponAnim(ANIM_STAB_SKILL_START);

			MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->pev);
			WRITE_BYTE(79);
			WRITE_SHORT(m_pPlayer->entindex());
			WRITE_SHORT(CHAN_VOICE);
			WRITE_STRING("weapons/whipsword_stab_loop_all.wav");
			MESSAGE_END();

			PLAYBACK_EVENT_FULL(FEV_GLOBAL, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, 0.0, 0.0, 6, 0, FALSE, FALSE);
#endif
			EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "weapons/whipsword_stab_skill_start.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		}

		m_flNextSecondaryAttack = m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.67s;
		return;
	}

	if (m_iPrimaryAttackRecord == 1)
		m_iPrimaryAttackRecord = 2;
	else if (m_iPrimaryAttackRecord == 2)
		m_iPrimaryAttackRecord = 3;
	else if (m_iPrimaryAttackRecord == 3)
		m_iPrimaryAttackRecord = 4;
	else if (m_iPrimaryAttackRecord == 4)
		m_iPrimaryAttackRecord = 5;
	else if (m_iPrimaryAttackRecord == 5)
	{
		m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_whipsword_1.mdl");

		SendWeaponAnim(ANIM_SLASH_LOOP);

		EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "weapons/whipsword_slash_loop_start.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

		m_iPrimaryAttackRecord = 0;
		m_iSecondaryAttackRecord = 1;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.67s;
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 5.0s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 9999.0s;

		m_tNextThink = gpGlobals->time + 5.2s;
		m_tNextAttack = gpGlobals->time + 0.1s;
		m_iThinkType = 3;
#ifndef CLIENT_DLL
		PLAYBACK_EVENT_FULL(FEV_GLOBAL, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, 0.0, 0.0, 5, m_iStabRecord, FALSE, FALSE);
#endif
		return;
	}
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
	PLAYBACK_EVENT_FULL(FEV_GLOBAL, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, 0.0, 0.0, 2, 0, FALSE, FALSE);
#endif
	m_iThinkType = 1;
	m_tNextThink = gpGlobals->time + 0.6s;
	m_tNextAttack = gpGlobals->time + 0.1s;
	m_iSecondaryAttackRecord = 1;

	m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_whipsword.mdl");
	pev->iuser2 = 0;
	
	SendWeaponAnim(ANIM_SLASH1);

	switch (RANDOM_LONG(0, 3))
	{
	case 1:
		EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "weapons/whipsword_slash1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM); break;
	case 2:
		EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "weapons/whipsword_slash2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM); break;
	case 3:
		EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "weapons/whipsword_slash3.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM); break;
	}

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.4s;
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.67s;


}

void CKnifeWhipSword::SecondaryAttack(void)
{
	if (!m_iPrimaryAttackRecord)
	{
#ifndef CLIENT_DLL
		SendWeaponAnim(ANIM_SLASH_SKILL);
#endif
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.67s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.3s;
		m_tNextThink = gpGlobals->time + 1.0s;
		m_iThinkType = 6;
		m_iPrimaryAttackRecord = 1;
		m_iSecondaryAttackRecord = 1;

		EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "weapons/whipsword_slash_skill.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
#ifndef CLIENT_DLL
		PLAYBACK_EVENT_FULL(FEV_GLOBAL, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, 0.0, 0.0, 1, 0, FALSE, FALSE);

		UTIL_MakeVectors(m_pPlayer->pev->v_angle);
		Vector vecSrc = m_pPlayer->GetGunPosition();
		Vector vecEnd = vecSrc + gpGlobals->v_forward * 200;

		TraceResult tr;
		UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr);

		if (tr.flFraction >= 1.0)
		{
			UTIL_TraceHull(vecSrc, vecEnd, dont_ignore_monsters, head_hull, ENT(m_pPlayer->pev), &tr);

			if (tr.flFraction >= 1.0)
			{
				return;
			}

			CBaseEntity* pHit = CBaseEntity::Instance(tr.pHit);

			if (!pHit || pHit->IsBSPModel())
			{
				FindHullIntersection(vecSrc, tr, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX, ENT(m_pPlayer->pev));
			}

			vecEnd = tr.vecEndPos;
			//HITNONE
		}

		if (tr.flFraction < 1.0)
		{
			CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);

			if (!pEntity || pEntity->IsBSPModel())
			{
				ClearMultiDamage();
				pEntity->TraceAttack(m_pPlayer->pev, GetSlashComboAttackDamage(), gpGlobals->v_forward, &tr, DMG_BULLET | DMG_NEVERGIB);
				ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/dgaxe_wall_stone1.wav", VOL_NORM, ATTN_NORM, 0, 94);
			}
			//WALL
		}

		CBaseEntity* pEntity = nullptr;
		while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, 200)) != nullptr) {
			if (pEntity->pev->takedamage != DAMAGE_NO) {
				if (pEntity->IsBSPModel())
					continue;

				if (pEntity->pev == m_pPlayer->pev)
					continue;

				Vector vecSpot = pEntity->BodyTarget(vecSrc);
				vecSpot.z = vecEnd.z;
				UTIL_TraceLine(vecSrc, vecSpot, missile, ENT(m_pPlayer->pev), &tr);

				if (AngleBetweenVectors(tr.vecEndPos - vecSrc, gpGlobals->v_forward) >= 360)
					continue;

				if (tr.flFraction == 1.0f || tr.pHit == pEntity->edict()) {
					if (tr.fStartSolid) {
						tr.vecEndPos = vecSrc;
						tr.flFraction = 0;
					}

					if (tr.flFraction == 1.0f) {
						pEntity->TakeDamage(m_pPlayer->pev, m_pPlayer->pev, GetSlashComboAttackDamage(), DMG_BULLET | DMG_NEVERGIB);
					}

					Vector vecRealDir = (tr.vecEndPos - vecSrc).Normalize();

					ClearMultiDamage();
					pEntity->TraceAttack(m_pPlayer->pev, GetSlashComboAttackDamage(), vecRealDir, &tr, DMG_BULLET | DMG_NEVERGIB);
					ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);

					CBasePlayer* pPlayer = (CBasePlayer*)pEntity;
					if (pPlayer->IsPlayer() && pPlayer->m_bIsZombie && !(pPlayer->m_iBuff & BUFF_IGNORE_KNOCKBACK))
					{
						pPlayer->m_flVelocityModifier = 1.0;
						Vector2D vecDir2D = (pPlayer->pev->origin.Make2D() - m_pPlayer->pev->origin.Make2D()).Normalize();

						pPlayer->pev->velocity.x = vecDir2D.x * 600;
						pPlayer->pev->velocity.y = vecDir2D.y * 600;
						pPlayer->pev->velocity.z = 250;

						auto strategy = dynamic_cast<CPlayerModStrategy_ZB1*>(pPlayer->m_pModStrategy.get());
						if (strategy)
						{
							strategy->SetStunSpeedTime(1.3s, 80.0);
							strategy->SetStunGravityTime(1.3s);
						}
					}

					if (pEntity->Classify() == CLASS_PLAYER)
						EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/dgaxe_slash_hit1.wav", VOL_NORM, ATTN_NORM, 0, 94);
					else
						EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/dgaxe_wall_stone1.wav", VOL_NORM, ATTN_NORM, 0, 94);
				}
			}
		}
#endif
		return;

	}

	if (m_iSecondaryAttackRecord == 1)
		m_iSecondaryAttackRecord = 2;
	else if (m_iSecondaryAttackRecord == 2)
		m_iSecondaryAttackRecord = 3;
	else if (m_iSecondaryAttackRecord == 3)
		m_iSecondaryAttackRecord = 4;
	else if (m_iSecondaryAttackRecord == 4)
		m_iSecondaryAttackRecord = 5;
	else if (m_iSecondaryAttackRecord == 5)
	{
		m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_whipsword_1.mdl");

		SendWeaponAnim(ANIM_STAB_LOOP);
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.67s;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 5.0s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 9999.0s;
		m_tNextThink = gpGlobals->time + 5.2s;
		m_tNextAttack = gpGlobals->time + 0.1s;
		m_iThinkType = 4;
		m_iPrimaryAttackRecord = 1;
		m_iSecondaryAttackRecord = 0;

		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_VOICE, "weapons/whipsword_stab_loop_all.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
#ifndef CLIENT_DLL
		PLAYBACK_EVENT_FULL(FEV_GLOBAL, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, 0.0, 0.0, 4, m_iStabRecord, FALSE, FALSE);
#endif
		return;
	}
	
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
	PLAYBACK_EVENT_FULL(FEV_GLOBAL, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, 0.0, 0.0, 3, m_iStabRecord, FALSE, FALSE);
#endif
	m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_whipsword.mdl");

	if (!m_iStabRecord)
	{
		SendWeaponAnim(ANIM_STAB1);
		m_iStabRecord = 1;
	}
	else
	{
		SendWeaponAnim(ANIM_STAB2);
		m_iStabRecord = 0;
	}

	switch (RANDOM_LONG(0, 3))
	{
	case 1:
		EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "weapons/whipsword_stab1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM); break;
	case 2:
		EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "weapons/whipsword_stab2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM); break;
	case 3:
		EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "weapons/whipsword_stab3.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM); break;
	}

	pev->iuser2 = 0;
	m_iPrimaryAttackRecord = 1;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.4s;
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.67s;
	m_iThinkType = 2;
	m_tNextThink = gpGlobals->time + 0.6s;
	m_tNextAttack = gpGlobals->time + 0.1s;

}

#ifndef CLIENT_DLL
bool CKnifeWhipSword::AngularCheck(CBaseEntity* pEntity, CBaseEntity* pAttacker, Vector vecSrc, Vector vecDir, Vector2D vecAngleCheckDir, float flRange, float flMinimumCosine, TraceResult* ptr, bool IsPrimaryAttack)
{
	bool bResult = false;
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

	return bResult;
}

hit_result_t CKnifeWhipSword::KnifeAttack(float flDamage, float flRadius, float flAngleDegrees, int bitsDamageType, entvars_t* pevInflictor, CBasePlayer* pAttackPlayer, bool IsPrimaryAttack)
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

		if (!AngularCheck(pEntity, pAttackPlayer, vecSrc, vecForward, vecForward2D, flRadius, c, &tr, IsPrimaryAttack))
			continue;

		float flRatio = 1.0;

		if (pEntity && pEntity->IsPlayer() && IsPrimaryAttack)
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

			if (player->m_bIsZombie && !(player->m_iBuff & BUFF_IGNORE_KNOCKBACK))
			{	
				auto strategy = dynamic_cast<CPlayerModStrategy_ZB1*>(player->m_pModStrategy.get());
				if (strategy)
				{
					strategy->SetStunSpeedTime(1.3s);
					strategy->SetStunGravityTime(1.3s);
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
}
