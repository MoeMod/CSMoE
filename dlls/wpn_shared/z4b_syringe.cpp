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
#include "z4b_syringe.h"
#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#include <dlls/gamemode/mod_zb1.h>
#endif


#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

LINK_ENTITY_TO_CLASS(z4b_syringe, CSyringe)

void CSyringe::Spawn(void)
{
	pev->classname = MAKE_STRING("z4b_syringe");

	Precache();
	m_iId = WEAPON_Z4B_SYRINGE;
	SET_MODEL(ENT(pev), "models/z4b/w_syringe.mdl");

	m_iDefaultAmmo = 1;

	FallInit();
}

void CSyringe::Precache(void)
{
	PRECACHE_MODEL("models/z4b/v_syringe.mdl");
	PRECACHE_MODEL("models/null.mdl");
	PRECACHE_SOUND("CK_WPN/WEAPON/NanoMedicalGun/NanoMedicalGun_Combo1.wav");
	PRECACHE_SOUND("CK_WPN/WEAPON/NanoMedicalGun/NanoMedicalGun_Fx.wav");
	PRECACHE_SOUND("CK_WPN/WEAPON/NanoMedicalGun/NanoMedicalGun_Select.wav");
}

int CSyringe::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "syringe";
	p->iMaxAmmo1 = 1;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 0;
	p->iPosition = 3;
	p->iId = m_iId = WEAPON_Z4B_SYRINGE;
	p->iWeight = 3;
	p->iFlags = ITEM_FLAG_LIMITINWORLD | ITEM_FLAG_EXHAUSTIBLE;

	return 1;
}

BOOL CSyringe::Deploy(void)
{
	m_flReleaseThrow = invalid_time_point;
	m_fMaxSpeed = 250;
	m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;
	m_pPlayer->m_bShieldDrawn = false;
	EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "CK_WPN/WEAPON/NanoMedicalGun/NanoMedicalGun_Select.wav", VOL_NORM, ATTN_NORM, 0, 94);
	return DefaultDeploy("models/z4b/v_syringe.mdl", "models/null.mdl", SYRINGE_DRAW, "dualpistols_2", UseDecrement() != FALSE);
}


void CSyringe::FindHullIntersection(const Vector& vecSrc, TraceResult& tr, const vec3_t pflMins, const vec3_t pfkMaxs, edict_t* pEntity)
{
	TraceResult trTemp;
	float flDistance = 1000000;
	const vec3_t pflMinMaxs[2] = { pflMins, pfkMaxs };
	Vector vecHullEnd = tr.vecEndPos;

	vecHullEnd = vecSrc + ((vecHullEnd - vecSrc) * 2);
	TRACE_LINE(vecSrc, vecHullEnd, dont_ignore_monsters, pEntity, &trTemp);

	if (trTemp.flFraction < 1)
	{
		tr = trTemp;
		return;
	}

	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			for (int k = 0; k < 2; k++)
			{
				Vector vecEnd;
				vecEnd.x = vecHullEnd.x + pflMinMaxs[i][0];
				vecEnd.y = vecHullEnd.y + pflMinMaxs[j][1];
				vecEnd.z = vecHullEnd.z + pflMinMaxs[k][2];

				TRACE_LINE(vecSrc, vecEnd, dont_ignore_monsters, pEntity, &trTemp);

				if (trTemp.flFraction < 1)
				{
					float flThisDistance = (trTemp.vecEndPos - vecSrc).Length();

					if (flThisDistance < flDistance)
					{
						tr = trTemp;
						flDistance = flThisDistance;
					}
				}
			}
		}
	}
}

void CSyringe::Holster(int skiplocal)
{
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5s;

	if (!m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
	{
		m_pPlayer->RemoveWeapon(m_iId);
		DestroyItem();
	}
}

void CSyringe::PrimaryAttack(void)
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] > 0)
	{
		SendWeaponAnim(SYRINGE_SHOOT, UseDecrement() != FALSE);
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "CK_WPN/WEAPON/NanoMedicalGun/NanoMedicalGun_Combo1.wav", VOL_NORM, ATTN_NORM, 0, 94);
		if (Swing())
			m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.1s;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.1s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;
		

		if (!m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
			m_flTimeWeaponIdle = m_flNextSecondaryAttack = m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.5s;
		ResetRunIdle();
		return;
	}
}

int CSyringe::Swing()
{
	BOOL fDidHit = FALSE;
#ifndef CLIENT_DLL

	strcpy(m_pPlayer->m_szCustomSequence, "M-combo1_MedicalGun_Cure");
	m_pPlayer->pev->frame = 0;
	m_pPlayer->ResetSequenceInfo();
	m_pPlayer->m_flCustomSequenceUntilTime = gpGlobals->time + 0.9s;

	UTIL_MakeVectors(m_pPlayer->pev->v_angle);
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecEnd = vecSrc + gpGlobals->v_forward * 55;

	TraceResult tr;
	UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr);

	if (tr.flFraction >= 1)
	{
		UTIL_TraceHull(vecSrc, vecEnd, dont_ignore_monsters, head_hull, ENT(m_pPlayer->pev), &tr);

		if (tr.flFraction < 1)
		{
			CBaseEntity* pHit = CBaseEntity::Instance(tr.pHit);

			if (!pHit || pHit->IsBSPModel())
				FindHullIntersection(vecSrc, tr, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX, ENT(m_pPlayer->pev));

			vecEnd = tr.vecEndPos;
		}
	}

	if (tr.flFraction < 1)
	{
		CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);
		if (pEntity && pEntity->IsPlayer())
		{
			fDidHit = TRUE;
			CBasePlayer* player = static_cast<CBasePlayer*>(pEntity);

			if (player->m_bIsZombie)
			{
				player->m_iTeam = CT;
				player->m_bIsZombie = false;
				player->m_bNotKilled = false;
				player->m_iBuff = 0;

				player->SetScoreboardAttributes();

				MESSAGE_BEGIN(MSG_ALL, gmsgTeamInfo);
				WRITE_BYTE(entindex());
				switch (player->m_iTeam)
				{
				case CT:
					WRITE_STRING("CT");
					break;
				case TERRORIST:
					WRITE_STRING("TERRORIST");
					break;
				case SPECTATOR:
					WRITE_STRING("SPECTATOR");
					break;
				default:
					WRITE_STRING("UNASSIGNED");
					break;
				}
				MESSAGE_END();

				player->UpdateLocation(true);

				MESSAGE_BEGIN(MSG_ALL, gmsgScoreInfo);
				WRITE_BYTE(ENTINDEX(edict()));
				WRITE_SHORT((int)pev->frags);
				WRITE_SHORT(player->m_iDeaths);
				WRITE_SHORT(0);
				WRITE_SHORT(player->m_iTeam);
				MESSAGE_END();

				TeamChangeUpdate(player, player->m_iTeam);

				TheBots->OnEvent(EVENT_PLAYER_CHANGED_TEAM, player);

				player->SetPlayerModel(false);

				player->pev->renderfx = kRenderFxNone;
				player->pev->rendercolor = { 255,255,255 };
				player->pev->renderamt = 255;

				auto strategy = dynamic_cast<CPlayerModStrategy_ZB1*>(player->m_pModStrategy.get());
				if (strategy)
				{
					strategy->OnSpawn();
				}
				player->GiveDefaultItems();

				g_pGameRules->CheckWinConditions();

				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "CK_WPN/WEAPON/NanoMedicalGun/NanoMedicalGun_Fx.wav", VOL_NORM, ATTN_NORM, 0, 94);
			}
			else
			{
				float flOldHealth = player->pev->health;
				player->pev->health += 1000;
				player->pev->health = min(player->pev->health, player->pev->max_health);

				if (flOldHealth >= player->pev->health)
					player->pev->armorvalue += 1000;

				EMIT_SOUND_DYN(ENT(player->pev), CHAN_AUTO, "zombi/td_buff.wav", 0.5, ATTN_NORM, 0, PITCH_NORM);
			}
			MESSAGE_BEGIN(MSG_ALL, gmsgMPToCL, NULL, player->pev);
			WRITE_BYTE(4);
			WRITE_SHORT(player->entindex());
			WRITE_BYTE(2);
			MESSAGE_END();

			MESSAGE_BEGIN(MSG_ALL, gmsgMPToCL, NULL, player->pev);
			WRITE_BYTE(4);
			WRITE_SHORT(player->entindex());
			WRITE_BYTE(3);
			MESSAGE_END();
		}
	}
#endif
	return fDidHit;
}

void CSyringe::WeaponIdle(void)
{
	ResetEmptySound();
	if (m_flTimeWeaponIdle < UTIL_WeaponTimeBase())
	{
		if (!m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
		{
			RetireWeapon();
			return;
		}

		if (m_pPlayer->pev->button & (IN_FORWARD | IN_MOVELEFT | IN_MOVERIGHT | IN_BACK))
		{
			if (m_flNextRunIdle <= UTIL_WeaponTimeBase())
			{
				SendWeaponAnim(SYRINGE_RUN, UseDecrement() != FALSE);

				m_flNextRunIdle = UTIL_WeaponTimeBase() + 0.6s;
			}

			m_flNextIdle = UTIL_WeaponTimeBase();
		}
		else
		{
			if (m_flNextIdle <= UTIL_WeaponTimeBase())
			{
				SendWeaponAnim(SYRINGE_IDLE, UseDecrement() != FALSE);

				m_flNextIdle = UTIL_WeaponTimeBase() + 20s;
			}

			m_flNextRunIdle = UTIL_WeaponTimeBase();
		}

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.1s;
	}

}

BOOL CSyringe::CanDeploy(void)
{
	return m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] != 0;
}

void CSyringe::ResetRunIdle()
{
	m_flNextIdle = m_flNextRunIdle = UTIL_WeaponTimeBase();
}

}
