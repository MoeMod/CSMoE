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

#define SWORD_TIME	 5.2s

	class CSwordBombardTouchBox : public CBaseAnimating
	{
	public:
		void Spawn(void);
		int GetWeaponsId(void) { return WEAPON_KNIFE; }
		float GetArmorRatioModifier() { return ::sv::GetArmorRatioModifier(WEAPON_KNIFE); }

		void Precache()
		{
			PRECACHE_MODEL("models/ef_swordbombard_potal.mdl");
			m_iBigSwordIndex = PRECACHE_MODEL("models/ef_swordbombard_big.mdl");
			m_iSwordIndex = PRECACHE_MODEL("models/ef_swordbombard.mdl");

			PRECACHE_SOUND("weapons/swordbombard_skill_fx.wav");
			PRECACHE_SOUND("weapons/swordbombard_fly_last.wav");
			PRECACHE_SOUND("weapons/swordbombard_fly1.wav");
			PRECACHE_SOUND("weapons/swordbombard_fly2.wav");
			PRECACHE_SOUND("weapons/swordbombard_fly3.wav");
			PRECACHE_SOUND("weapons/swordbombard_fly4.wav");
			PRECACHE_SOUND("weapons/swordbombard_fly5.wav");
		};

		KnockbackData GetKnockBackData() override
		{
			if (pev->iuser1)
				return { 5000.0f, 500.0f, 300.0f, 200.0f, 0.6f };
			else
				return { 0.0f, 0.0f, 0.0f, 0.0f, 0.6f };
		}


	public:
		void EXPORT Think(void);

	public:
		void Attack(bool bExplode);
		void AttackSound();

		void Remove()
		{
			MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, this->edict());
			WRITE_BYTE(79);
			WRITE_SHORT(this->entindex());
			WRITE_SHORT(CHAN_ITEM);
			WRITE_STRING("weapons/swordbombard_skill_fx.wav");
			MESSAGE_END();

			SetThink(NULL);
			SetTouch(NULL);

			UTIL_Remove(this);

		}
	public:
		
		time_point_t m_flDamageTime;
		time_point_t m_fLaunchTime;
		Vector m_vecAngle;
		Vector m_vecBoxOrigin;
		int m_iSwordIndex;
		int m_iBigSwordIndex;

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
			float flDamage = 15.0f;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 4600.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 10300.0f;
#endif
			return flDamage;
		}
	};

	LINK_ENTITY_TO_CLASS(swordbombard_touchbox, CSwordBombardTouchBox)


	void CSwordBombardTouchBox::Think(void)
	{
		if (pev->owner)
		{
			CBasePlayer* pOwner = (CBasePlayer*)CBaseEntity::Instance(pev->owner);

			if (pOwner)
			{
				if (pOwner->IsPlayer() && (pOwner->m_bIsZombie || !pOwner->IsAlive()))
					Remove();
			}
		}
		else
			Remove();

		if (gpGlobals->time >= m_fLaunchTime + SWORD_TIME)
		{
			Attack(true);
			return;
		}

		pev->nextthink = gpGlobals->time + 0.1s;

		if (m_flDamageTime < gpGlobals->time)
		{
			m_flDamageTime = gpGlobals->time + 0.145s;
			Attack(false);
		}

		UTIL_MakeVectors(m_vecAngle);

		Vector vecForward = gpGlobals->v_forward;
		//vecForward.z = 0;

		Vector vecAngle = m_vecAngle;

		vecAngle = UTIL_VecToAngles(vecForward);

		//vecAngle.x = 0;

		Vector vecStartOrigin = pev->origin;
		vecStartOrigin.z += RANDOM_FLOAT(-75, 75);
		vecStartOrigin.x += RANDOM_FLOAT(-75, 75);
		vecStartOrigin.y += RANDOM_FLOAT(-75, 75);

		UTIL_TempModel(vecStartOrigin, vecAngle, vecForward * 700, m_iSwordIndex, 5, 0, 10, 0, 0, 255, kRenderTransAdd, NULL, false, 0, 0, 255, 0);

		AttackSound();
		
	}

	void CSwordBombardTouchBox::AttackSound()
	{
		std::random_device rd;
		std::size_t N = std::uniform_int_distribution<std::size_t>(0, 4)(rd);
		static constexpr const char* sound[] = {
			"weapons/swordbombard_fly1.wav",
			"weapons/swordbombard_fly2.wav",
			"weapons/swordbombard_fly3.wav",
			"weapons/swordbombard_fly4.wav",
			"weapons/swordbombard_fly5.wav",
		};
		EMIT_SOUND_DYN(this->edict(), CHAN_WEAPON, sound[N], VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
	}

	void CSwordBombardTouchBox::Attack(bool bExplode)
	{
		TraceResult tr;
		pev->iuser1 = bExplode ? 1 : 0;
		float flDamage = bExplode ? GetSlashDamage() : GetRadiusDamage();
		CBaseEntity* pList[100];
		Vector vecStart;

		vecStart = m_vecBoxOrigin + Vector(0.0, 0.0, 1.0);

		const float flWidth = 17 * 39.37;
		const float flLength = 6 * 39.37 / 2.0;
		const float flHeight = 4 * 39.37 / 2.0;
		//int iCount = UTIL_EntitiesInBox(pList, 100, vecStart - Vector(flLength, flWidth, 0.0), vecStart + Vector(flLength, flWidth, flHeight), FL_CLIENT | FL_MONSTER);

		int iCount = UTIL_EntitiesInRotatedBox(pList, 100, vecStart, m_vecAngle, flLength, flWidth, flHeight, FL_CLIENT | FL_MONSTER);

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

			if (pEntity->IsPlayer())
			{
				CBasePlayer* player = (CBasePlayer*)pEntity;

				if (player->m_bIsZombie)
				{
					auto strategy = dynamic_cast<CPlayerModStrategy_ZB1*>(player->m_pModStrategy.get());
					if (strategy)
					{
						strategy->SetStunSpeedTime(1.1s, 140.0f);
						strategy->SetStunGravityTime(1.1s);
					}
				}
			}
		}

		if (bExplode)
		{
			UTIL_MakeVectors(m_vecAngle);

			Vector vecForward = gpGlobals->v_forward;
			//vecForward.z = 0;

			Vector vecAngle = m_vecAngle;

			vecAngle = UTIL_VecToAngles(vecForward);

			Vector vecStartOrigin = pev->origin;

			UTIL_TempModel(vecStartOrigin, vecAngle, vecForward * 700, m_iBigSwordIndex, 7, 0, 10, 0, 0, 255, kRenderTransAdd, NULL, false, 0, 0, 255, 0);

			EMIT_SOUND_DYN(this->edict(), CHAN_WEAPON, "weapons/swordbombard_fly_last.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

			SetThink(NULL);
			SetTouch(NULL);

			UTIL_Remove(this);
		}
	}

	void CSwordBombardTouchBox::Spawn(void)
	{
		pev->movetype = MOVETYPE_NOCLIP;
		pev->solid = SOLID_NOT;
		pev->friction = 0.0;
		pev->takedamage = DAMAGE_NO;
		pev->effects |= EF_NOCULL;


		Precache();

		SET_MODEL(ENT(pev), "models/ef_swordbombard_potal.mdl");
		UTIL_SetSize(pev, Vector(0.0, 0.0, 0.0), Vector(0.0, 0.0, 0.0));
		UTIL_SetOrigin(pev, pev->origin);

		pev->frame = 0;

		ResetSequenceInfo();

		EMIT_SOUND_DYN(this->edict(), CHAN_ITEM, "weapons/swordbombard_skill_fx.wav", VOL_NORM, ATTN_NORM, 0, 94);

		SetThink(&CSwordBombardTouchBox::Think);
		pev->nextthink = gpGlobals->time + 0.3s;

		SetTouch(NULL);
		m_fLaunchTime = gpGlobals->time;
		m_flDamageTime = gpGlobals->time;
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
		static constexpr float ArmorRatioModifier = 1.7f;
		static constexpr const char* AnimExtension = "sfsword";
		KnockbackData KnockBack = { 0.0f, 0.0f, 0.0f, 0.0f, 0.3f };

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

			PRECACHE_MODEL("sprites/muzzleflash268.spr");
			PRECACHE_MODEL("sprites/muzzleflash269.spr");
			
			PRECACHE_SOUND("weapons/swordbombard_slash_a1.wav");
			PRECACHE_SOUND("weapons/swordbombard_slash_a2.wav");
			PRECACHE_SOUND("weapons/swordbombard-slash_b.wav");

			PRECACHE_SOUND("weapons/mastercombat_wall.wav");
			PRECACHE_SOUND("weapons/swordbombard_skill.wav");
			PRECACHE_SOUND("weapons/swordbombard_skill_loop.wav");
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

		KnockbackData GetKnockBackData() override
		{
			if (pev->iuser1)
				return { 0.0f, 0.0f, 0.0f, 0.0f, 0.6f };
			else
				return { 0.0f, 0.0f, 0.0f, 0.0f, 0.3f };
		}


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
			float flDamage = 50.0f;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 2581.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 10400.0f;
#endif
			return flDamage;
		}
#ifndef CLIENT_DLL
		EntityHandle<CSwordBombardTouchBox> m_pTouchBox;
#endif
		int m_iCheck;
		int m_iSKillState;
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

	void CKnifeSwordBombard::PrimaryAttack(void)
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
		SendWeaponAnim(m_iSwing ? ANIM_SLASH_A1 : ANIM_SLASH_A2, UseDecrement() != FALSE);

		if (m_iSwing)
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/swordbombard_slash_a2.wav", VOL_NORM, ATTN_NORM, 0, 94);
		else
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/swordbombard_slash_a1.wav", VOL_NORM, ATTN_NORM, 0, 94);
		
		OnPrimaryAttack();

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.3s;
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.3s;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.32s;

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
				m_iSKillState = 0;
				m_iCheck = 0;
				SetThink(&CKnifeSwordBombard::ComboAttack);
				pev->nextthink = gpGlobals->time + 0.3s;
				m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 5.0s;		
				break;
			}
			default:
			{
				m_iCheck = 0;
				break;
			}
			
		}

		
		m_iSwing++;
		m_iSwing %= 2;
	}

	void CKnifeSwordBombard::SecondaryAttack(void)
	{
		m_iSwing = 0;
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
		SendWeaponAnim(ANIM_SLASH_B, UseDecrement() != FALSE);

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


			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/swordbombard_skill.wav", VOL_NORM, ATTN_NORM, 0, 94);

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

			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 9999.0s;

#ifndef CLIENT_DLL
			UTIL_MakeVectors(m_pPlayer->pev->v_angle);

			Vector vecForward = gpGlobals->v_forward;
			//vecForward.z = 0;

			Vector VecOrigin = m_pPlayer->pev->origin - vecForward * 39.37 * 2;

			Vector vecAngle = UTIL_VecToAngles(vecForward);

			Vector VecStart = m_pPlayer->pev->origin + vecForward * 39.37 * 4;

			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/swordbombard_skill_loop.wav", VOL_NORM, ATTN_NORM, 0, 94);

			m_pTouchBox = (CSwordBombardTouchBox*)CBaseEntity::Create("swordbombard_touchbox", VecOrigin, vecAngle, pev->owner);

			m_pTouchBox->m_vecBoxOrigin = VecOrigin;
			m_pTouchBox->m_iTeam = m_iTeam;
			m_pTouchBox->Attack(false);
			m_pTouchBox->m_vecAngle = m_pPlayer->pev->v_angle;
#endif
			m_iSKillState = 2;
			SetThink(&CKnifeSwordBombard::ComboAttack);
			pev->nextthink = gpGlobals->time + 5.0s;
			break;		
		}
		case 2:
		{
			SendWeaponAnim(ANIM_SKILL_END, UseDecrement() != FALSE);

			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;
			m_flNextSecondaryAttack = m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.83s;
			m_iCheck = 0;
			m_iSKillState = 0;
			break;
		}
		}
	}


	void CKnifeSwordBombard::OnPrimaryAttack(void)
	{
		pev->iuser1 = 0;
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
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/mastercombat_wall.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			m_pPlayer->m_iWeaponVolume = KNIFE_WALLHIT_VOLUME * 0.5;
			break;
		}
		}
#endif
	}

	void CKnifeSwordBombard::DelaySecondaryAttack(void)
	{
		pev->iuser1 = 1;
#ifndef CLIENT_DLL		
		UTIL_MakeVectors(m_pPlayer->pev->v_angle);

		Vector vecForward = gpGlobals->v_forward;

		Vector vecAngle = UTIL_VecToAngles(vecForward);

		UTIL_TempModel(m_pPlayer->pev->origin, vecAngle, vecForward * 500, m_iSlashModelIndex, 5, 0, 10, 0, 0, 255, kRenderTransAdd, NULL, false, 0, 0, 255, 0);

		TraceResult tr;
		float flDamage = GetSecondaryAttackDamage();
		CBaseEntity* pList[100];

		Vector vecStart = m_pPlayer->GetGunPosition();

		const float flWidth = 8 * 39.37;
		const float flLength = 5 * 39.37 / 2.0;
		const float flHeight = 3 * 39.37 / 2.0;
	
		int iCount = UTIL_EntitiesInRotatedBox(pList, 100, vecStart, m_pPlayer->pev->v_angle, flLength, flWidth, flHeight, FL_CLIENT | FL_MONSTER);

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

			pEntity->m_LastHitGroup = HITGROUP_CHEST;
			ClearMultiDamage();
			pEntity->TraceAttack(m_pPlayer->pev, flDamage, (tr.vecEndPos - vecStart).Normalize(), &tr, DMG_BULLET);
			ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);

			if (pEntity->IsPlayer())
			{
				CBasePlayer* player = (CBasePlayer*)pEntity;

				if (player->m_bIsZombie)
				{
					auto strategy = dynamic_cast<CPlayerModStrategy_ZB1*>(player->m_pModStrategy.get());
					if (strategy)
					{
						strategy->SetStunSpeedTime(1.1s, 140.0f);
						strategy->SetStunGravityTime(1.1s);
					}
				}
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
#ifndef CLIENT_DLL
		if (m_pTouchBox)
			m_pTouchBox->Attack(true);
#endif

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

		
	}

}
