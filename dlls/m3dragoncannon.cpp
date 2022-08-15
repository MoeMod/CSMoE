#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif


namespace sv {
	LINK_ENTITY_TO_CLASS(m3dragoncannon, CM3DragonCannon)

	CM3DragonCannon* CM3DragonCannon::Create(int iType, const Vector& vecOrigin, const Vector& vecAngles, edict_t* pentOwner, int iTeam)
	{
		edict_t* pent = CREATE_NAMED_ENTITY(MAKE_STRING("m3dragoncannon"));

		if (FNullEnt(pent))
		{
			ALERT(at_console, "NULL Ent in Create!\n");
			return NULL;
		}

		CM3DragonCannon* pCannon = (CM3DragonCannon*)Instance(pent);

		if (pCannon)
		{		
			pCannon->m_iType = iType;
			pCannon->pev->owner = pentOwner;
			pCannon->pev->origin = vecOrigin;
			pCannon->pev->angles = vecAngles;
			pCannon->m_iTeam = iTeam;
			pCannon->Spawn();
		}

		return pCannon;
	}

	void CM3DragonCannon::Spawn(void)
	{
		Precache();

		SetTouch(&CM3DragonCannon::FlyingTouch);
		SetThink(&CM3DragonCannon::FlyingThink);

		pev->nextthink = gpGlobals->time + 0.1s;
		pev->solid = SOLID_BBOX;
		pev->movetype = MOVETYPE_FLY;
		pev->rendermode = kRenderTransAdd;
		pev->renderamt = 255;

		if (m_iType)
		{
			m_flNumFrames = 300.0;
			pev->scale = 0.1;
			SET_MODEL(ENT(pev), "sprites/ef_m3water.spr");

		}
		else
		{
			SET_MODEL(ENT(pev), "models/ef_fireball2.mdl");
		}
			
		UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));
		
	}

	void CM3DragonCannon::Precache(void)
	{
		//WEAPON_M3DRAGONM
		PRECACHE_MODEL("sprites/ef_m3water.spr");	//flying
		m_iM3DragonmExp = PRECACHE_MODEL("sprites/ef_m3waterbomb.spr");	//exp

		//WEAPON_M3DRAGON
		PRECACHE_MODEL("models/ef_fireball2.mdl");
		PRECACHE_SOUND("weapons/m3dragon_exp.wav");
	}

	int CM3DragonCannon::GetWeaponsId(void) { return m_iType ? WEAPON_M3DRAGONM : WEAPON_M3DRAGON; }


	void CM3DragonCannon::FlyingThink()
	{
		pev->nextthink = gpGlobals->time + 0.1s;

		if (m_iType)
		{
			this->pev->frame = (float)(this->pev->framerate * gpGlobals->frametime / 1s) + this->pev->frame;
			if (pev->frame > m_flNumFrames)
			{
				pev->frame = fmod(pev->frame, m_flNumFrames);
			}	
		}

		if (!IsInWorld())
		{
			UTIL_Remove(this);
			return;
		}
	}

	bool CM3DragonCannon::RadiusDamage(Vector vecSrc, entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, float flRadius, int iClassIgnore, int bitsDamageType)
	{
		CBaseEntity* pEntity = NULL;
		TraceResult tr;
		float flAdjustedDamage, falloff;
		Vector vecSpot;

		if (flRadius)
		{
			falloff = flDamage / flRadius;

			if (g_pModRunning->DamageTrack() == DT_ZB)
				falloff *= 0.3;
		}
		else
			falloff = 1;

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
				break;

			if (pEntity->pev == pevAttacker)
				flDamage *= 0.3;

			vecSpot = pEntity->BodyTarget(vecSrc);
			UTIL_TraceLine(vecSrc, vecSpot, dont_ignore_monsters, ENT(pevInflictor), &tr);

			if (tr.flFraction == 1 || tr.pHit == pEntity->edict())
			{
				bool bDamageRate = false;
				float flDamageRate;

				if ((bitsDamageType & DMG_EXPLOSION) && false /* UTIL_IsGame("czero") */)
				{
					bDamageRate = true;
					flDamageRate = UTIL_CalculateDamageRate(vecSrc, pEntity);
				}
				else
				{
					flDamageRate = 1.0;
				}

				if (bDamageRate)
				{
					float dis = flRadius - (vecSrc - pEntity->pev->origin).Length();

					if (!flRadius)
						flRadius = flDamage;

					if (!flRadius)
						continue;

					flAdjustedDamage = (dis * dis) * 1.25 / (flRadius * flRadius) * flDamageRate * flDamage * 1.5;
				}
				else
				{
					flAdjustedDamage = (vecSrc - pEntity->pev->origin).Length() * falloff;
					flAdjustedDamage = flDamage - flAdjustedDamage;
				}

				if (flAdjustedDamage < 0.0)
					continue;

				pEntity->TakeDamage(pevInflictor, pevAttacker, flAdjustedDamage, bitsDamageType);

				if (pEntity->pev->health > 0.0)
					continue;

				if (pEntity != CBaseEntity::Instance(pevAttacker))
					bHit = true;
			}
		}

		return bHit;
		

	}

	void CM3DragonCannon::FlyingTouch(CBaseEntity* pOther)
	{
		if (!pOther)
			return;

		if (pOther->edict() == pev->owner)
			return;

		float flDamage = GetDirectDamage();
		float flExplodeDamage = GetExplodeDamage();

		entvars_t* pevOwner = VARS(pev->owner);

		if (!pOther->IsDormant())
		{
			if (pOther->pev->takedamage != DAMAGE_NO)
			{
				TraceResult tr = UTIL_GetGlobalTrace();

				ClearMultiDamage();
				tr.iHitgroup = HITGROUP_CHEST;
				pOther->TraceAttack(pevOwner, flDamage, pev->velocity, &tr, DMG_BULLET | DMG_NEVERGIB);
				ApplyMultiDamage(pev, pevOwner);
			}
		}

		RadiusDamage(pev->origin, pev, pevOwner, flExplodeDamage, 1.3 * 39.37, CLASS_NONE, DMG_EXPLOSION | DMG_NEVERGIB);

		Vector vecDir = pev->velocity.Normalize();
		Vector vecOrigin = pev->origin;
		vecOrigin -= vecDir * 40;
		TraceResult tr;
		Vector vecEnd;

		vecEnd = vecOrigin;
		vecEnd.z -= 8192;

		UTIL_TraceLine(vecOrigin, vecEnd, dont_ignore_monsters, ENT(pev), &tr);
		vecOrigin = tr.vecEndPos;

		if (!m_iType)
		{
			MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
			WRITE_BYTE(TE_EXPLOSION);	// This makes a dynamic light and the explosion sprites/sound
			WRITE_COORD(pev->origin.x);		// Send to PAS because of the sound
			WRITE_COORD(pev->origin.y);
			WRITE_COORD(pev->origin.z + 20.0f);
			WRITE_SHORT(g_sModelIndexFireball3);
			WRITE_BYTE(25);			// scale * 10
			WRITE_BYTE(30);		// framerate
			WRITE_BYTE(TE_EXPLFLAG_NONE);	// flags
			MESSAGE_END();

			MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
			WRITE_BYTE(TE_EXPLOSION);	// This makes a dynamic light and the explosion sprites/sound
			WRITE_COORD(pev->origin.x + RANDOM_FLOAT(-64, 64));	// Send to PAS because of the sound
			WRITE_COORD(pev->origin.y + RANDOM_FLOAT(-64, 64));
			WRITE_COORD(pev->origin.z + RANDOM_FLOAT(30, 35));
			WRITE_SHORT(g_sModelIndexFireball2);
			WRITE_BYTE(30);			// scale * 10
			WRITE_BYTE(30);		// framerate
			WRITE_BYTE(TE_EXPLFLAG_NONE);	// flags
			MESSAGE_END();

			CM3DragonCannon_Dragon* pDragon = CM3DragonCannon_Dragon::Create(0, vecOrigin, g_vecZero, pev->owner, m_iTeam);
		}
		else
		{
			MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
			WRITE_BYTE(TE_TEMPSPRITE);
			WRITE_COORD(pev->origin.x);
			WRITE_COORD(pev->origin.y);
			WRITE_COORD(pev->origin.z + 20);
			WRITE_SHORT(m_iM3DragonmExp);
			WRITE_BYTE(25);
			WRITE_BYTE(255);
			WRITE_BYTE(30);
			MESSAGE_END();

			CM3DragonCannon_Dragon* pDragon = CM3DragonCannon_Dragon::Create(1, vecOrigin, g_vecZero, pev->owner, m_iTeam);
		}

		EMIT_SOUND_DYN(ENT(pev), CHAN_AUTO, "weapons/m3dragon_exp.wav", VOL_NORM, 0.25, 0, PITCH_NORM);

		SetTouch(NULL);
		SetThink(NULL);

		UTIL_Remove(this);
	}

	float CM3DragonCannon::GetDirectDamage()
	{
		float flDamage;
		if (m_iType)
		{
			flDamage = 100;
			if (g_pModRunning->DamageTrack() == DT_ZB)
			{
				if (enhancedzombie.value)
					flDamage = 2300;
				else
					flDamage = 1100.0f;
			}		
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 8450;
		}
		else
		{
			flDamage = 150;
			if (g_pModRunning->DamageTrack() == DT_ZB)
			{
				if (enhancedzombie.value)
					flDamage = 3600;
				else
					flDamage = 1900;
			}
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 13000;
		}
		return flDamage;
	}

	float CM3DragonCannon::GetExplodeDamage()
	{	
		float flDamage;
		if (m_iType)
		{
			flDamage = 60;
			if (g_pModRunning->DamageTrack() == DT_ZB)
			{
				if (enhancedzombie.value)
					flDamage = 432;
				else
					flDamage = 232;
			}
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 600;
		}
		else
		{
			flDamage = 100;
			if (g_pModRunning->DamageTrack() == DT_ZB)
			{
				if (enhancedzombie.value)
					flDamage = 700;
				else
					flDamage = 432;
			}
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 1000;
		}
		return flDamage;
	}
}
