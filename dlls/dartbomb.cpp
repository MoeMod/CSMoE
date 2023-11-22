#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif


namespace sv {
	LINK_ENTITY_TO_CLASS(dartbomb_entity, CDartBombEntity)

	CDartBombEntity* CDartBombEntity::Create(int iType, const Vector& vecOrigin, const Vector& vecAngles, edict_t* pentOwner, int iTeam)
	{
		edict_t* pent = CREATE_NAMED_ENTITY(MAKE_STRING("dartbomb_entity"));

		if (FNullEnt(pent))
		{
			ALERT(at_console, "NULL Ent in Create!\n");
			return NULL;
		}

		CDartBombEntity* pBombEntity = (CDartBombEntity*)Instance(pent);

		if (pBombEntity)
		{		
			pBombEntity->m_iType = iType;
			pBombEntity->pev->owner = pentOwner;
			pBombEntity->pev->origin = vecOrigin;
			pBombEntity->pev->angles = vecAngles;
			pBombEntity->m_iTeam = iTeam;
			pBombEntity->Spawn();
		}

		return pBombEntity;
	}

	void CDartBombEntity::Spawn(void)
	{
		Precache();

		SetTouch(&CDartBombEntity::FlyingTouch);
		SetThink(&CDartBombEntity::FlyingThink);

		pev->nextthink = gpGlobals->time + 0.1s;

		pev->solid = SOLID_BBOX;
		pev->movetype = MOVETYPE_FLY;
		
		UTIL_SetOrigin(pev, pev->origin);
		UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));

		switch (m_iType)
		{
		case 1: 
			SET_MODEL(ENT(pev), "models/y23s1dart.mdl");
			break;
		default: 
			SET_MODEL(ENT(pev), "models/dart.mdl");
			break;
		}

		if (pev->waterlevel == 0)
		{
			MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
			WRITE_BYTE(TE_BEAMFOLLOW);
			WRITE_SHORT(entindex());
			WRITE_SHORT(m_iTrail);
			WRITE_BYTE(5);
			WRITE_BYTE(1);
			WRITE_BYTE(0);
			WRITE_BYTE(127);
			WRITE_BYTE(255);
			WRITE_BYTE(150);
			MESSAGE_END();
		}

		m_iExplodeCount = 5;
	}

	void CDartBombEntity::Precache(void)
	{
		m_iTrail = PRECACHE_MODEL("sprites/smoke.spr");

		m_iArrowModel[0] = PRECACHE_MODEL("models/dart.mdl");
		m_iArrowModel[1] = PRECACHE_MODEL("models/y23s1dart.mdl");

		m_iExpIndex = PRECACHE_MODEL("sprites/dart_explosion.spr");
	}

	int CDartBombEntity::GetWeaponsId(void) 
	{ 
		int iID = WEAPON_DARTPISTOL;
		switch (m_iType)
		{
		case 0: iID = WEAPON_DARTPISTOL; break;
		case 1: iID = WEAPON_Y23S1DARTPISTOL; break;
		}
		return iID;
	}

	KnockbackData CDartBombEntity::GetKnockBackData()
	{
		float flGround, flAir, flAirStrafing, flDucking, flVelocityModifier;

		flGround = 85.0f;
		flAir = 100.0f;
		flAirStrafing = 100.0f;
		flDucking = 80.0f;

		if (!pev->iuser1)
			flVelocityModifier = 0.95f;
		else if(pev->iuser1 == 1)
			flVelocityModifier = 0.35f;
		else
			flVelocityModifier = 0.5f;
		

		return { flGround, flAir, flAirStrafing, flDucking, flVelocityModifier };

	}

	void CDartBombEntity::FlyingThink()
	{
		if (!IsInWorld())
		{
			SetThink(NULL);
			SetTouch(NULL);

			UTIL_Remove(this);
			return;
		}
	}

	void CDartBombEntity::FlyingTouch(CBaseEntity* pOther)
	{
		float flDamage = GetDamageA();

		if (pOther->IsBSPModel())
		{
			pev->movetype = MOVETYPE_NONE;
			StuckInTheWallTouch(pOther);
			return;
		}

		if (!pOther->IsAlive())
		{
			StuckInTheWallTouch(pOther);
			return;
		}
			
		pev->effects |= EF_NODRAW;
		pev->velocity = g_vecZero;
		pev->solid = SOLID_NOT;

		if (pOther->IsPlayer() || pOther->Classify() == CLASS_PLAYER_ALLY)
		{
			Vector vecOffset = pev->origin - pOther->pev->origin;

			MESSAGE_BEGIN(MSG_ALL, SVC_TEMPENTITY);
			WRITE_BYTE(TE_FOLLOWINGARROW);
			WRITE_SHORT(pOther->entindex());
			WRITE_SHORT(m_iType ? m_iArrowModel[1] : m_iArrowModel[0]);
			WRITE_COORD(vecOffset.x);
			WRITE_COORD(vecOffset.y);
			WRITE_COORD(vecOffset.z);
			WRITE_COORD(pev->angles.x - pOther->pev->angles.x);
			WRITE_COORD(pev->angles.y - pOther->pev->angles.y);
			WRITE_COORD(pev->angles.z - pOther->pev->angles.z);
			WRITE_BYTE(4);
			MESSAGE_END();

			pev->movetype = MOVETYPE_FOLLOW;
			pev->aiment = pOther->edict();

			m_pStickingObject = pOther;

			SetTouch(NULL);
			SetThink(&CDartBombEntity::ExplodeThink);

			pev->nextthink = gpGlobals->time + 0.8s;

			MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
			WRITE_BYTE(TE_KILLBEAM);
			WRITE_SHORT(this->entindex());
			MESSAGE_END();

			TraceResult tr = UTIL_GetGlobalTrace();
			entvars_t* pevOwner = VARS(pev->owner);

			ClearMultiDamage();
		
			pOther->TraceAttack(pevOwner, flDamage, pev->velocity.Normalize(), &tr, DMG_BULLET | DMG_NEVERGIB);

			ApplyMultiDamage(pev, pevOwner);

			return;		
		}
		else
		{
			StuckInTheWallTouch(pOther);
		}
	}

	void CDartBombEntity::ExplodeThink()
	{
		pev->nextthink = gpGlobals->time + 0.8s;

		if (RANDOM_LONG(0, 1))
			UTIL_EMIT_SOUND(this->edict(), CHAN_ITEM, "weapons/dartpistol_explosion1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		else
			UTIL_EMIT_SOUND(this->edict(), CHAN_ITEM, "weapons/dartpistol_explosion2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

		MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
		WRITE_BYTE(TE_EXPLOSION);
		WRITE_COORD(pev->origin.x);
		WRITE_COORD(pev->origin.y);
		WRITE_COORD(pev->origin.z);
		WRITE_SHORT(m_iExpIndex);
		WRITE_BYTE(6); // scale * 10
		WRITE_BYTE(15); // framerate
		WRITE_BYTE(TE_EXPLFLAG_NODLIGHTS | TE_EXPLFLAG_NOPARTICLES | TE_EXPLFLAG_NOSOUND);
		MESSAGE_END();

		m_iExplodeCount--;
		TraceResult tr;

		Vector vecSrc, vecEnd;

		if (m_pStickingObject.Get() != NULL)
		{
			CBaseEntity* pEntity = m_pStickingObject;

			/*if (pEntity)
			{
				pev->origin = pEntity->pev->origin;
			}*/
		}

		entvars_t* pevOwner = VARS(pev->owner);

		CBaseEntity* pEntity = NULL;
		vecSrc = pev->origin;
		while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, 50.0)) != NULL)
		{
			if (pEntity == this)
				continue;

			if (pEntity->IsDormant())
				continue;

			if (pEntity->edict() == pev->owner)
				continue;

			if (pEntity->pev->takedamage == DAMAGE_NO)
				continue;

			vecEnd = pEntity->BodyTarget(vecSrc);

			UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, this->edict(), &tr);

			if (tr.flFraction < 1.0 && tr.pHit != ENT(pEntity->pev))
				continue;

			pev->iuser1 = 2;

			if (m_pStickingObject.Get() != NULL && pEntity == m_pStickingObject)
				pev->iuser1 = 1;
	
			pEntity->TakeDamage(pev, pevOwner, GetDamageB(), DMG_GENERIC);
		}

		if (!m_iExplodeCount)
		{
			SetThink(NULL);
			SetTouch(NULL);

			UTIL_Remove(this);
		}

	}

	void CDartBombEntity::StuckInTheWallTouch(CBaseEntity* pOther)
	{
		SetThink(NULL);
		SetTouch(NULL);

		SetThink(&CDartBombEntity::ExplodeThink);

		pev->nextthink = gpGlobals->time + 0.8s;

		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_KILLBEAM);
		WRITE_SHORT(this->entindex());
		MESSAGE_END();
	
		Vector vecDir = pev->velocity.Normalize();
		Vector vecSrc = pev->origin;
		UTIL_SetOrigin(pev, vecSrc);

		pev->angles = UTIL_VecToAngles(vecDir);

		pev->solid = SOLID_NOT;
		pev->movetype = MOVETYPE_NONE;

		pev->velocity = g_vecZero;
		pev->avelocity = g_vecZero;
	}

	float CDartBombEntity::GetDamageA()
	{
		float flDamage;

		switch (m_iType)
		{
		case 1:
		{
			flDamage = 75.0f;
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 85.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 85.0f;
			break;
		}
		default:
		{
			flDamage = 38.0f;
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 45.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 86.0f;
			break;
		}
		}
		return flDamage;
	}

	float CDartBombEntity::GetDamageB()
	{
		float flDamage;

		switch (m_iType)
		{
		case 1:
		{
			flDamage = 75.0f;
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 85.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 85.0f;
			break;
		}
		default:
		{
			flDamage = 10.0f;
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 65.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 125.0f;
			break;
		}
		}
		return flDamage;
	}

	float CDartBombEntity::GetArmorRatioModifier()
	{
		return 1.0;
	}

}
