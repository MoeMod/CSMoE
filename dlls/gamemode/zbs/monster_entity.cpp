
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "monsters.h"
#include "trains.h"

#include "monster_entity.h"
#include "zs_subs.h"

#include "game.h"
#include "bot_include.h" // nav

#include "zbs_const.h"
#include "monster_manager.h"
#include "gamemode/mods.h"

LINK_ENTITY_TO_CLASS(monster_entity, CMonster);

class CMonsterImprov : public CHostageImprov
{
public:
	CMonsterImprov(CBaseEntity *entity) : CHostageImprov(entity) 
	{
		
	}
	~CMonsterImprov() {} // virtual

	// remove some hostage staffs
	void Crouch() override { /* no code needed */ }
	void OnMoveToFailure(const Vector &goal, MoveToFailureType reason) override { /* no code needed */ }

	// jumping control
	bool GetSimpleGroundHeightWithFloor(const Vector *pos, float *height, Vector *normal) override
	{
		bool result = CHostageImprov::GetSimpleGroundHeightWithFloor(pos, height, normal);

		if (IsRunning() || IsWalking())
			return false;

		return result;
	}
	bool CanJump() const override
	{
		CBasePlayer *player = NULL;
		if (IsFollowing())
		{
			player = (CBasePlayer *)GetFollowLeader();
			if ((m_hostage->Center() - player->Center()).Length() < 48.0)
				return false;
		}

		return CHostageImprov::CanJump();
	}

	CBasePlayer *GetClosestVisiblePlayer(int team) override
	{
		CBasePlayer *close = NULL;
		float closeRangeSq = 1e8f;

		for (int i = 0; i < m_visiblePlayerCount; ++i)
		{
			CBasePlayer *player = (CBasePlayer *)CBaseEntity::Instance(m_visiblePlayer[i]);

			if (player == NULL)
				continue;

			if (!player->IsAlive())
				continue;

			float rangeSq = (GetCentroid() - player->pev->origin).LengthSquared();

			if (rangeSq < closeRangeSq)
			{
				closeRangeSq = rangeSq;
				close = player;
			}
		}

		return close;
	}
};

void CMonster::Spawn()
{
	Precache();

	if (pev->classname)
	{
		RemoveEntityHashValue(pev, STRING(pev->classname), CLASSNAME);
	}

	MAKE_STRING_CLASS("monster_entity", pev);
	AddEntityHashValue(pev, STRING(pev->classname), CLASSNAME);

	pev->movetype = MOVETYPE_STEP;
	pev->solid = SOLID_SLIDEBOX;
	pev->takedamage = DAMAGE_YES;
	pev->flags |= FL_MONSTER;
	pev->deadflag = DEAD_NO;
	pev->max_health = 100;
	pev->health = pev->max_health;
	pev->gravity = 1;
	pev->view_ofs = VEC_VIEW;
	pev->velocity = Vector(0, 0, 0);
	pev->maxspeed = 140.0f;

	if (pev->spawnflags & SF_MONSTER_HITMONSTERCLIP)
		pev->flags |= FL_MONSTERCLIP;

	if (pev->skin < 0)
		pev->skin = 0;

	SET_MODEL(edict(), "models/player/zombi_origin/zombi_origin.mdl");
	//SetActivity(ACT_IDLE);
	SetAnimation(MONSTERANIM_IDLE);

	m_flNextChange = 0;
	m_State = STAND;
	m_hTargetEnt = NULL;
	m_hStoppedTargetEnt = NULL;
	m_vPathToFollow[0] = Vector(0, 0, 0);
	m_flFlinchTime = 0;
	m_bRescueMe = FALSE;

	UTIL_SetSize(pev, VEC_HULL_MIN, VEC_HULL_MAX);

	TraceResult tr;
	TRACE_MONSTER_HULL(edict(), pev->origin, pev->origin, dont_ignore_monsters, edict(), &tr);

	if (tr.fStartSolid || tr.fAllSolid || !tr.fInOpen)
	{
		Killed(nullptr, GIB_NORMAL);
		return;
	}

	UTIL_MakeVectors(pev->v_angle);

	SetBoneController(0, UTIL_VecToYaw(gpGlobals->v_forward));
	SetBoneController(1, 0);
	SetBoneController(2, 0);
	SetBoneController(3, 0);
	SetBoneController(4, 0);

	DROP_TO_FLOOR(edict());

	SetThink(&CMonster::IdleThink);
	pev->nextthink = gpGlobals->time + RANDOM_FLOAT(0.1, 0.2);

	m_flNextFullThink = gpGlobals->time + RANDOM_FLOAT(0.1, 0.2);
	m_vStart = pev->origin;
	m_vStartAngles = pev->angles;
	m_vOldPos = Vector(9999, 9999, 9999);
	m_iHostageIndex = ++g_iHostageNumber;

	nTargetNode = -1;
	m_fHasPath = FALSE;

	m_flLastPathCheck = -1;
	m_flPathAcquired = -1;
	m_flPathCheckInterval = 3.0f;
	m_flNextRadarTime = gpGlobals->time + RANDOM_FLOAT(0, 1);

	m_LocalNav = new CLocalNav(this);
	m_bStuck = FALSE;
	m_flStuckTime = 0;

	if (m_improv)
		delete m_improv;
	m_improv = NULL;

	m_flNextAttack = 0;
	
	//CHostage::Spawn();
	pev->team = TEAM_TERRORIST; // allow bot attack...

	m_flAttackDamage = 1.0f;
	m_flTimeLastActive = gpGlobals->time;
}

void CMonster::Precache()
{
	PRECACHE_SOUND("zombi/zombi_death_1.wav");
	PRECACHE_SOUND("zombi/zombi_death_2.wav");
	PRECACHE_SOUND("zombi/zombi_hurt_01.wav");
	PRECACHE_SOUND("zombi/zombi_hurt_02.wav");
	PRECACHE_SOUND("zombi/zombi_attack_1.wav");
	PRECACHE_SOUND("zombi/zombi_attack_2.wav");
	PRECACHE_SOUND("zombi/zombi_attack_3.wav");
	PRECACHE_MODEL("models/player/zombi_origin/zombi_origin.mdl");
}

void CMonster::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{ 
	// skip CHostage::Use
	return CBaseMonster::Use(pActivator, pCaller, useType, value); 
}

void CMonster::Touch(CBaseEntity *pOther)
{
	//return CHostage::Touch(pOther);

	if (pOther->IsPlayer())
		return; // attack???

	if (!FClassnameIs(pOther->pev, "monster_entity"))
		return;

	if (m_improv != NULL)
	{
		m_improv->OnTouch(pOther);
		return;
	}
}

int CMonster::TakeDamage(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType)
{
	float flActualDamage;
	CBasePlayer *pAttacker = NULL;

	flActualDamage = GetModifiedDamage(flDamage, m_LastHitGroup);

	if (pevAttacker != NULL)
	{
		CBaseEntity *pAttackingEnt = GetClassPtr((CBaseEntity *)pevAttacker);

		if (pAttackingEnt->Classify() == CLASS_VEHICLE)
		{
			CBaseEntity *pDriver = ((CFuncVehicle *)pAttackingEnt)->m_pDriver;

			if (pDriver != NULL)
			{
				pevAttacker = pDriver->pev;
			}
		}

		if (pAttackingEnt->IsPlayer())
		{
			pAttacker = GetClassPtr((CBasePlayer *)pevAttacker);
		}
	}

	flActualDamage = g_pModRunning->GetAdjustedEntityDamage(this, pevInflictor, pevAttacker, flActualDamage, bitsDamageType);

	if (flActualDamage > pev->health)
		flActualDamage = pev->health;

	pev->health -= flActualDamage;

	if (m_improv != NULL)
	{
		m_improv->OnInjury(flActualDamage);
	}

	PlayPainSound();
	m_flTimeLastActive = gpGlobals->time;

	if (pev->health > 0)
	{
		// if attacking then dont play flinch anim
		if (m_flNextAttack <= gpGlobals->time)
		{
			m_flFlinchTime = gpGlobals->time + 0.7;
			//SetFlinchActivity();
			SetAnimation(MONSTERANIM_FLINCH);
		}

		if (pAttacker != NULL)
		{
			// Player damaged monster
			return 1;
		}
	}
	else
	{
		Killed(pevAttacker, GIB_NORMAL);
	}

	return 0;
}

void CMonster::PlayDeathSound()
{
	switch (RANDOM_LONG(1, 2))
	{
	case 1: EMIT_SOUND(ENT(pev), CHAN_VOICE, "zombi/zombi_death_1.wav", VOL_NORM, ATTN_NORM); break;
	case 2: EMIT_SOUND(ENT(pev), CHAN_VOICE, "zombi/zombi_death_2.wav", VOL_NORM, ATTN_NORM); break;
	}
}

void CMonster::BecomeDead(void) 
{ 
	pev->health = 0;
	pev->movetype = MOVETYPE_TOSS;
	pev->flags &= ~FL_ONGROUND;

	pev->takedamage = DAMAGE_NO;
	pev->deadflag = DEAD_DEAD;
	pev->solid = SOLID_NOT;
}

void CMonster::Killed(entvars_t *pevAttacker, int iGib)
{
	if (!IsAlive())
		return;
	// unsigned int cCount = 0;
	// BOOL fDone = FALSE;

	// clear the deceased's sound channels.(may have been firing or reloading when killed)
	EMIT_SOUND(ENT(pev), CHAN_WEAPON, "common/null.wav", VOL_NORM, ATTN_NORM);
	m_IdealMonsterState = MONSTERSTATE_DEAD;
	// Make sure this condition is fired too (TakeDamage breaks out before this happens on death)
	SetConditions(bits_COND_LIGHT_DAMAGE);

	// tell owner ( if any ) that we're dead.This is mostly for MonsterMaker functionality.
	CBaseEntity *pOwner = CBaseEntity::Instance(pev->owner);
	if (pOwner)
	{
		pOwner->DeathNotice(pev);
	}

	SetTouch(NULL);
	BecomeDead();

	//pev->enemy = ENT(pevAttacker);//why? (sjb)
	m_IdealMonsterState = MONSTERSTATE_DEAD;

	//SetDeathActivity();
	SetAnimation(MONSTERANIM_DIE);

	pev->nextthink = gpGlobals->time + 3;
	SetThink(&CMonster::Remove);

	m_flTimeLastActive = -1;

	if (pevAttacker) // pevAttacker can be NULL
	{
		CBaseEntity *attacker = CBaseEntity::Instance(pevAttacker);
		if (attacker->IsPlayer())
			KilledByPlayer(static_cast<CBasePlayer *>(attacker));
	}
	
}

void CMonster::Remove()
{
	pev->movetype = MOVETYPE_NONE;
	pev->solid = SOLID_NOT;
	pev->takedamage = DAMAGE_NO;

	UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));
	pev->nextthink = -1;
	m_flNextFullThink = -1;

	

	SUB_Remove();
}

CMonster::CMonster() : CHostage(), m_iKillBonusMoney(500), m_iKillBonusFrags(1), m_flTimeLastActive(0.0f)
{
	MonsterManager().OnEntityAdd(this);
}

CMonster::~CMonster()
{
	MonsterManager().OnEntityRemove(this);
}

void CMonster::IdleThink()
{
	float flInterval;
	const float upkeepRate = 0.03f;
	const float giveUpTime = (1 / 30.0f);
	float const updateRate = 0.1f;

	if (!m_improv)
	{
		m_improv = new CMonsterImprov(this);
	}

	pev->nextthink = gpGlobals->time + giveUpTime;

	flInterval = StudioFrameAdvance(0);
	DispatchAnimEvents(flInterval);

	if (m_improv != NULL)
	{
		m_improv->OnUpkeep(upkeepRate);
	}

	if (m_flNextFullThink > gpGlobals->time)
	{
		return;
	}

	// sth to be inserted

	m_flNextFullThink = gpGlobals->time + 0.1;

	if (m_hTargetEnt != NULL || m_improv != NULL)
	{
		CBasePlayer *player = NULL;

		if (m_improv != NULL)
		{
			if (m_improv->IsFollowing())
			{
				player = (CBasePlayer *)m_improv->GetFollowLeader();
			}
		}
		else
			player = GetClassPtr((CBasePlayer *)m_hTargetEnt->pev);

		// force repath
		if (gpGlobals->time > m_flLastPathCheck + m_flPathCheckInterval)
		{
			// forget the current target and find a new one.
			player = NULL;
			m_flLastPathCheck = gpGlobals->time;
		}

		// invalid target, find another one!
		if (player == NULL || !player->IsAlive())
			player = (CBasePlayer *)FindTarget();

		// fix: no target needs, should still attack (to wall, etc)
		//if (player != NULL)
		CheckAttack();
	}

	if (m_improv != NULL)
	{
		m_improv->OnUpdate(updateRate);
	}

	// sequence settings.
	if (gpGlobals->time >= m_flFlinchTime)
	{
		if (pev->velocity.Length() > 15)
		{
			SetAnimation(MONSTERANIM_WALK);
		}
		else
		{
			SetAnimation(MONSTERANIM_IDLE);
		}
	}
}

CBaseEntity *CMonster::FindTarget()
{
	CBasePlayer *player = NULL;
	if (player = m_improv->GetClosestVisiblePlayer(TEAM_CT))
	{
		m_improv->Follow(player);
		m_hTargetEnt = player;
		m_improv->SetFollowRange(9.9999998e10f, 3000.0f, 20.0f);
		m_flTimeLastActive = gpGlobals->time;
	}
	else if (player = m_improv->GetClosestPlayerByTravelDistance(TEAM_CT))
	{
		m_improv->MoveTo(player->Center());
		m_improv->SetFollowRange(9.9999998e10f, 3000.0f, 20.0f);
	}
	else
	{
		// TODO : no player found, wander around
		Wander();
	}

	return player;
}

CBaseEntity *CMonster::CheckAttack()
{
	const float flAttackDistance = 35.0f;
	const float flAttackRate = 2.0f;
	const float flAttackAnimTime = 0.6f;
	const float flAttackDamage = m_flAttackDamage;

	if (m_flNextAttack > gpGlobals->time)
		return nullptr;

	CBaseEntity *pHit = CheckTraceHullAttack(flAttackDistance, flAttackDamage, DMG_BULLET);

	if (!pHit)
		return nullptr;

	SetAnimation(MONSTERANIM_ATTACK);

	m_flNextAttack = gpGlobals->time + flAttackRate;
	m_flNextFullThink = gpGlobals->time + flAttackAnimTime;

	switch (RANDOM_LONG(1, 3))
	{
	case 1: EMIT_SOUND(ENT(pev), CHAN_VOICE, "zombi/zombi_attack_1.wav", VOL_NORM, ATTN_NORM); break;
	case 2: EMIT_SOUND(ENT(pev), CHAN_VOICE, "zombi/zombi_attack_2.wav", VOL_NORM, ATTN_NORM); break;
	case 3: EMIT_SOUND(ENT(pev), CHAN_VOICE, "zombi/zombi_attack_3.wav", VOL_NORM, ATTN_NORM); break;
	}
	m_flTimeLastActive = gpGlobals->time;
	return pHit;
}

void CMonster::Wander()
{
	if (m_improv)
	{
		CBaseEntity *target = NULL;

		float shorestDistance = 9.9999998e10f;
		CBaseEntity *shorestTarget = NULL;

		while ((target = UTIL_FindEntityByClassname(target, "func_buyzone")) != NULL)
		{
			ShortestPathCost cost;
			Vector vecCenter = target->Center();

			float range = NavAreaTravelDistance(m_improv->GetLastKnownArea(), TheNavAreaGrid.GetNearestNavArea(&vecCenter), cost);

			if (range < shorestDistance)
			{
				shorestDistance = range;
				shorestTarget = target;
			}
		}

		if (shorestTarget)
		{
			m_improv->MoveTo(shorestTarget->Center());
			m_improv->SetFollowRange(6000.0f, 3000.0f, 50.0f);
			return;
		}
	}

	Wiggle();
}

void CMonster::SetAnimation(MonsterAnim anim) // similar to CBasePlayer::SetAnimation
{
	int animDesired;
	float speed;
	char szAnim[64];

	if (!pev->modelindex)
		return;

	if (anim != MONSTERANIM_FLINCH && anim != MONSTERANIM_LARGE_FLINCH && m_flFlinchTime > gpGlobals->time && pev->health > 0.0f)
		return;

	speed = pev->velocity.Length2D();

	if (pev->flags & FL_FROZEN)
	{
		speed = 0;
		anim = MONSTERANIM_IDLE;
	}

	switch (anim)
	{
	case MONSTERANIM_JUMP:
	{
		if (m_Activity == ACT_SWIM || m_Activity == ACT_DIESIMPLE || m_Activity == ACT_HOVER)
			m_IdealActivity = m_Activity;
		else
			m_IdealActivity = ACT_HOP;
		break;
	}
	case MONSTERANIM_DIE:
	{
		m_IdealActivity = ACT_DIESIMPLE;
		PlayDeathSound();
		break;
	}
	case MONSTERANIM_ATTACK:
	{
		if (m_Activity == ACT_SWIM || m_Activity == ACT_DIESIMPLE || m_Activity == ACT_HOVER)
			m_IdealActivity = m_Activity;
		else
			m_IdealActivity = ACT_RANGE_ATTACK1;
		break;
	}
	case MONSTERANIM_IDLE:
		if (pev->flags & FL_ONGROUND || (m_Activity != ACT_HOP && m_Activity != ACT_LEAP))
		{
			if (pev->waterlevel <= 1)
				m_IdealActivity = ACT_IDLE;

			else if (speed == 0.0f)
				m_IdealActivity = ACT_HOVER;

			else
				m_IdealActivity = ACT_SWIM;
		}
		else
			m_IdealActivity = m_Activity;
		break;
	case MONSTERANIM_WALK:
	{
		if (pev->flags & FL_ONGROUND || (m_Activity != ACT_HOP && m_Activity != ACT_LEAP))
		{
			if (pev->waterlevel <= 1)
				m_IdealActivity = ACT_WALK;

			else if (speed == 0.0f)
				m_IdealActivity = ACT_HOVER;

			else
				m_IdealActivity = ACT_SWIM;
		}
		else
			m_IdealActivity = m_Activity;
		break;
	}
	case MONSTERANIM_FLINCH:
		m_IdealActivity = ACT_FLINCH;
		break;
	case MONSTERANIM_LARGE_FLINCH:
		m_IdealActivity = ACT_LARGE_FLINCH;
		break;
	default:
		break;
	}

	switch (m_IdealActivity)
	{
	case ACT_HOP:
	case ACT_LEAP:
	{
		if (m_Activity == m_IdealActivity)
			return;

		animDesired = LookupSequence("zbs_jump");
		if (animDesired == -1)
			animDesired = 0;

		if (pev->sequence != animDesired || !m_fSequenceLoops)
			pev->frame = 0;

		if (!m_fSequenceLoops)
			pev->effects |= EF_NOINTERP;

		m_Activity = m_IdealActivity;
		break;
	}
	case ACT_RANGE_ATTACK1:
	{
		if (speed == 0.0f)
			Q_strcpy(szAnim, "ref_shoot_knife");
		else if (speed < 135.0f)
			Q_strcpy(szAnim, "zbs_attack_walk");
		else
			Q_strcpy(szAnim, "zbs_attack1_run");

		animDesired = LookupSequence(szAnim);
		if (animDesired == -1)
			animDesired = 0;

		pev->sequence = animDesired;
		pev->frame = 0;

		ResetSequenceInfo();
		m_Activity = m_IdealActivity;
		break;
	}
	case ACT_RANGE_ATTACK2:
	{
		if (speed < 15.0f)
			Q_strcpy(szAnim, "ref_shoot_knife");
		else if (speed < 135.0f)
			Q_strcpy(szAnim, "zbs_attack_walk");
		else
			Q_strcpy(szAnim, "zbs_attack2_run");

		animDesired = LookupSequence(szAnim);
		if (animDesired == -1)
			animDesired = 0;

		pev->sequence = animDesired;
		pev->frame = 0;

		ResetSequenceInfo();
		m_Activity = m_IdealActivity;
		break;
	}
	case ACT_IDLE:
	case ACT_RUN:
	case ACT_WALK:
	{
		if ((m_Activity != ACT_RANGE_ATTACK1 || m_fSequenceFinished)
			&& (m_Activity != ACT_RANGE_ATTACK2 || m_fSequenceFinished)
			&& (m_Activity != ACT_FLINCH || m_fSequenceFinished)
			&& (m_Activity != ACT_LARGE_FLINCH || m_fSequenceFinished)
			&& (m_Activity != ACT_RELOAD || m_fSequenceFinished))
		{
			if (speed < 15.0f)
				Q_strcpy(szAnim, "zbs_idle1");
			else if (speed < 135.0f)
				Q_strcpy(szAnim, "zbs_walk");
			else
				Q_strcpy(szAnim, "zbs_run");

			animDesired = LookupSequence(szAnim);
			if (animDesired == -1)
				animDesired = 0;

			m_Activity = m_IdealActivity;
		}
		else
			animDesired = pev->sequence;
		break;
	}
	case ACT_FLINCH:
	case ACT_LARGE_FLINCH:
	{
		m_Activity = m_IdealActivity;

		switch (m_LastHitGroup)
		{
		case HITGROUP_GENERIC:
		{
			if (RANDOM_LONG(0, 1))
				animDesired = LookupSequence("head_flinch");
			else
				animDesired = LookupSequence("gut_flinch");
			break;
		}
		case HITGROUP_HEAD:
		case HITGROUP_CHEST:
			animDesired = LookupSequence("head_flinch");
			break;
		case HITGROUP_SHIELD:
			animDesired = 0;
			break;
		default:
			animDesired = LookupSequence("gut_flinch");
			break;
		}

		if (animDesired == -1)
			animDesired = 0;

		break;
	}
	case ACT_DIESIMPLE:
	{
		if (m_Activity == m_IdealActivity)
			return;

		m_Activity = m_IdealActivity;

		switch (m_LastHitGroup)
		{
		case HITGROUP_GENERIC:
		{
			switch (RANDOM_LONG(0, 8))
			{
			case 0:
				animDesired = LookupActivity(ACT_DIE_HEADSHOT);
				break;
			case 1:
				animDesired = LookupActivity(ACT_DIE_GUTSHOT);
				break;
			case 2:
				animDesired = LookupActivity(ACT_DIE_BACKSHOT);
				break;
			case 3:
				animDesired = LookupActivity(ACT_DIESIMPLE);
				break;
			case 4:
				animDesired = LookupActivity(ACT_DIEBACKWARD);
				break;
			case 5:
				animDesired = LookupActivity(ACT_DIEFORWARD);
				break;
			case 6:
				animDesired = LookupActivity(ACT_DIE_CHESTSHOT);
				break;
			case 7:
				animDesired = LookupActivity(ACT_DIE_GUTSHOT);
				break;
			case 8:
				animDesired = LookupActivity(ACT_DIE_HEADSHOT);
				break;
			default:
				break;
			}
			break;
		}
		case HITGROUP_HEAD:
			animDesired = LookupActivity(ACT_DIE_HEADSHOT);
			break;
		case HITGROUP_CHEST:
			animDesired = LookupActivity(ACT_DIE_CHESTSHOT);
			break;
		case HITGROUP_STOMACH:
			animDesired = LookupActivity(ACT_DIE_GUTSHOT);
			break;
		case HITGROUP_LEFTARM:
			animDesired = LookupSequence("left");
			break;
		case HITGROUP_RIGHTARM:
			animDesired = LookupSequence("right");
			break;
		default:
			animDesired = LookupActivity(ACT_DIESIMPLE);
			break;
		}

		if (animDesired == -1)
			animDesired = 0;

		if (pev->sequence != animDesired)
		{
			pev->sequence = animDesired;
			pev->frame = 0.0f;
			ResetSequenceInfo();
		}
		return;
	}
	default:
	{
		if (m_Activity == m_IdealActivity)
			return;

		m_Activity = m_IdealActivity;
		animDesired = LookupActivity(m_IdealActivity);

		if (pev->sequence != animDesired)
		{
			pev->sequence = animDesired;
			pev->frame = 0;

			ResetSequenceInfo();
		}
		return;
	}
	}
	
	if (pev->sequence != animDesired)
	{
		pev->sequence = animDesired;
		pev->frame = 0;

		ResetSequenceInfo();
	}

}

inline float CalcBoxDistance(CBaseEntity *ent1, CBaseEntity *ent2)
{
	/*Vector vecDistance = Vector(0, 0, 0);
	for (int i = 0; i < 3; i++)
	{
		if (((float *)pev->absmin)[i] > ((float *)pevTarget->absmax)[i])
			(float)(vecDistance)[i] = ((float *)pev->absmin)[i] - ((float *)pevTarget->absmax)[i];
		else if (((float *)pevTarget->absmin)[i] > ((float *)pev->absmax)[i])
			(float)(vecDistance)[i] = ((float *)pevTarget->absmin)[i] - ((float *)pev->absmax)[i];
	}
	return vecDistance.Length();*/
	return (ent1->Center() - ent2->Center()).Length();
}

CBaseEntity *CMonster::CheckTraceHullAttack(float flDist, int iDamage, int iDmgType)
{
	TraceResult tr;

	if (IsPlayer())
		UTIL_MakeVectors(pev->angles);
	else
		UTIL_MakeAimVectors(pev->angles);

	Vector vecStart = pev->origin;
	Vector vecEnd = vecStart + (gpGlobals->v_forward * flDist);

	UTIL_TraceHull(vecStart, vecEnd, dont_ignore_monsters, head_hull, ENT(pev), &tr);

	if (tr.pHit)
	{
		CBaseEntity *pEntity = CBaseEntity::Instance(tr.pHit);

		if (!ShouldAttack(pEntity))
			return nullptr;

		if (iDamage > 0)
		{
			pEntity->TakeDamage(pev, pev, iDamage, iDmgType);
		}

		return pEntity;
	}

	return NULL;
}

bool CMonster::ShouldAttack(CBaseEntity *target)
{
	if (target->pev->takedamage == DAMAGE_NO)
		return false;

	if (target->IsPlayer() && g_pGameRules->PlayerRelationship(static_cast<CBasePlayer *>(target), this) != GR_TEAMMATE)
		return true;

	CZBSBreak *zbs_break = dynamic_cast<CZBSBreak *>(target);
	if (zbs_break)
		return zbs_break->m_flZombiDamageRatio > 0.0f;

	return false;
}

void CMonster::KilledByPlayer(CBasePlayer *player)
{
	if (m_iKillBonusFrags)
		player->AddPoints(m_iKillBonusFrags, FALSE);
	if(m_iKillBonusMoney)
		player->AddAccount(m_iKillBonusMoney);

	MESSAGE_BEGIN(MSG_ONE, gmsgZBSTip, NULL, player->pev);
	WRITE_BYTE(ZBS_TIP_KILL);
	MESSAGE_END();
}
