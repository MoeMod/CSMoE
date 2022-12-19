#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif


namespace sv {
	LINK_ENTITY_TO_CLASS(throwingknife_entity, CThrowingKnifeEntity)

	CThrowingKnifeEntity* CThrowingKnifeEntity::Create(int iType, const Vector& vecOrigin, const Vector& vecAngles, edict_t* pentOwner, int iTeam)
	{
		edict_t* pent = CREATE_NAMED_ENTITY(MAKE_STRING("throwingknife_entity"));

		if (FNullEnt(pent))
		{
			ALERT(at_console, "NULL Ent in Create!\n");
			return NULL;
		}

		CThrowingKnifeEntity* pKnife = (CThrowingKnifeEntity*)Instance(pent);

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

	void CThrowingKnifeEntity::Spawn(void)
	{
		Precache();

		SetTouch(&CThrowingKnifeEntity::FlyingTouch);
		SetThink(&CThrowingKnifeEntity::FlyingThink);

		pev->nextthink = gpGlobals->time + 0.1s;

		pev->solid = SOLID_BBOX;
		pev->movetype = MOVETYPE_BOUNCE;
		pev->gravity = 0.5;
			
		UTIL_SetOrigin(pev, pev->origin);
		UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));

		SET_MODEL(ENT(pev), "models/tknife.mdl");

		pev->frame = 0;
		pev->sequence = 1;

		ResetSequenceInfo();

		switch (m_iType)
		{
		case 0: 
		case 1:
			pev->body = 0;
			break;
		case 2: 
			pev->body = 1;
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

		vecStart = pev->origin;
		m_flStartTime = gpGlobals->time;
	}

	void CThrowingKnifeEntity::Precache(void)
	{
		m_iTrail = PRECACHE_MODEL("sprites/smoke.spr");

		PRECACHE_MODEL("models/tknife.mdl");

		PRECACHE_SOUND("weapons/tknife_metal1.wav");
		PRECACHE_SOUND("weapons/tknife_metal2.wav");
		//PRECACHE_SOUND("weapons/tknife_metal3.wav");
		PRECACHE_SOUND("weapons/tknife_stone1.wav");
		PRECACHE_SOUND("weapons/tknife_stone2.wav");
		PRECACHE_SOUND("weapons/tknife_wood1.wav");
		PRECACHE_SOUND("weapons/tknife_wood2.wav");
	}

	int CThrowingKnifeEntity::GetWeaponsId(void) 
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

	KnockbackData CThrowingKnifeEntity::GetKnockBackData()
	{
		float flGround, flAir, flAirStrafing, flDucking, flVelocityModifier;

		flGround = 250.0f;
		flAir = 200.0f;
		flAirStrafing = 250.0f;
		flDucking = 90.0f;
		flVelocityModifier = 0.7f;

		return { flGround, flAir, flAirStrafing, flDucking, flVelocityModifier };

	}

	void CThrowingKnifeEntity::FlyingThink()
	{
		if (!IsInWorld())
		{
			SetThink(NULL);
			SetTouch(NULL);

			UTIL_Remove(this);
			return;
		}
	}

	void CThrowingKnifeEntity::FlyingTouch(CBaseEntity* pOther)
	{
		float flDamage = GetDamage();

		if (pOther->pev->takedamage == DAMAGE_NO)
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

	void CThrowingKnifeEntity::StuckInTheWallTouch(CBaseEntity* pOther)
	{
		SetThink(NULL);
		SetTouch(NULL);

		SetThink(&CBaseEntity::SUB_StartFadeOut);

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

	float CThrowingKnifeEntity::GetDamage()
	{
		float flDamage;

		switch (m_iType)
		{
		case 0:
		{
			flDamage = 80.0f;
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 90.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 90.0f;
			break;
		}
		case 1:
		{
			flDamage = 75.0f;
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 85.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 85.0f;
			break;
		}
		case 2:
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

	float CThrowingKnifeEntity::GetArmorRatioModifier()
	{
		float flArmorRatioModifier = 1.0;
		return flArmorRatioModifier;
	}

}
