#include <random>
#include "monster_boss_big08.h"

namespace sv {

extern std::vector<BossData> g_BossData;

LINK_ENTITY_TO_CLASS(zbs_big08_hurricane, CZBSBig08Hurricane);


void CZBSBig08Hurricane::Spawn(void)
{
	Precache();

	pev->movetype = MOVETYPE_BOUNCE;
	pev->solid = SOLID_TRIGGER;

	pev->frame = 0;

	ResetSequenceInfo();

	SET_MODEL(ENT(pev), "models/ef_hurricane.mdl");
	UTIL_SetSize(pev, { -0.1, -0.1, -0.1 }, { 0.1, 0.1, 0.1 });

	pev->nextthink = gpGlobals->time + 0.1s;

	SetThink(&CZBSBig08Hurricane::PoisonThink);
	
}

void CZBSBig08Hurricane::Precache(void)
{
	PRECACHE_MODEL("models/ef_hurricane.mdl");
	PRECACHE_MODEL("models/ef_hurricane2.mdl");
}

void CZBSBig08Hurricane::PoisonThink()
{
	SetTouch(&CZBSBig08Hurricane::PoisonTouch);
	SetThink(&CZBSBig08Hurricane::FollowThink);
	pev->nextthink = gpGlobals->time + 0.1s;
}

void CZBSBig08Hurricane::FollowThink(void)
{
	pev->angles = UTIL_VecToAngles(pev->velocity.Normalize());
	pev->nextthink = gpGlobals->time + 0.1s;
}

void CZBSBig08Hurricane::PoisonTouch(CBaseEntity* pOther)
{
	if (!pOther)
		return;

	if (pOther->edict() == pev->owner)
		return;


}

LINK_ENTITY_TO_CLASS(zbs_big08_tentacle, CZBSBig08Tentacle);


void CZBSBig08Tentacle::Spawn(void)
{
	Precache();

	pev->movetype = MOVETYPE_NONE;
	pev->solid = SOLID_NOT;
	pev->rendermode = kRenderTransAdd;
	pev->renderamt = 150.0;
	pev->frame = 0;

	ResetSequenceInfo();

	SET_MODEL(ENT(pev), "models/ef_tentacle_sign.mdl");
	UTIL_SetSize(pev, g_vecZero, g_vecZero);

	pev->nextthink = gpGlobals->time + 1.0s;

	SetThink(&CZBSBig08Tentacle::TentacleThink);
	
}

void CZBSBig08Tentacle::Precache(void)
{
	PRECACHE_MODEL("models/tentacle.mdl");
	PRECACHE_MODEL("models/ef_tentacle_sign.mdl");
	PRECACHE_MODEL("models/ef_tentacle.mdl");
}

void CZBSBig08Tentacle::TentacleThink()
{
	SET_MODEL(ENT(pev), "models/tentacle.mdl");
	UTIL_SetSize(pev, { -4, -4, -4 }, { 4, 4, 4 });

	SetTouch(&CZBSBig08Tentacle::TentacleTouch);

	pev->movetype = MOVETYPE_NONE;
	pev->solid = SOLID_NOT;
	pev->rendermode = kRenderNormal;
	pev->renderamt = 255.0;
	pev->frame = 0;

	ResetSequenceInfo();

	SetThink(&CZBSBig08Tentacle::RemoveThink);
	pev->nextthink = gpGlobals->time + 0.1s;
}

void CZBSBig08Tentacle::RemoveThink()
{
	SetTouch(nullptr);

	pev->nextthink = gpGlobals->time + 5.0s;
	SetThink(&CBaseEntity::SUB_Remove);
}

void CZBSBig08Tentacle::TentacleTouch(CBaseEntity* pOther)
{
	if (!pOther->IsPlayer())
		return;

	if (m_hTarget != nullptr)
	{
		if (m_hTarget->IsAlive())
			m_hTarget->pev->velocity = Vector(0, 0, -200);

		return;
	}

	CBasePlayer* p = static_cast<CBasePlayer*>(pOther);

	m_hTarget = p;
	pev->origin.x = p->pev->origin.x;
	pev->origin.y = p->pev->origin.y;

	p->m_tLockBuffTime = gpGlobals->time + GetTrappedTime();
	p->m_flLockGravity = 10.0f;
	p->m_flLockSpeed = 1.0f;
	p->ResetMaxSpeed();
}

void CBossSkill_Big08::OnThink()
{
	
	CBaseMonsterSkill::OnThink();

	if (m_iSkillStatus == SKILL_STATUS_USING && !m_task_running && m_CurSkillData.id)
	{
		if (m_pMonster->m_hTargetEnt && (m_CurSkillData.approachDist || m_CurSkillData.forceAttackTime != 0s))
		{
			if (m_CurSkillData.approachDist > (m_pMonster->m_hTargetEnt->pev->origin - m_pMonster->pev->origin).Length() || gpGlobals->time > m_flForceAttackTime)
				ActivateSkill(m_pMonster->m_hTargetEnt);
		}
		else
			ActivateSkill(m_pMonster->m_hTargetEnt);
	}

}

void CBossSkill_Big08::ActivateSkill(CBaseEntity* target)
{
	m_iSkillStatus = SKILL_STATUS_FREEZING;
	m_flTimeSkillNext = m_flForceAttackTime + 15.0s;
	m_MapSkillTime[m_CurSkillData.raw] = gpGlobals->time + m_CurSkillData.cooltime;

	//使用技能
	switch (m_CurSkillData.type)
	{
	case 10:
		StartTask(TASK_ZBS_APPEAR(), boost::asio::detached);
		break;
	case 11:
		StartTask(TASK_ZBS_RUN(), boost::asio::detached);
		break;
	case 12:
		StartTask(ZBS_ATTACK_BITE(), boost::asio::detached);
		break;
	case 13:
		StartTask(ZBS_ATTACK_SWING(), boost::asio::detached);
		break;
	case 14:
	case 15:
		StartTask(ZBS_ATTACK_TENTACLE1(), boost::asio::detached);
		break;
	case 16:
		StartTask(ZBS_ATTACK_TENTACLE2(), boost::asio::detached);
		break;
	case 17:
		StartTask(ZBS_ATTACK_POISON1(), boost::asio::detached);
		break;
	case 18:
		StartTask(ZBS_ATTACK_POISON2(), boost::asio::detached);
		break;
	case 19:
		StartTask(ZBS_ATTACK_ROLLING(), boost::asio::detached);
		break;
	default:
		break;
	}
}

void CBossSkill_Big08::OnActivate(CBaseEntity* target)
{
	if (m_task_running)
		return;

	if (m_iSkillStatus != SKILL_STATUS_READY)
		return;

	m_iSkillStatus = SKILL_STATUS_USING;
	std::vector<BossData> skill;
	for (auto iter = g_BossData.begin(); iter != g_BossData.end(); iter++)
	{
		if (iter->id != m_pMonster->m_iSkillId)
			continue;

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

			float dis = (m_pMonster->m_hTargetEnt->pev->origin - m_pMonster->pev->origin).Length();
			if (dis < iter->startDistMin || dis > iter->startDistMax)
				continue;

			if (iter->damageDegree)
			{
				float c = cos(90.0 * 0.5 * M_PI / 180.0);

				if (!m_pMonster->AngularCheck(m_pMonster->m_hTargetEnt, m_pMonster, m_pMonster->m_hTargetEnt->pev->origin, iter->damageDegree))
					continue;
			}

		}

		auto it = m_MapSkillTime.find(iter->raw);
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
		if (iter->type == 10)
		{
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
	m_flForceAttackTime = gpGlobals->time + useskill.forceAttackTime;

}


boost::asio::awaitable<void> CBossSkill_Big08::ZBS_ATTACK_BITE()
{
	m_pMonster->pev->movetype = MOVETYPE_NONE;
	m_pMonster->SetEntAnim(m_pMonster->LookupSequence(m_CurSkillData.aniSeq.c_str()), 1.0, m_CurSkillData.aniTime, false);
	m_pMonster->m_flStop = gpGlobals->time + m_CurSkillData.aniTime;

	co_await async_wait_timer(m_CurSkillData.damageDelay);

	EMIT_SOUND(m_pMonster->edict(), CHAN_BODY, "zombi/dione/zbs_attack_bite.wav", VOL_NORM, ATTN_NORM);
	m_pMonster->RadiusDamage(m_pMonster->pev->origin, m_CurSkillData.damage, m_CurSkillData.damageDegree, m_CurSkillData.damageDist, m_CurSkillData.damageHeight, DMG_SLASH, m_CurSkillData.kickSide, m_CurSkillData.kickUp, m_CurSkillData.rushPower, false);
	co_await async_wait_timer(m_CurSkillData.aniTime - m_CurSkillData.damageDelay);

	m_pMonster->pev->movetype = MOVETYPE_STEP;
	m_iSkillStatus = SKILL_STATUS_FREEZING;
	m_flTimeSkillNext = gpGlobals->time + 0.1s;
}

boost::asio::awaitable<void> CBossSkill_Big08::ZBS_ATTACK_SWING()
{
	m_pMonster->pev->movetype = MOVETYPE_NONE;
	m_pMonster->SetEntAnim(m_pMonster->LookupSequence(m_CurSkillData.aniSeq.c_str()), 1.0, m_CurSkillData.aniTime, false);
	m_pMonster->m_flStop = gpGlobals->time + m_CurSkillData.aniTime;

	co_await async_wait_timer(m_CurSkillData.damageDelay);

	EMIT_SOUND(m_pMonster->edict(), CHAN_BODY, "zombi/dione/zbs_attack_swing.wav", VOL_NORM, ATTN_NORM);
	m_pMonster->RadiusDamage(m_pMonster->pev->origin, m_CurSkillData.damage, m_CurSkillData.damageDegree, m_CurSkillData.damageDist, m_CurSkillData.damageHeight, DMG_SLASH, m_CurSkillData.kickSide, m_CurSkillData.kickUp, m_CurSkillData.rushPower, false);
	co_await async_wait_timer(m_CurSkillData.aniTime - m_CurSkillData.damageDelay);

	m_pMonster->pev->movetype = MOVETYPE_STEP;
	m_iSkillStatus = SKILL_STATUS_FREEZING;
	m_flTimeSkillNext = gpGlobals->time + 0.1s;
}

boost::asio::awaitable<void> CBossSkill_Big08::ZBS_ATTACK_TENTACLE1()
{
	m_pMonster->pev->movetype = MOVETYPE_NONE;
	m_pMonster->SetEntAnim(m_pMonster->LookupSequence(m_CurSkillData.aniSeq.c_str()), 1.0, m_CurSkillData.aniTime, false);
	m_pMonster->m_flStop = gpGlobals->time + m_CurSkillData.aniTime;
	m_iTentacleSkillCount = 3;

	MESSAGE_BEGIN(MSG_ALL, gmsgZBSMsg);
	WRITE_BYTE(0);
	WRITE_SHORT(std::atoi(m_CurSkillData.startMsg.c_str()));
	MESSAGE_END();

	co_await async_wait_timer(m_CurSkillData.damageDelay);

	EMIT_SOUND(m_pMonster->edict(), CHAN_BODY, "zombi/dione/zbs_attack_tentacle1.wav", VOL_NORM, ATTN_NORM);
	
	while (m_iTentacleSkillCount--)
	{
		CreateSpecialTentacle();

		co_await async_wait_timer(1.2s);
	}

	co_await async_wait_timer(m_CurSkillData.aniTime - m_CurSkillData.damageDelay - 1.2s * float(m_iTentacleSkillCount));

	m_pMonster->pev->movetype = MOVETYPE_STEP;
	m_iSkillStatus = SKILL_STATUS_FREEZING;
	m_flTimeSkillNext = gpGlobals->time + 0.1s;
}

CZBSBig08Tentacle* CBossSkill_Big08::CreateTentacle(Vector vecOrigin)
{
	CZBSBig08Tentacle* pEnt = static_cast<CZBSBig08Tentacle*>(CBaseEntity::Create("zbs_big08_tentacle", vecOrigin, g_vecZero, m_pMonster->edict()));
	if (pEnt)
	{
		pEnt->m_flDamage = m_CurSkillData.damage;
		pEnt->m_flRadius = 200;
	}
	return pEnt;
}

void CBossSkill_Big08::CreateSpecialTentacle()
{
	CMod_ZombieScenario* pModZBS = dynamic_cast<CMod_ZombieScenario*>(g_pModRunning);

	if (pModZBS)
	{
		int iCount = RANDOM_LONG(10, 15);
		Vector vecOrigin;
		float dis;
		int iDegree;
		if (pModZBS->m_vecPlayerSpawnOrigin.empty())
			return;

		for (auto iter = pModZBS->m_vecPlayerSpawnOrigin.begin(); iter != pModZBS->m_vecPlayerSpawnOrigin.end(); iter++)
		{
			vecOrigin = (*iter);
			dis = RANDOM_FLOAT(-200, 200);
			iDegree = RANDOM_LONG(0, 360);
			vecOrigin[0] += dis * cos(float(iDegree) * M_PI / 180.0);
			vecOrigin[1] += dis * sin(float(iDegree) * M_PI / 180.0);

			vecOrigin = TraceFloor(vecOrigin);

			vecOrigin.z += 8;

			CZBSBig08Tentacle* pEnt = CreateTentacle(vecOrigin);

		}

		CBaseEntity* pPlayer = NULL;
		while ((pPlayer = UTIL_FindEntityByClassname(pPlayer, "player")) != NULL)
		{
			vecOrigin = pPlayer->pev->origin;
			dis = RANDOM_FLOAT(-50.0, 50.0);
			iDegree = RANDOM_LONG(0, 360);
			vecOrigin[0] += dis * cos(float(iDegree) * M_PI / 180.0);
			vecOrigin[1] += dis * sin(float(iDegree) * M_PI / 180.0);

			vecOrigin = TraceFloor(vecOrigin);

			vecOrigin.z += 8;

			CZBSBig08Tentacle* pEnt = CreateTentacle(vecOrigin);

		}
	}
}

Vector CBossSkill_Big08::TraceFloor(Vector vecStart)
{
	Vector vecOrigin = vecStart;
	vecOrigin.z -= 1024.0;

	TraceResult tr;
	UTIL_TraceLine(vecStart, vecOrigin, ignore_monsters, m_pMonster->edict(), &tr);

	return tr.vecEndPos;
}

boost::asio::awaitable<void> CBossSkill_Big08::ZBS_ATTACK_TENTACLE2()
{
	m_pMonster->pev->movetype = MOVETYPE_NONE;
	m_pMonster->SetEntAnim(m_pMonster->LookupSequence(m_CurSkillData.aniSeq.c_str()), 1.0, m_CurSkillData.aniTime, false);
	m_pMonster->m_flStop = gpGlobals->time + m_CurSkillData.aniTime;
	m_iTentacleSkillCount = 3;

	MESSAGE_BEGIN(MSG_ALL, gmsgZBSMsg);
	WRITE_BYTE(0);
	WRITE_SHORT(std::atoi(m_CurSkillData.startMsg.c_str()));
	MESSAGE_END();

	co_await async_wait_timer(m_CurSkillData.damageDelay);

	EMIT_SOUND(m_pMonster->edict(), CHAN_BODY, "zombi/dione/zbs_attack_tentacle1.wav", VOL_NORM, ATTN_NORM);

	while (m_iTentacleSkillCount--)
	{
		CreateSpecialTentacle();

		co_await async_wait_timer(1.2s);
	}

	co_await async_wait_timer(m_CurSkillData.aniTime - m_CurSkillData.damageDelay - 1.2s * float(m_iTentacleSkillCount));

	m_pMonster->pev->movetype = MOVETYPE_STEP;
	m_iSkillStatus = SKILL_STATUS_FREEZING;
	m_flTimeSkillNext = gpGlobals->time + 0.1s;
}

void CBossSkill_Big08::GetPositon(float forward, float right, float up, Vector& start)
{
	Vector vecForward, vecRight, vecUp, vecAngle;
	vecAngle = m_pMonster->pev->angles;
	vecAngle.x = 0.0;
	UTIL_MakeVectors(vecAngle);
	vecForward = gpGlobals->v_forward;
	vecRight = gpGlobals->v_right;
	vecUp = gpGlobals->v_up;

	start = m_pMonster->pev->origin + forward * vecForward + right * vecRight + up * vecUp;

}

boost::asio::awaitable<void> CBossSkill_Big08::ZBS_ATTACK_POISON1()
{
	m_pMonster->pev->movetype = MOVETYPE_NONE;
	m_pMonster->SetEntAnim(m_pMonster->LookupSequence(m_CurSkillData.aniSeq.c_str()), 1.0, m_CurSkillData.aniTime, false);
	m_pMonster->m_flStop = gpGlobals->time + m_CurSkillData.aniTime;

	MESSAGE_BEGIN(MSG_ALL, gmsgZBSMsg);
	WRITE_BYTE(0);
	WRITE_SHORT(std::atoi(m_CurSkillData.startMsg.c_str()));
	MESSAGE_END();

	co_await async_wait_timer(m_CurSkillData.damageDelay);

	Vector vecOrigin, vecOrigin2;
	Vector vecDir, vecVelocity;

	float flDist = 600.0;
	float flROffset = -150.0;
	float flUOffset = -50.0;
	if (m_pMonster->m_hTargetEnt)
		flDist = max(600.0, (m_pMonster->m_hTargetEnt->pev->origin - m_pMonster->pev->origin).Length());

	g_engfuncs.pfnGetAttachment(m_pMonster->edict(), 0, vecOrigin, vecOrigin2);

	for (int i = 0; i < 6; i++)
	{
		GetPositon(flDist, flROffset, flUOffset, vecOrigin2);

		vecVelocity = UTIL_GetSpeedVector(vecOrigin, vecOrigin2, 1000.0);

		CZBSBig08Hurricane* pEnt = CreatePoisonBall(vecOrigin);
		if (pEnt)
		{
			float gravity = RANDOM_FLOAT(0.5, 3.0);
			pEnt->pev->gravity = gravity;
			pEnt->pev->velocity = vecVelocity;
			UTIL_VecToAngles(vecVelocity);
			pEnt->pev->angles = vecVelocity;
		}

		flROffset += 50.0;
		flUOffset += 20.0;
	}

	EMIT_SOUND(m_pMonster->edict(), CHAN_BODY, "zombi/dione/zbs_attack_poison1.wav", VOL_NORM, ATTN_NORM);
	co_await async_wait_timer(m_CurSkillData.aniTime - m_CurSkillData.damageDelay);

	m_pMonster->pev->movetype = MOVETYPE_STEP;
	m_iSkillStatus = SKILL_STATUS_FREEZING;
	m_flTimeSkillNext = gpGlobals->time + 0.1s;
}

CZBSBig08Hurricane* CBossSkill_Big08::CreatePoisonBall(Vector vecOrigin)
{
	CZBSBig08Hurricane* pEnt = static_cast<CZBSBig08Hurricane*>(CBaseEntity::Create("zbs_big08_poisonball", vecOrigin, g_vecZero, m_pMonster->edict()));
	if (pEnt)
	{
		pEnt->m_flDamage = m_CurSkillData.damage;
		pEnt->m_flRadius = 200;
	}
	return pEnt;
}

boost::asio::awaitable<void> CBossSkill_Big08::ZBS_ATTACK_POISON2()
{
	m_pMonster->pev->movetype = MOVETYPE_NONE;
	m_pMonster->SetEntAnim(m_pMonster->LookupSequence(m_CurSkillData.aniSeq.c_str()), 1.0, m_CurSkillData.aniTime, false);
	m_pMonster->m_flStop = gpGlobals->time + m_CurSkillData.aniTime;

	MESSAGE_BEGIN(MSG_ALL, gmsgZBSMsg);
	WRITE_BYTE(0);
	WRITE_SHORT(std::atoi(m_CurSkillData.startMsg.c_str()));
	MESSAGE_END();

	co_await async_wait_timer(m_CurSkillData.damageDelay);

	EMIT_SOUND(m_pMonster->edict(), CHAN_BODY, "zombi/dione/zbs_attack_poison2.wav", VOL_NORM, ATTN_NORM);
	m_pMonster->RadiusDamage(m_pMonster->pev->origin, m_CurSkillData.damage, m_CurSkillData.damageDegree, m_CurSkillData.damageDist, m_CurSkillData.damageHeight, DMG_SLASH, m_CurSkillData.kickSide, m_CurSkillData.kickUp, m_CurSkillData.rushPower, false);
	co_await async_wait_timer(m_CurSkillData.aniTime - m_CurSkillData.damageDelay);

	m_pMonster->pev->movetype = MOVETYPE_STEP;
	m_iSkillStatus = SKILL_STATUS_FREEZING;
	m_flTimeSkillNext = gpGlobals->time + 0.1s;
}

boost::asio::awaitable<void> CBossSkill_Big08::ZBS_ATTACK_ROLLING()
{
	m_pMonster->pev->movetype = MOVETYPE_NONE;
	m_pMonster->SetEntAnim(m_pMonster->LookupSequence(m_CurSkillData.aniSeq.c_str()), 1.0, m_CurSkillData.aniTime, false);
	m_pMonster->m_flStop = gpGlobals->time + m_CurSkillData.aniTime;

	MESSAGE_BEGIN(MSG_ALL, gmsgZBSMsg);
	WRITE_BYTE(0);
	WRITE_SHORT(std::atoi(m_CurSkillData.startMsg.c_str()));
	MESSAGE_END();

	co_await async_wait_timer(m_CurSkillData.damageDelay);

	EMIT_SOUND(m_pMonster->edict(), CHAN_BODY, "zombi/dione/zbs_rolling1.wav", VOL_NORM, ATTN_NORM);
	m_pMonster->RadiusDamage(m_pMonster->pev->origin, m_CurSkillData.damage, m_CurSkillData.damageDegree, m_CurSkillData.damageDist, m_CurSkillData.damageHeight, DMG_SLASH, m_CurSkillData.kickSide, m_CurSkillData.kickUp, m_CurSkillData.rushPower, false);
	co_await async_wait_timer(m_CurSkillData.aniTime - m_CurSkillData.damageDelay);

	m_pMonster->pev->movetype = MOVETYPE_STEP;
	m_iSkillStatus = SKILL_STATUS_FREEZING;
	m_flTimeSkillNext = gpGlobals->time + 0.1s;
}

boost::asio::awaitable<void> CBossSkill_Big08::TASK_ZBS_RUN()
{
	m_pMonster->pev->takedamage = DAMAGE_NO;
	Vector vecOrigin = g_vecZero, vecAngle = g_vecZero;
	CBaseEntity* ent = nullptr;
	if (!strcmp(STRING(gpGlobals->mapname), "zs_trace"))
	{
		while ((ent = UTIL_FindEntityByClassname(ent, "zombiespawn")) != nullptr)
		{
			CZombieSpawn* pAct = static_cast<CZombieSpawn*>(ent);
			if (pAct->m_iId == 1000)
			{
				vecOrigin = pAct->pev->origin;
				vecAngle = pAct->pev->angles;
				m_pMonster->m_flTargetChange = gpGlobals->time + 10s;

				break;
			}
		}
	}

	if (!vecOrigin.IsZero())
	{
		m_pMonster->pev->origin = vecOrigin;
		m_pMonster->pev->angles = vecAngle;
	}
	m_pMonster->pev->movetype = MOVETYPE_NONE;
	m_pMonster->SetEntAnim(m_pMonster->LookupSequence(m_CurSkillData.aniSeq.c_str()), 1.0, m_CurSkillData.aniTime, false);
	m_pMonster->m_flStop = gpGlobals->time + m_CurSkillData.aniTime + 1.5s;

	co_await async_wait_timer(m_CurSkillData.aniTime);

	m_pMonster->pev->movetype = MOVETYPE_TOSS;
	UTIL_MakeVectors(m_pMonster->pev->angles);
	Vector vecVelocity = gpGlobals->v_forward;
	vecVelocity *= 800.0;
	vecVelocity.z += 500;
	m_pMonster->pev->velocity = vecVelocity;
	m_pMonster->SetEntAnim(m_pMonster->LookupSequence("appear2"), 1.0, 1.5s, false);
	m_pMonster->m_flStop = gpGlobals->time + 1.5s;

	while (m_pMonster->m_flStop > gpGlobals->time)
	{
		if (m_pMonster->pev->flags & FL_ONGROUND)
			break;

		co_await async_wait_timer(0.1s);
	}

	m_pMonster->pev->movetype = MOVETYPE_NONE;
	m_pMonster->SetEntAnim(m_pMonster->LookupSequence("appear3"), 1.0, 8.7s, false);

	EMIT_SOUND(m_pMonster->edict(), CHAN_BODY, "zombi/dione/appear3.wav", VOL_NORM, ATTN_NORM);

	m_pMonster->m_flStop = gpGlobals->time + 8.7s;
	co_await async_wait_timer(8.7s);

	m_pMonster->pev->movetype = MOVETYPE_STEP;
	m_iSkillStatus = SKILL_STATUS_FREEZING;
	m_flTimeSkillNext = gpGlobals->time + 0.1s;
}

boost::asio::awaitable<void> CBossSkill_Big08::TASK_ZBS_APPEAR()
{
	m_pMonster->pev->takedamage = DAMAGE_NO;
	Vector vecOrigin = g_vecZero, vecAngle = g_vecZero;
	CBaseEntity* ent = nullptr;
	if (!strcmp(STRING(gpGlobals->mapname), "zs_trace"))
	{
		while ((ent = UTIL_FindEntityByClassname(ent, "zombiespawn")) != nullptr)
		{
			CZombieSpawn* pAct = static_cast<CZombieSpawn*>(ent);
			if (pAct->m_iId == 1000)
			{
				vecOrigin = pAct->pev->origin;
				vecAngle = pAct->pev->angles;
				m_pMonster->m_flTargetChange = gpGlobals->time + 10s;
				
				break;
			}
		}
	}

	if (!vecOrigin.IsZero())
	{
		m_pMonster->pev->origin = vecOrigin;
		m_pMonster->pev->angles = vecAngle;
	}
	m_pMonster->pev->movetype = MOVETYPE_NONE;
	m_pMonster->SetEntAnim(m_pMonster->LookupSequence(m_CurSkillData.aniSeq.c_str()), 1.0, m_CurSkillData.aniTime, false);
	m_pMonster->m_flStop = gpGlobals->time + m_CurSkillData.aniTime + 5.0s;

	co_await async_wait_timer(m_CurSkillData.aniTime);

	m_pMonster->pev->movetype = MOVETYPE_TOSS;
	UTIL_MakeVectors(m_pMonster->pev->angles);
	Vector vecVelocity = gpGlobals->v_forward;
	vecVelocity *= 800.0;
	vecVelocity.z += 500;
	m_pMonster->pev->velocity = vecVelocity;
	m_pMonster->SetEntAnim(m_pMonster->LookupSequence("appear2"), 1.0, 1.5s, false);
	m_pMonster->m_flStop = gpGlobals->time + 1.5s;

	while (m_pMonster->m_flStop > gpGlobals->time)
	{
		if (m_pMonster->pev->flags & FL_ONGROUND)
			break;

		co_await async_wait_timer(0.1s);
	}

	m_pMonster->pev->movetype = MOVETYPE_NONE;
	m_pMonster->SetEntAnim(m_pMonster->LookupSequence("appear3"), 1.0, 8.7s, false);

	EMIT_SOUND(m_pMonster->edict(), CHAN_BODY, "zombi/dione/appear3.wav", VOL_NORM, ATTN_NORM);

	m_pMonster->m_flStop = gpGlobals->time + 8.7s;
	co_await async_wait_timer(8.7s);

	m_pMonster->pev->movetype = MOVETYPE_STEP;
	m_iSkillStatus = SKILL_STATUS_FREEZING;
	m_flTimeSkillNext = gpGlobals->time + 0.1s;
}

void CBossSkill_Big08::OnTouch(CBaseEntity* other)
{
	if (m_pMonster->pev->movetype != MOVETYPE_PUSHSTEP)
		return;

	if (other->IsPlayer())
	{
		Vector2D vecKickRateDir = (other->pev->origin.Make2D() - m_pMonster->pev->origin.Make2D()).Normalize();

		other->pev->velocity.x = vecKickRateDir.x * m_CurSkillData.rushPower;
		other->pev->velocity.y = vecKickRateDir.y * m_CurSkillData.rushPower;
		other->pev->velocity.z += 150.0;

		other->TakeDamage(m_pMonster->pev, m_pMonster->pev, m_CurSkillData.damage, DMG_SLASH);
	}
}

void CBossClass_Big08::OnSpawn()
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

void CBossClass_Big08::OnInit()
{
	m_bAnimDeath = false;
	PRECACHE_MODEL("models/monster/zbs_bossl_big08.mdl");

	PRECACHE_SOUND("zombi/scorpion/idle1.wav");
	PRECACHE_SOUND("zombi/scorpion/death.wav");
	PRECACHE_SOUND("zombi/scorpion/appear1.wav");
	PRECACHE_SOUND("zombi/scorpion/attack1.wav");
	PRECACHE_SOUND("zombi/scorpion/attack2.wav");
	PRECACHE_SOUND("zombi/scorpion/attack3.wav");
	PRECACHE_SOUND("zombi/scorpion/zbs_attack_storm_end.wav");
	PRECACHE_SOUND("zombi/scorpion/zbs_attack_storm1_1.wav");
	PRECACHE_SOUND("zombi/scorpion/zbs_attack_storm1_2.wav");
	PRECACHE_SOUND("zombi/scorpion/zbs_attack_storm2_1.wav");
	PRECACHE_SOUND("zombi/scorpion/zbs_attack_tentacle1_1.wav");
	PRECACHE_SOUND("zombi/scorpion/zbs_attack_tentacle1_2.wav");
	PRECACHE_SOUND("zombi/scorpion/zbs_attack_tentacle2_1.wav");
	PRECACHE_SOUND("zombi/scorpion/zbs_dash_start.wav");
	PRECACHE_SOUND("zombi/scorpion/zbs_dash_end.wav");
	PRECACHE_SOUND("zombi/scorpion/zbs_footstep1.wav");
	PRECACHE_SOUND("zombi/scorpion/zbs_footstep2.wav");
	PRECACHE_SOUND("zombi/scorpion/zbs_guard_start.wav");
	PRECACHE_SOUND("zombi/scorpion/zbs_guard_loop.wav");
	PRECACHE_SOUND("zombi/scorpion/zbs_guard_end.wav");
	PRECACHE_SOUND("zombi/scorpion/zbs_guard_broken.wav");
	PRECACHE_SOUND("zombi/scorpion/windstorm.wav");
	PRECACHE_SOUND("zombi/scorpion/sandstorm.wav");

	SET_MODEL(m_pMonster->edict(), "models/monster/zbs_bossl_big08.mdl");
	UTIL_SetSize(m_pMonster->pev, VEC_HULL_MIN, VEC_HULL_MAX);
	m_pMonster->SetSkill(std::make_unique<CBossSkill_Big08>(m_pMonster));
	m_pMonster->SetAble(JUMP, false);
	m_pMonster->m_turnTolerance = 400.0f;
}

void CBossClass_Big08::OnKilled(entvars_t* pKiller, int iGib)
{
	m_pMonster->pev->nextthink = gpGlobals->time + 10s;
	m_pMonster->pev->sequence = 20;
	m_pMonster->pev->frame = 0;
	m_pMonster->ResetSequenceInfo();
}

}
