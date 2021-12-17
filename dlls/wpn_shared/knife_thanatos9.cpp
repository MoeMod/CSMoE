/*
knife_thanatos9.cpp
Copyright (C) 2021 Itndkl

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

class CKnifeThanatos9: public LinkWeaponTemplate<CKnifeThanatos9,
	TGeneralData,
	BuildTGetItemInfoFromCSW<WEAPON_KNIFE>::template type
>
{
public:
	static constexpr const char *ClassName = "knife_thanatos9";
	static constexpr const char *V_Model = "models/v_thanatos9.mdl";
	static constexpr const char *P_Model = "models/p_thanatos9a.mdl";
	static constexpr const char *W_Model = "models/w_knife.mdl";

	static constexpr int MaxClip = -1;
	static constexpr auto ItemSlot = KNIFE_SLOT;
	static constexpr const char *AnimExtension = "skullaxe";
	KnockbackData KnockBack = { 1.0f, 1.0f, 1.0f, 1.0f, 0.5f };

public:


	enum thanatos9_e
	{
		THANATOS9_DRAWA,
		THANATOS9_SHOOTB_LOOP,
		THANATOS9_SHOOTB_START,
		THANATOS9_SHOOTB_END,
		THANATOS9_IDLEB,
		THANATOS9_IDLEA,
		THANATOS9_DRAWB,
		THANATOS9_SHOOTA1,
		THANATOS9_SHOOTA2,
		THANATOS9_CHANGEA,
		THANATOS9_CHANGEB
	};

	void Precache() override
	{
		Base::Precache();
		PRECACHE_MODEL("models/p_thanatos9b.mdl");

		PRECACHE_SOUND("weapons/thanatos9_shoota1.wav");
		PRECACHE_SOUND("weapons/thanatos9_shoota2.wav");
		PRECACHE_SOUND("weapons/thanatos9_shootb_loop.wav");
		PRECACHE_SOUND("weapons/tomahawk_wall.wav");

	}

	void Spawn() override;
	float GetMaxSpeed() override { return m_fMaxSpeed; }
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	void WeaponIdle() override;
	BOOL CanDrop() override { return false; }

public:
	void DelayPrimaryAttack();
	void DelaySecondaryAttack();
	bool AngularCheck(CBaseEntity* pEntity, CBaseEntity* pAttacker, Vector vecSrc, Vector vecDir, Vector2D vecAngleCheckDir, float flRange, float flMinimumCosine, TraceResult* ptr);

	BOOL CanHolster() override
	{
		if (m_iWeaponState == 2)
			return FALSE;

		return TRUE;
	}

	void Holster(int skiplocal) override
	{
		if (m_iWeaponState != 1)
			m_iWeaponState = 0;

		if (m_pPlayer->pev->weaponanim == THANATOS9_CHANGEA)
		{
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase();
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase();
		}

		m_flThanatos9SmokeTime = invalid_time_point;
		CreateSmoke(0);

		EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "weapons/thanatos9_shootb_loop.wav", 0.0, 0.0, 1 << 5, PITCH_NORM);

		SetThink(NULL);
		pev->nextthink = invalid_time_point;
	}

	void CreateSmoke(int iType)
	{
		//PLAYBACK_EVENT_FULL(0, p->m_pPlayer->edict(), PRECACHE_EVENT(1, "events/wpneffects.sc"), 0.0, (float*)&g_vecZero, (float*)&g_vecZero, iType != -1 ? iType : p->m_flThanatos9SmokeTime, 0.0, iType, 11, m_iWeaponState == 0, m_iState);
	}

	BOOL Deploy() override
	{
		m_fMaxSpeed = 250;	
		m_pPlayer->m_bShieldDrawn = false;

		duration_t tDeployTime = max(m_flNextPrimaryAttack, 0.75s);
		
		m_iSwing = 0;
		if (m_iWeaponState != 0)
			return DefaultDeploy("models/v_thanatos9.mdl", "models/p_thanatos9b.mdl", THANATOS9_DRAWB, "m249", UseDecrement() != FALSE, tDeployTime, 1.53s);
		else
			return DefaultDeploy("models/v_thanatos9.mdl", "models/p_thanatos9a.mdl", THANATOS9_DRAWA, "skullaxe", UseDecrement() != FALSE, tDeployTime, 1.53s);
	}

	float GetChargeAttackDamage() const	
	{
		float flDamage = 45;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 720.0f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage *= 5.5f;
#endif
		return flDamage;
	}
	float GetPrimaryAttackDamage() const
	{
		float flDamage = 110;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 1045.0f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage *= 5.5f;
#endif
		return flDamage;
	}

	int m_iState;
	time_point_t m_flThanatos9SmokeTime;
};

LINK_ENTITY_TO_CLASS(knife_thanatos9, CKnifeThanatos9)

bool CKnifeThanatos9::AngularCheck(CBaseEntity* pEntity, CBaseEntity* pAttacker, Vector vecSrc, Vector vecDir, Vector2D vecAngleCheckDir, float flRange, float flMinimumCosine, TraceResult* ptr)
{
#ifndef CLIENT_DLL
	if (pEntity->IsBSPModel())
	{
		UTIL_TraceLine(vecSrc, vecSrc + vecDir * flRange, ignore_monsters, pAttacker->edict(), ptr);

		if (ptr->flFraction < 1.0)
		{
			if (ptr->pHit)
			{
				CBaseEntity* pHit = CBaseEntity::Instance(ptr->pHit);

				if (pHit && pHit == pEntity)
					return true;
			}
		}
	}
	else
	{
		UTIL_TraceHull(vecSrc, vecSrc + vecDir * flRange, dont_ignore_monsters, head_hull, pAttacker->edict(), ptr);

		if (ptr->flFraction < 1.0 && ptr->pHit != NULL && CBaseEntity::Instance(ptr->pHit) == pEntity)
		{
			UTIL_TraceLine(vecSrc, vecSrc + vecDir * flRange, dont_ignore_monsters, pAttacker->edict(), ptr);

			return true;
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
						return false;

					CBaseEntity* pHit = CBaseEntity::Instance(ptr->pHit);

					if (ptr->flFraction < 1.0 || pHit == pEntity)
						return true;

					if (pHit->IsBSPModel())
						return false;

					vecSrc = ptr->vecEndPos + vecDir * 5.0;
				}
			}
		}
	}
#endif
	return false;
}


void CKnifeThanatos9::Spawn()
{
	pev->classname = MAKE_STRING("knife_thanatos9");
	m_iId = WEAPON_KNIFE;
	Precache();
	SET_MODEL(ENT(pev), "models/w_knife.mdl");
	m_iClip = WEAPON_NOCLIP;
	FallInit();
}

void CKnifeThanatos9::PrimaryAttack(void)
{
	if (!m_iWeaponState)
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
		switch (m_iSwing++ % 2)
		{
		case 0:
			SendWeaponAnim(THANATOS9_SHOOTA1, UseDecrement() != FALSE);

			EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "weapons/thanatos9_shoota1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			break;

		case 1:
			SendWeaponAnim(THANATOS9_SHOOTA2, UseDecrement() != FALSE);

			EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "weapons/thanatos9_shoota2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			break;
		}

		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.4s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.4s;
	}
	else
	{
		SendWeaponAnim(THANATOS9_SHOOTB_START, UseDecrement() != FALSE);

		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 3.0s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.0s;
	}

	m_flThanatos9SmokeTime = invalid_time_point;
	SetThink(&CKnifeThanatos9::DelayPrimaryAttack);

	if(!m_iWeaponState)
		pev->nextthink = gpGlobals->time + 1.0s;
	else
		pev->nextthink = gpGlobals->time + 0.5s;
}

void CKnifeThanatos9::SecondaryAttack(void)
{
	m_iState = 5;

	if(!m_iWeaponState)
	{
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 5.03s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 5.03s;

		SendWeaponAnim(THANATOS9_CHANGEA, UseDecrement() != FALSE);

		SetThink(&CKnifeThanatos9::DelaySecondaryAttack);
		pev->nextthink = m_flThanatos9SmokeTime = gpGlobals->time + 1.0s;
	}
	else
	{
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 3.4s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.4s;

		SendWeaponAnim(THANATOS9_CHANGEB, UseDecrement() != FALSE);

		SetThink(&CKnifeThanatos9::DelaySecondaryAttack);
		pev->nextthink = m_flThanatos9SmokeTime = gpGlobals->time + 0.967s;
	}
}


void CKnifeThanatos9::DelayPrimaryAttack()
{
#ifndef CLIENT_DLL
	if (!m_iWeaponState)
	{
		CBaseEntity* pEntity = NULL;
		BOOL fDidHit = FALSE;
		TraceResult tr;
		UTIL_MakeVectors(m_pPlayer->pev->v_angle);
		Vector vecEnd;
		Vector vecSrc = m_pPlayer->GetGunPosition();

		Vector vecForward = gpGlobals->v_forward;
		Vector2D vecForward2D = vecForward.Make2D().Normalize();

		float c = cos(360.0 * 0.5 * M_PI / 180.0);

		while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, 110.0)) != NULL)
		{
			if (pEntity == m_pPlayer)
				continue;

			if (pEntity->IsDormant())
				continue;

			if (pEntity->pev->takedamage == DAMAGE_NO)
				continue;

			if (!AngularCheck(pEntity, m_pPlayer, vecSrc, vecForward, vecForward2D, 110.0, c, &tr))
				continue;

			float flRatio = 1.0;

			if (pEntity && pEntity->IsPlayer())
			{
				UTIL_MakeVectors(pEntity->pev->angles);

				if (DotProduct((pEntity->pev->origin.Make2D() - m_pPlayer->pev->origin.Make2D()).Normalize(), gpGlobals->v_forward.Make2D()) > 0.8)
					flRatio = 3.0;
			}

			m_pPlayer->m_iWeaponVolume = 128;

			EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_ITEM, "weapons/mastercombat_hit1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

			ClearMultiDamage();

			pEntity->TraceAttack(m_pPlayer->pev, GetPrimaryAttackDamage() * flRatio, (tr.vecEndPos - vecSrc).Normalize(), &tr, DMG_BULLET | DMG_NEVERGIB);

			ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);

			fDidHit = TRUE;
		}

		if (!fDidHit)
		{
			static int calced = 0;
			static float calcRes = 0.0f;

			if (!(calced & 1))
			{
				calced |= 1;
				calcRes = sqrt(512.0);
			}

			vecEnd = vecSrc + vecForward * (110.0 - calcRes);

			UTIL_TraceHull(vecSrc, vecEnd, dont_ignore_monsters, human_hull, m_pPlayer->edict(), &tr);

			if (tr.flFraction < 1.0f)
			{
				EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_ITEM, "weapons/nata_wall.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			}

			SetThink(NULL);
		}
	}
	else if (m_iWeaponState == 3)
	{
		SecondaryAttack();
	}
	else
	{
		Vector vecSrc, vecEnd;
		TraceResult tr;
		CBaseEntity* pEntity = NULL;
		BOOL fDidHit = FALSE;

		if (m_iWeaponState == 1)
		{
			m_iWeaponState = 2;
			m_flThanatos9SmokeTime = gpGlobals->time;

			CreateSmoke(1);

			EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "weapons/thanatos9_shootb_loop.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		}

		UTIL_MakeVectors(m_pPlayer->pev->v_angle);

		vecSrc = m_pPlayer->GetGunPosition();

		Vector vecForward = gpGlobals->v_forward;
		Vector2D vecForward2D = vecForward.Make2D().Normalize();

		float c = cos(30.0 * 0.5 * M_PI / 180.0);

		while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, 130.0)) != NULL)
		{
			if (pEntity == m_pPlayer)
				continue;

			if (pEntity->IsDormant())
				continue;

			if (pEntity->pev->takedamage == DAMAGE_NO)
				continue;

			if (!AngularCheck(pEntity, m_pPlayer, vecSrc, vecForward, vecForward2D, 130.0, c, &tr))
				continue;

			float flRatio = 1.0;

			if (pEntity && pEntity->IsPlayer())
			{
				UTIL_MakeVectors(pEntity->pev->angles);

				if (DotProduct((pEntity->pev->origin.Make2D() - m_pPlayer->pev->origin.Make2D()).Normalize(), gpGlobals->v_forward.Make2D()) > 0.8)
					flRatio = 3.0;
			}

			m_pPlayer->m_iWeaponVolume = 128;

			ClearMultiDamage();

			pEntity->TraceAttack(m_pPlayer->pev, GetChargeAttackDamage() * flRatio, (tr.vecEndPos - vecSrc).Normalize(), &tr, DMG_BULLET | DMG_NEVERGIB);

			ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);

			fDidHit = TRUE;
		}

		if (!fDidHit)
		{
			static int calced = 0;
			static float calcRes = 0.0f;

			if (!(calced & 1))
			{
				calced |= 1;
				calcRes = sqrt(512.0);
			}

			vecEnd = vecSrc + vecForward * (110.0 - calcRes);

			UTIL_TraceHull(vecSrc, vecEnd, dont_ignore_monsters, human_hull, m_pPlayer->edict(), &tr);

			if (tr.flFraction < 1.0f)
			{
				EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_ITEM, "weapons/tomahawk_wall.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			}
		}
		else
			EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_ITEM, "weapons/mastercombat_hit1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

		if (gpGlobals->time < m_flThanatos9SmokeTime + 2.8s)
		{
			m_iState = 0;

			CreateSmoke(-1);

			SendWeaponAnim(THANATOS9_SHOOTB_LOOP, UseDecrement() != FALSE);
			pev->nextthink = gpGlobals->time + 0.2s;

			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 3.0s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.0s;
		}
		else
		{
			SendWeaponAnim(THANATOS9_SHOOTB_END, UseDecrement() != FALSE);

			m_iWeaponState = 3;
			pev->nextthink = gpGlobals->time + 0.7s;

			m_flThanatos9SmokeTime = invalid_time_point;
			CreateSmoke(0);

			EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "weapons/thanatos9_shootb_loop.wav", 0.0, 0.0, 1 << 5, PITCH_NORM);

			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 4.09s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 4.09s;
		}

	}
#endif
}

void CKnifeThanatos9::DelaySecondaryAttack()
{
	if (m_iState <= 0)
	{
		if (m_iWeaponState)
		{
			Q_strcpy(m_pPlayer->m_szAnimExtention, "skullaxe");

			m_pPlayer->pev->weaponmodel = MAKE_STRING("models/p_thanatos9a.mdl");
			m_iWeaponState = 0;
		}
		else
		{
			Q_strcpy(m_pPlayer->m_szAnimExtention, "m249");

			m_pPlayer->pev->weaponmodel = MAKE_STRING("models/p_thanatos9b.mdl");
			m_iWeaponState = 1;
		}

		m_flThanatos9SmokeTime = invalid_time_point;
		SetThink(NULL);

		return;
	}

	if (m_iWeaponState)
	{
		int frames[] = { 29, 63, 68, 75, 80, 82 };

		CreateSmoke(-1);

		pev->nextthink = m_flThanatos9SmokeTime + duration_t((frames[6 - m_iState--] - 29) / 30.0);
	}
	else
	{
		int frames[] = { 30, 48, 60, 70, 76, 136 };

		CreateSmoke(-1);

		pev->nextthink = m_flThanatos9SmokeTime + duration_t((frames[6 - m_iState--] - 30) / 30.0);
	}
}

void CKnifeThanatos9::WeaponIdle()
{
	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	SendWeaponAnim(m_iWeaponState ? THANATOS9_IDLEB : THANATOS9_IDLEA, UseDecrement() != FALSE);
}

}
