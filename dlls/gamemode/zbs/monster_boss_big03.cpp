#include <random>
#include "monster_boss_big03.h"

namespace sv {

extern std::vector<BossData> g_BossData;

LINK_ENTITY_TO_CLASS(zbs_big03_poisonball, CZBSBig03PoisonBall);


void CZBSBig03PoisonBall::Spawn(void)
{
	Precache();

	pev->movetype = MOVETYPE_TOSS;
	pev->solid = SOLID_TRIGGER;

	pev->frame = 0;

	ResetSequenceInfo();

	SET_MODEL(ENT(pev), "models/ef_poison01.mdl");
	UTIL_SetSize(pev, { -0.1, -0.1, -0.1 }, { 0.1, 0.1, 0.1 });

	pev->nextthink = gpGlobals->time + 0.1s;
	m_flPoisonDamage = 50.0;

	SetThink(&CZBSBig03PoisonBall::PoisonThink);
	
}

void CZBSBig03PoisonBall::Precache(void)
{
	PRECACHE_MODEL("models/ef_poison01.mdl");
	m_iPoisonIndex[1] = PRECACHE_MODEL("sprites/poison.spr");
	m_iPoisonIndex[0] = PRECACHE_MODEL("models/ef_poison02.mdl");
}

void CZBSBig03PoisonBall::PoisonThink()
{
	SetTouch(&CZBSBig03PoisonBall::PoisonTouch);
	SetThink(&CZBSBig03PoisonBall::FollowThink);
	pev->nextthink = gpGlobals->time + 0.1s;
}

void CZBSBig03PoisonBall::FollowThink(void)
{
	pev->angles = UTIL_VecToAngles(pev->velocity.Normalize());
	pev->nextthink = gpGlobals->time + 0.1s;
}

void CZBSBig03PoisonBall::PoisonTouch(CBaseEntity* pOther)
{
	if (!pOther)
		return;

	if (pOther->edict() == pev->owner)
		return;

	if (!pOther->IsBSPModel())
		return;

	SetTouch(nullptr);
	SetThink(nullptr);

	Vector vecOrigin = pev->origin;
	Vector vecAngle = pev->angles;
	vecAngle.x = 0;

	UTIL_TempModel(vecOrigin, vecAngle, g_vecZero, m_iPoisonIndex[0], 40, 0, 10, true, 32, 255, kRenderTransAdd, NULL, false, 0, 0, 255, 0);

	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
	WRITE_BYTE(TE_TEMPSPRITE);
	WRITE_COORD(vecOrigin.x);
	WRITE_COORD(vecOrigin.y);
	WRITE_COORD(vecOrigin.z);
	WRITE_SHORT(m_iPoisonIndex[1]);
	WRITE_BYTE(20);
	WRITE_BYTE(255);
	WRITE_BYTE(15);
	MESSAGE_END();

	entvars_t* pevOwner = VARS(pev->owner);

	CBaseEntity* pEntity = NULL;
	while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, m_flRadius)) != NULL)
	{
		if (!pEntity->IsPlayer())
			continue;

		if (pEntity->pev->takedamage == DAMAGE_NO)
			continue;

		if (pEntity->pev == pevOwner)
			continue;

		int falloff = m_flDamage / m_flRadius;

		float flCurrentDamage = max(0, m_flDamage - (pev->origin - pEntity->pev->origin).Length() * falloff);

		pEntity->TakeDamage(pev, pevOwner, flCurrentDamage, DMG_NEVERGIB | DMG_BULLET);

		CBasePlayer* pPlayer = (CBasePlayer*)pEntity;

		pPlayer->m_iHealthDecreaseCount = 5;
		pPlayer->m_flHealthDecreaseAmount = m_flPoisonDamage;
		pPlayer->m_flHealthDecreaseInterval = 1.0s;
		
	}
	
	UTIL_Remove(this);

}

LINK_ENTITY_TO_CLASS(zbs_big03_tentacle, CZBSBig03Tentacle);


void CZBSBig03Tentacle::Spawn(void)
{
	Precache();

	pev->movetype = MOVETYPE_NONE;
	pev->solid = SOLID_NOT;
	pev->rendermode = kRenderNormal;
	pev->renderamt = 255.0;
	pev->frame = 0;
	pev->sequence = 0;

	ResetSequenceInfo();

	SET_MODEL(ENT(pev), "models/ef_tentacle_sign.mdl");
	UTIL_SetSize(pev, { -48, -48, -4 }, { 48, 48, 4 });

	pev->nextthink = gpGlobals->time + 1.0s;

	SetThink(&CZBSBig03Tentacle::TentacleThink);
	
}

void CZBSBig03Tentacle::Precache(void)
{
	m_iTentacleIndex[0] = PRECACHE_MODEL("models/tentacle.mdl");
	PRECACHE_MODEL("models/ef_tentacle_sign.mdl");
	m_iTentacleIndex[1] = PRECACHE_MODEL("models/ef_tentacle.mdl");

	PRECACHE_SOUND("zombi/dione/zbs_tentacle_pierce.wav");
}

void CZBSBig03Tentacle::TentacleThink()
{
	vecOrigin = pev->origin;
	vecOrigin.z += 4;

	pev->effects |= EF_NODRAW;

	EMIT_SOUND(edict(), CHAN_STATIC, "zombi/dione/zbs_tentacle_pierce.wav", VOL_NORM, ATTN_NORM);

	UTIL_TempModel(vecOrigin, g_vecZero, g_vecZero, m_iTentacleIndex[0], 10, 0, 10, false, 0, 255, kRenderNormal, NULL, false, 0, 0, 255, 0);

	SetTouch(&CZBSBig03Tentacle::TentacleTouch);

	pev->movetype = MOVETYPE_PUSHSTEP;
	pev->solid = SOLID_TRIGGER;
	pev->rendermode = kRenderNormal;
	pev->renderamt = 255.0;
	pev->frame = 0;

	ResetSequenceInfo();

	SetThink(&CZBSBig03Tentacle::RemoveThink);
	pev->nextthink = gpGlobals->time + 0.1s;
}

void CZBSBig03Tentacle::RemoveThink()
{
	SetTouch(nullptr);

	vecOrigin = pev->origin;
	UTIL_TempModel(vecOrigin, g_vecZero, g_vecZero, m_iTentacleIndex[1], 10, 0, 10, true, 32, 255, kRenderNormal, NULL, false, 0, 0, 255, 0);

	pev->nextthink = gpGlobals->time + 0.9s;
	SetThink(&CBaseEntity::SUB_Remove);
}

void CZBSBig03Tentacle::TentacleTouch(CBaseEntity* pOther)
{
	if (!pOther)
		return;

	if (!pOther->IsPlayer())
		return;

	SetTouch(nullptr);

	entvars_t* pevOwner = VARS(pev->owner);

	pOther->TakeDamage(pev, pevOwner, m_flDamage, DMG_NEVERGIB | DMG_BULLET);

	pOther->pev->velocity.z += 500.0;

}

void CBossSkill_Big03::OnThink()
{
	
	float flHealth = (m_pMonster->pev->health / m_pMonster->pev->max_health) * 100;
	if (flHealth <= 1)
		m_pMonster->pev->takedamage = DAMAGE_NO;

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

void CBossSkill_Big03::ActivateSkill(CBaseEntity* target)
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
	case 18:
		StartTask(ZBS_ATTACK_POISON1(), boost::asio::detached);
		break;
	case 19:
		StartTask(ZBS_ATTACK_ROLLING(), boost::asio::detached);
		break;
	default:
		break;
	}
}

void CBossSkill_Big03::OnActivate(CBaseEntity* target)
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


boost::asio::awaitable<void> CBossSkill_Big03::ZBS_ATTACK_BITE()
{
	m_pMonster->pev->movetype = MOVETYPE_NONE;
	m_pMonster->SetEntAnim(m_pMonster->LookupSequence(m_CurSkillData.aniSeq.c_str()), 1.0, m_CurSkillData.aniTime, false);
	m_pMonster->m_flStop = gpGlobals->time + m_CurSkillData.aniTime;

	co_await async_wait_timer(m_CurSkillData.damageDelay);

	EMIT_SOUND(m_pMonster->edict(), CHAN_BODY, "zombi/dione/zbs_attack_bite.wav", VOL_NORM, ATTN_NORM);
	m_pMonster->RadiusDamage(m_pMonster->pev->origin, m_CurSkillData.damage, m_CurSkillData.damageDegree, m_CurSkillData.damageDist, m_CurSkillData.damageHeight, DMG_SLASH, m_CurSkillData.kickSide, m_CurSkillData.kickUp, m_CurSkillData.rushPower, false, false, true);
	co_await async_wait_timer(m_CurSkillData.aniTime - m_CurSkillData.damageDelay);

	m_pMonster->pev->movetype = MOVETYPE_STEP;
	m_iSkillStatus = SKILL_STATUS_FREEZING;
	m_flTimeSkillNext = gpGlobals->time + 0.1s;
}

boost::asio::awaitable<void> CBossSkill_Big03::ZBS_ATTACK_SWING()
{
	m_pMonster->pev->movetype = MOVETYPE_NONE;
	m_pMonster->SetEntAnim(m_pMonster->LookupSequence(m_CurSkillData.aniSeq.c_str()), 1.0, m_CurSkillData.aniTime, false);
	m_pMonster->m_flStop = gpGlobals->time + m_CurSkillData.aniTime;

	co_await async_wait_timer(m_CurSkillData.damageDelay);

	EMIT_SOUND(m_pMonster->edict(), CHAN_AUTO, "zombi/dione/zbs_attack_swing.wav", VOL_NORM, ATTN_NORM);

	UTIL_TempModel({ 0, 0, 0 }, g_vecZero, g_vecZero, m_iSwingIndex, 10, 0, 10, true, 32, 255, kRenderTransAdd, m_pMonster, false, 0, 0, 255, 0);

	m_pMonster->RadiusDamage(m_pMonster->pev->origin, m_CurSkillData.damage, 360.0, m_CurSkillData.damageDist, m_CurSkillData.damageHeight, DMG_SLASH, m_CurSkillData.kickSide, m_CurSkillData.kickUp, m_CurSkillData.rushPower, false, false, true);
	co_await async_wait_timer(m_CurSkillData.aniTime - m_CurSkillData.damageDelay);

	m_pMonster->pev->movetype = MOVETYPE_STEP;
	m_iSkillStatus = SKILL_STATUS_FREEZING;
	m_flTimeSkillNext = gpGlobals->time + 0.1s;
}

boost::asio::awaitable<void> CBossSkill_Big03::ZBS_ATTACK_TENTACLE1()
{
	m_pMonster->pev->movetype = MOVETYPE_NONE;
	m_pMonster->SetEntAnim(m_pMonster->LookupSequence(m_CurSkillData.aniSeq.c_str()), 1.0, m_CurSkillData.aniTime, false);
	m_pMonster->m_flStop = gpGlobals->time + m_CurSkillData.aniTime;
	m_iTentacleSkillCount = 2;

	MESSAGE_BEGIN(MSG_ALL, gmsgZBSMsg);
	WRITE_BYTE(0);
	WRITE_SHORT(std::atoi(m_CurSkillData.startMsg.c_str()));
	MESSAGE_END();

	co_await async_wait_timer(m_CurSkillData.damageDelay);

	EMIT_SOUND(m_pMonster->edict(), CHAN_AUTO, "zombi/dione/zbs_attack_tentacle1.wav", VOL_NORM, ATTN_NORM);
	
	while (m_iTentacleSkillCount--)
	{
		CreateStraightTentacle();

		co_await async_wait_timer(2.3s);
	}

	co_await async_wait_timer(m_CurSkillData.aniTime - m_CurSkillData.damageDelay - 1.2s * float(m_iTentacleSkillCount));

	m_pMonster->pev->movetype = MOVETYPE_STEP;
	m_iSkillStatus = SKILL_STATUS_FREEZING;
	m_flTimeSkillNext = gpGlobals->time + 0.1s;
}

CZBSBig03Tentacle* CBossSkill_Big03::CreateTentacle(Vector vecOrigin)
{
	CZBSBig03Tentacle* pEnt = static_cast<CZBSBig03Tentacle*>(CBaseEntity::Create("zbs_big03_tentacle", vecOrigin, g_vecZero, m_pMonster->edict()));
	if (pEnt)
	{
		pEnt->m_flDamage = m_CurSkillData.damage;
		pEnt->m_flRadius = 200;
	}
	return pEnt;
}

void CBossSkill_Big03::CreateStraightTentacle()
{
	Vector vecOrigin, vecDir, vecLeftDir, vecRightDir;
	vecOrigin = g_vecZero;
	if (m_pMonster->m_hTargetEnt && m_pMonster->m_hTargetEnt->IsAlive())
	{
		vecOrigin = m_pMonster->m_hTargetEnt->pev->origin;

	}
	else
	{
		CBaseEntity* pPlayer = NULL;
		while ((pPlayer = UTIL_FindEntityByClassname(pPlayer, "player")) != NULL)
		{
			vecOrigin = pPlayer->pev->origin;
			break;
		}
	}

	vecDir = (vecOrigin - m_pMonster->pev->origin);
	vecDir.z = 0;
	vecDir = vecDir.Normalize();

	vRotationTransform(vecDir.x, vecDir.y, (30.0 * M_PI / 180.0), vecLeftDir.x, vecLeftDir.y);
	vRotationTransform(vecDir.x, vecDir.y, (-30.0 * M_PI / 180.0), vecRightDir.x, vecRightDir.y);

	float dis = 180.0;
	for (int i = 1; i < 6; i++)
	{
		vecOrigin = m_pMonster->pev->origin + vecDir * dis * i;
		vecOrigin.z += 32.0;
		vecOrigin = TraceFloor(vecOrigin);

		CZBSBig03Tentacle* pEnt = CreateTentacle(vecOrigin);
		if (pEnt)
		{
			pEnt->pev->nextthink = gpGlobals->time + 2.0s;
		}

		vecOrigin = m_pMonster->pev->origin + vecLeftDir * dis * i;
		vecOrigin.z += 32.0;
		vecOrigin = TraceFloor(vecOrigin);

		pEnt = CreateTentacle(vecOrigin);
		if (pEnt)
		{
			pEnt->pev->nextthink = gpGlobals->time + 2.0s;
		}

		vecOrigin = m_pMonster->pev->origin + vecRightDir * dis * i;
		vecOrigin.z += 32.0;
		vecOrigin = TraceFloor(vecOrigin);

		pEnt = CreateTentacle(vecOrigin);
		if (pEnt)
		{
			pEnt->pev->nextthink = gpGlobals->time + 2.0s;
		}
	}
}

void CBossSkill_Big03::vRotationTransform(float dx, float dy, float dAngle, float& dbNewX, float& dbNewY)
{
	dbNewX = dx * cos(dAngle) - dy * sin(dAngle); 
	dbNewY = dx * sin(dAngle) + dy * cos(dAngle);
}

void CBossSkill_Big03::CreateSpecialTentacle()
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

			CZBSBig03Tentacle* pEnt = CreateTentacle(vecOrigin);
			if (pEnt)
			{
				pEnt->pev->nextthink = gpGlobals->time + RANDOM_FLOAT(2.0, 3.0) * 1.0s;
			}

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

			CZBSBig03Tentacle* pEnt = CreateTentacle(vecOrigin);
			if (pEnt)
			{
				pEnt->pev->nextthink = gpGlobals->time + RANDOM_FLOAT(2.0, 3.0) * 1.0s;
			}

		}
	}
}

Vector CBossSkill_Big03::TraceFloor(Vector vecStart)
{
	Vector vecOrigin = vecStart;
	vecOrigin.z -= 1024.0;

	TraceResult tr;
	UTIL_TraceHull(vecStart, vecOrigin, ignore_monsters, human_hull, m_pMonster->edict(), &tr);

	return tr.vecEndPos;
}

boost::asio::awaitable<void> CBossSkill_Big03::ZBS_ATTACK_TENTACLE2()
{
	Q_strcpy(m_pMonster->m_szRunAnim, "zbs_run");
	m_pMonster->pev->maxspeed = 270.0;
	m_pMonster->pev->movetype = MOVETYPE_NONE;
	m_pMonster->SetEntAnim(m_pMonster->LookupSequence(m_CurSkillData.aniSeq.c_str()), 1.0, m_CurSkillData.aniTime, false);
	m_pMonster->m_flStop = gpGlobals->time + m_CurSkillData.aniTime;
	m_iTentacleSkillCount = 6;

	MESSAGE_BEGIN(MSG_ALL, gmsgZBSMsg);
	WRITE_BYTE(0);
	WRITE_SHORT(std::atoi(m_CurSkillData.startMsg.c_str()));
	MESSAGE_END();

	co_await async_wait_timer(m_CurSkillData.damageDelay);

	EMIT_SOUND(m_pMonster->edict(), CHAN_BODY, "zombi/dione/zbs_attack_tentacle2.wav", VOL_NORM, ATTN_NORM);

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

void CBossSkill_Big03::GetPositon(float forward, float right, float up, Vector& start)
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

boost::asio::awaitable<void> CBossSkill_Big03::ZBS_ATTACK_POISON1()
{
	m_pMonster->pev->movetype = MOVETYPE_NONE;
	m_pMonster->SetEntAnim(m_pMonster->LookupSequence(m_CurSkillData.aniSeq.c_str()), 1.0, m_CurSkillData.aniTime, false);
	m_pMonster->m_flStop = gpGlobals->time + m_CurSkillData.aniTime;

	MESSAGE_BEGIN(MSG_ALL, gmsgZBSMsg);
	WRITE_BYTE(0);
	WRITE_SHORT(std::atoi(m_CurSkillData.startMsg.c_str()));
	MESSAGE_END();

	co_await async_wait_timer(m_CurSkillData.damageDelay);

	float gravity = 1.0;
	for (int j = 0; j < 3; j++)
	{
		CreatePoisonBall(0.0, 0.4 + (gravity * j));

		for (int i = 1; i <= 3; i++)
		{
			CreatePoisonBall(i * 15.0, 0.3 + (gravity * j));
			CreatePoisonBall(-i * 15.0, 0.3 + (gravity * j));
		}
	}

	EMIT_SOUND(m_pMonster->edict(), CHAN_BODY, "zombi/dione/zbs_attack_poison1.wav", VOL_NORM, ATTN_NORM);
	co_await async_wait_timer(m_CurSkillData.aniTime - m_CurSkillData.damageDelay);

	m_pMonster->pev->movetype = MOVETYPE_STEP;
	m_iSkillStatus = SKILL_STATUS_FREEZING;
	m_flTimeSkillNext = gpGlobals->time + 0.1s;
}

CZBSBig03PoisonBall* CBossSkill_Big03::CreatePoisonBall(float fYaw, float gravity)
{
	UTIL_MakeVectors(m_pMonster->pev->angles);

	Vector vecOrigin, vecAngle;

	g_engfuncs.pfnGetAttachment(m_pMonster->edict(), 0, vecOrigin, vecAngle);

	float sinA = sin(fYaw * M_PI / 180);
	float cosA = cos(fYaw * M_PI / 180);
	Vector vecDir = gpGlobals->v_forward * cosA + gpGlobals->v_right * sinA;

	VectorAngles(vecDir, vecAngle);

	CZBSBig03PoisonBall* pEnt = static_cast<CZBSBig03PoisonBall*>(CBaseEntity::Create("zbs_big03_poisonball", vecOrigin, vecAngle, m_pMonster->edict()));
	if (pEnt)
	{
		pEnt->pev->velocity = vecDir * 1000.0;
		pEnt->pev->gravity = gravity;
		pEnt->m_flDamage = m_CurSkillData.damage;
		pEnt->m_flRadius = 150;
	}
	return pEnt;
}

boost::asio::awaitable<void> CBossSkill_Big03::ZBS_ATTACK_ROLLING()
{
	m_pMonster->pev->movetype = MOVETYPE_NONE;
	m_pMonster->SetEntAnim(m_pMonster->LookupSequence(m_CurSkillData.aniSeq.c_str()), 1.0, m_CurSkillData.aniTime, false);
	co_await async_wait_timer(m_CurSkillData.damageDelay);

	m_pMonster->pev->movetype = MOVETYPE_PUSHSTEP;
	m_pMonster->m_flStop = gpGlobals->time + 1.1s;
	m_pMonster->SetEntAnim(m_pMonster->LookupSequence("zbs_attack_rolling2"), 1.0, 1.0s, false);
	g_engfuncs.pfnWalkMove(m_pMonster->edict(), m_pMonster->pev->angles.y, 9.0, WALKMOVE_NORMAL);
	UTIL_MakeVectors(m_pMonster->pev->angles);
	Vector vecVelocity = gpGlobals->v_forward;
	vecVelocity *= 900.0;
	m_pMonster->pev->velocity = vecVelocity;

	EMIT_SOUND(m_pMonster->edict(), CHAN_BODY, "zombi/dione/zbs_rolling1.wav", VOL_NORM, ATTN_NORM);

	co_await async_wait_timer(1.0s);

	m_pMonster->pev->movetype = MOVETYPE_NONE;
	m_pMonster->SetEntAnim(m_pMonster->LookupSequence("zbs_attack_rolling3"), 1.0, 1.7s, false);
	m_pMonster->m_flStop = gpGlobals->time + 1.7s;
	co_await async_wait_timer(1.7s);

	m_pMonster->pev->movetype = MOVETYPE_STEP;
	m_iSkillStatus = SKILL_STATUS_FREEZING;
	m_flTimeSkillNext = gpGlobals->time + 0.1s;
}

boost::asio::awaitable<void> CBossSkill_Big03::TASK_ZBS_RUN()
{
	m_pMonster->pev->takedamage = DAMAGE_NO;
	Q_strcpy(m_pMonster->m_szRunAnim, "zbs_walk");
	
	Vector vecOrigin = g_vecZero, vecAngle = g_vecZero;
	CBaseEntity* ent = nullptr;
	if (!strcmp(STRING(gpGlobals->mapname), "zs_trace"))
	{
		while ((ent = UTIL_FindEntityByClassname(ent, "zombiespawn")) != nullptr)
		{
			CZombieSpawn* pAct = static_cast<CZombieSpawn*>(ent);
			if (pAct->m_iId == 1001)
			{
				vecOrigin = pAct->pev->origin;
				vecAngle = pAct->pev->angles;
				m_pMonster->m_improv->MoveTo(vecOrigin);
				break;
			}
		}
	}
	else if (!strcmp(STRING(gpGlobals->mapname), "zs_nightmare2"))
	{
		while ((ent = UTIL_FindEntityByClassname(ent, "boss_action")) != nullptr)
		{
			CBossAction* pAct = static_cast<CBossAction*>(ent);

			if (pAct->m_iId == 3000)
			{
				vecOrigin = pAct->pev->origin;
				vecAngle = pAct->pev->angles;
				m_pMonster->m_improv->MoveTo(vecOrigin);
				break;
			}
		}
	}

	MESSAGE_BEGIN(MSG_ALL, gmsgZBSMsg);
	WRITE_BYTE(0);
	WRITE_SHORT(std::atoi(m_CurSkillData.startMsg.c_str()));
	MESSAGE_END();

	if (!vecOrigin.IsZero())
	{
		m_pMonster->SetEntAnim(m_pMonster->LookupSequence(m_CurSkillData.aniSeq.c_str()), 1.0, m_CurSkillData.aniTime, false);

		while ((m_pMonster->pev->origin - vecOrigin).Length2D() >= 30)
		{
			m_pMonster->m_improv->MoveTo(vecOrigin);
			co_await async_wait_timer(0.1s);
		}
	}

	m_pMonster->m_flStop = gpGlobals->time + m_CurSkillData.aniTime;
	m_pMonster->pev->movetype = MOVETYPE_NONE;
	m_pMonster->SetEntAnim(m_pMonster->LookupSequence("exit1"), 1.0, 10.0s, false);

	EMIT_SOUND(m_pMonster->edict(), CHAN_BODY, "zombi/dione/exit1.wav", VOL_NORM, ATTN_NORM);

	co_await async_wait_timer(10.0s);

	MESSAGE_BEGIN(MSG_ALL, gmsgZBSMsg);
	WRITE_BYTE(0);
	WRITE_SHORT(265);
	MESSAGE_END();

	m_pMonster->SetEntAnim(m_pMonster->LookupSequence("zbs_attack_swing"), 1.0, 3.5s, false);
	m_pMonster->pev->movetype = MOVETYPE_NONE;

	co_await async_wait_timer(0.75s);

	EMIT_SOUND(m_pMonster->edict(), CHAN_BODY, "zombi/dione/zbs_attack_swing.wav", VOL_NORM, ATTN_NORM);

	UTIL_TempModel({ 0, 0, -16 }, g_vecZero, g_vecZero, m_iSwingIndex, 10, 0, 10, true, 32, 255, kRenderTransAdd, m_pMonster, false, 0, 0, 255, 0);

	m_pMonster->RadiusDamage(m_pMonster->pev->origin, m_CurSkillData.damage, 360.0, m_CurSkillData.damageDist, m_CurSkillData.damageHeight, DMG_SLASH, m_CurSkillData.kickSide, m_CurSkillData.kickUp, m_CurSkillData.rushPower, false, false, true);

	co_await async_wait_timer(2.75s);

	MESSAGE_BEGIN(MSG_ALL, gmsgZBSMsg);
	WRITE_BYTE(0);
	WRITE_SHORT(266);
	MESSAGE_END();

	m_pMonster->pev->angles = vecAngle;
	m_pMonster->SetEntAnim(m_pMonster->LookupSequence("exit2"), 1.0, 9.0s, false);
	m_pMonster->pev->movetype = MOVETYPE_NONE;

	co_await async_wait_timer(9.0s);

	if (!strcmp(STRING(gpGlobals->mapname), "zs_trace"))
	{
		while ((ent = UTIL_FindEntityByClassname(ent, "zombiespawn")) != nullptr)
		{
			CZombieSpawn* pAct = static_cast<CZombieSpawn*>(ent);
			if (pAct->m_iId == 1003)
			{
				vecOrigin = pAct->pev->origin;
				vecAngle = pAct->pev->angles;
				m_pMonster->m_improv->MoveTo(vecOrigin);
				break;
			}
		}
	}

	if (!vecOrigin.IsZero())
	{
		m_pMonster->pev->origin = vecOrigin;
		m_pMonster->pev->angles = vecAngle;
	}

	m_pMonster->Killed(NULL, 0);
}

boost::asio::awaitable<void> CBossSkill_Big03::TASK_ZBS_APPEAR()
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
				break;
			}
		}
	}

	if (!vecOrigin.IsZero())
	{
		m_pMonster->pev->origin = vecOrigin;

		DROP_TO_FLOOR(m_pMonster->edict());

		m_pMonster->pev->angles = vecAngle;
	}

	m_pMonster->pev->movetype = MOVETYPE_NONE;
	m_pMonster->m_flStop = gpGlobals->time + m_CurSkillData.aniTime + 2s;
	m_pMonster->SetEntAnim(m_pMonster->LookupSequence(m_CurSkillData.aniSeq.c_str()), 1.0, m_CurSkillData.aniTime, false);

	EMIT_SOUND(m_pMonster->edict(), CHAN_AUTO, "zombi/dione/appear1.wav", VOL_NORM, ATTN_NORM);

	m_flTimeSkillNext = gpGlobals->time + 15.0s;

	co_await async_wait_timer(m_CurSkillData.aniTime);

	m_pMonster->pev->movetype = MOVETYPE_TOSS;
	m_pMonster->pev->angles = vecAngle;
	UTIL_MakeVectors(m_pMonster->pev->angles);
	Vector vecVelocity = gpGlobals->v_forward;
	vecVelocity *= 500.0;
	vecVelocity.z += 250.0;
	m_pMonster->pev->velocity = vecVelocity;
	m_pMonster->SetEntAnim(m_pMonster->LookupSequence("appear2"), 1.0, 1.5s, false);

	m_pMonster->m_flStop = gpGlobals->time + 1.5s;
	m_flTimeSkillNext = gpGlobals->time + 15.0s;

	co_await async_wait_timer(0.2s);

	EMIT_SOUND(m_pMonster->edict(), CHAN_AUTO, "zombi/dione/appear2.wav", VOL_NORM, ATTN_NORM);

	while (m_pMonster->m_flStop > gpGlobals->time)
	{
		if (m_pMonster->pev->flags & FL_ONGROUND)
			break;

		co_await async_wait_timer(0.1s);
	}

	m_pMonster->pev->takedamage = DAMAGE_YES;
	m_pMonster->pev->movetype = MOVETYPE_NONE;
	m_pMonster->SetEntAnim(m_pMonster->LookupSequence("appear3"), 1.0, 8.7s, false);

	EMIT_SOUND(m_pMonster->edict(), CHAN_AUTO, "zombi/dione/appear3.wav", VOL_NORM, ATTN_NORM);

	m_pMonster->m_flStop = gpGlobals->time + 8.5s;
	co_await async_wait_timer(8.7s);

	m_pMonster->pev->movetype = MOVETYPE_STEP;
	m_iSkillStatus = SKILL_STATUS_FREEZING;
	m_flTimeSkillNext = gpGlobals->time + 0.1s;
}

void CBossSkill_Big03::OnTouch(CBaseEntity* other)
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

	PRECACHE_SOUND("zombi/dione/appear1.wav");
	PRECACHE_SOUND("zombi/dione/appear2.wav");
	PRECACHE_SOUND("zombi/dione/appear3.wav");
	PRECACHE_SOUND("zombi/dione/appear4.wav");
	PRECACHE_SOUND("zombi/dione/exit1.wav");
	PRECACHE_SOUND("zombi/dione/exit2.wav");
	PRECACHE_SOUND("zombi/dione/zbs_attack_bite.wav");
	PRECACHE_SOUND("zombi/dione/zbs_attack_poison1.wav");
	PRECACHE_SOUND("zombi/dione/zbs_attack_swing.wav");
	PRECACHE_SOUND("zombi/dione/zbs_attack_tentacle1.wav");
	PRECACHE_SOUND("zombi/dione/zbs_attack_tentacle2.wav");
	PRECACHE_SOUND("zombi/dione/zbs_poison_spit.wav");
	PRECACHE_SOUND("zombi/dione/zbs_rolling1.wav");
	PRECACHE_SOUND("zombi/dione/zbs_rolling2.wav");
	PRECACHE_SOUND("zombi/dione/zbs_rolling3.wav");

	Q_strcpy(m_pMonster->m_szRunAnim, "zbs_walk");

	SET_MODEL(m_pMonster->edict(), "models/monster/zbs_bossl_big03.mdl");
	UTIL_SetSize(m_pMonster->pev, VEC_HULL_MIN, VEC_HULL_MAX);
	m_pMonster->SetSkill(std::make_unique<CBossSkill_Big03>(m_pMonster));
	m_pMonster->SetAble(JUMP, false);
	m_pMonster->m_turnTolerance = 400.0f;
}

void CBossClass_Big03::OnKilled(entvars_t* pKiller, int iGib)
{
	m_pMonster->pev->nextthink = gpGlobals->time + 4.1s;
	m_pMonster->pev->sequence = 18;
	m_pMonster->pev->frame = 0;
	m_pMonster->ResetSequenceInfo();
}

}
