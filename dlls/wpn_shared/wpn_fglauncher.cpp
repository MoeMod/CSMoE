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

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

#ifndef CLIENT_DLL
	
	void CFgLauncherRocket::Spawn(void)
	{
		Precache();

		pev->movetype = MOVETYPE_BOUNCE;
		pev->solid = SOLID_BBOX;

		SET_MODEL(ENT(pev), "models/s_oicw.mdl");
		UTIL_SetSize(pev, { -8, -8, -2 }, { 8, 8, 1.3 });
		UTIL_SetOrigin(pev, pev->origin);

		pev->classname = MAKE_STRING("fglauncher_rocket");
		SetThink(&CFgLauncherRocket::IgniteThink);
		SetTouch(&CFgLauncherRocket::RocketTouch);
		

		pev->angles.x -= 30.0;
		UTIL_MakeVectors(pev->angles);
		pev->angles.x = -(pev->angles.x + 30.0);

		pev->velocity = gpGlobals->v_forward * 300;
		pev->gravity = 0.5;
		pev->nextthink = gpGlobals->time + 0.1s;
		m_iEffect = 0;


		EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "weapons/firecracker-wick.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
	}

	void CFgLauncherRocket::Precache(void)
	{
		PRECACHE_MODEL("models/s_oicw.mdl");
		PRECACHE_SOUND("weapons/firecracker_explode.wav");
		PRECACHE_SOUND("weapons/firecracker-wick.wav");

		PRECACHE_MODEL("sprites/spark1.spr");
		PRECACHE_MODEL("sprites/spark2.spr");
		PRECACHE_MODEL("sprites/spark3.spr");
		m_iTrail = PRECACHE_MODEL("sprites/smoke.spr");
	}

	void CFgLauncherRocket::FollowThink(void)
	{
		UTIL_MakeAimVectors(pev->angles);

		pev->angles = UTIL_VecToAngles(gpGlobals->v_forward);
		pev->nextthink = gpGlobals->time + 0.1s;
	}
	void CFgLauncherRocket::IgniteThink(void)
	{
		pev->movetype = MOVETYPE_BOUNCE;

		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_BEAMFOLLOW);
		WRITE_SHORT(entindex());
		WRITE_SHORT(m_iTrail);
		WRITE_BYTE(40);
		WRITE_BYTE(5);
		WRITE_BYTE(255);
		WRITE_BYTE(127);
		WRITE_BYTE(127);
		WRITE_BYTE(255);
		MESSAGE_END();

		m_flIgniteTime = gpGlobals->time;

		SetThink(&CFgLauncherRocket::FollowThink);
		pev->nextthink = gpGlobals->time + 0.1s;
	}

	void CFgLauncherRocket::ForceParabolicMotion(Vector vecEnd, float flOffset)
	{
		Vector vecStart; vecStart = pev->origin;
		float flGravity; flGravity = pev->gravity;

		float flYVelocity, flXVelocity;
		Vector flVelocity;

		Vector vecOrigin[2];
		vecOrigin[0] = vecStart;
		vecOrigin[1] = vecEnd;

		float Acceleration = flGravity > 0.0 ? flGravity * 800.0 : 800.0f;
		float Leveldistance = (vecOrigin[0] - vecOrigin[1]).Length();

		if (vecEnd.z - vecStart.z + flOffset >= 0.0)
		{
			//Vy = ¡Ì 2 * g * h
			flYVelocity = sqrt(2.0 * Acceleration * (vecEnd.z - vecStart.z + flOffset));
			flXVelocity = Leveldistance / (flYVelocity / Acceleration + sqrt(2.0 * flOffset / Acceleration));
		}
		else flXVelocity = Leveldistance / sqrt(2.0 * (vecStart.z - vecEnd.z) / Acceleration);

		flVelocity.x = vecOrigin[1].x - vecOrigin[0].x;
		flVelocity.y = vecOrigin[1].y - vecOrigin[0].y;
		flVelocity.z = vecOrigin[1].z - vecOrigin[0].z;


		float flVectorDiv = 1.0f / Leveldistance / flXVelocity;
		flVelocity.x = flVelocity.x * flVectorDiv;
		flVelocity.y = flVelocity.y * flVectorDiv;
		flVelocity.z = flVelocity.z * flVectorDiv;

		flVelocity.z = flYVelocity;
		pev->velocity = flVelocity;
	}
	
	void CFgLauncherRocket::EffectThink(void)
	{
		pev->nextthink = gpGlobals->time + 0.1s;

		pev->frame += m_iEffect == 1 ? 0.1 : 0.15;

		char szmasg[64]; sprintf(szmasg, "%f", pev->frame);

		UTIL_ClientPrintAll( 4, "FRAME : %s", szmasg);

		if (m_iEffect == 1)
		{

			if (m_flNextTime <= gpGlobals->time)
			{
				pev->effects = pev->effects | EF_NODRAW;
			}
			if (m_flNextFadeTime <= gpGlobals->time)
			{
				m_flNextFadeTime = gpGlobals->time + 0.3s;
				m_iExpTimes ++;

#ifndef CLIENT_DLL
				Vector vecSrc = pev->origin;
				CFgLauncherEffect* pCannon = CFgLauncherEffect::Create(vecSrc, g_vecZero, ENT(pev), NULL);
#endif
				
				
				if (m_iExpTimes > 5)
				{
					SetThink(NULL);
					SetThink(&CBaseEntity::SUB_StartFadeOut);
				}
			}
		}
		else if (m_iEffect == 2)
		{
			if (pev->movetype == MOVETYPE_FLY)
			{
				pev->origin.z += 1.0;
			}
			pev->origin.x += m_iExpTimes ? RANDOM_FLOAT(0.0, 0.25) : RANDOM_FLOAT(-0.25, 0.0);
			pev->origin.y += m_iExpTimes ? RANDOM_FLOAT(-0.25, 0.0) : RANDOM_FLOAT(0.0, 0.25);

			pev->scale += RANDOM_FLOAT(0.001, 0.005);

			if (m_flNextTime <= gpGlobals->time)
			{
				SetThink(NULL);
				SetThink(&CBaseEntity::SUB_StartFadeOut);
			}
		}
	}
	
	void CFgLauncherRocket::RocketTouch(CBaseEntity* pOther)
	{
		if (!pOther)
			return;

		if (pOther->edict() == pev->owner)
			return;

		float flExplodeDamage = GetExplodeDamage();
		float flRadius = GetRadius();
		entvars_t* pevOwner = VARS(pev->owner);

		CBaseEntity* pAttacker = CBaseEntity::Instance(pev->owner);
		CBasePlayer* pAttackPlayer = nullptr;
		if (pAttacker && pAttacker->IsPlayer())
			pAttackPlayer = static_cast<CBasePlayer*>(pAttacker);

		if (!pAttackPlayer)
			return;

		if (!pOther->IsDormant())
		{
			if (pOther->pev->takedamage != DAMAGE_NO)
			{
				pOther->TakeDamage(pev, pevOwner, flExplodeDamage, DMG_NEVERGIB | DMG_EXPLOSION);
			}
		}

		CBaseEntity* pEntity = NULL;

		while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, flRadius)) != NULL)
		{
			if (pEntity->IsDormant())
				continue;

			if (pEntity->pev->takedamage == DAMAGE_NO)
				continue;

			if (pEntity == pOther)
				continue;

			if (pEntity->edict() == pev->owner)
				flExplodeDamage *= 0.35;

			int falloff = flExplodeDamage / flRadius;

			float flCurrentDamage = max(0, flExplodeDamage - (pev->origin - pEntity->pev->origin).Length() * falloff);

			pEntity->TakeDamage(pev, pevOwner, flCurrentDamage, DMG_NEVERGIB | DMG_EXPLOSION);
		}

		EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "weapons/firecracker_explode.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_KILLBEAM);
		WRITE_SHORT(this->entindex());
		MESSAGE_END();

		MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
		WRITE_BYTE(TE_SPRITE);
		WRITE_COORD(pev->origin.x);
		WRITE_COORD(pev->origin.y);
		WRITE_COORD(pev->origin.z + 5.0);
		WRITE_SHORT(MODEL_INDEX("sprites/spark1.spr"));	// sprite index
		WRITE_BYTE(5);			// scale in 0.1's
		WRITE_BYTE(255);			// brightness
		MESSAGE_END();


		MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
		WRITE_BYTE(TE_EXPLOSION);
		WRITE_COORD(pev->origin.x);
		WRITE_COORD(pev->origin.y);
		WRITE_COORD(pev->origin.z + 10.0);
		WRITE_SHORT(MODEL_INDEX("sprites/spark2.spr"));
		WRITE_BYTE(8);
		WRITE_BYTE(30);
		WRITE_BYTE(TE_EXPLFLAG_NODLIGHTS | TE_EXPLFLAG_NOPARTICLES | TE_EXPLFLAG_NOSOUND);
		MESSAGE_END();


		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_WORLDDECAL);
		WRITE_COORD(pev->origin.x);
		WRITE_COORD(pev->origin.y);
		WRITE_COORD(pev->origin.z);
		WRITE_BYTE(RANDOM_LONG(46,48));
		MESSAGE_END();

		int bInWater = (UTIL_PointContents(pev->origin) == CONTENTS_WATER);
		while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, flRadius)) != NULL)
		{
			TraceResult tr2;
			CBasePlayer* pPlayer;


			if (!pEntity->IsPlayer())
				continue;

			pPlayer = (CBasePlayer*)pEntity;

			if (pPlayer->pev->takedamage == DAMAGE_NO)
				continue;

			if (bInWater && pPlayer->pev->waterlevel == 0)
				continue;

			if (!bInWater && pPlayer->pev->waterlevel == 3)
				continue;

			MESSAGE_BEGIN(MSG_ONE, gmsgShake, NULL, pPlayer->edict());
			WRITE_SHORT((1 << 12) * 8);
			WRITE_SHORT((1 << 12) * 18);
			WRITE_SHORT((1 << 12) * 6);
			MESSAGE_END();
	
		}
		//UTIL_ScreenShake(pev->origin, (1 << 12) * 8, (1 << 12) * 18, (1 << 12) * 6, flRadius);

		pev->solid = SOLID_NOT;
		pev->movetype = MOVETYPE_NONE;


		SET_MODEL(ENT(pev), "sprites/spark3.spr");

		pev->rendermode = kRenderTransAdd;
		pev->renderamt = 254.0f;
		pev->velocity = g_vecZero;
		pev->frame = 1.0;
		pev->framerate = 30.0;
		pev->animtime = gpGlobals->time;

		pev->effects |= EF_NOCULL;

		m_iEffect = 1;
		m_flNextTime = gpGlobals->time + 0.8s;

		
		pev->nextthink = gpGlobals->time + 0.01s;
		SetThink(&CFgLauncherRocket::EffectThink);

		SetTouch(NULL);


	}
	LINK_ENTITY_TO_CLASS(fglauncher_rocket, CFgLauncherRocket);

	CFgLauncherEffect* CFgLauncherEffect::Create(const Vector& vecOrigin, const Vector& vecAngles, edict_t* pentOwner, int iTeam)
	{
		edict_t* pent = CREATE_NAMED_ENTITY(MAKE_STRING("fglauncher_effect"));

		if (FNullEnt(pent))
		{
			ALERT(at_console, "NULL Ent in Create!\n");
			return NULL;
		}

		CFgLauncherEffect* pCannon = (CFgLauncherEffect*)Instance(pent);

		if (pCannon)
		{
			pCannon->pev->owner = pentOwner;
			pCannon->pev->origin = vecOrigin;
			pCannon->pev->angles = vecAngles;
			pCannon->m_iTeam = iTeam;
			pCannon->Spawn();
		}

		return pCannon;
	}

	void CFgLauncherEffect::Spawn()
	{
		Precache();

		CBaseEntity* pOwner = CBaseEntity::Instance(pev->owner);
		CBasePlayer* pEntityOwner = nullptr;
		if (pOwner && pOwner->IsPlayer())
			pEntityOwner = static_cast<CBasePlayer*>(pOwner);


		SET_MODEL(ENT(pev), "sprites/spark2.spr");
		UTIL_SetSize(pev, g_vecZero, g_vecZero);


		pev->classname = MAKE_STRING("fglauncher_effect");

		pev->rendermode = kRenderTransAdd;
		pev->renderamt = 254.0f;
		pev->scale = 0.5;

		pev->movetype = (pev->flags & FL_ONGROUND) ? MOVETYPE_FLY : MOVETYPE_NONE;
		pev->solid = SOLID_NOT;
		pev->velocity = g_vecZero;
		pev->effects |= EF_NOCULL;

		pev->iuser1 = 2;
		pev->iuser2 = RANDOM_LONG(0,1);
		m_flTimeDead = gpGlobals->time + 1.75s;

		pev->frame = 1.0;
		pev->framerate = 1.0;
		pev->animtime = gpGlobals->time;

		vec3_t fRenderColor;
		switch (RANDOM_LONG(0, 3))
		{
		case 1: 
		{
			fRenderColor = { 101.0, 162.0, 253.0 };
			break;
		}
		
		case 2:
		{
			fRenderColor = { 255.0, 153.0, 102.0 };
			break;
		}
		case 3:
		{

			fRenderColor = { 172.0, 251.0, 102.0 };
			break;
		}
		default:
		{
			fRenderColor = { 255.0, 255.0, 255.0 };
			break;
		}
		}
		pev->rendercolor = fRenderColor;

		SetThink(&CFgLauncherEffect::OnThink);

		m_flNumFrames = (float)(MODEL_FRAMES(pev->modelindex) - 1);
		pev->nextthink = gpGlobals->time + 0.01s;

	}
	void CFgLauncherEffect::OnThink(void)
	{
		/*if (pev->frame < 15)
		{
			pev->frame++;
		}*/

		if (pev->movetype == MOVETYPE_FLY)
		{
			pev->origin.z += 1.0;
		}
		pev->origin.x += m_iExpTimes ? RANDOM_FLOAT(0.0, 0.25) : RANDOM_FLOAT(-0.25, 0.0);
		pev->origin.y += m_iExpTimes ? RANDOM_FLOAT(-0.25, 0.0) : RANDOM_FLOAT(0.0, 0.25);

		pev->scale += RANDOM_FLOAT(0.001, 0.005);


		if (pev->iuser1)
		{
			if (m_flTimeDead <= gpGlobals->time)
			{
				SetThink(NULL);
				SetThink(&CBaseEntity::SUB_StartFadeOut);
			}
		}
	}
	void CFgLauncherEffect::Precache(void)
	{
		
	}

	LINK_ENTITY_TO_CLASS(fglauncher_effect, CFgLauncherEffect);

#endif

	LINK_ENTITY_TO_CLASS(weapon_fglauncher, CFgLauncher)

	void CFgLauncher::Spawn(void)
	{
		Precache();
		m_iId = WEAPON_FGLAUNCHER;
		pev->classname = MAKE_STRING("weapon_fglauncher");
		SET_MODEL(ENT(pev), "models/w_fglauncher.mdl");
		m_iDefaultAmmo = 90;

		FallInit();
	}

	void CFgLauncher::Precache(void)
	{
		PRECACHE_MODEL("models/v_fglauncher.mdl");
		PRECACHE_MODEL("models/p_fglauncher.mdl");
		PRECACHE_MODEL("models/w_fglauncher.mdl");

		PRECACHE_SOUND("weapons/fglauncher-1.wav");

		m_usFireFgLauncher = PRECACHE_EVENT(1, "events/fglauncher.sc");
	}

	int CFgLauncher::GetItemInfo(ItemInfo* p)
	{
		p->pszName = STRING(pev->classname);
		p->pszAmmo1 = "FgGrenade";
		p->iMaxAmmo1 = 90;
		p->pszAmmo2 = NULL;
		p->iMaxAmmo2 = -1;
		p->iMaxClip = 10;
		p->iSlot = 0;
		p->iPosition = 12;
		p->iId = m_iId = WEAPON_FGLAUNCHER;
		p->iFlags = 0;
		p->iWeight = M3_WEIGHT;

		return 1;
	}

	BOOL CFgLauncher::Deploy(void)
	{
#ifndef CLIENT_DLL
		m_pPlayer->UpdateShieldCrosshair(false);
#endif

		return DefaultDeploy("models/v_fglauncher.mdl", "models/p_fglauncher.mdl", FGLAUNCHER_DRAW, "m249", UseDecrement() != FALSE);
	}

	void CFgLauncher::SecondaryAttack(void)
	{
		switch (m_pPlayer->m_iFOV)
		{
		case 90: m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 65; break;
		default: m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 90; break;
		}

		m_pPlayer->ResetMaxSpeed();
		EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/zoom.wav", 0.2, 2.4);
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3s;
	}

	void CFgLauncher::PrimaryAttack(void)
	{
		if (m_pPlayer->pev->waterlevel == 3)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15s;
			return;
		}

		if (m_iClip <= 0)
		{
			Reload();

			if (m_iClip == 0)
				PlayEmptySound();

			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.88s;
			return;
		}

		m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
		m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

		m_iClip--;
		m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
		UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

#ifndef CLIENT_DLL
		Vector vecSrc = m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 16 + gpGlobals->v_right * 8 + gpGlobals->v_up * -8;
		CFgLauncherRocket* pEnt = static_cast<CFgLauncherRocket*>(CBaseEntity::Create("fglauncher_rocket", vecSrc, m_pPlayer->pev->v_angle, m_pPlayer->edict()));
		if (pEnt)
		{
			pEnt->m_iTeam = m_pPlayer->m_iTeam;
			UTIL_MakeVectors(m_pPlayer->pev->v_angle);
			if (m_pPlayer->m_iFOV == 90)
				pEnt->pev->velocity += gpGlobals->v_forward * 800;
			else
				pEnt->pev->velocity += gpGlobals->v_forward * 1000;
		}
#endif
		float flVol = VOL_NORM;

		PLAYBACK_EVENT_FULL(0, ENT(m_pPlayer->pev), m_usFireFgLauncher, 0, g_vecZero, g_vecZero, flVol, 0.0735, 7, m_vVecAiming.x * 100, FALSE, TRUE);

#ifndef CLIENT_DLL
		if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
		if (m_iClip)
			m_flPumpTime = UTIL_WeaponTimeBase() + 0.125s;

		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.76s;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.76s;

		if (m_iClip)
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;
		else
			m_flTimeWeaponIdle = 0.75s;

		m_fInSpecialReload = 0;

		if (m_pPlayer->pev->flags & FL_ONGROUND)
		{
			m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 3, 7);
			m_pPlayer->pev->punchangle.y -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, -1, 1);
			m_pPlayer->pev->punchangle.y = 0.0;
		}
			
		else
		{
			m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 7, 10);
			m_pPlayer->pev->punchangle.y -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, -5, 10);
			m_pPlayer->pev->punchangle.y = 0.0;
		}
			
	}

	void CFgLauncher::Reload(void)
	{
		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 || m_iClip == 10)
			return;

		if (m_flNextPrimaryAttack > UTIL_WeaponTimeBase())
			return;

		if (DefaultReload(10, FGLAUNCHER_RELOAD, 5.03s))
		{
#ifndef CLIENT_DLL
			m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
			m_flAccuracy = 0.2;
			m_iShotsFired = 0;
		}

		
	}

	void CFgLauncher::WeaponIdle(void)
	{
		ResetEmptySound();
		m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);

		if (m_flPumpTime != zero_duration && m_flPumpTime < UTIL_WeaponTimeBase())
			m_flPumpTime = zero_duration;

		if (m_flTimeWeaponIdle < UTIL_WeaponTimeBase())
		{
			
				SendWeaponAnim(FGLAUNCHER_IDLE, UseDecrement() != FALSE);

				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20.0s;
		}
	}


}
