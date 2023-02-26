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

		CDartBombEntity* pKnife = (CDartBombEntity*)Instance(pent);

		if (pKnife)
		{		
			pKnife->m_iType = iType;
			pKnife->pev->owner = pentOwner;
			pKnife->pev->origin = vecOrigin;
			pKnife->pev->angles = vecAngles;
			pKnife->m_iTeam = iTeam;
			pKnife->Spawn();
		}

		return pKnife;
	}

	void CDartBombEntity::Spawn(void)
	{
		Precache();

		SetTouch(&CDartBombEntity::FlyingTouch);
		SetThink(&CDartBombEntity::FlyingThink);

		pev->nextthink = gpGlobals->time + 0.1s;

		pev->solid = SOLID_BBOX;
		pev->movetype = MOVETYPE_BOUNCE;
		pev->gravity = 0.5;
			
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
			WRITE_BYTE(3);
			WRITE_BYTE(2);
			WRITE_BYTE(244);
			WRITE_BYTE(244);
			WRITE_BYTE(244);
			WRITE_BYTE(200);
			MESSAGE_END();
		}
	}

	void CDartBombEntity::Precache(void)
	{
		m_iTrail = PRECACHE_MODEL("sprites/smoke.spr");

		PRECACHE_MODEL("models/dart.mdl");

		PRECACHE_SOUND("weapons/dartpistol_explosion1.wav");
		PRECACHE_SOUND("weapons/dartpistol_explosion2.wav");
	}

	int CDartBombEntity::GetWeaponsId(void) 
	{ 
		int iID = WEAPON_TKNIFE;
		switch (m_iType)
		{
		case 0: iID = WEAPON_TKNIFE; break;
		case 1: iID = WEAPON_TKNIFEEX; break;
		case 2: iID = WEAPON_TKNIFEEX2; break;
		}
		return iID;
	}

	KnockbackData CDartBombEntity::GetKnockBackData()
	{
		float flGround, flAir, flAirStrafing, flDucking, flVelocityModifier;

		flGround = 250.0f;
		flAir = 200.0f;
		flAirStrafing = 250.0f;
		flDucking = 90.0f;
		flVelocityModifier = 0.7f;

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
		float flDamage = GetDamage();

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

		if (!pOther->IsPlayer() && pOther->Classify() != CLASS_PLAYER_ALLY)
		{
			TraceResult tr = UTIL_GetGlobalTrace();

			Vector vecStart = pev->origin;
			Vector vecEnd = vecStart + pev->velocity.Normalize() * 12;

			const char* rgsz[2];

			switch (UTIL_TextureHit(&tr, vecStart, vecEnd))
			{
			case CHAR_TEX_WOOD:
				rgsz[0] = "weapons/tknife_wood1.wav";
				rgsz[1] = "weapons/tknife_wood2.wav";

				break;

			case CHAR_TEX_GRATE:
			case CHAR_TEX_METAL:
			case CHAR_TEX_COMPUTER:
				rgsz[0] = "weapons/tknife_metal1.wav";
				rgsz[1] = "weapons/tknife_metal2.wav";

				break;

			default:
				rgsz[0] = "weapons/tknife_stone1.wav";
				rgsz[1] = "weapons/tknife_stone2.wav";

				break;
			}

			UTIL_EmitAmbientSound(ENT(0), tr.vecEndPos, rgsz[RANDOM_LONG(0, 1)], 0.9, ATTN_NORM, 0, 96 + RANDOM_LONG(0, 15));

			if (UTIL_PointContents(pev->origin) != CONTENTS_WATER)
			{
				UTIL_GunshotDecalTrace(&tr, DamageDecal(DMG_BULLET | DMG_NEVERGIB), false, NULL);

				if (m_flStartTime + 0.3s > gpGlobals->time)
				{
					pev->velocity *= 0.4;
					return;
				}
				else
				{			
					StuckInTheWallTouch(pOther);
					return;
				}
			}
			else
			{
				StuckInTheWallTouch(pOther);
				return;
			}
		}
		else
		{

			m_pStickingObject = pOther;

			SetThink(NULL);
			SetTouch(NULL);

			TraceResult tr = UTIL_GetGlobalTrace();
			entvars_t* pevOwner = VARS(pev->owner);

			ClearMultiDamage();
			flDamage -= (gpGlobals->time - m_flStartTime) / 1s * 10;
			//flDamage *= (1.0 - (pOther->pev->origin - vecStart).Length() / 4096.0);

			pOther->TraceAttack(pevOwner, flDamage, pev->velocity.Normalize(), &tr, DMG_BULLET | DMG_NEVERGIB);

			ApplyMultiDamage(pev, pevOwner);

			pev->velocity = g_vecZero;

			EMIT_SOUND_DYN(ENT(pev), CHAN_BODY, "weapons/knife_hit2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			UTIL_Remove(this);
		}
	}

	void CDartBombEntity::StuckInTheWallThink()
	{



		UTIL_Remove(this);
	}

	void CDartBombEntity::ExplodeThink()
	{
		TraceResult tr;

		Vector vecSrc;

		if (m_pStickingObject.Get() != NULL)
		{
			CBaseEntity* pEntity = m_pStickingObject;

			if (pEntity)
			{
				pev->origin = pEntity->pev->origin;
			}
		}

		CBaseEntity* pEntity = NULL;
		vecSrc = pev->origin;
		while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, 50.0)) != NULL)
		{
			if (pEntity == this)
				continue;

			if (pEntity->IsDormant())
				continue;

			if (pEntity->pev->takedamage == DAMAGE_NO)
				continue;

			vecEnd = pEntity->BodyTarget(vecSrc);

			if (DotProduct((vecEnd.Make2D() - vecSrc.Make2D()).Normalize(), vecAngleCheckDir) < angCheck)
				continue;

			UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr);

			if (tr.flFraction < 1.0 && tr.pHit != ENT(pEntity->pev))
				continue;

			ClearMultiDamage();

			pEntity->TraceAttack(this->pev, flDamage, (tr.vecEndPos - vecSrc).Normalize(), &tr, DMG_BULLET | DMG_NEVERGIB);

			ApplyMultiDamage(this->pev, this->pev);
		}

	}

	void CDartBombEntity::StuckInTheWallTouch(CBaseEntity* pOther)
	{
		SetThink(NULL);
		SetTouch(NULL);

		SetThink(&CBaseEntity::StuckInTheWallThink);

		pev->frame = 0;
		pev->sequence = 0;

		pev->nextthink = gpGlobals->time + 3.0s;

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

	float CDartBombEntity::GetDamage()
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
			flDamage = 70.0f;
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 60.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 60.0f;
			break;
		}
		}
		return flDamage;
	}

	float CDartBombEntity::GetArmorRatioModifier()
	{
		float flArmorRatioModifier = 1.0;
		return flArmorRatioModifier;
	}

}
