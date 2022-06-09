#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif

namespace sv {
	LINK_ENTITY_TO_CLASS(m3dragoncannon_dragon, CM3DragonCannon_Dragon)

	CM3DragonCannon_Dragon* CM3DragonCannon_Dragon::Create(int iType, const Vector& vecOrigin, const Vector& vecAngles, edict_t* pentOwner, int iTeam)
	{
		CM3DragonCannon_Dragon* pDragon = m3dragoncannon_dragon(NULL);

		if (pDragon)
		{
			pDragon->m_iType = iType;
			pDragon->pev->classname = MAKE_STRING("m3dragoncannon_dragon");
			pDragon->pev->owner = pentOwner;
			pDragon->pev->origin = vecOrigin;
			pDragon->pev->angles = vecAngles;
			pDragon->m_iTeam = iTeam;
			pDragon->Spawn();
		}

		return pDragon;
	}

	void CM3DragonCannon_Dragon::Spawn(void)
	{
		Precache();
	
		pev->nextthink = gpGlobals->time + 0.1s;
		pev->solid = SOLID_BBOX;
		pev->movetype = MOVETYPE_NONE;

		if (m_iType)
		{
			SET_MODEL(ENT(pev), "sprites/ef_m3water.spr");
			SetThink(&CM3DragonCannon_Dragon::OnSharkThink);		
		}
			
		else
		{
			SET_MODEL(ENT(pev), "models/ef_fireball2.mdl");
			SetThink(&CM3DragonCannon_Dragon::OnThink);
		}
			

		UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));
	}

	void CM3DragonCannon_Dragon::Precache(void)
	{
		//WEAPON_M3DRAGONM
		PRECACHE_MODEL("models/ef_m3dragonm_sign.mdl");	//flying
		PRECACHE_MODEL("models/m3dragonm_effect.mdl");	//shark
		PRECACHE_MODEL("sprites/ef_m3dragonm_water.spr");	//splash
		PRECACHE_SOUND("weapons/m3dragonm_shark_fx.wav");

		//WEAPON_M3DRAGON
		PRECACHE_MODEL("models/m3dragon_effect.mdl");
		PRECACHE_SOUND("weapons/m3dragon_dragon_fx.wav");
	}

	int CM3DragonCannon_Dragon::GetWeaponsId(void) { return m_iType ? WEAPON_M3DRAGONM : WEAPON_M3DRAGON; }
}
