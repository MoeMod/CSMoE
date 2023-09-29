/*
knife_dgaxeex.cpp
Copyright (C) 2019 JustANoobQAQ

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

#define DGAXE_MAX_AMMO	120

	class CKnifeDgaxeEX : public LinkWeaponTemplate<CKnifeDgaxeEX,
		TGeneralData,
		BuildTGetItemInfoFromCSW<WEAPON_KNIFE>::template type
	>
	{
	public:
		static constexpr const char* ClassName = "knife_dgaxeex";
		static constexpr const char* V_Model = "models/v_dgaxeex.mdl";
		static constexpr const char* P_Model = "models/p_dgaxeex_a.mdl";
		static constexpr const char* P_Model_B = "models/p_dgaxeex_b.mdl";
		static constexpr const char* W_Model = "models/w_knife.mdl";

		static constexpr int MaxClip = DGAXE_MAX_AMMO;
		static constexpr auto ItemSlot = KNIFE_SLOT;
		static constexpr const char* AnimExtension = "tomahawk";
		KnockbackData KnockBack = { .0f, .0f, .0f, .0f, 1.0f };

	public:

		enum dgaxeex_e
		{
			DGAXEEX_IDLE,
			DGAXEEX_IDLE2,
			DGAXEEX_DRAW,
			DGAXEEX_SLASH1,
			DGAXEEX_SLASH2,
			DGAXEEX_SLASH3,
			DGAXEEX_SLASH4,
			DGAXEEX_DUALSLASH1,
			DGAXEEX_DUALSLASH2,
			DGAXEEX_COMBO,
			DGAXEEX_SKILL1,
			DGAXEEX_SKILL2
		};

		void Precache() override
		{
			PRECACHE_MODEL("models/v_dgaxeex.mdl");
			PRECACHE_MODEL("models/p_dgaxeex_a.mdl");
			PRECACHE_MODEL("models/p_dgaxeex_b.mdl");
#ifndef CLIENT_DLL
			PRECACHE_MODEL("sprites/muzzleflash333.spr");
			PRECACHE_MODEL("sprites/muzzleflash334.spr");
			PRECACHE_MODEL("sprites/muzzleflash335.spr");
			PRECACHE_MODEL("sprites/muzzleflash336.spr");
#endif
			PRECACHE_MODEL("sprites/ef_dgaxeex_change.spr");

			m_iShockWaveIndex = PRECACHE_MODEL("sprites/ef_dgaxeex_shockwave.spr");
				
			PRECACHE_SOUND("weapons/dgaxe_combo.wav");


			PRECACHE_SOUND("weapons/dgaxe_dualslash1.wav"); //left
			PRECACHE_SOUND("weapons/dgaxe_dualslash2.wav");

			PRECACHE_SOUND("weapons/dgaxe_slash1.wav");
			PRECACHE_SOUND("weapons/dgaxe_slash2.wav");
			PRECACHE_SOUND("weapons/dgaxe_slash3.wav");
			PRECACHE_SOUND("weapons/dgaxe_slash4.wav");

			PRECACHE_SOUND("weapons/dgaxe_slash_hit1.wav");
			PRECACHE_SOUND("weapons/dgaxe_slash_hit2.wav");
			PRECACHE_SOUND("weapons/dgaxe_slash_hit3.wav");
			PRECACHE_SOUND("weapons/dgaxe_wall_metal1.wav");
			PRECACHE_SOUND("weapons/dgaxe_wall_metal2.wav");
			PRECACHE_SOUND("weapons/dgaxe_wall_stone1.wav");
			PRECACHE_SOUND("weapons/dgaxe_wall_stone2.wav");

			m_usFire = PRECACHE_EVENT(1, "events/dgaxe.sc");

		}
		
		BOOL Deploy(void) override
		{
			m_fMaxSpeed = 260;
			m_iComboState = 0;
			m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;
			m_pPlayer->m_bShieldDrawn = false;

			m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] += (gpGlobals->time - m_flTimeNextCharge) / GetChargeTime();
			m_flTimeNextCharge = gpGlobals->time + GetChargeTime();
			if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] >= DGAXE_MAX_AMMO)
				m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] = DGAXE_MAX_AMMO;

			if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] >= 50 && m_iHolsterAmmo < 50)
			{		
				PLAYBACK_EVENT_FULL(FEV_HOSTONLY, m_pPlayer->edict(), m_usFire, 0.0, g_vecZero, g_vecZero, 0.0, 0.0, 0, 0, TRUE, FALSE);
			}

			if (DefaultDeploy(V_Model, P_Model, DGAXEEX_DRAW, "tomahawk", UseDecrement() != FALSE, 0.1s))
			{		
				if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] >= 50)
				{
					pev->skin = 1;
				}			
				else
				{
					pev->skin = 0;
				}
					
				return TRUE;
			}
			return FALSE;
		}

		int GetItemInfo(ItemInfo* p) override;
		float GetMaxSpeed() override { return 260; }
		void PrimaryAttack() override;
		void SecondaryAttack() override;
		void Spawn() override;
		void Holster(int skiplocal) override;
		void WeaponIdle() override;
		int ExtractAmmo(CBasePlayerWeapon* pWeapon) override;
		void ComboAttack();
		void Guard();
		KnockbackData GetKnockBackData() override;
		BOOL CanDrop() override { return false; }

		BOOL CanHolster() override;
#ifndef CLIENT_DLL
		hit_result_t KnifeAttack(float flDamage, float flRadius, float flAngleDegrees, int bitsDamageType, entvars_t* pevInflictor, CBasePlayer* pAttackPlayer, bool IsPrimaryAttack);
		Vector Get_ShootPosition(CBaseEntity* pevAttacker, Vector Start, int* bHit);
		void ItemPostFrame() override;
#endif

	public:
		float GetPrimaryAttackDamage() const
		{
			float flDamage = 90;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
			{
				if (enhancedzombie.value)
					flDamage = 2520;
				else
					flDamage *= 18.5f;
			}
				
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 10899.0f;
#endif
			return flDamage;
		}
		float GetSecondaryAttackDamage() const
		{
			float flDamage = 45;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
			{
				if (enhancedzombie.value)
					flDamage = 441;
				else
					flDamage *= 8.0f;
			}
			
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 1748.0f;
#endif
			return flDamage;
		}
	
		float ComboAttackDamage() const
		{
			float flDamage = 100.0f;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
			{
				if (enhancedzombie.value)
					flDamage = 4553;
				else
					flDamage *= 35.0f;
			}		
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 21352.0f;
#endif
			return flDamage;
		}

		duration_t GetChargeTime()
		{
			duration_t flTime = 0.25s;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flTime = 0.25s;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flTime = 0.25s;
#endif
			return flTime;
		}

		void ChangeCrossHairState(int iState)
		{
#ifndef CLIENT_DLL
			MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->pev);
			WRITE_BYTE(49);
			WRITE_BYTE(iState);
			MESSAGE_END();
#endif
			m_CrossHairState = iState;

		}

		time_point_t m_flTimeNextCharge;
		time_point_t m_flTimeNextSkill1;
		time_point_t m_flTimeNextSkill2;

		int m_iComboState;
		int m_iHolsterAmmo;
		int m_iSlashAnim;
		int m_iDualSlashAnim;	
		int m_CrossHairState;
		int m_usFire;
		int m_iShockWaveIndex;

	};

	LINK_ENTITY_TO_CLASS(knife_dgaxeex, CKnifeDgaxeEX)

	void CKnifeDgaxeEX::Spawn()
	{
		m_iKnifeID = KNIFE_DGAXEEX;
		pev->classname = MAKE_STRING("knife_dgaxeex");
		m_iId = WEAPON_KNIFE;
		Precache();
		m_iHolsterAmmo = 0;
		SET_MODEL(ENT(pev), "models/w_knife.mdl");
		m_flTimeNextSkill1 = m_flTimeNextSkill2 = invalid_time_point;
		m_flTimeNextCharge = gpGlobals->time;
		m_iClip = -1;
		m_iComboState = 0;
		m_iDefaultAmmo = 0;
		m_iSlashAnim = 3;
		m_iDualSlashAnim = 7;
		FallInit();
	}

	int CKnifeDgaxeEX::GetItemInfo(ItemInfo* p)
	{
		p->pszName = STRING(pev->classname);
		p->pszAmmo1 = "dgaxeexammo";
		p->iMaxAmmo1 = DGAXE_MAX_AMMO;
		p->pszAmmo2 = NULL;
		p->iMaxAmmo2 = -1;
		p->iMaxClip = DGAXE_MAX_AMMO;
		p->iSlot = 2;
		p->iPosition = 1;
		p->iId = WEAPON_KNIFE;
		p->iFlags = 0;
		p->iWeight = KNIFE_WEIGHT;

		return 1;
	}

	KnockbackData CKnifeDgaxeEX::GetKnockBackData() { return { 0.0f, 0.0f, 0.0f, 0.0f, 1.0f }; }

#ifndef CLIENT_DLL
	hit_result_t CKnifeDgaxeEX::KnifeAttack(float flDamage, float flRadius, float flAngleDegrees, int bitsDamageType, entvars_t* pevInflictor, CBasePlayer* pAttackPlayer, bool IsPrimaryAttack)
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
					if (IsPrimaryAttack)
					{
						if (!(player->m_iBuff & BUFF_IGNORE_KNOCKBACK))
						{
							auto strategy = dynamic_cast<CPlayerModStrategy_ZB1*>(player->m_pModStrategy.get());
							if (strategy)
							{
								strategy->SetStunSpeedTime(2.0s);
								strategy->SetStunGravityTime(2.0s);
							}
						}
					}
					else
					{
						Vector vecDir = (player->pev->origin - m_pPlayer->pev->origin);
						if (pev->iuser1)
						{
							UTIL_AddKickRateBoost(pEntity, vecDir.Normalize().Make2D(), 1.0, 1000.0, 600);
						}
						else
						{
							ApplyKnockbackData(player, vecDir, { 0.0, 0.0, 0.0, 0.0, 0.4f });
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

	void CKnifeDgaxeEX::PrimaryAttack(void)
	{
		switch (m_iComboState)
		{
		case 1:
		{
			m_iComboState = 2;
			break;
		}
		case 2:
		{
			m_pPlayer->pev->weaponmodel = MAKE_STRING(P_Model_B);
			Q_strcpy(m_pPlayer->m_szAnimExtention, "hammer");
#ifndef CLIENT_DLL	
			m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
			SendWeaponAnim(DGAXEEX_COMBO, UseDecrement() != FALSE);

			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/dgaxe_combo.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

			SetThink(&CKnifeDgaxeEX::ComboAttack);
			pev->nextthink = gpGlobals->time + 0.73s;

			m_pPlayer->m_iBuff |= BUFF_AVOID_INFECT;
			m_pPlayer->m_tAvoidInfectBuffTime = gpGlobals->time + 2.0s;

			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.51s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.03s;
			m_iComboState = 0;
			currentammo = 1;
			return;
		}
		default:
		{
			m_iComboState = 0;
			break;
		}
		}

		if (m_iDualSlashAnim > 8)
			m_iDualSlashAnim = 7;

#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);

		switch (KnifeAttack(GetPrimaryAttackDamage(), 180, 360, DMG_NEVERGIB | DMG_BULLET, this->pev, m_pPlayer, true))
		{
		case HIT_NONE:
		{
			SendWeaponAnim(m_iDualSlashAnim, UseDecrement() != FALSE);
			if (m_iDualSlashAnim % 2)
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/dgaxe_dualslash1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			else
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/dgaxe_dualslash2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			break;
		}
		case HIT_PLAYER:
		{
			SendWeaponAnim(m_iDualSlashAnim, UseDecrement() != FALSE);
			if (m_iDualSlashAnim % 2)
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/dgaxe_slash_hit2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			else
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/dgaxe_slash_hit3.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;
			break;
		}
		case HIT_WALL:
		{
			SendWeaponAnim(m_iDualSlashAnim, UseDecrement() != FALSE);
			if (m_iDualSlashAnim % 2)
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/dgaxe_wall_stone1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			else
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/dgaxe_wall_stone2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

			m_pPlayer->m_iWeaponVolume = KNIFE_WALLHIT_VOLUME * 0.5;
			break;
		}
		}
#endif
		m_iDualSlashAnim++;
		m_iSlashAnim++;
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.97s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.03s;
	}

	int CKnifeDgaxeEX::ExtractAmmo(CBasePlayerWeapon* pWeapon)
	{
		if (m_iDefaultAmmo)
		{
			m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] = m_iDefaultAmmo;
			m_iClip = WEAPON_NOCLIP;
			m_iDefaultAmmo = 0;
			return TRUE;
		}
		return CBasePlayerWeapon::ExtractAmmo(pWeapon);
	}

	void CKnifeDgaxeEX::SecondaryAttack(void)
	{
		if (!m_iComboState)
			m_iComboState = 1;

		if (m_iSlashAnim >= 7)
			m_iSlashAnim = 3;

		m_pPlayer->pev->weaponmodel = MAKE_STRING(P_Model);
		Q_strcpy(m_pPlayer->m_szAnimExtention, "tomahawk");
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
		switch (KnifeAttack(GetSecondaryAttackDamage(), 180, 180, DMG_NEVERGIB | DMG_BULLET, this->pev, m_pPlayer, true))
		{
		case HIT_NONE:
		{
			SendWeaponAnim(m_iSlashAnim, UseDecrement() != FALSE);
			switch (m_iSlashAnim)
			{
			case 3:EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/dgaxe_slash1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM); break;
			case 4:EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/dgaxe_slash2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM); break;
			case 5:EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/dgaxe_slash3.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM); break;
			case 6:EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/dgaxe_slash4.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM); break;
			}
			break;
		}
		case HIT_PLAYER:
		{
			SendWeaponAnim(m_iSlashAnim, UseDecrement() != FALSE);
			if (m_iSlashAnim % 2)
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/dgaxe_slash_hit1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			else
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/dgaxe_slash_hit3.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;
			break;
		}
		case HIT_WALL:
		{
			SendWeaponAnim(m_iSlashAnim, UseDecrement() != FALSE);
			if (m_iSlashAnim % 2)
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/dgaxe_wall_stone1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			else
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/dgaxe_wall_stone2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

			m_pPlayer->m_iWeaponVolume = KNIFE_WALLHIT_VOLUME * 0.5;
			break;
		}
		}
#endif
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.15s;
		m_iSlashAnim++;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.03s;
	}

	void CKnifeDgaxeEX::ComboAttack()
	{
		UTIL_MakeVectors(m_pPlayer->pev->v_angle);
		Vector vecSrc = m_pPlayer->GetGunPosition();
		Vector vecEnd = vecSrc + gpGlobals->v_forward * 220;

		m_pPlayer->pev->weaponmodel = MAKE_STRING(P_Model);
		Q_strcpy(m_pPlayer->m_szAnimExtention, "tomahawk");
#ifndef CLIENT_DLL
		currentammo = 0;

		Vector angle = UTIL_VecToAngles({ 0, 0, 1 });

		UTIL_TempModel({ 0, 0, (m_pPlayer->pev->flags & FL_DUCKING ? -16.0f : -32.0f) }, angle, g_vecZero, m_iShockWaveIndex, 20, 0, 255, 0, 0, 255, kRenderNormal, m_pPlayer, false, 0, 30, 256, 0x20000000);
		
		pev->iuser1 = 1;
		switch (KnifeAttack(ComboAttackDamage(), 220, 180, DMG_NEVERGIB | DMG_BULLET, this->pev, m_pPlayer, false))
		{
		case HIT_NONE:
		{

		}
		case HIT_PLAYER:
		{

			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/dgaxe_slash_hit1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			break;
		}
		case HIT_WALL:
		{

			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/dgaxe_wall_stone1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			break;
		}
		}
		pev->iuser1 = 0;
#endif
	}

	void CKnifeDgaxeEX::WeaponIdle(void)
	{
		ResetEmptySound();
		m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;

		m_iComboState = 0;

		if (RANDOM_LONG(0, 5))
		{
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 8s;
			SendWeaponAnim(DGAXEEX_IDLE, UseDecrement() != FALSE);
		}
		else
		{
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;
			SendWeaponAnim(DGAXEEX_IDLE2, UseDecrement() != FALSE);
		}
		
	}

	void CKnifeDgaxeEX::Holster(int skiplocal)
	{
		SetThink(nullptr);
		m_pPlayer->m_iBuff &= ~BUFF_AVOID_INFECT;
		m_iHolsterAmmo = m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType];
		m_flTimeNextCharge = gpGlobals->time;
		return CBasePlayerWeapon::Holster(skiplocal);
	}

#ifndef CLIENT_DLL
	Vector CKnifeDgaxeEX::Get_ShootPosition(CBaseEntity* pevAttacker, Vector Start, int* bHit = 0)
	{
		Vector end;

		end = pevAttacker->pev->v_angle;
		UTIL_MakeVectors(end);
		end = gpGlobals->v_forward;
		end = end * 8192.0;
		end = Start + end;

		TraceResult tr;
		UTIL_TraceLine(Start, end, dont_ignore_monsters, pevAttacker->edict(), &tr);
		end = tr.vecEndPos;
		if (tr.flFraction != 1.0f)
		{
			CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);
			{
				if (pEntity->pev->takedamage != DAMAGE_NO)
				{
					if (pEntity->IsBSPModel())
						return end;

					if (pEntity->IsPlayer() && g_pGameRules->PlayerRelationship(m_pPlayer, pEntity) == GR_TEAMMATE)
						return end;

					*bHit = 1;
				}
			}
		}
		return end;
	}

	void CKnifeDgaxeEX::ItemPostFrame()
	{
		int m_iButton = m_pPlayer->pev->button;

		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] < DGAXE_MAX_AMMO)
		{
			if (m_flTimeNextCharge <= gpGlobals->time)
			{
				m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]++;

				m_flTimeNextCharge = gpGlobals->time + GetChargeTime();

				if(m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] == 50)
					PLAYBACK_EVENT_FULL(FEV_HOSTONLY, m_pPlayer->edict(), m_usFire, 0.0, g_vecZero, g_vecZero, 0.0, 0.0, 0, 0, TRUE, FALSE);
			}
		}
		else
			m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] = DGAXE_MAX_AMMO;


#ifndef CLIENT_DLL
		int bHit = 0;
		Vector vecSrc = m_pPlayer->GetGunPosition();
		CBaseEntity* pevAttacker = this->m_pPlayer;
		auto vecShootPosition = Get_ShootPosition(pevAttacker, vecSrc, &bHit);
		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] >= 50)
		{
			if (m_iButton & IN_ATTACK)
			{
				if (!m_CrossHairState)
				{
					ChangeCrossHairState(1);
				}
				
			}
			else
			{
				if (m_CrossHairState)
				{
					ChangeCrossHairState(0);
				}
			}


			if (bHit)
			{
				if (m_iButton & IN_RELOAD)
				{
					if (m_flTimeNextSkill2 < gpGlobals->time)
					{
						m_pPlayer->SetAnimation(PLAYER_ATTACK1);
						SendWeaponAnim(DGAXEEX_SKILL1, UseDecrement() != FALSE);
						m_pPlayer->m_iBuff |= BUFF_AVOID_INFECT;
						m_pPlayer->m_tAvoidInfectBuffTime = gpGlobals->time + 2.0s;

						CKnifeDGAXE_LuBu* pEnt = CKnifeDGAXE_LuBu::Create(1, vecShootPosition, g_vecZero, ENT(m_pPlayer->pev), m_pPlayer, m_pPlayer->m_iTeam, 0);

						m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] -= 40;
						m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.51s;
						m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.03s;
						m_flTimeNextSkill2 = gpGlobals->time + 3.0s;
					}
				}
			}
			else
			{
				if (m_iButton & IN_RELOAD && m_iButton & IN_ATTACK)
				{
					if (m_flTimeNextSkill1 < gpGlobals->time)
					{
						m_pPlayer->SetAnimation(PLAYER_ATTACK1);
						SendWeaponAnim(DGAXEEX_SKILL2, UseDecrement() != FALSE);
						m_pPlayer->m_iBuff |= BUFF_AVOID_INFECT;
						m_pPlayer->m_tAvoidInfectBuffTime = gpGlobals->time + 2.0s;

						CKnifeDGAXE_LuBu* pEnt = CKnifeDGAXE_LuBu::Create(1, vecShootPosition, g_vecZero, ENT(m_pPlayer->pev), m_pPlayer, m_pPlayer->m_iTeam, 1);
		
						m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] -= 50;
						m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.51s;
						m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.03s;
						m_flTimeNextSkill1 = gpGlobals->time + 8.0s;
					}

				}
			}
				
		}



		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] >= 50)
		{
			if(!pev->skin)
				pev->skin = 1;
		}
			
		else
		{
			if (pev->skin)
			{
				pev->skin = 0;

			}
		}
		

#endif
		return CBasePlayerWeapon::ItemPostFrame();
	}
#endif

	BOOL CKnifeDgaxeEX::CanHolster(void)
	{
#ifndef CLIENT_DLL
		if (currentammo)
			return FALSE;
		else
			return TRUE;
#endif
		return FALSE;
	}
}
