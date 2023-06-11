#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif


namespace sv {
	LINK_ENTITY_TO_CLASS(at4_rocket, CAT4Rocket)

	CAT4Rocket* CAT4Rocket::Create(int iType, const Vector& vecOrigin, const Vector& vecAngles, edict_t* pentOwner, int iTeam)
	{
		edict_t* pent = CREATE_NAMED_ENTITY(MAKE_STRING("at4_rocket"));

		if (FNullEnt(pent))
		{
			ALERT(at_console, "NULL Ent in Create!\n");
			return NULL;
		}

		CAT4Rocket* pRocket = (CAT4Rocket*)Instance(pent);

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

	void CAT4Rocket::Spawn(void)
	{
		Precache();

		pev->movetype = MOVETYPE_FLY;
		pev->solid = SOLID_BBOX;

		SET_MODEL(ENT(pev), "models/shell_svdex.mdl");
		UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));
		UTIL_SetOrigin(pev, pev->origin);

		SetTouch(&CAT4Rocket::RocketTouch);
		SetThink(&CAT4Rocket::IgniteThink);

		pev->velocity = gpGlobals->v_forward * 300;
		pev->nextthink = gpGlobals->time + 0.1s;
		pev->dmg = GetDamage();

	}

	void CAT4Rocket::Precache(void)
	{
		m_iTrail = PRECACHE_MODEL("sprites/black_smoke3.spr");

		PRECACHE_MODEL("models/shell_svdex.mdl");
	}

	int CAT4Rocket::GetWeaponsId(void) 
	{ 
		int iID = WEAPON_AT4;
		switch (m_iType)
		{
		case 0: iID = WEAPON_AT4; break;
		case 1: 
		case 2: iID = WEAPON_AT4EX; break;
		case 3: iID = WEAPON_BAZOOKA; break;
		}
		return iID;
	}

	void CAT4Rocket::IgniteThink()
	{
		pev->movetype = MOVETYPE_FLY;

		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_BEAMPARTICLE); //defined as 32
		WRITE_SHORT(this->entindex());  // short (entity:attachment to follow)
		WRITE_SHORT(m_iTrail);// short (sprite index)
		WRITE_SHORT(m_iTrail);// short (sprite index)
		WRITE_SHORT(m_iTrail);// short (sprite index)
		WRITE_BYTE(255); //r
		WRITE_BYTE(255); //g
		WRITE_BYTE(255); //b
		WRITE_BYTE(0); //randomcolor
		WRITE_BYTE(100); //scale in 0.01's
		WRITE_BYTE(6); //freq(time of one cycle) in 0.01's
		WRITE_BYTE(3); //fadetime in 0.1s
		WRITE_BYTE(1);  //count
		WRITE_BYTE(0);  //random in org
		WRITE_BYTE(0);  //speed (if>0 do slow gravity)
		MESSAGE_END();

		m_flIgniteTime = gpGlobals->time;

		SetThink(&CAT4Rocket::FollowThink);
		pev->nextthink = gpGlobals->time + 0.1s;
	}

	void CAT4Rocket::FollowThink()
	{
		UTIL_MakeAimVectors(pev->angles);

		pev->angles = UTIL_VecToAngles(gpGlobals->v_forward);
		pev->nextthink = gpGlobals->time + 0.1s;
	}

	void CAT4Rocket::RocketTouch(CBaseEntity* pOther)
	{
		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_KILLBEAM);
		WRITE_SHORT(this->entindex());
		MESSAGE_END();

		ExplodeTouch3(pOther, GetRadius(), EXPTYPE_AT4ROCKET, false);
	}

	float CAT4Rocket::GetRadius()
	{
		float flRadius = 1.0f;

		switch (m_iType)
		{
		case 0:
		{
			//at4
			flRadius = 3.0f;
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flRadius = 4.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flRadius = 4.0f;
			break;
		}
		case 1:
		{
			//at4ex
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

	float CAT4Rocket::GetDamage()
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
