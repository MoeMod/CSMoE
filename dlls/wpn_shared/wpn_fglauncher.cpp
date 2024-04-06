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
#include "wpn_fglauncher.h"

#ifndef CLIENT_DLL
#include "soundent.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

#ifndef CLIENT_DLL	

	CFGLauncherRocket* CFGLauncherRocket::Create(Vector vecSrc, Vector vecAngles, Vector vecVelocity, CBasePlayer* pPlayer, float flDamage, float radius, float kickrate)
	{
		CFGLauncherRocket* pRocket = (CFGLauncherRocket*)CBaseEntity::Create("fglauncher_rocket", vecSrc, vecAngles, pPlayer->edict());

		if (pRocket)
		{
			pRocket->pev->velocity = vecVelocity;
			pRocket->pev->owner = pPlayer->edict();
			pRocket->m_iTeam = pPlayer->m_iTeam;
			pRocket->m_bBounce = false;
			pRocket->m_iBounce = 0;
			pRocket->pev->dmg = pRocket->m_flFGLauncherRocketDamage = flDamage;
			pRocket->m_flFGLauncherRocketExplodeRadius = radius;
			pRocket->m_flFGLauncherRocketExplodeKickRate = kickrate;
			pRocket->SetTouch(&CFGLauncherRocket::RocketTouch);
		}
		return pRocket;
	}

	void CFGLauncherRocket::Spawn(void)
	{
		Precache();

		pev->classname = MAKE_STRING("fglauncher_rocket");
		pev->movetype = MOVETYPE_BOUNCE;
		pev->solid = SOLID_BBOX;
		pev->gravity = 0.4;
		pev->friction = 0.3;
		pev->dmg = m_flFGLauncherRocketDamage;
		pev->dmgtime = gpGlobals->time + 1.5s;
		pev->angles.x -= 30.0;

		UTIL_MakeVectors(pev->angles);
		pev->angles.x = -(pev->angles.x + 30.0);

		SET_MODEL(edict(), "models/shell_svdex.mdl");
		UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));

		SetThink(&CFGLauncherRocket::IgniteThink);
		SetTouch(&CGrenade::ExplodeTouch);

		pev->nextthink = gpGlobals->time + 0.1s;
	}

	void CFGLauncherRocket::Precache(void)
	{
		PRECACHE_MODEL("models/shell_svdex.mdl");
		PRECACHE_MODEL("sprites/spark2.spr");
		PRECACHE_MODEL("sprites/spark3.spr");

		PRECACHE_SOUND("weapons/firecracker_bounce1.wav");
		PRECACHE_SOUND("weapons/firecracker_bounce2.wav");
		PRECACHE_SOUND("weapons/firecracker_bounce3.wav");
		PRECACHE_SOUND("weapons/firecracker_explode.wav");

		m_iTrail = PRECACHE_MODEL("sprites/smoke.spr");
	}

	void CFGLauncherRocket::FollowThink(void)
	{
		pev->angles = UTIL_VecToAngles(pev->velocity);

		if (gpGlobals->time > pev->dmgtime)
			Explode(this);

		MESSAGE_BEGIN(MSG_BROADCAST, gmsgMPToCL);
		WRITE_BYTE(36);
		WRITE_COORD(pev->origin.x);
		WRITE_COORD(pev->origin.y);
		WRITE_COORD(pev->origin.z);
		MESSAGE_END();

		m_vecLastPos = pev->origin;
	}

	void CFGLauncherRocket::IgniteThink(void)
	{
		UTIL_EMIT_SOUND(edict(), CHAN_VOICE, "weapons/firecracker-wick.wav", VOL_NORM, 0.5, 0, PITCH_NORM);

		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_BEAMFOLLOW);
		WRITE_SHORT(entindex());
		WRITE_SHORT(m_iTrail);
		WRITE_BYTE(4);
		WRITE_BYTE(2);
		WRITE_BYTE(224);
		WRITE_BYTE(90);
		WRITE_BYTE(90);
		WRITE_BYTE(200);
		MESSAGE_END();

		SetThink(&CFGLauncherRocket::FollowThink);
		m_vecLastPos = pev->origin;
		pev->nextthink = gpGlobals->time + 0.1s;
	}

	void CFGLauncherRocket::RocketTouch(CBaseEntity* pOther)
	{
		if (m_bBounce && !pOther->IsPlayer())
		{
			char szSound[64];

			sprintf(szSound, "weapons/firecracker_bounce%d.wav", RANDOM_LONG(1, 3));
			UTIL_EMIT_SOUND(edict(), CHAN_VOICE, szSound, VOL_NORM, 0.5, 0, PITCH_NORM);

			m_iBounce++;

			if (m_iBounce < 5)
				return;

			Explode(pOther);
		}
		else
		{
			Explode(pOther);
		}
	}

	void CFGLauncherRocket::Explode(CBaseEntity* pOther)
	{
		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_KILLBEAM);
		WRITE_SHORT(this->entindex());
		MESSAGE_END();

		int flags = PARTICLE_ANIMATE | PARTICLE_COLLIDEKILL | PARTICLE_ROTATE | PARTICLE_EMIT_PARENT_ONLY;
		TraceResult tr;
		Vector vecParticleVel;

		UTIL_TraceLine(pev->origin, pev->origin + (pev->origin - m_vecLastPos).Normalize() * 5.0, ignore_monsters, edict(), &tr);

		if (tr.flFraction < 1.0)
		{
			if (tr.vecPlaneNormal.z < 0)
			{
				vecParticleVel = tr.vecPlaneNormal * 100;
			}
			else if (tr.vecPlaneNormal.z < 0.8)
			{
				vecParticleVel = tr.vecPlaneNormal * 50;
			}
			else
			{
				vecParticleVel = tr.vecPlaneNormal * 200;
			}
		}
		else
		{
			vecParticleVel = Vector(0, 0, 200);
		}

		UTIL_TempEmit(
			"sprites/spark2.spr",
			3,
			RANDOM_LONG(10, 30),
			18,
			flags,
			RANDOM_LONG(1, 5),
			16,
			kRenderTransAdd,
			240,
			RANDOM_LONG(70, 250),
			RANDOM_LONG(70, 250),
			RANDOM_LONG(70, 250),
			pev->origin + tr.vecPlaneNormal,
			vecParticleVel,
			300,
			5,
			32,
			2,
			0,
			0,
			0,
			0
		);

		UTIL_TempEmit(
			"sprites/spark2.spr",
			3,
			RANDOM_LONG(10, 30),
			18,
			flags,
			RANDOM_LONG(1, 5),
			17,
			kRenderTransAdd,
			240.0,
			RANDOM_LONG(70, 250),
			RANDOM_LONG(70, 250),
			RANDOM_LONG(70, 250),
			pev->origin + tr.vecPlaneNormal,
			vecParticleVel,
			300,
			5,
			32,
			2,
			0,
			0,
			0,
			0
		);

		MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
		WRITE_BYTE(TE_EXPLOSION);
		WRITE_COORD(pev->origin.x);
		WRITE_COORD(pev->origin.y);
		WRITE_COORD(pev->origin.z);
		WRITE_SHORT(g_sModelIndexFireball2);
		WRITE_BYTE(5);
		WRITE_BYTE(30);
		WRITE_BYTE(TE_EXPLFLAG_NONE);
		MESSAGE_END();

		UTIL_EMIT_SOUND(edict(), CHAN_VOICE, "weapons/firecracker_explode.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

		ExplodeTouchWithKickRate(pOther, m_flFGLauncherRocketExplodeRadius * 39.37, EXPTYPE_FGLAUNCHERROCKET, m_flFGLauncherRocketExplodeKickRate, true);
	}

	LINK_ENTITY_TO_CLASS(fglauncher_rocket, CFGLauncherRocket);

#endif

	LINK_ENTITY_TO_CLASS(weapon_fglauncher, CFGLauncher)

	void CFGLauncher::Spawn(void)
	{
		Precache();
		m_iId = WEAPON_FGLAUNCHER;
		pev->classname = MAKE_STRING("weapon_fglauncher");
		SET_MODEL(ENT(pev), "models/w_fglauncher.mdl");
		m_iDefaultAmmo = 30;

		FallInit();
	}

	void CFGLauncher::Precache(void)
	{
		PRECACHE_MODEL("models/v_fglauncher.mdl");
		PRECACHE_MODEL("models/p_fglauncher.mdl");
		PRECACHE_MODEL("models/w_fglauncher.mdl");

		m_iShellId = 0;
		m_usFireFGLauncher = PRECACHE_EVENT(1, "events/fglauncher.sc");

#ifndef CLIENT_DLL
		UTIL_PrecacheOther("fglauncher_rocket");
#endif
	}

	int CFGLauncher::GetItemInfo(ItemInfo* p)
	{
		p->pszName = STRING(pev->classname);
		p->pszAmmo1 = "FGLauncherRocket";
		p->iMaxAmmo1 = 40;
		p->pszAmmo2 = NULL;
		p->iMaxAmmo2 = -1;
		p->iMaxClip = 10;
		p->iSlot = 0;
		p->iPosition = 1;
		p->iId = m_iId = WEAPON_FGLAUNCHER;
		p->iFlags = 0;
		p->iWeight = TMP_WEIGHT;

		return 1;
	}

	BOOL CFGLauncher::Deploy(void)
	{
		return DefaultDeploy("models/v_fglauncher.mdl", "models/p_fglauncher.mdl", FGLAUNCHER_DRAW, "m249", UseDecrement() != FALSE);
	}

	void CFGLauncher::PrimaryAttack(void)
	{
		if (m_iClip <= 0)
		{
			PlayEmptySound();

			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.4s;

			return;
		}
	
		m_iClip--;

		m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
		m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;
		

#ifndef CLIENT_DLL
		float flFGLauncherRocketVelBase = 800;
		float flFGLauncherRocketDamage = 150;
		float flFGLauncherRocketExplodeRadius = 3;
		float flFGLauncherRocketExplodeKickRate = 0;

		m_pPlayer->m_iExtraSoundTypes = bits_SOUND_DANGER;

		m_pPlayer->SetAnimation(PLAYER_ATTACK1);

		if (g_pModRunning->DamageTrack() == DT_ZBS)
		{
			flFGLauncherRocketVelBase = 800;
			flFGLauncherRocketDamage = 380;
			flFGLauncherRocketExplodeRadius = 4;
			flFGLauncherRocketExplodeKickRate = 0.5;
		}
		else if (g_pModRunning->DamageTrack() == DT_ZB)
		{
			flFGLauncherRocketVelBase = 800;
			flFGLauncherRocketDamage = 400;
			flFGLauncherRocketExplodeRadius = 4;
			flFGLauncherRocketExplodeKickRate = 0.5;
		}
	

		UTIL_MakeVectors(m_pPlayer->pev->v_angle);

		CFGLauncherRocket::Create(m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 16 + gpGlobals->v_right * 8 + gpGlobals->v_up * -8,
			m_pPlayer->pev->v_angle, gpGlobals->v_forward * flFGLauncherRocketVelBase + Vector(0, 0, 100), m_pPlayer, flFGLauncherRocketDamage, flFGLauncherRocketExplodeRadius, flFGLauncherRocketExplodeKickRate);

		if (m_iClip)
			UTIL_EMIT_SOUND(edict(), CHAN_WEAPON, "weapons/fglauncher-1.wav", VOL_NORM, 0.7, 0, PITCH_NORM);
#endif

		int flags;

#ifdef CLIENT_WEAPONS
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif

		PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFireFGLauncher, 0, g_vecZero, g_vecZero, 0.0, 0.0, int(m_pPlayer->pev->punchangle.x * 100), int(m_pPlayer->pev->punchangle.y * 100), m_iClip, FALSE);

		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.8s;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.8s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + m_flNextPrimaryAttack;

		if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 2, 5);
		else
			m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 2, 5);
	}

	void CFGLauncher::Reload(void)
	{
		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			return;

		if (m_flNextPrimaryAttack > UTIL_WeaponTimeBase())
			return;

		if (DefaultReload(iGetMaxClip(), FGLAUNCHER_RELOAD, 5.04s))
		{
#ifndef CLIENT_DLL
			m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
			m_bDelayFire = false;
			m_iShotsFired = 0;
		}
	}

	void CFGLauncher::WeaponIdle(void)
	{
		ResetEmptySound();
		m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20.0s;
		SendWeaponAnim(FGLAUNCHER_IDLE, UseDecrement() != FALSE);
	}


}
