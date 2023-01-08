#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif


namespace sv {
	LINK_ENTITY_TO_CLASS(m79_rocket, CM79Rocket)

	CM79Rocket* CM79Rocket::Create(int iType, const Vector& vecOrigin, const Vector& vecAngles, edict_t* pentOwner, int iTeam)
	{
		edict_t* pent = CREATE_NAMED_ENTITY(MAKE_STRING("m79_rocket"));

		if (FNullEnt(pent))
		{
			ALERT(at_console, "NULL Ent in Create!\n");
			return NULL;
		}

		CM79Rocket* pRocket = (CM79Rocket*)Instance(pent);

		if (pRocket)
		{		
			pRocket->m_iType = iType;
			pRocket->pev->owner = pentOwner;
			pRocket->pev->origin = vecOrigin;
			pRocket->pev->angles = vecAngles;
			pRocket->m_iTeam = iTeam;
			pRocket->Spawn();
		}

		return pRocket;
	}

	void CM79Rocket::Spawn(void)
	{
		Precache();

		pev->movetype = MOVETYPE_BOUNCE;
		pev->solid = SOLID_BBOX;

		SET_MODEL(ENT(pev), "models/shell_svdex.mdl");
		UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));
		UTIL_SetOrigin(pev, pev->origin);

		SetTouch(&CM79Rocket::RocketTouch);
		SetThink(&CM79Rocket::IgniteThink);

		pev->angles.x -= 30.0;

		UTIL_MakeVectors(pev->angles);
		pev->angles.x = -(pev->angles.x + 30.0);

		pev->velocity = gpGlobals->v_forward * 300;
		pev->gravity = 0.5;
		pev->nextthink = gpGlobals->time + 0.1s;
		pev->dmg = GetDamage();

	}

	void CM79Rocket::Precache(void)
	{
		m_iTrail = PRECACHE_MODEL("sprites/smoke.spr");

		PRECACHE_MODEL("models/shell_svdex.mdl");
	}

	int CM79Rocket::GetWeaponsId(void) 
	{ 
		int iID = WEAPON_M79;
		switch (m_iType)
		{
		case 0: iID = WEAPON_M79; break;
		case 1: iID = WEAPON_M79G; break;
		case 2: 
		case 3: iID = WEAPON_JANUS11; break;
		case 4: iID = WEAPON_FIRECRACKER; break;
		}
		return iID;
	}

	void CM79Rocket::IgniteThink()
	{
		pev->movetype = MOVETYPE_BOUNCE;

		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_BEAMFOLLOW);
		WRITE_SHORT(entindex());
		WRITE_SHORT(m_iTrail);
		WRITE_BYTE(40);
		WRITE_BYTE(5);
		WRITE_BYTE(224);
		WRITE_BYTE(224);
		WRITE_BYTE(255);
		WRITE_BYTE(255);
		MESSAGE_END();

		m_flIgniteTime = gpGlobals->time;

		SetThink(&CM79Rocket::FollowThink);
		pev->nextthink = gpGlobals->time + 0.1s;
	}

	void CM79Rocket::FollowThink()
	{
		UTIL_MakeAimVectors(pev->angles);

		pev->angles = UTIL_VecToAngles(gpGlobals->v_forward);
		pev->nextthink = gpGlobals->time + 0.1s;
	}

	void CM79Rocket::RocketTouch(CBaseEntity* pOther)
	{
		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_KILLBEAM);
		WRITE_SHORT(this->entindex());
		MESSAGE_END();

		ExplodeTouch3(pOther, GetRadius(), EXPTYPE_M79ROCKET, false);
	}

	float CM79Rocket::GetRadius()
	{
		float flRadius = 1.0f;

		switch (m_iType)
		{
		case 0:
		{
			//m79
			flRadius = 3.0f;
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flRadius = 4.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flRadius = 4.0f;
			break;
		}
		case 1:
		{
			//m79g
			flRadius = 3.0f;
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flRadius = 4.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flRadius = 4.0f;
			break;
		}
		case 2:
		{
			//janus1 mode a
			flRadius = 3.0f;
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flRadius = 4.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flRadius = 4.0f;
			break;
		}
		case 3:
		{
			//janus1 mode b
			flRadius = 3.0f;
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flRadius = 4.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flRadius = 4.0f;
			break;
		}
		case 4:
		{
			//firecracker
			flRadius = 3.0f;
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flRadius = 4.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flRadius = 4.0f;
			break;
		}
		}

		return flRadius * 39.37f;
	}

	float CM79Rocket::GetDamage()
	{
		float flDamage;

		switch (m_iType)
		{
		case 0:
		{
			//m79
			flDamage = 80.0f;
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 400.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 160.0f;
			break;
		}
		case 1:
		{
			//m79g
			flDamage = 80.0f;
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 410.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 190.0f;
			break;
		}
		case 2:
		{
			//janus1 mode a
			flDamage = 60.0f;
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 200.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 120.0f;
			break;
		}
		case 3:
		{
			//janus1 mode b
			flDamage = 80.0f;
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 400.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 160.0f;
			break;
		}
		case 4:
		{
			//firecracker
			flDamage = 80.0f;
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 500.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 700.0f;
			break;
		}
		}
		return flDamage;
	}

}
