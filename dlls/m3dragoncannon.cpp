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
		CM3DragonCannon* pCannon = m3dragoncannon(NULL);

		if (pCannon)
		{		
			pCannon->m_iType = iType;
			pCannon->pev->classname = MAKE_STRING("m3dragoncannon");
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
		pev->movetype = MOVETYPE_BOUNCE;

		if(m_iType)
			SET_MODEL(ENT(pev), "sprites/ef_m3water.spr");
		else
			SET_MODEL(ENT(pev), "models/ef_fireball2.mdl");
			
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
		
		PRECACHE_MODEL("sprites/plasmabomb_d3a3.spr");
	}

	int CM3DragonCannon::GetWeaponsId(void) { return m_iType ? WEAPON_M3DRAGONM : WEAPON_M3DRAGON; }


	void CM3DragonCannon::FlyingThink()
	{
		if (!IsInWorld())
		{
			UTIL_Remove(this);
			return;
		}
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
				pOther->TakeDamage(pev, pevOwner, flDamage, DMG_BULLET | DMG_NEVERGIB);
			}
		}

		RadiusDamage(pev, pevOwner, flExplodeDamage, CLASS_NONE, DMG_EXPLOSION | DMG_NEVERGIB);

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
		}
		else
		{
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

			MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
			WRITE_BYTE(TE_TEMPSPRITE);
			WRITE_COORD(pEntity->pev->origin.x);
			WRITE_COORD(pEntity->pev->origin.y);
			WRITE_COORD(pEntity->pev->origin.z + 20);
			WRITE_SHORT(m_iM3DragonmExp);
			WRITE_BYTE(25);
			WRITE_BYTE(255);
			WRITE_BYTE(30);
			MESSAGE_END();
		}

		EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "weapons/m3dragon_exp.wav", VOL_NORM, 0.25, 0, PITCH_NORM);



		SetTouch(NULL);
		SetThink(NULL);

		UTIL_Remove(this);
	}

	float GetDirectDamage()
	{
		float flDamage = 150;
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 3600;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 13000;
		return flDamage;
	}

	float GetExplodeDamage()
	{
		float flDamage = 100;
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 700;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 1000;
		return flDamage;
	}
}
