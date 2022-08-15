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
#include "z4b_dbarrelamethyst.h"

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

	LINK_ENTITY_TO_CLASS(z4b_dbarrelamethyst, CDBARRELAMETHYST)

		void CDBARRELAMETHYST::Spawn(void)
	{
		Precache();
		m_iId = WEAPON_Z4B_DBARRELAMETHYST;
		SET_MODEL(ENT(pev), "models/w_dbarrel_amethyst.mdl");

		m_iDefaultAmmo = 9999;

		FallInit();
	}

	void CDBARRELAMETHYST::Precache(void)
	{
		PRECACHE_MODEL("models/v_dbarrel_amethyst.mdl");
		PRECACHE_MODEL("models/p_dbarrel_amethyst.mdl");
		PRECACHE_MODEL("models/w_dbarrel_amethyst.mdl");
		PRECACHE_MODEL("sprites/tsf_laser.spr");

		m_iShellId = m_iShell = PRECACHE_MODEL("models/shotgunshell.mdl");

		PRECACHE_SOUND("weapons/dbarrel-1.wav");
		PRECACHE_SOUND("weapons/dbarrel_foley1.wav");
		PRECACHE_SOUND("weapons/dbarrel_foley2.wav");

		m_usFireDBARRELAMETHYS = PRECACHE_EVENT(1, "events/dbarrelamethyst.sc");
	}

	int CDBARRELAMETHYST::GetItemInfo(ItemInfo* p)
	{
		p->pszName = STRING(pev->classname);
		p->pszAmmo1 = "ammo_amethyst";
		p->iMaxAmmo1 = 9999;
		p->pszAmmo2 = NULL;
		p->iMaxAmmo2 = -1;
		p->iMaxClip = 2;
		p->iSlot = 0;
		p->iPosition = 12;
		p->iId = m_iId = WEAPON_Z4B_DBARRELAMETHYST;
		p->iFlags = 0;
		p->iWeight = XM1014_WEIGHT;

		return 1;
	}

	BOOL CDBARRELAMETHYST::Deploy(void)
	{
#ifndef CLIENT_DLL
		if (m_pPlayer->IsAlive())
			CheckWeapon(m_pPlayer, this);
#endif
		m_NextInspect = gpGlobals->time + 0.75s;
		return DefaultDeploy("models/v_dbarrel_amethyst.mdl", "models/p_dbarrel_amethyst.mdl", 2, "m249", UseDecrement() != FALSE);
	}

	void CDBARRELAMETHYST::PrimaryAttack(void)
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

			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.0s;
			return;
		}

		m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
		m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

		m_iClip--;
		m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
		m_NextInspect = gpGlobals->time;
		UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
#ifndef CLIENT_DLL
		m_pPlayer->FireBullets(6, m_pPlayer->GetGunPosition(), gpGlobals->v_forward, Vector(0.0725, 0.0725, 0.0), 3048, BULLET_PLAYER_BUCKSHOT, 0);
#endif
		int flags;
#ifdef CLIENT_WEAPONS
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif

#ifndef CLIENT_DLL
		TraceResult tr;
		UTIL_TraceLine(m_pPlayer->GetGunPosition(), m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 8192.0, dont_ignore_monsters, m_pPlayer->edict(), &tr);

		if (tr.flFraction < 1.0)
		{
			TraceResult CheckHeadShotTR;
			CBaseEntity* pEntity = NULL;

			Vector vecEnd;

			while ((pEntity = UTIL_FindEntityInSphere(pEntity, tr.vecEndPos, 50.0)) != NULL)
			{
				if (pEntity->edict() == m_pPlayer->edict())
					continue;

				if (pEntity->IsDormant())
					continue;

				if (pEntity->pev->takedamage == DAMAGE_NO)
					continue;

				if (pEntity->Classify() == CLASS_NONE)
					continue;

				if (pEntity->IsPlayer())
				{
					CBasePlayer* target = static_cast<CBasePlayer*>(pEntity);

					if (g_pGameRules->PlayerRelationship(m_pPlayer, target) == GR_TEAMMATE)
						continue;
				}

				vecEnd = pEntity->BodyTarget(pev->origin);
				m_vSavePos = vecEnd;

				UTIL_TraceLine(tr.vecEndPos, vecEnd, dont_ignore_monsters, pev->owner, &CheckHeadShotTR);

				if (CheckHeadShotTR.flFraction < 1.0 && CheckHeadShotTR.pHit != pEntity->edict())
					continue;

				if (pEntity->IsAlive())
				{
					if (m_NextThink < gpGlobals->time)
					{
						m_NextThink = gpGlobals->time + 5.0s;
					}
				}
				else
				{
					m_NextThink = invalid_time_point;
					pev->iuser1 = 0;
				}
					
			}
		}
#endif

		PLAYBACK_EVENT_FULL(flags, ENT(m_pPlayer->pev), m_usFireDBARRELAMETHYS, 0, g_vecZero, g_vecZero, m_vVecAiming.x, m_vVecAiming.y, 7, m_vVecAiming.x * 100, m_iClip != 0, FALSE);

		if (m_iClip)
			m_flPumpTime = UTIL_WeaponTimeBase() + 0.125s;

#ifndef CLIENT_DLL
		if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
		if (m_iClip)
			m_flPumpTime = UTIL_WeaponTimeBase() + 0.125s;

		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.25s;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.25s;

		if (m_iClip)
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.25s;
		else
			m_flTimeWeaponIdle = 0.75s;

		m_fInSpecialReload = 0;

		if (m_pPlayer->pev->flags & FL_ONGROUND)
			m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 3, 5);
		else
			m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 7, 10);
	}


	void CDBARRELAMETHYST::Reload(void)
	{

		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 || m_iClip == XM1014_MAX_CLIP)
			return;

		if (DefaultReload(2, 1, 2.5s))
		{
#ifndef CLIENT_DLL
			m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
			m_bDelayFire = false;
			m_iShotsFired = 0;
			m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] = 9999;


		}

		m_flNextReload = UTIL_WeaponTimeBase() + 0.3s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.3s;

	}

	void CDBARRELAMETHYST::WeaponIdle(void)
	{
		ResetEmptySound();
		m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);

		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 70s;
		SendWeaponAnim(0, UseDecrement() != FALSE);
	}

	void CDBARRELAMETHYST::ItemPostFrame(void)
	{
		if (m_NextThink > gpGlobals->time)
		{
			if (pev->iuser1 >= 4)
			{
				pev->iuser1 = 0;
				m_NextThink = invalid_time_point;

				const float flRadius = 250;

				CBaseEntity* pAttacker = CBaseEntity::Instance(pev->owner);
				CBasePlayer* pAttackPlayer = nullptr;
				if (pAttacker && pAttacker->IsPlayer())
					pAttackPlayer = static_cast<CBasePlayer*>(pAttacker);
				if (!pAttackPlayer)
					return CBasePlayerWeapon::ItemPostFrame();

#ifndef CLIENT_DLL
				CBaseEntity* pEntity = NULL;
				CBaseEntity* pStartEntity = NULL;
				pStartEntity = pAttackPlayer;
				CBaseEntity* m_iTarget[3]{};
				float m_flDist[3]{};

				while ((pEntity = UTIL_FindEntityInSphere(pEntity, m_vSavePos? m_vSavePos:pStartEntity->pev->origin, flRadius)) != nullptr)
				{
					if (!pAttackPlayer->IsAlive())
						continue;
#ifndef CLIENT_DLL
					if (g_pGameRules->PlayerRelationship(pAttackPlayer, pEntity) == GR_TEAMMATE)
						continue;
#endif
					if (pEntity->pev == pAttackPlayer->pev)
						continue;

					if (!pEntity->IsAlive())
						continue;

					Vector vecTarget;
					vecTarget = pEntity->pev->origin;

					for (int i = 0; i < 3; i++)
					{
						float flDistance = (pStartEntity->pev->origin - pEntity->pev->origin).Length();
						if (!m_iTarget[i] || m_flDist[i] > flDistance)
						{
							if (i < 2)
							{
								for (int j = 1; j >= i; j--)
								{
									m_iTarget[j + 1] = m_iTarget[j];
									m_flDist[j + 1] = m_flDist[j];
								}
							}
							m_iTarget[i] = pEntity;
							m_flDist[i] = flDistance;

							break;
						}
					}
				}

				for (int i = 0; i < 3; i++)
				{

					if (!m_iTarget[i])
						continue;
					if (!m_iTarget[i]->IsAlive())
						continue;

					Vector vecDirection = (m_iTarget[i]->pev->origin - pAttackPlayer->pev->origin).Normalize();

					TraceResult tr;
					UTIL_TraceLine(pAttackPlayer->pev->origin, m_iTarget[i]->pev->origin, missile, ENT(pAttackPlayer->pev), &tr);

					float flDamageAdjust = 1.0;

#ifndef CLIENT_DLL

					MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, m_pPlayer->pev->origin);
					WRITE_BYTE(TE_BEAMPOINTS);
					WRITE_SHORT(m_pPlayer->entindex() | 0x1000);
					WRITE_COORD(m_iTarget[i]->pev->origin.x);
					WRITE_COORD(m_iTarget[i]->pev->origin.y);
					WRITE_COORD(m_iTarget[i]->pev->origin.z);
					WRITE_SHORT(MODEL_INDEX("sprites/tsf_laser.spr"));
					WRITE_BYTE(1);
					WRITE_BYTE(30);
					WRITE_BYTE(20);
					WRITE_BYTE(60); //Line Width
					WRITE_BYTE(5);
					WRITE_BYTE(255);
					WRITE_BYTE(255);
					WRITE_BYTE(255);
					WRITE_BYTE(255);
					WRITE_BYTE(5);
					MESSAGE_END();

					//UTIL_DrawBeamPoints(m_iTarget[i - 1 < 0 ? 0 : i - 1]->pev->origin, m_iTarget[i]->pev->origin, 5, 255, 50, 50);
					MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, m_iTarget[i - 1 < 0 ? 0 : i - 1]->pev->origin);
					WRITE_BYTE(TE_BEAMPOINTS);
					WRITE_COORD(m_iTarget[i - 1 < 0 ? 0 : i - 1]->pev->origin.x + RANDOM_FLOAT(-10.0,10.0));
					WRITE_COORD(m_iTarget[i - 1 < 0 ? 0 : i - 1]->pev->origin.y + RANDOM_FLOAT(-10.0, 10.0));
					WRITE_COORD(m_iTarget[i - 1 < 0 ? 0 : i - 1]->pev->origin.z + RANDOM_FLOAT(-10.0, 10.0));
					WRITE_COORD(m_iTarget[i]->pev->origin.x);
					WRITE_COORD(m_iTarget[i]->pev->origin.y);
					WRITE_COORD(m_iTarget[i]->pev->origin.z);
					WRITE_SHORT(MODEL_INDEX("sprites/tsf_laser.spr"));
					WRITE_BYTE(1);
					WRITE_BYTE(30);
					WRITE_BYTE(20);
					WRITE_BYTE(60); //Line Width
					WRITE_BYTE(5);
					WRITE_BYTE(255);
					WRITE_BYTE(255);
					WRITE_BYTE(255);
					WRITE_BYTE(255);
					WRITE_BYTE(5);
					MESSAGE_END();

					/*MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, m_iTarget[i]->pev->origin);
					WRITE_BYTE(TE_BEAMPOINTS);
					WRITE_COORD(m_iTarget[i]->pev->origin.x);
					WRITE_COORD(m_iTarget[i]->pev->origin.y);
					WRITE_COORD(m_iTarget[i]->pev->origin.z - 10.0);
					WRITE_COORD(m_iTarget[i]->pev->origin.x);
					WRITE_COORD(m_iTarget[i]->pev->origin.y);
					WRITE_COORD(m_iTarget[i]->pev->origin.z + 2048.0);
					WRITE_SHORT(MODEL_INDEX("sprites/tsf_laser.spr"));
					WRITE_BYTE(1);
					WRITE_BYTE(30);
					WRITE_BYTE(20);
					WRITE_BYTE(60); //Line Width
					WRITE_BYTE(5);
					WRITE_BYTE(255);
					WRITE_BYTE(255);
					WRITE_BYTE(255);
					WRITE_BYTE(255);
					WRITE_BYTE(5);
					MESSAGE_END();
					*/
					MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, m_iTarget[i]->pev->origin);
					WRITE_BYTE(TE_EXPLOSION);
					WRITE_COORD(m_iTarget[i]->pev->origin.x);
					WRITE_COORD(m_iTarget[i]->pev->origin.y);
					WRITE_COORD(m_iTarget[i]->pev->origin.z);
					WRITE_SHORT(MODEL_INDEX("sprites/ame5stack.spr"));
					WRITE_BYTE(5);
					WRITE_BYTE(5);
					WRITE_BYTE(TE_EXPLFLAG_NODLIGHTS | TE_EXPLFLAG_NOPARTICLES | TE_EXPLFLAG_NOSOUND);
					MESSAGE_END();


					ClearMultiDamage();
					m_iTarget[i]->TraceAttack(pAttackPlayer->pev, GetDamage_PrimaryAttack_Instant() * flDamageAdjust, vecDirection, &tr, DMG_BULLET);
					ApplyMultiDamage(pAttackPlayer->pev, pAttackPlayer->pev);

					EMIT_SOUND_DYN(ENT(m_iTarget[i]->pev), CHAN_AUTO, "weapons/divinetitan_A_mode-1_loop.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

#endif
				}

#endif
			}
			return CBasePlayerWeapon::ItemPostFrame();
		}
		return CBasePlayerWeapon::ItemPostFrame();
	}
	double CDBARRELAMETHYST::GetDamage_PrimaryAttack_Instant() const
	{
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZBS)
			return 1092.0;
		if (g_pModRunning->DamageTrack() == DT_ZB)
			return 152.0;
#endif
		return 20.0;
	}
}
