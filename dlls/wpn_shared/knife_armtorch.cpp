/*
knife_armtorch.cpp
Copyright (C) 2021 Original Author ���դξ��� Modify by JustANoobQAQ , Entity Think by Rentro


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

	class CKnifeArmTorch : public LinkWeaponTemplate<CKnifeArmTorch,
		TGeneralData,
		BuildTGetItemInfoFromCSW<WEAPON_KNIFE>::template type
	>
	{
	public:
		static constexpr const char* ClassName = "knife_armtorch";
		static constexpr const char* V_Model = "models/v_armtorch.mdl";
		static constexpr const char* P_Model = "models/p_armtorch.mdl";
		static constexpr const char* W_Model = "models/w_knife.mdl";

		static constexpr int MaxClip = -1;
		static constexpr auto ItemSlot = KNIFE_SLOT;
		static constexpr const char* AnimExtension = "dragontail";
		KnockbackData KnockBack = { 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };

	public:

		enum armtorch_e
		{
			ANIM_IDLE,
			ANIM_SLASH1,
			ANIM_SLASH1_1,
			ANIM_SLASH2,
			ANIM_SLASH3,
			ANIM_SLASH3_1,
			ANIM_SLASH3_2,
			ANIM_STAB,
			ANIM_DRAW,
		};

		void Precache() override
		{
			PRECACHE_MODEL("models/v_armtorch.mdl");
			PRECACHE_MODEL("models/p_armtorch.mdl");

			PRECACHE_MODEL("sprites/muzzleflash68.spr");
			PRECACHE_MODEL("sprites/muzzleflash69.spr");
			PRECACHE_MODEL("sprites/muzzleflash70.spr");
			PRECACHE_MODEL("sprites/muzzleflash71.spr");

			
			PRECACHE_SOUND("weapons/armtorch_fire1.wav");
			PRECACHE_SOUND("weapons/armtorch_fire2.wav");
	
			PRECACHE_SOUND("weapons/armtorch_idle.wav");
		}
		
	
		BOOL Deploy(void) override
		{
			m_fMaxSpeed = 260;		
			m_iCheck = 0;
			m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;
			m_flDashTime = invalid_time_point;
			m_pPlayer->m_bShieldDrawn = false;

			return DefaultDeploy(V_Model, P_Model, ANIM_DRAW, "dragontail", UseDecrement() != FALSE);
		}

		int GetItemInfo(ItemInfo* p) override;
		void PrimaryAttack() override;	
		void SecondaryAttack() override;
		void Spawn() override;
		void WeaponIdle() override;
		void Holster(int skiplocal) override;
		void DelayPrimaryAttack();
		void DelaySecondaryAttack();
		void DelayDashAttack();
		void DelaySpecialAttack();
		float GetMaxSpeed(void) override { return 260.0; }

#ifndef CLIENT_DLL
		hit_result_t KnifeAttack(float flDamage, float flRadius, float flAngleDegrees, int bitsDamageType, entvars_t* pevInflictor, CBasePlayer* pAttackPlayer, int iType);
		void MaterialSound();
# endif
		BOOL CanDrop() override { return false; }

		KnockbackData GetKnockBackData() override
		{
			if (pev->iuser1 == 2)
				return { 1300.0f, 1200.0f, 1000.0f, 800.0f, 1.0f };
			else if (pev->iuser1 == 1)
				return { 650.0f, 550.0f, 600.0f, 500.0f, 0.7f };
			else
				return { 450.0f, 350.0f, 400.0f, 300.0f, 0.7f };
		}

	public:
		float GetPrimaryAttackDamage() const
		{
			float flDamage = 85;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 1530.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 1105.0f;
#endif
			return flDamage;
		}

		float GetSecondaryAttackDamage() const
		{
			float flDamage = 110.0f;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 1980.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 1430.0f;
#endif
			return flDamage;
		}

		float GetDashAttackDamage() const
		{
			float flDamage = 150.0f;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 2700.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 1950.0f;
#endif
			return flDamage;
		}

		float GetSpecialAttackDamage() const
		{
			float flDamage = 200.0f;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 3600.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 2600.0f;
#endif
			return flDamage;
		}


		float GetBuffDamage() const
		{
			float flDamage = 2.0;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 100.0;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 100.0;
#endif
			return flDamage;
		}
	

		time_point_t m_flDashTime;
		int m_iCheck;	//g_iCheck
	};

	LINK_ENTITY_TO_CLASS(knife_armtorch, CKnifeArmTorch)

	void CKnifeArmTorch::Spawn()
	{
		pev->classname = MAKE_STRING("knife_armtorch");
		m_iId = WEAPON_KNIFE;
		Precache();
		SET_MODEL(ENT(pev), "models/w_knife.mdl");

		m_iClip = WEAPON_NOCLIP;
		m_flDashTime = invalid_time_point;
		m_iCheck = 0;
		FallInit();
	}

	int CKnifeArmTorch::GetItemInfo(ItemInfo* p)
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
	void CKnifeArmTorch::MaterialSound()
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
							EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_AUTO, "weapons/armtorch_wall_metal1.wav", VOL_NORM, ATTN_NORM, 0, 94);
						else
							EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_AUTO, "weapons/armtorch_wall_metal2.wav", VOL_NORM, ATTN_NORM, 0, 94);					
						break;
					}
				case CHAR_TEX_WOOD:
				case CHAR_TEX_SLOSH:
					{
						if (RANDOM_LONG(0, 1))
							EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_AUTO, "weapons/armtorch_wall_stone1.wav", VOL_NORM, ATTN_NORM, 0, 94);
						else
							EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_AUTO, "weapons/armtorch_wall_stone1.wav", VOL_NORM, ATTN_NORM, 0, 94);
						break;
					}
				default:
				{
					if (RANDOM_LONG(0, 1))
						EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_AUTO, "weapons/armtorch_wall_metal1.wav", VOL_NORM, ATTN_NORM, 0, 94);
					else
						EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_AUTO, "weapons/armtorch_wall_metal2.wav", VOL_NORM, ATTN_NORM, 0, 94);
					break;
				}
			}
		}
	}

	hit_result_t CKnifeArmTorch::KnifeAttack(float flDamage, float flRadius, float flAngleDegrees, int bitsDamageType, entvars_t* pevInflictor, CBasePlayer* pAttackPlayer, int iType)
	{
		if (iType == 2)
			pev->iuser1 = 2;
		else if (iType == 1)
			pev->iuser1 = 1;
		else
			pev->iuser1 = 0;

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

			if ((pEntity->IsPlayer() || pEntity->Classify() == CLASS_PLAYER_ALLY) && (g_pGameRules->PlayerRelationship(m_pPlayer, pEntity) != GR_TEAMMATE))
			{
				pEntity->m_pBuffAttacker = m_pPlayer->pev;
				pEntity->m_flHealthDecreaseAmount = GetBuffDamage();
				pEntity->m_iHealthDecreaseCount = 2;
				pEntity->m_flHealthDecreaseInterval = 1.0s;

				MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, pEntity->pev);
				WRITE_BYTE(4);
				WRITE_SHORT(pEntity->entindex());
				WRITE_BYTE(18);
				MESSAGE_END();

				if (pEntity->IsPlayer())
				{
					CBasePlayer* pPlayer = (CBasePlayer*)pEntity;

					if (pPlayer->m_bIsZombie && !(pPlayer->m_iBuff & BUFF_IGNORE_KNOCKBACK))
					{
						pEntity->pev->velocity.z += 200.0;
					}
				}
				else
				{
					pEntity->pev->velocity.z += 200.0;
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

	void CKnifeArmTorch::DelayPrimaryAttack(void)
	{
#ifndef CLIENT_DLL		
		UTIL_MakeVectors(m_pPlayer->pev->v_angle);
		Vector vecSrc = m_pPlayer->GetGunPosition();
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
		switch (KnifeAttack(GetPrimaryAttackDamage(), 100, 60, DMG_NEVERGIB | DMG_BULLET, m_pPlayer->pev, m_pPlayer, 1))
		{
		case HIT_PLAYER:
		{
			switch (RANDOM_LONG(0, 1))
			{
			case 0: { UTIL_EMIT_SOUND(m_pPlayer->edict(), CHAN_ITEM, "weapons/armtorch_hit1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM); break; }
			case 1: { UTIL_EMIT_SOUND(m_pPlayer->edict(), CHAN_ITEM, "weapons/armtorch_hit2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM); break; }
			}
			break;
		}
		case HIT_WALL:
		{
			UTIL_EMIT_SOUND(m_pPlayer->edict(), CHAN_ITEM, "weapons/armtorch_wall.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			//MaterialSound();
			break;
		}
		}
#endif
		SetThink(nullptr);
	}

	void CKnifeArmTorch::DelaySecondaryAttack(void)
	{
#ifndef CLIENT_DLL		
		UTIL_MakeVectors(m_pPlayer->pev->v_angle);
		Vector vecSrc = m_pPlayer->GetGunPosition();
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
		switch (KnifeAttack(GetSecondaryAttackDamage(), 125, 60, DMG_NEVERGIB | DMG_BULLET, m_pPlayer->pev, m_pPlayer, 0))
		{
		case HIT_PLAYER:
		{
			switch (RANDOM_LONG(0, 1))
			{
			case 0: { UTIL_EMIT_SOUND(m_pPlayer->edict(), CHAN_ITEM, "weapons/armtorch_hit1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM); break; }
			case 1: { UTIL_EMIT_SOUND(m_pPlayer->edict(), CHAN_ITEM, "weapons/armtorch_hit2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM); break; }
			}
			break;
		}
		case HIT_WALL:
		{
			UTIL_EMIT_SOUND(m_pPlayer->edict(), CHAN_ITEM, "weapons/armtorch_wall.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			//MaterialSound();
			break;
		}
		}
#endif

		SetThink(nullptr);
	}

	void CKnifeArmTorch::DelayDashAttack()
	{
#ifndef CLIENT_DLL		
		UTIL_MakeVectors(m_pPlayer->pev->v_angle);
		Vector vecSrc = m_pPlayer->GetGunPosition();
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
		switch (KnifeAttack(GetDashAttackDamage(), 100, 60, DMG_NEVERGIB | DMG_BULLET, m_pPlayer->pev, m_pPlayer, 2))
		{
		case HIT_PLAYER:
		{
			switch (RANDOM_LONG(0, 1))
			{
			case 0: { UTIL_EMIT_SOUND(m_pPlayer->edict(), CHAN_ITEM, "weapons/armtorch_hit1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM); break; }
			case 1: { UTIL_EMIT_SOUND(m_pPlayer->edict(), CHAN_ITEM, "weapons/armtorch_hit2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM); break; }
			}
			break;
		}
		case HIT_WALL:
		{
			UTIL_EMIT_SOUND(m_pPlayer->edict(), CHAN_ITEM, "weapons/armtorch_wall.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			//MaterialSound();
			break;
		}
		}
#endif
		SetThink(nullptr);
	}

	void CKnifeArmTorch::DelaySpecialAttack()
	{
#ifndef CLIENT_DLL		
		UTIL_MakeVectors(m_pPlayer->pev->v_angle);
		Vector vecSrc = m_pPlayer->GetGunPosition();
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
		switch (KnifeAttack(GetSpecialAttackDamage(), 150, 180, DMG_NEVERGIB | DMG_BULLET, m_pPlayer->pev, m_pPlayer, 0))
		{
		case HIT_PLAYER:
		{
			switch (RANDOM_LONG(0, 1))
			{
			case 0: { UTIL_EMIT_SOUND(m_pPlayer->edict(), CHAN_ITEM, "weapons/armtorch_hit1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM); break; }
			case 1: { UTIL_EMIT_SOUND(m_pPlayer->edict(), CHAN_ITEM, "weapons/armtorch_hit2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM); break; }
			}
			break;
		}
		case HIT_WALL:
		{
			UTIL_EMIT_SOUND(m_pPlayer->edict(), CHAN_ITEM, "weapons/armtorch_wall.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			//MaterialSound();
			break;
		}
		}
#endif
		SetThink(nullptr);
	}

	void CKnifeArmTorch::PrimaryAttack(void)
	{
		switch (m_iCheck)
		{
			case 0:
			case 1:
			{
				SendWeaponAnim(ANIM_SLASH1, UseDecrement() != FALSE);
				SetThink(&CKnifeArmTorch::DelayPrimaryAttack);
				pev->nextthink = gpGlobals->time + 0.4s;

				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;				
				m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.13s;
				
				m_iCheck = 1;
				break;
			}
			case 2:
			{
				SendWeaponAnim(ANIM_SLASH3_2, UseDecrement() != FALSE);

#ifndef CLIENT_DLL	
				Vector vecAngle = m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle;
				vecAngle.x = 0;

				UTIL_MakeVectors(vecAngle);

				Vector vecOrigin = m_pPlayer->GetGunPosition();
				Vector vecDir = gpGlobals->v_forward;
				Vector vecEnd = vecOrigin + gpGlobals->v_forward * 100.0;

				TraceResult tr;
				UTIL_TraceLine(vecOrigin, vecEnd, dont_ignore_monsters, m_pPlayer->edict(), &tr);

				Vector vecVelocity = UTIL_GetSpeedVector(m_pPlayer->pev->origin, tr.vecEndPos, 400.0);
				vecVelocity.z = 0;

				m_pPlayer->pev->velocity = vecVelocity;
				m_pPlayer->m_flVelocityModifier = 1.0;

				m_flDashTime = gpGlobals->time + 0.27s;
#endif

				SetThink(&CKnifeArmTorch::DelayDashAttack);
				pev->nextthink = gpGlobals->time + 0.27s;

				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;
				m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.2s;

				m_iCheck = 0;
				break;
			}
		}
	}

	void CKnifeArmTorch::SecondaryAttack(void)
	{
		switch (m_iCheck)
		{
		case 0:
		{
			SendWeaponAnim(ANIM_SLASH3, UseDecrement() != FALSE);

			SetThink(&CKnifeArmTorch::DelaySecondaryAttack);
			pev->nextthink = gpGlobals->time + 0.6s;

			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.7s;
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.4s;
			break;
		}
		case 1:
		{
			SendWeaponAnim(ANIM_SLASH2, UseDecrement() != FALSE);

			SetThink(&CKnifeArmTorch::DelaySecondaryAttack);
			pev->nextthink = gpGlobals->time + 0.2s;

			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.33s;
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.03s;

			m_iCheck = 2;
			break;
		}
		case 2:
		{
			SendWeaponAnim(ANIM_STAB, UseDecrement() != FALSE);

			SetThink(&CKnifeArmTorch::DelaySpecialAttack);
			pev->nextthink = gpGlobals->time + 0.47s;

			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.83s;
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.53s;

			m_iCheck = 0;
			break;
		}
		}
	}

	void CKnifeArmTorch::Holster(int skiplocal)
	{
		SetThink(nullptr);

		if (m_flDashTime > gpGlobals->time)
		{
			m_pPlayer->pev->velocity = g_vecZero;
		}

		m_flDashTime = invalid_time_point;
		return CBasePlayerWeapon::Holster(skiplocal);
	}

	void CKnifeArmTorch::WeaponIdle(void)
	{
		ResetEmptySound();
		m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;

		m_iCheck = 0;

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
		SendWeaponAnim(ANIM_IDLE, UseDecrement() != FALSE);
		
	}


}
