/*
knife_vulcanus9.cpp
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
#endif

#include "weapons/WeaponTemplate.hpp"
#include "weapons/KnifeAttack.h"

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

class CKnifeVulcanus9: public LinkWeaponTemplate<CKnifeVulcanus9,
	TGeneralData,
	BuildTGetItemInfoFromCSW<WEAPON_KNIFE>::template type
>
{
public:
	static constexpr const char *ClassName = "knife_vulcanus9";
	static constexpr const char *V_Model = "models/v_vulcanus9.mdl";
	static constexpr const char *P_Model = "models/p_vulcanus9.mdl";
	static constexpr const char* P_Model_1 = "models/p_vulcanus92.mdl";
	static constexpr const char *W_Model = "models/w_knife.mdl";

	static constexpr int MaxClip = -1;
	static constexpr auto ItemSlot = KNIFE_SLOT;
	static constexpr const char *AnimExtension = "vulcanus9";
	KnockbackData KnockBack = { 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };

public:

	enum vulcanus9_e
	{
		ANIM_IDLE = 0,
		ANIM_ON,
		ANIM_OFF,
		ANIM_DRAW,
		ANIM_STAB,
		ANIM_STAB_MISS,
		ANIM_MIDSLASH1,
		ANIM_MIDSLASH2,
		ANIM_OFF_DRAW,
		ANIM_OFF_IDLE,
		ANIM_OFF_SLASH1,
		ANIM_OFF_SLASH2,
	};

	void Precache() override
	{
		Base::Precache();
		PRECACHE_MODEL("models/p_vulcanus92.mdl");

		PRECACHE_SOUND("weapons/vulcanus9_stab1.wav");
		PRECACHE_SOUND("weapons/vulcanus9_stab_miss.wav");
		PRECACHE_SOUND("weapons/vulcanus9_off_slash1.wav");
		PRECACHE_SOUND("weapons/vulcanus9_off_slash2.wav");
		PRECACHE_SOUND("weapons/vulcanus9_idle.wav");
	}

	BOOL Deploy() override
	{
		m_fMaxSpeed = 250;
		m_iSwing = 0;
		m_pPlayer->m_bShieldDrawn = false;

		if (m_flTimeholster != invalid_time_point)
		{
			m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] += (gpGlobals->time - m_flTimeholster) / 0.33s;
		}
		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] >= 100)
			m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] = 100;
			
		BOOL bResult;

		if(m_iWeaponState)
		bResult = DefaultDeploy(V_Model, P_Model, ANIM_DRAW, "katana", UseDecrement() != FALSE);

		return bResult;
	}

	void Spawn() override;
	float GetMaxSpeed() override { return m_fMaxSpeed; }
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	int GetItemInfo(ItemInfo* p) override;
	BOOL CanDrop() override { return false; }

public:
#ifndef CLIENT_DLL
	hit_result_t KnifeAttack(float flDamage, float flRadius, float flAngleDegrees, int bitsDamageType, entvars_t* pevInflictor, CBasePlayer* pAttackPlayer, bool IsPrimaryAttack);
#endif
	void OffPrimaryAttack();
	void DelaySecondaryAttack();
	float GetPrimaryAttackDamage() const
	{
		float flDamage = 100;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage *= 20.5f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 9760.0f;
#endif
		return flDamage;
	}

	float GetSecondaryAttackDamage() const
	{
		float flDamage = 100;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage *= 27.5f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 9760.0f;
#endif
		return flDamage;
	}

	float GetEffectAttackDamage() const
	{
		float flDamage = 100;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage *= 35.5f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 9760.0f;
#endif
		return flDamage;
	}

	void Holster(int skiplocal) override;
	int ExtractAmmo(CBasePlayerWeapon* pWeapon) override;
	void ItemPostFrame() override;

	time_point_t m_flTimeNextCharge;
	time_point_t m_flTimeholster;

};

LINK_ENTITY_TO_CLASS(knife_vulcanus9, CKnifeVulcanus9)

int CKnifeVulcanus9::ExtractAmmo(CBasePlayerWeapon* pWeapon)
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

int CKnifeVulcanus9::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "vulcanus9ammo";
	p->iMaxAmmo1 = 10;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = 10;
	p->iSlot = 2;
	p->iPosition = 1;
	p->iId = WEAPON_KNIFE;
	p->iFlags = 0;
	p->iWeight = KNIFE_WEIGHT;

	return 1;
}

void CKnifeVulcanus9::Spawn()
{
	pev->classname = MAKE_STRING("knife_vulcanus9");
	m_iId = WEAPON_KNIFE;
	Precache();
	SET_MODEL(ENT(pev), "models/w_knife.mdl");
	m_iClip = WEAPON_NOCLIP;
	m_flTimeholster = gpGlobals->time;
	m_iDefaultAmmo = 0;
	FallInit();
}

void CKnifeVulcanus9::PrimaryAttack(void)
{
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
	if (m_iWeaponState)
	{

	}
	else
		OffPrimaryAttack();
}

void CKnifeVulcanus9::SecondaryAttack(void)
{
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
	SendWeaponAnim(ANIM_ON, UseDecrement() != FALSE);

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.4s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;
}

void CKnifeVulcanus9::OffPrimaryAttack()
{
	
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.2s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;

	SendWeaponAnim(ANIM_OFF_SLASH1, UseDecrement() != FALSE);

	BOOL fDidHit = FALSE;

#ifndef CLIENT_DLL
	m_iSwing++;
	switch (KnifeAttack(GetPrimaryAttackDamage(), 117, 60, DMG_NEVERGIB | DMG_BULLET, m_pPlayer->pev, m_pPlayer, true))
	{
	case HIT_NONE:
	{
		if(m_iSwing & 1)
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/vulcanus9_slash1.wav", VOL_NORM, ATTN_NORM, 0, 94);
		else
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/vulcanus9_slash2.wav", VOL_NORM, ATTN_NORM, 0, 94);
		break;
	}
	case HIT_PLAYER:
	{
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/vulcanus9_hit.wav", VOL_NORM, ATTN_NORM, 0, 94);
		m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;
		fDidHit = TRUE;
		break;
	}
	case HIT_WALL:
	{
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/vulcanus9_wall.wav", VOL_NORM, ATTN_NORM, 0, 94);
		m_pPlayer->m_iWeaponVolume = KNIFE_WALLHIT_VOLUME * 0.5;
		fDidHit = TRUE;
		break;
	}
	}
#endif
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;
	SetThink(nullptr);

	//return fDidHit;
}

void CKnifeVulcanus9::DelaySecondaryAttack()
{
	BOOL fDidHit = FALSE;
#ifndef CLIENT_DLL
	switch (KnifeAttack(GetSecondaryAttackDamage(), 130, 180, DMG_NEVERGIB | DMG_BULLET, m_pPlayer->pev, m_pPlayer, false))
	{
	case HIT_NONE:
	{
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/vulcanus9_miss.wav", VOL_NORM, ATTN_NORM, 0, 94);
		break;
	}
	case HIT_PLAYER:
	{
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/vulcanus9_hit.wav", VOL_NORM, ATTN_NORM, 0, 94);
		m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;
		fDidHit = TRUE;
		break;
	}
	case HIT_WALL:
	{
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/vulcanus9_wall.wav", VOL_NORM, ATTN_NORM, 0, 94);
		m_pPlayer->m_iWeaponVolume = KNIFE_WALLHIT_VOLUME * 0.1;
		fDidHit = TRUE;
		break;
	}
	}
#endif

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;
	SetThink(nullptr);

	//return fDidHit;
}

#ifndef CLIENT_DLL
hit_result_t CKnifeVulcanus9::KnifeAttack(float flDamage, float flRadius, float flAngleDegrees, int bitsDamageType, entvars_t* pevInflictor, CBasePlayer* pAttackPlayer, bool IsPrimaryAttack)
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
				Vector vecDir = (player->pev->origin - pev->origin);
				vecDir.z = 0;
				if (!IsPrimaryAttack)
				{
					if (!(player->m_iBuff & BUFF_IGNORE_KNOCKBACK))
					{
						Vector2D vecDir2D = (player->pev->origin.Make2D() - m_pPlayer->pev->origin.Make2D()).Normalize();

						player->pev->velocity.x = vecDir2D.x * 600;
						player->pev->velocity.y = vecDir2D.y * 600;
						player->pev->velocity.z = 310;

						player->m_flVelocityModifier = 1.0;
					}
				}
				else
				{
					Vector2D vecDir2D = (player->pev->origin.Make2D() - m_pPlayer->pev->origin.Make2D()).Normalize();

					player->pev->velocity.x = vecDir2D.x * 600;
					player->pev->velocity.y = vecDir2D.y * 600;

					player->m_flVelocityModifier = 1.0;
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

void CKnifeVulcanus9::Holster(int skiplocal)
{
	m_flTimeholster = gpGlobals->time;
	return CBasePlayerWeapon::Holster(skiplocal);
}

void CKnifeVulcanus9::ItemPostFrame()
{
#ifndef CLIENT_DLL
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] < 100)
	{
		if (m_flTimeNextCharge <= gpGlobals->time)
		{
			m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]++;
			m_flTimeNextCharge = gpGlobals->time + 0.3s;
		}
	}
#endif
	return CBasePlayerWeapon::ItemPostFrame();
}


}
