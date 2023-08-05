

#include "monster_boss_big00.h"

namespace sv {

void CBossSkill_Big00::OnThink()
{
	if (m_flTimeTriggerZombieSkill != invalid_time_point && m_flTimeTriggerZombieSkill < gpGlobals->time)
	{
		FireZombieSkill();
	}
	CBaseMonsterSkill::OnThink();
}

void CBossSkill_Big00::FireZombieSkill()
{
	UTIL_MakeAimVectors(m_pMonster->pev->angles);

	Vector velocity = gpGlobals->v_forward;

	velocity.x *= 1000.0f;
	velocity.y *= 1000.0f;

	m_pMonster->pev->velocity = velocity;
	m_flTimeTriggerZombieSkill = invalid_time_point;
}

void CBossSkill_Big00::OnActivate(CBaseEntity* target)
{
	if (!(m_pMonster->pev->flags & FL_ONGROUND)) return;
	if (!target) return;
	if (!target->IsAlive()) return;
	if (!target->IsPlayer()) return;

	Vector vecDelta = target->pev->origin - m_pMonster->pev->origin;
	if (vecDelta.IsLengthGreaterThan(1500.0f))
		return;

	const duration_t flSkillTime = 2.2s;

	m_pMonster->m_iCustomSequence = m_pMonster->LookupSequence("zbs_skill_mahadash");
	m_pMonster->pev->velocity = g_vecZero;
	m_pMonster->pev->maxspeed = 1.0f;
	m_pMonster->m_flCustomSequenceUntilTime = gpGlobals->time + 2.2s;

	m_pMonster->SetAnimation(MONSTERANIM_IDLE);

	m_pMonster->m_flStop = gpGlobals->time + flSkillTime;

	m_flTimeTriggerZombieSkill = gpGlobals->time + 0.65s;
	m_iSkillStatus = SKILL_STATUS_USING;
	m_flTimeSkillEnd = m_pMonster->m_flStop;
}

void CBossSkill_Big00::OnSkillEnd()
{
	m_iSkillStatus = SKILL_STATUS_FREEZING;
	m_pMonster->pev->maxspeed = 225.0f;
	m_flTimeSkillNext = gpGlobals->time + 7s;
}

void CBossSkill_Big00::OnTouch(CBaseEntity* other)
{
	if (m_iSkillStatus != SKILL_STATUS_USING)
		return;

	Vector2D vecKickRateDir = (other->pev->origin.Make2D() - m_pMonster->pev->origin.Make2D()).Normalize();

	other->pev->velocity.x = vecKickRateDir.x * 800;
	other->pev->velocity.y = vecKickRateDir.y * 800;
	other->pev->velocity.z += 150.0;

	if(other->IsPlayer())
		other->TakeDamage(m_pMonster->pev, m_pMonster->pev, GetDamage(), DMG_BULLET);
}

float CBossSkill_Big00::GetDamage()
{
	return 2.0;
}

void CBossClass_Big00::OnSpawn()
{
	m_pMonster->pev->max_health = 1000.0f;
	m_pMonster->m_flAttackDamage = 10.0f;
	m_pMonster->m_flAttackRate = 2.0s;
	m_pMonster->m_flAttackDist = 72.0f;
	m_pMonster->pev->maxspeed = 225.0f;
}

void CBossClass_Big00::OnInit()
{
	PRECACHE_MODEL("models/monster/zbs_bossl_big00.mdl");
	SET_MODEL(m_pMonster->edict(), "models/monster/zbs_bossl_big00.mdl");
	UTIL_SetSize(m_pMonster->pev, VEC_HULL_MIN, VEC_HULL_MAX);
	m_pMonster->SetSkill(std::make_unique<CBossSkill_Big00>(m_pMonster));
}

}
