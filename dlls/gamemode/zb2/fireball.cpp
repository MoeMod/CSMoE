#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "client.h"
#include "cbase/cbase_hash.h"

#include "fireball.h"
#include "gamemode/mods.h"
#include "player/player_mod_strategy.h"

#include <utility>

namespace sv {

LINK_ENTITY_TO_CLASS(aksha_fireball, CAkshaFireBall);

void CAkshaFireBall::Spawn(void)
{
	m_iSkin = 0;
	Precache();

	pev->rendermode = kRenderTransAdd;
	pev->renderfx = kRenderFxNone;
	pev->renderamt = 180;
	pev->framerate = 10;
	pev->scale = 0.3;
	pev->movetype = MOVETYPE_TOSS;
	pev->solid = SOLID_BBOX;
	pev->gravity = 1.0;
	pev->sequence = 0;
	pev->framerate = 30.0;


	SetTouch(&CAkshaFireBall::OnTouch);
	SetThink(&CAkshaFireBall::IgniteThink);
	
	m_flRemoveTime = gpGlobals->time + GetAliveTime();

	CBaseEntity* pAttacker = CBaseEntity::Instance(pev->owner);
	m_pOwner = nullptr;
	if (pAttacker && pAttacker->IsPlayer())
		m_pOwner = static_cast<CBasePlayer*>(pAttacker);

	if (!m_pOwner)
	{
		Remove();
		return;
	}


	if (m_pOwner->m_iZombieClassSkin[ZOMBIE_CLASS_AKSHA] == SKIN_Y21S1AKSHA)
	{
		SET_MODEL(ENT(pev), "sprites/ef_y21s1aksha_fireball.spr");
		m_iSkin = 1;
	}
	else
	{
		SET_MODEL(ENT(pev), "sprites/ef_aksha_fireball.spr");
		m_iSkin = 0;
	}
		
	m_flMaxFrames = float(MODEL_FRAMES(pev->modelindex) - 1);
	
	UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));

	pev->nextthink = gpGlobals->time + 0.1s;
}



void CAkshaFireBall::Precache(void)
{
	PRECACHE_SOUND("zombi/akshazombi_skill_exp.wav");
	m_iSmokeTrail[0] = PRECACHE_MODEL("sprites/ef_aksha_fireball.spr");
	m_iFireBallDestroy[0] = PRECACHE_MODEL("sprites/ef_aksha_fireballdestroy.spr");
	m_iFireBallExplosion[0] = PRECACHE_MODEL("sprites/ef_aksha_fireballexplosion.spr");

	m_iSmokeTrail[1] = PRECACHE_MODEL("sprites/ef_y21s1aksha_fireball.spr");
	m_iFireBallDestroy[1] = PRECACHE_MODEL("sprites/ef_y21s1aksha_fireballdestroy.spr");
	m_iFireBallExplosion[1] = PRECACHE_MODEL("sprites/ef_y21s1aksha_fireballexplosion.spr");
}

void CAkshaFireBall::IgniteThink()
{
	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
	WRITE_BYTE(TE_BEAMPARTICLE); //defined as 32
	WRITE_SHORT(this->entindex());  // short (entity:attachment to follow)
	WRITE_SHORT(m_iSmokeTrail[m_iSkin]);// short (sprite index)
	WRITE_SHORT(m_iSmokeTrail[m_iSkin]);// short (sprite index)
	WRITE_SHORT(m_iSmokeTrail[m_iSkin]);// short (sprite index)
	WRITE_BYTE(255); //r
	WRITE_BYTE(255); //g
	WRITE_BYTE(255); //b
	WRITE_BYTE(0); //randomcolor
	WRITE_BYTE(30); //scale in 0.01's
	WRITE_BYTE(6); //freq(time of one cycle) in 0.01's
	WRITE_BYTE(3); //fadetime in 0.1s
	WRITE_BYTE(1);  //count
	WRITE_BYTE(0);  //random in org
	WRITE_BYTE(0);  //speed (if>0 do slow gravity)
	MESSAGE_END();

	SetThink(&CAkshaFireBall::FollowThink);
	pev->nextthink = gpGlobals->time + 0.1s;
}

void CAkshaFireBall::FollowThink()
{
	pev->frame += pev->framerate * gpGlobals->frametime / 1s;

	if (pev->frame > m_flMaxFrames)
		pev->frame = fmod(pev->frame, m_flMaxFrames);


	if (m_pOwner)
	{
		if (!m_pOwner->IsAlive())
		{
			MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
			WRITE_BYTE(TE_EXPLOSION);
			WRITE_COORD(pev->origin.x);
			WRITE_COORD(pev->origin.y);
			WRITE_COORD(pev->origin.z + 25);
			WRITE_SHORT(m_iFireBallDestroy[m_iSkin]);
			WRITE_BYTE(15);
			WRITE_BYTE(30);
			WRITE_BYTE(TE_EXPLFLAG_NOPARTICLES | TE_EXPLFLAG_NODLIGHTS | TE_EXPLFLAG_NOSOUND);
			MESSAGE_END();

			Remove();
		}
	}

	if (m_flRemoveTime < gpGlobals->time)
	{
		Explosion();
		RadiusDamage();
		Remove();
	}

	pev->nextthink = gpGlobals->time + 0.01s;

}

void CAkshaFireBall::Remove(void)
{
	SetThink(NULL);
	SetTouch(NULL);

	UTIL_Remove(this);
}

void CAkshaFireBall::Explosion()
{
	EMIT_SOUND(ENT(pev), CHAN_ITEM, "zombi/akshazombi_skill_exp.wav", VOL_NORM, ATTN_NORM);

	MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
	WRITE_BYTE(TE_EXPLOSION);
	WRITE_COORD(pev->origin.x);
	WRITE_COORD(pev->origin.y);
	WRITE_COORD(pev->origin.z + 25);
	WRITE_SHORT(m_iFireBallExplosion[m_iSkin]);
	WRITE_BYTE(15);
	WRITE_BYTE(30);
	WRITE_BYTE(TE_EXPLFLAG_NOPARTICLES | TE_EXPLFLAG_NODLIGHTS | TE_EXPLFLAG_NOSOUND);
	MESSAGE_END();
}

void CAkshaFireBall::RadiusDamage()
{
	float flDamage = GetDamage();
	float flRadius = GetRange();
	float flDist;

	TraceResult tr;
	CBaseEntity* pEntity = NULL;
	Vector vecVelocityAdd;

	Vector vecEnd;
	int bInWater = (UTIL_PointContents(pev->origin) == CONTENTS_WATER);

	while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, flRadius)) != NULL)
	{
		if (pEntity->edict() == pev->owner)
			continue;

		if (pEntity->IsDormant())
			continue;

		if (pEntity->pev->takedamage == DAMAGE_NO)
			continue;

		if (pEntity->Classify() == CLASS_NONE)
			continue;

		if (pEntity->IsPlayer())
		{
			CBasePlayer* target = static_cast<CBasePlayer*>(pEntity);

			if (m_pOwner->m_iTeam == target->m_iTeam)
				continue;
		}

		if ((bInWater && pEntity->pev->waterlevel == 0) || (!bInWater && pEntity->pev->waterlevel == 3))
			continue;

		vecEnd = pEntity->BodyTarget(pev->origin);

		UTIL_TraceLine(pev->origin, vecEnd, ignore_monsters, pev->owner, &tr);

		if (tr.flFraction < 1.0 && tr.pHit != pEntity->edict())
			continue;

		int falloff = flDamage / flRadius;

		float flCurrentDamage = max(0, flDamage - (pev->origin - pEntity->pev->origin).Length() * falloff);

		pEntity->TakeDamage(pev, m_pOwner->pev, flCurrentDamage, DMG_EXPLOSION);

		if (pEntity->IsPlayer())
		{
			CBasePlayer* player = (CBasePlayer*)pEntity;

			vecVelocityAdd = player->pev->origin - pev->origin;
			flDist = vecVelocityAdd.Length();
			vecVelocityAdd *= 1 / (flDist / GetKnockBack());
			player->pev->velocity += vecVelocityAdd;
		}
	}
	
}

void EXPORT CAkshaFireBall::OnTouch(CBaseEntity* pOther)
{
	Explosion();

	if (pev->owner == pOther->edict())
	{
		Remove();
		return;
	}

	float flDamage = GetDamage();
	float flRadius = GetRange();
	float flDist;
	Vector vecVelocityAdd;

	if (pOther->pev->takedamage != DAMAGE_NO && pOther->IsAlive())
	{
		Vector vecDirection = (pOther->pev->origin - pev->origin).Normalize();
		pOther->TakeDamage(pev, m_pOwner->pev, flDamage, DMG_EXPLOSION);

		if (pOther->IsPlayer())
		{
			vecVelocityAdd = pOther->pev->origin - pev->origin;
			flDist = vecVelocityAdd.Length();
			vecVelocityAdd *= 1 / (flDist / GetKnockBack());
			pOther->pev->velocity += vecVelocityAdd;
		}
	}

	RadiusDamage();
	

	Remove();
}

}
