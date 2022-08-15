#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif


namespace sv {
	LINK_ENTITY_TO_CLASS(lockongun_missile, CLockOnGunMissile)

	CLockOnGunMissile* CLockOnGunMissile::Create(int iType, const Vector& vecOrigin, const Vector& vecAngles, edict_t* pentOwner, CBaseEntity* pEntity, int iHitGroup, int iTeam)
	{
		edict_t* pent = CREATE_NAMED_ENTITY(MAKE_STRING("lockongun_missile"));

		if (FNullEnt(pent))
		{
			ALERT(at_console, "NULL Ent in Create!\n");
			return NULL;
		}

		CLockOnGunMissile* pMissile = (CLockOnGunMissile*)Instance(pent);

		if (pMissile)
		{		
			pMissile->m_iType = iType;
			pMissile->pev->owner = pentOwner;
			pMissile->pev->origin = vecOrigin;
			pMissile->pev->angles = vecAngles;
			pMissile->m_iTeam = iTeam;
			pMissile->m_iHitGroup = iHitGroup;
			pMissile->m_pEntity = pEntity;
			pMissile->Spawn();
		}

		return pMissile;
	}

	void CLockOnGunMissile::Spawn(void)
	{
		Precache();

		SetTouch(&CLockOnGunMissile::MissileTouch);
		SetThink(&CLockOnGunMissile::MissileThink);

		pev->nextthink = gpGlobals->time + 0.1s;
		pev->solid = SOLID_BBOX;
		pev->movetype = MOVETYPE_FLY;
		pev->rendermode = kRenderTransAdd;
		pev->renderamt = 255;

		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_BEAMFOLLOW);
		WRITE_SHORT(entindex());
		WRITE_SHORT(m_iTail);
		WRITE_BYTE(7);
		WRITE_BYTE(2);
		WRITE_BYTE(255);
		WRITE_BYTE(255);
		WRITE_BYTE(255);
		WRITE_BYTE(150);
		MESSAGE_END();

		SET_MODEL(ENT(pev), "models/lockongun_bullet.mdl");

		UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));
		
	}

	void CLockOnGunMissile::Precache(void)
	{
		//WEAPON_M3DRAGONM
		m_iTail = PRECACHE_MODEL("sprites/ef_lockongun_trail.spr");	//flying
		m_iExp[0] = PRECACHE_MODEL("sprites/ef_lockongun_explosion01.spr");
		m_iExp[1] = PRECACHE_MODEL("sprites/ef_lockongun_explosion02.spr");
		m_iExp[2] = PRECACHE_MODEL("sprites/ef_lockongun_explosion03.spr");

		//WEAPON_M3DRAGON
		PRECACHE_MODEL("models/lockongun_bullet.mdl");
		PRECACHE_SOUND("weapons/lockongun_exp.wav");
	}

	int CLockOnGunMissile::GetWeaponsId(void) { return WEAPON_LOCKONGUN; }


	void CLockOnGunMissile::MissileThink()
	{
		pev->nextthink = gpGlobals->time + 0.1s;

		if (m_pEntity != nullptr)
		{
			if (m_pEntity->IsAlive())
			{
				Vector vecDir = (m_pEntity->pev->origin - pev->origin).Normalize();
				pev->velocity = vecDir * 1000;
			}	
		}

		if (!IsInWorld())
		{
			MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
			WRITE_BYTE(TE_KILLBEAM);
			WRITE_SHORT(this->entindex());
			MESSAGE_END();

			UTIL_Remove(this);
			return;
		}
	}

	bool CLockOnGunMissile::RadiusDamage(Vector vecSrc, entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, float flRadius, int iClassIgnore, int bitsDamageType)
	{
		CBaseEntity* pEntity = NULL;
		TraceResult tr;
		Vector vecSpot;

		int bInWater = (UTIL_PointContents(vecSrc) == CONTENTS_WATER);
		bool bHit = false;

		vecSrc.z += 1;

		if (!pevAttacker)
			pevAttacker = pevInflictor;

		while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, flRadius)) != NULL)
		{
			if (pEntity->pev->takedamage == DAMAGE_NO)
				continue;

			if (iClassIgnore != CLASS_NONE && pEntity->Classify() == iClassIgnore)
				continue;

			if (bInWater && !pEntity->pev->waterlevel)
				continue;

			if (!bInWater && pEntity->pev->waterlevel == 3)
				continue;

			if (!pEntity->IsPlayer())
				continue;

			if (pEntity->pev == pevAttacker)
				continue;

			vecSpot = pEntity->BodyTarget(vecSrc);
			UTIL_TraceLine(vecSrc, vecSpot, dont_ignore_monsters, ENT(pevInflictor), &tr);

			if (tr.flFraction == 1 || tr.pHit == pEntity->edict())
			{
				pEntity->TakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType);

				if (m_pEntity == nullptr)
				{
					Vector vecDirection = (pEntity->pev->origin - pevAttacker->origin).Normalize();
					UTIL_AddKickRateBoost(pEntity, vecDirection.Make2D(), 1.0, 2000.0, 360);
				}

				if (pEntity->pev->health > 0.0)
					continue;

				if (pEntity != CBaseEntity::Instance(pevAttacker))
					bHit = true;
			}
		}

		return bHit;
	}

	void CLockOnGunMissile::MissileTouch(CBaseEntity* pOther)
	{
		if (!pOther)
			return;

		if (pOther->edict() == pev->owner)
			return;

		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_KILLBEAM);
		WRITE_SHORT(this->entindex());
		MESSAGE_END();

		float flDamage = GetDirectDamage();
		float flExplodeDamage = GetExplodeDamage();

		entvars_t* pevOwner = VARS(pev->owner);

		if (!pOther->IsDormant())
		{
			if (pOther->pev->takedamage != DAMAGE_NO)
			{
				TraceResult tr = UTIL_GetGlobalTrace();

				ClearMultiDamage();
				if (m_iHitGroup != -1)
					tr.iHitgroup = m_iHitGroup;
				else
					tr.iHitgroup = HITGROUP_CHEST;
				pOther->TraceAttack(pevOwner, flDamage, pev->velocity, &tr, DMG_BULLET | DMG_NEVERGIB);
				ApplyMultiDamage(pev, pevOwner);

				if (m_pEntity == nullptr)
				{
					Vector vecDirection = (pOther->pev->origin - pevOwner->origin).Normalize();
					UTIL_AddKickRateBoost(pOther, vecDirection.Make2D(), 1.0, 2000.0, 360);
				}
			}
		}

		RadiusDamage(pev->origin, pev, pevOwner, flExplodeDamage, 100.0, CLASS_NONE, DMG_EXPLOSION | DMG_NEVERGIB);

		MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
		WRITE_BYTE(TE_EXPLOSION);
		WRITE_COORD(pev->origin.x);
		WRITE_COORD(pev->origin.y);
		WRITE_COORD(pev->origin.z);
		WRITE_SHORT(m_iExp[1]);
		WRITE_BYTE(5); // scale * 10
		WRITE_BYTE(45); // framerate
		WRITE_BYTE(TE_EXPLFLAG_NOSOUND);
		MESSAGE_END();

		EMIT_SOUND_DYN(ENT(pev), CHAN_AUTO, "weapons/lockongun_exp.wav", VOL_NORM, 0.25, 0, PITCH_NORM);

		SetTouch(NULL);
		SetThink(NULL);

		UTIL_Remove(this);
	}

	float CLockOnGunMissile::GetDirectDamage()
	{
		float flDamage;
		if (m_iType)
		{
			flDamage = 7;
			if (g_pModRunning->DamageTrack() == DT_ZB)
			{
				if (enhancedzombie.value)
					flDamage = 850;
				else
					flDamage = 600;
			}		
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 1400;
		}
		else
		{
			flDamage = 7;
			if (g_pModRunning->DamageTrack() == DT_ZB)
			{
				if (enhancedzombie.value)
					flDamage = 400;
				else
					flDamage = 300;
			}
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 700;
		}
		return flDamage;
	}

	float CLockOnGunMissile::GetExplodeDamage()
	{	
		float flDamage;
		if (m_iType)
		{
			flDamage = 7;
			if (g_pModRunning->DamageTrack() == DT_ZB)
			{
				if (enhancedzombie.value)
					flDamage = 400;
				else
					flDamage = 300;
			}
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 700;
		}
		else
		{
			flDamage = 100;
			if (g_pModRunning->DamageTrack() == DT_ZB)
			{
				if (enhancedzombie.value)
					flDamage = 400;
				else
					flDamage = 300;
			}
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 1000;
		}
		return flDamage;
	}
}
