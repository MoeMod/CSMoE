
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "soundent.h"
#include "decals.h"
#include "player.h"
#include "gamerules.h"
#include "hltv.h"
#include "game.h"
#include "globals.h"
#include "cbase/cbase_hash.h"

#include "bot_include.h"

#ifndef CLIENT_DLL
#include "player/player_mod_strategy.h"
#endif

namespace sv {


/*
* Globals initialization
*/
TYPEDESCRIPTION CGrenade::m_SaveData[] =
{
	DEFINE_FIELD(CGrenade, m_fAttenu, FIELD_FLOAT),
	DEFINE_FIELD(CGrenade, m_flNextFreq, FIELD_TIME),
	DEFINE_FIELD(CGrenade, m_flC4Blow, FIELD_TIME),
	DEFINE_FIELD(CGrenade, m_flNextFreqInterval, FIELD_TIME),
	DEFINE_FIELD(CGrenade, m_flNextBeep, FIELD_TIME),
	DEFINE_FIELD(CGrenade, m_flDefuseCountDown, FIELD_TIME),
	DEFINE_FIELD(CGrenade, m_flNextBlink, FIELD_TIME),
	DEFINE_FIELD(CGrenade, m_pentCurBombTarget, FIELD_EDICT),
	DEFINE_FIELD(CGrenade, m_sBeepName, FIELD_POINTER),
	DEFINE_FIELD(CGrenade, m_bIsC4, FIELD_BOOLEAN),
	DEFINE_FIELD(CGrenade, m_bStartDefuse, FIELD_BOOLEAN),
	DEFINE_FIELD(CGrenade, m_SGSmoke, FIELD_INTEGER),
	DEFINE_FIELD(CGrenade, m_bJustBlew, FIELD_BOOLEAN),
	DEFINE_FIELD(CGrenade, m_bLightSmoke, FIELD_BOOLEAN),
	DEFINE_FIELD(CGrenade, m_usEvent, FIELD_INTEGER),
};

LINK_ENTITY_TO_CLASS(grenade, CGrenade);

void CGrenade::Explode(Vector vecSrc, Vector vecAim)
{
	TraceResult tr;
	UTIL_TraceLine(pev->origin, pev->origin + Vector(0, 0, -32), ignore_monsters, ENT(pev), &tr);
	Explode(&tr, DMG_BLAST);
}

// UNDONE: temporary scorching for PreAlpha - find a less sleazy permenant solution.

void CGrenade::Explode(TraceResult *pTrace, int bitsDamageType)
{
	if (to_int(g_pModRunning->QueryModConfig("EnableKnockBackBomb")))
	{
#define NADE_FLASHBANG_RADIUS 420
		EMIT_SOUND(ENT(pev), CHAN_WEAPON, "metalarena/acc_up.wav", VOL_NORM, ATTN_NORM);	//preache

		Vector vecOrigin = pev->origin;

		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY, vecOrigin);
		WRITE_BYTE(TE_TEMPSPRITE);
		WRITE_COORD(vecOrigin.x);
		WRITE_COORD(vecOrigin.y);
		WRITE_COORD(vecOrigin.z + 50.0);
		WRITE_SHORT(g_sModelIndexWind);
		WRITE_BYTE(15);
		WRITE_BYTE(255);
		WRITE_BYTE(100);
		MESSAGE_END();

		MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, vecOrigin);
		WRITE_BYTE(TE_BEAMCYLINDER);
		WRITE_COORD(vecOrigin[0]);
		WRITE_COORD(vecOrigin[1]);
		WRITE_COORD(vecOrigin[2]);
		WRITE_COORD(vecOrigin[0]);
		WRITE_COORD(vecOrigin[1]);
		WRITE_COORD(vecOrigin[2] + NADE_FLASHBANG_RADIUS);
		WRITE_SHORT(g_sModelIndexShockWave);
		WRITE_BYTE(0); // Start Frame
		WRITE_BYTE(20); // Framerate
		WRITE_BYTE(4); // Live Time
		WRITE_BYTE(10); // Width
		WRITE_BYTE(10); // Noise
		WRITE_BYTE(0); // R
		WRITE_BYTE(255); // G
		WRITE_BYTE(255); // B
		WRITE_BYTE(255); // Bright
		WRITE_BYTE(9); // Speed
		MESSAGE_END();

		MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, vecOrigin);
		WRITE_BYTE(TE_BEAMCYLINDER);
		WRITE_COORD(vecOrigin[0]);
		WRITE_COORD(vecOrigin[1]);
		WRITE_COORD(vecOrigin[2]);
		WRITE_COORD(vecOrigin[0]);
		WRITE_COORD(vecOrigin[1]);
		WRITE_COORD(vecOrigin[2] + NADE_FLASHBANG_RADIUS);
		WRITE_SHORT(g_sModelIndexShockWave);
		WRITE_BYTE(0); // Start Frame
		WRITE_BYTE(10); // Framerate
		WRITE_BYTE(4); // Live Time
		WRITE_BYTE(10); // Width
		WRITE_BYTE(20); // Noise
		WRITE_BYTE(0); // R
		WRITE_BYTE(255); // G
		WRITE_BYTE(0); // B
		WRITE_BYTE(150); // Bright
		WRITE_BYTE(9); // Speed
		MESSAGE_END();

		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY, vecOrigin);
		WRITE_BYTE(TE_TEMPSPRITE);
		WRITE_COORD(vecOrigin.x);
		WRITE_COORD(vecOrigin.y);
		WRITE_COORD(vecOrigin.z + 50.0);
		WRITE_SHORT(g_sModelIndexWindExp);
		WRITE_BYTE(15);
		WRITE_BYTE(255);
		WRITE_BYTE(100);
		MESSAGE_END();

		CBaseEntity* pEntity = NULL;
		while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecOrigin, NADE_FLASHBANG_RADIUS)) != NULL)
		{
			if (!pEntity->IsPlayer())
				continue;

			if (!pEntity->IsAlive())
				continue;

			CBasePlayer* pPlayer = (CBasePlayer*)pEntity;
			if (pPlayer->m_bIsZombie)
				continue;

			Vector vecDir = (pPlayer->pev->origin - vecOrigin);

			ApplyKnockbackData(pPlayer, vecDir, { 2000, 2000, 2000, 2000, 1.0f });
		}
		pev->effects |= EF_NODRAW;

		if (TheBots != NULL)
		{
			TheBots->RemoveGrenade(this);
		}
		UTIL_Remove(this);
		return;
	}
	float flRndSound; // sound randomizer

	pev->model = iStringNull; // invisible
	pev->solid = SOLID_NOT; // intangible
	pev->takedamage = DAMAGE_NO;

	// Pull out of the wall a bit
	if (pTrace->flFraction != 1.0f)
	{
		pev->origin = pTrace->vecEndPos + (pTrace->vecPlaneNormal * (pev->dmg - 24.0f) * 0.6f);
	}

	int iContents = UTIL_PointContents(pev->origin);
	CSoundEnt::InsertSound(bits_SOUND_COMBAT, pev->origin, NORMAL_EXPLOSION_VOLUME, 3s);

	entvars_t *pevOwner;
	if (pev->owner)
		pevOwner = VARS(pev->owner);
	else
		pevOwner = NULL;

	if (TheBots != NULL)
	{
		TheBots->OnEvent(EVENT_FLASHBANG_GRENADE_EXPLODED, CBaseEntity::Instance(pev->owner), (CBaseEntity *)&pev->origin);
	}

	// can't traceline attack owner if this is set
	pev->owner = NULL;

	RadiusFlash(pev->origin, pev, pevOwner, 4);

	if (RANDOM_FLOAT(0, 1) < 0.5f)
		UTIL_DecalTrace(pTrace, DECAL_SCORCH1);
	else
		UTIL_DecalTrace(pTrace, DECAL_SCORCH2);

	// TODO: unused
	flRndSound = RANDOM_FLOAT(0, 1);

	switch (RANDOM_LONG(0, 1))
	{
	case 0: EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/flashbang-2.wav", 0.55, ATTN_NORM); break;
	case 1: EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/flashbang-1.wav", 0.55, ATTN_NORM); break;
	}

	pev->effects |= EF_NODRAW;
	SetThink(&CGrenade::Smoke);
	pev->velocity = g_vecZero;
	pev->nextthink = gpGlobals->time + 0.3s;

	if (iContents != CONTENTS_WATER)
	{
		int sparkCount = RANDOM_LONG(0, 3);

		for (int i = 0; i < sparkCount; ++i)
			Create("spark_shower", pev->origin, pTrace->vecPlaneNormal, NULL);
	}
}

void CGrenade::Explode2(TraceResult *pTrace, int bitsDamageType)
{
	float flRndSound; // sound randomizer
	CHalfLifeMultiplay *mp = g_pGameRules;

	pev->model = iStringNull; // invisible
	pev->solid = SOLID_NOT; // intangible
	pev->takedamage = DAMAGE_NO;

	UTIL_ScreenShake(pTrace->vecEndPos, 25, 150, 1, 3000);

	mp->m_bTargetBombed = true;

	if (mp->IsCareer())
	{
		if (TheCareerTasks != NULL)
		{
			TheCareerTasks->LatchRoundEndMessage();
		}
	}

	m_bJustBlew = true;
	mp->CheckWinConditions();

	// Pull out of the wall a bit
	if (pTrace->flFraction != 1.0f)
	{
		pev->origin = pTrace->vecEndPos + (pTrace->vecPlaneNormal * (pev->dmg - 24.0f) * 0.6f);
	}

	int iContents = UTIL_PointContents(pev->origin);

	MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
		WRITE_BYTE(TE_SPRITE);
		WRITE_COORD(pev->origin.x);
		WRITE_COORD(pev->origin.y);
		WRITE_COORD(pev->origin.z - 10.0f);
		WRITE_SHORT(g_sModelIndexFireball3);
		WRITE_BYTE((pev->dmg - 275.0f) * 0.6f);
		WRITE_BYTE(150);
	MESSAGE_END();

	MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
		WRITE_BYTE(TE_SPRITE);
		WRITE_COORD(pev->origin.x + RANDOM_FLOAT(-512, 512));
		WRITE_COORD(pev->origin.y + RANDOM_FLOAT(-512, 512));
		WRITE_COORD(pev->origin.z + RANDOM_FLOAT(-10, 10));
		WRITE_SHORT(g_sModelIndexFireball2);
		WRITE_BYTE((pev->dmg - 275.0f) * 0.6f);
		WRITE_BYTE(150);
	MESSAGE_END();

	MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
		WRITE_BYTE(TE_SPRITE);
		WRITE_COORD(pev->origin.x + RANDOM_FLOAT(-512, 512));
		WRITE_COORD(pev->origin.y + RANDOM_FLOAT(-512, 512));
		WRITE_COORD(pev->origin.z + RANDOM_FLOAT(-10, 10));
		WRITE_SHORT(g_sModelIndexFireball3);
		WRITE_BYTE((pev->dmg - 275.0f) * 0.6f);
		WRITE_BYTE(150);
	MESSAGE_END();

	MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
		WRITE_BYTE(TE_SPRITE);
		WRITE_COORD(pev->origin.x + RANDOM_FLOAT(-512, 512));
		WRITE_COORD(pev->origin.y + RANDOM_FLOAT(-512, 512));
		WRITE_COORD(pev->origin.z + RANDOM_FLOAT(-10, 10));
		WRITE_SHORT(g_sModelIndexFireball);
		WRITE_BYTE((pev->dmg - 275.0f) * 0.6f);
		WRITE_BYTE(17);
	MESSAGE_END();

	// Sound! for everyone
	EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/c4_explode1.wav", VOL_NORM, 0.25);
	CSoundEnt::InsertSound(bits_SOUND_COMBAT, pev->origin, NORMAL_EXPLOSION_VOLUME, 3s);

	entvars_t *pevOwner;
	if (pev->owner)
		pevOwner = VARS(pev->owner);
	else
		pevOwner = NULL;

	pev->owner = NULL;
	RadiusDamage(pev, pevOwner, mp->m_flBombRadius, CLASS_NONE, bitsDamageType);

	if (mp->IsCareer())
	{
		if (TheCareerTasks != NULL)
		{
			TheCareerTasks->UnlatchRoundEndMessage();
		}
	}

	// tell director about it
	// send director message, that something important happed here
	MESSAGE_BEGIN(MSG_SPEC, SVC_DIRECTOR);
		WRITE_BYTE(9);		// command length in bytes
		WRITE_BYTE(DRC_CMD_EVENT);	// bomb explode
		WRITE_SHORT(ENTINDEX(edict()));	// index number of primary entity
		WRITE_SHORT(0);		// index number of secondary entity
		WRITE_LONG(15 | DRC_FLAG_FINAL);	// eventflags (priority and flags)
	MESSAGE_END();

	// Decal!
	if (RANDOM_FLOAT(0, 1) < 0.5f)
		UTIL_DecalTrace(pTrace, DECAL_SCORCH1);
	else
		UTIL_DecalTrace(pTrace, DECAL_SCORCH2);

	// TODO: unused
	flRndSound = RANDOM_FLOAT(0, 1);

	switch (RANDOM_LONG(0, 2))
	{
	case 0: EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/debris1.wav", 0.55, ATTN_NORM); break;
	case 1: EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/debris2.wav", 0.55, ATTN_NORM); break;
	case 2: EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/debris3.wav", 0.55, ATTN_NORM); break;
	}

	pev->effects |= EF_NODRAW;
	SetThink(&CGrenade::Smoke2);
	pev->velocity = g_vecZero;
	pev->nextthink = gpGlobals->time + 0.85s;

	if (iContents != CONTENTS_WATER)
	{
		int sparkCount = RANDOM_LONG(0, 3);

		for (int i = 0; i < sparkCount; ++i)
			Create("spark_shower", pev->origin, pTrace->vecPlaneNormal, NULL);
	}
}

void CGrenade::Explode3(TraceResult *pTrace, int bitsDamageType, CGrenade::E_EXPLODE_TYPE type, int iRadius)
{
	float flRndSound; // sound randomizer
	//float damage;

	pev->model = iStringNull; // invisible
	pev->solid = SOLID_NOT; // intangible
	pev->takedamage = DAMAGE_NO;

	if (pTrace->flFraction != 1.0f)
	{
		pev->origin = pTrace->vecEndPos + (pTrace->vecPlaneNormal * (pev->dmg - 24.0f) * 0.6f);
	}

	switch (type)
	{
	case EXPTYPE_EVENTBOMB:
	{
		if (m_iGrenadeID == GRENADE_HEARTBOMB)
		{
			MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
			WRITE_BYTE(TE_EXPLOSION);
			WRITE_COORD(pev->origin.x);
			WRITE_COORD(pev->origin.y);
			WRITE_COORD(pev->origin.z + 20.0);
			WRITE_SHORT(g_sModelIndexEventBombExp);
			WRITE_BYTE(25);
			WRITE_BYTE(30);
			WRITE_BYTE(TE_EXPLFLAG_NOSOUND);
			MESSAGE_END();

			EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "weapons/heartbomb_explosion.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		}
		else
		{
			MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
			WRITE_BYTE(TE_EXPLOSION);
			WRITE_COORD(pev->origin.x);
			WRITE_COORD(pev->origin.y);
			WRITE_COORD(pev->origin.z + 20.0);
			WRITE_SHORT(g_sModelIndexEventBombExp2);
			WRITE_BYTE(25);
			WRITE_BYTE(30);
			WRITE_BYTE(TE_EXPLFLAG_NOSOUND);
			MESSAGE_END();

			EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "weapons/cakeexp.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		}

		break;
	}

	case EXPTYPE_MOONCAKE:
		MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
		WRITE_BYTE(TE_EXPLOSION);
		WRITE_COORD(pev->origin.x + RANDOM_FLOAT(-64.0, 64.0));
		WRITE_COORD(pev->origin.y + RANDOM_FLOAT(-64.0, 64.0));
		WRITE_COORD(pev->origin.z + RANDOM_FLOAT(30.0, 35.0));
		WRITE_SHORT(g_sModelIndexMoonCakeExp);
		WRITE_BYTE(30);
		WRITE_BYTE(30);
		WRITE_BYTE(TE_EXPLFLAG_NOSOUND);
		MESSAGE_END();

		EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "weapons/mooncake_exp.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

		break;

	case EXPTYPE_CARTFRAG:
		MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
		WRITE_BYTE(TE_EXPLOSION);
		WRITE_COORD(pev->origin.x);
		WRITE_COORD(pev->origin.y);
		WRITE_COORD(pev->origin.z + 20.0);
		WRITE_SHORT(g_sModelIndexFireball3);
		WRITE_BYTE(25);
		WRITE_BYTE(30);
		WRITE_BYTE(TE_EXPLFLAG_NOSOUND);
		MESSAGE_END();

		MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
		WRITE_BYTE(TE_EXPLOSION);
		WRITE_COORD(pev->origin.x + RANDOM_FLOAT(-64.0, 64.0));
		WRITE_COORD(pev->origin.y + RANDOM_FLOAT(-64.0, 64.0));
		WRITE_COORD(pev->origin.z + RANDOM_FLOAT(30.0, 35.0));
		WRITE_SHORT(g_sModelIndexFireball2);
		WRITE_BYTE(30);
		WRITE_BYTE(30);
		WRITE_BYTE(TE_EXPLFLAG_NOSOUND);
		MESSAGE_END();

		EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "weapons/cartfrag.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		break;

	case EXPTYPE_THANATOS5ROCKET:
		MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
		WRITE_BYTE(TE_EXPLOSION);
		WRITE_COORD(pev->origin.x);
		WRITE_COORD(pev->origin.y);
		WRITE_COORD(pev->origin.z + 20.0);
		WRITE_SHORT(g_sModelIndexHolyWater);
		WRITE_BYTE(25);
		WRITE_BYTE(30);
		WRITE_BYTE(TE_EXPLFLAG_NONE);
		MESSAGE_END();

		break;

	case EXPTYPE_VULCANUS7ROCKET:
		MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
		WRITE_BYTE(TE_EXPLOSION);
		WRITE_COORD(pev->origin.x);
		WRITE_COORD(pev->origin.y);
		WRITE_COORD(pev->origin.z + 20.0);
		WRITE_SHORT(g_sModelIndexFireball3);
		WRITE_BYTE(25);
		WRITE_BYTE(30);
		WRITE_BYTE(TE_EXPLFLAG_NOSOUND);
		MESSAGE_END();

		MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
		WRITE_BYTE(TE_EXPLOSION);
		WRITE_COORD(pev->origin.x + RANDOM_FLOAT(-64.0, 64.0));
		WRITE_COORD(pev->origin.y + RANDOM_FLOAT(-64.0, 64.0));
		WRITE_COORD(pev->origin.z + RANDOM_FLOAT(30.0, 35.0));
		WRITE_SHORT(g_sModelIndexFireball2);
		WRITE_BYTE(30);
		WRITE_BYTE(30);
		WRITE_BYTE(TE_EXPLFLAG_NONE);
		MESSAGE_END();

		break;

	case EXPTYPE_FGLAUNCHERROCKET:
		break;

	default:
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

		break;
	}

	CSoundEnt::InsertSound(bits_SOUND_COMBAT, pev->origin, NORMAL_EXPLOSION_VOLUME, 3s);

	entvars_t *pevOwner;
	if (pev->owner)
		pevOwner = VARS(pev->owner);
	else
		pevOwner = NULL;

	if (TheBots != NULL)
	{
		TheBots->OnEvent(EVENT_HE_GRENADE_EXPLODED, CBaseEntity::Instance(pev->owner));
	}

	pev->owner = NULL;
	float flDamage = pev->dmg;
	switch (type)
	{
	case EXPTYPE_M79ROCKET:
	case EXPTYPE_AT4ROCKET:
	case EXPTYPE_FIRECRACKERROCKET:
	case EXPTYPE_M32ROCKET:
	case EXPTYPE_OICWROCKET:
		m_bHit = RadiusDamage(pev, pevOwner, flDamage, CLASS_NONE, bitsDamageType, iRadius);

		break;
	case EXPTYPE_FIREBOMB:

		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage *= 5.0f;

		FireBombExplode(pev->origin, pev, pevOwner, flDamage, 0.05, 325);

		break;
	default:
		
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage *= 5.0f;

		RadiusDamage(pev, pevOwner, flDamage, CLASS_NONE, bitsDamageType);

		break;
	}

	if (RANDOM_FLOAT(0, 1) < 0.5f)
		UTIL_DecalTrace(pTrace, DECAL_SCORCH1);
	else
		UTIL_DecalTrace(pTrace, DECAL_SCORCH2);

	// TODO: unused
	flRndSound = RANDOM_FLOAT(0, 1);

	switch (RANDOM_LONG(0, 2))
	{
	case 0: EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/debris1.wav", 0.55, ATTN_NORM); break;
	case 1: EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/debris2.wav", 0.55, ATTN_NORM); break;
	case 2: EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/debris3.wav", 0.55, ATTN_NORM); break;
	}

	pev->effects |= EF_NODRAW;
	SetThink(&CGrenade::Smoke3_C);
	pev->velocity = g_vecZero;
	pev->nextthink = gpGlobals->time + 0.55s;

	if (type == EXPTYPE_THANATOS5ROCKET)
		return;

	int sparkCount = RANDOM_LONG(0, 3);

	for (int i = 0; i < sparkCount; ++i)
		Create("spark_shower", pev->origin, pTrace->vecPlaneNormal, NULL);

	switch (type)
	{
	case EXPTYPE_M79ROCKET:
	case EXPTYPE_AT4ROCKET:
	case EXPTYPE_FIRECRACKERROCKET:
	case EXPTYPE_M32ROCKET:
	case EXPTYPE_OICWROCKET:
		break;
	default:
	{
		CBasePlayer* player = GetClassPtr<CBasePlayer>(pevOwner);
		if (player)
		{
			player->m_pModStrategy->GrenadeExploded(pev->origin);
		}
		break;
	}
	}
	
}

NOXREF void CGrenade::SG_Explode(TraceResult *pTrace, int bitsDamageType)
{
	float flRndSound; // sound randomizer
	//entvars_t *pevOwner;

	pev->model = iStringNull; // invisible
	pev->solid = SOLID_NOT; // intangible

	pev->takedamage = DAMAGE_NO;

	if (pTrace->flFraction != 1.0f)
	{
		pev->origin = pTrace->vecEndPos + (pTrace->vecPlaneNormal * (pev->dmg - 24.0f) * 0.6f);
	}

	int iContents = UTIL_PointContents(pev->origin);
	CSoundEnt::InsertSound(bits_SOUND_COMBAT, pev->origin, NORMAL_EXPLOSION_VOLUME, 3s);

	// can't traceline attack owner if this is set
	pev->owner = NULL;

	if (RANDOM_FLOAT(0, 1) < 0.5f)
		UTIL_DecalTrace(pTrace, DECAL_SCORCH1);
	else
		UTIL_DecalTrace(pTrace, DECAL_SCORCH2);

	// TODO: unused
	flRndSound = RANDOM_FLOAT(0, 1);

	switch (RANDOM_LONG(0, 1))
	{
	case 0: EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/flashbang-2.wav", 0.55, ATTN_NORM); break;
	case 1: EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/flashbang-1.wav", 0.55, ATTN_NORM); break;
	}

	pev->effects |= EF_NODRAW;
	SetThink(&CGrenade::Smoke);
	pev->velocity = g_vecZero;
	pev->nextthink = gpGlobals->time + 0.1s;

	if (iContents != CONTENTS_WATER)
	{
		int sparkCount = RANDOM_LONG(0, 3);

		for (int i = 0; i < sparkCount; ++i)
			Create("spark_shower", pev->origin, pTrace->vecPlaneNormal, NULL);
	}
}

void CGrenade::Smoke3_C()
{
	if (UTIL_PointContents(pev->origin) == CONTENTS_WATER)
	{
		UTIL_Bubbles(pev->origin - Vector(64, 64, 64), pev->origin + Vector(64, 64, 64), 100);
	}
	else
	{
		MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
			WRITE_BYTE(TE_SMOKE);
			WRITE_COORD(pev->origin.x);
			WRITE_COORD(pev->origin.y);
			WRITE_COORD(pev->origin.z - 5.0f);
			WRITE_SHORT(g_sModelIndexSmoke);
			WRITE_BYTE(35 + RANDOM_FLOAT(0, 10)); // scale * 10
			WRITE_BYTE(5); // framerate
		MESSAGE_END();
	}

	UTIL_Remove(this);
}

void CGrenade::Smoke3_B()
{
	if (UTIL_PointContents(pev->origin) == CONTENTS_WATER)
	{
		UTIL_Bubbles(pev->origin - Vector(64, 64, 64), pev->origin + Vector(64, 64, 64), 100);
	}
	else
	{
		MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
			WRITE_BYTE(TE_SMOKE);
			WRITE_COORD(pev->origin.x + RANDOM_FLOAT(-128, 128));
			WRITE_COORD(pev->origin.y + RANDOM_FLOAT(-128, 128));
			WRITE_COORD(pev->origin.z + RANDOM_FLOAT(-10, 10));
			WRITE_SHORT(g_sModelIndexSmoke);
			WRITE_BYTE(15 + RANDOM_FLOAT(0, 10)); // scale * 10
			WRITE_BYTE(10); // framerate
		MESSAGE_END();
	}

	pev->nextthink = gpGlobals->time + 0.15s;
	SetThink(&CGrenade::Smoke3_A);
}

void CGrenade::Smoke3_A()
{
	if (UTIL_PointContents(pev->origin) == CONTENTS_WATER)
	{
		UTIL_Bubbles(pev->origin - Vector(64, 64, 64), pev->origin + Vector(64, 64, 64), 100);
	}
	else
	{
		MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
			WRITE_BYTE(TE_SMOKE);
			WRITE_COORD(pev->origin.x + RANDOM_FLOAT(-128, 128));
			WRITE_COORD(pev->origin.y + RANDOM_FLOAT(-128, 128));
			WRITE_COORD(pev->origin.z + RANDOM_FLOAT(-10, 10));
			WRITE_SHORT(g_sModelIndexSmoke);
			WRITE_BYTE(15 + RANDOM_FLOAT(0, 10)); // scale * 10
			WRITE_BYTE(12); // framerate
		MESSAGE_END();
	}
}

void CGrenade::Smoke2()
{
	if (UTIL_PointContents(pev->origin) == CONTENTS_WATER)
	{
		UTIL_Bubbles(pev->origin - Vector(64, 64, 64), pev->origin + Vector(64, 64, 64), 100);
	}
	else
	{
		MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
			WRITE_BYTE(TE_SMOKE);
			WRITE_COORD(pev->origin.x);
			WRITE_COORD(pev->origin.y);
			WRITE_COORD(pev->origin.z);
			WRITE_SHORT(g_sModelIndexSmoke);
			WRITE_BYTE(150); // scale * 10
			WRITE_BYTE(8); // framerate
		MESSAGE_END();
	}

	UTIL_Remove(this);
}

void CGrenade::Smoke()
{
	if (UTIL_PointContents(pev->origin) == CONTENTS_WATER)
	{
		UTIL_Bubbles(pev->origin - Vector(64, 64, 64), pev->origin + Vector(64, 64, 64), 100);
	}
	else
	{
		MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
			WRITE_BYTE(TE_SMOKE);
			WRITE_COORD(pev->origin.x);
			WRITE_COORD(pev->origin.y);
			WRITE_COORD(pev->origin.z);
			WRITE_SHORT(g_sModelIndexSmoke);
			WRITE_BYTE(25); // scale * 10
			WRITE_BYTE(6); // framerate
		MESSAGE_END();
	}

	UTIL_Remove(this);
}

void CGrenade::SG_Smoke()
{
	int iMaxSmokePuffs = 100;
	float flSmokeInterval;

	if (UTIL_PointContents(pev->origin) == CONTENTS_WATER)
	{
		UTIL_Bubbles(pev->origin - Vector(64, 64, 64), pev->origin + Vector(64, 64, 64), 100);
	}
	else
	{
		Vector origin, angle;
		float x_old, y_old, R_angle;

		UTIL_MakeVectors(pev->angles);

		origin = gpGlobals->v_forward * RANDOM_FLOAT(3, 8);

		flSmokeInterval = RANDOM_FLOAT(1.5, 3.5) * iMaxSmokePuffs;

		R_angle = m_angle / (180.00433335 / M_PI);

		x_old = cos((float)R_angle);
		y_old = sin((float)R_angle);

		angle.x = origin.x * x_old - origin.y * y_old;
		angle.y = origin.x * y_old + origin.y * x_old;

		m_angle = (m_angle + 30) % 360;

		PLAYBACK_EVENT_FULL(0, NULL, m_usEvent, 0, pev->origin, m_vSmokeDetonate, angle.x, angle.y, flSmokeInterval, 4, m_bLightSmoke, 6);
	}

	if (m_SGSmoke <= 20)
	{
		pev->nextthink = gpGlobals->time + 1.0s;
		SetThink(&CGrenade::SG_Smoke);
		++m_SGSmoke;
	}
	else
	{
		pev->effects |= EF_NODRAW;

		if (TheBots != NULL)
		{
			TheBots->RemoveGrenade(this);
		}
		UTIL_Remove(this);
	}
}

void CGrenade::Killed(entvars_t *pevAttacker, int iGib)
{
	Detonate();
}

// Timed grenade, this think is called when time runs out.

void CGrenade::DetonateUse(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	SetThink(&CGrenade::Detonate);
	pev->nextthink = gpGlobals->time;
}

void CGrenade::PreDetonate()
{
	CSoundEnt::InsertSound(bits_SOUND_DANGER, pev->origin, 400, 0.3s);

	SetThink(&CGrenade::Detonate);
	pev->nextthink = gpGlobals->time + 1.0s;
}

void CGrenade::Detonate()
{
	TraceResult tr;
	Vector vecSpot; // trace starts here!

	vecSpot = pev->origin + Vector(0, 0, 8);
	UTIL_TraceLine(vecSpot, vecSpot + Vector(0, 0, -40), ignore_monsters, ENT(pev), &tr);
	Explode(&tr, DMG_BLAST);
}

void CGrenade::SG_Detonate()
{
	if (to_int(g_pModRunning->QueryModConfig("EnableForstBomb")))
	{
#define NADE_HEGRENADE_RADIUS 420
#define NADE_SMOKEGRENADE_RADIUS 420
		EMIT_SOUND(ENT(pev), CHAN_WEAPON, "warcraft3/impalehit.wav", VOL_NORM, ATTN_NORM);	//preache

		Vector vecOrigin = pev->origin;

		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY, vecOrigin);
		WRITE_BYTE(TE_TEMPSPRITE);
		WRITE_COORD(vecOrigin.x);
		WRITE_COORD(vecOrigin.y);
		WRITE_COORD(vecOrigin.z + 50.0);
		WRITE_SHORT(g_sModelIndexFrostExp);
		WRITE_BYTE(15);
		WRITE_BYTE(255);
		WRITE_BYTE(100);
		MESSAGE_END();

		MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, vecOrigin);
		WRITE_BYTE(TE_BEAMCYLINDER);
		WRITE_COORD(vecOrigin[0]);
		WRITE_COORD(vecOrigin[1]);
		WRITE_COORD(vecOrigin[2]);
		WRITE_COORD(vecOrigin[0]);
		WRITE_COORD(vecOrigin[1]);
		WRITE_COORD(vecOrigin[2] + NADE_HEGRENADE_RADIUS);
		WRITE_SHORT(g_sModelIndexShockWave);
		WRITE_BYTE(0); // Start Frame
		WRITE_BYTE(20); // Framerate
		WRITE_BYTE(4); // Live Time
		WRITE_BYTE(10); // Width
		WRITE_BYTE(10); // Noise
		WRITE_BYTE(0); // R
		WRITE_BYTE(255); // G
		WRITE_BYTE(255); // B
		WRITE_BYTE(255); // Bright
		WRITE_BYTE(9); // Speed
		MESSAGE_END();

		MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, vecOrigin);
		WRITE_BYTE(TE_BEAMCYLINDER);
		WRITE_COORD(vecOrigin[0]);
		WRITE_COORD(vecOrigin[1]);
		WRITE_COORD(vecOrigin[2]);
		WRITE_COORD(vecOrigin[0]);
		WRITE_COORD(vecOrigin[1]);
		WRITE_COORD(vecOrigin[2] + NADE_HEGRENADE_RADIUS);
		WRITE_SHORT(g_sModelIndexShockWave);
		WRITE_BYTE(0); // Start Frame
		WRITE_BYTE(10); // Framerate
		WRITE_BYTE(4); // Live Time
		WRITE_BYTE(10); // Width
		WRITE_BYTE(20); // Noise
		WRITE_BYTE(0); // R
		WRITE_BYTE(0); // G
		WRITE_BYTE(255); // B
		WRITE_BYTE(150); // Bright
		WRITE_BYTE(9); // Speed
		MESSAGE_END();

		MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, vecOrigin);
		WRITE_BYTE(TE_SPRITETRAIL);
		WRITE_COORD(vecOrigin[0]);
		WRITE_COORD(vecOrigin[1]);
		WRITE_COORD(vecOrigin[2] + 200.0);
		WRITE_COORD(vecOrigin[0]);// velocity
		WRITE_COORD(vecOrigin[1]);
		WRITE_COORD(vecOrigin[2] + 30.0);
		WRITE_SHORT(g_sModelIndexFrostGibs);
		WRITE_BYTE(60); // count
		WRITE_BYTE(RANDOM_LONG(27,30)); // life
		WRITE_BYTE(2); // scale
		WRITE_BYTE(50); // velocity along vector in 10's
		WRITE_BYTE(10); // randomness of velocity in 10's
		MESSAGE_END();

		CBaseEntity * pEntity = NULL;
		while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecOrigin, NADE_SMOKEGRENADE_RADIUS)) != NULL)
		{
			if (!pEntity->IsPlayer())
				continue;

			if (!pEntity->IsAlive())
				continue;

			CBasePlayer* pPlayer = (CBasePlayer*)pEntity;
			if (pPlayer->m_bIsZombie)
				continue;

			//freeze

		}
		pev->effects |= EF_NODRAW;

		if (TheBots != NULL)
		{
			TheBots->RemoveGrenade(this);
		}
		UTIL_Remove(this);
		return;
	}
	TraceResult tr;
	Vector vecSpot;
	edict_t *pentFind = NULL;

	vecSpot = pev->origin + Vector(0, 0, 8);

	UTIL_TraceLine(vecSpot, vecSpot + Vector(0, 0, -40), ignore_monsters, ENT(pev), &tr);

	if (TheBots != NULL)
	{
		TheBots->OnEvent(EVENT_SMOKE_GRENADE_EXPLODED, CBaseEntity::Instance(pev->owner));
		TheBots->AddGrenade(WEAPON_SMOKEGRENADE, this);
	}

	EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/sg_explode.wav", VOL_NORM, ATTN_NORM);

	while ((pentFind = FIND_ENTITY_BY_CLASSNAME(pentFind, "grenade")) != NULL)
	{
		if (FNullEnt(pentFind))
			break;

		CBaseEntity *pEnt = CBaseEntity::Instance(pentFind);

		if (pEnt)
		{
			//float
			float fDistance = (pEnt->pev->origin - pev->origin).Length();

			if (fDistance != 0.0f && fDistance <= 250.0f)
			{
				if (gpGlobals->time > pEnt->pev->dmgtime)
				{
					m_bLightSmoke = true;
				}
			}
		}
	}

	m_bDetonated = true;
	PLAYBACK_EVENT_FULL(0, NULL, m_usEvent, 0, pev->origin, g_vecZero, 0, 0, 0, 1, m_bLightSmoke, FALSE);
	m_vSmokeDetonate = pev->origin;

	pev->velocity.x = RANDOM_FLOAT(-175, 175);
	pev->velocity.y = RANDOM_FLOAT(-175, 175);
	pev->velocity.z = RANDOM_FLOAT(250, 350);

	pev->nextthink = gpGlobals->time + 0.1s;
	SetThink(&CGrenade::SG_Smoke);
}

void CGrenade::Detonate2()
{
	TraceResult tr;
	Vector vecSpot;// trace starts here!

	vecSpot = pev->origin + Vector(0, 0, 8);

	UTIL_TraceLine(vecSpot, vecSpot + Vector(0, 0, -40), ignore_monsters, ENT(pev), &tr);
	Explode2(&tr, DMG_BLAST);
}

void CGrenade::Detonate3()
{
	TraceResult tr;
	Vector vecSpot;// trace starts here!

	vecSpot = pev->origin + Vector(0, 0, 8);
	UTIL_TraceLine(vecSpot, vecSpot + Vector(0, 0, -40), ignore_monsters, ENT(pev), &tr);
	Explode3(&tr, DMG_EXPLOSION, EXPTYPE_NORMAL, 0.0);
}


void CGrenade::DetonateCartFrag(void)
{
	TraceResult tr;
	Vector vecStart = pev->origin - Vector(0, 0, 32);

	UTIL_TraceLine(pev->origin + Vector(0, 0, 8), pev->origin - Vector(0, 0, 32), ignore_monsters, ENT(pev), &tr);
	Explode3(&tr, DMG_EXPLOSION, EXPTYPE_CARTFRAG, 0.0);
}

void CGrenade::DetonateEventBomb(void)
{
	TraceResult tr;
	Vector vecStart = pev->origin - Vector(0, 0, 32);

	UTIL_TraceLine(pev->origin + Vector(0, 0, 8), pev->origin - Vector(0, 0, 32), ignore_monsters, ENT(pev), &tr);
	Explode3(&tr, DMG_EXPLOSION, EXPTYPE_EVENTBOMB, 0.0);
}

void CGrenade::DetonateMoonCake(void)
{
	TraceResult tr;
	Vector vecStart = pev->origin - Vector(0, 0, 32);

	UTIL_TraceLine(pev->origin + Vector(0, 0, 8), pev->origin - Vector(0, 0, 32), ignore_monsters, ENT(pev), &tr);
	Explode3(&tr, DMG_EXPLOSION, EXPTYPE_MOONCAKE, 0.0);
}

void CGrenade::ZombieBombExplosion()
{
	Vector vecSpot;// trace starts here!

	vecSpot = pev->origin - Vector(0, 0, 20.0);

	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
	WRITE_BYTE(TE_EXPLOSION);	// This makes a dynamic light and the explosion sprites/sound
	WRITE_COORD(pev->origin.x);		// Send to PAS because of the sound
	WRITE_COORD(pev->origin.y);
	WRITE_COORD(pev->origin.z);
	WRITE_SHORT(g_sModelIndexZombiebomb_exp);
	WRITE_BYTE(40);			// scale * 10
	WRITE_BYTE(30);		// framerate
	WRITE_BYTE(TE_EXPLFLAG_NODLIGHTS | TE_EXPLFLAG_NOPARTICLES | TE_EXPLFLAG_NOSOUND);	// flags
	MESSAGE_END();

	EMIT_SOUND(ENT(pev), CHAN_WEAPON, "zombi/zombi_bomb_exp.wav", 0.25, ATTN_NORM);
	entvars_t* pevAttacker = VARS(pev->owner);
	sv::CGrenade::ZombieBombKnockback(vecSpot, pev, pevAttacker, pev->dmg);

	pev->effects = EF_NODRAW;
	pev->flags |= FL_KILLME;
}
// Contact grenade, explode when it touches something

void CGrenade::ExplodeTouch3(CBaseEntity* pOther, int iRadius, E_EXPLODE_TYPE type, bool bKickRate)
{
	TraceResult tr;

	if (pOther)
		pev->enemy = ENT(pOther->pev);
	else
		pev->enemy = NULL;

	UTIL_TraceLine(pev->origin - pev->velocity.Normalize() * 32, pev->origin + pev->velocity.Normalize() * 32, ignore_monsters, ENT(pev), &tr);

	Explode3(&tr, DMG_EXPLOSION, type, iRadius);

	if (bKickRate)
		KickRate(NULL, NULL, ((rand() % 6) / 10.0 + 1.7) * 350.0, 250.0, 0.0, DMGFLAG_TRONLY, 0.0);
}

void CGrenade::ExplodeTouch(CBaseEntity *pOther)
{
	TraceResult tr;
	Vector vecSpot; // trace starts here!

	pev->enemy = pOther->edict();

	vecSpot = pev->origin - pev->velocity.Normalize() * 32.0f;
	UTIL_TraceLine(vecSpot, vecSpot + pev->velocity.Normalize() * 64, ignore_monsters, ENT(pev), &tr);
	Explode(&tr, DMG_BLAST);
}

void CGrenade::DangerSoundThink()
{
	if (!IsInWorld())
	{
		UTIL_Remove(this);
		return;
	}

	CSoundEnt::InsertSound(bits_SOUND_DANGER, pev->origin + pev->velocity * 0.5, pev->velocity.Length(), 0.2s);
	pev->nextthink = gpGlobals->time + 0.2s;

	if (pev->waterlevel != 0)
	{
		pev->velocity = pev->velocity * 0.5f;
	}
}

void CGrenade::BounceTouch(CBaseEntity *pOther)
{
	// don't hit the guy that launched this grenade
	if (pOther->edict() == pev->owner)
		return;

	if (FClassnameIs(pOther->pev, "func_breakable") && pOther->pev->rendermode != kRenderNormal)
	{
		pev->velocity = pev->velocity * -2.0f;
		return;
	}

	Vector vecTestVelocity;

	// this is my heuristic for modulating the grenade velocity because grenades dropped purely vertical
	// or thrown very far tend to slow down too quickly for me to always catch just by testing velocity.
	// trimming the Z velocity a bit seems to help quite a bit.
	vecTestVelocity = pev->velocity;
	vecTestVelocity.z *= 0.7f;

	if (!m_fRegisteredSound && vecTestVelocity.Length() <= 60.0f)
	{
		// grenade is moving really slow. It's probably very close to where it will ultimately stop moving.
		// go ahead and emit the danger sound.

		// register a radius louder than the explosion, so we make sure everyone gets out of the way
		CSoundEnt::InsertSound(bits_SOUND_DANGER, pev->origin, pev->dmg / 0.4f, 0.3s);
		m_fRegisteredSound = TRUE;
	}

	if (pev->flags & FL_ONGROUND)
	{
		// add a bit of static friction
		pev->velocity = pev->velocity * 0.8f;
		pev->sequence = RANDOM_LONG(1, 1);	// TODO: what?
	}
	else
	{
		if (m_iBounceCount < 5)
		{
			// play bounce sound
			BounceSound();
		}

		if (m_iBounceCount >= 10)
		{
			pev->groundentity = ENT(0);
			pev->flags |= FL_ONGROUND;
			pev->velocity = g_vecZero;
		}

		++m_iBounceCount;
	}

	if (!m_bStaticFramerate)
	{
		pev->framerate = pev->velocity.Length() / 200.0f;

		if (pev->framerate > 1)
		{
			pev->framerate = 1.0f;
		}
		else if (pev->framerate < 0.5f)
		{
			pev->framerate = 0.0f;
		}
	}
}

void CGrenade::SlideTouch(CBaseEntity *pOther)
{
	// don't hit the guy that launched this grenade
	if (pOther->edict() == pev->owner)
		return;

	if (pev->flags & FL_ONGROUND)
	{
		// add a bit of static friction
		pev->velocity = pev->velocity * 0.95f;
	}
	else
	{
		BounceSound();
	}
}

void CGrenade::BounceSound()
{
	if (pev->dmg == 999)
	{
		EMIT_SOUND(ENT(pev), CHAN_VOICE, RANDOM_LONG(0, 1) ? "zombi/zombi_bomb_bounce_2.wav" : "zombi/zombi_bomb_bounce_1.wav", 0.25, ATTN_NORM);
		return;
	}

	if (pev->dmg > 50.0f)
	{
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/he_bounce-1.wav", 0.25, ATTN_NORM);
		return;
	}

	switch (RANDOM_LONG(0, 2))
	{
	case 0:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/grenade_hit1.wav", 0.25, ATTN_NORM); break;
	case 1:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/grenade_hit2.wav", 0.25, ATTN_NORM); break;
	case 2:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/grenade_hit3.wav", 0.25, ATTN_NORM); break;
	}
}

void CGrenade::TumbleThink()
{
	if (!IsInWorld())
	{
		UTIL_Remove(this);
		return;
	}

	CalculateFramerate();
	StudioFrameAdvance();
	pev->nextthink = gpGlobals->time + 0.1s;

	if (pev->dmgtime - 1s < gpGlobals->time)
	{
		CSoundEnt::InsertSound(bits_SOUND_DANGER, pev->origin + pev->velocity * (pev->dmgtime - gpGlobals->time).count(), 400, 0.1s);
	}

	if (pev->dmgtime <= gpGlobals->time)
	{
		if (pev->dmg == 888)
		{
			pev->dmg = 100;

			SetThink(&CGrenade::DetonateEventBomb);
		}
		else if (pev->dmg == 788)
		{
			pev->dmg = 100;

			SetThink(&CGrenade::DetonateMoonCake);
		}
		else if (pev->dmg == 156)
		{
			pev->dmg = 100;

			SetThink(&CGrenade::DetonateFireBomb);
		}
		else if (pev->dmg == 766)
		{
			pev->dmg = 100;

			SetThink(&CGrenade::DetonateCartFrag);
		}
		else if (pev->dmg == 765)
		{
			pev->dmg = 100;

			SetThink(&CGrenade::SFGrenadeExplodeThink);
		}
		else if (pev->dmg > 40.0f)
		{
			SetThink(&CGrenade::Detonate3);		
		}
		else
			SetThink(&CGrenade::Detonate);
	}

	if (pev->waterlevel != 0)
	{
		pev->velocity = pev->velocity * 0.5f;
		pev->framerate = 0.2f;
	}
}

void CGrenade::ZombieBomb_TumbleThink()
{
	if (!IsInWorld())
	{
		UTIL_Remove(this);
		return;
	}

	float flSpeed = pev->velocity.Length();

	if (flSpeed > 100)
		flSpeed = 100;

	pev->framerate = flSpeed / 20.0;

	StudioFrameAdvance();
	pev->nextthink = gpGlobals->time + 0.05s;

	if (pev->dmgtime <= gpGlobals->time)
	{
		SetThink(&CGrenade::ZombieBombExplosion);
	}

	if (pev->waterlevel != 0)
	{
		pev->velocity = pev->velocity * 0.5f;
		pev->framerate = 0.2f;
	}
}

void CGrenade::SG_TumbleThink()
{
	if (!IsInWorld())
	{
		UTIL_Remove(this);
		return;
	}

	if (pev->flags & FL_ONGROUND)
	{
		pev->velocity = pev->velocity * 0.95f;
	}

	StudioFrameAdvance();
	pev->nextthink = gpGlobals->time + 0.1s;

	if (pev->dmgtime - 1s < gpGlobals->time)
	{
		CSoundEnt::InsertSound(bits_SOUND_DANGER, pev->origin + pev->velocity * (pev->dmgtime - gpGlobals->time).count(), 400, 0.1s);
	}

	if (pev->dmgtime <= gpGlobals->time)
	{
		if (pev->flags & FL_ONGROUND)
		{
			SetThink(&CGrenade::SG_Detonate);
		}
	}

	if (pev->waterlevel != 0)
	{
		pev->velocity = pev->velocity * 0.5f;
		pev->framerate = 0.2f;
	}
}

void CGrenade::Spawn()
{
	m_iBounceCount = 0;
	pev->movetype = MOVETYPE_BOUNCE;

	if (pev->classname)
	{
		RemoveEntityHashValue(pev, STRING(pev->classname), CLASSNAME);
	}

	MAKE_STRING_CLASS("grenade", pev);
	AddEntityHashValue(pev, STRING(pev->classname), CLASSNAME);

	m_bIsC4 = false;
	pev->solid = SOLID_BBOX;

	SET_MODEL(ENT(pev), "models/grenade.mdl");
	UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));

	pev->dmg = 30.0f;
	m_fRegisteredSound = FALSE;
}

NOXREF CGrenade *CGrenade::ShootContact(entvars_t *pevOwner, Vector vecStart, Vector vecVelocity)
{
	CGrenade *pGrenade = CreateClassPtr<CGrenade>();
	pGrenade->Spawn();

	// contact grenades arc lower
	pGrenade->pev->gravity = 0.5f;	// lower gravity since grenade is aerodynamic and engine doesn't know it.

	UTIL_SetOrigin(pGrenade->pev, vecStart);
	pGrenade->pev->velocity = vecVelocity;
	pGrenade->pev->angles = UTIL_VecToAngles(pGrenade->pev->velocity);
	pGrenade->pev->owner = ENT(pevOwner);

	// make monsters afaid of it while in the air
	pGrenade->SetThink(&CGrenade::DangerSoundThink);
	pGrenade->pev->nextthink = gpGlobals->time;

	// Tumble in air
	pGrenade->pev->avelocity.x = RANDOM_FLOAT(-100, -500);

	// Explode on contact
	pGrenade->SetTouch(&CGrenade::ExplodeTouch);

	pGrenade->pev->dmg = gSkillData.plrDmgM203Grenade;
	pGrenade->m_bJustBlew = true;
	return pGrenade;
}

CGrenade *CGrenade::ShootTimed2(entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, duration_t time, int iTeam, unsigned short usEvent)
{
	CGrenade *pGrenade = CreateClassPtr<CGrenade>();
	pGrenade->Spawn();

	UTIL_SetOrigin(pGrenade->pev, vecStart);
	pGrenade->pev->velocity = vecVelocity;
	pGrenade->pev->angles = pevOwner->angles;
	pGrenade->pev->owner = ENT(pevOwner);
	pGrenade->m_usEvent = usEvent;
	pGrenade->SetTouch(&CGrenade::BounceTouch);
	pGrenade->pev->dmgtime = gpGlobals->time + time;
	pGrenade->SetThink(&CGrenade::TumbleThink);
	pGrenade->pev->nextthink = gpGlobals->time + 0.1s;
	pGrenade->pev->sequence = RANDOM_LONG(3, 6);
	pGrenade->pev->framerate = 1.0f;
	pGrenade->m_bJustBlew = true;
	pGrenade->pev->gravity = 0.55f;
	pGrenade->pev->friction = 0.7f;
	pGrenade->m_iTeam = iTeam;
	pGrenade->m_bStaticFramerate = false;
	CBasePlayer* pOwner = (CBasePlayer*)CBaseEntity::Instance(pevOwner);

	pGrenade->m_iGrenadeID = pOwner->m_iGrenadeID;
	switch (pOwner->m_iGrenadeID)
	{
	case GRENADE_M24GRENADE:
		SET_MODEL(ENT(pGrenade->pev), "models/w_m24grenade.mdl");

		pGrenade->m_bStaticFramerate = true;
		pGrenade->pev->dmg = 100;
		break;

	case GRENADE_FGRENADE2:
		SET_MODEL(ENT(pGrenade->pev), "models/w_fgrenade2.mdl");

		pGrenade->m_bStaticFramerate = true;
		pGrenade->pev->dmg = 125;
		break;

	case GRENADE_FIREBOMB:
		SET_MODEL(ENT(pGrenade->pev), "models/w_fgrenade.mdl");

		pGrenade->m_bStaticFramerate = true;
		pGrenade->pev->dmg = 156;
		break;

	case GRENADE_HEARTBOMB:
	case GRENADE_CAKEBOMB:
		SET_MODEL(ENT(pGrenade->pev), pOwner->m_iGrenadeID == GRENADE_HEARTBOMB ? "models/w_heartbomb.mdl" : "models/w_cake.mdl");

		pGrenade->m_bStaticFramerate = false;
		pGrenade->pev->dmg = 888;
		break;

	case GRENADE_MOONCAKE:
		SET_MODEL(ENT(pGrenade->pev), "models/w_mooncake.mdl");

		pGrenade->m_bStaticFramerate = false;
		pGrenade->pev->dmg = 788;
		break;

	case GRENADE_CARTFRAG:
		SET_MODEL(ENT(pGrenade->pev), "models/w_cartfrag.mdl");

		pGrenade->m_bStaticFramerate = true;
		pGrenade->pev->dmg = 766;
		break;

	case GRENADE_HOLYBOMB:
		SET_MODEL(ENT(pGrenade->pev), "models/w_holybomb.mdl");

		pGrenade->m_bStaticFramerate = true;
		pGrenade->pev->dmg = 100;

		pGrenade->SetTouch(&CGrenade::HolyBombTouch);
		pGrenade->SetThink(&CGrenade::HolyBombThink);
		break;

	case GRENADE_SFGRENADE:
		SET_MODEL(ENT(pGrenade->pev), "models/w_sfgrenade.mdl");

		pGrenade->pev->movetype = MOVETYPE_TOSS;
		pGrenade->m_bStaticFramerate = true;
		pGrenade->pev->dmg = 765;
		pGrenade->m_pStickingObject = NULL;

		pGrenade->SetTouch(&CGrenade::SFGrenadeTouch);

		break;

	default:
		SET_MODEL(ENT(pGrenade->pev), "models/w_hegrenade.mdl");
		pGrenade->pev->dmg = 100.0f;

		break;
	}

	/*CBasePlayer* player = GetClassPtr<CBasePlayer>(pevOwner);
	if (player)
	{
		player->m_pModStrategy->ShootGrenade(vecStart, vecVelocity, time, iTeam, usEvent);
	}*/
	return pGrenade;
}

CGrenade *CGrenade::ShootTimed(entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, duration_t time)
{
	CGrenade *pGrenade = CreateClassPtr<CGrenade>();
	pGrenade->Spawn();

	UTIL_SetOrigin(pGrenade->pev, vecStart);
	pGrenade->pev->velocity = vecVelocity;
	pGrenade->pev->angles = pevOwner->angles;
	pGrenade->pev->owner = ENT(pevOwner);

	// Bounce if touched
	pGrenade->SetTouch(&CGrenade::BounceTouch);

	// Take one second off of the desired detonation time and set the think to PreDetonate. PreDetonate
	// will insert a DANGER sound into the world sound list and delay detonation for one second so that
	// the grenade explodes after the exact amount of time specified in the call to ShootTimed().

	pGrenade->pev->dmgtime = gpGlobals->time + time;
	pGrenade->SetThink(&CGrenade::TumbleThink);
	pGrenade->pev->nextthink = gpGlobals->time + 0.1s;

	if (time < 0.1s)
	{
		pGrenade->pev->nextthink = gpGlobals->time;
		pGrenade->pev->velocity = Vector(0, 0, 0);
	}

	pGrenade->pev->sequence = RANDOM_LONG(3, 6);
	pGrenade->pev->framerate = 1.0f;

	pGrenade->m_bJustBlew = true;

	pGrenade->pev->gravity = 0.5f;
	pGrenade->pev->friction = 0.8f;
	pGrenade->m_bStaticFramerate = false;
	SET_MODEL(ENT(pGrenade->pev), "models/w_flashbang.mdl");
	pGrenade->pev->dmg = 35.0f;

	return pGrenade;
}

CGrenade* CGrenade::ShootZombieBomb(entvars_t* pevOwner, Vector vecStart, Vector vecVelocity, duration_t time, int iTeam, unsigned short usEvent)
{
	CGrenade* pGrenade = CreateClassPtr<CGrenade>();
	pGrenade->Spawn();
	
	SET_MODEL(ENT(pGrenade->pev), "models/w_zombibomb.mdl");
	UTIL_SetOrigin(pGrenade->pev, vecStart);
	pGrenade->pev->velocity = vecVelocity;
	pGrenade->pev->angles = pevOwner->angles;
	pGrenade->pev->owner = ENT(pevOwner);

	pGrenade->SetTouch(&CGrenade::BounceTouch);

	pGrenade->pev->sequence = RANDOM_LONG(1, 3);
	pGrenade->pev->frame = 0.0;
	pGrenade->ResetSequenceInfo();
	pGrenade->pev->framerate = 5.0f;
	pGrenade->m_fSequenceLoops = TRUE;
	pGrenade->pev->nextthink = gpGlobals->time + 0.05s;
	pGrenade->pev->dmgtime = gpGlobals->time + time;
	pGrenade->pev->dmg = 999;
	pGrenade->pev->health = 100;	//damage
	pGrenade->pev->fuser1 = 350.0; //kickrate;
	pGrenade->pev->fuser2 = 250.0; //radius;
	pGrenade->m_bStaticFramerate = true;
	pGrenade->SetThink(&CGrenade::ZombieBomb_TumbleThink);

	pGrenade->m_bJustBlew = true;

	pGrenade->pev->gravity = 0.55f;
	pGrenade->pev->friction = 0.7f;

	pGrenade->m_iTeam = iTeam;

	return pGrenade;
}


CGrenade* CGrenade::ShootZombiBomb2(entvars_t* pevOwner, Vector vecStart, Vector vecVelocity, duration_t time, int iTeam, unsigned short usEvent, float flDamage)
{
	CGrenade* pGrenade = CreateClassPtr<CGrenade>();
	pGrenade->Spawn();
	
	SET_MODEL(ENT(pGrenade->pev), "models/@frogvari_w_zombibomb.mdl");
	UTIL_SetOrigin(pGrenade->pev, vecStart);
	pGrenade->pev->velocity = vecVelocity;
	pGrenade->pev->angles = pevOwner->angles;
	pGrenade->pev->owner = ENT(pevOwner);

	pGrenade->SetTouch(&CGrenade::ZombiBomb2Touch);

	pGrenade->pev->sequence = RANDOM_LONG(1, 3);
	pGrenade->pev->frame = 0.0;
	pGrenade->ResetSequenceInfo();
	pGrenade->pev->framerate = 5.0f;
	pGrenade->m_fSequenceLoops = TRUE;
	pGrenade->pev->nextthink = gpGlobals->time + 0.05s;
	pGrenade->pev->dmgtime = gpGlobals->time + time;
	pGrenade->pev->dmg = 999;
	pGrenade->pev->solid = SOLID_TRIGGER;
	pGrenade->pev->health = flDamage;
	pGrenade->pev->fuser1 = 450.0; //kickrate;
	pGrenade->pev->fuser2 = 250.0; //radius;
	pGrenade->m_bStaticFramerate = true;
	UTIL_SetSize(pGrenade->pev, Vector(-4, -4, -4), Vector(4, 4, 4));
	pGrenade->SetThink(&CGrenade::ZombieBomb_TumbleThink);

	pGrenade->m_bJustBlew = true;

	pGrenade->pev->gravity = 0.55f;
	pGrenade->pev->friction = 0.7f;

	pGrenade->m_iTeam = iTeam;

	return pGrenade;
}


void CGrenade::ZombiBomb2Touch(CBaseEntity* pOther)
{
	if (pOther->IsPlayer())
	{
		if (!FBitSet(pev->flags, FL_ONGROUND) && pev->owner == pOther->edict())
			return;

		if (FBitSet(pev->flags, FL_ONGROUND))
		{
			if (((CBasePlayer*)pOther)->m_bIsZombie)
			{
				pOther->pev->velocity *= 1.5;
				pOther->pev->velocity.z = 250;
			}
		}

		ZombieBombExplosion();

		pOther->pev->velocity.z *= 0.85;
	}
	else
	{
		if (FClassnameIs(pOther->pev, "func_breakable") && pOther->pev->rendermode != kRenderNormal)
		{
			pev->velocity *= -2;
		}
		else
		{
			Vector testVelocity = pev->velocity;
			testVelocity.z *= 0.7;

			if (!m_fRegisteredSound && testVelocity.LengthSquared() <= 3600)
			{
				CSoundEnt::InsertSound(bits_SOUND_DANGER, pev->origin, pev->dmg * 2.5, 0.3s);
				m_fRegisteredSound = TRUE;
			}

			if (pev->flags & FL_ONGROUND)
			{
				pev->velocity *= 0.8;
				pev->sequence = RANDOM_LONG(1, 1);
			}
			else
			{
				BounceSound();
			}
		}
	}
}


CGrenade* CGrenade::ShootMolotov(entvars_t* pevOwner, Vector vecStart, Vector vecVelocity, duration_t time, unsigned short usEvent)
{
	CGrenade* pGrenade = CreateClassPtr<CGrenade>();
	pGrenade->Spawn();

	UTIL_SetOrigin(pGrenade->pev, vecStart);
	pGrenade->pev->velocity = vecVelocity;
	pGrenade->pev->angles = pevOwner->angles;
	pGrenade->pev->owner = ENT(pevOwner);
	pGrenade->m_usEvent = usEvent;
	pGrenade->m_bLightSmoke = false;
	pGrenade->m_bDetonated = false;
	pGrenade->SetTouch(&CGrenade::FB_BounceTouch);
	pGrenade->pev->dmgtime = gpGlobals->time + time;
	pGrenade->SetThink(&CGrenade::FB_TumbleThink);
	pGrenade->pev->nextthink = gpGlobals->time + 0.1s;

	if (time < 0.1s)
	{
		pGrenade->pev->nextthink = gpGlobals->time;
		pGrenade->pev->velocity = Vector(0, 0, 0);
	}

	pGrenade->pev->sequence = RANDOM_LONG(4, 6);
	pGrenade->pev->framerate = 1.0f;
	pGrenade->m_bJustBlew = true;
	pGrenade->pev->gravity = 0.5f;
	pGrenade->pev->friction = 0.8f;
	pGrenade->m_bStaticFramerate = false;
	SET_MODEL(ENT(pGrenade->pev), "models/w_molotov.mdl");
	pGrenade->pev->dmg = 35.0f;

	return pGrenade;
}

void CGrenade::FB_BounceTouch(CBaseEntity* pOther)
{
	// don't hit the guy that launched this grenade
	if (pOther->edict() == pev->owner)
		return;

	if (FClassnameIs(pOther->pev, "func_breakable"))
	{
		return;
	}
	// don't detonate on ladders
	if (FClassnameIs(pOther->pev, "func_ladder"))
	{
		return;
	}


	// this is my heuristic for modulating the grenade velocity because grenades dropped purely vertical
	// or thrown very far tend to slow down too quickly for me to always catch just by testing velocity.
	// trimming the Z velocity a bit seems to help quite a bit.
	Vector vecTestVelocity;
	vecTestVelocity = pev->velocity;
	vecTestVelocity.z *= 0.7f;

	if (!m_fRegisteredSound && vecTestVelocity.Length() <= 60.0f)
	{
		// grenade is moving really slow. It's probably very close to where it will ultimately stop moving.
		// go ahead and emit the danger sound.

		// register a radius louder than the explosion, so we make sure everyone gets out of the way
		CSoundEnt::InsertSound(bits_SOUND_DANGER, pev->origin, pev->dmg / 0.4f, 0.3s);
		m_fRegisteredSound = TRUE;
	}

	if (pev->flags & FL_ONGROUND)
	{
		// add a bit of static friction
		pev->velocity = pev->velocity * 0.8f;
		pev->sequence = RANDOM_LONG(1, 1);	// TODO: what?
	}
	else
	{
		if (m_iBounceCount < 5)
		{
			// play bounce sound
			BounceSound();
		}

		if (m_iBounceCount >= 10)
		{
			pev->groundentity = ENT(0);
			pev->flags |= FL_ONGROUND;
			pev->velocity = g_vecZero;
		}

		++m_iBounceCount;
	}

	pev->framerate = pev->velocity.Length() / 200.0f;

	if (pev->framerate > 1)
	{
		pev->framerate = 1.0f;
	}
	else if (pev->framerate < 0.5f)
	{
		pev->framerate = 0.0f;
	}

	Vector vecSpot;// trace starts here!
	TraceResult tr;

	vecSpot = pev->origin + Vector(0, 0, 8);
	UTIL_TraceLine(vecSpot, vecSpot + Vector(0, 0, -40), ignore_monsters, ENT(pev), &tr);
	if (tr.flFraction < 1)
	{
		CBaseEntity* pHit = CBaseEntity::Instance(tr.pHit);
		if (pHit->IsPlayer())
		{
			return;
		}
		else
		{
			// only detonate on surfaces less steep than this
			const float kMinCos = cosf(30.0f * M_PI /180.0f);
			if (tr.vecPlaneNormal.z >= kMinCos)
			{
				CGrenade::FB_Detonate();
			}
		}
	}

}


void CGrenade::FB_TumbleThink()
{
	if (!IsInWorld())
	{
		UTIL_Remove(this);
		return;
	}

	StudioFrameAdvance();
	pev->nextthink = gpGlobals->time + 0.1s;

	if (pev->dmgtime - 1s < gpGlobals->time)
	{
		CSoundEnt::InsertSound(bits_SOUND_DANGER, pev->origin + pev->velocity * (pev->dmgtime - gpGlobals->time).count(), 400, 0.1s);
	}

	if (pev->dmgtime <= gpGlobals->time)
	{
		CGrenade::FB_Detonate();
	}

	if (pev->waterlevel != 0)
	{
		pev->velocity = pev->velocity * 0.5f;
		pev->framerate = 0.2f;
	}
}

void CGrenade::FB_Detonate()
{
	TraceResult tr;
	Vector vecSpot;// trace starts here!

	vecSpot = pev->origin + Vector(0, 0, 8);
	UTIL_TraceLine(vecSpot, vecSpot + Vector(0, 0, -40), ignore_monsters, ENT(pev), &tr);
	FB_Explode(&tr, DMG_EXPLOSION);
}

void CGrenade::FB_Explode(TraceResult* pTrace, int bitsDamageType)
{
	pev->model = iStringNull; // invisible
	pev->solid = SOLID_NOT; // intangible
	pev->takedamage = DAMAGE_NO;

	switch (RANDOM_LONG(0, 2))
	{
	case 0:	EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/molotov_hit1.wav", 0.25, ATTN_NORM); break;
	case 1:	EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/molotov_hit2.wav", 0.25, ATTN_NORM); break;
	case 2:	EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/molotov_hit3.wav", 0.25, ATTN_NORM); break;
	}

	entvars_t* pevOwner;
	if (pev->owner)
		pevOwner = VARS(pev->owner);
	else
		pevOwner = NULL;

	if (TheBots != NULL)
	{
		TheBots->OnEvent(EVENT_HE_GRENADE_EXPLODED, CBaseEntity::Instance(pev->owner));
	}

	Vector origin;
	origin = pev->origin;
	origin[2] += 25.0;

	Vector forward = { 1, 0, 0 };
	Vector right = { 0, 1, 0 };
	Vector up = { 0, 0, 1 };

	int num = 3;
	float range = 100.0;
	float f = range / num;
	for (int i = -num; i <= num; i++)	//i= -5 10�ݻ���
	{
		for (int j = -num; j <= num; j++) //��10��
		{
			Vector origin2 = origin;
			origin2 = origin2 + forward * i * f + right * j * f;

			if ((origin2 - origin).Length2D() > range)
				continue;

			//origin2[2] += ((i == 0 && j == 0) ? 45.0 : 5.0);

			origin2[0] += RANDOM_FLOAT(-7.0, 7.0);
			origin2[1] += RANDOM_FLOAT(-7.0, 7.0);
			origin2[2] += 5.0;

			Create("molotov_explosion", origin2, pTrace->vecPlaneNormal, pev->owner);
		}
	}

	pev->effects = EF_NODRAW;
	pev->flags |= FL_KILLME;

	
}

void CGrenade::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	if (!m_bIsC4)
		return;

	CBasePlayer *player = GetClassPtr<CBasePlayer>(pActivator->pev);

	// For CTs to defuse the c4
	if (player->m_iTeam != CT)
	{
		return;
	}

	if (m_bStartDefuse)
	{
		m_fNextDefuse = gpGlobals->time + 0.5s;
		return;
	}
	else if ((player->pev->flags & FL_ONGROUND) != FL_ONGROUND) // Defuse should start only on ground
	{
		ClientPrint(player->pev, HUD_PRINTCENTER, "#C4_Defuse_Must_Be_On_Ground");
		return;
	}

	// freeze the player in place while defusing
	SET_CLIENT_MAXSPEED(player->edict(), 1);

	if (TheBots != NULL)
	{
		TheBots->OnEvent(EVENT_BOMB_DEFUSING, pActivator);
	}

	if (g_pGameRules->IsCareer())
	{
		if (TheCareerTasks != NULL)
		{
			TheCareerTasks->HandleEvent(EVENT_BOMB_DEFUSING);
		}
	}

	if (player->m_bHasDefuser)
	{
		UTIL_LogPrintfDetail("\"%s<%i><%s><CT>\" triggered \"Begin_Bomb_Defuse_With_Kit\"\n",
			STRING(player->pev->netname),
			GETPLAYERUSERID(player->edict()),
			GETPLAYERAUTHID(player->edict()));

		// TODO show messages on clients on event
		ClientPrint(player->pev, HUD_PRINTCENTER, "#Defusing_Bomb_With_Defuse_Kit");
		EMIT_SOUND(ENT(player->pev), CHAN_ITEM, "weapons/c4_disarm.wav", VOL_NORM, ATTN_NORM);

		player->m_bIsDefusing = true;
		m_pBombDefuser = player;
		m_bStartDefuse = true;
		m_flDefuseCountDown = gpGlobals->time + 5.0s;
		m_fNextDefuse = gpGlobals->time + 0.5s;

		// start the progress bar
		player->SetProgressBarTime(5);
	}
	else
	{
		UTIL_LogPrintfDetail("\"%s<%i><%s><CT>\" triggered \"Begin_Bomb_Defuse_Without_Kit\"\n",
			STRING(player->pev->netname),
			GETPLAYERUSERID(player->edict()),
			GETPLAYERAUTHID(player->edict()));

		// TODO show messages on clients on event
		ClientPrint(player->pev, HUD_PRINTCENTER, "#Defusing_Bomb_Without_Defuse_Kit");
		EMIT_SOUND(ENT(player->pev), CHAN_ITEM, "weapons/c4_disarm.wav", VOL_NORM, ATTN_NORM);

		player->m_bIsDefusing = true;
		m_pBombDefuser = player;
		m_bStartDefuse = true;
		m_flDefuseCountDown = gpGlobals->time + 10.0s;
		m_fNextDefuse = gpGlobals->time + 0.5s;

		// start the progress bar
		player->SetProgressBarTime(10);
	}
}

CGrenade* CGrenade::ShootSatchelCharge(entvars_t* pevOwner, Vector vecStart, Vector vecAngles)
{
	CGrenade* pGrenade = CreateClassPtr<CGrenade>();
	pGrenade->pev->movetype = MOVETYPE_TOSS;

	if (pGrenade->pev->classname)
		RemoveEntityHashValue(pGrenade->pev, STRING(pGrenade->pev->classname), CLASSNAME);

	MAKE_STRING_CLASS("grenade", pGrenade->pev);
	AddEntityHashValue(pGrenade->pev, STRING(pGrenade->pev->classname), CLASSNAME);

	pGrenade->pev->solid = SOLID_BBOX;

	// Change this to satchel charge model
	SET_MODEL(ENT(pGrenade->pev), "models/w_c4.mdl");

	UTIL_SetSize(pGrenade->pev, Vector(-3, -6, 0), Vector(3, 6, 8));

	pGrenade->pev->dmg = 100.0f;
	UTIL_SetOrigin(pGrenade->pev, vecStart);
	pGrenade->pev->velocity = g_vecZero;
	pGrenade->pev->angles = vecAngles;
	pGrenade->pev->owner = ENT(pevOwner);

	// Detonate in "time" seconds
	pGrenade->SetThink(&CGrenade::C4Think);
	pGrenade->SetTouch(&CGrenade::C4Touch);
	pGrenade->pev->spawnflags = SF_DETONATE;

	pGrenade->pev->nextthink = gpGlobals->time + 0.1s;
	pGrenade->m_flC4Blow = gpGlobals->time + g_pGameRules->m_iC4Timer;
	pGrenade->m_flNextFreqInterval = std::chrono::seconds(g_pGameRules->m_iC4Timer / 4);
	pGrenade->m_flNextFreq = gpGlobals->time;

	pGrenade->m_iCurWave = 0;
	pGrenade->m_fAttenu = 0;
	pGrenade->m_sBeepName = NULL;
	pGrenade->m_flNextBeep = gpGlobals->time + 0.5s;
	pGrenade->m_bIsC4 = true;
	pGrenade->m_fNextDefuse = {};
	pGrenade->m_bStartDefuse = false;
	pGrenade->m_flNextBlink = gpGlobals->time + 2.0s;

	pGrenade->pev->friction = 0.9f;
	pGrenade->m_bJustBlew = false;

	CBasePlayer* owner = dynamic_cast<CBasePlayer*>(CBasePlayer::Instance(pevOwner));

	if (owner != NULL && owner->IsPlayer())
	{
		pGrenade->m_pentCurBombTarget = owner->m_pentCurBombTarget;
	}
	else
		pGrenade->m_pentCurBombTarget = NULL;

	return pGrenade;
}

CGrenade* CGrenade::ShootSmokeGrenade(entvars_t* pevOwner, Vector vecStart, Vector vecVelocity, duration_t time, unsigned short usEvent)
{
	CGrenade* pGrenade = CreateClassPtr<CGrenade>();
	pGrenade->Spawn();

	UTIL_SetOrigin(pGrenade->pev, vecStart);
	pGrenade->pev->velocity = vecVelocity;
	pGrenade->pev->angles = pevOwner->angles;
	pGrenade->pev->owner = ENT(pevOwner);
	pGrenade->m_usEvent = usEvent;
	pGrenade->m_bLightSmoke = false;
	pGrenade->m_bDetonated = false;
	pGrenade->SetTouch(&CGrenade::BounceTouch);
	pGrenade->pev->dmgtime = gpGlobals->time + time;
	pGrenade->SetThink(&CGrenade::SG_TumbleThink);
	pGrenade->pev->nextthink = gpGlobals->time + 0.1s;

	if (time < 0.1s)
	{
		pGrenade->pev->nextthink = gpGlobals->time;
		pGrenade->pev->velocity = Vector(0, 0, 0);
	}

	pGrenade->pev->sequence = RANDOM_LONG(3, 6);
	pGrenade->pev->framerate = 1.0f;
	pGrenade->m_bJustBlew = true;
	pGrenade->pev->gravity = 0.5f;
	pGrenade->pev->friction = 0.8f;
	pGrenade->m_SGSmoke = 0;
	pGrenade->m_bStaticFramerate = false;
	SET_MODEL(ENT(pGrenade->pev), "models/w_smokegrenade.mdl");
	pGrenade->pev->dmg = 35.0f;

	return pGrenade;
}

void AnnounceFlashInterval(float interval, float offset)
{
	if (!g_bIsCzeroGame)
		return;

	MESSAGE_BEGIN(MSG_ALL, gmsgScenarioIcon);
	WRITE_BYTE(1);
	WRITE_STRING("bombticking");
	WRITE_BYTE(255);
	WRITE_SHORT((int)interval);	// interval
	WRITE_SHORT((int)offset);
	MESSAGE_END();
}

void CGrenade::C4Think()
{
	if (!IsInWorld())
	{
		UTIL_Remove(this);
		return;
	}

	pev->nextthink = gpGlobals->time + 0.12s;

	if (gpGlobals->time >= m_flNextFreq)
	{
		m_flNextFreq = gpGlobals->time + m_flNextFreqInterval;
		m_flNextFreqInterval *= 0.9f;

		switch (m_iCurWave)
		{
		case 0:
			m_sBeepName = "weapons/c4_beep1.wav";
			m_fAttenu = 1.5f;

			AnnounceFlashInterval(139);
			break;
		case 1:
			m_sBeepName = "weapons/c4_beep2.wav";
			m_fAttenu = 1.0f;

			AnnounceFlashInterval(69, 10);
			break;
		case 2:
			m_sBeepName = "weapons/c4_beep3.wav";
			m_fAttenu = 0.8f;

			AnnounceFlashInterval(40);
			break;
		case 3:
			m_sBeepName = "weapons/c4_beep4.wav";
			m_fAttenu = 0.5f;

			AnnounceFlashInterval(30);
			break;
		case 4:
			m_sBeepName = "weapons/c4_beep5.wav";
			m_fAttenu = 0.2f;

			AnnounceFlashInterval(20);
			break;
		}

		++m_iCurWave;
	}

	if (gpGlobals->time >= m_flNextBeep)
	{
		m_flNextBeep = gpGlobals->time + 1.4s;
		EMIT_SOUND(ENT(pev), CHAN_VOICE, m_sBeepName, VOL_NORM, m_fAttenu);

		// let the bots hear the bomb beeping
		// BOTPORT: Emit beep events at same time as client effects
		if (TheBots != NULL)
		{
			TheBots->OnEvent(EVENT_BOMB_BEEP, this);
		}
	}

	if (gpGlobals->time >= m_flNextBlink)
	{
		m_flNextBlink = gpGlobals->time + 2.0s;

		MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
		WRITE_BYTE(TE_GLOWSPRITE);
		WRITE_COORD(pev->origin.x);
		WRITE_COORD(pev->origin.y);
		WRITE_COORD(pev->origin.z + 5.0f);
		WRITE_SHORT(g_sModelIndexC4Glow);
		WRITE_BYTE(1);
		WRITE_BYTE(3);
		WRITE_BYTE(255);
		MESSAGE_END();
	}

	// If the timer has expired ! blow this bomb up!
	if (gpGlobals->time >= m_flC4Blow)
	{
		if (TheBots != NULL)
		{
			TheBots->OnEvent(EVENT_BOMB_EXPLODED);
		}

		MESSAGE_BEGIN(MSG_ALL, gmsgScenarioIcon);
		WRITE_BYTE(0);
		MESSAGE_END();

		if (m_pentCurBombTarget)
		{
			CBaseEntity* pBombTarget = CBaseEntity::Instance(m_pentCurBombTarget);

			if (pBombTarget)
			{
				pBombTarget->Use(CBaseEntity::Instance(pev->owner), this, USE_TOGGLE, 0);
			}
		}

		CBasePlayer* pBombOwner = dynamic_cast<CBasePlayer*>(CBaseEntity::Instance(pev->owner));
		if (pBombOwner != NULL)
		{
			pBombOwner->pev->frags += 3.0f;
		}

		MESSAGE_BEGIN(MSG_ALL, gmsgBombPickup);
		MESSAGE_END();

		g_pGameRules->m_bBombDropped = false;

		if (pev->waterlevel != 0)
			UTIL_Remove(this);
		else
			SetThink(&CGrenade::Detonate2);
	}

	// if the defusing process has started
	if (m_bStartDefuse && m_pBombDefuser != nullptr)
	{
		CBasePlayer* pPlayer = static_cast<CBasePlayer*>(m_pBombDefuser);

		// if the defusing process has not ended yet
		if (gpGlobals->time < m_flDefuseCountDown)
		{
			int iOnGround = ((m_pBombDefuser->pev->flags & FL_ONGROUND) == FL_ONGROUND);

			// if the bomb defuser has stopped defusing the bomb
			if (gpGlobals->time > m_fNextDefuse || !iOnGround)
			{
				if (!iOnGround)
				{
					ClientPrint(m_pBombDefuser->pev, HUD_PRINTCENTER, "#C4_Defuse_Must_Be_On_Ground");
				}

				// release the player from being frozen
				pPlayer->ResetMaxSpeed();
				pPlayer->m_bIsDefusing = false;

				// cancel the progress bar
				pPlayer->SetProgressBarTime(0);
				m_pBombDefuser = NULL;
				m_bStartDefuse = false;
				m_flDefuseCountDown = {};

				// tell the bots someone has aborted defusing
				if (TheBots != NULL)
				{
					TheBots->OnEvent(EVENT_BOMB_DEFUSE_ABORTED);
				}
			}
		}
		// if the defuse process has ended, kill the c4
		else if (m_pBombDefuser->pev->deadflag == DEAD_NO)
		{
			Broadcast("BOMBDEF");

			if (TheBots != NULL)
			{
				TheBots->OnEvent(EVENT_BOMB_DEFUSED, (CBaseEntity*)m_pBombDefuser);
			}

			MESSAGE_BEGIN(MSG_SPEC, SVC_DIRECTOR);
			WRITE_BYTE(9);
			WRITE_BYTE(DRC_CMD_EVENT);
			WRITE_SHORT(ENTINDEX(m_pBombDefuser->edict()));
			WRITE_SHORT(0);
			WRITE_LONG(15 | DRC_FLAG_FINAL | DRC_FLAG_FACEPLAYER | DRC_FLAG_DRAMATIC);
			MESSAGE_END();

			UTIL_LogPrintfDetail("\"%s<%i><%s><CT>\" triggered \"Defused_The_Bomb\"\n",
				STRING(m_pBombDefuser->pev->netname),
				GETPLAYERUSERID(m_pBombDefuser->edict()),
				GETPLAYERAUTHID(m_pBombDefuser->edict()));

			UTIL_EmitAmbientSound(ENT(pev), pev->origin, "weapons/c4_beep5.wav", 0, ATTN_NONE, SND_STOP, 0);
			EMIT_SOUND(ENT(m_pBombDefuser->pev), CHAN_WEAPON, "weapons/c4_disarmed.wav", VOL_NORM, ATTN_NORM);
			UTIL_Remove(this);

			m_bJustBlew = true;

			// release the player from being frozen
			pPlayer->ResetMaxSpeed();
			pPlayer->m_bIsDefusing = false;

			MESSAGE_BEGIN(MSG_ALL, gmsgScenarioIcon);
				WRITE_BYTE(0);
			MESSAGE_END();

			if (g_pGameRules->IsCareer() && !pPlayer->IsBot())
			{
				if (TheCareerTasks != NULL)
				{
					TheCareerTasks->HandleEvent(EVENT_BOMB_DEFUSED, pPlayer);
				}
			}

			g_pGameRules->m_bBombDefused = true;
			g_pGameRules->CheckWinConditions();

			// give the defuser credit for defusing the bomb
			m_pBombDefuser->pev->frags += 3.0f;

			MESSAGE_BEGIN(MSG_ALL, gmsgBombPickup);
			MESSAGE_END();

			g_pGameRules->m_bBombDropped = FALSE;
			m_pBombDefuser = NULL;
			m_bStartDefuse = false;
		}
		else
		{
			// if it gets here then the previouse defuser has taken off or been killed
			// release the player from being frozen
			pPlayer->ResetMaxSpeed();
			pPlayer->m_bIsDefusing = false;

			m_bStartDefuse = false;
			m_pBombDefuser = NULL;

			// tell the bots someone has aborted defusing
			if (TheBots != NULL)
			{
				TheBots->OnEvent(EVENT_BOMB_DEFUSE_ABORTED);
			}
		}
	}
}


void CGrenade::ZombieBombKnockback(Vector vecSrc, entvars_t* pevInflictor, entvars_t* pevAttacker, float iType )
{
	if (!pevAttacker)
		pevAttacker = pevInflictor;

	CBasePlayer* player = GetClassPtr<CBasePlayer>(pevAttacker);
	if (player)
	{
		player->m_pModStrategy->ZombiBombExploded(pev->fuser1, pev->fuser2, pev->health);
	}

	KickRate(pev, pevAttacker, pev->fuser1, pev->fuser2, pev->health, DMGFLAG_DECAY, 0.0);

#if 0
	CBaseEntity* pEntity = NULL;
	TraceResult tr;
	float flAdjustedDamage, falloff, flRadius, flPower;
	Vector vecSpot;
	flRadius = 250.0;
	flPower = 250.0;
	
	if (iType == 999)
	{
		flPower = 450;
		flRadius = 350.0;
	}
	
	float flMul;
	int bInWater = (UTIL_PointContents(vecSrc) == CONTENTS_WATER);
	Vector vecVelocityAdd;
	float flMaxKickRate;
	flAdjustedDamage = pev->health;

	vecSrc.z += 1;

	if (!pevAttacker)
		pevAttacker = pevInflictor;

	while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, flRadius)) != NULL) 	
	{
		if (!pEntity->IsPlayer())
			continue;
		if (!pEntity->IsAlive())
			continue;

		Vector vecSub = pEntity->pev->origin - vecSrc;
		if (vecSub.Length() > flRadius)
			continue;
		
		falloff = (flRadius - vecSub.Length()) / flRadius;
		vecSub *= 1 / vecSub.Length();

		if (falloff < 0.0)
			falloff = 0.0;

		flAdjustedDamage *= falloff;

		CBasePlayer* pPlayer = (CBasePlayer*)pEntity;
		if (pPlayer && !pPlayer->m_bIsZombie)
		{
			if (pPlayer->pev->takedamage != DAMAGE_NO && flAdjustedDamage > 0)
			{
				pEntity->TakeDamage(pevInflictor, pevAttacker, flAdjustedDamage, DMG_EXPLOSION);
			}
			pPlayer->pev->punchangle.x = (RANDOM_LONG(0, 1) ? 1 : -1) * 20.0f * falloff;
			pPlayer->pev->punchangle.z = (RANDOM_LONG(0, 1) ? 1 : -1) * 35.0f * falloff;
		}

		flMaxKickRate = RANDOM_LONG(0, 10) / 10.0 + 1.5 * flPower;
		if (flPower * 1.2 > flMaxKickRate * falloff)
		{
			falloff = 1.0;
			flMaxKickRate = flPower * 1.2;
		}

		vecVelocityAdd = vecSub * flMaxKickRate * falloff;
		if (pEntity->pev->flags & FL_ONGROUND)
		{
			if (pEntity->pev->flags & FL_DUCKING)
				vecVelocityAdd *= 0.8;
		}
		else
			vecVelocityAdd *= 0.9;

		pEntity->pev->velocity += vecVelocityAdd;
		pEntity->pev->velocity.z *= falloff * 0.75;

		MESSAGE_BEGIN(MSG_ONE_UNRELIABLE, gmsgShake, NULL, pEntity->pev);
		WRITE_SHORT((1 << 12));
		WRITE_SHORT((1 << 12) * 2);
		WRITE_SHORT((1 << 12) * 6);
		MESSAGE_END();
	}
#endif
}

void CGrenade::C4Touch(CBaseEntity *pOther)
{
	;
}

NOXREF void CGrenade::UseSatchelCharges(entvars_t *pevOwner, SATCHELCODE code)
{
	edict_t *pentFind;
	edict_t *pentOwner;

	if (!pevOwner)
		return;

	CBaseEntity *pOwner = CBaseEntity::Instance(pevOwner);

	pentOwner = pOwner->edict();

	pentFind = FIND_ENTITY_BY_CLASSNAME(NULL, "grenade");
	while (!FNullEnt(pentFind))
	{
		CBaseEntity *pEnt = Instance(pentFind);

		if (pEnt != NULL)
		{
			if ((pEnt->pev->spawnflags & SF_DETONATE) && pEnt->pev->owner == pentOwner)
			{
				if (code == SATCHEL_DETONATE)
					pEnt->Use(pOwner, pOwner, USE_ON, 0);
				else
				{
					// SATCHEL_RELEASE
					pEnt->pev->owner = NULL;
				}
			}
		}
		pentFind = FIND_ENTITY_BY_CLASSNAME(pentFind, "grenade");
	}
}

void CGrenade::CalculateFramerate(void)
{
	if (m_bStaticFramerate)
	{
		float flSpeed = pev->velocity.Length();

		if (flSpeed > 100)
			flSpeed = 100;

		pev->framerate = flSpeed / 25.0;
	}
}

void CGrenade::ExplodeTouchHolyBomb(void)
{
	pev->model = iStringNull;
	pev->solid = SOLID_NOT;
	pev->takedamage = DAMAGE_NO;

	MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
	WRITE_BYTE(TE_EXPLOSION);
	WRITE_COORD(pev->origin.x);
	WRITE_COORD(pev->origin.y);
	WRITE_COORD(pev->origin.z + 20.0);
	WRITE_SHORT(g_sModelIndexHolyWater);
	WRITE_BYTE(30);
	WRITE_BYTE(30);
	WRITE_BYTE(TE_EXPLFLAG_NODLIGHTS | TE_EXPLFLAG_NOSOUND | TE_EXPLFLAG_NOPARTICLES);
	MESSAGE_END();

	CSoundEnt::InsertSound(bits_SOUND_COMBAT, pev->origin, NORMAL_EXPLOSION_VOLUME, 3.0s);

	entvars_t* pevOwner = VARS(pev->owner);
	pev->owner = NULL;

	EMIT_SOUND_DYN(edict(), CHAN_WEAPON, "zombi/Zombi_Bomb_Exp.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

	HolyBombExplode(pev->origin, pev, pevOwner, 1100, 0.05, 325);

	switch (RANDOM_LONG(0, 1))
	{
	case 0: EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/debris1.wav", 0.55, ATTN_NORM); break;
	case 1: EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/debris2.wav", 0.55, ATTN_NORM); break;
	}

	pev->effects |= EF_NODRAW;
	UTIL_Remove(this);
}

void CGrenade::HolyBombThink(void)
{
	if (!IsInWorld())
	{
		UTIL_Remove(this);

		return;
	}

	CalculateFramerate();
	StudioFrameAdvance();

	pev->nextthink = gpGlobals->time + 0.1s;

	if (gpGlobals->time + 1.0s > pev->dmgtime)
	{
		CSoundEnt::InsertSound(bits_SOUND_DANGER, pev->origin + pev->velocity * ((pev->dmgtime - gpGlobals->time) / 1s), 400, 0.1s);
	}

	if (pev->waterlevel != 0)
	{
		pev->velocity = pev->velocity * 0.5;
		pev->framerate = 0.2;
	}
}

void CGrenade::HolyBombTouch(CBaseEntity* pOther)
{
	if (ENT(pOther->pev) != pev->owner)
		ExplodeTouchHolyBomb();
}

void CGrenade::HolyBombExplode(Vector vecSrc, entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, float flBurnAmount, float flRadius)
{
	CBaseEntity* pEntity = NULL;
	TraceResult tr;
	Vector vecEnd;

	float falloff;

	if (flRadius)
		falloff = flDamage / flRadius;
	else
		falloff = 1.0f;

	bool bInWater = UTIL_PointContents(vecSrc) == CONTENTS_WATER;

	vecSrc.z += 1.0;

	entvars_t* pevAttack = pevAttacker ? pevAttacker : pevInflictor;

	while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, flRadius)) != NULL)
	{
		if (pEntity->pev->takedamage == DAMAGE_NO)
			continue;

		if (bInWater && pEntity->pev->waterlevel == 0)
			continue;

		if (!bInWater && pEntity->pev->waterlevel == 3)
			continue;

		if (pEntity->Classify() == CLASS_PLAYER)
		{
			if (!((CBasePlayer*)pEntity)->m_bIsZombie)
				continue;

			UTIL_TraceLine(vecSrc, pEntity->BodyTarget(vecSrc), dont_ignore_monsters, ENT(pevInflictor), &tr);

			if (tr.flFraction < 1.0f && tr.pHit != pEntity->edict())
				continue;

			if (tr.fStartSolid)
			{
				tr.vecEndPos = vecSrc;
				tr.flFraction = 0;
			}

			float flCurrentDamage = flDamage - (vecSrc - pEntity->pev->origin).Length() * falloff;

			pEntity->TakeDamage(pevInflictor, pevAttack, flCurrentDamage, DMG_EXPLOSION);

			CBasePlayer* pPlayer = (CBasePlayer*)pEntity;

			pPlayer->m_pBuffAttacker = pevAttacker;
			pPlayer->m_iHealthDecreaseCount = 8;
			pPlayer->m_flHealthDecreaseAmount = pPlayer->pev->max_health * flBurnAmount;
			pPlayer->m_flHealthDecreaseInterval = 1.0s;

			for (int id = 1; id <= gpGlobals->maxClients; id++)
			{
				pPlayer = (CBasePlayer*)UTIL_PlayerByIndex(id);

				if (!pPlayer)
					continue;

				if (FNullEnt(pPlayer->pev))
					continue;

				MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, pPlayer->pev);
				WRITE_BYTE(4);
				WRITE_SHORT(pEntity->entindex());
				WRITE_BYTE(8);
				MESSAGE_END();
			}
		}
		else if (pEntity->Classify() != CLASS_PLAYER_ALLY)
			continue;

		// ..
	}
}

void CGrenade::DetonateFireBomb()
{
	TraceResult tr;
	Vector vecStart = pev->origin - Vector(0, 0, 32);

	UTIL_TraceLine(pev->origin + Vector(0, 0, 8), pev->origin - Vector(0, 0, 32), ignore_monsters, ENT(pev), &tr);
	Explode3(&tr, DMG_EXPLOSION, EXPTYPE_FIREBOMB, 0.0);
}

void CGrenade::FireBombExplode(Vector vecSrc, entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, float flBurnAmount, float flRadius)
{
	CBaseEntity* pEntity = NULL;
	TraceResult tr;
	Vector vecEnd;

	float falloff;

	if (flRadius)
		falloff = flDamage / flRadius;
	else
		falloff = 1.0f;

	bool bInWater = UTIL_PointContents(vecSrc) == CONTENTS_WATER;

	vecSrc.z += 1.0;

	entvars_t* pevAttack = pevAttacker ? pevAttacker : pevInflictor;

	while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, flRadius)) != NULL)
	{
		if (pEntity->pev->takedamage == DAMAGE_NO)
			continue;

		if (bInWater && pEntity->pev->waterlevel == 0)
			continue;

		if (!bInWater && pEntity->pev->waterlevel == 3)
			continue;

		if (pEntity->Classify() == CLASS_PLAYER)
		{
			UTIL_TraceLine(vecSrc, pEntity->BodyTarget(vecSrc), dont_ignore_monsters, ENT(pevInflictor), &tr);

			if (tr.flFraction < 1.0f && tr.pHit != pEntity->edict())
				continue;

			if (tr.fStartSolid)
			{
				tr.vecEndPos = vecSrc;
				tr.flFraction = 0;
			}

			float flCurrentDamage = flDamage - (vecSrc - pEntity->pev->origin).Length() * falloff;

			pEntity->TakeDamage(pevInflictor, pevAttack, flCurrentDamage, DMG_EXPLOSION);

			CBasePlayer* pPlayer = (CBasePlayer*)pEntity;

			pPlayer->m_pBuffAttacker = pevAttacker;
			pPlayer->m_iHealthDecreaseCount = 5;
			pPlayer->m_flHealthDecreaseAmount = pPlayer->pev->max_health * flBurnAmount;
			pPlayer->m_flHealthDecreaseInterval = 1.0s;

			for (int id = 1; id <= gpGlobals->maxClients; id++)
			{
				pPlayer = (CBasePlayer*)UTIL_PlayerByIndex(id);

				if (!pPlayer)
					continue;

				if (FNullEnt(pPlayer->pev))
					continue;

				MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, pPlayer->pev);
				WRITE_BYTE(4);
				WRITE_SHORT(pEntity->entindex());
				WRITE_BYTE(18);
				MESSAGE_END();
			}
		}
		else if (pEntity->Classify() != CLASS_PLAYER_ALLY)
			continue;

		// ..
	}
}

void CGrenade::SFGrenadeTouch(CBaseEntity * pOther)
{
	SetTouch(NULL);

	if (pOther->pev->takedamage == DAMAGE_NO && /*pOther->Classify() == CLASS_BREAKABLE && */ pOther->pev->rendermode != kRenderNormal)
	{
		SetThink(NULL);

		pev->dmg = 100;
		SFGrenadeExplodeThink();

		return;
	}

	if (pOther->IsBSPModel())
	{
		pev->movetype = MOVETYPE_NONE;

		return;
	}

	if (!pOther->IsAlive())
		return;

	if (pOther->pev->takedamage == DAMAGE_NO)
		return;

	pev->effects |= EF_NODRAW;
	pev->velocity = g_vecZero;
	pev->solid = SOLID_NOT;

	m_pStickingObject = pOther;

	/*MESSAGE_BEGIN(MSG_ALL, SVC_TEMPENTITY);
	WRITE_BYTE(TE_FOLLOWINGGRENADE);
	WRITE_SHORT(pOther->entindex());
	WRITE_SHORT(pev->modelindex);
	WRITE_COORD(pev->origin.x - pOther->pev->origin.x);
	WRITE_COORD(pev->origin.y - pOther->pev->origin.y);
	WRITE_COORD(pev->origin.z - pOther->pev->origin.z);
	WRITE_COORD(0.0f);
	WRITE_COORD(1.0f);
	WRITE_COORD(0.0f);
	WRITE_BYTE((int)(pev->dmgtime - gpGlobals->time + 1.0f));
	MESSAGE_END();*/
}


void CGrenade::SFGrenadeExplodeThink(void)
{
	TraceResult tr;

	if (m_pStickingObject.Get() != NULL)
	{
		CBaseEntity* pEntity = m_pStickingObject;

		if (pEntity)
		{
			pev->origin = pEntity->pev->origin;
		}
	}

	UTIL_TraceLine(pev->origin + Vector(0, 0, 8), pev->origin - Vector(0, 0, 32), ignore_monsters, ENT(pev), &tr);
	Explode3(&tr, DMG_EXPLOSION, EXPTYPE_SFGRENADE, 0);
}

IMPLEMENT_SAVERESTORE(CGrenade, CBaseMonster);

}
