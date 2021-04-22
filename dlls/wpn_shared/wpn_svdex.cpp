/*
wpn_svdex.cpp - CSMoE Gameplay server : Zombie Hero
Copyright (C) 2019 TmNine!~

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
#include "monsters.h"
#include "wpn_svdex.h"

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

#ifndef CLIENT_DLL
	class CSVDEXGrenade : public CBaseEntity
	{
	public:
		void Spawn() override
		{
			Precache();

			m_fSequenceLoops = 0;
			//ph26 = 0;
			SetThink(&CSVDEXGrenade::IgniteThink);
			SetTouch(&CSVDEXGrenade::OnTouch);
			SET_MODEL(this->edict(), "models/shell_svdex.mdl");

			//ph32 = ?
			pev->gravity = 0.58;
			pev->framerate = 10;
			pev->scale = 0.2;
			pev->solid = SOLID_BBOX; // 2
			pev->movetype = MOVETYPE_BOUNCE; // 9
			pev->nextthink = gpGlobals->time + 0.1s;
			m_flRemoveTime = gpGlobals->time + 5s;
			m_flMaxFrames = 300.0;
			UTIL_SetSize(pev, { -4, -4, -4 }, { 4, 4, 4 });

			pev->classname = MAKE_STRING("d_svdex");
		}

		void Precache() override
		{
			PRECACHE_MODEL("models/shell_svdex.mdl");
			m_iSprEffect = PRECACHE_MODEL("sprites/laserbeam.spr");
		}

		KnockbackData GetKnockBackData()
		{
			return { 2000.0f, 1250.0f, 1000.0f, 1750.0f, 1.0f };
		}

		void EXPORT OnTouch(CBaseEntity *pOther)
		{
			if (pev->owner == pOther->edict())
				return;

			CBaseEntity *pAttacker = CBaseEntity::Instance(pev->owner);
			CBasePlayer *pAttackePlayer = nullptr;
			if (pAttacker && pAttacker->IsPlayer())
				pAttackePlayer = static_cast<CBasePlayer *>(pAttacker);

			if (pAttackePlayer &&
				pOther->pev->takedamage != DAMAGE_NO &&
				pOther->IsAlive()
				)
			{
				Vector vecDirection = (pOther->pev->origin - pev->origin).Normalize();

				TraceResult tr;
				UTIL_TraceLine(pev->origin, pOther->pev->origin, missile, ENT(pAttackePlayer->pev), &tr);
				tr.iHitgroup = HITGROUP_CHEST; // ...

				ClearMultiDamage();
				pOther->TraceAttack(pAttackePlayer->pev, m_flTouchDamage, vecDirection, &tr, DMG_BULLET);
				ApplyMultiDamage(pAttackePlayer->pev, pAttackePlayer->pev);
			}

			RadiusDamage();
		}

		void EXPORT IgniteThink()
		{
			MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
			WRITE_BYTE(TE_BEAMFOLLOW);
			WRITE_SHORT(this->entindex());
			WRITE_SHORT(m_iSprEffect);
			WRITE_BYTE(40);
			WRITE_BYTE(5);
			WRITE_BYTE(200);
			WRITE_BYTE(200);
			WRITE_BYTE(200);
			WRITE_BYTE(200);
			MESSAGE_END();

			SetThink(&CSVDEXGrenade::FollowThink);
			pev->nextthink = gpGlobals->time + 100ms;
		}

		void EXPORT FollowThink(void)
		{
			UTIL_MakeAimVectors(pev->angles);

			pev->angles = UTIL_VecToAngles(gpGlobals->v_forward);
			pev->nextthink = gpGlobals->time + 100ms;

			if(gpGlobals->time > m_flRemoveTime)
			{
				Remove();
			}
		}

		void RadiusDamage()
		{
			const float flRadius = m_flExplodeRadius;
			const float flDamage = m_flExplodeDamage;
			const Vector vecSrc = pev->origin;
			entvars_t * const pevAttacker = VARS(pev->owner);
			entvars_t * const pevInflictor = this->pev;
			int bitsDamageType = DMG_BULLET;

			TraceResult tr;
			const float falloff = flRadius ? flDamage / flRadius : 1;
			const int bInWater = (UTIL_PointContents(vecSrc) == CONTENTS_WATER);

			CBaseEntity *pEntity = NULL;
			while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, flRadius)) != NULL)
			{
				if (pEntity->pev->takedamage != DAMAGE_NO)
				{
					if (bInWater && !pEntity->pev->waterlevel)
						continue;

					if (!bInWater && pEntity->pev->waterlevel == 3)
						continue;

					if (pEntity->IsBSPModel())
						continue;

					/*if (pEntity->pev == pevAttacker)
						continue;*/

					Vector vecSpot = pEntity->BodyTarget(vecSrc);
					UTIL_TraceLine(vecSrc, vecSpot, missile, ENT(pevInflictor), &tr);

					if (tr.flFraction == 1.0f || tr.pHit == pEntity->edict())
					{
						if (tr.fStartSolid)
						{
							tr.vecEndPos = vecSrc;
							tr.flFraction = 0;
						}
						float flAdjustedDamage = flDamage - (vecSrc - pEntity->pev->origin).Length() * falloff;
						flAdjustedDamage = Q_max(0, flAdjustedDamage);

						if (tr.flFraction == 1.0f)
						{
							pEntity->TakeDamage(pevInflictor, pevAttacker, flAdjustedDamage, bitsDamageType);
						}
						else
						{
							tr.iHitgroup = HITGROUP_CHEST;
							ClearMultiDamage();
							pEntity->TraceAttack(pevInflictor, flAdjustedDamage, (tr.vecEndPos - vecSrc).Normalize(), &tr, bitsDamageType);
							ApplyMultiDamage(pevInflictor, pevAttacker);
						}

						CBasePlayer *pVictim = dynamic_cast<CBasePlayer *>(pEntity);
						if (pVictim->m_bIsZombie) // Zombie Knockback...
						{
							ApplyKnockbackData(pVictim, vecSpot - vecSrc, { 800.0f, 800.0, 800.0f, 800.0f, 0.5f });
						}
					}
				}
			}

			MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
			WRITE_BYTE(TE_EXPLOSION);
			WRITE_COORD(pev->origin.x);
			WRITE_COORD(pev->origin.y);
			WRITE_COORD(pev->origin.z);
			WRITE_SHORT(MODEL_INDEX("sprites/eexplo.spr"));
			WRITE_BYTE(25);
			WRITE_BYTE(30);
			WRITE_BYTE(TE_EXPLFLAG_NONE);
			MESSAGE_END();

			return Remove();
		}

		void Init(Vector vecVelocity, float flTouchDamage, float flExplodeDamage, float flExplodeRadius, TeamName iTeam)
		{
			std::tie(m_flTouchDamage, m_flExplodeDamage, m_flExplodeRadius, m_iTeam) = std::make_tuple(flTouchDamage, flExplodeDamage, flExplodeRadius, iTeam);
			pev->velocity = std::move(vecVelocity);
		}

		int m_fSequenceLoops;
		time_point_t m_flRemoveTime;
		float m_flMaxFrames;
		float m_flTouchDamage;
		float m_flExplodeDamage;
		float m_flExplodeRadius;
		TeamName m_iTeam;
		int m_iSprEffect;

	protected:
		void Remove()
		{
			SetThink(nullptr);
			SetTouch(nullptr);
			pev->effects |= EF_NODRAW; // 0x80u
			return UTIL_Remove(this);
		}
	};
	LINK_ENTITY_TO_CLASS(svdex_grenade, CSVDEXGrenade)
#endif
enum CSVDEX::svdex_e
{
	SVDEX_IDLEA,
	SVDEX_SHOOTA,
	SVDEX_RELOAD,
	SVDEX_DRAWA,
	SVDEX_IDLEB,
	SVDEX_SHOOTB_1,
	SVDEX_SHOOTB_LAST,
	SVDEX_DRAWB,
	SVDEX_MOVE_GRENADE,
	SVDEX_MOVE_CARBINE
};

LINK_ENTITY_TO_CLASS(weapon_svdex, CSVDEX)

void CSVDEX::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_svdex");

	Precache();
	m_iId = WEAPON_AK47;
	SET_MODEL(ENT(pev), "models/w_svdex.mdl");

	m_iDefaultAmmo = 20;  //????
	m_iDefaultAmmo2 = 10;
	m_flAccuracy = 0.2;
	m_iShotsFired = 0;
	m_bDelayFire = true;

	FallInit();
}

void CSVDEX::Precache(void)
{
	PRECACHE_MODEL("models/v_svdex.mdl");
	PRECACHE_MODEL("models/p_svdex.mdl");
	PRECACHE_MODEL("models/w_svdex.mdl");

	PRECACHE_SOUND("weapons/svdex-1.wav");
	PRECACHE_SOUND("weapons/svdex-launcher.wav");
	PRECACHE_SOUND("weapons/svdex_exp.wav");
	PRECACHE_SOUND("weapons/svdex_foley1.wav");
	PRECACHE_SOUND("weapons/svdex_foley2.wav");
	PRECACHE_SOUND("weapons/svdex_foley3.wav");
	PRECACHE_SOUND("weapons/svdex_foley4.wav");
	PRECACHE_SOUND("weapons/svdex_draw.wav");
	PRECACHE_SOUND("weapons/svdex_clipon.wav");
	PRECACHE_SOUND("weapons/svdex_clipin.wav");
	PRECACHE_SOUND("weapons/svdex_clipout.wav");

	m_iShell = PRECACHE_MODEL("models/rshell_big.mdl");
	m_usFireSVDEX = PRECACHE_EVENT(1, "events/svdex.sc");

#ifndef CLIENT_DLL
	UTIL_PrecacheOther("svdex_grenade");
#endif
}

int CSVDEX::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "556Nato";
	p->iMaxAmmo1 = MAX_AMMO_556NATO;
	p->pszAmmo2 = "SVDEXGrenade"; // whatever it is, it can't be bought
	p->iMaxAmmo2 = 10;
	p->iMaxClip = 20;
	p->iSlot = 0;
	p->iPosition = 6;
	p->iId = m_iId = WEAPON_M4A1;
	p->iFlags = 0;
	p->iWeight = M4A1_WEIGHT;

	return 1;
}

BOOL CSVDEX::Deploy(void)
{
	m_bDelayFire = true;
	iShellOn = 1;
	m_flAccuracy = 0.2;
	m_iShotsFired = 0;

	m_iWeaponState &= ~WPNSTATE_M4A1_SILENCED;
	return DefaultDeploy("models/v_svdex.mdl", "models/p_svdex.mdl", SVDEX_DRAWA, "rifle", UseDecrement() != FALSE);
}

void CSVDEX::Holster(int skiplocal)
{
#ifndef CLIENT_DLL
	m_pPlayer->UpdateShieldCrosshair((m_iWeaponState & WPNSTATE_M4A1_SILENCED) == 0);
#endif
	return CBasePlayerWeapon::Holster(skiplocal);
}

int CSVDEX::ExtractAmmo(CBasePlayerWeapon* pWeapon)
{
	if (m_iDefaultAmmo2)
	{
		m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = m_iDefaultAmmo2;
		m_iDefaultAmmo2 = 0;
	}
	return CBasePlayerWeapon::ExtractAmmo(pWeapon);
}

void CSVDEX::SecondaryAttack(void)
{
	if (m_iWeaponState & WPNSTATE_M4A1_SILENCED)
	{
		m_iWeaponState &= ~WPNSTATE_M4A1_SILENCED;
		SendWeaponAnim(SVDEX_MOVE_CARBINE, UseDecrement() != FALSE);
		strcpy(m_pPlayer->m_szAnimExtention, "rifle");
	}
	else
	{
		m_iWeaponState |= WPNSTATE_M4A1_SILENCED;
		SendWeaponAnim(SVDEX_MOVE_GRENADE, UseDecrement() != FALSE);
		strcpy(m_pPlayer->m_szAnimExtention, "rifle");
	}

#ifndef CLIENT_DLL
	m_pPlayer->UpdateShieldCrosshair((m_iWeaponState & WPNSTATE_M4A1_SILENCED) == 0);
#endif
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.8s;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.9s;
	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 2s;
}

void CSVDEX::PrimaryAttack(void)
{
	if (m_iWeaponState & WPNSTATE_M4A1_SILENCED)
	{
		SVDEXFire2(3.0s, FALSE);
	}
	else
	{
		if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			SVDEXFire1(0.8, 0.346s, FALSE);
		else if (m_pPlayer->pev->velocity.Length2D() > 140)
			SVDEXFire1(0.15, 0.346s, FALSE);
		else if (m_pPlayer->pev->velocity.Length2D() > 10)
			SVDEXFire1(0.1, 0.346s, FALSE);
		else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
			SVDEXFire1(0.003, 0.346s, FALSE);
		else
			SVDEXFire1(0.007, 0.346s, FALSE);
	}
}

void CSVDEX::SVDEXFire1(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
{
	m_bDelayFire = true;
	m_iShotsFired++;
	m_flAccuracy = ((float)(m_iShotsFired * m_iShotsFired * m_iShotsFired) / 200) + 0.35;

	if (m_flAccuracy > 1)
		m_flAccuracy = 1;

	if (m_iClip <= 0)
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2s;
		}

		return;
	}

	m_iClip--;
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;	// 1000
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;	// 512

	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecDir;

	vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 2, BULLET_PLAYER_556MM, 490, 0.99, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);
	m_pPlayer->pev->effects |= EF_MUZZLEFLASH;

	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFireSVDEX, 0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), FALSE, FALSE);
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;

	m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomFloat(m_pPlayer->random_seed + 4, 0.55, 1.0);
	m_pPlayer->pev->punchangle.y += UTIL_SharedRandomFloat(m_pPlayer->random_seed + 5, -0.65, 0.95);
}

void CSVDEX::SVDEXFire2(duration_t flCycleTime, BOOL fUseAutoAim)
{
	m_bDelayFire = true;

	if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] <= 0)
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2s;
		}

		return;
	}
	--m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType];
	
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;	// 1000
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;	// 512

	Vector vecDir;

#ifndef CLIENT_DLL
	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
	Vector vecSrc = m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 16 + gpGlobals->v_right * 8 + gpGlobals->v_up * -8;
	CSVDEXGrenade *pEnt = static_cast<CSVDEXGrenade *>(CBaseEntity::Create("svdex_grenade", vecSrc, m_pPlayer->pev->v_angle, ENT(m_pPlayer->pev)));
	if (pEnt)
	{
		pEnt->Init(gpGlobals->v_forward * 1000 + gpGlobals->v_up * 135, 0, 900.0, 300, m_pPlayer->m_iTeam);
	}
#endif

	//vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 2, BULLET_PLAYER_556MM, 490, 0.99, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);
	m_pPlayer->pev->effects |= EF_MUZZLEFLASH;

	int flags = 0;

	const bool bEmpty = m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] == 0;
	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFireSVDEX, 0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), TRUE, bEmpty);

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.0s;
}

void CSVDEX::Reload(void)
{
	if (m_iWeaponState & WPNSTATE_M4A1_SILENCED)
		return;

	if (m_pPlayer->ammo_556nato <= 0)
		return;


	if (DefaultReload(20, SVDEX_RELOAD, 3.8s))
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
		m_flAccuracy = 0.2;
		m_iShotsFired = 0;
		m_bDelayFire = false;
	}
}

void CSVDEX::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;

	if (m_iWeaponState & WPNSTATE_M4A1_SILENCED)
		SendWeaponAnim(SVDEX_IDLEB, UseDecrement() != FALSE);
	else
		SendWeaponAnim(SVDEX_IDLEA, UseDecrement() != FALSE);
}

}
