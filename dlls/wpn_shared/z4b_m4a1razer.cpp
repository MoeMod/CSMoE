/*
z4b_m4a1razer.cpp - 
Copyright (C) 2019 Moemod Haoyuan

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
#include "z4b_m4a1razer.h"
#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

#ifndef CLIENT_DLL
	class CM4A1RazerGrenade : public CBaseEntity
	{
	public:
		void Spawn() override
		{
			Precache();

			m_fSequenceLoops = 0;
			//ph26 = 0;
			SetThink(&CM4A1RazerGrenade::IgniteThink);
			SetTouch(&CM4A1RazerGrenade::OnTouch);
			SET_MODEL(this->edict(), "sprites/plasmaroll.spr");

			//ph32 = ?
			pev->rendermode = kRenderTransAdd;
			pev->renderamt = 250;
			pev->scale = 1.0 - float(m_iType) * 0.4;
			pev->framerate = 10;
			pev->scale = 0.2;
			pev->angles = { RANDOM_FLOAT(-60, 60), RANDOM_FLOAT(-60, 60) , RANDOM_FLOAT(-60, 60) };
			pev->solid = SOLID_TRIGGER;
			pev->movetype = MOVETYPE_FLY;
			pev->nextthink = gpGlobals->time + 0.1s;
			m_flRemoveTime = gpGlobals->time + 500ms;
			m_flMaxFrames = 300.0;
			UTIL_SetSize(pev, { -4, -4, -4 }, { 4, 4, 4 });

			pev->classname = MAKE_STRING("d_m4a1razer");
		}

		void Precache() override
		{
			PRECACHE_MODEL("sprites/plasmaroll.spr");
			PRECACHE_SOUND("weapons/plasmagun_exp.wav");
			m_iSprEffect = PRECACHE_MODEL("sprites/lgtning.spr");
			m_iSprExplo = PRECACHE_MODEL("sprites/plasmabomb.spr");
		}

		KnockbackData GetKnockBackData()
		{
			return {  };
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
			Remove();
		}

		void EXPORT IgniteThink()
		{
			MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
			WRITE_BYTE(TE_BEAMFOLLOW);
			WRITE_SHORT(this->entindex());
			WRITE_SHORT(m_iSprEffect);
			WRITE_BYTE(30);
			WRITE_BYTE(12);
			WRITE_BYTE(30);
			WRITE_BYTE(200);
			WRITE_BYTE(30);
			WRITE_BYTE(200);
			MESSAGE_END();

			SetThink(&CM4A1RazerGrenade::FollowThink);
			pev->nextthink = gpGlobals->time + 100ms;
		}

		void EXPORT FollowThink(void)
		{
			UTIL_MakeAimVectors(pev->angles);

			pev->scale += 0.01;

			constexpr auto Frames = 33;
			pev->frame = (static_cast<int>(pev->frame) + 1) % Frames;
			pev->nextthink = gpGlobals->time + 45ms;
			
			if(gpGlobals->time > m_flRemoveTime)
			{
				if (m_iType == 2)
				{
					if ((pev->renderamt -= 15) < 5)
					{
						Remove();
					}
				}
				else
				{
					// separation
					RadiusDamage();
					
					const Vector vecForward = m_vecStartVelocity.Normalize();
					const Vector vecRight = CrossProduct(vecForward, { 0.0f, 0.0f, 1.0f });
					const Vector vecUp = CrossProduct(vecRight, vecForward);

					auto f = [this](Vector vecNewOrigin) {
						CM4A1RazerGrenade* pEntity = CreateClassPtr<CM4A1RazerGrenade>();
						if(pEntity)
						{
							pEntity->pev->owner = pev->owner;
							pEntity->pev->origin = vecNewOrigin;
							DispatchSpawn(pEntity->edict());
							pEntity->Init(m_vecStartVelocity, 0, g_pModRunning->DamageTrack() == DT_NONE ? 20 : 650, 180, m_iTeam, m_iType + 1);
						}
						return pEntity;
					};

					f(pev->origin - (vecUp) * 35);
					f(pev->origin + (vecRight * sqrt(3) / 2 + vecUp / 2) * 50);
					f(pev->origin + (-vecRight * sqrt(3) / 2 + vecUp / 2) * 35);

					Remove();
				}
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
			WRITE_COORD(pev->origin.z + 5);
			WRITE_SHORT(m_iSprExplo);
			WRITE_BYTE(m_iType ? 5 : 8);
			WRITE_BYTE(30);
			WRITE_BYTE(TE_EXPLFLAG_NOSOUND | TE_EXPLFLAG_NOPARTICLES);
			MESSAGE_END();

			EMIT_SOUND_DYN(edict(), CHAN_AUTO, "weapons/plasmagun_exp.wav", 1.0, ATTN_NORM, 0, PITCH_NORM);
		}

		void Init(Vector vecVelocity, float flTouchDamage, float flExplodeDamage, float flExplodeRadius, TeamName iTeam, int type = 0)
		{
			std::tie(m_flTouchDamage, m_flExplodeDamage, m_flExplodeRadius, m_iTeam) = std::make_tuple(flTouchDamage, flExplodeDamage, flExplodeRadius, iTeam);
			m_vecStartVelocity = pev->velocity = std::move(vecVelocity);
			m_iType = type;
		}

		int m_fSequenceLoops;
		time_point_t m_flRemoveTime;
		float m_flMaxFrames;
		float m_flTouchDamage;
		float m_flExplodeDamage;
		float m_flExplodeRadius;
		TeamName m_iTeam;
		int m_iSprEffect;
		int m_iSprExplo;
		Vector m_vecStartVelocity;
		int m_iType;

	protected:
		void Remove()
		{
			SetThink(nullptr);
			SetTouch(nullptr);
			pev->effects |= EF_NODRAW; // 0x80u
			return UTIL_Remove(this);
		}
	};
	LINK_ENTITY_TO_CLASS(m4a1razer_grenade, CM4A1RazerGrenade)
#endif
enum
{
	ANIM_IDLEB = 0,
	ANIM_SHOOTB1,
	ANIM_SHOOTB2,
	ANIM_RELOADB,
	ANIM_DRAWB,
	ANIM_CHANGEA,
	ANIM_IDLE,
	ANIM_SHOOT,
	ANIM_RELOAD,
	ANIM_DRAW,
	ANIM_CHANGEB,
};

LINK_ENTITY_TO_CLASS(z4b_m4a1razer, CM4A1Razer)

void CM4A1Razer::Spawn(void)
{
	pev->classname = MAKE_STRING("z4b_m4a1razer");

	Precache();
	m_iId = WEAPON_M4A1;
	SET_MODEL(ENT(pev), "models/z4b/w_m4a1razer.mdl");

	m_iDefaultAmmo = 30;
	m_iDefaultAmmo2 = 10;
	m_flAccuracy = 0.2;
	m_iShotsFired = 0;
	m_bDelayFire = true;

	FallInit();
}

void CM4A1Razer::Precache(void)
{
	PRECACHE_MODEL("models/z4b/p_m4a1razer.mdl");
	PRECACHE_MODEL("models/z4b/v_m4a1razer.mdl");
	PRECACHE_MODEL("models/z4b/w_m4a1razer.mdl");

	PRECACHE_SOUND("weapons/balrog9_charge_finish1.wav");

	m_iShell = PRECACHE_MODEL("models/rshell_big.mdl");
	m_usFireSVDEX = PRECACHE_EVENT(1, "events/m4a1razer.sc");

#ifndef CLIENT_DLL
	UTIL_PrecacheOther("m4a1razer_grenade");
#endif
}

int CM4A1Razer::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "556Nato";
	p->iMaxAmmo1 = MAX_AMMO_556NATO;
	p->pszAmmo2 = "M4A1RazerGrenade"; // whatever it is, it can't be bought
	p->iMaxAmmo2 = 10;
	p->iMaxClip = 30;
	p->iSlot = 0;
	p->iPosition = 6;
	p->iId = m_iId = WEAPON_M4A1;
	p->iFlags = 0;
	p->iWeight = M4A1_WEIGHT;

	return 1;
}

BOOL CM4A1Razer::Deploy(void)
{
	m_bDelayFire = true;
	iShellOn = 1;
	m_flAccuracy = 0.2;
	m_iShotsFired = 0;

#ifndef CLIENT_DLL
	m_pPlayer->UpdateShieldCrosshair((m_iWeaponState & WPNSTATE_M4A1_SILENCED) == 0);
#endif
	
	return DefaultDeploy("models/z4b/v_m4a1razer.mdl", "models/z4b/p_m4a1razer.mdl", m_iWeaponState & WPNSTATE_M4A1_SILENCED ? ANIM_DRAWB : ANIM_DRAW, "rifle", UseDecrement() != FALSE);
}

void CM4A1Razer::Holster(int skiplocal)
{
	return CBasePlayerWeapon::Holster(skiplocal);
}

int CM4A1Razer::ExtractAmmo(CBasePlayerWeapon* pWeapon)
{
	if (m_iDefaultAmmo2)
	{
		m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = m_iDefaultAmmo2;
		m_iDefaultAmmo2 = 0;
	}
	return CBasePlayerWeapon::ExtractAmmo(pWeapon);
}

void CM4A1Razer::SecondaryAttack(void)
{
	if (m_iWeaponState & WPNSTATE_M4A1_SILENCED)
	{
		m_iWeaponState &= ~WPNSTATE_M4A1_SILENCED;
		SendWeaponAnim(ANIM_CHANGEA, UseDecrement() != FALSE);
		strcpy(m_pPlayer->m_szAnimExtention, "rifle");
	}
	else
	{
		m_iWeaponState |= WPNSTATE_M4A1_SILENCED;
		SendWeaponAnim(ANIM_CHANGEB, UseDecrement() != FALSE);
		strcpy(m_pPlayer->m_szAnimExtention, "rifle");
	}

#ifndef CLIENT_DLL
	m_pPlayer->UpdateShieldCrosshair((m_iWeaponState & WPNSTATE_M4A1_SILENCED) == 0);
#endif
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.2s;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 2s;
	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 2s;
}

void CM4A1Razer::PrimaryAttack(void)
{
	if (m_iWeaponState & WPNSTATE_M4A1_SILENCED)
	{
		SVDEXFire2(3.5s, FALSE);
	}
	else
	{
		if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			SVDEXFire1(0.035 + (0.4) * m_flAccuracy, 0.09s, FALSE);
		else if (m_pPlayer->pev->velocity.Length2D() > 140)
			SVDEXFire1(0.035 + (0.07) * m_flAccuracy, 0.09s, FALSE);
		else
			SVDEXFire1((0.025) * m_flAccuracy, 0.09s, FALSE);
	}
}

void CM4A1Razer::SVDEXFire1(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
{
	m_bDelayFire = true;
	m_iShotsFired++;
	m_flAccuracy = ((float)(m_iShotsFired * m_iShotsFired * m_iShotsFired) / 220) + 0.32;

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

	bool bCharge = false;
	if(m_iShotsFired == 18 && m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] < 10)
	{
		// give ammo
		++m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType];
		bCharge = true;
		EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "weapons/balrog9_charge_finish1.wav", 1.0, ATTN_NORM, 0, PITCH_NORM);
	}
	
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;	// 1000
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;	// 512

	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecDir;

	vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 2, BULLET_PLAYER_556MM, 33, 0.96, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);
	m_pPlayer->pev->effects |= EF_MUZZLEFLASH;

	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFireSVDEX, 0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), FALSE, bCharge);
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;

	if (m_pPlayer->pev->velocity.Length2D() > 0)
		KickBack(0.7, 0.275, 0.425, 0.0335, 3.5, 2.5, 8);
	else if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		KickBack(1.5, 0.9, 0.4, 0.3, 7.0, 4.5, 8);
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		KickBack(0.5, 0.12, 0.24, 0.001, 3.15, 2.1, 6);
	else
		KickBack(0.595, 0.225, 0.32, 0.0115, 3.25, 2.1, 7);
}

void CM4A1Razer::SVDEXFire2(duration_t flCycleTime, BOOL fUseAutoAim)
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
	CM4A1RazerGrenade *pEnt = static_cast<CM4A1RazerGrenade *>(CBaseEntity::Create("m4a1razer_grenade", vecSrc, m_pPlayer->pev->v_angle, ENT(m_pPlayer->pev)));
	if (pEnt)
	{
		float flExplodeDamage = g_pModRunning->DamageTrack() == DT_NONE ? 50 : 1750;
		pEnt->Init(gpGlobals->v_forward * 1000, 0, g_pModRunning->DamageTrack() == DT_NONE ? 50 : 1750, 300, m_pPlayer->m_iTeam);
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
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + flCycleTime + 0.2s;
}

void CM4A1Razer::Reload(void)
{
	if (m_iWeaponState & WPNSTATE_M4A1_SILENCED)
		return;

	if (m_pPlayer->ammo_556nato <= 0)
		return;

	if (DefaultReload(30, ANIM_RELOAD, 3.8s))
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
		m_flAccuracy = 0.2;
		m_iShotsFired = 0;
		m_bDelayFire = false;
	}
}

void CM4A1Razer::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;

	if (m_iWeaponState & WPNSTATE_M4A1_SILENCED)
		SendWeaponAnim(ANIM_IDLEB, UseDecrement() != FALSE);
	else
		SendWeaponAnim(ANIM_IDLE, UseDecrement() != FALSE);
}

}
