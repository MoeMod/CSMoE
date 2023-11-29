/*
knife_y22s2sfsword.cpp
Copyright (C) 2022

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
#include <dlls/gamemode/mod_zb1.h>
#endif

#include "weapons/WeaponTemplate.hpp"
#include "weapons/KnifeAttack.h"


#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

	//Y22s2sfsword   y22s2sfsword
	class CKnifeY22s2SFSword : public LinkWeaponTemplate<CKnifeY22s2SFSword,
		TGeneralData,
		BuildTGetItemInfoFromCSW<WEAPON_KNIFE>::template type
	>
	{
	public:
		static constexpr const char* ClassName = "knife_y22s2sfsword";
		static constexpr const char* V_Model = "models/v_y22s2sfsword.mdl";
		static constexpr const char* P_Model = "models/p_y22s2sfsword.mdl";
		static constexpr const char* W_Model = "models/w_knife.mdl";

		static constexpr int MaxClip = 100;
		static constexpr auto ItemSlot = KNIFE_SLOT;
		static constexpr const char* AnimExtension = "sfsword";
		KnockbackData KnockBack = { 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };

	public:

		enum y22s2sfsword_e
		{
			ANIM_IDLE,
			ANIM_ON,
			ANIM_OFF,
			ANIM_DRAW,
			ANIM_STAB,
			ANIM_STAB_MISS,
			ANIM_SLASH1,
			ANIM_SLASH2,
			ANIM_SLASH3,
			ANIM_OFF_IDLE,
			ANIM_OFF_SLASH1,
			ANIM_SKILL,
		};

		void Precache() override
		{
			PRECACHE_MODEL("models/v_y22s2sfsword.mdl");
			PRECACHE_MODEL("models/p_y22s2sfsword.mdl");

			PRECACHE_MODEL("sprites/muzzleflash241.spr");
			PRECACHE_MODEL("sprites/muzzleflash242.spr");
			PRECACHE_MODEL("sprites/muzzleflash243.spr");
			PRECACHE_MODEL("sprites/muzzleflash244.spr");
			PRECACHE_MODEL("sprites/muzzleflash245.spr");
			PRECACHE_MODEL("sprites/muzzleflash246.spr");
			PRECACHE_MODEL("sprites/muzzleflash249.spr");
			m_iEfHit = PRECACHE_MODEL("sprites/ef_sfsword_hit.spr");


			PRECACHE_SOUND("weapons/y22s2sfsword_midslash1.wav");
			PRECACHE_SOUND("weapons/y22s2sfsword_midslash2.wav");
			PRECACHE_SOUND("weapons/y22s2sfsword_midslash3.wav");
			PRECACHE_SOUND("weapons/y22s2sfsword_skill.wav");
			PRECACHE_SOUND("weapons/y22s2sfsword_stab.wav");
		}

		BOOL Deploy(void) override
		{
			m_fMaxSpeed = 260;
			m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;

			m_pPlayer->m_bShieldDrawn = false;

			m_flNextTimeCanHolster = invalid_time_point;
#ifndef CLIENT_DLL
			if (m_flTimeholster != invalid_time_point)
			{
				m_iClip += (gpGlobals->time - m_flTimeholster) / 0.1s;
			}
			if (m_iClip >= 100)
				m_iClip = 100;


			MESSAGE_BEGIN(MSG_ONE, gmsgSpecialCrossHair, NULL, m_pPlayer->pev);
			WRITE_BYTE(4);
			WRITE_BYTE(m_CrossHairState);
			MESSAGE_END();
	

#endif
			int iAnim = ANIM_DRAW;
			const char* szAnim = "sfsword";
			duration_t delay = 0.17s;

			return DefaultDeploy(V_Model, P_Model, iAnim, szAnim, UseDecrement() != FALSE, delay);
		}


		int GetItemInfo(ItemInfo* p) override;
		void PrimaryAttack() override;
		void SecondaryAttack() override;
		void Spawn() override;
		BOOL IsSignal();
		void WeaponIdle() override;
		float GetMaxSpeed() override;
		void Holster(int skiplocal) override;
		void DelayPrimaryAttack();
#ifndef CLIENT_DLL
		hit_result_t KnifeAttack(float flDamage, float flRadius, float flAngleDegrees, int bitsDamageType, entvars_t* pevInflictor, CBasePlayer* pAttackPlayer, bool IsPrimaryAttack);

# endif
		BOOL CanDrop() override { return false; }
		void ItemPostFrame() override;


	public:
		float GetPrimaryAttackDamage1() const
		{
			float flDamage = 28;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
			{
				flDamage = 1190.0f;
			}
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
			{
				flDamage = 3547.0f;
			}
#endif
			return flDamage;
		}
		float GetPrimaryAttackDamage2() const
		{
			float flDamage = 56;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
			{
				flDamage = 2380.0f;
			}
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
			{
				flDamage = 7094.0f;
			}
#endif
			return flDamage;
		}


		float GetSecondaryAttackDamage1() const
		{
			float flDamage = 36;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 1500.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 3000.0f;
#endif
			return flDamage;
		}
		float GetSecondaryAttackDamage2() const
		{
			float flDamage = 100;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 3676.5f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 10958.0f;
#endif
			return flDamage;
		}
		


#ifndef CLIENT_DLL
		time_point_t m_flTimeNextCharge;
		time_point_t m_flTimeholster;
#endif
		int m_usFireY22s2sfsword;
		time_point_t m_tTimeSoundEmit;
		time_point_t m_flTimeBuff;
		int m_iDamageType;
		int m_iState;	//pev_iuser1
		int m_iEfHit;

		time_point_t m_tNextThink;
		enum y22s2sfsword_crosshairstate_e
		{
			Y22S2SFSWORD_AIM_OFF = 3,
			Y22S2SFSWORD_AIM_ON = 15,
		};
		private:
		time_point_t m_flNextTimeCanHolster; // flNextTimeCanHolster
		time_point_t m_flTimeNextMessage;
		y22s2sfsword_crosshairstate_e m_CrossHairState;
		y22s2sfsword_crosshairstate_e m_CrossHairStateStored;
	};

	LINK_ENTITY_TO_CLASS(knife_y22s2sfsword, CKnifeY22s2SFSword)

		void CKnifeY22s2SFSword::Spawn()
	{
		m_iKnifeID = KNIFE_Y22S2SFSWORD;
		pev->classname = MAKE_STRING("knife_y22s2sfsword");
		m_iId = WEAPON_KNIFE;
		Precache();
		SET_MODEL(ENT(pev), "models/w_knife.mdl");

#ifndef CLIENT_DLL
		m_flTimeholster = gpGlobals->time;
#endif
		m_iClip = WEAPON_NOCLIP;
		m_iDefaultAmmo = 0;
		m_iState = 0;

		m_CrossHairState = m_CrossHairStateStored = Y22S2SFSWORD_AIM_OFF;
		m_flTimeNextMessage = invalid_time_point;

		FallInit();
	}

	int CKnifeY22s2SFSword::GetItemInfo(ItemInfo* p)
	{
		p->pszName = STRING(pev->classname);
		p->pszAmmo1 = "Y22s2sfswordcharge";
		p->iMaxAmmo1 = -1;
		p->pszAmmo2 = NULL;
		p->iMaxAmmo2 = -1;
		p->iMaxClip = 100;
		p->iSlot = 2;
		p->iPosition = 1;
		p->iId = WEAPON_KNIFE;
		p->iFlags = 0;
		p->iWeight = KNIFE_WEIGHT;

		return 1;
	}

	BOOL CKnifeY22s2SFSword::IsSignal()
	{
#ifndef CLIENT_DLL
		if (m_iClip == 100)
		{
			return TRUE;
		}
		else
		{
			m_CrossHairState = Y22S2SFSWORD_AIM_OFF;
			return FALSE;
		}
#endif
		return TRUE;
	}

#ifndef CLIENT_DLL
	hit_result_t CKnifeY22s2SFSword::KnifeAttack(float flDamage, float flRadius, float flAngleDegrees, int bitsDamageType, entvars_t* pevInflictor, CBasePlayer* pAttackPlayer, bool IsPrimaryAttack)
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

			MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
			WRITE_BYTE(TE_TEMPSPRITE);
			WRITE_COORD(pEntity->pev->origin.x);
			WRITE_COORD(pEntity->pev->origin.y);
			WRITE_COORD(pEntity->pev->origin.z);
			WRITE_SHORT(m_iEfHit);
			WRITE_BYTE(5);
			WRITE_BYTE(255);
			WRITE_BYTE(30);
			MESSAGE_END();

			if (pEntity->IsPlayer())
			{
				CBasePlayer* player = (CBasePlayer*)pEntity;

				if (player->m_bIsZombie && !(player->m_iBuff & BUFF_IGNORE_KNOCKBACK))
				{
					Vector vecDir = (player->pev->origin - m_pPlayer->pev->origin);
					vecDir.z = 0;
					if (IsPrimaryAttack)
					{
						auto strategy = dynamic_cast<CPlayerModStrategy_ZB1*>(player->m_pModStrategy.get());
						if (strategy)
						{
							strategy->SetStunSpeedTime(1.0s);
							strategy->SetStunGravityTime(1.0s);
						}
						ApplyKnockbackData(player, vecDir, { -1, -1, -1, -1, 1.0f });
					}
					else
					{
						if(m_iClip)
							ApplyKnockbackData(player, vecDir, { 1200, 1200, 1200, 1200, 1.0f });

						auto strategy = dynamic_cast<CPlayerModStrategy_ZB1*>(player->m_pModStrategy.get());
						if (strategy)
						{
							strategy->SetStunSpeedTime(3.0s);
							strategy->SetStunGravityTime(3.0s);
						}
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

	void CKnifeY22s2SFSword::DelayPrimaryAttack(void)
	{
#ifndef CLIENT_DLL		
		UTIL_MakeVectors(m_pPlayer->pev->v_angle);
		Vector vecSrc = m_pPlayer->GetGunPosition();

		float flDamage = GetPrimaryAttackDamage1();
		float flDistance = 140;
		float flAngle = 90.0;

		if (m_iState < 2)
		{
			flDamage = GetPrimaryAttackDamage1();
		}
		else
		{
			flDamage = GetPrimaryAttackDamage2();
		}

		switch (KnifeAttack(flDamage, flDistance, flAngle, DMG_NEVERGIB | DMG_BULLET, this->pev, m_pPlayer, true))
		{
		case HIT_PLAYER:
		{
			if (RANDOM_LONG(0, 1))
			{
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/sfsword_hit1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			}
			else
			{
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/sfsword_hit2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			}

			break;
		}
		case HIT_WALL:
		{
			if (RANDOM_LONG(0, 1))
			{
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/sfsword_wall1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			}
			else
			{
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/sfsword_wall2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			}
			break;
		}
		}
#endif

		SetThink(nullptr);
	}

	void CKnifeY22s2SFSword::PrimaryAttack(void)
	{

		switch (m_iState)
		{
		case 0:
		{
			//PLAYBACK_EVENT_FULL(FEV_HOSTONLY, m_pPlayer->edict(), m_usFireY22s2sfsword, 0.266, g_vecZero, g_vecZero, 0.0, 0.0, ANIM_SLASH1, 0, FALSE, FALSE);
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/y22s2sfsword_midslash1.wav", VOL_NORM, ATTN_NORM, 0, 94);

			m_iState = 1;
			SendWeaponAnim(ANIM_SLASH1, UseDecrement() != FALSE);
			SetThink(&CKnifeY22s2SFSword::DelayPrimaryAttack);
			pev->nextthink = gpGlobals->time + 0.1s;

			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.7s;
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5s;

#ifndef CLIENT_DLL
			m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
			break;
		}

		case 1:
		{
			m_iState = 2;

			PLAYBACK_EVENT_FULL(FEV_HOSTONLY, m_pPlayer->edict(), m_usFireY22s2sfsword, 0.266, g_vecZero, g_vecZero, 0.0, 0.0, ANIM_STAB, 0, FALSE, FALSE);
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/y22s2sfsword_stab.wav", VOL_NORM, ATTN_NORM, 0, 94);

			SendWeaponAnim(ANIM_STAB, UseDecrement() != FALSE);

			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.7s;
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.625s;
			pev->nextthink = gpGlobals->time + 0.1s;

#ifndef CLIENT_DLL	
			float flDamage = GetSecondaryAttackDamage1();

			m_pPlayer->SetAnimation(PLAYER_ATTACK2);
			switch (KnifeAttack(flDamage, 220, 45, DMG_NEVERGIB | DMG_BULLET, this->pev, m_pPlayer, false))
			{
			case HIT_PLAYER:
			{
				if (RANDOM_LONG(0, 1))
				{
					EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/sfsword_hit1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
				}
				else
				{
					EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/sfsword_hit2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
				}

				break;
			}
			case HIT_WALL:
			{
				if (RANDOM_LONG(0, 1))
				{
					EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/sfsword_wall1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
				}
				else
				{
					EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/sfsword_wall2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
				}
				break;
			}

			}
#endif
			
			break;
		}

		case 2:
		{
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/y22s2sfsword_midslash2.wav", VOL_NORM, ATTN_NORM, 0, 94);
			//PLAYBACK_EVENT_FULL(FEV_HOSTONLY, m_pPlayer->edict(), m_usFireY22s2sfsword, 0.266, g_vecZero, g_vecZero, 0.0, 0.0, ANIM_SLASH2, 0, FALSE, FALSE);


			m_iState = 3;
			SendWeaponAnim(ANIM_SLASH2, UseDecrement() != FALSE);

			SetThink(&CKnifeY22s2SFSword::DelayPrimaryAttack);
			pev->nextthink = gpGlobals->time + 0.1s;

			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.7s;
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5s;


#ifndef CLIENT_DLL	
			m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
			break;
		}

		case 3:
		{

			m_iState = 4;

			PLAYBACK_EVENT_FULL(FEV_HOSTONLY, m_pPlayer->edict(), m_usFireY22s2sfsword, 0.266, g_vecZero, g_vecZero, 0.0, 0.0, ANIM_STAB, 0, FALSE, FALSE);
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/y22s2sfsword_stab.wav", VOL_NORM, ATTN_NORM, 0, 94);

			SendWeaponAnim(ANIM_STAB, UseDecrement() != FALSE);

			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.7s;
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.625s;
			pev->nextthink = gpGlobals->time + 0.1s;

#ifndef CLIENT_DLL	
			float flDamage = GetSecondaryAttackDamage1();

			m_pPlayer->SetAnimation(PLAYER_ATTACK2);
			switch (KnifeAttack(flDamage, 220, 45, DMG_NEVERGIB | DMG_BULLET, this->pev, m_pPlayer, false))
			{
			case HIT_PLAYER:
			{
				if (RANDOM_LONG(0, 1))
				{
					EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/sfsword_hit1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
				}
				else
				{
					EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/sfsword_hit2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
				}

				break;
			}
			case HIT_WALL:
			{
				if (RANDOM_LONG(0, 1))
				{
					EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/sfsword_wall1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
				}
				else
				{
					EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/sfsword_wall2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
				}
				break;
			}

			}
#endif
			break;
		}


		case 4:
		{

			//PLAYBACK_EVENT_FULL(FEV_HOSTONLY, m_pPlayer->edict(), m_usFireY22s2sfsword, 0.266, g_vecZero, g_vecZero, 0.0, 0.0, ANIM_SLASH3, 0, FALSE, FALSE);

			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/y22s2sfsword_midslash3.wav", VOL_NORM, ATTN_NORM, 0, 94);
			SendWeaponAnim(ANIM_SLASH3, UseDecrement() != FALSE);

#ifndef CLIENT_DLL	
			m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

			m_iState = 5;
			SetThink(&CKnifeY22s2SFSword::DelayPrimaryAttack);
			pev->nextthink = gpGlobals->time + 0.1s;

			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.7s;
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5s;

			break;
		}
		case 5:
		{

			m_iState = 0;

			PLAYBACK_EVENT_FULL(FEV_HOSTONLY, m_pPlayer->edict(), m_usFireY22s2sfsword, 0.266, g_vecZero, g_vecZero, 0.0, 0.0, ANIM_STAB, 0, FALSE, FALSE);
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/y22s2sfsword_stab.wav", VOL_NORM, ATTN_NORM, 0, 94);

			SendWeaponAnim(ANIM_STAB, UseDecrement() != FALSE);

			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.7s;
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.625s;
			pev->nextthink = gpGlobals->time + 0.1s;

#ifndef CLIENT_DLL	
			float flDamage = GetSecondaryAttackDamage1();

			m_pPlayer->SetAnimation(PLAYER_ATTACK2);
			switch (KnifeAttack(flDamage, 220, 45, DMG_NEVERGIB | DMG_BULLET, this->pev, m_pPlayer, false))
			{
			case HIT_PLAYER:
			{
				if (RANDOM_LONG(0, 1))
				{
					EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/sfsword_hit1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
				}
				else
				{
					EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/sfsword_hit2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
				}

				break;
			}
			case HIT_WALL:
			{
				if (RANDOM_LONG(0, 1))
				{
					EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/sfsword_wall1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
				}
				else
				{
					EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/sfsword_wall2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
				}
				break;
			}

			}
#endif
			break;
		}
		}
	}

	void CKnifeY22s2SFSword::ItemPostFrame()
	{
#ifndef CLIENT_DLL
		if (m_iClip < 100)
		{
			if (m_flTimeNextCharge <= gpGlobals->time)
			{
				m_iClip++;
				m_flTimeNextCharge = gpGlobals->time + 0.1s;
			}
		}
		else if (m_iClip = 100)
		{
			if (IsSignal())
			{
			m_CrossHairState = Y22S2SFSWORD_AIM_ON;
			if (m_CrossHairState != m_CrossHairStateStored)
			{
				MESSAGE_BEGIN(MSG_ONE, gmsgSpecialCrossHair, NULL, m_pPlayer->pev);
				WRITE_BYTE(4);
				WRITE_BYTE(m_CrossHairState);
				MESSAGE_END();

				m_CrossHairStateStored = m_CrossHairState;
			}
		}
		if (m_flTimeNextMessage < gpGlobals->time)
		{
			if (m_CrossHairState != m_CrossHairStateStored)
			{
#ifndef CLIENT_DLL
				MESSAGE_BEGIN(MSG_ONE_UNRELIABLE, gmsgSpecialCrossHair, NULL, m_pPlayer->pev);
				WRITE_BYTE(4);
				WRITE_BYTE(m_CrossHairState);
				MESSAGE_END();
#endif
			}

			m_flTimeNextMessage = gpGlobals->time + 0.1s;
			m_CrossHairStateStored = m_CrossHairState;
		}
	}
#endif
		return CBasePlayerWeapon::ItemPostFrame();
	}

	void CKnifeY22s2SFSword::SecondaryAttack(void)
	{
		if (m_iClip == 100)
		{
			PLAYBACK_EVENT_FULL(FEV_HOSTONLY, m_pPlayer->edict(), m_usFireY22s2sfsword, 0.266, g_vecZero, g_vecZero, 0.0, 0.0, ANIM_SKILL, 0, FALSE, FALSE);
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/y22s2sfsword_skill.wav", VOL_NORM, ATTN_NORM, 0, 94);

			SendWeaponAnim(ANIM_SKILL, UseDecrement() != FALSE);

			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.2s;
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.2s;
			pev->nextthink = gpGlobals->time + 0.1s;

#ifndef CLIENT_DLL	
			float flDamage = GetSecondaryAttackDamage2();

			m_pPlayer->SetAnimation(PLAYER_ATTACK2);

			switch (KnifeAttack(flDamage, 220, 45, DMG_NEVERGIB | DMG_BULLET, this->pev, m_pPlayer, false))
			{
			case HIT_PLAYER:
			case HIT_WALL:
			{
				break;
			}
			}
			if (IsSignal())
			{

				m_CrossHairState = Y22S2SFSWORD_AIM_OFF;
				if (m_CrossHairState != m_CrossHairStateStored)
				{
					MESSAGE_BEGIN(MSG_ONE, gmsgSpecialCrossHair, NULL, m_pPlayer->pev);
					WRITE_BYTE(4);
					WRITE_BYTE(m_CrossHairState);
					MESSAGE_END();

					m_CrossHairStateStored = m_CrossHairState;
				}
		}
#endif

			m_iState = 0;
			m_iClip = 0;


		}
		/*else
		{
			PLAYBACK_EVENT_FULL(FEV_HOSTONLY, m_pPlayer->edict(), m_usFireY22s2sfsword, 0.266, g_vecZero, g_vecZero, 0.0, 0.0, ANIM_STAB, 0, FALSE, FALSE);
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/y22s2sfsword_stab.wav", VOL_NORM, ATTN_NORM, 0, 94);

			SendWeaponAnim(ANIM_STAB, UseDecrement() != FALSE);

			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.7s;
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.5s;
			pev->nextthink = gpGlobals->time + 0.1s;

#ifndef CLIENT_DLL	
			float flDamage = GetSecondaryAttackDamage1();

			m_pPlayer->SetAnimation(PLAYER_ATTACK2);
			switch (KnifeAttack(flDamage, 220, 45, DMG_NEVERGIB | DMG_BULLET, this->pev, m_pPlayer, false))
			{
			case HIT_PLAYER:
			{
				if (RANDOM_LONG(0, 1))
				{
					EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/sfsword_hit1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
				}
				else
				{
					EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/sfsword_hit2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
				}

				break;
			}
			case HIT_WALL:
			{
				if (RANDOM_LONG(0, 1))
				{
					EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/sfsword_wall1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
				}
				else
				{
					EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/sfsword_wall2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
				}
				break;
			}

			}
#endif

			m_iState = 0;

		}
		*/
	}



	


	float CKnifeY22s2SFSword::GetMaxSpeed(void)
	{
		return 260;
	}

	void CKnifeY22s2SFSword::Holster(int skiplocal)
	{
		SetThink(nullptr);
		m_iState = 0;

#ifndef CLIENT_DLL
		m_flTimeholster = gpGlobals->time;

		MESSAGE_BEGIN(MSG_ONE, gmsgSpecialCrossHair, NULL, m_pPlayer->pev);
		WRITE_BYTE(4);
		WRITE_BYTE(3);
		MESSAGE_END();

#endif
		return CBasePlayerWeapon::Holster(skiplocal);
	}

	void CKnifeY22s2SFSword::WeaponIdle(void)
	{
		ResetEmptySound();
		m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;

		SendWeaponAnim(ANIM_IDLE, UseDecrement() != FALSE);

	}

}