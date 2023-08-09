
#include "monster_boss_big02.h"

namespace sv {

extern std::vector<BossData> g_BossData;

LINK_ENTITY_TO_CLASS(zbs_big02_bomb, CZBSBig02Bomb);


void CZBSBig02Bomb::Spawn(void)
{
	Precache();

	pev->movetype = MOVETYPE_BOUNCE;
	pev->solid = SOLID_BBOX;

	pev->frame = 0;

	ResetSequenceInfo();

	SET_MODEL(ENT(pev), "models/zbs_bossl_big02_bomb.mdl");
	UTIL_SetSize(pev, { -0.1, -0.1, -0.1 }, { 0.1, 0.1, 0.1 });

	SetTouch(&CZBSBig02Bomb::BombTouch);
}

void CZBSBig02Bomb::Precache(void)
{
	PRECACHE_MODEL("models/zbs_bossl_big02_bomb.mdl");
}

void CZBSBig02Bomb::BombTouch(CBaseEntity* pOther)
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

void CBossSkill_Big02::OnThink()
{
	CBaseMonsterSkill::OnThink();

	if (m_pMonster->m_flStop > gpGlobals->time)
		return;

	switch (m_iBossSkillStatus)
	{
	case sv::CBossSkill_Big02::ZBS_NONE:
		if(m_pMonster->pev->movetype != MOVETYPE_STEP)
			m_pMonster->pev->movetype = MOVETYPE_STEP;
		break;
	case sv::CBossSkill_Big02::ZBS_ATTACK1:

		m_iBossSkillStatus = ZBS_ATTACK1_DMG;
		m_pMonster->pev->movetype = MOVETYPE_NONE;
		m_pMonster->SetEntAnim(m_pMonster->LookupSequence(m_CurSkillData.aniSeq.c_str()), 1.0, m_CurSkillData.aniTime, false);
		m_pMonster->m_flStop = gpGlobals->time + m_CurSkillData.damageDelay;
		break;
	case sv::CBossSkill_Big02::ZBS_ATTACK1_DMG:
		if (strstr(m_CurSkillData.aniSeq.c_str(), "knife"))
			UTIL_TempModel({ 0, 0, 10 }, m_pMonster->pev->angles, g_vecZero, m_iKnifeIndex, 10, 0, 10, 0, 0, 255, kRenderTransAdd, m_pMonster, true, 16, 0, 256, 0x20000000);
		m_pMonster->RadiusDamage(m_pMonster->pev->origin, m_CurSkillData.damage, m_CurSkillData.damageDegree, m_CurSkillData.damageDist, m_CurSkillData.damageHeight, DMG_SLASH, m_CurSkillData.kickSide, m_CurSkillData.kickUp, m_CurSkillData.rushPower, false);
		m_pMonster->m_flStop = gpGlobals->time + m_CurSkillData.aniTime - m_CurSkillData.damageDelay;
		m_iBossSkillStatus = ZBS_NONE;
		break;
	case sv::CBossSkill_Big02::ZBS_ATTACK2:
		m_pMonster->pev->movetype = MOVETYPE_NONE;
		m_iBossSkillStatus = ZBS_ATTACK2_DMG;
		m_pMonster->SetEntAnim(m_pMonster->LookupSequence(m_CurSkillData.aniSeq.c_str()), 1.0, m_CurSkillData.aniTime, false);
		m_pMonster->m_flStop = gpGlobals->time + m_CurSkillData.damageDelay + 0.16s;
		break;
	case sv::CBossSkill_Big02::ZBS_ATTACK2_DMG:
		if (strstr(m_CurSkillData.aniSeq.c_str(), "knife"))
			UTIL_TempModel({ 0, 0, 10 }, m_pMonster->pev->angles, g_vecZero, m_iKnifeIndex, 10, 1, 10, 0, 0, 255, kRenderTransAdd, m_pMonster, true, 16, 0, 256, 0x20000000);
		m_pMonster->m_flStop = gpGlobals->time + m_CurSkillData.aniTime - m_CurSkillData.damageDelay;
		m_pMonster->RadiusDamage(m_pMonster->pev->origin, m_CurSkillData.damage, m_CurSkillData.damageDegree, m_CurSkillData.damageDist, m_CurSkillData.damageHeight, DMG_SLASH, m_CurSkillData.kickSide, m_CurSkillData.kickUp, m_CurSkillData.rushPower, false);
		m_iBossSkillStatus = ZBS_NONE;
		break;
	case sv::CBossSkill_Big02::ZBS_ATTACK3:
		m_pMonster->pev->movetype = MOVETYPE_NONE;
		m_iBossSkillStatus = ZBS_ATTACK3_ACTION_JUMP;
		m_pMonster->SetEntAnim(m_pMonster->LookupSequence(m_CurSkillData.aniSeq.c_str()), 1.0, m_CurSkillData.aniTime, false);
		m_pMonster->m_flStop = gpGlobals->time + m_CurSkillData.damageDelay;
		break;
	case sv::CBossSkill_Big02::ZBS_ATTACK3_ACTION_JUMP:
	{
		m_pMonster->pev->movetype = MOVETYPE_BOUNCE;
		m_iBossSkillStatus = ZBS_ATTACK3_ACTION_TRACE;
		UTIL_MakeVectors(m_pMonster->pev->angles);
		Vector vecVelocity = gpGlobals->v_forward;
		vecVelocity *= 1000.0;
		vecVelocity.z += 500;
		m_pMonster->pev->velocity = vecVelocity;
		m_pMonster->m_flStop = gpGlobals->time + m_CurSkillData.damageDelay;
		break;
	}
	case sv::CBossSkill_Big02::ZBS_ATTACK3_ACTION_TRACE:
		m_iBossSkillStatus = ZBS_ATTACK3_END;
		m_pMonster->m_flStop = gpGlobals->time + 1s;
		Jump(m_CurSkillData, m_pMonster->m_hTargetEnt);
		break;
	case sv::CBossSkill_Big02::ZBS_ATTACK3_END:
		m_pMonster->pev->movetype = MOVETYPE_NONE;
		m_pMonster->RadiusDamage(m_pMonster->pev->origin, m_CurSkillData.damage, 360.0f, m_CurSkillData.damageDist, m_CurSkillData.damageHeight, DMG_SLASH, m_CurSkillData.kickSide, m_CurSkillData.kickUp, m_CurSkillData.rushPower, false);
		m_pMonster->m_flStop = gpGlobals->time + 1s;
		m_iBossSkillStatus = ZBS_NONE;
		break;
	case sv::CBossSkill_Big02::ZBS_ATTACK_BOMB:
		m_pMonster->pev->movetype = MOVETYPE_NONE;
		m_iBombSkillCount = 3;
		m_pMonster->SetEntAnim(m_pMonster->LookupSequence(m_CurSkillData.aniSeq.c_str()), 1.0, m_CurSkillData.aniTime, false);
		m_pMonster->m_flStop = gpGlobals->time + m_CurSkillData.damageDelay;
		m_iBossSkillStatus = ZBS_ATTACK_BOMB_ACTION;
		break;
	case sv::CBossSkill_Big02::ZBS_ATTACK_BOMB_ACTION:
	{
		m_iBombSkillCount--;
		if (!m_iBombSkillCount)
		{
			m_pMonster->m_flStop = gpGlobals->time + 2s;
			m_iBossSkillStatus = ZBS_NONE;
		}
		else
			m_pMonster->m_flStop = gpGlobals->time + m_CurSkillData.damageDelay;

		Vector vecSrc = m_pMonster->pev->origin;
		vecSrc.z += 40.0;
	
		for (int i = 0; i < 3; i++)
		{
			Vector vecVelocity;

			vecVelocity.z = RANDOM_FLOAT(800.0f, 900.0f);

			//Upper right 
			float flDispersion = 500.0;
			vecVelocity[0] += flDispersion + RANDOM_FLOAT(100.0f, 200.0f);
			vecVelocity[1] += flDispersion + RANDOM_FLOAT(100.0f, 200.0f);

			vecSrc.x += RANDOM_FLOAT(-5, 5);
			CZBSBig02Bomb* pEntUR = static_cast<CZBSBig02Bomb*>(CBaseEntity::Create("zbs_big02_bomb", vecSrc, g_vecZero, m_pMonster->edict()));
			if (pEntUR)
			{
				pEntUR->pev->velocity = vecVelocity;
				pEntUR->m_flDamage = m_CurSkillData.damage;
				pEntUR->m_flRadius = m_CurSkillData.damageDist;
			}

			//Upper left
			vecSrc.y += RANDOM_FLOAT(-5, 5);
			vecVelocity[0] -= flDispersion * 2 - RANDOM_FLOAT(100.0f, 200.0f);
			CZBSBig02Bomb* pEntUL = static_cast<CZBSBig02Bomb*>(CBaseEntity::Create("zbs_big02_bomb", vecSrc, g_vecZero, m_pMonster->edict()));
			if (pEntUL)
			{
				pEntUL->pev->velocity = vecVelocity;
				pEntUL->m_flDamage = m_CurSkillData.damage;
				pEntUL->m_flRadius = m_CurSkillData.damageDist;
			}

			//Lower left
			vecSrc.y += RANDOM_FLOAT(-5, 5);
			vecVelocity[1] -= flDispersion * 2 - RANDOM_FLOAT(100.0f, 200.0f);
			CZBSBig02Bomb* pEntLL = static_cast<CZBSBig02Bomb*>(CBaseEntity::Create("zbs_big02_bomb", vecSrc, g_vecZero, m_pMonster->edict()));
			if (pEntLL)
			{
				pEntLL->pev->velocity = vecVelocity;
				pEntLL->m_flDamage = m_CurSkillData.damage;
				pEntLL->m_flRadius = m_CurSkillData.damageDist;
			}

			//Lower right
			vecSrc.x += RANDOM_FLOAT(-5, 5);
			vecVelocity[0] += flDispersion * 2 + RANDOM_FLOAT(100.0f, 200.0f);
			CZBSBig02Bomb* pEntLR = static_cast<CZBSBig02Bomb*>(CBaseEntity::Create("zbs_big02_bomb", vecSrc, g_vecZero, m_pMonster->edict()));
			if (pEntLR)
			{
				pEntLR->pev->velocity = vecVelocity;
				pEntLR->m_flDamage = m_CurSkillData.damage;
				pEntLR->m_flRadius = m_CurSkillData.damageDist;
			}
		}
		
		break;
	}
	case sv::CBossSkill_Big02::ZBS_ATTACK_HOLE:
	{
		m_pMonster->pev->movetype = MOVETYPE_NONE;
		UTIL_TempModel({ 0, 0, -32 }, g_vecZero, g_vecZero, m_iHoleIndex, 54, 0, 10, 0, 0, 255, kRenderTransAdd, m_pMonster, false, 0, 0, 256, 0x20000000);
		m_iBossSkillStatus = ZBS_ATTACK_HOLE_ACTION;
		m_pMonster->SetEntAnim(m_pMonster->LookupSequence(m_CurSkillData.aniSeq.c_str()), 1.0, m_CurSkillData.aniTime, false);
		m_flHoleSkillTime = gpGlobals->time + m_CurSkillData.damageDelay;
		m_pMonster->m_flStop = gpGlobals->time;
		break;
	}
	case sv::CBossSkill_Big02::ZBS_ATTACK_HOLE_ACTION:
		if(m_flHoleSkillTime < gpGlobals->time)
			m_iBossSkillStatus = ZBS_ATTACK_HOLE_END;
		AttackHole(m_CurSkillData, true);
		m_pMonster->m_flStop = gpGlobals->time + 0.1s;
		break;
	case sv::CBossSkill_Big02::ZBS_ATTACK_HOLE_END:
		if (strstr(m_CurSkillData.aniSeq.c_str(), "knife"))
			UTIL_TempModel({ 0, 0, 10 }, m_pMonster->pev->angles, g_vecZero, m_iKnifeIndex, 10, 2, 10, 0, 0, 255, kRenderTransAdd, m_pMonster, true, 16, 0, 256, 0x20000000);
		AttackHole(m_CurSkillData, false);
		m_pMonster->m_flStop = gpGlobals->time + m_CurSkillData.aniTime - m_CurSkillData.damageDelay;
		m_iBossSkillStatus = ZBS_NONE;
		break;
	case sv::CBossSkill_Big02::ZBS_APPEAR:
		m_pMonster->pev->movetype = MOVETYPE_NONE;
		m_pMonster->SetEntAnim(m_pMonster->LookupSequence(m_CurSkillData.aniSeq.c_str()), 1.0, m_CurSkillData.aniTime, false);
		m_pMonster->m_flStop = gpGlobals->time + m_CurSkillData.aniTime;
		m_iBossSkillStatus = ZBS_NONE;
		break;
	case sv::CBossSkill_Big02::ZBS_SCENE_KNIFE:	
		MESSAGE_BEGIN(MSG_ALL, gmsgZBSMsg);
		WRITE_SHORT(std::atoi(m_CurSkillData.startMsg.c_str()));
		MESSAGE_END();

		m_pMonster->pev->movetype = MOVETYPE_NONE;
		m_pMonster->SetEntAnim(m_pMonster->LookupSequence(m_CurSkillData.aniSeq.c_str()), 1.0, m_CurSkillData.aniTime, false);
		m_pMonster->m_flStop = gpGlobals->time + m_CurSkillData.aniTime;
		m_iBossSkillStatus = ZBS_NONE;
		break;
	default:
		break;
	}
}

void CBossSkill_Big02::OnActivate(CBaseEntity* target)
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
	if (useskill.type == 3)
	{
		m_iBossSkillStatus = ZBS_ATTACK1;
	}
	if (useskill.type == 4)
	{
		m_iBossSkillStatus = ZBS_ATTACK2;
	}
	else if (useskill.type == 5)
	{
		m_iBossSkillStatus = ZBS_ATTACK3;
	}
	else if (useskill.type == 6)
	{
		m_iBossSkillStatus = ZBS_ATTACK_HOLE;
	}
	else if (useskill.type == 7)
	{
		m_iBossSkillStatus = ZBS_ATTACK_BOMB;
	}
	else if (useskill.type == 8)
	{
		m_iBossSkillStatus = ZBS_APPEAR;
	}
	else if (useskill.type == 9)
	{
		m_iBossSkillStatus = ZBS_SCENE_KNIFE;
	}
}

void CBossSkill_Big02::Jump(const BossData& sk, CBaseEntity* target)
{
	m_pMonster->pev->movetype = MOVETYPE_FLY;
	Vector vecDir = (target->pev->origin - m_pMonster->pev->origin).Normalize();
	m_pMonster->pev->velocity = vecDir *= 1000.0;
}

void CBossSkill_Big02::AttackHole(const BossData& sk, bool loop)
{
	TraceResult tr;
	float flDamage;
	CBaseEntity* pList[100];
	Vector vecStart;

	vecStart = m_pMonster->pev->origin + Vector(0.0, 0.0, 5.0);

	float flRange = 200.0f + sk.damageDist;

	if (!loop)
		flRange = 200;

	int iCount = UTIL_EntitiesInBox(pList, 100, vecStart - Vector(flRange, flRange, flRange), vecStart + Vector(flRange, flRange, flRange), FL_CLIENT | FL_MONSTER);

	flDamage = sk.damage;

	for (int i = 0; i < iCount; i++)
	{
		CBaseEntity* pEntity = pList[i];

		if (pEntity == m_pMonster)
			continue;

		if (!pEntity->IsPlayer())
			continue;

		if (pEntity->pev->takedamage == DAMAGE_NO)
			continue;

		if (pEntity->pev->origin.z - vecStart.z > sk.damageHeight)
			continue;

		Vector vecDir = (m_pMonster->pev->origin - pEntity->pev->origin).Normalize();

		if (loop)
		{
			pEntity->pev->velocity.x = vecDir.x * 800;
			pEntity->pev->velocity.y = vecDir.x * 800;
		}
		else
		{
			pEntity->pev->velocity.x += vecDir.x * sk.rushPower;
			pEntity->pev->velocity.y += vecDir.x * sk.rushPower;
			pEntity->pev->velocity.z += 100.0;

			pEntity->TakeDamage(m_pMonster->pev, m_pMonster->pev, flDamage, DMG_SLASH);
		}
	}
}

void CBossClass_Big02::OnSpawn()
{
	m_pMonster->pev->max_health = 1000.0f;
	m_pMonster->m_flAttackDamage = 10.0f;
	m_pMonster->m_flAttackRate = 2.0s;
	m_pMonster->m_flAttackDist = 72.0f;
	m_pMonster->pev->maxspeed = 225.0f;
	m_pMonster->m_turnTolerance = 400.0f;
	m_pMonster->pev->nextthink = gpGlobals->time;
	m_pMonster->m_flNextFullThink = gpGlobals->time;
	m_pMonster->m_flStop = gpGlobals->time;
	m_pMonster->m_flSkillCheck = gpGlobals->time;
}

void CBossClass_Big02::OnInit()
{
	m_bAnimDeath = false;
	PRECACHE_MODEL("models/monster/zbs_bossl_big02.mdl");
	SET_MODEL(m_pMonster->edict(), "models/monster/zbs_bossl_big02.mdl");
	UTIL_SetSize(m_pMonster->pev, VEC_HULL_MIN, VEC_HULL_MAX);
	m_pMonster->SetSkill(std::make_unique<CBossSkill_Big02>(m_pMonster));
	m_pMonster->SetAble(JUMP, false);
	
}

void CBossClass_Big02::OnKilled(entvars_t* pKiller, int iGib)
{
	m_pMonster->pev->nextthink = gpGlobals->time + 10s;
	m_pMonster->pev->sequence = 20;
	m_pMonster->pev->frame = 0;
	m_pMonster->ResetSequenceInfo();
}

}
