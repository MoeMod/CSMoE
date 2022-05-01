#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "client.h"
#include "cbase/cbase_hash.h"

#include "zombiepile.h"
#include "gamemode/mod_zb2.h"

namespace sv {

LINK_ENTITY_TO_CLASS(zombiepile, CZombiePile);

CZombiePile* CZombiePile::Create(CBasePlayer* player)
{
	edict_t* pent = CREATE_NAMED_ENTITY(MAKE_STRING("zombiepile"));

	if (FNullEnt(pent))
	{
		ALERT(at_console, "NULL Ent in Create!\n");
		return NULL;
	}

	Vector vecAngles = Vector(0, player->pev->v_angle.y, 0);
	UTIL_MakeVectors(vecAngles);	
	CZombiePile* pEntity = (CZombiePile*)Instance(pent);
	pEntity->pev->origin = player->GetGunPosition() + gpGlobals->v_forward * 50;
	pEntity->pev->angles = vecAngles;
	pEntity->m_hOwner = player;

	TraceResult tr;
	UTIL_TraceHull(pEntity->pev->origin, pEntity->pev->origin, dont_ignore_monsters, (player->pev->flags & FL_DUCKING) ? head_hull : human_hull, pEntity->edict(), &tr);
	if (tr.fStartSolid || tr.fAllSolid || !tr.fInOpen)
	{
		pEntity->Slowdown();
		pEntity->Explosion();
	}

	DispatchSpawn(pEntity->edict());
	return pEntity;
}

void CZombiePile::Slowdown()
{
	auto flTime = GetSlowdownTime();
	float flRadius = GetRange();
	CBaseEntity* pVictim = NULL;
	while ((pVictim = UTIL_FindEntityInSphere(pVictim, pev->origin, flRadius)) != NULL)
	{
		if (!pVictim->IsPlayer())
			continue;
		if (!pVictim->IsAlive())
			continue;

		CBasePlayer* pPlayer = (CBasePlayer*)pVictim;
		if (pPlayer->m_bIsZombie)
			continue;

		MESSAGE_BEGIN(MSG_ALL, gmsgMPToCL, NULL, pPlayer->pev);
		WRITE_BYTE(4);
		WRITE_SHORT(pPlayer->entindex());
		WRITE_BYTE(10);
		MESSAGE_END();

		MESSAGE_BEGIN(MSG_ONE, gmsgShake, NULL, pPlayer->pev);
		WRITE_SHORT(0xA000);
		WRITE_SHORT(0x1000);
		WRITE_SHORT(0x6400);
		MESSAGE_END();

		auto strategy = dynamic_cast<CPlayerModStrategy_ZB2*>(pPlayer->m_pModStrategy.get());
		if (strategy)
		{
			strategy->SetSlowdownTime(flTime);
		}
	}

	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
	WRITE_BYTE(TE_BEAMDISK);
	WRITE_COORD(pev->origin.x);
	WRITE_COORD(pev->origin.y);
	WRITE_COORD(pev->origin.z - (m_bStomp ? 16 : 32));
	WRITE_COORD(pev->origin.x);
	WRITE_COORD(pev->origin.y);
	WRITE_COORD(pev->origin.z + 500);
	WRITE_SHORT(m_idShockWave);
	WRITE_BYTE(0);
	WRITE_BYTE(0);
	WRITE_BYTE(4);
	WRITE_BYTE(35);
	WRITE_BYTE(0);
	WRITE_BYTE(255);
	WRITE_BYTE(255);
	WRITE_BYTE(230);
	WRITE_BYTE(60);
	WRITE_BYTE(0);
	MESSAGE_END();

}

void CZombiePile::Spawn(void)
{
	Precache();

	if (pev->classname)
	{
		RemoveEntityHashValue(pev, STRING(pev->classname), CLASSNAME);
	}

	MAKE_STRING_CLASS("zombiepile", pev);
	AddEntityHashValue(pev, STRING(pev->classname), CLASSNAME);

	SetTouch(NULL);
	SetThink(&CZombiePile::OnGround);
	SET_MODEL(ENT(pev), "models/zombipile.mdl");

	pev->deadflag = DEAD_NO;
	pev->takedamage = DAMAGE_YES;
	pev->health = 400.0f;
	pev->solid = SOLID_BBOX;
	pev->movetype = MOVETYPE_PUSHSTEP; // MOVETYPE_PUSHSTEP will not ignore by traceline

	if (m_hOwner && m_hOwner->pev->flags & FL_DUCKING)
	{
		pev->body = 2;
		UTIL_SetSize(pev, Vector(-12.0, -12.0, -18.0), Vector(12.0, 12.0, 18.0));
		m_bStomp = true;
	}
	else
	{
		pev->body = 1;
		UTIL_SetSize(pev, Vector(-12.0, -12.0, -36.0), Vector(12.0, 12.0, 36.0));
		m_bStomp = false;
	}

	pev->sequence = 0;
	pev->frame = 0;
	ResetSequenceInfo();
	m_fSequenceLoops = true;
	m_flRemoveTime = gpGlobals->time + GetAliveTime();
	pev->nextthink = gpGlobals->time + 0.2s;
}

void CZombiePile::Precache(void)
{
	PRECACHE_SOUND("zombi/zombi_stamper_iron_maiden_stamping.wav");
	PRECACHE_SOUND("zombi/Zombi_Wood_Broken.wav");
	PRECACHE_SOUND("zombi/zombi_stamper_iron_maiden_explosion.wav");
	PRECACHE_MODEL("models/zombipile.mdl");
	m_idShard = PRECACHE_MODEL("models/gibs_woodplank.mdl");
	m_idShockWave = PRECACHE_MODEL("sprites/ef_shockwave.spr");
}

void EXPORT CZombiePile::OnGround()
{
	if (m_flRemoveTime <= gpGlobals->time)
	{
		SetThink(NULL);
		Explosion();

		return;
	}

	pev->nextthink = gpGlobals->time + 0.01s;
	if (pev->flags & FL_ONGROUND)
	{
		SetThink(&CZombiePile::OnThink);
		Slowdown();
		EMIT_SOUND(ENT(pev), CHAN_BODY, "zombi/zombi_stamper_iron_maiden_stamping.wav", 1.0, ATTN_NORM);
	}
}

void EXPORT CZombiePile::OnThink()
{
	if (m_flRemoveTime <= gpGlobals->time)
	{
		SetThink(NULL);
		Break();

		return;
	}

	StudioFrameAdvance();
	pev->nextthink = gpGlobals->time + 0.2s;
}

int CZombiePile::TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
{
	auto pAttack = dynamic_cast<CBasePlayer*>(CBasePlayer::Instance(pevAttacker));

	if (!pAttack)
		return 0;

	if (pAttack->IsPlayer() && !pAttack->m_bIsZombie)
	{
		flDamage *= 0.1f;
	}
	
	return CBaseAnimating::TakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType);
}


void CZombiePile::Break()
{
	int pitch = 95 + RANDOM_LONG(0, 29);

	if (pitch <= 102)
		pitch = PITCH_NORM;

	EMIT_SOUND_DYN(ENT(pev), CHAN_ITEM, "zombi/Zombi_Wood_Broken.wav", VOL_NORM, ATTN_NORM, 0, pitch);

	Vector vecOrigin = pev->origin + (pev->mins + pev->maxs) * 0.5;

	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, vecOrigin);
	WRITE_BYTE(TE_BREAKMODEL);
	WRITE_COORD(vecOrigin.x);
	WRITE_COORD(vecOrigin.y);
	WRITE_COORD(vecOrigin.z);
	WRITE_COORD(pev->size.x);
	WRITE_COORD(pev->size.y);
	WRITE_COORD(pev->size.z);
	WRITE_COORD(0.0);
	WRITE_COORD(0.0);
	WRITE_COORD(0.0);
	WRITE_BYTE(10);
	WRITE_SHORT(m_idShard);
	WRITE_BYTE(30);
	WRITE_BYTE(25);
	WRITE_BYTE(BREAK_WOOD);
	MESSAGE_END();

	pev->takedamage = DAMAGE_NO;
	pev->effects = EF_NODRAW;
	pev->deadflag = DEAD_DEAD;
	UTIL_Remove(this);
}

void CZombiePile::Killed(entvars_t* pevAttacker, int iGib)
{
	Explosion();
}

void CZombiePile::Explosion()
{
	int pitch = 95 + RANDOM_LONG(0, 29);

	if (pitch <= 102)
		pitch = PITCH_NORM;

	EMIT_SOUND_DYN(ENT(pev), CHAN_ITEM, "zombi/Zombi_Wood_Broken.wav", VOL_NORM, ATTN_NORM, 0, pitch);

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

	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
	WRITE_BYTE(TE_BREAKMODEL);
	WRITE_COORD(pev->origin.x);		// position
	WRITE_COORD(pev->origin.y);
	WRITE_COORD(pev->origin.z);
	WRITE_COORD(pev->size.x);	// size
	WRITE_COORD(pev->size.y);
	WRITE_COORD(pev->size.z);
	WRITE_COORD(2.0f);	// velocity
	WRITE_COORD(2.0f);
	WRITE_COORD(2.0f);
	WRITE_BYTE(10);			// randomization
	WRITE_SHORT(m_idShard);		// model id#
	WRITE_BYTE(30);			// # of shards, let client decide
	WRITE_BYTE(25);			// duration, 2.5 seconds
	WRITE_BYTE(BREAK_WOOD);		// flags
	MESSAGE_END();

	EMIT_SOUND(ENT(pev), CHAN_BODY, "zombi/zombi_stamper_iron_maiden_explosion.wav", 1.0, ATTN_NORM);

	entvars_t* pevAttacker = m_hOwner->pev;
	if (!pevAttacker)
		pevAttacker = pev;

	CBaseEntity* pEntity = NULL;
	float flAdjustedDamage, falloff;
	float flRadius = GetRange();
	float flDamage = GetDamage(), flDist;
	Vector vecVelocityAdd;

	while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, flRadius)) != NULL)
	{
		if (!pEntity->IsPlayer())
			continue;
		if (!pEntity->IsAlive())
			continue;

		vecVelocityAdd = pEntity->pev->origin - pev->origin;
		flDist = vecVelocityAdd.Length();
		vecVelocityAdd *= 1 / (flDist / GetKnockBack());

		CBasePlayer* pPlayer = (CBasePlayer*)pEntity;
		if (pPlayer && !pPlayer->m_bIsZombie && pPlayer->pev->takedamage != DAMAGE_NO)
		{
			falloff = (flRadius - flDist) / flRadius;
			flAdjustedDamage = flDamage * falloff;

			if (flAdjustedDamage > 0.0f)
				pEntity->TakeDamage(pev, pevAttacker, flAdjustedDamage, DMG_EXPLOSION);

			MESSAGE_BEGIN(MSG_ALL, gmsgMPToCL, NULL, pPlayer->pev);
			WRITE_BYTE(10);
			WRITE_SHORT(pPlayer->entindex());
			WRITE_BYTE(0);
			MESSAGE_END();

			auto strategy = dynamic_cast<CPlayerModStrategy_ZB2*>(pPlayer->m_pModStrategy.get());
			if (strategy)
			{
				auto flTime = GetSlowdownTime();
				strategy->SetSlowdownTime(flTime);
			}
		}
		pEntity->pev->velocity += vecVelocityAdd;

		MESSAGE_BEGIN(MSG_ONE_UNRELIABLE, gmsgShake, NULL, pEntity->pev);
		WRITE_SHORT((1 << 12));
		WRITE_SHORT((1 << 12) * 2);
		WRITE_SHORT((1 << 12) * 6);
		MESSAGE_END();
	}

	pev->takedamage = DAMAGE_NO;
	pev->effects = EF_NODRAW;
	pev->deadflag = DEAD_DEAD;
	UTIL_Remove(this);
}

}