/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "wpn_sfsmg.h"
#include "weapons/RadiusDamage.hpp"
#ifndef CLIENT_DLL
#include "gamemode/mod_zb1.h"
#include "gamemode/mods.h"
#endif


#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

#ifndef CLIENT_DLL
	class CEF_Y23S1SFSMG : public CBaseAnimating
	{
	public:

		void Spawn() override
		{
			Precache();

			SetThink(&CEF_Y23S1SFSMG::OnThink);
			
			SET_MODEL(this->edict(), "models/ef_y23s1sfsmg.mdl");

			pev->rendermode = kRenderNormal;
			pev->renderfx = kRenderFxNone;
			pev->renderamt = 255;
			pev->solid = SOLID_NOT;
			pev->movetype = MOVETYPE_NONE;

			pev->frame = 0;
			pev->sequence = 0;

			ResetSequenceInfo();

			UTIL_SetSize(pev, g_vecZero, g_vecZero);

			pev->nextthink = gpGlobals->time + 0.06s;
		}

		void Precache() override
		{
			PRECACHE_MODEL("models/ef_y23s1sfsmg.mdl");
		}

		void SetOrigin(Vector origin)
		{
			pev->origin = origin;
		}

		void RemoveEnt()
		{
			SetThink(nullptr);
			UTIL_Remove(this);
		}

		void EXPORT OnThink()
		{
			pev->nextthink = gpGlobals->time + 0.6s;
		}
	};

	LINK_ENTITY_TO_CLASS(ef_y23s1sfsmg, CEF_Y23S1SFSMG)
#endif

LINK_ENTITY_TO_CLASS(weapon_y23s1sfsmg, CY23S1SFSMG)

enum y23s1sfsmg_e
{
	ANIM_IDLE1,
	ANIM_RELOAD,
	ANIM_DRAW,
	ANIM_SHOOT1,
	ANIM_SHOOT2,
	ANIM_SHOOTB,
	ANIM_SHOOTB_END,
};

#define Y23S1SFSMG_MAX_AMMO2	90

void CY23S1SFSMG::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_y23s1sfsmg");

	Precache();
	m_iId = WEAPON_Y23S1SFSMG;
	SET_MODEL(ENT(pev), "models/w_y23s1sfsmg.mdl");

	m_iDefaultAmmo = 45;
	m_iDefaultAmmo2 = 0;
	m_flAccuracy = 0.15;
	m_bDelayFire = false;

	FallInit();
}

void CY23S1SFSMG::Precache(void)
{
	PRECACHE_MODEL("models/v_y23s1sfsmg.mdl");
	PRECACHE_MODEL("models/w_y23s1sfsmg.mdl");
	PRECACHE_MODEL("models/p_y23s1sfsmg.mdl");

	PRECACHE_SOUND("weapons/y23s1sfsmg-1.wav");
	
	PRECACHE_MODEL("sprites/muzzleflash288.spr");
	PRECACHE_MODEL("sprites/muzzleflash289.spr");
	PRECACHE_MODEL("sprites/ef_y23s1sfsmg_laser.spr");
	PRECACHE_MODEL("sprites/ef_y23s1sfsmg_laser_hit.spr");

	m_iShell = PRECACHE_MODEL("models/pshell.mdl");
	m_usFireY23S1SFSMG = PRECACHE_EVENT(1, "events/y23s1sfsmg.sc");
}

int CY23S1SFSMG::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "45acp";
	p->iMaxAmmo1 = MAX_AMMO_45ACP;
	p->pszAmmo2 = "y23s1sfsmg";
	p->iMaxAmmo2 = Y23S1SFSMG_MAX_AMMO2;
	p->iMaxClip = 45;
	p->iSlot = 0;
	p->iPosition = 13;
	p->iId = m_iId = WEAPON_Y23S1SFSMG;
	p->iFlags = 0;
	p->iWeight = MAC10_WEIGHT;

	return 1;
}

int CY23S1SFSMG::ExtractAmmo(CBasePlayerWeapon* pWeapon)
{
	if (m_iDefaultAmmo2)
	{
		m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = m_iDefaultAmmo2;
		m_iDefaultAmmo2 = 0;
	}
	return CBasePlayerWeapon::ExtractAmmo(pWeapon);
}

BOOL CY23S1SFSMG::Deploy(void)
{
	m_flAccuracy = 0.1;
	m_bDelayFire = false;

	return DefaultDeploy("models/v_y23s1sfsmg.mdl", "models/p_y23s1sfsmg.mdl", ANIM_DRAW, "rifle", UseDecrement() != FALSE);
}

void CY23S1SFSMG::PrimaryAttack(void)
{
	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		Y23S1SFSMGFire((0.2) * m_flAccuracy, 0.066s, FALSE);
	else if (m_pPlayer->pev->velocity.Length2D() > 170)
		Y23S1SFSMGFire((0.012) * m_flAccuracy, 0.066s, FALSE);
	else
		Y23S1SFSMGFire((0.032) * m_flAccuracy, 0.066s, FALSE);
}

void CY23S1SFSMG::Y23S1SFSMGFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
{
	m_bDelayFire = true;
	m_iShotsFired++;
	m_flAccuracy = ((m_iShotsFired * m_iShotsFired * m_iShotsFired) / 200.0) + 0.6;

	if (m_flAccuracy > 0.7)
		m_flAccuracy = 0.7;

	if (m_iClip <= 0)
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2s;
		}

		return;
	}

	m_iWeaponState = 0;

	if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] < Y23S1SFSMG_MAX_AMMO2)
		m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]++;

	m_iClip--;
	m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = DIM_GUN_FLASH;

	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 1, BULLET_PLAYER_45ACP, GetDamage(), 0.88, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);

	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFireY23S1SFSMG, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), FALSE, FALSE);
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1s;

	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		KickBack(0.9, 0.475, 0.35, 0.0425, 5.0, 3.0, 8);
	else if (m_pPlayer->pev->velocity.Length2D() > 0)
		KickBack(0.34, 0.175, 0.14, 0.03, 3.0, 2.0, 7);
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		KickBack(0.225, 0.15, 0.1, 0.015, 2.0, 1.0, 5);
	else
		KickBack(0.225, 0.15, 0.15, 0.017, 2.25, 1.25, 6);
}

void CY23S1SFSMG::Reload(void)
{
	if (m_pPlayer->ammo_45acp <= 0)
		return;

	if (DefaultReload(45, ANIM_RELOAD, 3.3s))
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
		m_flAccuracy = 0.1;
		m_iShotsFired = 0;
	}
}

void CY23S1SFSMG::SecondaryAttack(void)
{
	if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] <= 0)
	{
		if (m_pPlayer->pev->weaponanim == ANIM_SHOOTB)
		{
			SendWeaponAnim(ANIM_SHOOTB_END, UseDecrement() != FALSE);

#ifndef CLIENT_DLL
			if (m_pEffect)
				m_pEffect->RemoveEnt();
#endif
		}
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.2s;
		return;
	}

	m_iWeaponState = 1;
	m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]--;
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.066s;

	m_pPlayer->m_iWeaponVolume = BIG_EXPLOSION_VOLUME;
	m_pPlayer->m_iWeaponFlash = DIM_GUN_FLASH;

	SendWeaponAnim(ANIM_SHOOTB, UseDecrement() != FALSE);

	UTIL_MakeVectors(m_pPlayer->pev->v_angle);

	Vector vecDir = m_pPlayer->FireBullets3(m_pPlayer->GetGunPosition(), gpGlobals->v_forward, 0.0, 4096, 1, 50, GetDamageB(), 0.88, m_pPlayer->pev, true, m_pPlayer->random_seed);

	int flags;

#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFireY23S1SFSMG, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, int(m_pPlayer->pev->punchangle.x * 100), 0, m_iClip == 0, TRUE);
}

#ifndef CLIENT_DLL
void CY23S1SFSMG::FireBullet3CallBack(int iPenetration, Vector vecDir, TraceResult* pTrace)
{
	if (m_iWeaponState)
	{
		int iOriginalPenetration = 1;

		if (iOriginalPenetration == iPenetration)
		{
			struct
			{
				float RadiusDamageRadius;
				int RadiusDamageBitsDamageType;
				bool RadiusDamageCanHeadshot;
				float RadiusDamageAmount;
			} Data;

			Vector vecSrc = pTrace->vecEndPos;

			if(!m_pEffect)
				m_pEffect = static_cast<CEF_Y23S1SFSMG*>(CBaseEntity::Create("ef_y23s1sfsmg", vecSrc, m_pPlayer->pev->v_angle, ENT(m_pPlayer->pev)));

			m_pEffect->SetOrigin(vecSrc);

			Data.RadiusDamageRadius = 100.0f;
			Data.RadiusDamageBitsDamageType = DMG_BULLET;
			Data.RadiusDamageCanHeadshot = true;
			Data.RadiusDamageAmount = GetDamageC();

			std::vector<CBaseEntity*> out;
			RadiusDamage(Data, vecSrc, m_pPlayer, m_pPlayer, std::back_inserter(out));
			for (CBaseEntity* ent : out)
			{
				CBasePlayer* player = dynamic_ent_cast<CBasePlayer*>(ent);
				if (player && player->m_bIsZombie)
				{
					Vector vecDir = (player->pev->origin - m_pPlayer->pev->origin);

					UTIL_AddKickRateBoost(ent, vecDir.Make2D().Normalize(), 1.0, 250.0f, 0.0f);

					auto strategy = dynamic_cast<CPlayerModStrategy_ZB1*>(player->m_pModStrategy.get());
					if (strategy)
					{
						strategy->SetStunSpeedTime(1.0s, 150);
						strategy->SetStunGravityTime(1.0s);
					}
				}
			}
		}

	}
	
}
#endif


void CY23S1SFSMG::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_pPlayer->pev->weaponanim == ANIM_SHOOTB)
	{
		SendWeaponAnim(ANIM_SHOOTB_END, UseDecrement() != FALSE);

		m_flNextPrimaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.83s;

#ifndef CLIENT_DLL
		if(m_pEffect)
			m_pEffect->RemoveEnt();
#endif
	}

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 60s;

	SendWeaponAnim(ANIM_IDLE1, UseDecrement() != FALSE);
}

float CY23S1SFSMG::GetDamage() const
{
	float flDamage = 20.0f;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flDamage = 121.0f;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 400.0f;
#endif
	return flDamage;
}


float CY23S1SFSMG::GetDamageB() const
{
	float flDamage = 10.0f;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flDamage = 25.0f;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 101.0f;
#endif
	return flDamage;
}


float CY23S1SFSMG::GetDamageC() const
{
	float flDamage = 5.0f;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flDamage = 70.0f;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 283.0f;
#endif
	return flDamage;
}

}
