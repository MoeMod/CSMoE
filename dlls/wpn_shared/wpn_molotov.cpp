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
#include "wpn_molotov.h"
#include "monsters.h"
#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

#ifndef CLIENT_DLL
	class CMolotovExplosion : public CBaseEntity
	{
	public:
		void Spawn() override
		{
			Precache();
			pev->classname = MAKE_STRING("molotov");

			switch (RANDOM_LONG(0, 2))
			{
			case 0:SET_MODEL(this->edict(), "sprites/flame1.spr"); break;
			case 1:SET_MODEL(this->edict(), "sprites/flame2.spr"); break;
			case 2:SET_MODEL(this->edict(), "sprites/flame3.spr"); break;
			}
			EMIT_SOUND_DYN(this->edict(), CHAN_WEAPON, "weapons/molotov-1.wav", VOL_NORM, 1.3, 0, PITCH_NORM);
			pev->rendermode = kRenderTransAdd; // 5;
			pev->renderfx = kRenderFxNone;
			pev->renderamt = 100.0;
			pev->scale = 0.5;
			m_flMaxFrames = 17.0;
			m_iAttackStep = 45;
			flife = gpGlobals->time;
			SetThink(&CMolotovExplosion::OnThink);
			pev->nextthink = gpGlobals->time + 0.1s;
			pev->framerate = 17;

			pev->solid = SOLID_BBOX; // 2
		}


		void Precache() override
		{
			PRECACHE_MODEL("sprites/flame1.spr");
			PRECACHE_MODEL("sprites/flame2.spr");
			PRECACHE_MODEL("sprites/flame3.spr");
		}

		void FireThink()
		{
			
		}
		void OnThink()
		{
			this->pev->frame = (float)(this->pev->framerate * gpGlobals->frametime / 1s) + this->pev->frame;
			if (pev->frame > m_flMaxFrames)
			{
				pev->frame = fmod(pev->frame, m_flMaxFrames);
			}
			Vector origin = pev->origin;
			origin[2] = origin[2] - 50.0;
			if (g_engfuncs.pfnPointContents(origin) != CONTENTS_SOLID)
			{
				pev->origin[2] -= RANDOM_FLOAT(2.0, 3.0);
			}
			pev->nextthink = gpGlobals->time + 0.0099999998s;
			m_iAttackStep += 1;
			if (m_iAttackStep == 60)
			{
				RadiusDamage(origin, FireDamage());
				m_iAttackStep = 0;
			}
			if (flife + 8.0s < gpGlobals->time)
				Remove();
		}

		time_point_t flife;
		int m_iAttackStep;
		float m_flMaxFrames;


	protected:
		void Remove()
		{
			SetThink(nullptr);
			pev->effects |= EF_NODRAW; // 0x80u
			return UTIL_Remove(this);
		}


		float FireDamage()
		{
			if (g_pModRunning->DamageTrack() == DT_ZBS)
				return 35.0f;
			if (g_pModRunning->DamageTrack() == DT_ZB)
				return 35.0f;
			return 10.0f;
		}

		void RadiusDamage(Vector vecAiming, float flDamage)
		{
			float flRadius = 85.0f;
			if (g_pModRunning->DamageTrack() == DT_ZBS)
				flRadius = 140.0f;
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flRadius = 125.0f;	
			const Vector vecSrc = vecAiming;
			entvars_t* const pevAttacker = VARS(pev->owner);
			entvars_t* const pevInflictor = this->pev;
			int bitsDamageType = DMG_FALL;

			TraceResult tr;
			const float falloff = flRadius ? flDamage / flRadius : 1;
			const int bInWater = (UTIL_PointContents(vecSrc) == CONTENTS_WATER);

			CBaseEntity* pEntity = NULL;
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

						/*CBasePlayer *pVictim = dynamic_cast<CBasePlayer *>(pEntity);
						if (pVictim->m_bIsZombie) // Zombie Knockback...
						{
						ApplyKnockbackData(pVictim, vecSpot - vecSrc, GetKnockBackData());
						}*/
					}
				}
			}

		}
	};

	LINK_ENTITY_TO_CLASS(molotov_explosion, CMolotovExplosion);
#endif


enum molotov_e
{
	SMOKEGRENADE_IDLE,
	SMOKEGRENADE_PINPULL,
	SMOKEGRENADE_THROW,
	SMOKEGRENADE_DRAW
};

LINK_ENTITY_TO_CLASS(weapon_molotov, CMolotov)

void CMolotov::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_molotov");

	Precache();
	m_iId = WEAPON_SMOKEGRENADE;
	SET_MODEL(ENT(pev), "models/w_molotov.mdl");

	pev->dmg = 4;
	m_iDefaultAmmo = SMOKEGRENADE_DEFAULT_GIVE;
	m_flStartThrow = invalid_time_point;
	m_flReleaseThrow = invalid_time_point;
	m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;

	FallInit();
}

void CMolotov::Precache(void)
{
	PRECACHE_MODEL("models/v_molotov.mdl");
	PRECACHE_MODEL("models/p_molotov.mdl");
	PRECACHE_SOUND("weapons/molotov-1.wav");
	PRECACHE_SOUND("weapons/molotov-2.wav");
	PRECACHE_SOUND("weapons/molotov_hit1.wav");
	PRECACHE_SOUND("weapons/molotov_hit2.wav");
	PRECACHE_SOUND("weapons/molotov_hit3.wav");
#ifdef ENABLE_SHIELD
	PRECACHE_MODEL("models/shield/v_shield_smokegrenade.mdl");
#endif
}

int CMolotov::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "SmokeGrenade";
	p->iMaxAmmo1 = MAX_AMMO_SMOKEGRENADE;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 3;
	p->iPosition = 3;
	p->iId = m_iId = WEAPON_SMOKEGRENADE;
	p->iWeight = SMOKEGRENADE_WEIGHT;
	p->iFlags = ITEM_FLAG_LIMITINWORLD | ITEM_FLAG_EXHAUSTIBLE;

	return 1;
}

BOOL CMolotov::Deploy(void)
{
	m_flReleaseThrow = invalid_time_point;
	m_fMaxSpeed = 250;
	m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;
	m_pPlayer->m_bShieldDrawn = false;
#ifdef ENABLE_SHIELD
	if (m_pPlayer->HasShield() != false)
		return DefaultDeploy("models/shield/v_shield_smokegrenade.mdl", "models/shield/p_shield_smokegrenade.mdl", SMOKEGRENADE_DRAW, "shieldgren", UseDecrement() != FALSE);
	else
#endif
		return DefaultDeploy("models/v_molotov.mdl", "models/p_molotov.mdl", SMOKEGRENADE_DRAW, "grenade", UseDecrement() != FALSE);
}

void CMolotov::Holster(int skiplocal)
{
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5s;

	if (!m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
	{
		m_pPlayer->pev->weapons &= ~(1 << WEAPON_SMOKEGRENADE);
		DestroyItem();
	}

	m_flStartThrow = invalid_time_point;
	m_flReleaseThrow = invalid_time_point;
}

void CMolotov::PrimaryAttack(void)
{
	if (m_iWeaponState & WPNSTATE_SHIELD_DRAWN)
		return;

	if (m_flStartThrow == invalid_time_point && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] > 0)
	{
		m_flStartThrow = gpGlobals->time;
		m_flReleaseThrow = invalid_time_point;
		SendWeaponAnim(SMOKEGRENADE_PINPULL, UseDecrement() != FALSE);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5s;
	}
}

void CMolotov::SetPlayerShieldAnim(void)
{
	if (m_pPlayer->HasShield() == true)
	{
		if (m_iWeaponState & WPNSTATE_SHIELD_DRAWN)
			strcpy(m_pPlayer->m_szAnimExtention, "shield");
		else
			strcpy(m_pPlayer->m_szAnimExtention, "shieldgren");
	}
}

void CMolotov::ResetPlayerShieldAnim(void)
{
	if (m_pPlayer->HasShield() == true)
	{
		if (m_iWeaponState & WPNSTATE_SHIELD_DRAWN)
			strcpy(m_pPlayer->m_szAnimExtention, "shieldgren");
	}
}

bool CMolotov::ShieldSecondaryFire(int up_anim, int down_anim)
{
	if (m_pPlayer->HasShield() == false)
		return false;

	if (m_flStartThrow != invalid_time_point)
		return false;

	if (m_iWeaponState & WPNSTATE_SHIELD_DRAWN)
	{
		m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;
		SendWeaponAnim(down_anim, UseDecrement() != FALSE);
		strcpy(m_pPlayer->m_szAnimExtention, "shieldgren");
		m_fMaxSpeed = 250;
		m_pPlayer->m_bShieldDrawn = false;
	}
	else
	{
		m_iWeaponState |= WPNSTATE_SHIELD_DRAWN;
		SendWeaponAnim(up_anim, UseDecrement() != FALSE);
		strcpy(m_pPlayer->m_szAnimExtention, "shielded");
		m_fMaxSpeed = 180;
		m_pPlayer->m_bShieldDrawn = true;
	}
#ifndef CLIENT_DLL
	m_pPlayer->UpdateShieldCrosshair((m_iWeaponState & WPNSTATE_SHIELD_DRAWN) == 0);
	m_pPlayer->ResetMaxSpeed();
#endif
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.4s;
	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.4s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.6s;
	return true;
}

void CMolotov::SecondaryAttack(void)
{
	ShieldSecondaryFire(SHIELDGUN_DRAW, SHIELDGUN_DRAWN_IDLE);
}

void CMolotov::WeaponIdle(void)
{
	if (m_flReleaseThrow == invalid_time_point && m_flStartThrow != invalid_time_point)
		m_flReleaseThrow = gpGlobals->time;

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	if (m_flStartThrow != invalid_time_point)
	{
#ifndef CLIENT_DLL	
		if ((int)CVAR_GET_FLOAT("mp_csgospecialeffect"))
		{
			switch (RANDOM_LONG(0, 2))
			{
			case 0:m_pPlayer->Radio("%!MRAD_MOLOTOV01", "#Molotov_out"); break;
			case 1:m_pPlayer->Radio("%!MRAD_MOLOTOV02", "#Molotov_out"); break;
			case 2:m_pPlayer->Radio("%!MRAD_MOLOTOV03", "#Molotov_out"); break;
			case 3:m_pPlayer->Radio("%!MRAD_MOLOTOV04", "#Molotov_out"); break;
			}
		}
		else
			m_pPlayer->Radio("%!MRAD_FIREINHOLE", "#Fire_in_the_hole");
#endif
		Vector angThrow = m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle;

		if (angThrow.x < 0)
			angThrow.x = -10 + angThrow.x * ((90 - 10) / 90.0);
		else
			angThrow.x = -10 + angThrow.x * ((90 + 10) / 90.0);

		float flVel = (90 - angThrow.x) * 6;

		if (flVel > 750)
			flVel = 750;

		UTIL_MakeVectors(angThrow);
		Vector vecSrc = m_pPlayer->pev->origin + m_pPlayer->pev->view_ofs + gpGlobals->v_forward * 16;
		Vector vecThrow = gpGlobals->v_forward * flVel + m_pPlayer->pev->velocity;
		auto time = 2s;
		CGrenade::ShootMolotov(m_pPlayer->pev, vecSrc, vecThrow, time, NULL);

		SendWeaponAnim(SMOKEGRENADE_THROW, UseDecrement() != FALSE);
		SetPlayerShieldAnim();

#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
		m_flStartThrow = invalid_time_point;
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.5s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.75s;
		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;

		if (!m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
			m_flTimeWeaponIdle = m_flNextSecondaryAttack = m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.5s;

		ResetPlayerShieldAnim();
		return;
	}
	else if (m_flReleaseThrow != invalid_time_point)
	{
		m_flStartThrow = invalid_time_point;

		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
		{
			SendWeaponAnim(SMOKEGRENADE_DRAW, UseDecrement() != FALSE);
			m_flReleaseThrow = invalid_time_point;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + RandomDuration<float>(10s, 15s);
		}
		else
			RetireWeapon();

		return;
	}

	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
	{
		if (m_pPlayer->HasShield() != false)
		{
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20.0s;

			if (m_iWeaponState & WPNSTATE_SHIELD_DRAWN)
				SendWeaponAnim(SHIELDREN_IDLE, UseDecrement() != FALSE);

			return;
		}

		int iAnim;
		float flRand = RANDOM_FLOAT(0, 1);

		if (flRand <= 0.75)
		{
			iAnim = SMOKEGRENADE_IDLE;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + RandomDuration<float>(10s, 15s);
		}
		else
		{
			iAnim = SMOKEGRENADE_IDLE;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 75.0s / 30.0;
		}

		SendWeaponAnim(iAnim, UseDecrement() != FALSE);
	}
}

BOOL CMolotov::CanDeploy(void)
{
	return m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] != 0;
}

}
