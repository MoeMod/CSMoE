/*
knife_swordbombard.cpp
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

#ifndef CLIENT_DLL
namespace sv {

#define SWORD_TIME	 5s

	class CPetrolMine : public CBaseEntity
	{
	public:
		void Spawn(void);
		int GetWeaponsId(void) { return KNIFE_SWORDBOMBARD; }

		void Precache()
		{
			PRECACHE_MODEL("models/ef_swordbombard_potal.mdl");
			PRECACHE_MODEL("models/ef_swordbombard.mdl");
			PRECACHE_MODEL("models/ef_swordbombard.mdl");

			PRECACHE_SOUND("weapons/swordbombard_fly_last.wav");
			PRECACHE_SOUND("weapons/swordbombard_fly1.wav");
			PRECACHE_SOUND("weapons/swordbombard_fly2.wav");
			PRECACHE_SOUND("weapons/swordbombard_fly3.wav");
			PRECACHE_SOUND("weapons/swordbombard_fly4.wav");
			PRECACHE_SOUND("weapons/swordbombard_fly5.wav");

			PRECACHE_SOUND("weapons/swordbombard_skill.wav");
			PRECACHE_SOUND("weapons/swordbombard_skill_fx.wav");
			PRECACHE_SOUND("weapons/swordbombard_skill_loop.wav");
		};


	public:
		void EXPORT PetrolMine_Think(void);

	public:
		void Attack();

	public:
		time_point_t m_fLaunchTime;
		Vector m_vecAngle;

	private:
		float GetRadiusDamage() const
		{
			float flDamage = 4.0f;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 629.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 1260.0f;
#endif
			return flDamage;
		}

		float GetSlashDamage() const
		{
			float flDamage = 4.0f;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 629.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 1260.0f;
#endif
			return flDamage;
		}
	};

	LINK_ENTITY_TO_CLASS(petrol_mine, CPetrolMine)


	void CPetrolMine::PetrolMine_Think(void)
	{
		if (gpGlobals->time >= m_fLaunchTime + SWORD_TIME)
		{
			SetThink(NULL);
			SetTouch(NULL);

			UTIL_Remove(this);

			return;
		}

		SetThink(&CPetrolMine::PetrolMine_Think);

		pev->nextthink = gpGlobals->time + 0.3s;

		Attack();
	}

	void CPetrolMine::Attack()
	{
		TraceResult tr;
		float flDamage = GetRadiusDamage();
		CBaseEntity* pList[100];
		Vector vecStart;

		vecStart = pev->origin + Vector(0.0, 0.0, 1.0);

		const float flWidth = 8 * 39.37;
		const float flLength = 3 * 39.37;
		const float flHeight = 4 * 39.37;
		//int iCount = UTIL_EntitiesInBox(pList, 100, vecStart - Vector(flLength, flWidth, 0.0), vecStart + Vector(flLength, flWidth, flHeight), FL_CLIENT | FL_MONSTER);

		int iCount = UTIL_EntitiesInRotatedBox(pList, 100, pev->origin, m_vecAngle, flLength, flWidth, flHeight, FL_CLIENT | FL_MONSTER);

		if (pev->owner)
		{
			CBasePlayer* pOwner = (CBasePlayer*)CBaseEntity::Instance(pev->owner);

			if (pOwner)
			{
				if (pOwner->IsPlayer() && pOwner->m_bIsZombie)
					flDamage = 0.0;
			}
		}

		for (int i = 0; i < iCount; i++)
		{
			CBaseEntity* pEntity = pList[i];

			if (pEntity == this)
				continue;

			if (pEntity->pev->takedamage == DAMAGE_NO)
				continue;

			if (pEntity->edict() == pev->owner)
				continue;

			UTIL_TraceLine(vecStart, pEntity->BodyTarget(vecStart), dont_ignore_monsters, edict(), &tr);

			if (tr.flFraction < 1.0 && tr.pHit != pEntity->edict())
				continue;

			pEntity->TakeDamage(pev, VARS(pev->owner), flDamage, DMG_GENERIC);
		}
	}

	void CPetrolMine::Spawn(void)
	{
		pev->movetype = MOVETYPE_TOSS;
		pev->solid = SOLID_BBOX;
		pev->gravity = 0.5;
		pev->friction = 0.0;
		pev->takedamage = DAMAGE_NO;
		pev->max_health = 100;
		pev->health = 100;

		SET_MODEL(ENT(pev), "models/null.mdl");
		UTIL_SetSize(pev, Vector(0.0, 0.0, 0.0), Vector(0.0, 0.0, 0.0));
		UTIL_SetOrigin(pev, pev->origin);

		SetThink(&CPetrolMine::PetrolMine_Think);
		pev->nextthink = gpGlobals->time + 0.1s;

		SetTouch(NULL);
		m_fLaunchTime = gpGlobals->time;
	}
}
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif


#define KNIFE_BODYHIT_VOLUME 128
#define KNIFE_WALLHIT_VOLUME 512

	class CKnifeSwordBombard : public LinkWeaponTemplate<CKnifeSwordBombard,
		TGeneralData,
		BuildTGetItemInfoFromCSW<WEAPON_KNIFE>::template type
	>
	{
	public:
		static constexpr const char* ClassName = "knife_swordbombard";
		static constexpr const char* V_Model = "models/v_swordbombard.mdl";
		static constexpr const char* P_Model = "models/p_swordbombard.mdl";
		static constexpr const char* W_Model = "models/w_knife.mdl";

		static constexpr int MaxClip = -1;
		static constexpr auto ItemSlot = KNIFE_SLOT;
		static constexpr const char* AnimExtension = "sfsword";
		KnockbackData KnockBack = { 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };

	public:
		enum swordbombard_e
		{
			ANIM_IDLE,
			ANIM_SLASH_A1,
			ANIM_SLASH_A2,
			ANIM_SLASH_B,
			ANIM_DRAW,
			ANIM_SKILL_START,
			ANIM_SKILL_LOOP,
			ANIM_SKILL_END,
		};

		void Precache() override
		{
			PRECACHE_MODEL("models/v_swordbombard.mdl");
			PRECACHE_MODEL("models/p_swordbombard.mdl");

			m_iSlashModelIndex = PRECACHE_MODEL("models/ef_swordbombard_slashb.mdl");
			PRECACHE_MODEL("models/ef_swordbombard_potal.mdl");
			PRECACHE_MODEL("models/ef_swordbombard_big.mdl");
			PRECACHE_MODEL("models/ef_swordbombard.mdl");

			PRECACHE_MODEL("sprites/muzzleflash268.spr");
			PRECACHE_MODEL("sprites/muzzleflash269.spr");
			
			PRECACHE_SOUND("weapons/swordbombard_slash_a1.wav");
			PRECACHE_SOUND("weapons/swordbombard_slash_a2.wav");
			PRECACHE_SOUND("weapons/swordbombard-slash_b.wav");
		}
		
		BOOL Deploy(void) override
		{
			m_fMaxSpeed = 260;
			m_iCheck = 0;

			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.35s;
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3s;
			return DefaultDeploy(V_Model, P_Model, ANIM_DRAW, "sfsword", UseDecrement() != FALSE, 0.1s);
		}

		void PrimaryAttack() override;	
		void SecondaryAttack() override;
		void Spawn() override;
		void WeaponIdle() override;
		float GetMaxSpeed() override;
		void Holster(int skiplocal) override;
		void OnPrimaryAttack();
		void DelaySecondaryAttack();
		void ComboAttack();
		BOOL CanDrop() override { return false; }
		void ItemPostFrame() override;

	public:
		float GetPrimaryAttackDamage() const
		{
			float flDamage = 45.0f;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 1242.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 2600.0f;
#endif
			return flDamage;
		}

		float GetSecondaryAttackDamage() const
		{
			float flDamage = 50;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 2581.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 10400.0f;
#endif
			return flDamage;
		}
		
		int m_iCheck;
		int m_iSKillState;
		int m_iLastAnim;
		int m_iSlashModelIndex;
	};

	LINK_ENTITY_TO_CLASS(knife_swordbombard, CKnifeSwordBombard)

	void CKnifeSwordBombard::Spawn()
	{
		pev->classname = MAKE_STRING("knife_swordbombard");
		m_iId = WEAPON_KNIFE;
		Precache();
		SET_MODEL(ENT(pev), "models/w_knife.mdl");

		m_iClip = WEAPON_NOCLIP;
		m_iSKillState = 0;
		m_iCheck = 0;
		FallInit();
	}

	void CKnifeSwordBombard::ItemPostFrame()
	{
		return CBasePlayerWeapon::ItemPostFrame();
	}

	void CKnifeSwordBombard::PrimaryAttack(void)
	{
		m_iLastAnim = m_iSwing ? ANIM_SLASH_A1 : ANIM_SLASH_A2;
		SendWeaponAnim(m_iLastAnim, UseDecrement() != FALSE);

		if (m_iSwing)
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/swordbombard_slash_a2.wav", VOL_NORM, ATTN_NORM, 0, 94);
		else
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/swordbombard_slash_a1.wav", VOL_NORM, ATTN_NORM, 0, 94);
		
		OnPrimaryAttack();

		switch (m_iCheck)
		{
			case 1:
			{
				m_iCheck = 2;
				break;
			}
			case 2:
			{

				m_iCheck = 3;
				break;
			}
			case 4:
			{
				m_iCheck = 0;
				SetThink(&CKnifeSwordBombard::ComboAttack);
				pev->nextthink = gpGlobals->time + 0.3s;
				break;
			}
			default:
			{
				m_iCheck = 0;
				break;
			}
			
		}

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.3s;
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.3s;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.32s;

		m_iSwing++;
		m_iSwing %= 2;
	}

	void CKnifeSwordBombard::SecondaryAttack(void)
	{
		SendWeaponAnim(ANIM_SLASH_B, UseDecrement() != FALSE);
		m_iLastAnim = ANIM_SLASH_B;

		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/swordbombard-slash_b.wav", VOL_NORM, ATTN_NORM, 0, 94);

		switch (m_iCheck)
		{
		case 0:
		{
			m_iCheck = 1;
			break;
		}
		case 3:
		{
			m_iCheck = 4;
			break;
		}
		default:
		{
			m_iCheck = 0;
			break;
		}
		}

		SetThink(&CKnifeSwordBombard::DelaySecondaryAttack);
		pev->nextthink = gpGlobals->time + 0.5s;

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0s;
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.1s;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.2s;
	}

	void CKnifeSwordBombard::ComboAttack()
	{
		SetThink(nullptr);

		switch (m_iSKillState)
		{
		case 0:
		{
			SendWeaponAnim(ANIM_SKILL_START, UseDecrement() != FALSE);
			m_iLastAnim = ANIM_SKILL_START;
			m_flNextSecondaryAttack = m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 10.0s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.2s;
			SetThink(&CKnifeSwordBombard::ComboAttack);
			m_iSKillState = 1;
			pev->nextthink = gpGlobals->time + 0.2s;
			break;
		}
		case 1:
		{
			SendWeaponAnim(ANIM_SKILL_LOOP, UseDecrement() != FALSE);
			m_iLastAnim = ANIM_SKILL_LOOP;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 9999.0s;

#ifndef CLIENT_DLL
			UTIL_MakeVectors(m_pPlayer->pev->v_angle);

			Vector VecStart = m_pPlayer->pev->origin + gpGlobals->v_forward * 39.37 * 4;

			CPetrolMine* pMine = (CPetrolMine*)CBaseEntity::Create("petrol_mine", VecStart, Vector(0.0, 0.0, 0.0), pev->owner);

			pMine->m_iTeam = m_iTeam;
			pMine->Attack();
			pMine->m_vecAngle = m_pPlayer->pev->v_angle;
#endif
			m_iSKillState = 2;
			SetThink(&CKnifeSwordBombard::ComboAttack);
			pev->nextthink = gpGlobals->time + 5.0s;
			break;		
		}
		case 2:
		{
			SendWeaponAnim(ANIM_SKILL_END, UseDecrement() != FALSE);
			m_iLastAnim = ANIM_SKILL_END;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;
			m_flNextSecondaryAttack = m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.83s;
			m_iCheck = 0;
			m_iSKillState = 0;
			SetThink(nullptr);
			break;
		}
		}
	}


	void CKnifeSwordBombard::OnPrimaryAttack(void)
	{
		m_iWeaponState = 2;

		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/crow9_slashc_2.wav", VOL_NORM, ATTN_NORM, 0, 94);

#ifndef CLIENT_DLL
		switch (KnifeAttack5(GetPrimaryAttackDamage(), 158, 200, DMG_NEVERGIB | DMG_BULLET, this->pev, m_pPlayer))
		{
		case HIT_NONE:
		{
			break;
		}
		case HIT_PLAYER:
		{
			if (RANDOM_LONG(0, 1))
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/balrog9_hit1.wav", VOL_NORM, ATTN_NORM, 0, 94);
			else
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/balrog9_hit2.wav", VOL_NORM, ATTN_NORM, 0, 94);
			m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;
			break;
		}
		case HIT_WALL:
		{
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_BODY, "weapons/crow9_hitwall1.wav", VOL_NORM, ATTN_NORM, 0, 94);
			m_pPlayer->m_iWeaponVolume = KNIFE_WALLHIT_VOLUME * 0.5;
			break;
		}
		}
#endif
	}

	void CKnifeSwordBombard::DelaySecondaryAttack(void)
	{
#ifndef CLIENT_DLL		
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);

		UTIL_MakeVectors(m_pPlayer->pev->v_angle);

		Vector vecForward = gpGlobals->v_forward;


		Vector vecAngle = UTIL_VecToAngles(vecForward);

		UTIL_TempModel(m_pPlayer->pev->origin, vecAngle, vecForward * 270, m_iSlashModelIndex, 40, 0, 10, 0, 0, 255, kRenderTransAdd, NULL, false, 0, 0, 255, 0);

		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/swordbombard_snd_attack.wav", VOL_NORM, ATTN_NORM, 0, 94);

		switch (KnifeAttack5(GetSecondaryAttackDamage(), 158, 200, DMG_NEVERGIB | DMG_BULLET, this->pev, m_pPlayer))
		{
		case HIT_PLAYER:
		{
			switch (RANDOM_LONG(0, 2))
			{
			case 0: { EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_AUTO, "weapons/swordbombard_slash_hit1.wav", VOL_NORM, ATTN_NORM, 0, 94); break; }
			case 1: { EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_AUTO, "weapons/swordbombard_slash_hit2.wav", VOL_NORM, ATTN_NORM, 0, 94); break; }
			case 2: { EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_AUTO, "weapons/swordbombard_slash_hit3.wav", VOL_NORM, ATTN_NORM, 0, 94); break; }
			}
			break;
		}
		case HIT_WALL:
		{
			break;
		}
		}
#endif

		SetThink(nullptr);
	}

	float CKnifeSwordBombard::GetMaxSpeed(void)
	{
			return 260.0f;
	}

	void CKnifeSwordBombard::Holster(int skiplocal)
	{
		SetThink(nullptr);
		return CBasePlayerWeapon::Holster(skiplocal);
	}

	void CKnifeSwordBombard::WeaponIdle(void)
	{
		ResetEmptySound();
		m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;

		m_iCheck = 0;

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
		SendWeaponAnim(ANIM_IDLE, UseDecrement() != FALSE);
		m_iLastAnim = ANIM_IDLE;
		
	}

}
