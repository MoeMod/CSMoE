#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "schedule.h"
#include "player.h"
#include "monsters.h"
#include "trains.h"

#include "gamemode/mods.h"

#include "zsh_building_turret.h"

namespace sv
{

void CBuilding_Turret::OnCreate()
{
    PRECACHE_MODEL("models/zsh_building/zb_turret.mdl");
    SET_MODEL(m_pBuilding->edict(), "models/zsh_building/zb_turret.mdl");
    m_vecMins = Vector(-22.0, -15.0, 0.0);
    m_vecMaxs = Vector(22.0, 15.0, 75.0);
	m_pBuilding->m_flFieldOfView = VIEW_FIELD_FULL;
	m_flTurnRate = 10.0f;
	m_iRange = 1200;
	m_flFireRate = invalid_time_point;
	//m_iMinPitch = 15;
}

void CBuilding_Turret::OnSettle()
{
	SetAnim(3);
}

void CBuilding_Turret::OnCompleted()
{
	m_vecCurAngles = m_vecGoalAngles = m_pBuilding->pev->angles;
}

void CBuilding_Turret::OnThink()
{
	if (!m_pBuilding->isCompleted()) return;

	m_pBuilding->pev->nextthink = gpGlobals->time + 0.1s;
	m_pBuilding->StudioFrameAdvance();

	Search();
	if (m_pBuilding->m_fSequenceFinished)
	{
		SetAnim(0);
	}

	if (m_pBuilding->m_hEnemy)
	{
		Active();
	}
	else
	{
		Retire();
	}
}

void CBuilding_Turret::Search()
{
	// If we have a target and we're still healthy
	if (m_pBuilding->m_hEnemy)
	{
		if (!m_pBuilding->m_hEnemy->IsAlive())
			m_pBuilding->m_hEnemy = NULL;// Dead enemy forces a search for new one
	}

	if (m_pBuilding->m_hEnemy) return;

	// Acquire Target
	m_pBuilding->Look(m_iRange);
	m_pBuilding->m_hEnemy = BestVisibleEnemy();
}

void CBuilding_Turret::Active()
{
	auto pev = m_pBuilding->pev;
	Vector vecMid = pev->origin + pev->view_ofs;
	Vector vecMidEnemy = m_pBuilding->m_hEnemy->BodyTarget(vecMid);
	Vector vecDirToEnemy = vecMidEnemy - vecMid;	// calculate dir and dist to enemy
	float flDistToEnemy = vecDirToEnemy.LengthSquared();

	// Current enmey is not visible.

	if ((flDistToEnemy > m_iRange * m_iRange) || !FBoxVisible(pev, m_pBuilding->m_hEnemy->pev, vecMidEnemy))
	{
		m_pBuilding->m_hEnemy = NULL;
		return;
	}

	Vector vec = UTIL_VecToAngles(vecDirToEnemy);
	// [1...360]
	if (vec.y > 360)
		vec.y -= 360;
	if (vec.y < 0)
		vec.y += 360;
	// [-90...m_iMinPitch]
	/*if (vec.x < -180)
		vec.x += 360;
	if (vec.x > 180)
		vec.x -= 360;
	if (vec.x > 90)
		vec.x = 90;
	else if (vec.x < m_iMinPitch)
		vec.x = m_iMinPitch;*/

	m_vecGoalAngles.y = vec.y;
	//m_vecGoalAngles.x = vec.x;
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
	SetAnim(1);
	m_pBuilding->GetAttachment(0, vecLOS, vec);
	vecDirToEnemy = vecMidEnemy - vecLOS;
	m_pBuilding->FireBullets(1, vecLOS, vecDirToEnemy, g_vecZero, m_iRange, BULLET_MONSTER_9MM, 1, 10, m_pBuilding->m_hBuilder ? m_pBuilding->m_hBuilder->pev : NULL);

	switch (RANDOM_LONG(0, 2))
	{
	case 0: EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/hks1.wav", 1, ATTN_NORM); break;
	case 1: EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/hks2.wav", 1, ATTN_NORM); break;
	case 2: EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/hks3.wav", 1, ATTN_NORM); break;
	}
	pev->effects = pev->effects | EF_MUZZLEFLASH;
}

void CBuilding_Turret::Retire()
{
}

CBaseEntity* CBuilding_Turret::BestVisibleEnemy()
{
	CBaseEntity* pReturn;
	CBaseEntity* pNextEnt;
	int iNearest;
	int iDist;

	// so first visible entity will become the closest.
	iNearest = 67108864;
	pNextEnt = m_pBuilding->m_pLink;
	pReturn = NULL;

	while (pNextEnt != NULL)
	{
		if (!pNextEnt->IsPlayer() && pNextEnt->IsAlive() && FClassnameIs(pNextEnt->pev, "monster_entity"))
		{
			iDist = (pNextEnt->pev->origin - m_pBuilding->pev->origin).LengthSquared();

			if (iDist <= iNearest)
			{
				iNearest = iDist;
				pReturn = pNextEnt;
			}
		}

		pNextEnt = pNextEnt->m_pLink;
	}

	return pReturn;
}

bool CBuilding_Turret::MoveTurret()
{
	bool state = false;
	//if (m_vecCurAngles.x != m_vecGoalAngles.x)
	//{
	//	float flDir = m_vecGoalAngles.x > m_vecCurAngles.x ? 1 : -1 ;

	//	m_vecCurAngles.x += 0.1 * m_flTurnRate * flDir;

	//	// if we started below the goal, and now we're past, peg to goal
	//	if (flDir == 1)
	//	{
	//		if (m_vecCurAngles.x > m_vecGoalAngles.x)
	//			m_vecCurAngles.x = m_vecGoalAngles.x;
	//	} 
	//	else
	//	{
	//		if (m_vecCurAngles.x < m_vecGoalAngles.x)
	//			m_vecCurAngles.x = m_vecGoalAngles.x;
	//	}

	//	m_pBuilding->SetBoneController(1, -m_vecCurAngles.x);
	//	state = true;
	//}

	/*m_vecCurAngles.y -= int(m_vecCurAngles.y / 360.0) * 360.0;
	m_vecGoalAngles.y -= int(m_vecGoalAngles.y / 360.0) * 360.0;

	float delta = m_vecGoalAngles.y - m_vecCurAngles.y;
	if (delta < 0.0) delta += 360.0;

	if (delta <= 0.2 || delta >= 358.0) state = true;
	else if (delta < 180.0) m_vecCurAngles.y += m_flTurnRate;
	else {
		if (m_vecCurAngles.y - m_flTurnRate < m_vecGoalAngles.y) m_vecCurAngles.y -= 8.0;
		else m_vecCurAngles.y -= m_flTurnRate;
	}
	if (delta <= m_flTurnRate) m_vecCurAngles.y = m_vecGoalAngles.y;
	if (m_vecCurAngles.y >= 360.0) m_vecCurAngles.y -= 360.0;
	else if (m_vecCurAngles.y < 0.0) m_vecCurAngles.y += 360.0;

	if (!state) m_pBuilding->SetBoneController(0, m_vecCurAngles.y - m_pBuilding->pev->angles.y);*/

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

		if (m_vecCurAngles.y < 0)
			m_vecCurAngles.y += 360;
		else if (m_vecCurAngles.y >= 360)
			m_vecCurAngles.y -= 360;

		if (flDist < m_flTurnRate)
			m_vecCurAngles.y = m_vecGoalAngles.y;
		//ALERT(at_console, "m_vecCurAngles.y %f %f\n", m_vecCurAngles.y, m_vecGoalAngles.y);
		m_pBuilding->SetBoneController(0, m_vecCurAngles.y - m_pBuilding->pev->angles.y);
		state = true;
	}

	return state;
}

void CBuilding_Turret::SetAnim(int iAnim)
{
	if(m_pBuilding->pev->sequence == iAnim) return;

	m_pBuilding->pev->frame = 0;
	m_pBuilding->pev->sequence = iAnim;
	m_pBuilding->ResetSequenceInfo();
}

}