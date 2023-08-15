
#include "monster_boss_big07.h"

namespace sv {

extern std::vector<BossData> g_BossData;

LINK_ENTITY_TO_CLASS(zbs_big07_landmine, CZBSBig07Landmine);

void CZBSBig07Landmine::Spawn(void)
{
	Precache();

	pev->takedamage = DAMAGE_NO;
	pev->movetype = MOVETYPE_TOSS;
	pev->solid = SOLID_BBOX;

	pev->frame = 0;
	pev->gravity = 0.8;
	SET_MODEL(ENT(pev), "models/titan_m14.mdl");
	UTIL_SetSize(pev, { -1.0, -1.0, -1.0 }, { 1.0, 1.0, 1.0 });

	SetTouch(&CZBSBig07Landmine::LandmineTouch);
	SetThink(&CZBSBig07Landmine::LandmineThink);

	pev->nextthink = gpGlobals->time + 0.2s;
}

void CZBSBig07Landmine::Precache(void)
{
	PRECACHE_MODEL("models/titan_m14.mdl");

	PRECACHE_SOUND("zombi/fallentitan/landmine_drop1.wav");
	PRECACHE_SOUND("zombi/fallentitan/landmine_drop2.wav");
	PRECACHE_SOUND("zombi/fallentitan/landmine_drop3.wav");
	PRECACHE_SOUND("zombi/fallentitan/landmine_drop4.wav");
	PRECACHE_SOUND("zombi/fallentitan/landmine_exp.wav");
	
}

void CZBSBig07Landmine::LandmineThink()
{
	pev->nextthink = gpGlobals->time + 0.2s;

	CBaseEntity* pEntity = NULL;
	while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, 60.0)) != NULL)
	{
		if (!pEntity->IsPlayer())
			continue;

		if (!pEntity->IsAlive())
			continue;

		if (pev->flags & ~FL_ONGROUND)
			continue;

		m_flExpTime = invalid_time_point;
	}

	if (m_flExpTime < gpGlobals->time || m_flExpTime == invalid_time_point)
	{
		MakeExplosion();
	}
}

void CZBSBig07Landmine::MakeExplosion()
{
	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
	WRITE_BYTE(TE_EXPLOSION);
	WRITE_COORD(pev->origin.x);
	WRITE_COORD(pev->origin.y);
	WRITE_COORD(pev->origin.z);
	WRITE_SHORT(MODEL_INDEX("sprites/eexplo.spr"));
	WRITE_BYTE(RANDOM_LONG(20, 40));
	WRITE_BYTE(30);
	WRITE_BYTE(TE_EXPLFLAG_NOSOUND);
	MESSAGE_END();

	EMIT_SOUND(this->edict(), CHAN_ITEM, "zombi/fallentitan/landmine_exp.wav", VOL_NORM, ATTN_NORM);

	entvars_t* pevOwner = VARS(pev->owner);
	if (!pevOwner)
		return;

	CBaseEntity* pEntity = NULL;
	while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, m_flRadius)) != NULL)
	{
		if (!pEntity->IsPlayer())
			continue;

		if (!pEntity->IsAlive())
			continue;

		int falloff = m_flDamage / m_flRadius;

		float flCurrentDamage = max(0, m_flDamage - (pev->origin - pEntity->pev->origin).Length() * falloff);

		pEntity->TakeDamage(pev, pevOwner, flCurrentDamage, DMG_NEVERGIB | DMG_EXPLOSION);

		UTIL_ScreenShakeOne(pEntity, 10, 3, 10);
	}

	SetThink(NULL);
	UTIL_Remove(this);
}

void CZBSBig07Landmine::LandmineTouch(CBaseEntity* pOther)
{
	if (!pOther)
		return;

	if (pOther->edict() == pev->owner)
		return;

	if (pev->velocity.IsZero())
		return;

	LieFlat();
	SetTouch(NULL);
}

void CZBSBig07Landmine::LieFlat()
{
	if (pev->flags & ~FL_ONGROUND)
		return;

	Vector vecTemp = { 0, 0, 10 };
	Vector traceTo = pev->origin - vecTemp;

	TraceResult tr;

	UTIL_TraceLine(pev->origin, traceTo, ignore_monsters, ENT(pev), &tr);

	if (tr.flFraction == 1.0)
		return;

	Vector vecForward, vecRight, vecUp, vecAngle, vecAngle2;
	UTIL_MakeVectors(pev->angles);
	vecUp = tr.vecPlaneNormal;
	vecRight = CrossProduct(gpGlobals->v_forward, vecUp);
	vecForward = CrossProduct(vecUp, vecRight);

	vecAngle = UTIL_VecToAngles(vecForward);
	vecAngle2 = UTIL_VecToAngles(vecRight);

	vecAngle[2] = -1.0 * vecAngle2[0];
	pev->angles = vecAngle;

	int i = RANDOM_LONG(1, 4);
	char szString[64];
	sprintf(szString, "zombi/fallentitan/landmine_drop%d.wav", i);

	EMIT_SOUND(this->edict(), CHAN_ITEM, szString, VOL_NORM, ATTN_NORM);
}

LINK_ENTITY_TO_CLASS(zbs_big07_grenade, CZBSBig07Grenade);

void CZBSBig07Grenade::Spawn(void)
{
	Precache();

	pev->movetype = MOVETYPE_BOUNCE;
	pev->solid = SOLID_TRIGGER;

	pev->frame = 0;

	ResetSequenceInfo();

	SET_MODEL(ENT(pev), "models/grenade.mdl");
	UTIL_SetSize(pev, { -0.1, -0.1, -0.1 }, { 0.1, 0.1, 0.1 });

	SetTouch(&CZBSBig07Grenade::GrenadeTouch);
	SetThink(&CZBSBig07Grenade::IgniteThink);

	pev->nextthink = gpGlobals->time + 0.1s;

}

void CZBSBig07Grenade::Precache(void)
{
	m_iTrail = PRECACHE_MODEL("sprites/smoke.spr");

	PRECACHE_MODEL("models/grenade.mdl");
}

void CZBSBig07Grenade::IgniteThink()
{
	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
	WRITE_BYTE(TE_BEAMFOLLOW);
	WRITE_SHORT(entindex());
	WRITE_SHORT(m_iTrail);
	WRITE_BYTE(10);
	WRITE_BYTE(4);
	WRITE_BYTE(224);
	WRITE_BYTE(224);
	WRITE_BYTE(255);
	WRITE_BYTE(255);
	MESSAGE_END();

	SetThink(&CZBSBig07Grenade::FollowThink);
	pev->nextthink = gpGlobals->time + 0.1s;
}

void CZBSBig07Grenade::FollowThink(void)
{
	pev->angles = UTIL_VecToAngles(pev->velocity.Normalize());
	pev->nextthink = gpGlobals->time + 0.1s;
}

void CZBSBig07Grenade::GrenadeTouch(CBaseEntity* pOther)
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

	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
	WRITE_BYTE(TE_KILLBEAM);
	WRITE_SHORT(this->entindex());
	MESSAGE_END();

	SetTouch(NULL);
	SetThink(NULL);

	UTIL_Remove(this);
}

void CBossSkill_Big07::OnThink()
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

void CBossSkill_Big07::ActivateSkill(CBaseEntity* target)
{
	m_iSkillStatus = SKILL_STATUS_FREEZING;
	m_flTimeSkillNext = m_flForceAttackTime + 15.0s;
	m_MapSkillTime[m_CurSkillData.raw] = gpGlobals->time + m_CurSkillData.cooltime;

	//使用技能
	switch (m_CurSkillData.type)
	{
	case 61:
		StartTask(TASK_ZBS_APPEAR(), boost::asio::detached);
		break;
	case 62:
		StartTask(TASK_ZBS_CANNON_READY(), boost::asio::detached);
		break;
	case 63:
		StartTask(TASK_ZBS_CANNON2(), boost::asio::detached);
		break;
	case 64:
		StartTask(TASK_ZBS_LANDMINE1(), boost::asio::detached);
		break;
	case 65:
		StartTask(TASK_ZBS_LANDMINE2(), boost::asio::detached);
		break;
	case 66:
		StartTask(TASK_ZBS_ATTACK1(), boost::asio::detached);
		break;
	case 67:
		StartTask(TASK_ZBS_ATTACK1(), boost::asio::detached);
		break;
	case 68:
		StartTask(TASK_ZBS_DASH(), boost::asio::detached);
		break;
	case 69:
	case 70:
		StartTask(TASK_ZBS_HOWLING(), boost::asio::detached);
		break;
	default:
		break;
	}
}


void CBossSkill_Big07::OnActivate(CBaseEntity* target)
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

		if (iter->type == 69)
		{
			int iBossCount = 0;
			CBaseEntity* npc = nullptr;
			while ((npc = UTIL_FindEntityByClassname(npc, "monster_entity")) != nullptr)
			{
				CMonster* pMonster = static_cast<CMonster*>(npc);

				if (pMonster)
				{
					//already has boss continue
					if (pMonster->m_iZombiId == 30801)
						iBossCount++;

					if (pMonster->m_iZombiId == 30802)
						iBossCount++;
				}
			}

			if (iBossCount >= 2)
				continue;
		}
		


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

boost::asio::awaitable<void> CBossSkill_Big07::TASK_ZBS_ATTACK1()
{
	m_pMonster->pev->movetype = MOVETYPE_NONE;
	m_pMonster->SetEntAnim(m_pMonster->LookupSequence(m_CurSkillData.aniSeq.c_str()), 1.0, m_CurSkillData.aniTime, false);
	m_pMonster->m_flStop = gpGlobals->time + m_CurSkillData.damageDelay;
	EMIT_SOUND(m_pMonster->edict(), CHAN_BODY, "zombi/fallentitan/zbs_attack1.wav", VOL_NORM, ATTN_NORM);

	co_await async_wait_timer(m_CurSkillData.damageDelay);

	m_pMonster->RadiusDamage(m_pMonster->pev->origin, m_CurSkillData.damage, m_CurSkillData.damageDegree, m_CurSkillData.damageDist, m_CurSkillData.damageHeight, DMG_SLASH, m_CurSkillData.kickSide, m_CurSkillData.kickUp, m_CurSkillData.rushPower, false);
	m_pMonster->m_flStop = gpGlobals->time + m_CurSkillData.aniTime - m_CurSkillData.damageDelay;
	co_await async_wait_timer(m_CurSkillData.aniTime - m_CurSkillData.damageDelay);

	m_pMonster->pev->movetype = MOVETYPE_STEP;
	m_iSkillStatus = SKILL_STATUS_FREEZING;
	m_flTimeSkillNext = gpGlobals->time + 0.1s;
}

boost::asio::awaitable<void> CBossSkill_Big07::TASK_ZBS_ATTACK2()
{
	m_pMonster->pev->movetype = MOVETYPE_NONE;
	m_pMonster->SetEntAnim(m_pMonster->LookupSequence(m_CurSkillData.aniSeq.c_str()), 1.0, m_CurSkillData.aniTime, false);
	m_pMonster->m_flStop = gpGlobals->time + m_CurSkillData.damageDelay + 0.16s;
	EMIT_SOUND(m_pMonster->edict(), CHAN_BODY, "zombi/fallentitan/zbs_attack2.wav", VOL_NORM, ATTN_NORM);

	co_await async_wait_timer(m_CurSkillData.damageDelay + 0.16s);

	m_pMonster->m_flStop = gpGlobals->time + m_CurSkillData.aniTime - m_CurSkillData.damageDelay;
	m_pMonster->RadiusDamage(m_pMonster->pev->origin, m_CurSkillData.damage, m_CurSkillData.damageDegree, m_CurSkillData.damageDist, m_CurSkillData.damageHeight, DMG_SLASH, m_CurSkillData.kickSide, m_CurSkillData.kickUp, m_CurSkillData.rushPower, false);
	co_await async_wait_timer(m_CurSkillData.aniTime - m_CurSkillData.damageDelay);

	m_pMonster->pev->movetype = MOVETYPE_STEP;
	m_iSkillStatus = SKILL_STATUS_FREEZING;
	m_flTimeSkillNext = gpGlobals->time + 0.1s;
}

boost::asio::awaitable<void> CBossSkill_Big07::TASK_ZBS_DASH()
{
	m_pMonster->pev->movetype = MOVETYPE_NONE;
	m_pMonster->SetEntAnim(m_pMonster->LookupSequence(m_CurSkillData.aniSeq.c_str()), 1.0, m_CurSkillData.aniTime, false);
	//m_pMonster->m_flStop = gpGlobals->time + m_CurSkillData.damageDelay;
	co_await async_wait_timer(m_CurSkillData.aniTime);

	m_pMonster->pev->movetype = MOVETYPE_PUSHSTEP;
	m_pMonster->m_flStop = gpGlobals->time + 1.1s;
	m_pMonster->SetEntAnim(m_pMonster->LookupSequence("zbs_dash"), 1.0, 1.0s, false);
	g_engfuncs.pfnWalkMove(m_pMonster->edict(), m_pMonster->pev->angles.y, 15.0, WALKMOVE_NORMAL);
	UTIL_MakeVectors(m_pMonster->pev->angles);
	Vector vecVelocity = gpGlobals->v_forward;
	vecVelocity *= 1500.0;
	m_pMonster->pev->velocity = vecVelocity;
	
	co_await async_wait_timer(1.0s);

	m_pMonster->pev->movetype = MOVETYPE_NONE;
	m_pMonster->SetEntAnim(m_pMonster->LookupSequence("zbs_dash_end"), 1.0, 1.8s, false);
	m_pMonster->m_flStop = gpGlobals->time + 1.9s;
	co_await async_wait_timer(1.8s);

	m_pMonster->pev->movetype = MOVETYPE_STEP;
	m_iSkillStatus = SKILL_STATUS_FREEZING;
	m_flTimeSkillNext = gpGlobals->time + 0.1s;
}

boost::asio::awaitable<void> CBossSkill_Big07::TASK_ZBS_CANNON_READY()
{
	m_pMonster->pev->movetype = MOVETYPE_NONE;
	m_pMonster->SetEntAnim(m_pMonster->LookupSequence(m_CurSkillData.aniSeq.c_str()), 1.0, m_CurSkillData.aniTime, false);
	m_flCannon1SkillTime = gpGlobals->time + m_CurSkillData.damageDelay + 4s;
	EMIT_SOUND(m_pMonster->edict(), CHAN_BODY, "zombi/fallentitan/zbs_cannon_ready.wav", VOL_NORM, ATTN_NORM);

	co_await async_wait_timer(m_CurSkillData.damageDelay);

	m_pMonster->m_flStop = gpGlobals->time + 4s;
	while (m_flCannon1SkillTime > gpGlobals->time)
	{
		
		Vector vecOrigin = m_pMonster->pev->origin;
		m_pMonster->SetEntAnim(m_pMonster->LookupSequence("zbs_cannon1"), 1.0, m_CurSkillData.aniTime, false);

		EMIT_SOUND(m_pMonster->edict(), CHAN_BODY, "zombi/fallentitan/zbs_cannon1.wav", VOL_NORM, ATTN_NORM);

		float flROffset = -27.0; 
		float flUOffset = 73.0;
		int iAmount = RANDOM_LONG(3, 4);
		float flDist = 600.0;
		if (m_pMonster->m_hTargetEnt)
			flDist = max(600.0, (m_pMonster->m_hTargetEnt->pev->origin - m_pMonster->pev->origin).Length());
		
		for (int i = 0; i < iAmount; i++)
		{
			GetPositon(50.0, flROffset, flUOffset, vecOrigin);
			CreateStraightRocket(vecOrigin, flDist, flROffset, flUOffset);
			flROffset -= 3.0;
			flUOffset += 3.0;
		}
		co_await async_wait_timer(0.25s);
	}
	
	m_pMonster->SetEntAnim(m_pMonster->LookupSequence("zbs_cannon_end"), 1.0, 1.5s, false);
	m_pMonster->m_flStop = gpGlobals->time + 1.5s;
	co_await async_wait_timer(1.5s);

	m_pMonster->pev->movetype = MOVETYPE_STEP;
	m_iSkillStatus = SKILL_STATUS_FREEZING;
	m_flTimeSkillNext = gpGlobals->time + 0.1s;
}

void CBossSkill_Big07::CreateStraightRocket(Vector vecOrigin, float flDistance, float flROffset, float flUOffset)
{
	CZBSBig07Grenade* pEnt = CreateRocket(vecOrigin);
	if (pEnt)
	{
		float gravity = RANDOM_FLOAT(0.5, 3.0);
		pEnt->pev->gravity = gravity;
		float Acceleration = gravity * 400.0;

		Vector vecOrigin2, vecVelocity;
		GetPositon(flDistance + RANDOM_FLOAT(-400.0, 400.0), flROffset, flUOffset, vecOrigin2);
		vecVelocity = UTIL_GetSpeedVector(vecOrigin, vecOrigin2, flDistance + Acceleration);
		pEnt->pev->velocity = vecVelocity;
		UTIL_VecToAngles(vecVelocity);
		pEnt->pev->angles = vecVelocity;
	}
}

void CBossSkill_Big07::GetPositon(float forward, float right, float up, Vector& start)
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

boost::asio::awaitable<void> CBossSkill_Big07::TASK_ZBS_CANNON2()
{
	m_pMonster->pev->movetype = MOVETYPE_NONE;
	m_pMonster->SetEntAnim(m_pMonster->LookupSequence(m_CurSkillData.aniSeq.c_str()), 1.0, m_CurSkillData.aniTime, false);
	m_pMonster->m_flStop = gpGlobals->time + m_CurSkillData.damageDelay;
	m_flCannon2SkillTime = gpGlobals->time + m_CurSkillData.damageDelay;
	m_iShootTimes = 0;
	co_await async_wait_timer(m_CurSkillData.damageDelay);

	while (m_flCannon2SkillTime != invalid_time_point)
	{	
		duration_t flNextThink;
		float flFOffset, flROffset, flUOffset;
		Vector vecOrigin2;
		int iAmount = 3;

		switch (m_iShootTimes)
		{
		case 0:
		{
			flFOffset = -5.0, flROffset = 3.0, flUOffset = 150.0;
			vecOrigin2 = { 5.0, 12.0, 12.0 };
			flNextThink = 0.2666s;
			m_flCannon2SkillTime = gpGlobals->time + 0.2666s; // Frame: 34 (Next Shoot)
			break;
		}
		case 1:
		{
			flFOffset = -5.0, flROffset = 12.0, flUOffset = 140.0;
			vecOrigin2 = { 6.0, 11.0, 11.0 };
			flNextThink = 0.2334s;
			m_flCannon2SkillTime = gpGlobals->time + 0.2334s; // Frame: 41
			break;
		}
		case 2:
		{
			flFOffset = -1.0, flROffset = 20.0, flUOffset = 130.0;
			vecOrigin2 = { 7.0, 11.0, 10.0 };
			flNextThink = 0.3s;
			m_flCannon2SkillTime = gpGlobals->time + 0.3s; // Frame: 50
			break;
		}
		case 3:
		{
			flFOffset = 15.0, flROffset = 20.0, flUOffset = 130.0;
			vecOrigin2 = { 7.0, 2.0, 9.0 };
			flNextThink = 0.3s;
			m_flCannon2SkillTime = gpGlobals->time + 0.3s; // Frame: 59
			break;
		}
		case 4:
		{
			flFOffset = 25.0, flROffset = 12.0, flUOffset = 130.0;
			vecOrigin2 = { 7.0, 1.0, 8.0 };
			flNextThink = 0.3s;
			m_flCannon2SkillTime = gpGlobals->time + 0.3s; // Frame: 68
			break;
		}
		case 5:
		{
			flFOffset = 45.0, flROffset = 3.0, flUOffset = 130.0;
			vecOrigin2 = { 7.0, 0.0, 7.0 };
			flNextThink = 0.3666s;
			m_flCannon2SkillTime = gpGlobals->time + 0.3666s; // Frame: 79
			break;
		}
		case 6:
		{
			flFOffset = 50.0, flROffset = 1.0, flUOffset = 130.0;
			vecOrigin2 = { 5.0, -1.0, 6.0 };
			flNextThink = 0.2334s;
			m_flCannon2SkillTime = gpGlobals->time + 0.2334s; // Frame: 86
			break;
		}
		case 7:
		{
			flFOffset = 30.0, flROffset = 1.0, flUOffset = 130.0;
			vecOrigin2 = { 5.0, -1.0, 6.0 };
			flNextThink = 0.2333s;
			m_flCannon2SkillTime = gpGlobals->time + 0.2333s;// Frame: 93
			break;
		}
		case 8:
		{
			flFOffset = 30.0, flROffset = -1.0, flUOffset = 130.0;
			vecOrigin2 = { 4.0, -2.0, 7.0 };
			flNextThink = 0.2367s;
			m_flCannon2SkillTime = gpGlobals->time + 0.2367s; // Frame: 101
			break;
		}
		case 9:
		{
			flFOffset = 30.0, flROffset = -5.0, flUOffset = 150.0;
			vecOrigin2 = { 3.0, -3.0, 8.0 };
			flNextThink = 0.2966s;
			m_flCannon2SkillTime = gpGlobals->time + 0.2966s; // Frame: 109
			break;
		}
		case 10:
		{
			flFOffset = 20.0, flROffset = -10.0, flUOffset = 150.0;
			vecOrigin2 = { 2.0, -11.0, 9.0 };
			flNextThink = 0.2334s;
			m_flCannon2SkillTime = gpGlobals->time + 0.2334s; // Frame: 116
			break;
		}
		case 11:
		{
			flFOffset = 10.0, flROffset = -30.0, flUOffset = 150.0;
			vecOrigin2 = { 1.0, -11.0, 11.0 };
			flNextThink = 0.5s;
			m_flCannon2SkillTime = gpGlobals->time + 0.5s; // Frame: 131
			break;
		}
		case 12:
		{
			flFOffset = -30.0, flROffset = -40.0, flUOffset = 150.0;
			vecOrigin2 = { -1.0, -15.0, 12.0 };
			flNextThink = 2.0666s;
			m_flCannon2SkillTime = invalid_time_point;
			break;
		}
		default:
		{
			m_flCannon2SkillTime = invalid_time_point;
			flNextThink = 0.1s;
		}
		}

		Vector vecTo;
		for (int i = 0; i < iAmount; i++)
		{
			vecTo[0] = vecOrigin2[0] * RANDOM_FLOAT(1.0, 10.0);
			vecTo[1] = vecOrigin2[1] * RANDOM_FLOAT(10.0, 20.0);
			vecTo[2] = vecOrigin2[2] * RANDOM_FLOAT(20.0, 30.0);

			GetPositon(vecTo[0], vecTo[1], vecTo[2], vecTo);
			CreateSpecialRocket(flFOffset, flROffset, flUOffset, vecTo);

			flROffset -= 3.0;
			flUOffset += 3.0;
		}

		EMIT_SOUND(m_pMonster->edict(), CHAN_BODY, "zombi/fallentitan/zbs_cannon1.wav", VOL_NORM, ATTN_NORM);

		m_iShootTimes++;
		co_await async_wait_timer(flNextThink);
	}

	m_pMonster->pev->movetype = MOVETYPE_STEP;
	m_iSkillStatus = SKILL_STATUS_FREEZING;
	m_flTimeSkillNext = gpGlobals->time + 0.1s;
}

void CBossSkill_Big07::CreateSpecialRocket(float flFOffset, float flROffset, float flUOffset, Vector vecOrigin2)
{
	Vector vecOrigin;
	GetPositon(flFOffset, flROffset, flUOffset, vecOrigin);
	CZBSBig07Grenade* pEnt = CreateRocket(vecOrigin);
	if (pEnt)
	{
		pEnt->pev->gravity = 0.5;
		float Acceleration = pEnt->pev->gravity * 400.0;

		Vector vecVelocity;
		if (VecCheckThrow(vecOrigin, vecOrigin2, 400.0, 0.5, vecVelocity))
		{
			pEnt->pev->velocity = vecVelocity;
		}
		else
		{
			vecVelocity = UTIL_GetSpeedVector(vecOrigin, vecOrigin2, RANDOM_FLOAT(300.0, 400.0) + Acceleration);
			pEnt->pev->velocity = vecVelocity;
		}

		UTIL_VecToAngles(vecVelocity);
		pEnt->pev->angles = vecVelocity;
	}
}

int CBossSkill_Big07::VecCheckThrow(Vector vecSpot1, Vector vecSpot2, float flSpeed, float flGravityAdj, Vector &vecGrenadeVel)
{
	float flGravity = CVAR_GET_FLOAT("sv_gravity") * flGravityAdj;
	vecGrenadeVel = vecSpot2 - vecSpot1;

	// throw at a constant time
	duration_t time = (vecGrenadeVel.Length() / flSpeed) * 1s;
	vecGrenadeVel = vecGrenadeVel * (1.0s / time);

	// adjust upward toss to compensate for gravity loss
	vecGrenadeVel[2] += flGravity * time * 0.5 / 1s;

	Vector vecApex;
	vecApex = vecSpot2 - vecSpot1;
	vecApex = 0.5 * vecApex;
	vecApex = vecApex + vecSpot1;

	vecApex[2] += 0.5 * flGravity * (time * 0.5 / 1s) * (time * 0.5 / 1s);

	TraceResult tr;
	UTIL_TraceLine(vecSpot1, vecApex, ignore_monsters, m_pMonster->edict(), &tr);
	if (tr.flFraction != 1.0)
	{
		// fail!
		return 0;
	}

	UTIL_TraceLine(vecSpot2, vecApex, ignore_monsters, m_pMonster->edict(), &tr);
	if (tr.flFraction != 1.0)
	{
		// fail!
		return 0;
	}

	return 1;
}


CZBSBig07Grenade* CBossSkill_Big07::CreateRocket(Vector vecOrigin)
{
	CZBSBig07Grenade* pEnt = static_cast<CZBSBig07Grenade*>(CBaseEntity::Create("zbs_big07_grenade", vecOrigin, g_vecZero, m_pMonster->edict()));
	if (pEnt)
	{
		pEnt->m_flDamage = m_CurSkillData.damage;
		pEnt->m_flRadius = 250;
	}
	return pEnt;
}

boost::asio::awaitable<void> CBossSkill_Big07::TASK_ZBS_LANDMINE1()
{
	m_pMonster->pev->movetype = MOVETYPE_NONE;
	m_pMonster->SetEntAnim(m_pMonster->LookupSequence(m_CurSkillData.aniSeq.c_str()), 1.0, m_CurSkillData.aniTime, false);
	m_pMonster->m_flStop = gpGlobals->time + m_CurSkillData.damageDelay;
	co_await async_wait_timer(m_CurSkillData.damageDelay);

	//TODO
	EMIT_SOUND(m_pMonster->edict(), CHAN_BODY, "zombi/fallentitan/zbs_landmine1.wav", VOL_NORM, ATTN_NORM);

	CreateCircleLandmines(m_pMonster->pev->origin);
	co_await async_wait_timer(m_CurSkillData.aniTime - m_CurSkillData.damageDelay);

	m_pMonster->pev->movetype = MOVETYPE_STEP;
	m_iSkillStatus = SKILL_STATUS_FREEZING;
	m_flTimeSkillNext = gpGlobals->time + 0.1s;
}


void CBossSkill_Big07::CreateCircleLandmines(Vector vecOrigin)
{
	float x, y;
	int iStartPos = 0;
	Vector vecStart, vecOrigin2;
	vecStart[2] = vecOrigin[2] + 50.0;
	int amount = RANDOM_LONG(20, 30);

	for (int i = 0; i < amount; i++)
	{
		x = vecOrigin[0] + 25.0 * cos(float(iStartPos) * M_PI / 180.0);
		y = vecOrigin[1] + 25.0 * sin(float(iStartPos) * M_PI / 180.0);
		vecStart[0] = x;
		vecStart[1] = y;
		x = vecOrigin[0] + RANDOM_FLOAT(50.0, 150.0) * cos(float(iStartPos) * M_PI / 180.0);
		y = vecOrigin[1] + RANDOM_FLOAT(50.0, 150.0) * sin(float(iStartPos) * M_PI / 180.0);
		vecOrigin2[0] = x;
		vecOrigin2[1] = y;
		vecOrigin2[2] = vecOrigin[2] + RANDOM_FLOAT(50.0, 200.0);
		CreateLandmine(vecStart, vecOrigin2);
		iStartPos += 360 / amount;
	}
}

CZBSBig07Landmine* CBossSkill_Big07::CreateLandmine(Vector vecOrigin, Vector vecOrigin2)
{
	CZBSBig07Landmine* pEnt = static_cast<CZBSBig07Landmine*>(CBaseEntity::Create("zbs_big07_landmine", vecOrigin, g_vecZero, m_pMonster->edict()));
	if (pEnt)
	{
		if (vecOrigin != vecOrigin2)
		{
			Vector vecVelocity = UTIL_GetSpeedVector(vecOrigin, vecOrigin2, RANDOM_FLOAT(400.0, 600.0));
			pEnt->pev->velocity = vecVelocity;

			pEnt->pev->avelocity[0] = RANDOM_FLOAT(-vecVelocity.Length(), vecVelocity.Length());
			pEnt->pev->avelocity[1] = RANDOM_FLOAT(-vecVelocity.Length(), vecVelocity.Length());
		}

		pEnt->m_flDamage = m_CurSkillData.damage;
		pEnt->m_flRadius = 200.0;
		pEnt->m_flExpTime = gpGlobals->time + RANDOM_FLOAT(5.0, 15.0) * 1s;
	}
	return pEnt;
}

boost::asio::awaitable<void> CBossSkill_Big07::TASK_ZBS_LANDMINE2()
{
	m_pMonster->pev->movetype = MOVETYPE_NONE;
	m_pMonster->SetEntAnim(m_pMonster->LookupSequence(m_CurSkillData.aniSeq.c_str()), 1.0, m_CurSkillData.aniTime, false);
	m_pMonster->m_flStop = gpGlobals->time + m_CurSkillData.aniTime;

	EMIT_SOUND(m_pMonster->edict(), CHAN_AUTO, "zombi/fallentitan/zbs_cannon_ready.wav", VOL_NORM, ATTN_NORM);

	co_await async_wait_timer(m_CurSkillData.damageDelay);

	//TODO
	EMIT_SOUND(m_pMonster->edict(), CHAN_AUTO, "zombi/fallentitan/landmine_jut.wav", VOL_NORM, ATTN_NORM);

	CreateStraightLandmines(m_pMonster->pev->origin);
	co_await async_wait_timer(m_CurSkillData.aniTime - m_CurSkillData.damageDelay);

	m_pMonster->pev->movetype = MOVETYPE_STEP;
	m_iSkillStatus = SKILL_STATUS_FREEZING;
	m_flTimeSkillNext = gpGlobals->time + 0.1s;

}

void CBossSkill_Big07::CreateStraightLandmines(Vector vecStart)
{
	float x = 0, y = 0;
	int iStartPos = 0;
	Vector vecOrigin;
	GetPositon(1500, 0.0, 0.0, vecOrigin);

	TraceResult tr;
	UTIL_TraceLine(vecStart, vecOrigin, ignore_monsters, m_pMonster->edict(), &tr);
	vecOrigin = tr.vecEndPos;

	int amount = RANDOM_LONG(20, 30);
	y = (vecOrigin - vecStart).Length() / float(amount);

	for (int i = 0; i < amount; i++)
	{
		x += y;
		GetPositon(x, RANDOM_FLOAT(-100.0, 100.0), 0.0, vecOrigin);
		CZBSBig07Landmine* pEnt = CreateLandmine(vecOrigin, vecOrigin);
		if (pEnt)
		{
			DROP_TO_FLOOR(pEnt->edict());
			pEnt->m_flExpTime = gpGlobals->time + 10.0s - float(i) * 0.2s;
		}
	}
}

boost::asio::awaitable<void> CBossSkill_Big07::TASK_ZBS_HOWLING()
{
	m_pMonster->pev->movetype = MOVETYPE_NONE;
	EMIT_SOUND(m_pMonster->edict(), CHAN_BODY, "zombi/fallentitan/scene_howling.wav", VOL_NORM, ATTN_NORM);

	m_pMonster->SetEntAnim(m_pMonster->LookupSequence(m_CurSkillData.aniSeq.c_str()), 1.0, m_CurSkillData.aniTime, false);
	m_pMonster->m_flStop = gpGlobals->time + m_CurSkillData.aniTime;

	MESSAGE_BEGIN(MSG_ALL, gmsgZBSMsg);
	WRITE_BYTE(0);
	WRITE_SHORT(std::atoi(m_CurSkillData.startMsg.c_str()));
	MESSAGE_END();

	CMod_ZombieScenario* pModZBS = dynamic_cast<CMod_ZombieScenario*>(g_pModRunning);
	if (pModZBS)
	{
		int iBossCount = 0;
		CBaseEntity* npc = nullptr;
		while ((npc = UTIL_FindEntityByClassname(npc, "monster_entity")) != nullptr)
		{
			CMonster* pMonster = static_cast<CMonster*>(npc);

			if (pMonster)
			{
				//already has boss continue
				if (pMonster->m_iZombiId == 30801)
				{
					std::map<int, int> vecZombi;
					vecZombi.insert(std::pair(30802, 100));
					pModZBS->MakeZombieNPC(26, 201, vecZombi);
					iBossCount++;
					break;
				}
					

				if (pMonster->m_iZombiId == 30802)
				{
					std::map<int, int> vecZombi;
					vecZombi.insert(std::pair(30801, 100));
					pModZBS->MakeZombieNPC(26, 201, vecZombi);
					iBossCount++;
					break;
				}	
			}
		}

		if (iBossCount < 1)
		{
			std::map<int, int> vecZombi;
			vecZombi.insert(std::pair(30801, 100));
			pModZBS->MakeZombieNPC(26, 201, vecZombi);
		}
	}
	
	co_await async_wait_timer(m_CurSkillData.aniTime);

	m_pMonster->pev->movetype = MOVETYPE_STEP;
	m_iSkillStatus = SKILL_STATUS_FREEZING;
	m_flTimeSkillNext = gpGlobals->time + 0.1s;
}

boost::asio::awaitable<void> CBossSkill_Big07::TASK_ZBS_APPEAR()
{	
	m_pMonster->pev->movetype = MOVETYPE_NONE;
	m_pMonster->SetEntAnim(m_pMonster->LookupSequence(m_CurSkillData.aniSeq.c_str()), 1.0, 1.1s, false);
	m_pMonster->m_flStop = gpGlobals->time + 2s;
	
	MESSAGE_BEGIN(MSG_ALL, gmsgZBSMsg);
	WRITE_BYTE(0);
	WRITE_SHORT(std::atoi(m_CurSkillData.startMsg.c_str()));
	MESSAGE_END();

	co_await async_wait_timer(1.1s);

	m_pMonster->pev->movetype = MOVETYPE_STEP;
	m_pMonster->m_flStop = gpGlobals->time + 8s;
	m_flAppearTime = gpGlobals->time + 8s;
	m_pMonster->SetEntAnim(m_pMonster->LookupSequence("scene_appear2"), 1.0, 8s, false);
	g_engfuncs.pfnWalkMove(m_pMonster->edict(), m_pMonster->pev->angles.y, 100.0, WALKMOVE_NORMAL);
	float flOriginZ = m_pMonster->pev->origin.z;
	UTIL_MakeVectors(m_pMonster->m_vecSpawnAngle);
	Vector vecForward = gpGlobals->v_forward;
	m_pMonster->pev->velocity = vecForward * 500;

	
	while (m_flAppearTime > gpGlobals->time && m_flAppearTime != invalid_time_point)
	{
		//TODO
		if ((m_pMonster->pev->flags & FL_ONGROUND))
		{	
			if (flOriginZ - m_pMonster->pev->origin.z > 150.0)
			{
				m_pMonster->pev->movetype = MOVETYPE_NONE;
				m_pMonster->pev->velocity = g_vecZero;
				m_flAppearTime = invalid_time_point;
				m_pMonster->SetEntAnim(m_pMonster->LookupSequence("scene_appear3"), 1.0, 6.4s, false);

				EMIT_SOUND(m_pMonster->edict(), CHAN_BODY, "zombi/fallentitan/scene_appear3.wav", VOL_NORM, ATTN_NORM);

				m_pMonster->m_flStop = gpGlobals->time + 6.4s;
				co_await async_wait_timer(6.4s);
				break;
			}

			m_pMonster->pev->velocity = vecForward * 500;
			co_await async_wait_timer(0.1s);

		}
		else
		{
			co_await async_wait_timer(0.1s);
		}		
	}

	m_pMonster->pev->movetype = MOVETYPE_STEP;
	m_iSkillStatus = SKILL_STATUS_FREEZING;
	m_flTimeSkillNext = gpGlobals->time + 0.1s;
}


void CBossSkill_Big07::OnTouch(CBaseEntity* other)
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

void CBossClass_Big07::OnSpawn()
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

void CBossClass_Big07::OnInit()
{
	m_bAnimDeath = false;
	PRECACHE_MODEL("models/monster/zbs_bossl_big07.mdl");

	PRECACHE_SOUND("zombi/fallentitan/landmine_jut.wav");
	PRECACHE_SOUND("zombi/fallentitan/death.wav");
	PRECACHE_SOUND("zombi/fallentitan/footstep1.wav");
	PRECACHE_SOUND("zombi/fallentitan/footstep2.wav");
	PRECACHE_SOUND("zombi/fallentitan/zbs_landmine1.wav");
	PRECACHE_SOUND("zombi/fallentitan/scene_appear1.wav");
	PRECACHE_SOUND("zombi/fallentitan/scene_appear3.wav");
	PRECACHE_SOUND("zombi/fallentitan/scene_howling.wav");
	PRECACHE_SOUND("zombi/fallentitan/zbs_attack1.wav");
	PRECACHE_SOUND("zombi/fallentitan/zbs_attack2.wav");
	PRECACHE_SOUND("zombi/fallentitan/zbs_cannon_ready.wav");
	PRECACHE_SOUND("zombi/fallentitan/zbs_cannon1.wav");
	PRECACHE_SOUND("zombi/fallentitan/zbs_idle1.wav");

	SET_MODEL(m_pMonster->edict(), "models/monster/zbs_bossl_big07.mdl");
	UTIL_SetSize(m_pMonster->pev, { -50.0, -50.0, 30.0 }, { 50.0, 50.0, 130.0 });
	m_pMonster->SetSkill(std::make_unique<CBossSkill_Big07>(m_pMonster));
	m_pMonster->SetAble(JUMP, false);
}

void CBossClass_Big07::OnKilled(entvars_t* pKiller, int iGib)
{
	EMIT_SOUND(m_pMonster->edict(), CHAN_BODY, "zombi/fallentitan/death.wav", VOL_NORM, ATTN_NORM);

	m_pMonster->pev->nextthink = gpGlobals->time + 15s;
	m_pMonster->pev->sequence = 19;
	m_pMonster->pev->frame = 0;
	m_pMonster->ResetSequenceInfo();
}

}
