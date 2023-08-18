
#include "monster_boss_big05.h"


namespace sv {

extern std::vector<BossData> g_BossData;

LINK_ENTITY_TO_CLASS(spring_cooler, CSpringCooler);

void CSpringCooler::Spawn()
{
	m_iSmokeIndex = PRECACHE_MODEL("sprites/smoke_ia.spr");
	return CPointEntity::Spawn();
}

void CSpringCooler::Run()
{
	;
}

LINK_ENTITY_TO_CLASS(revenant_groggy, CRevenantGroggy);

void CRevenantGroggy::Spawn()
{
	return CBaseTrigger::Spawn();
}

void CRevenantGroggy::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "targetname"))
	{

	}
	else
		CBaseTrigger::KeyValue(pkvd);
}


void CRevenantGroggy::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	char szString[64];
	CBaseButton* pButton = static_cast<CBaseButton*>(pCaller);
	CBaseEntity* ent = nullptr;
	while ((ent = UTIL_FindEntityByClassname(ent, "monster_entity")) != nullptr)
	{
		CMonster* pBoss = static_cast<CMonster*>(ent);

		if (pBoss)
		{
			//How to call CBossSkill_Big05 ??? TODO
			/*auto p = (CMonsterModStrategy_Default*)pBoss->m_pMonsterStrategy.get();
			if (p)
			{
				p->m_pSkill == std::make_unique<CBossSkill_Big05>(pBoss);

				CBossSkill_Big05* pSkill = (CBossSkill_Big05*)(p->m_pSkill);
			}

			pBoss->m_pMonsterStrategy.get()

			if (pSkill)
			{
				if (!pSkill->m_iSpringCoolerIndex)
					return;

				if (pButton)
				{
					sprintf(szString, "boss_button%d", pSkill->m_iSpringCoolerIndex);
					if (strcmp(STRING(pButton->pev->targetname), szString))
						return;

					if (!pSkill->m_bSpringCoolerUsed)
					{
						pSkill->m_bSpringCoolerUsed = true;
						pSkill->SetSpringCooler(true);

						MESSAGE_BEGIN(MSG_ALL, gmsgZBSMsg);
						WRITE_BYTE(0);
						WRITE_SHORT(360);
						MESSAGE_END();

						pSkill->StopTask();
						pSkill->StartTask(pSkill->TASK_ZBS_GROGGY(), boost::asio::detached);

						MESSAGE_BEGIN(MSG_ALL, SVC_TEMPENTITY);
						WRITE_BYTE(TE_KILLENTITYATTACHMENTS);
						WRITE_SHORT(pBoss->entindex());
						MESSAGE_END();
					}
				}
			}*/
		}	
	}


}


LINK_ENTITY_TO_CLASS(zbs_big05_fireball, CZBSBig05FireBall);

void CZBSBig05FireBall::Spawn(void)
{
	Precache();

	m_iType = 0;
	pev->movetype = MOVETYPE_TOSS;
	pev->solid = SOLID_TRIGGER;
	pev->renderamt = 255;
	pev->frame = 0;

	ResetSequenceInfo();

	SET_MODEL(ENT(pev), "models/zombiegiant/fireball.mdl");
	UTIL_SetSize(pev, { -0.1, -0.1, -0.1 }, { 0.1, 0.1, 0.1 });

	SetThink(&CZBSBig05FireBall::IgniteThink);

	pev->nextthink = gpGlobals->time + 0.1s;

}

void CZBSBig05FireBall::Precache(void)
{
	m_iTrail = PRECACHE_MODEL("sprites/flame2.spr");

	PRECACHE_MODEL("models/zombiegiant/fireball.mdl");

}

void CZBSBig05FireBall::Launch()
{
	pev->aiment = nullptr;
	pev->movetype = MOVETYPE_FLY;
	pev->solid = SOLID_BBOX;

	SetThink(&CZBSBig05FireBall::IgniteThink);
	pev->nextthink = gpGlobals->time + 0.1s;
}

void CZBSBig05FireBall::PosThink()
{
	Vector vecOrigin, vecAngles;
	g_engfuncs.pfnGetAttachment(pev->owner, 1, vecOrigin, vecAngles);

	pev->origin = vecOrigin;

	pev->nextthink = gpGlobals->time + 0.02s;

}

void CZBSBig05FireBall::IgniteThink()
{
	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
	WRITE_BYTE(TE_BEAMPARTICLE); //defined as 32
	WRITE_SHORT(this->entindex());  // short (entity:attachment to follow)
	WRITE_SHORT(m_iTrail);// short (sprite index)
	WRITE_SHORT(m_iTrail);// short (sprite index)
	WRITE_SHORT(m_iTrail);// short (sprite index)
	WRITE_BYTE(255); //r
	WRITE_BYTE(255); //g
	WRITE_BYTE(255); //b
	WRITE_BYTE(0); //randomcolor
	WRITE_BYTE(140); //scale in 0.01's
	WRITE_BYTE(4); //freq(time of one cycle) in 0.01's
	WRITE_BYTE(3); //fadetime in 0.1s
	WRITE_BYTE(1);  //count
	WRITE_BYTE(0);  //random in org
	WRITE_BYTE(0);  //speed (if>0 do slow gravity)
	MESSAGE_END();

	SetTouch(&CZBSBig05FireBall::FireBallTouch);
	SetThink(&CZBSBig05FireBall::FollowThink);
	pev->nextthink = gpGlobals->time + 0.1s;
}

void CZBSBig05FireBall::FollowThink(void)
{
	pev->angles = UTIL_VecToAngles(pev->velocity.Normalize());
	pev->nextthink = gpGlobals->time + 0.1s;
}

void CZBSBig05FireBall::FireBallTouch(CBaseEntity* pOther)
{
	if (!pOther)
		return;

	if (pOther->edict() == pev->owner)
		return;

	if (pOther->pev->classname == pev->classname)
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

		UTIL_ScreenShakeOne(pEntity, 10, 3, 10);
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

void CBossSkill_Big05::OnThink()
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

void CBossSkill_Big05::ActivateSkill(CBaseEntity* target)
{
	m_iSkillStatus = SKILL_STATUS_FREEZING;
	m_flTimeSkillNext = m_flForceAttackTime + 15.0s;
	m_MapSkillTime[m_CurSkillData.raw] = gpGlobals->time + m_CurSkillData.cooltime;

	//使用技能
	switch (m_CurSkillData.type)
	{
	case 35:
		StartTask(TASK_ZBS_APPEAR(), boost::asio::detached);
		break;
	case 36:
		StartTask(TASK_ZBS_ATTACK1(), boost::asio::detached);
		break;
	case 37:
		StartTask(TASK_ZBS_ATTACK4(), boost::asio::detached);
		break;
	case 38:
		StartTask(TASK_ZBS_DASH(), boost::asio::detached);
		break;
	case 39:
		StartTask(TASK_ZBS_FIREBALL1(), boost::asio::detached);
		break;
	case 40:
		StartTask(TASK_ZBS_FIREBALL2(), boost::asio::detached);
		break;
	case 41:
		StartTask(TASK_ZBS_ATTACK5(), boost::asio::detached);
		break;
	case 42:
		StartTask(TASK_ZBS_BURNNING1(), boost::asio::detached);
		break;
	case 43:
		StartTask(TASK_ZBS_BURNNING2(), boost::asio::detached);
		break;
	default:
		break;
	}
}


void CBossSkill_Big05::OnActivate(CBaseEntity* target)
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

		if (iter->type == 41 && !m_bHellFireActivated)
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
	m_flForceAttackTime = gpGlobals->time + useskill.forceAttackTime;
}

boost::asio::awaitable<void> CBossSkill_Big05::TASK_ZBS_ATTACK1()
{
	m_pMonster->pev->movetype = MOVETYPE_NONE;
	m_pMonster->SetEntAnim(m_pMonster->LookupSequence(m_CurSkillData.aniSeq.c_str()), 1.0, m_CurSkillData.aniTime, false);
	m_pMonster->m_flStop = gpGlobals->time + m_CurSkillData.aniTime;
	
	co_await async_wait_timer(m_CurSkillData.damageDelay);

	EMIT_SOUND(m_pMonster->edict(), CHAN_BODY, "zombi/revenant/revenant_zbs_attack1.wav", VOL_NORM, ATTN_NORM);
	m_pMonster->RadiusDamage(m_pMonster->pev->origin, m_CurSkillData.damage, m_CurSkillData.damageDegree, m_CurSkillData.damageDist, m_CurSkillData.damageHeight, DMG_SLASH, m_CurSkillData.kickSide, m_CurSkillData.kickUp, m_CurSkillData.rushPower, false);
	co_await async_wait_timer(m_CurSkillData.aniTime - m_CurSkillData.damageDelay);

	m_pMonster->pev->movetype = MOVETYPE_STEP;
	m_iSkillStatus = SKILL_STATUS_FREEZING;
	m_flTimeSkillNext = gpGlobals->time + 0.1s;
}

boost::asio::awaitable<void> CBossSkill_Big05::TASK_ZBS_DASH()
{
	m_pMonster->pev->movetype = MOVETYPE_NONE;

	EMIT_SOUND(m_pMonster->edict(), CHAN_BODY, "zombi/revenant/revenant_zbs_ready.wav", VOL_NORM, ATTN_NORM);

	m_pMonster->SetEntAnim(m_pMonster->LookupSequence(m_CurSkillData.aniSeq.c_str()), 1.0, 1.5s, false);
	co_await async_wait_timer(1.5s);

	m_pMonster->pev->movetype = MOVETYPE_PUSHSTEP;
	m_pMonster->m_flStop = gpGlobals->time + 1.1s;
	m_pMonster->SetEntAnim(m_pMonster->LookupSequence("zbs_dash"), 1.0, 1.0s, false);
	g_engfuncs.pfnWalkMove(m_pMonster->edict(), m_pMonster->pev->angles.y, 10.0, WALKMOVE_NORMAL);
	UTIL_MakeVectors(m_pMonster->pev->angles);
	Vector vecVelocity = gpGlobals->v_forward;
	vecVelocity *= 1000.0;
	m_pMonster->pev->velocity = vecVelocity;
	
	co_await async_wait_timer(1.0s);

	m_pMonster->pev->movetype = MOVETYPE_NONE;
	m_pMonster->SetEntAnim(m_pMonster->LookupSequence("zbs_dash_end"), 1.0, 0.87s, false);
	m_pMonster->m_flStop = gpGlobals->time + 0.87s;
	co_await async_wait_timer(0.87s);

	m_pMonster->pev->movetype = MOVETYPE_STEP;
	m_iSkillStatus = SKILL_STATUS_FREEZING;
	m_flTimeSkillNext = gpGlobals->time + 0.1s;
}

void CBossSkill_Big05::GetPositon(float forward, float right, float up, Vector& start)
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

boost::asio::awaitable<void> CBossSkill_Big05::TASK_ZBS_FIREBALL1()
{
	m_pMonster->pev->movetype = MOVETYPE_NONE;
	m_pMonster->SetEntAnim(m_pMonster->LookupSequence(m_CurSkillData.aniSeq.c_str()), 1.0, m_CurSkillData.aniTime, false);
	co_await async_wait_timer(m_CurSkillData.damageDelay);	//0.6s

	//CreateFireBall
	EMIT_SOUND(m_pMonster->edict(), CHAN_BODY, "zombi/revenant/revenant_zbs_fireball1.wav", VOL_NORM, ATTN_NORM);

	Vector vecOrigin, vecAngles;
	Vector vecDir, vecVelocity;

	g_engfuncs.pfnGetAttachment(m_pMonster->edict(), 0, vecOrigin, vecAngles);

	CZBSBig05FireBall* pEntL = CreateFireBall(vecOrigin);
	if (pEntL)
	{
		pEntL->pev->solid = SOLID_NOT;
		pEntL->pev->movetype = MOVETYPE_NOCLIP;
		pEntL->SetThink(&CZBSBig05FireBall::PosThink);
		pEntL->pev->nextthink = gpGlobals->time + 0.1s;
	}

	g_engfuncs.pfnGetAttachment(m_pMonster->edict(), 1, vecOrigin, vecAngles);

	CZBSBig05FireBall* pEntR = CreateFireBall(vecOrigin);
	if (pEntR)
	{
		pEntR->pev->solid = SOLID_NOT;
		pEntR->pev->movetype = MOVETYPE_NOCLIP;
		pEntR->SetThink(&CZBSBig05FireBall::PosThink);
		pEntR->pev->nextthink = gpGlobals->time + 0.1s;
	}

	co_await async_wait_timer(2s - m_CurSkillData.damageDelay);

	//FireL	60 Frame
	
	if (pEntL)
	{
		if (m_pMonster->m_hTargetEnt)
		{
			vecDir = (m_pMonster->m_hTargetEnt->pev->origin - m_pMonster->pev->origin).Normalize();
			vecVelocity = vecDir * 1000.0;
		}
		else
		{
			GetPositon(500, 0, -30, vecDir);
			vecVelocity = UTIL_GetSpeedVector(m_pMonster->pev->origin, vecDir, 1000.0);
		}
		pEntL->Launch();
		pEntL->pev->velocity = vecVelocity;
	}


	co_await async_wait_timer(0.33s);
	
	//FireR	69 Frame
	if (pEntR)
	{
		if (m_pMonster->m_hTargetEnt)
		{
			vecDir = (m_pMonster->m_hTargetEnt->pev->origin - m_pMonster->pev->origin).Normalize();
			vecVelocity = vecDir * 1000.0;
		}
		else
		{
			GetPositon(500, 0, -30, vecDir);
			vecVelocity = UTIL_GetSpeedVector(m_pMonster->pev->origin, vecDir, 1000.0);
		}
		pEntR->Launch();
		pEntR->pev->velocity = vecVelocity;
	}

	m_pMonster->m_flStop = gpGlobals->time + 2s;
	co_await async_wait_timer(2s);

	m_pMonster->pev->movetype = MOVETYPE_STEP;
	m_iSkillStatus = SKILL_STATUS_FREEZING;
	m_flTimeSkillNext = gpGlobals->time + 0.1s;
}

boost::asio::awaitable<void> CBossSkill_Big05::TASK_ZBS_FIREBALL2()
{
	m_pMonster->pev->movetype = MOVETYPE_NONE;
	m_pMonster->SetEntAnim(m_pMonster->LookupSequence(m_CurSkillData.aniSeq.c_str()), 1.0, m_CurSkillData.aniTime, false);

	EMIT_SOUND(m_pMonster->edict(), CHAN_BODY, "zombi/revenant/revenant_zbs_fireball2.wav", VOL_NORM, ATTN_NORM);

	co_await async_wait_timer(m_CurSkillData.damageDelay);	//0.6s

	Vector vecOrigin, vecAngles;
	Vector vecDir, vecVelocity;

	g_engfuncs.pfnGetAttachment(m_pMonster->edict(), 0, vecOrigin, vecAngles);

	CZBSBig05FireBall* pEntL = CreateFireBall(vecOrigin);
	if (pEntL)
	{
		pEntL->pev->solid = SOLID_NOT;
		pEntL->pev->movetype = MOVETYPE_NOCLIP;
		pEntL->SetThink(&CZBSBig05FireBall::PosThink);
		pEntL->pev->nextthink = gpGlobals->time + 0.1s;

	}

	g_engfuncs.pfnGetAttachment(m_pMonster->edict(), 1, vecOrigin, vecAngles);

	CZBSBig05FireBall* pEntR = CreateFireBall(vecOrigin);
	if (pEntR)
	{
		pEntL->pev->solid = SOLID_NOT;
		pEntL->pev->movetype = MOVETYPE_NOCLIP;
		pEntR->SetThink(&CZBSBig05FireBall::PosThink);
		pEntR->pev->nextthink = gpGlobals->time + 0.1s;
	}

	co_await async_wait_timer(1.5s - m_CurSkillData.damageDelay);

	//FireL	45 Frame
	if (pEntL)
	{
		if (m_pMonster->m_hTargetEnt)
		{
			vecDir = (m_pMonster->m_hTargetEnt->pev->origin - m_pMonster->pev->origin).Normalize();
			vecVelocity = vecDir * 1000.0;
		}
		else
		{
			GetPositon(500, 0, -30, vecDir);
			vecVelocity = UTIL_GetSpeedVector(m_pMonster->pev->origin, vecDir, 1000.0);
		}
		pEntL->Launch();
		pEntL->pev->velocity = vecVelocity;
	}


	co_await async_wait_timer(0.4s);

	//FireR	57 Frame
	if (pEntR)
	{
		if (m_pMonster->m_hTargetEnt)
		{
			vecDir = (m_pMonster->m_hTargetEnt->pev->origin - m_pMonster->pev->origin).Normalize();
			vecVelocity = vecDir * 1000.0;
		}
		else
		{
			GetPositon(500, 0, -30, vecDir);
			vecVelocity = UTIL_GetSpeedVector(m_pMonster->pev->origin, vecDir, 1000.0);
		}
		pEntR->Launch();
		pEntR->pev->velocity = vecVelocity;
	}

	co_await async_wait_timer(0.4s);
	//FireL	69 Frame

	g_engfuncs.pfnGetAttachment(m_pMonster->edict(), 0, vecOrigin, vecAngles);

	CZBSBig05FireBall* pEntL2 = CreateFireBall(vecOrigin);
	if (pEntL2)
	{
		
		if (m_pMonster->m_hTargetEnt)
		{
			vecDir = (m_pMonster->m_hTargetEnt->pev->origin - m_pMonster->pev->origin).Normalize();
			vecVelocity = vecDir * 1000.0;
		}
		else
		{
			GetPositon(500, 0, -30, vecDir);
			vecVelocity = UTIL_GetSpeedVector(m_pMonster->pev->origin, vecDir, 1000.0);
		}
		pEntL2->pev->velocity = vecVelocity;
	}

	co_await async_wait_timer(0.4s);
	//FireR	81 Frame
	g_engfuncs.pfnGetAttachment(m_pMonster->edict(), 1, vecOrigin, vecAngles);

	CZBSBig05FireBall* pEntR2 = CreateFireBall(vecOrigin);
	if (pEntR2)
	{

		if (m_pMonster->m_hTargetEnt)
		{
			vecDir = (m_pMonster->m_hTargetEnt->pev->origin - m_pMonster->pev->origin).Normalize();
			vecVelocity = vecDir * 1000.0;
		}
		else
		{
			GetPositon(500, 0, -30, vecDir);
			vecVelocity = UTIL_GetSpeedVector(m_pMonster->pev->origin, vecDir, 1000.0);
		}
		pEntR2->pev->velocity = vecVelocity;
	}


	m_pMonster->m_flStop = gpGlobals->time + 3.6s;
	co_await async_wait_timer(3.6s);
	
	m_pMonster->pev->movetype = MOVETYPE_STEP;
	m_iSkillStatus = SKILL_STATUS_FREEZING;
	m_flTimeSkillNext = gpGlobals->time + 0.1s;
}

CZBSBig05FireBall* CBossSkill_Big05::CreateFireBall(Vector vecOrigin)
{
	CZBSBig05FireBall* pEnt = static_cast<CZBSBig05FireBall*>(CBaseEntity::Create("zbs_big05_fireball", vecOrigin, g_vecZero, m_pMonster->edict()));
	if (pEnt)
	{
		pEnt->m_flDamage = m_CurSkillData.damage;
		pEnt->m_flRadius = 200;
	}
	return pEnt;
}

boost::asio::awaitable<void> CBossSkill_Big05::TASK_ZBS_ATTACK5()
{
	m_bHellFireActivated = false;

	MESSAGE_BEGIN(MSG_ALL, gmsgZBSMsg);
	WRITE_BYTE(0);
	WRITE_SHORT(std::atoi(m_CurSkillData.startMsg.c_str()));
	MESSAGE_END();

	m_pMonster->pev->movetype = MOVETYPE_NONE;
	m_pMonster->SetEntAnim(m_pMonster->LookupSequence(m_CurSkillData.aniSeq.c_str()), 1.0, m_CurSkillData.aniTime, false);	//10.4s
	m_pMonster->m_flStop = gpGlobals->time + m_CurSkillData.aniTime;


	Vector vecOrigin, vecAngles;
	Vector vecDir, vecVelocity;

	co_await async_wait_timer(m_CurSkillData.damageDelay);	//0.6s

	vecOrigin = m_pMonster->pev->origin;

	vecOrigin[2] -= 16.0;

	UTIL_TempModel(vecOrigin, g_vecZero, g_vecZero, m_iSkyFlameIndex, 90, 0, 10, true, 32, 255, kRenderTransAdd, NULL, true, 64, 0, 255, 0);

	vecOrigin[2] += 206.0;

	UTIL_TempModel(vecOrigin, g_vecZero, { 0, 0, 150.0 }, m_iHellFireIndex, 34, 0, 10, false, 0, 255, kRenderTransAdd, NULL, false, 0, 0, 255, 0);

	//CreateHellFire
	EMIT_SOUND(m_pMonster->edict(), CHAN_BODY, "zombi/revenant/revenant_zbs_attack5.wav", VOL_NORM, ATTN_NORM);

	co_await async_wait_timer(3.4s);

	vecOrigin[2] += 800.0;
	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, vecOrigin);
	WRITE_BYTE(TE_EXPLOSION);
	WRITE_COORD(vecOrigin.x);
	WRITE_COORD(vecOrigin.y);
	WRITE_COORD(vecOrigin.z);
	WRITE_SHORT(MODEL_INDEX("sprites/eexplo.spr"));
	WRITE_BYTE(125);
	WRITE_BYTE(30);
	WRITE_BYTE(TE_EXPLFLAG_NONE);
	MESSAGE_END();


	time_point_t flHellFire = gpGlobals->time + 5s;
	while (flHellFire > gpGlobals->time)
	{
		CreateHellFire();

		co_await async_wait_timer(RANDOM_FLOAT(0.3, 0.5) * 1.0s);
	}

	co_await async_wait_timer(1.3s);

	m_pMonster->pev->movetype = MOVETYPE_STEP;
	m_iSkillStatus = SKILL_STATUS_FREEZING;
	m_flTimeSkillNext = gpGlobals->time + 0.1s;
}

void CBossSkill_Big05::CreateHellFire()
{
	CMod_ZombieScenario* pModZBS = dynamic_cast<CMod_ZombieScenario*>(g_pModRunning);

	if (pModZBS)
	{
		int iCount = RANDOM_LONG(10, 15);
		Vector vecOrigin;
		float dis;
		int iDegree;
		for (auto iter = pModZBS->m_vecPlayerSpawnOrigin.begin(); iter != pModZBS->m_vecPlayerSpawnOrigin.end(); iter++)
		{
			vecOrigin = (*iter);
			vecOrigin[2] += RANDOM_LONG(500, 600);
			dis = RANDOM_FLOAT(-200, 200);
			iDegree = RANDOM_LONG(0, 360);
			vecOrigin[0] += dis * cos(float(iDegree) * M_PI / 180.0);
			vecOrigin[1] += dis * cos(float(iDegree) * M_PI / 180.0);

			CZBSBig05FireBall* pEnt = CreateFireBall(vecOrigin);
			if (pEnt)
			{
				pEnt->pev->gravity = RANDOM_FLOAT(1.0, 2.0);
			}
		}

		CBaseEntity* pPlayer = NULL;
		while ((pPlayer = UTIL_FindEntityByClassname(pPlayer, "player")) != NULL)
		{
			vecOrigin = pPlayer->pev->origin;
			vecOrigin[2] += RANDOM_LONG(500, 600);
			dis = RANDOM_FLOAT(-100, 100);
			iDegree = RANDOM_LONG(0, 360);
			vecOrigin[0] += dis * cos(float(iDegree) * M_PI / 180.0);
			vecOrigin[1] += dis * cos(float(iDegree) * M_PI / 180.0);

			CZBSBig05FireBall* pEnt = CreateFireBall(vecOrigin);
			if (pEnt)
			{
				pEnt->pev->gravity = RANDOM_FLOAT(1.0, 2.0);
			}
		}
	}
}

boost::asio::awaitable<void> CBossSkill_Big05::TASK_ZBS_ATTACK4()
{
	m_pMonster->pev->movetype = MOVETYPE_NONE;
	m_pMonster->SetEntAnim(m_pMonster->LookupSequence(m_CurSkillData.aniSeq.c_str()), 1.0, m_CurSkillData.aniTime, false);
	m_pMonster->m_flStop = gpGlobals->time + m_CurSkillData.aniTime;

	co_await async_wait_timer(0.6s);

	//TODO
	EMIT_SOUND(m_pMonster->edict(), CHAN_AUTO, "zombi/revenant/revenant_zbs_attack4.wav", VOL_NORM, ATTN_NORM);

	m_pMonster->RadiusDamage(m_pMonster->pev->origin, m_CurSkillData.damage, 360.0, m_CurSkillData.damageDist, m_CurSkillData.damageHeight, DMG_SLASH, m_CurSkillData.kickSide, m_CurSkillData.kickUp, m_CurSkillData.rushPower, false, true);

	CreateRoundFire(m_pMonster->pev->origin, 80.0);

	co_await async_wait_timer(0.2s);

	CreateRoundFire(m_pMonster->pev->origin, 160.0);

	co_await async_wait_timer(0.2s);

	m_pMonster->RadiusDamage(m_pMonster->pev->origin, m_CurSkillData.damage, 360.0, m_CurSkillData.damageDist, m_CurSkillData.damageHeight, DMG_SLASH, m_CurSkillData.kickSide, m_CurSkillData.kickUp, m_CurSkillData.rushPower, false, true);

	CreateRoundFire(m_pMonster->pev->origin, 240.0);

	co_await async_wait_timer(m_CurSkillData.aniTime - 1.0s);

	m_pMonster->pev->movetype = MOVETYPE_STEP;
	m_iSkillStatus = SKILL_STATUS_FREEZING;
	m_flTimeSkillNext = gpGlobals->time + 0.1s;

}

void CBossSkill_Big05::CreateRoundFire(Vector vecOrigin, float dis)
{
	float x, y;
	int iStartPos = 0;
	Vector vecStart, vecOrigin2;
	vecStart[2] = vecOrigin[2] - 16.0;
	int amount = 10;

	for (int i = 0; i < amount; i++)
	{
		x = vecOrigin[0] + dis * cos(float(iStartPos) * M_PI / 180.0);
		y = vecOrigin[1] + dis * sin(float(iStartPos) * M_PI / 180.0);
		vecStart[0] = x;
		vecStart[1] = y;

		iStartPos += 360 / amount;

		MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, vecStart);
		WRITE_BYTE(TE_EXPLOSION);
		WRITE_COORD(vecStart.x);
		WRITE_COORD(vecStart.y);
		WRITE_COORD(vecStart.z);
		WRITE_SHORT(MODEL_INDEX("sprites/eexplo.spr"));
		WRITE_BYTE(10);
		WRITE_BYTE(30);
		WRITE_BYTE(TE_EXPLFLAG_NONE);
		MESSAGE_END();
	}
	
	EMIT_SOUND(m_pMonster->edict(), CHAN_AUTO, "zombi/revenant/revenant_fireball_explode.wav", VOL_NORM, ATTN_NORM);
}

boost::asio::awaitable<void> CBossSkill_Big05::TASK_ZBS_BURNNING1()
{
	m_pMonster->pev->takedamage = DAMAGE_NO;
	m_pMonster->pev->movetype = MOVETYPE_NONE;
	EMIT_SOUND(m_pMonster->edict(), CHAN_BODY, "zombi/revenant/revenant_zbs_burning1.wav", VOL_NORM, ATTN_NORM);

	m_pMonster->SetEntAnim(m_pMonster->LookupSequence(m_CurSkillData.aniSeq.c_str()), 1.0, m_CurSkillData.aniTime, false);
	m_pMonster->m_flStop = gpGlobals->time + m_CurSkillData.aniTime;

	MESSAGE_BEGIN(MSG_ALL, gmsgZBSMsg);
	WRITE_BYTE(0);
	WRITE_SHORT(std::atoi(m_CurSkillData.startMsg.c_str()));
	MESSAGE_END();

	MESSAGE_BEGIN(MSG_ALL, gmsgMPToCL, NULL);
	WRITE_BYTE(43);
	WRITE_SHORT(m_pMonster->entindex());
	MESSAGE_END();

	co_await async_wait_timer(m_CurSkillData.aniTime);

	m_pMonster->pev->takedamage = DAMAGE_YES;
	m_pMonster->pev->movetype = MOVETYPE_STEP;
	m_iSkillStatus = SKILL_STATUS_FREEZING;
	m_flTimeSkillNext = gpGlobals->time + 0.1s;
}

boost::asio::awaitable<void> CBossSkill_Big05::TASK_ZBS_BURNNING2()
{
	m_pMonster->pev->takedamage = DAMAGE_NO;
	m_pMonster->pev->movetype = MOVETYPE_NONE;
	EMIT_SOUND(m_pMonster->edict(), CHAN_BODY, "zombi/revenant/revenant_zbs_burning2.wav", VOL_NORM, ATTN_NORM);

	m_pMonster->SetEntAnim(m_pMonster->LookupSequence(m_CurSkillData.aniSeq.c_str()), 1.0, m_CurSkillData.aniTime, false);
	m_pMonster->m_flStop = gpGlobals->time + m_CurSkillData.aniTime;

	m_bHellFireActivated = false;
	m_bSpringCoolerUsed = false;

	MESSAGE_BEGIN(MSG_ALL, gmsgZBSMsg);
	WRITE_BYTE(0);
	WRITE_SHORT(std::atoi(m_CurSkillData.startMsg.c_str()));
	MESSAGE_END();

	SetSpringCooler(false);

	co_await async_wait_timer(6s);

	if (m_bSpringCoolerUsed)
		m_bHellFireActivated = false;
	else
	{
		m_bHellFireActivated = true;
		SetSpringCooler(true);

		MESSAGE_BEGIN(MSG_ALL, gmsgZBSMsg);
		WRITE_BYTE(0);
		WRITE_SHORT(360);
		MESSAGE_END();

		MESSAGE_BEGIN(MSG_ALL, gmsgMPToCL, NULL);
		WRITE_BYTE(43);
		WRITE_SHORT(m_pMonster->entindex());
		MESSAGE_END();
	}


	co_await async_wait_timer(m_CurSkillData.aniTime - 6s);

	m_pMonster->pev->takedamage = DAMAGE_YES;
	m_pMonster->pev->movetype = MOVETYPE_STEP;
	m_iSkillStatus = SKILL_STATUS_FREEZING;
	m_flTimeSkillNext = gpGlobals->time + 0.1s;
}

void CBossSkill_Big05::SetSpringCooler(bool reset)
{
	CBaseEntity* ent = nullptr;
	if (!strcmp(STRING(gpGlobals->mapname), "zs_decoy"))
	{
		if(!reset)
			m_iSpringCoolerIndex = RANDOM_LONG(1, 4);
		char szString[64];
		sprintf(szString, "boss_pipe%d", m_iSpringCoolerIndex);

		while ((ent = UTIL_FindEntityByClassname(ent, "cycler_sprite")) != nullptr)
		{
			if (!strcmp(STRING(ent->pev->targetname), szString))
			{
				if (!reset)
				{
					ent->pev->rendermode = kRenderTransAlpha;
					ent->pev->renderamt = 50;
				}
				else
				{
					ent->pev->rendermode = kRenderNormal;
					ent->pev->renderamt = 255;
				}
					
			}
		}

		sprintf(szString, "boss_use%d", m_iSpringCoolerIndex);
		while ((ent = UTIL_FindEntityByClassname(ent, "func_illusionary")) != nullptr)
		{
			if (!strcmp(STRING(ent->pev->targetname), szString))
			{
				if (!reset)
					ent->pev->renderamt = 255;
				else
					ent->pev->renderamt = 0;
			}
		}
	}
	
}

boost::asio::awaitable<void> CBossSkill_Big05::TASK_ZBS_GROGGY()
{
	m_pMonster->pev->takedamage = DAMAGE_YES;
	m_iSkillStatus = SKILL_STATUS_FREEZING;
	m_flTimeSkillNext = gpGlobals->time + 8.0s;
	m_pMonster->pev->movetype = MOVETYPE_NONE;
	m_pMonster->SetEntAnim(m_pMonster->LookupSequence("zbs_groggy"), 1.0, 8.0s, false);
	m_pMonster->m_flStop = gpGlobals->time + 8.0s;

	co_await async_wait_timer(8.0s);

	m_pMonster->pev->movetype = MOVETYPE_STEP;
	m_iSkillStatus = SKILL_STATUS_FREEZING;
	m_flTimeSkillNext = gpGlobals->time + 0.1s;
}

boost::asio::awaitable<void> CBossSkill_Big05::TASK_ZBS_APPEAR()
{	
	m_pMonster->pev->takedamage = DAMAGE_NO;
	Vector vecOrigin = g_vecZero, vecAngle = g_vecZero;
	CBaseEntity* ent = nullptr;
	while ((ent = UTIL_FindEntityByClassname(ent, "zombiespawn")) != nullptr)
	{
		CZombieSpawn* pAct = static_cast<CZombieSpawn*>(ent);
		
		if (!strcmp(STRING(gpGlobals->mapname), "zs_decoy"))
		{
			if (pAct->m_iId == 1000)
			{
				vecOrigin = pAct->pev->origin;
				vecAngle = pAct->pev->angles;
				m_pMonster->m_flTargetChange = gpGlobals->time + 10s;
				m_pMonster->m_improv->MoveTo(vecOrigin);
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
		
		while ((m_pMonster->pev->origin - vecOrigin).Length2D() >= 20)
		{
			m_pMonster->m_improv->MoveTo(vecOrigin);
			m_pMonster->m_flTargetChange = gpGlobals->time + 0.1s;
			co_await async_wait_timer(0.1s);
		}
	}

	m_pMonster->m_flStop = gpGlobals->time + m_CurSkillData.aniTime;
	m_pMonster->pev->movetype = MOVETYPE_NONE;
	m_pMonster->pev->velocity = g_vecZero;
	m_flAppearTime = invalid_time_point;
	m_pMonster->SetEntAnim(m_pMonster->LookupSequence("scene_appear"), 1.0, m_CurSkillData.aniTime, false);

	EMIT_SOUND(m_pMonster->edict(), CHAN_BODY, "zombi/revenant/revenant_scene_appear.wav", VOL_NORM, ATTN_NORM);
	
	co_await async_wait_timer(m_CurSkillData.aniTime);

	m_pMonster->pev->takedamage = DAMAGE_YES;
	m_pMonster->pev->movetype = MOVETYPE_STEP;
	m_iSkillStatus = SKILL_STATUS_FREEZING;
	m_pMonster->m_flTargetChange = gpGlobals->time + 0.1s;
	m_flTimeSkillNext = gpGlobals->time + 0.1s;
}

void CBossSkill_Big05::RadiusDamage(Vector vecSrc, float flDamage, float angle, float flRadius, float flHeight, int bitsDamageType, float flKickSide, float flKickUp, float flRushPower, bool bFalloff, bool bDropWeapons)
{
	CBaseEntity* pEntity = NULL;
	float flAdjustedDamage, falloff;

	if (bFalloff)
	{
		if (flRadius)
			falloff = flDamage / flRadius;
		else
			falloff = 1;
	}
	else
		falloff = 1;

	vecSrc.z += 1;


	while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, flRadius)) != NULL)
	{
		if (!m_pMonster->AngularCheck(pEntity, m_pMonster, vecSrc, angle))
			continue;

		if (pEntity->pev->takedamage == DAMAGE_NO)
			continue;

		if (!pEntity->IsAlive())
			continue;

		flAdjustedDamage = (vecSrc - pEntity->pev->origin).Length() * falloff;
		flAdjustedDamage = flDamage - flAdjustedDamage;

		if (flAdjustedDamage < 0)
			flAdjustedDamage = 0;

		pEntity->TakeDamage(m_pMonster->pev, m_pMonster->pev, bFalloff ? flAdjustedDamage : flDamage, bitsDamageType);

		if (pEntity->IsPlayer())
		{
			CBasePlayer* player = GetClassPtr<CBasePlayer>(pEntity->pev);

			if (player)
			{
				Vector2D vecDir2D = (pEntity->pev->origin.Make2D() - m_pMonster->pev->origin.Make2D()).Normalize();

				pEntity->pev->velocity.x = vecDir2D.x * flRushPower;
				pEntity->pev->velocity.y = vecDir2D.y * flRushPower;

				if (bDropWeapons)
				{
					player->ReactToDeimosTail();
				}

				//catch fire
			}
		}



	}
}

void CBossSkill_Big05::OnTouch(CBaseEntity* other)
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

void CBossClass_Big05::OnSpawn()
{
	m_pMonster->pev->max_health = 1000.0f;
	m_pMonster->m_flAttackDamage = 10.0f;
	m_pMonster->m_flAttackRate = 2.0s;
	m_pMonster->m_flAttackDist = 72.0f;
	m_pMonster->pev->maxspeed = 225.0f;
	m_pMonster->m_turnTolerance = 400.0f;
	//Scene skill activate instantly
	m_pMonster->pev->nextthink = gpGlobals->time;
	m_pMonster->m_flNextFullThink = invalid_time_point;
	m_pMonster->m_flStop = gpGlobals->time;
	m_pMonster->m_flSkillCheck = gpGlobals->time;
}

void CBossClass_Big05::OnInit()
{
	m_bAnimDeath = false;
	PRECACHE_MODEL("models/monster/zbs_bossl_big05.mdl");
	PRECACHE_MODEL("sprites/flame.spr");

	PRECACHE_SOUND("zombi/revenant/revenant_scene_appear.wav");
	PRECACHE_SOUND("zombi/revenant/revenant_zbs_death.wav");
	PRECACHE_SOUND("zombi/fallentitan/footstep1.wav");
	PRECACHE_SOUND("zombi/fallentitan/footstep2.wav");
	PRECACHE_SOUND("zombi/revenant/revenant_zbs_burning1.wav");
	PRECACHE_SOUND("zombi/revenant/revenant_zbs_burning2.wav");
	PRECACHE_SOUND("zombi/revenant/revenant_zbs_attack4.wav");
	PRECACHE_SOUND("zombi/revenant/revenant_zbs_fireball1.wav");
	PRECACHE_SOUND("zombi/revenant/revenant_zbs_fireball2.wav");
	PRECACHE_SOUND("zombi/revenant/revenant_zbs_attack1.wav");
	PRECACHE_SOUND("zombi/revenant/revenant_zbs_ready.wav");
	PRECACHE_SOUND("zombi/revenant/revenant_fireball_explode.wav");
	PRECACHE_SOUND("zombi/revenant/revenant_zbs_idle1.wav");

	SET_MODEL(m_pMonster->edict(), "models/monster/zbs_bossl_big05.mdl");
	UTIL_SetSize(m_pMonster->pev, { -50.0, -50.0, 30.0 }, { 50.0, 50.0, 130.0 });
	m_pMonster->SetSkill(std::make_unique<CBossSkill_Big05>(m_pMonster));
	m_pMonster->SetAble(JUMP, false);
}

void CBossClass_Big05::OnKilled(entvars_t* pKiller, int iGib)
{
	EMIT_SOUND(m_pMonster->edict(), CHAN_BODY, "zombi/revenant/revenant_zbs_death.wav", VOL_NORM, ATTN_NORM);

	m_pMonster->pev->nextthink = gpGlobals->time + 15s;
	m_pMonster->pev->sequence = 16;
	m_pMonster->pev->frame = 0;
	m_pMonster->ResetSequenceInfo();
}

}

