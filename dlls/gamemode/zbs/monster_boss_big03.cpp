#include <random>
#include "monster_boss_big03.h"

namespace sv {

extern std::vector<BossData> g_BossData;

LINK_ENTITY_TO_CLASS(zbs_big03_tentacle, CZBSBig03Tentacle);


void CZBSBig03Tentacle::Spawn(void)
{
	Precache();

	pev->movetype = MOVETYPE_BOUNCE;
	pev->solid = SOLID_BBOX;

	pev->frame = 0;

	ResetSequenceInfo();

	SET_MODEL(ENT(pev), "models/zbs_bossl_big02_bomb.mdl");
	UTIL_SetSize(pev, { -0.1, -0.1, -0.1 }, { 0.1, 0.1, 0.1 });

	SetTouch(&CZBSBig03Tentacle::BombTouch);
}

void CZBSBig03Tentacle::Precache(void)
{
	PRECACHE_MODEL("models/zbs_bossl_big02_bomb.mdl");
}

void CZBSBig03Tentacle::BombTouch(CBaseEntity* pOther)
{
	if (!pOther)
		return;

	if (pOther->edict() == pev->owner)
		return;

	entvars_t* pevOwner = VARS(pev->owner);

	CBaseEntity* pEntity = NULL;
	while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, m_flRadius)) != NULL)
	{
		if (pEntity->IsDormant())
			continue;

		if (pEntity->pev->takedamage == DAMAGE_NO)
			continue;

		if (pEntity->pev == pevOwner)
			continue;

		int falloff = m_flDamage / m_flRadius;

		float flCurrentDamage = max(0, m_flDamage - (pev->origin - pEntity->pev->origin).Length() * falloff);

		pEntity->TakeDamage(pev, pevOwner, flCurrentDamage, DMG_NEVERGIB | DMG_EXPLOSION);
	}

	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
	WRITE_BYTE(TE_EXPLOSION);
	WRITE_COORD(pev->origin.x);
	WRITE_COORD(pev->origin.y);
	WRITE_COORD(pev->origin.z);
	WRITE_SHORT(MODEL_INDEX("sprites/eexplo.spr"));
	WRITE_BYTE(25);
	WRITE_BYTE(30);
	WRITE_BYTE(TE_EXPLFLAG_NONE);
	MESSAGE_END();

	SetTouch(NULL);
	SetThink(NULL);

	UTIL_Remove(this);
}

void CBossSkill_Big03::OnThink()
{
	
	CBaseMonsterSkill::OnThink();

	if (m_pMonster->m_flStop > gpGlobals->time)
		return;

	switch (m_iBossSkillStatus)
	{
	case sv::CBossSkill_Big03::ZBS_NONE:
		if(m_pMonster->pev->movetype != MOVETYPE_STEP)
			m_pMonster->pev->movetype = MOVETYPE_STEP;
		break;
	case sv::CBossSkill_Big03::ZBS_APPEAR:
	{
		m_iBossSkillStatus = ZBS_APPEAR2;	
		m_pMonster->pev->movetype = MOVETYPE_NONE;
		m_pMonster->SetEntAnim(m_pMonster->LookupSequence(m_CurSkillData.aniSeq.c_str()), 1.0, m_CurSkillData.aniTime, false);
		m_pMonster->m_flStop = gpGlobals->time + m_CurSkillData.aniTime;
		break;
	}
	case sv::CBossSkill_Big03::ZBS_APPEAR2:
	{
		m_pMonster->pev->movetype = MOVETYPE_TOSS;
		UTIL_MakeVectors({ 0, 180, 0 });
		Vector vecVelocity = gpGlobals->v_forward;
		vecVelocity *= 1000.0;
		vecVelocity.z += 500;
		m_pMonster->pev->velocity = vecVelocity;
		m_pMonster->SetEntAnim(m_pMonster->LookupSequence("appear2"), 1.0, 1.5s, false);
		m_pMonster->m_flStop = gpGlobals->time + 0.2s;
		m_iBossSkillStatus = ZBS_APPEAR3;
		break;
	}
	case sv::CBossSkill_Big03::ZBS_APPEAR3:
		if (m_pMonster->pev->flags & FL_ONGROUND)
		{
			m_pMonster->pev->movetype = MOVETYPE_NONE;
			m_pMonster->SetEntAnim(m_pMonster->LookupSequence("appear3"), 1.0, 8.7s, false);
			m_pMonster->m_flStop = gpGlobals->time + 8.7s;
			m_iBossSkillStatus = ZBS_NONE;
		}
		break;
	case sv::CBossSkill_Big03::ZBS_ATTACK_BITE:
		m_pMonster->pev->movetype = MOVETYPE_NONE;
		m_iBossSkillStatus = ZBS_ATTACK_BITE_DMG;
		m_pMonster->SetEntAnim(m_pMonster->LookupSequence(m_CurSkillData.aniSeq.c_str()), 1.0, m_CurSkillData.aniTime, false);
		m_pMonster->m_flStop = gpGlobals->time + m_CurSkillData.damageDelay;
		break;
	case sv::CBossSkill_Big03::ZBS_ATTACK_BITE_DMG:
		m_pMonster->RadiusDamage(m_pMonster->pev->origin, m_CurSkillData.damage, m_CurSkillData.damageDegree, m_CurSkillData.damageDist, m_CurSkillData.damageHeight, DMG_SLASH, m_CurSkillData.kickSide, m_CurSkillData.kickUp, m_CurSkillData.rushPower, false);
		m_pMonster->m_flStop = gpGlobals->time + m_CurSkillData.aniTime - m_CurSkillData.damageDelay;
		m_iBossSkillStatus = ZBS_NONE;
		break;
	case sv::CBossSkill_Big03::ZBS_ATTACK_SWING:
		m_pMonster->pev->movetype = MOVETYPE_NONE;
		m_iBossSkillStatus = ZBS_ATTACK_BITE_DMG;
		m_pMonster->SetEntAnim(m_pMonster->LookupSequence(m_CurSkillData.aniSeq.c_str()), 1.0, m_CurSkillData.aniTime, false);
		m_pMonster->m_flStop = gpGlobals->time + m_CurSkillData.damageDelay;
		break;
	case sv::CBossSkill_Big03::ZBS_ATTACK_SWING_DMG:
		m_pMonster->RadiusDamage(m_pMonster->pev->origin, m_CurSkillData.damage, 360, m_CurSkillData.damageDist, m_CurSkillData.damageHeight, DMG_SLASH, m_CurSkillData.kickSide, m_CurSkillData.kickUp, m_CurSkillData.rushPower, false);
		m_pMonster->m_flStop = gpGlobals->time + m_CurSkillData.aniTime - m_CurSkillData.damageDelay;
		m_iBossSkillStatus = ZBS_NONE;
		break;
	default:
		break;
	}
}

void CBossSkill_Big03::OnActivate(CBaseEntity* target)
{
	if (m_iBossSkillStatus != ZBS_NONE)
		return;

	std::vector<BossData> skill;
	for (auto iter = g_BossData.begin(); iter != g_BossData.end(); iter++)
	{
		//根据血量筛选技能,血量区间的技能type必定唯一
		float flHealth = (m_pMonster->pev->health / m_pMonster->pev->max_health) * 100;
		if (flHealth < iter->startHealthMin || flHealth > iter->startHealthMax)
			continue;

		if (m_pMonster->m_hTargetEnt)
		{
			if (!m_pMonster->m_hTargetEnt->IsAlive())
				continue;

			if (!m_pMonster->m_hTargetEnt->IsPlayer())
				continue;

			if (iter->damageDegree)
			{
				float c = cos(90.0 * 0.5 * M_PI / 180.0);

				if (!m_pMonster->AngularCheck(m_pMonster->m_hTargetEnt, m_pMonster, m_pMonster->m_hTargetEnt->pev->origin, iter->damageDegree, c))
					continue;
			}

			float dis = (m_pMonster->m_hTargetEnt->pev->origin - m_pMonster->pev->origin).Length();
			if (dis < iter->startDistMin || dis > iter->startDistMax)
				continue;
		}
		
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
		if (iter->startHealthMax - iter->startHealthMin < 2)
		{
			//most high level appear & exit
			items.clear();
			items.push_back(std::pair(*iter, iter->ratio));
			break;
		}

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
	if (useskill.type == 10)
	{
		m_iBossSkillStatus = ZBS_APPEAR;
	}
	else if (useskill.type == 11)
	{
		m_iBossSkillStatus = ZBS_ESCAPE;
	}
	else if (useskill.type == 12)
	{
		m_iBossSkillStatus = ZBS_ATTACK_BITE;
	}
	else if (useskill.type == 13)
	{
		m_iBossSkillStatus = ZBS_ATTACK_SWING;
	}
	else
	{
		m_iBossSkillStatus = ZBS_ATTACK_BITE;
	}
#if 0
	else if (useskill.type == 14)
	{
		m_iBossSkillStatus = ZBS_ATTACK_TENTACLE;
	}
	else if (useskill.type == 15)
	{
		m_iBossSkillStatus = ZBS_ATTACK_TENTACLE;
	}
	else if (useskill.type == 16)
	{
		m_iBossSkillStatus = ZBS_ATTACK_TENTACLE;
	}
	else if (useskill.type == 17)
	{
		m_iBossSkillStatus = ZBS_ATTACK_POISON;
	}
	else if (useskill.type == 18)
	{
		m_iBossSkillStatus = ZBS_ATTACK_POISON;
	}
	else if (useskill.type == 19)
	{
		m_iBossSkillStatus = ZBS_ATTACK_ROLLING;
	}
#endif // 0

}

void CBossSkill_Big03::Jump(const BossData& sk, CBaseEntity* target)
{
	m_pMonster->pev->movetype = MOVETYPE_FLY;
	Vector vecDir = (target->pev->origin - m_pMonster->pev->origin).Normalize();
	m_pMonster->pev->velocity = vecDir *= 1000.0;
}

void CBossClass_Big03::OnSpawn()
{
	m_pMonster->pev->max_health = 1000.0f;
	m_pMonster->m_flAttackDamage = 10.0f;
	m_pMonster->m_flAttackRate = 2.0s;
	m_pMonster->m_flAttackDist = 72.0f;
	m_pMonster->pev->maxspeed = 225.0f;
	m_pMonster->pev->nextthink = gpGlobals->time;
	m_pMonster->m_flNextFullThink = gpGlobals->time;
	m_pMonster->m_flStop = gpGlobals->time;
	m_pMonster->m_flSkillCheck = gpGlobals->time;
}

void CBossClass_Big03::OnInit()
{
	m_bAnimDeath = false;
	PRECACHE_MODEL("models/monster/zbs_bossl_big03.mdl");
	SET_MODEL(m_pMonster->edict(), "models/monster/zbs_bossl_big03.mdl");
	UTIL_SetSize(m_pMonster->pev, VEC_HULL_MIN, VEC_HULL_MAX);
	m_pMonster->SetSkill(std::make_unique<CBossSkill_Big03>(m_pMonster));
	m_pMonster->SetAble(JUMP, false);
	m_pMonster->m_turnTolerance = 400.0f;
}

void CBossClass_Big03::OnKilled(entvars_t* pKiller, int iGib)
{
	m_pMonster->pev->nextthink = gpGlobals->time + 10s;
	m_pMonster->pev->sequence = 20;
	m_pMonster->pev->frame = 0;
	m_pMonster->ResetSequenceInfo();
}

}
