#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "schedule.h"
#include "player.h"
#include "monsters.h"
#include "trains.h"
#include "sentrygun.h"

#include "gamemode/mods.h"

namespace sv
{

LINK_ENTITY_TO_CLASS(ghosthunter_sentrygun, CSentryGun);


CSentryGun* CSentryGun::Create(const Vector& vecOrigin, const Vector& vecAngles, int iTeam)
{
	edict_t* pent = CREATE_NAMED_ENTITY(MAKE_STRING("ghosthunter_sentrygun"));

	if (FNullEnt(pent))
	{
		ALERT(at_console, "NULL Ent in Create!\n");
		return NULL;
	}

	CSentryGun* pSentryGun = (CSentryGun*)Instance(pent);

	if (pSentryGun)
	{
		pSentryGun->pev->origin = vecOrigin;
		pSentryGun->pev->angles = vecAngles;
		pSentryGun->m_iTeam = iTeam;
		pSentryGun->Spawn();
	}

	return pSentryGun;
}

void CSentryGun::Spawn(void)
{
	Precache();

	pev->classname = MAKE_STRING("ghosthunter_sentrygun");

	SET_MODEL(ENT(pev), "models/z4b/sentrygun.mdl");
	UTIL_SetOrigin(pev, pev->origin);
	UTIL_SetSize(pev, { -23, -26, -26 }, { 33, 26, 25 });
	pev->angles = g_vecZero;
	pev->gravity = 1.0;
	pev->health = 2000.0f;
	pev->movetype = MOVETYPE_PUSHSTEP;
	pev->solid = SOLID_SLIDEBOX;
	pev->flags |= FL_MONSTER;
	pev->deadflag = DEAD_NO;
	pev->takedamage = DAMAGE_YES;

	pev->frame = 0;
	pev->sequence = 0;
	ResetSequenceInfo();

	m_flTurnRate = 10.0f;
	m_flVerticalTurnRate = 3.0f;
	m_iRange = 1000;
	m_flFireRate = invalid_time_point;
	m_flFieldOfView = VIEW_FIELD_FULL;

	pev->nextthink = gpGlobals->time + 0.1s;
	SetThink(&CSentryGun::Think);
}

void CSentryGun::Precache(void)
{
	PRECACHE_MODEL("models/z4b/sentrygun.mdl");

}

void CSentryGun::SetLife(duration_t flTime)
{
	m_flRemoveTime = gpGlobals->time + flTime;
}

void CSentryGun::Killed(entvars_t* pevAttacker, int iGib)
{
	if (!IsAlive())
		return;

	iGib = GIB_ALWAYS;

	return CBaseMonster::Killed(pevAttacker, iGib);
}

void CSentryGun::Think()
{
	pev->nextthink = gpGlobals->time + 0.1s;
	StudioFrameAdvance();

	Search();
	if (m_fSequenceFinished)
	{
		SetAnim(0);
	}

	if (m_hEnemy)
	{
		Active();
	}
	else
	{
		Retire();
	}

	if (m_flRemoveTime < gpGlobals->time)
	{
		SetThink(nullptr);
		UTIL_Remove(this);
		return;
	}
}

void CSentryGun::Search()
{
	// If we have a target and we're still healthy
	if (m_hEnemy)
	{
		if (!m_hEnemy->IsAlive())
			m_hEnemy = NULL;// Dead enemy forces a search for new one
	}

	if (m_hEnemy) 
		return;

	Look(m_iRange);
	m_hEnemy = BestVisibleEnemy();
}

void CSentryGun::Active()
{
	Vector vecMid = pev->origin + pev->view_ofs;
	Vector vecMidEnemy = m_hEnemy->BodyTarget(vecMid);
	Vector vecDirToEnemy = vecMidEnemy - vecMid;	// calculate dir and dist to enemy
	float flDistToEnemy = vecDirToEnemy.LengthSquared();

	// Current enmey is not visible.

	if ((flDistToEnemy > m_iRange * m_iRange) || !FBoxVisible(pev, m_hEnemy->pev, vecMidEnemy))
	{
		m_hEnemy = NULL;
		return;
	}

	float flCos1 = DotProduct(vecDirToEnemy.Normalize(), { 0, 0, 1 });

	if (flCos1 > 0.5 || flCos1 < -0.5)
	{
		m_hEnemy = NULL;
		return;
	}

	Vector vec = UTIL_VecToAngles(vecDirToEnemy);

	if (vec.x > 360)
		vec.x -= 360;
	if (vec.x < 0)
		vec.x += 360;

	// [1...360]
	if (vec.y > 360)
		vec.y -= 360;
	if (vec.y < 0)
		vec.y += 360;

	m_vecGoalAngles.y = vec.y;
	m_vecGoalAngles.x = vec.x;

	if (m_vecGoalAngles.x > 180)
		m_vecGoalAngles.x -= 360;


	MoveTurret();
	if (m_flFireRate > gpGlobals->time)
		return;

	m_flFireRate = gpGlobals->time + 0.25s;
	UTIL_MakeAimVectors(m_vecCurAngles);
	Vector vecLOS = vecDirToEnemy.Normalize();
	// Is the Gun looking at the target
	if (DotProduct(vecLOS, gpGlobals->v_forward) <= 0.866) // 30 degree slop
		return;

	// Shoot
	SetAnim(2);
	GetAttachment(0, vecLOS, vec);
	vecDirToEnemy = vecMidEnemy - vecLOS;
	FireBullets(1, vecLOS, vecDirToEnemy, g_vecZero, m_iRange, BULLET_MONSTER_9MM, 1, 10, m_pOwner->pev);

	switch (RANDOM_LONG(0, 2))
	{
	case 0: EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/hks1.wav", 1, ATTN_NORM); break;
	case 1: EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/hks2.wav", 1, ATTN_NORM); break;
	case 2: EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/hks3.wav", 1, ATTN_NORM); break;
	}
	pev->effects = pev->effects | EF_MUZZLEFLASH;
}

void CSentryGun::Retire()
{
}

CBaseEntity* CSentryGun::BestVisibleEnemy()
{
	CBaseEntity* pReturn;
	CBaseEntity* pNextEnt;
	int iNearest;
	int iDist;

	// so first visible entity will become the closest.
	iNearest = 67108864;
	pNextEnt = m_pLink;
	pReturn = NULL;

	while (pNextEnt != NULL)
	{
		if (pNextEnt->IsAlive() && (pNextEnt->Classify() == CLASS_PLAYER_ALLY || pNextEnt->IsPlayer()))
		{
			if (pNextEnt == m_pOwner)
				goto END_OF_SEARCH;

			if (g_pGameRules->PlayerRelationship(m_pOwner, pNextEnt) == GR_TEAMMATE)
				goto END_OF_SEARCH;

			iDist = (pNextEnt->pev->origin - pev->origin).LengthSquared();

			if (iDist <= iNearest)
			{
				iNearest = iDist;
				pReturn = pNextEnt;
			}
		}

	END_OF_SEARCH:
		pNextEnt = pNextEnt->m_pLink;
	}

	return pReturn;
}

bool CSentryGun::MoveTurret()
{
	bool state = false;

	if (m_vecCurAngles.x > 180)
		m_vecCurAngles.x -= 360;

	if (m_vecCurAngles.x != m_vecGoalAngles.x)
	{
		float flDir = m_vecGoalAngles.x > m_vecCurAngles.x ? 1 : -1;
		float flDist = fabs(m_vecGoalAngles.x - m_vecCurAngles.x);

		m_vecCurAngles.x += m_flVerticalTurnRate * flDir;

		// if we started below the goal, and now we're past, peg to goal
		if (flDir == 1)
		{
			if (m_vecCurAngles.x > m_vecGoalAngles.x)
				m_vecCurAngles.x = m_vecGoalAngles.x;
		}
		else
		{
			if (m_vecCurAngles.x < m_vecGoalAngles.x)
				m_vecCurAngles.x = m_vecGoalAngles.x;
		}

		if (flDist < m_flVerticalTurnRate)
			m_vecCurAngles.x = m_vecGoalAngles.x;

		SetBoneController(1, m_vecCurAngles.x);
		state = true;
	}

	if (m_vecCurAngles.y != m_vecGoalAngles.y)
	{
		float flDir = m_vecGoalAngles.y > m_vecCurAngles.y ? 1 : -1;
		float flDist = fabs(m_vecGoalAngles.y - m_vecCurAngles.y);

		// 反方向更近
		if (flDist > 180)
		{
			flDist = 360 - flDist;
			flDir = -flDir;
		}

		m_vecCurAngles.y += m_flTurnRate * flDir;

		if (flDir == 1)
		{
			if (m_vecCurAngles.y > m_vecGoalAngles.y)
				m_vecCurAngles.y = m_vecGoalAngles.y;
		}
		else
		{
			if (m_vecCurAngles.y < m_vecGoalAngles.y)
				m_vecCurAngles.y = m_vecGoalAngles.y;
		}

	/*	if (m_vecCurAngles.y < 0)
			m_vecCurAngles.y += 360;
		else if (m_vecCurAngles.y >= 360)
			m_vecCurAngles.y -= 360;*/

		if (flDist < m_flTurnRate)
			m_vecCurAngles.y = m_vecGoalAngles.y;
		//ALERT(at_console, "m_vecCurAngles.y %f %f\n", m_vecCurAngles.y, m_vecGoalAngles.y);
		SetBoneController(0, m_vecCurAngles.y - pev->angles.y);
		state = true;
	}

	return state;
}

void CSentryGun::SetAnim(int iAnim)
{
	if(pev->sequence == iAnim) return;

	pev->frame = 0;
	pev->sequence = iAnim;
	ResetSequenceInfo();
}

}