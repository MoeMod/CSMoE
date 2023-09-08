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
	TPrecacheEvent,
	BuildTGetItemInfoFromCSW<WEAPON_KNIFE>::template type
>
{
public:
	static constexpr const char *ClassName = "knife_vulcanus9";
	static constexpr const char *V_Model = "models/v_vulcanus9.mdl";
	static constexpr const char *P_Model = "models/p_vulcanus9.mdl";
	static constexpr const char* P_Model_1 = "models/p_vulcanus92.mdl";
	static constexpr const char *W_Model = "models/w_knife.mdl";
	static constexpr const char* EventFile = "events/vulcanus9.sc";

	static constexpr int MaxClip = -1;
	static constexpr auto ItemSlot = KNIFE_SLOT;
	static constexpr const char *AnimExtension = "vulcanus9";
	KnockbackData KnockBack = { 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };

public:

#define WPNSTATE_VULCANUS9_MODEB (1<<0)
#define VULCANUS9_MAXAMMO	10
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
		
		PRECACHE_MODEL("sprites/flame3.spr");
		PRECACHE_MODEL("models/p_vulcanus92.mdl");

		PRECACHE_SOUND("weapons/vulcanus9_stab1.wav");
		PRECACHE_SOUND("weapons/vulcanus9_stab_miss.wav");
		PRECACHE_SOUND("weapons/vulcanus9_off_slash1.wav");
		PRECACHE_SOUND("weapons/vulcanus9_off_slash2.wav");
		PRECACHE_SOUND("weapons/vulcanus9_idle.wav");
		PRECACHE_SOUND("weapons/tomahawk_wall.wav");
		PRECACHE_SOUND("weapons/tomahawk_slash1_hit.wav");
	}

	BOOL Deploy() override
	{
		m_fMaxSpeed = 250;
		m_iSwing = 0;
		m_iWeaponState = 0;

		if (m_flTimeholster != invalid_time_point)
		{
			m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] += (gpGlobals->time - m_flTimeholster) / 2.0s;
		}
		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] >= VULCANUS9_MAXAMMO)
			m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] = VULCANUS9_MAXAMMO;
			
		SetSkin();

		PLAYBACK_EVENT_FULL(FEV_GLOBAL, ENT(m_pPlayer->pev), m_usFire, 0, g_vecZero, g_vecZero, 0.0, 0.0, 0, 0, FALSE, TRUE);

		return DefaultDeploy(V_Model, P_Model, ANIM_OFF_DRAW, "katana", UseDecrement() != FALSE);

	}

	void Spawn() override;
	float GetMaxSpeed() override { return m_fMaxSpeed; }
	void WeaponIdle();
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	int GetItemInfo(ItemInfo* p) override;
	BOOL CanDrop() override { return false; }

public:
#ifndef CLIENT_DLL
	hit_result_t KnifeAttack(float flDamage, float flRadius, float flAngleDegrees, int bitsDamageType, entvars_t* pevInflictor, CBasePlayer* pAttackPlayer);
#endif
	void OffPrimaryAttack();
	void OnPrimaryAttack();
	void SetSkin();
	float GetPrimaryAttackDamage() const
	{
		float flDamage = 65.0f;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 715.0f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 715.0f;
#endif
		return flDamage;
	}

	float GetSecondaryAttackDamage() const
	{
		bool bFireOn = m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] > 0 ? true : false;

		float flDamage = bFireOn ? 120.0f : 80.0f;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = bFireOn ? 1320.0f : 880.0f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = bFireOn ? 1320.0f : 880.0f;
#endif
		return flDamage;
	}

	float GetBuffDamage() const
	{
		float flDamage = 5.0f;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 50.0f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 50.0f;
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
	p->iMaxAmmo1 = VULCANUS9_MAXAMMO;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = VULCANUS9_MAXAMMO;
	p->iSlot = 2;
	p->iPosition = 1;
	p->iId = WEAPON_KNIFE;
	p->iFlags = 0;
	p->iWeight = KNIFE_WEIGHT;

	return 1;
}

void CKnifeVulcanus9::Spawn()
{
	m_iKnifeID = KNIFE_VULCANUS9;
	pev->classname = MAKE_STRING("knife_vulcanus9");
	m_iId = WEAPON_KNIFE;
	Precache();
	SET_MODEL(ENT(pev), "models/w_knife.mdl");
	m_iClip = WEAPON_NOCLIP;
	m_flTimeholster = gpGlobals->time;

	m_iDefaultAmmo = VULCANUS9_MAXAMMO;
	FallInit();
}

void CKnifeVulcanus9::PrimaryAttack(void)
{
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

	if (m_iWeaponState & WPNSTATE_VULCANUS9_MODEB)
		OnPrimaryAttack();
	else
		OffPrimaryAttack();

}

void CKnifeVulcanus9::SecondaryAttack(void)
{
	if (m_iWeaponState & WPNSTATE_VULCANUS9_MODEB)
	{
		m_iWeaponState &= ~WPNSTATE_VULCANUS9_MODEB;

		SendWeaponAnim(ANIM_OFF, UseDecrement() != FALSE);

		m_pPlayer->m_flNextAttack = 2.33s;
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 2.33s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.5s;

		strcpy(m_pPlayer->m_szAnimExtention, "katana");

		m_pPlayer->pev->weaponmodel = MAKE_STRING(P_Model);
	}
	else
	{
		m_iWeaponState |= WPNSTATE_VULCANUS9_MODEB;
		SendWeaponAnim(ANIM_ON, UseDecrement() != FALSE);

		m_pPlayer->m_flNextAttack = 2.4s;
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 2.4s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.5s;

		strcpy(m_pPlayer->m_szAnimExtention, "sfsword");

		m_pPlayer->pev->weaponmodel = MAKE_STRING(P_Model_1);
	}
}


void CKnifeVulcanus9::OnPrimaryAttack()
{
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.7s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;
	SendWeaponAnim(m_iSwing++ % 4 ? ANIM_MIDSLASH1 : ANIM_MIDSLASH2, UseDecrement() != FALSE);
#ifndef CLIENT_DLL
	bool bFireOn = m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] > 0 ? true : false;

	UTIL_MakeVectors(m_pPlayer->pev->v_angle);

	switch (KnifeAttack(GetSecondaryAttackDamage(), 90, 100, DMG_NEVERGIB | DMG_BULLET, m_pPlayer->pev, m_pPlayer))
	{
	case HIT_NONE:
	{
		if (m_iSwing & 1)
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/vulcanus9_off_slash1.wav", VOL_NORM, ATTN_NORM, 0, 94);
		else
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/vulcanus9_off_slash2.wav", VOL_NORM, ATTN_NORM, 0, 94);

		if(!RANDOM_LONG(0,4))
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/vulcanus9_stab_miss.wav", VOL_NORM, ATTN_NORM, 0, 94);

		break;
	}
	case HIT_PLAYER:
	{
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.5s;
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/tomahawk_slash1_hit.wav", VOL_NORM, ATTN_NORM, 0, 94);

		if (!RANDOM_LONG(0, 4))
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/vulcanus9_stab_miss.wav", VOL_NORM, ATTN_NORM, 0, 94);

		m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;
		break;
	}
	case HIT_WALL:
	{
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.5s;
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/tomahawk_wall.wav", VOL_NORM, ATTN_NORM, 0, 94);
		m_pPlayer->m_iWeaponVolume = KNIFE_WALLHIT_VOLUME * 0.5;
		break;
	}
	}
#endif
}

void CKnifeVulcanus9::OffPrimaryAttack()
{
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.2s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;
	SendWeaponAnim(m_iSwing++ % 2 ? ANIM_OFF_SLASH2 : ANIM_OFF_SLASH1, UseDecrement() != FALSE);
#ifndef CLIENT_DLL
	UTIL_MakeVectors(m_pPlayer->pev->v_angle);

	switch (sv::KnifeAttack(m_pPlayer->GetGunPosition(), gpGlobals->v_forward, GetPrimaryAttackDamage(), 70, DMG_NEVERGIB | DMG_BULLET, m_pPlayer->pev, m_pPlayer->pev))
	{
	case HIT_NONE:
	{
		if(m_iSwing & 1)
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/vulcanus9_off_slash1.wav", VOL_NORM, ATTN_NORM, 0, 94);
		else
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/vulcanus9_off_slash2.wav", VOL_NORM, ATTN_NORM, 0, 94);
		break;
	}
	case HIT_PLAYER:
	{
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0s;
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/tomahawk_slash1_hit.wav", VOL_NORM, ATTN_NORM, 0, 94);
		m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;
		break;
	}
	case HIT_WALL:
	{
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0s;
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/tomahawk_wall.wav", VOL_NORM, ATTN_NORM, 0, 94);
		m_pPlayer->m_iWeaponVolume = KNIFE_WALLHIT_VOLUME * 0.5;
		break;
	}
	}
#endif
}

#ifndef CLIENT_DLL
hit_result_t CKnifeVulcanus9::KnifeAttack(float flDamage, float flRadius, float flAngleDegrees, int bitsDamageType, entvars_t* pevInflictor, CBasePlayer* pAttackPlayer)
{
	bool bFireOn = m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] > 0 ? true : false;

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

		if (bFireOn && (pEntity->IsPlayer() || pEntity->Classify() == CLASS_PLAYER_ALLY) && (g_pGameRules->PlayerRelationship(m_pPlayer, pEntity) != GR_TEAMMATE))
		{	
			float flBurnAmount = GetBuffDamage();

			pEntity->m_pBuffAttacker = m_pPlayer->pev;
			pEntity->m_iHealthDecreaseCount = 5;
			pEntity->m_flHealthDecreaseAmount = flBurnAmount;
			pEntity->m_flHealthDecreaseInterval = 1.0s;

			MESSAGE_BEGIN(MSG_ALL, gmsgMPToCL, NULL, pEntity->pev);
			WRITE_BYTE(4);
			WRITE_SHORT(pEntity->entindex());
			WRITE_BYTE(18);
			MESSAGE_END();
		}

		result = HIT_PLAYER;
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


void CKnifeVulcanus9::SetSkin()
{
	float flRatio = float(m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]) / float(VULCANUS9_MAXAMMO);

	if (flRatio >= 0.8)
		pev->skin = 4;
	else if (flRatio >= 0.6)
		pev->skin = 3;
	else if (flRatio >= 0.4)
		pev->skin = 2;
	else if (flRatio >= 0.2)
		pev->skin = 1;
	else
		pev->skin = 0;
}

void CKnifeVulcanus9::Holster(int skiplocal)
{
	m_flTimeholster = gpGlobals->time;

	PLAYBACK_EVENT_FULL(FEV_GLOBAL, ENT(m_pPlayer->pev), m_usFire, 0, g_vecZero, g_vecZero, 0.0, 0.0, 0, 0, FALSE, TRUE);

	return CBasePlayerWeapon::Holster(skiplocal);
}

void CKnifeVulcanus9::ItemPostFrame()
{
#ifndef CLIENT_DLL
	if (m_iWeaponState & WPNSTATE_VULCANUS9_MODEB)
	{
		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] > 0 && m_flTimeNextCharge <= gpGlobals->time)
		{
			m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;
			
			if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			{
				m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] = 0;

				MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->edict());
				WRITE_BYTE(79);
				WRITE_SHORT(m_pPlayer->entindex());
				WRITE_SHORT(CHAN_ITEM);
				WRITE_STRING("weapons/vulcanus9_idle.wav");
				MESSAGE_END();

				PLAYBACK_EVENT_FULL(FEV_GLOBAL, ENT(m_pPlayer->pev), m_usFire, 0, g_vecZero, g_vecZero, 0.0, 0.0, 0, 0, FALSE, TRUE);
			}
			else
			{
				PLAYBACK_EVENT_FULL(FEV_GLOBAL, ENT(m_pPlayer->pev), m_usFire, 0, g_vecZero, g_vecZero, 0.0, 0.0, 0, 0, TRUE, TRUE);

				m_flTimeNextCharge = gpGlobals->time + 1.0s;
			}

			SetSkin();
		}
	}
	else
	{
		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] < VULCANUS9_MAXAMMO)
		{
			if (m_flTimeNextCharge <= gpGlobals->time)
			{
				m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]++;
				m_flTimeNextCharge = gpGlobals->time + 2.0s;
				SetSkin();
			}
		}
	}
#endif
	return CBasePlayerWeapon::ItemPostFrame();
}

void CKnifeVulcanus9::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
	SendWeaponAnim(m_iWeaponState ? ANIM_IDLE :  ANIM_OFF_IDLE, UseDecrement() != FALSE);
}


}
