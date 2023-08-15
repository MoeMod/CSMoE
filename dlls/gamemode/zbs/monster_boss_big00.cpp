
#include "monster_boss_big00.h"

namespace sv {

extern std::vector<BossData> g_BossData;


void CBossSkill_Big00::OnThink()
{
	CBaseMonsterSkill::OnThink();

	if (m_pMonster->m_flStop > gpGlobals->time)
		return;

	switch (m_iBossSkillStatus)
	{
	case sv::CBossSkill_Big00::NONE:
		if (m_pMonster->pev->movetype != MOVETYPE_STEP)
			m_pMonster->pev->movetype = MOVETYPE_STEP;
		break;
	case sv::CBossSkill_Big00::SWING:
		m_pMonster->pev->movetype = MOVETYPE_NONE;
		m_iBossSkillStatus = SWING_DMG;
		m_pMonster->SetEntAnim(m_pMonster->LookupSequence(m_CurSkillData.aniSeq.c_str()), 1.0, m_CurSkillData.aniTime, false);
		m_pMonster->m_flStop = gpGlobals->time + m_CurSkillData.damageDelay;
		break;
	case sv::CBossSkill_Big00::SWING_DMG:
		m_pMonster->RadiusDamage(m_pMonster->pev->origin, m_CurSkillData.damage, m_CurSkillData.damageDegree, m_CurSkillData.damageDist, m_CurSkillData.damageHeight, DMG_SLASH, m_CurSkillData.kickSide, m_CurSkillData.kickUp, m_CurSkillData.rushPower, false);
		m_pMonster->m_flStop = gpGlobals->time + m_CurSkillData.aniTime - m_CurSkillData.damageDelay;
		m_iBossSkillStatus = NONE;
		break;
	case sv::CBossSkill_Big00::SHOCKWAVE:
		m_pMonster->pev->movetype = MOVETYPE_NONE;
		m_iBossSkillStatus = SHOCKWAVE_DMG;
		m_pMonster->SetEntAnim(m_pMonster->LookupSequence(m_CurSkillData.aniSeq.c_str()), 1.0, m_CurSkillData.aniTime, false);
		m_pMonster->m_flStop = gpGlobals->time + m_CurSkillData.damageDelay + 0.16s;
		break;
	case sv::CBossSkill_Big00::SHOCKWAVE_DMG:
		m_pMonster->m_flStop = gpGlobals->time + m_CurSkillData.aniTime - m_CurSkillData.damageDelay;
		ShockWave(m_CurSkillData);
		m_iBossSkillStatus = NONE;
		break;
	case sv::CBossSkill_Big00::DASH:
		m_pMonster->pev->movetype = MOVETYPE_NONE;
		m_iBossSkillStatus = DASH_ACTION;
		m_pMonster->SetEntAnim(m_pMonster->LookupSequence(m_CurSkillData.aniSeq.c_str()), 1.0, 2.14s, false);
		m_pMonster->m_flStop = gpGlobals->time + m_CurSkillData.damageDelay;
		break;
	case sv::CBossSkill_Big00::DASH_ACTION:
		Dash(m_CurSkillData, m_pMonster->m_hTargetEnt);
		m_pMonster->m_flStop = gpGlobals->time + 2.14s - m_CurSkillData.damageDelay;
		m_iBossSkillStatus = DASH_END;
		break;
	case sv::CBossSkill_Big00::DASH_END:
		m_iBossSkillStatus = NONE;
		break;
	default:
		break;
	}
}

void CBossSkill_Big00::OnActivate(CBaseEntity* target)
{
	if (m_iBossSkillStatus != NONE)
		return;

	if (!m_pMonster->m_hTargetEnt) return;
	if (!m_pMonster->m_hTargetEnt->IsAlive()) return;
	if (!m_pMonster->m_hTargetEnt->IsPlayer()) return;

	std::vector<BossData> skill;
	for (auto iter = g_BossData.begin(); iter != g_BossData.end(); iter++)
	{
		if (iter->id != m_pMonster->m_iSkillId)
			continue;
		//根据血量筛选技能,血量区间的技能type必定唯一
		float flHealth = (m_pMonster->pev->health / m_pMonster->pev->max_health) * 100;
		if (flHealth < iter->startHealthMin || flHealth > iter->startHealthMax)
			continue;

		if (iter->damageDegree)
		{
			float c = cos(90.0 * 0.5 * M_PI / 180.0);

			if (!m_pMonster->AngularCheck(m_pMonster->m_hTargetEnt, m_pMonster, m_pMonster->m_hTargetEnt->pev->origin, iter->damageDegree))
				continue;
		}

		float dis = (m_pMonster->m_hTargetEnt->pev->origin - m_pMonster->pev->origin).Length();
		if (dis < iter->startDistMin || dis > iter->startDistMax)
			continue;

		auto it = m_MapSkillTime.find(iter->type);
		if (it != m_MapSkillTime.end() && it->second > gpGlobals->time)
			continue;

		skill.push_back(*iter);
	}

	if (skill.empty())
		return;

	//根据ratio选技能释放
	std::vector<std::pair<BossData, int>> items;
	for (auto iter = skill.begin(); iter != skill.end(); iter++)
	{
		items.push_back(std::pair(*iter, iter->ratio));
	}

	BossData useskill = RandomHelper::getInstance().randomWeightedChoice(items);
	if (useskill.id == 0)
		return;

	m_CurSkillData = useskill;
	m_iSkillStatus = SKILL_STATUS_FREEZING;
	m_flTimeSkillEnd = gpGlobals->time + 1.0s;
	m_MapSkillTime[useskill.type] = gpGlobals->time + useskill.cooltime;

	//使用技能
	if (useskill.type == 0)
	{
		m_iBossSkillStatus = SHOCKWAVE;
	}
	if (useskill.type == 1)
	{
		m_iBossSkillStatus = SWING;
	}
	else if (useskill.type == 2)
	{
		m_iBossSkillStatus = DASH;
	}
}

void CBossSkill_Big00::Dash(const BossData& sk, CBaseEntity* target)
{
	m_pMonster->pev->movetype = MOVETYPE_FLY;
	g_engfuncs.pfnWalkMove(m_pMonster->edict(), m_pMonster->pev->angles.y, 10.0, WALKMOVE_NORMAL);
	UTIL_MakeVectors(m_pMonster->pev->angles);
	Vector vecVelocity = gpGlobals->v_forward;
	vecVelocity *= 1000.0;
	m_pMonster->pev->velocity = vecVelocity;
}

void CBossSkill_Big00::ShockWave(const BossData& sk)
{
	TraceResult tr;
	float flDamage;
	CBaseEntity* pList[100];
	Vector vecStart;

	vecStart = m_pMonster->pev->origin + Vector(0.0, 0.0, 5.0);

	const float flRange = sk.damageDist / 2.0;
	int iCount = UTIL_EntitiesInBox(pList, 100, vecStart - Vector(flRange, flRange, flRange), vecStart + Vector(flRange, flRange, flRange), FL_CLIENT | FL_MONSTER);

	flDamage = sk.damage;

	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, vecStart);
	WRITE_BYTE(TE_BEAMCYLINDER);
	WRITE_COORD(vecStart[0]);
	WRITE_COORD(vecStart[1]);
	WRITE_COORD(vecStart[2]);
	WRITE_COORD(vecStart[0]);
	WRITE_COORD(vecStart[1]);
	WRITE_COORD(vecStart[2] + flRange);
	WRITE_SHORT(g_sModelIndexShockWave);
	WRITE_BYTE(0); // Start Frame
	WRITE_BYTE(0); // Framerate
	WRITE_BYTE(5); // Live Time
	WRITE_BYTE(60); // Width
	WRITE_BYTE(0); // Noise
	WRITE_BYTE(130); // R
	WRITE_BYTE(139); // G
	WRITE_BYTE(253); // B
	WRITE_BYTE(150); // Bright
	WRITE_BYTE(0); // Speed
	MESSAGE_END();

	for (int i = 0; i < iCount; i++)
	{
		CBaseEntity* pEntity = pList[i];

		if (pEntity == m_pMonster)
			continue;

		if (!pEntity->IsPlayer())
			continue;

		if (pEntity->pev->takedamage == DAMAGE_NO)
			continue;

		if(pEntity->pev->origin.z - vecStart.z > sk.damageHeight)
			continue;

		UTIL_TraceLine(vecStart, pEntity->BodyTarget(vecStart), dont_ignore_monsters, m_pMonster->edict(), &tr);

		if (tr.flFraction < 1.0 && tr.pHit != pEntity->edict())
			continue;

		pEntity->TakeDamage(m_pMonster->pev, m_pMonster->pev, flDamage, DMG_SLASH);

		((CBasePlayer*)pEntity)->ReactToDeimosTail();
	}
}

void CBossSkill_Big00::OnTouch(CBaseEntity* other)
{
	if (m_iBossSkillStatus != DASH_END)
		return;

	Vector2D vecKickRateDir = (other->pev->origin.Make2D() - m_pMonster->pev->origin.Make2D()).Normalize();

	other->pev->velocity.x = vecKickRateDir.x * m_CurSkillData.rushPower;
	other->pev->velocity.y = vecKickRateDir.y * m_CurSkillData.rushPower;
	other->pev->velocity.z += 150.0;

	if(other->IsPlayer())
		other->TakeDamage(m_pMonster->pev, m_pMonster->pev, m_CurSkillData.damage, DMG_SLASH);
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
	m_bAnimDeath = false;
	PRECACHE_MODEL("models/monster/zbs_bossl_big00.mdl");
	SET_MODEL(m_pMonster->edict(), "models/monster/zbs_bossl_big00.mdl");
	UTIL_SetSize(m_pMonster->pev, VEC_HULL_MIN, VEC_HULL_MAX);
	m_pMonster->SetSkill(std::make_unique<CBossSkill_Big00>(m_pMonster));
	m_pMonster->SetAble(JUMP, false);
	m_pMonster->m_turnTolerance = 400.0f;
}

void CBossClass_Big00::OnKilled(entvars_t* pKiller, int iGib)
{
	m_pMonster->pev->nextthink = gpGlobals->time + 5s;
	m_pMonster->pev->sequence = 3;
	m_pMonster->pev->frame = 0;
	m_pMonster->ResetSequenceInfo();
}
}
