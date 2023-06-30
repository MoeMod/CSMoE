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

		static constexpr int MaxClip = 100;
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
	
			PRECACHE_SOUND("weapons/armtorch_hit1.wav");
			PRECACHE_SOUND("weapons/armtorch_hit2.wav");
			PRECACHE_SOUND("weapons/armtorch_wall.wav");
			
			PRECACHE_SOUND("weapons/armtorch_idle.wav");
		}
		
		BOOL Deploy(void) override
		{
			m_fMaxSpeed = 250;
			m_iState = 0;
			m_iState2 = 0;
			m_iCheck = 0;
			m_iActSlash3 = 0;
			m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;

			m_tTimeRestMaxSpeed = m_tTimeSoundEmit = m_flTimeCheckCharge = invalid_time_point;
			m_pPlayer->m_bShieldDrawn = false;

			return DefaultDeploy(V_Model, P_Model, ANIM_DRAW, "dragontail", UseDecrement() != FALSE);
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
		void Slash3Think();
		void OnAttack2();
		void OnAttack2Released();
		void ComboAttack();
		void FakeKnockBack();
#ifndef CLIENT_DLL
		hit_result_t KnifeAttack(float flDamage, float flRadius, float flAngleDegrees, int bitsDamageType, entvars_t* pevInflictor, CBasePlayer* pAttackPlayer, int iType);
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
				flDamage = 332.0f;
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
				flDamage = 900.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 3600.0f;
#endif
			return flDamage;
		}
		float FieldAttackDamage() const
		{
			float flDamage = 4;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 110.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 240.0f;
#endif
			return flDamage;
		}
		float SpecialAttackDamage() const
		{
			float flDamage = 35;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 850.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 1280.0f;
#endif
			return flDamage;
		}
		float Slash3ExpDamage() const
		{
			float flDamage = 95;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 1100.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 4900.0f;
#endif
			return flDamage;
		}

		time_point_t m_flTimeCheckCharge;
		time_point_t m_flNextThink;
		time_point_t m_tTimeRestMaxSpeed;
		int m_iThinkType;
		int m_iDamageType;
		int m_iState;	//pev_iuser1
		int m_iState2;	//pev_iuser4
		int m_iActSlash3;
		int m_iCheck;	//g_iCheck
		int m_iFrozen;	//g_Frozen
		int m_iCheckSpecial;	//CheckSpecial
	};

	LINK_ENTITY_TO_CLASS(knife_armtorch, CKnifeArmTorch)

	void CKnifeArmTorch::Spawn()
	{
		pev->classname = MAKE_STRING("knife_armtorch");
		m_iId = WEAPON_KNIFE;
		Precache();
		SET_MODEL(ENT(pev), "models/w_knife.mdl");

		m_iClip = WEAPON_NOCLIP;
		m_iState = 0;
		m_iState2 = 0;
		m_iCheck = 0;
		m_iActSlash3 = 0;
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
					Vector vecDir = (player->pev->origin - m_pPlayer->pev->origin);
					vecDir.z = 0;
					if (!iType)
					{
						//left
						ApplyKnockbackData(player, vecDir, { 0.0, 0.0, 0.0, 0.0, 0.3f });

						auto strategy = dynamic_cast<CPlayerModStrategy_ZB1*>(player->m_pModStrategy.get());
						if (strategy)
						{
							strategy->SetStunSpeedTime(3.0s);
							strategy->SetStunGravityTime(3.0s);
						}	
					}
					else if(iType == 1)
					{
						//left3
						if (player->m_bIsZombie && !(player->m_iBuff & BUFF_IGNORE_KNOCKBACK))
						{
							Vector2D vecDir = (player->pev->origin - pAttackPlayer->pev->origin).Make2D().Normalize();

							player->pev->velocity.x = vecDir.x * 2400.0;
							player->pev->velocity.y = vecDir.y * 2400.0;
						}
					}
					else
					{
						//right
						if (player->m_bIsZombie && !(player->m_iBuff & BUFF_IGNORE_KNOCKBACK))
						{
							Vector2D vecDir = (player->pev->origin - pAttackPlayer->pev->origin).Make2D().Normalize();

							player->pev->velocity.x = vecDir.x * 2100.0;
							player->pev->velocity.y = vecDir.y * 2100.0;
							player->pev->velocity.z = 290.0;
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

	void CKnifeArmTorch::DelayPrimaryAttack(void)
	{
		
#ifndef CLIENT_DLL		
		UTIL_MakeVectors(m_pPlayer->pev->v_angle);
		Vector vecSrc = m_pPlayer->GetGunPosition();
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
		switch (KnifeAttack(GetPrimaryAttackDamage(), 180, 85, DMG_NEVERGIB | DMG_BULLET, m_pPlayer->pev, m_pPlayer, 0))
		{
		case HIT_PLAYER:
		{
			switch (RANDOM_LONG(0, 2))
			{
			case 0: { EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_AUTO, "weapons/armtorch_slash_hit1.wav", VOL_NORM, ATTN_NORM, 0, 94); break; }
			case 1: { EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_AUTO, "weapons/armtorch_slash_hit2.wav", VOL_NORM, ATTN_NORM, 0, 94); break; }
			case 2: { EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_AUTO, "weapons/armtorch_slash_hit3.wav", VOL_NORM, ATTN_NORM, 0, 94); break; }
			}
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


	void CKnifeArmTorch::ItemPostFrame()
	{


		return CBasePlayerWeapon::ItemPostFrame();
	}

	void CKnifeArmTorch::PrimaryAttack(void)
	{
		if (m_iCheck == 3)
		{
			m_iCheck = 4;
			m_iCheckSpecial = 1;
			ComboAttack();

			return;
		}

		switch (m_iCheck)
		{
			case 0:
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

				SetThink(&CKnifeArmTorch::DelayPrimaryAttack);
				pev->nextthink = gpGlobals->time + 0.27s;

				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;
				m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.2s;

				m_iCheck = 0;
				break;
			}
			case 3:
			{
			
			}
		}
	}

	void CKnifeArmTorch::Slash3Think()
	{
		if (!m_iActSlash3)
		{
			
#ifndef CLIENT_DLL
			UTIL_MakeVectors(m_pPlayer->pev->v_angle);
			Vector vecSrc = m_pPlayer->GetGunPosition();
			m_pPlayer->SetAnimation(PLAYER_ATTACK1);
			switch (KnifeAttack(GetPrimaryAttackDamage(), 180, 85, DMG_NEVERGIB | DMG_BULLET, m_pPlayer->pev, m_pPlayer, 0))
			{
			case HIT_PLAYER:
			{
				switch (RANDOM_LONG(0, 2))
				{
				case 0: { EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_AUTO, "weapons/armtorch_slash_hit1.wav", VOL_NORM, ATTN_NORM, 0, 94); break; }
				case 1: { EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_AUTO, "weapons/armtorch_slash_hit2.wav", VOL_NORM, ATTN_NORM, 0, 94); break; }
				case 2: { EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_AUTO, "weapons/armtorch_slash_hit3.wav", VOL_NORM, ATTN_NORM, 0, 94); break; }
				}			
				break;
			}
			case HIT_WALL:
			{
				MaterialSound();
				break;
			}
			}	
#endif
			m_iActSlash3 = 1;
			SetThink(nullptr);
			SetThink(&CKnifeArmTorch::Slash3Think);
			pev->nextthink = gpGlobals->time + 0.67s;
		}
		else
		{	
			UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
			Vector vecSrc = m_pPlayer->GetGunPosition();
			Vector vecOrigin = vecSrc + gpGlobals->v_forward * 55;

#ifndef CLIENT_DLL
			KnifeAttack(Slash3ExpDamage(), 260, 85, DMG_NEVERGIB | DMG_BULLET, m_pPlayer->pev, m_pPlayer, 1);
			PLAYBACK_EVENT_FULL(FEV_GLOBAL, m_pPlayer->edict(), PRECACHE_EVENT(1, "events/wpneffects.sc"), 0.0, vecOrigin, g_vecZero, 0.0, 0.0, 8, 0, FALSE, FALSE);
#endif			
			FakeKnockBack();
			m_iActSlash3 = 0;
			SetThink(nullptr);
		}

		m_iState = 0;
		
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

			if (m_iCheck == 1)
				m_iCheck = 2;
			break;
		}
		case 1:
		{
			SendWeaponAnim(ANIM_SLASH2, UseDecrement() != FALSE);

			SetThink(&CKnifeArmTorch::DelaySecondaryAttack);
			pev->nextthink = gpGlobals->time + 0.2s;

			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.43s;
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.03s;

			m_iCheck = 2;
			break;
		}
		default:
			
		}



		if (!m_iState2)
		{
			SendWeaponAnim(ANIM_FIELD_ATTACK_CHECK, UseDecrement() != FALSE);

			m_iState2 = 1;

			m_flTimeCheckCharge = gpGlobals->time + 0.2s;
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.2s;
		}
		if (m_flTimeCheckCharge < gpGlobals->time)
		{
			switch (m_iState2)
			{
			case 1:
			{
				SendWeaponAnim(ANIM_FIELD_ATTACK_START, UseDecrement() != FALSE);

				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_AUTO, "weapons/armtorch_field_attack_start.wav", VOL_NORM, ATTN_NORM, 0, 94);

				m_iState2 = 2;

				m_flTimeCheckCharge = gpGlobals->time + 0.66s;
				m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.67s;
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.85s;
				break;
			}
			case 2:
			{
				SendWeaponAnim(ANIM_FIELD_ATTACK_CHARGE, UseDecrement() != FALSE);

				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/armtorch_field_attack_charge.wav", VOL_NORM, ATTN_NORM, 0, 94);

				m_iState2 = 3;

				m_flTimeCheckCharge = gpGlobals->time + 1.13s;
				m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 999999s;
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.33s;
				break;
			}
			case 3:
			{
				m_tTimeSoundEmit = gpGlobals->time + 0.01s;
				m_flTimeCheckCharge = gpGlobals->time + 999999s;
				m_iState2 = 4; //ChareFinsh
				break;
			}
			}
		}
	}

	void CKnifeArmTorch::ComboAttack()
	{
		switch (m_iCheck)
		{
		case 4:
		{
			SendWeaponAnim(ANIM_SPECIAL_ATTACK, UseDecrement() != FALSE);

#ifndef CLIENT_DLL
			UTIL_MakeVectors(m_pPlayer->pev->v_angle);
			Vector vecSrc = m_pPlayer->GetGunPosition();
			m_pPlayer->SetAnimation(PLAYER_ATTACK1);
			switch (KnifeAttack(GetPrimaryAttackDamage(), 180, 85, DMG_NEVERGIB | DMG_BULLET, m_pPlayer->pev, m_pPlayer, 0))
			{
			case HIT_PLAYER:
			{
				switch (RANDOM_LONG(0, 2))
				{
				case 0: { EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_AUTO, "weapons/armtorch_slash_hit1.wav", VOL_NORM, ATTN_NORM, 0, 94); break; }
				case 1: { EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_AUTO, "weapons/armtorch_slash_hit2.wav", VOL_NORM, ATTN_NORM, 0, 94); break; }
				case 2: { EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_AUTO, "weapons/armtorch_slash_hit3.wav", VOL_NORM, ATTN_NORM, 0, 94); break; }
				}

				break;
			}
			case HIT_WALL:
			{
				MaterialSound();
				break;
			}
			}
#endif
			m_flNextSecondaryAttack = m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 2.42s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.35s;
			m_pPlayer->m_flNextAttack = 2.4s;
			SetThink(&CKnifeArmTorch::ComboAttack);
			m_iCheck = 5;
			pev->nextthink = gpGlobals->time + 1.16s;

			m_iState = m_iState2 = 0;
			break;
		}
		case 5:
		{
			SendWeaponAnim(ANIM_SPECIAL_ATTACK_INOPERATION, UseDecrement() != FALSE);

			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;

			m_tTimeRestMaxSpeed = gpGlobals->time + 0.7s;
			m_pPlayer->pev->velocity.x = 0.0;
			m_pPlayer->pev->velocity.y = 0.0;
			m_pPlayer->ResetMaxSpeed();
			//do buff
#ifndef CLIENT_DLL
			Vector vecAngle = m_pPlayer->pev->punchangle + m_pPlayer->pev->v_angle;
			vecAngle[0] = 0;
			UTIL_MakeVectors(vecAngle);
			Vector vecSrc = m_pPlayer->GetGunPosition();
			Vector vecOrigin = vecSrc + gpGlobals->v_forward * 80;
			Vector vecEnd = vecOrigin;
			vecEnd[2] -= 150;	
			TraceResult tr;
			UTIL_TraceHull(vecOrigin, vecEnd, dont_ignore_monsters, point_hull, ENT(m_pPlayer->pev), &tr);
			if (tr.flFraction != 1.0f)
				vecOrigin = tr.vecEndPos;
			else
				vecOrigin[2] -= 64;
			CarmtorchFountainSpecialAttack* pEnt = CarmtorchFountainSpecialAttack::Create("armtorch_fountainspecial", vecOrigin, { 0,0,0 }, ENT(m_pPlayer->pev), 1);
			if (pEnt)
			{
				pEnt->Init(SpecialAttackDamage(), 0, gpGlobals->v_forward);
			}
#endif
			m_iCheck = 6;
			SetThink(nullptr);
			SetThink(&CKnifeArmTorch::ComboAttack);
			pev->nextthink = gpGlobals->time + 0.33s;
			m_iFrozen = 1;

			//m_pPlayer->pev->maxspeed = 1.0;
			break;		
		}
		case 6:
		{
			SendWeaponAnim(ANIM_SPECIAL_ATTCK_END, UseDecrement() != FALSE);

			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.86s;
			m_iFrozen = 0;
			m_iCheck = 0;

			SetThink(nullptr);
			break;
		}
		}
	}

	void CKnifeArmTorch::OnAttack2Released(void)
	{
		switch (m_iState2)
		{
		case 4:
		{
#ifndef CLIENT_DLL	
			m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
			SendWeaponAnim(ANIM_FIELD_ATTACK_ACTUATION, UseDecrement() != FALSE);

			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/armtorch_field_attack_actuation.wav", VOL_NORM, ATTN_NORM, 0, 94);

			m_flNextSecondaryAttack = m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.5s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.86s;
			m_iState2 = 5;
			m_pPlayer->m_flNextAttack = 1.5s;
			SetThink(&CKnifeArmTorch::OnAttack2Released);
			pev->nextthink = gpGlobals->time + 0.53s;
			break;
		}
		case 5:
		{
			SendWeaponAnim(ANIM_FIELD_ATTACK_INOPERATION, UseDecrement() != FALSE);

			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.86s;
			m_iState2 = 6;

			m_tTimeRestMaxSpeed = gpGlobals->time + 0.7s;
			m_pPlayer->pev->velocity.x = 0.0;
			m_pPlayer->pev->velocity.y = 0.0;
			m_pPlayer->ResetMaxSpeed();

			//do buff
#ifndef CLIENT_DLL
			Vector vecOrigin = m_pPlayer->pev->origin;
			vecOrigin[2] -= 20.0;

			CarmtorchFountainFieldAttack* pEnt = static_cast<CarmtorchFountainFieldAttack*>(CBaseEntity::Create("armtorch_fountainfield", vecOrigin, { 0,0,0 }, ENT(m_pPlayer->pev)));
			if (pEnt)
			{
				pEnt->Init(FieldAttackDamage(), 0, m_pPlayer);
			}
#endif

			SetThink(nullptr);
			SetThink(&CKnifeArmTorch::OnAttack2Released);
			pev->nextthink = gpGlobals->time + 0.33s;
			break;
		}
		case 6:
		{
			SendWeaponAnim(ANIM_FIELD_ATTACK_END, UseDecrement() != FALSE);

			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.7s;
			m_iState2 = 0;
			SetThink(nullptr);
			break;
		}
		}
	}

	void CKnifeArmTorch::DelaySecondaryAttack(void)
	{
#ifndef CLIENT_DLL		
		UTIL_MakeVectors(m_pPlayer->pev->v_angle);
		Vector vecSrc = m_pPlayer->GetGunPosition();
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);

		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/armtorch_snd_attack.wav", VOL_NORM, ATTN_NORM, 0, 94);

		switch (KnifeAttack(GetSecondaryAttackDamage(), 185, 360, DMG_NEVERGIB | DMG_BULLET, m_pPlayer->pev, m_pPlayer, 2))
		{
		case HIT_PLAYER:
		{
			switch (RANDOM_LONG(0, 2))
			{
			case 0: { EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_AUTO, "weapons/armtorch_slash_hit1.wav", VOL_NORM, ATTN_NORM, 0, 94); break; }
			case 1: { EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_AUTO, "weapons/armtorch_slash_hit2.wav", VOL_NORM, ATTN_NORM, 0, 94); break; }
			case 2: { EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_AUTO, "weapons/armtorch_slash_hit3.wav", VOL_NORM, ATTN_NORM, 0, 94); break; }
			}
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

	void CKnifeArmTorch::OnAttack2(void)
	{
		if (m_iCheck == 1)
			m_iCheck = 2;
		else if (m_iCheck == 2)
			m_iCheck = 3;
		else
			m_iCheck = 0;

		SendWeaponAnim(ANIM_SNDATTACK, UseDecrement() != FALSE);

		SetThink(&CKnifeArmTorch::DelaySecondaryAttack);
		pev->nextthink = gpGlobals->time + 0.3s;
		
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.2s;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.8s;

		m_iState2 = 0;
	}

	float CKnifeArmTorch::GetMaxSpeed(void)
	{
		if (m_tTimeRestMaxSpeed > gpGlobals->time)
			return 1;
		else if (m_tTimeRestMaxSpeed == invalid_time_point)
			return 252;
		else
			return 252;
	}

	void CKnifeArmTorch::Holster(int skiplocal)
	{
		SetThink(nullptr);
		//EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "", VOL_NORM, ATTN_NORM, 0, 94);
		//EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "", VOL_NORM, ATTN_NORM, 0, 94);
		return CBasePlayerWeapon::Holster(skiplocal);
	}

	void CKnifeArmTorch::WeaponIdle(void)
	{
		ResetEmptySound();
		m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

		m_iState = 0;

		if (m_iState2 && !(m_pPlayer->pev->button & IN_ATTACK2))
		{
			if (m_iState2 == 4)
				OnAttack2Released();
			else
				OnAttack2();
		}

		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;

		m_iCheck = 0;

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
		SendWeaponAnim(ANIM_IDLE, UseDecrement() != FALSE);
		
	}

}
