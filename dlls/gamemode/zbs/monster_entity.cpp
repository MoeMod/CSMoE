
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "schedule.h"
#include "player.h"
#include "monsters.h"
#include "trains.h"

#include "monster_entity.h"
#include "zs_subs.h"
#include "cbase/cbase_hash.h"

#include "game.h"
#include "bot_include.h" // nav

#include "zbs_const.h"
#include "monster_manager.h"
#include "gamemode/mods.h"
#include "gamemode/zb2/deimostail.h"
#include "gamemode/zb2/zombietrap.h"
#include <future>
#include <atomic>
#include <algorithm>
#include <vector>

namespace sv {

template<class T>
std::shared_ptr<CBaseMonsterClass> MakeMonsterClass(CMonster* p)
{
	return std::make_shared<T>(p);
}
const std::tuple<MonsterClass, unsigned int, std::shared_ptr<CBaseMonsterClass>(*)(CMonster*)> g_MonsterClassList[] = {
	{MONSTER_CLASS_ZOMBIE, 100, MakeMonsterClass<CMonsterClass_Normal>},
	{MONSTER_CLASS_ZOMBIEDOG, 100, MakeMonsterClass<CMonsterClass_Dog>},
#ifdef XASH_DEDICATED
	{MONSTER_CLASS_SPEED, 100, MakeMonsterClass<CMonsterClass_Speed>},
	{MONSTER_CLASS_HEAVY, 100, MakeMonsterClass<CMonsterClass_Heavy>},
	{MONSTER_CLASS_THROWER, 100, MakeMonsterClass<CMonsterClass_Thrower>},
	{MONSTER_CLASS_HEAL, 100, MakeMonsterClass<CMonsterClass_Heal>},
	{MONSTER_CLASS_BOOMER, 100, MakeMonsterClass<CMonsterClass_Boomer>},
	{MONSTER_CLASS_RESIDENT, 100, MakeMonsterClass<CMonsterClass_Resident>},
	{MONSTER_CLASS_DEIMOS, 100, MakeMonsterClass<CMonsterClass_Deimos>},
	{MONSTER_CLASS_HELMET, 100, MakeMonsterClass<CMonsterClass_Helmet>},
	{MONSTER_CLASS_DEIMOS2, 100, MakeMonsterClass<CMonsterClass_Deimos2>},
#endif
};

constexpr auto NUM_MONSTERCLASSES = std::extent<decltype(g_MonsterClassList)>::value;

template <size_t ...N>
constexpr std::array<unsigned int, NUM_MONSTERCLASSES> MakeMClassWeightList(std::index_sequence<N...>) {
	return { std::get<1>(g_MonsterClassList[N])... };
}
auto MCLASSES_WEIGHT = MakeMClassWeightList(std::make_index_sequence<NUM_MONSTERCLASSES>{});

std::shared_ptr<CBaseMonsterClass> CMonsterModStrategy_Default::MonsterClassFactory(MonsterClass iClass)
{
	if (iClass == MONSTER_CLASS_RANDOM)
	{
		std::random_device rd;
		std::mt19937 gen(rd());
		const std::size_t N = std::discrete_distribution<>(MCLASSES_WEIGHT.begin(), MCLASSES_WEIGHT.end())(gen);
		m_pMonster->m_iClass = std::get<0>(g_MonsterClassList[N]);

		return std::get<2>(g_MonsterClassList[N])(m_pMonster);
	}

	for (auto&& pr : g_MonsterClassList)
	{
		if (iClass == std::get<0>(pr))
		{
			m_pMonster->m_iClass = iClass;
			return std::get<2>(pr)(m_pMonster);
		}
			
	}

	return std::get<2>(g_MonsterClassList[0])(m_pMonster);
}

LINK_ENTITY_TO_CLASS(monster_entity, CMonster);

class CMonsterImprov : public CHostageImprov
{
public:
	CMonsterImprov(CBaseEntity *entity) : CHostageImprov(entity), 
		m_bCalculatingPath(false)
	{
	}
	~CMonsterImprov() {} // virtual

	// remove some hostage staffs
	void Crouch() override { /* no code needed */ }
	void OnMoveToFailure(const Vector &goal, MoveToFailureType reason) override { /* no code needed */ }
	bool IsTerroristNearby() override { return false; }

	// jumping control
	bool GetSimpleGroundHeightWithFloor(const Vector *pos, float *height, Vector *normal) override
	{
		Vector to(*pos);
		to.z -=  9999.9f;

		TraceResult tr;

		UTIL_TraceLine(*pos, to, ignore_monsters, dont_ignore_glass, NULL, &tr);

		if (tr.fStartSolid)
			return false;

		*height = tr.vecEndPos.z;

		if (normal != NULL)
		{
			*normal = tr.vecPlaneNormal;
		}

		UTIL_MakeVectors(m_hostage->pev->angles);

		// no need to jump down !
		if (DotProduct2D(tr.vecPlaneNormal, gpGlobals->v_forward) > 0)
			return false;

		if (m_lastKnownArea != NULL && m_lastKnownArea->IsOverlapping(pos))
			*height = Q_max((*height), m_lastKnownArea->GetZ(pos));

		return true;
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

	// No animation update needed...
	void OnUpdate(float deltaT) override
	{
		if (!IsAlive())
			return;

		UpdateVision();

		m_behavior.Update();

		m_actualVel.x = m_hostage->pev->origin.x - m_lastPosition.x;
		m_actualVel.y = m_hostage->pev->origin.y - m_lastPosition.y;

		EngineClock::duration safeTime = 0.4s;

		if (!m_collisionTimer.HasStarted() || m_collisionTimer.IsGreaterThen(safeTime))
			SetKnownGoodPosition(m_lastPosition);

		m_lastPosition = m_hostage->pev->origin;
	}

	void OnUpkeep(float deltaT) override
	{
		if (IsAlive())
		{
			UpdatePosition(deltaT);
		}
	}

	void UpdatePosition(float deltaT)
	{
		CNavArea *area = TheNavAreaGrid.GetNavArea(&m_hostage->pev->origin);

		if (area != NULL)
		{
			m_lastKnownArea = area;
		}

		if (IsJumping() && CanJump())
		{
			Vector dir;
			const float pushSpeed = 100.0f;

			if (!m_hasJumped)
			{
				m_hasJumped = true;
				m_hasJumpedIntoAir = false;
				m_hostage->pev->velocity.z += 300.0f;
			}
			else
				ResetJump();

			dir = m_jumpTarget - GetFeet();
			dir.z = 0;

			dir.NormalizeInPlace();

			m_hostage->pev->velocity.x = dir.x * pushSpeed;
			m_hostage->pev->velocity.y = dir.y * pushSpeed;

			m_hostage->SetBoneController(0, 0.0f);
			m_hostage->SetBoneController(1, 0.0f);

			FaceTowards(m_jumpTarget, deltaT);
			return;
		}

		if (m_isLookingAt)
		{
			Vector angles = UTIL_VecToAngles(m_viewGoal - GetEyes());
			float pitch = angles.x - m_hostage->pev->angles.x;
			float yaw = angles.y - m_hostage->pev->angles.y;

			while (yaw > 180.0f)
				yaw -= 360.0f;

			while (yaw < -180.0f)
				yaw += 360.0f;

			while (pitch > 180.0f)
				pitch -= 360.0f;

			while (pitch < -180.0f)
				pitch += 360.0f;

			m_hostage->SetBoneController(0, yaw);
			m_hostage->SetBoneController(1, -pitch);

			if (IsAtMoveGoal() && !HasFaceTo())
			{
				if (yaw < -45.0f || yaw > 45.0f)
				{
					FaceTowards(m_viewGoal, deltaT);
				}
			}
		}
		else
		{
			m_hostage->SetBoneController(0, 0.0f);
			m_hostage->SetBoneController(1, 0.0f);
		}

		if (HasFaceTo() && FaceTowards(m_faceGoal, deltaT))
			ClearFaceTo();

		if (!IsAtMoveGoal() || m_path.GetSegmentCount() > 0)
		{
			if (m_path.GetSegmentCount() <= 0)
			{
				if (m_bCalculatingPath == false)
				{
					m_bCalculatingPath = true;
					m_fbCalcPathResult = std::async([&] {
						HostagePathCost pathCost;
						bool result = m_path.Compute(&GetFeet(), &m_moveGoal, pathCost);
						m_bCalculatingPath = false;
						return result;
					});
				}

				if(m_fbCalcPathResult.valid() && m_fbCalcPathResult.get())
				{
					m_follower.SetPath(&m_path);
					m_follower.SetImprov(this);

					m_follower.Reset();
					m_follower.Debug(cv_hostage_debug.value > 0.0);
				}
			}

			m_follower.Update(deltaT, m_inhibitObstacleAvoidance.IsElapsed());

			if (m_moveType == Stopped)
			{
				m_follower.ResetStuck();
			}

			if (m_follower.IsStuck())
			{
				Wiggle();
			}
		}

		const float friction = 3.0f;

		m_vel.x += m_vel.x * -friction * deltaT;
		m_vel.y += m_vel.y * -friction * deltaT;

		float speed = m_vel.NormalizeInPlace();

		//const float maxSpeed = 285.0f;
		const float maxSpeed = m_hostage->pev->maxspeed;
		if (speed > maxSpeed)
		{
			speed = maxSpeed;
		}

		m_vel.x = m_vel.x * speed;
		m_vel.y = m_vel.y * speed;

		/*KeepPersonalSpace spacer(this);
		ForEachPlayer(spacer);

		if (g_pHostages != NULL)
		{
			g_pHostages->ForEachHostage(spacer);
		}*/

		m_hostage->pev->velocity.x = m_vel.x;
		m_hostage->pev->velocity.y = m_vel.y;

		m_moveFlags = 0;
	}

	void OnTouch(CBaseEntity *other) override
	{
		m_collisionTimer.Start();

		if (FClassnameIs(other->pev, "worldspawn"))
		{
			const float lookAheadRange = 30.0f;
			float ground;
			Vector normal = Vector(0, 0, 1);
			Vector alongFloor;
			TraceResult result;
			bool isStep = false;

			UTIL_MakeVectors(m_hostage->pev->angles);

			if (!GetSimpleGroundHeightWithFloor(&GetEyes(), &ground, &normal))
				return;

			if (cv_hostage_debug.value < 0.0)
			{
				UTIL_DrawBeamPoints(GetFeet() + normal * 50, GetFeet(), 2, 255, 255, 0);
			}

			alongFloor = CrossProduct(normal, gpGlobals->v_right);

			Vector pos = alongFloor * lookAheadRange;

			for (double offset = 1.0f; offset <= 18.0f; offset += 3.0f)
			{
				Vector vecStart = GetFeet();
				vecStart.z += offset;

				UTIL_TraceLine(vecStart, vecStart + pos, ignore_monsters, dont_ignore_glass, m_hostage->pev->pContainingEntity, &result);

				if (result.flFraction < 1.0f && result.vecPlaneNormal[2] < 0.7f)
				{
					isStep = true;
					break;
				}
			}

			if (isStep)
			{
				float stepAheadGround = pos.z;
				Vector stepAheadNormal = Vector(0, 0, stepAheadGround);

				m_inhibitObstacleAvoidance.Start(0.5s);

				for (float range = 1.0f; range <= 30.5f; range += 5.0f)
				{
					Vector stepAhead = GetFeet() + alongFloor * range;
					stepAhead.z = GetEyes().z;

					if (GetSimpleGroundHeightWithFloor(&stepAhead, &stepAheadGround, &stepAheadNormal))
					{
						float dz = stepAheadGround - GetFeet().z;

						if (dz > 0.0f && dz < 18.0f)
						{
							m_hostage->pev->origin.z = stepAheadGround + 3.0f;
							break;
						}
					}
				}
			}
			else if (!IsMoving() && !IsUsingLadder())
			{
				bool isSeam = false;
				const float checkSeamRange = 50.0f;
				Vector posBehind;

				posBehind = GetEyes() - alongFloor * checkSeamRange;
				UTIL_TraceLine(posBehind, posBehind - Vector(0, 0, 9999), ignore_monsters, dont_ignore_glass, m_hostage->pev->pContainingEntity, &result);

				if (result.flFraction < 1.0f && DotProduct(result.vecPlaneNormal, normal) < 1.0f)
				{
					isSeam = true;
				}
				else
				{
					Vector posAhead = GetEyes() + alongFloor * checkSeamRange;
					UTIL_TraceLine(posAhead, posAhead - Vector(0, 0, 9999), ignore_monsters, dont_ignore_glass, m_hostage->pev->pContainingEntity, &result);

					if (result.flFraction < 1.0f && DotProduct(result.vecPlaneNormal, normal) < 1.0f)
						isSeam = true;
				}

				if (isSeam)
				{
					if (cv_hostage_debug.value != 0.0)
					{
						CONSOLE_ECHO("Hostage stuck on seam.\n");
					}

					const float nudge = 3.0f;
					m_hostage->pev->origin.z += nudge;
				}
			}
		}
		else if (FClassnameIs(other->pev, STRING(m_hostage->pev->classname)))
		{
			const float pushForce = 10.0f;
			Vector2D to = (m_hostage->pev->origin - other->pev->origin).Make2D();
			to.NormalizeInPlace();

			m_vel.x += to.x * pushForce;
			m_vel.y += to.y * pushForce;
		}
	}

	const Vector& GetCentroid() const override
	{
		auto pMonster = (CMonster*)m_hostage;
		auto p = (CMonsterModStrategy_Default*)pMonster->m_pMonsterStrategy.get();
		m_centroid = p->m_pClass->GetCenterid();
		return m_centroid;
	}

	CBasePlayer* GetClosestPlayerByTravelDistance(int team, float* retRange) override
	{
		CBasePlayer* close = NULL;
		float closeRange = 9.9999998e10f;

		if (GetLastKnownArea() == NULL)
			return NULL;

		for (int i = 1; i <= gpGlobals->maxClients; ++i)
		{
			CBasePlayer* player = static_cast<CBasePlayer*>(UTIL_PlayerByIndex(i));

			if (!IsEntityValid(player))
				continue;

			if (player->IsAlive() && (team == UNASSIGNED || player->m_iTeam == team))
			{
				if (m_bCalculatingPath == false)
				{
					m_bCalculatingPath = true;
					m_ffCalcDistResult = std::async([&] {
						Vector vecCenter = player->Center();
						ShortestPathCost cost;
						float result = NavAreaTravelDistance(GetLastKnownArea(), TheNavAreaGrid.GetNearestNavArea(&vecCenter), cost);
						m_bCalculatingPath = false;
						return result;
					});
				}

				if (m_ffCalcDistResult.valid())
				{
					float range = m_ffCalcDistResult.get();
					if (range > 0 && range < closeRange)
					{
						closeRange = range;
						close = player;
					}
				}
			}
		}

		if (retRange)
			*retRange = closeRange;

		return close;
	}

	bool IsFriendInTheWay() const override
	{
		CheckAhead check(this);
		CBaseEntity* pEntity = NULL;
		while ((pEntity = UTIL_FindEntityByClassname(pEntity, "monster_entity")) != NULL)
		{
			auto pMonster = (CMonster*)pEntity;
			if (pMonster == m_hostage) continue;
			if (!pMonster->IsAlive()) continue;
			if(check(pMonster)) continue;
			break;
		}
		ForEachPlayer(check);

		return check.IsBlocked();
	}

	std::atomic<bool> m_bCalculatingPath;
	std::future<bool> m_fbCalcPathResult;
	std::future<float> m_ffCalcDistResult;
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
	pev->maxspeed = 100.0f;

	if (pev->spawnflags & SF_MONSTER_HITMONSTERCLIP)
		pev->flags |= FL_MONSTERCLIP;

	if (pev->skin < 0)
		pev->skin = 0;

	SetAnimation(MONSTERANIM_IDLE);

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
	pev->nextthink = gpGlobals->time + RandomDuration(0.1s, 0.2s);
	m_flNextFullThink = gpGlobals->time + RandomDuration(0.1s, 0.2s);

	m_flNextChange = invalid_time_point;
	m_State = STAND;
	m_hTargetEnt = NULL;
	m_hStoppedTargetEnt = NULL;
	m_vPathToFollow[0] = Vector(0, 0, 0);
	m_flFlinchTime = invalid_time_point;
	m_bRescueMe = FALSE;

	m_vStart = pev->origin;
	m_vStartAngles = pev->angles;
	m_vOldPos = Vector(9999, 9999, 9999);
	m_iHostageIndex = ++g_iHostageNumber;

	nTargetNode = -1;
	m_fHasPath = FALSE;

	m_flLastPathCheck = invalid_time_point;
	m_flPathAcquired = invalid_time_point;
	m_flPathCheckInterval = 3.0s;
	m_flNextRadarTime = gpGlobals->time + RandomDuration(0.0s, 1.0s);

	m_LocalNav = new CLocalNav(this);
	m_bStuck = FALSE;
	m_flStuckTime = invalid_time_point;

	if (m_improv)
		delete m_improv;
	m_improv = NULL;

	m_timeNextAttack = invalid_time_point;

	pev->team = TEAM_TERRORIST; // allow bot attack...

	m_flAttackDist = 35.0f;
	m_flAttackDamage = 1.0f;
	m_flAttackRate = 2.0s;
	m_flAttackAnimTime = 0.6s;
	m_flTimeLastActive = gpGlobals->time;
	m_flCustomSequenceUntilTime = invalid_time_point;

	m_pMonsterStrategy->OnSpawn();
}

void CMonster::Precache()
{
	PRECACHE_SOUND("zombi/zbs_death_1.wav");
	PRECACHE_SOUND("zombi/zbs_death_female_1.wav");
	PRECACHE_SOUND("zombi/boomer_death.wav");
	PRECACHE_SOUND("zombi/zombiedog_death1.wav");
	PRECACHE_SOUND("zombi/zombiedog_death2.wav");
	PRECACHE_SOUND("zombi/zombi_hurt_01.wav");
	PRECACHE_SOUND("zombi/zombi_hurt_02.wav");
	PRECACHE_SOUND("zombi/zombiedog_hurt1.wav");
	PRECACHE_SOUND("zombi/zombiedog_hurt2.wav");
	PRECACHE_SOUND("zombi/zombi_attack_1.wav");
	PRECACHE_SOUND("zombi/zombi_attack_2.wav");
	PRECACHE_SOUND("zombi/zombi_attack_3.wav");

	PRECACHE_MODEL("models/player/zombi_origin/zombi_origin.mdl");
	PRECACHE_MODEL("models/monster/zombiedog.mdl");
}

void CMonster::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{ 
	// skip CHostage::Use
	return CBaseMonster::Use(pActivator, pCaller, useType, value); 
}

void CMonster::Touch(CBaseEntity *pOther)
{
	if (m_improv != NULL)
	{
		m_improv->OnTouch(pOther);
	}

	m_pMonsterStrategy->OnTouch(pOther);
}

float CMonster::GetModifiedDamage(float flDamage, int nHitGroup) const
{
	switch (nHitGroup)
	{
		case HITGROUP_GENERIC: flDamage *= 1; break;
		case HITGROUP_HEAD: flDamage *= 4; break;
		case HITGROUP_CHEST: flDamage *= 1; break;
		case HITGROUP_STOMACH: flDamage *= 1.25; break;
		case HITGROUP_LEFTARM:
		case HITGROUP_RIGHTARM: flDamage *= 1; break;
		case HITGROUP_LEFTLEG:
		case HITGROUP_RIGHTLEG: flDamage *= 0.75; break;
		default: flDamage *= 1; break;
	}

	return flDamage;
}

int CMonster::TakeDamage(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType)
{
	float flActualDamage;
	CBasePlayer *pAttacker = NULL;

	flActualDamage = GetModifiedDamage(flDamage, m_LastHitGroup);

	if (pevAttacker != NULL)
	{
		CBaseEntity *pAttackingEnt = GetClassPtr<CBaseEntity>(pevAttacker);

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
			pAttacker = GetClassPtr<CBasePlayer>(pevAttacker);
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
		if (m_timeNextAttack <= gpGlobals->time)
		{
			m_flFlinchTime = gpGlobals->time + 0.25s;
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
		CBasePlayer* pAttackPlayer = nullptr;

		if (pAttacker && pAttacker->IsPlayer())
			pAttackPlayer = static_cast<CBasePlayer*>(pAttacker);


		if (pAttackPlayer && FClassnameIs(this->pev, "monster_entity"))
		{
			CBasePlayerWeapon* pWeapon = (CBasePlayerWeapon*)pAttackPlayer->m_rgpPlayerItems[PRIMARY_WEAPON_SLOT];
			if (pWeapon)
			{
				if (pWeapon->m_iId == WEAPON_CHAINSR)
					pWeapon->EntityTakeDamageCallBack(WEAPON_CHAINSR, this, pevInflictor, pAttackPlayer, flDamage, bitsDamageType);
			}
		}

		if (this && FClassnameIs(this->pev, "monster_entity"))
		{
			if (this->m_vecTempAttackers.size())
			{
				for (std::vector<CBasePlayer*>::iterator iter = this->m_vecTempAttackers.begin(); iter != this->m_vecTempAttackers.end(); ++iter)
				{
					CBasePlayer* TragetPlayer = *iter;
					if (TragetPlayer && TragetPlayer->IsAlive())
					{
						CBasePlayerWeapon* pWeapon = (CBasePlayerWeapon*)TragetPlayer->m_rgpPlayerItems[PRIMARY_WEAPON_SLOT];
						if (TragetPlayer->pev != pevAttacker)
						{
							pWeapon = (CBasePlayerWeapon*)TragetPlayer->m_rgpPlayerItems[PRIMARY_WEAPON_SLOT];

							if (pWeapon && pWeapon->m_iId == WEAPON_CHAINSR)
							{
								pWeapon->WpnDeathCallBack(this, pevInflictor, pevAttacker, flDamage, bitsDamageType);
							}
						}
					}
				}
				this->m_vecTempAttackers.clear();
			}
		}

		Killed(pevAttacker, GIB_NORMAL);
	}

	return 0;
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

	pev->nextthink = gpGlobals->time + 3s;
	SetThink(&CMonster::Remove);

	m_flTimeLastActive = invalid_time_point;

	m_pMonsterStrategy->OnKilled(pevAttacker, iGib);
}

void CMonster::Remove()
{
	pev->movetype = MOVETYPE_NONE;
	pev->solid = SOLID_NOT;
	pev->takedamage = DAMAGE_NO;

	UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));
	pev->nextthink = invalid_time_point;
	m_flNextFullThink = invalid_time_point;

	SUB_Remove();
}

CMonster::CMonster() : CHostage(), 
	m_iKillBonusMoney(500), 
	m_iKillBonusFrags(1), 
	m_flTimeLastActive(0.0s), 
	m_iClass(MONSTER_CLASS_ZOMBIE),
	m_pMonsterStrategy(new CMonsterModStrategy_Default(this))
{
	m_pLifeCycleHolder = std::make_shared<CMonster *>(this);
	MonsterManager().OnEntityAdd(m_pLifeCycleHolder);
}

void CMonster::IdleThink()
{
	constexpr float upkeepRate = 0.03f;
	constexpr auto giveUpTime = (1 / 30.0f) * 1s;
	constexpr float updateRate = 0.1f;

	if (!m_improv)
	{
		m_improv = new CMonsterImprov(this);
	}

	if (m_iHealthDecreaseCount && gpGlobals->time > m_flHealthDecreaseStartTime)
	{
		pev->health -= m_flHealthDecreaseAmount;

		if (m_pBuffAttacker)
		{
			MESSAGE_BEGIN(MSG_ONE, gmsgHitMsg, NULL, m_pBuffAttacker);
			WRITE_LONG((long)m_flHealthDecreaseAmount);
			WRITE_SHORT(ENTINDEX(edict()));
			WRITE_BYTE(0);
			MESSAGE_END();
		}

		if (pev->health > 0)
		{
			m_iHealthDecreaseCount--;
			m_flHealthDecreaseStartTime = gpGlobals->time + m_flHealthDecreaseInterval;

			if (!m_iHealthDecreaseCount)
			{
				m_flHealthDecreaseStartTime = invalid_time_point;
				m_flHealthDecreaseAmount = 0;
				m_flHealthDecreaseInterval = 0.0s;
				m_pBuffAttacker = nullptr;
			}
		}
		else
		{
			m_iHealthDecreaseCount = 0;
			pev->health = 1;

			m_flHealthDecreaseStartTime = invalid_time_point;
			m_flHealthDecreaseAmount = 0;
			m_flHealthDecreaseInterval = 0.0s;
			m_pBuffAttacker = nullptr;
		}
	}

	pev->nextthink = gpGlobals->time + giveUpTime;

	// Outer...
	EngineClock::duration flInterval = StudioFrameAdvance(0s);
	std::future<void> handleDispatchAnimEvents = std::async(&CMonster::DispatchAnimEvents, this, flInterval);
	if (m_flStop > gpGlobals->time)
	{
		handleDispatchAnimEvents.get();
		m_pMonsterStrategy->OnThink();
		return;
	}

	std::future<void> handleUpkeepImprov = std::async(&CHostageImprov::OnUpkeep, m_improv, upkeepRate);
	// wait for them...
	handleDispatchAnimEvents.get();
	handleUpkeepImprov.get();

	if (gpGlobals->time > m_flNextFullThink)
	{
		m_flNextFullThink = gpGlobals->time + 0.1s;
		std::future<bool> results[] = {
			std::async(&CMonster::CheckTarget, this),
			std::async(&CMonster::CheckAttack, this),
			std::async(&CMonster::CheckSequence, this),
			std::async([=] {return m_improv->OnUpdate(updateRate), false; })
		};

		bool bActive = false;
		for (auto &&f : results)
			bActive |= f.get();
		if (bActive)
			m_flTimeLastActive = gpGlobals->time;
	}

	m_pMonsterStrategy->OnThink();
}

bool CMonster::CheckTarget()
{
	if (m_flTargetChange <= gpGlobals->time)
	{
		auto result = FindTarget();

		CBasePlayer *player = result.first;

		if(!player)
		{
			Wander();
			m_hTargetEnt = player;
		}
		else if (result.second)
		{
			m_improv->Follow(player);
			m_improv->SetFollowRange(6000.0f, 3000.0f, 20.0f);
			m_hTargetEnt = player;
		}
		else
		{
			m_hTargetEnt = player;
			m_improv->MoveTo(player->Center());
		}

		m_flTargetChange = gpGlobals->time + RandomDuration(10.0s, 20.0s);
		return result.second;
	}

	if (m_hTargetEnt && !m_hTargetEnt->IsAlive())
	{
		m_flTargetChange = gpGlobals->time + 0.2s;
		return false;
	}
	return false;
}

std::pair<CBasePlayer *, bool> CMonster::FindTarget() const
{
	std::future<CBasePlayer *> fpNear = std::async(&CMonster::GetClosestPlayer, this, true);
	// TODO: fix for async
	//std::future<CBasePlayer*> fpFar = std::async(&CMonsterImprov::GetClosestPlayerByTravelDistance, m_improv, UNASSIGNED, nullptr);
	std::future<CBasePlayer*> fpFar = std::async(&CMonster::GetClosestPlayer, this, false);

	CBasePlayer *player = nullptr;

	if ((player = fpNear.get()))
	{
		// abandon fpFar...
		return { player, true };
	}
	else if ((player = fpFar.get()))
	{
		return { player, false };
	}

	return { nullptr, false};
}

bool CMonster::CheckAttack()
{
	if (m_flSkillCheck <= gpGlobals->time)
	{
		m_flSkillCheck = gpGlobals->time + 2.0s;
		auto p = (CMonsterModStrategy_Default*)m_pMonsterStrategy.get();
		if (p->m_pSkill && p->m_pSkill->GetStatus() == SKILL_STATUS_READY) {
			p->m_pSkill->OnActivate(m_hTargetEnt);
		}
	}

	if (m_timeNextAttack > gpGlobals->time || !m_flAttackDamage)
		return false;

	if (!m_hTargetEnt)
		return false;

	if (!m_hTargetEnt->IsAlive())
		return false;

	if ((m_hTargetEnt->pev->origin - pev->origin).IsLengthGreaterThan(m_flAttackDist))
		return false;

	CBaseEntity *pHit = CheckTraceHullAttack(m_flAttackDist, m_flAttackDamage, DMG_BULLET);

	if (!pHit)
		return false;

	SetAnimation(MONSTERANIM_ATTACK);

	m_timeNextAttack = gpGlobals->time + m_flAttackRate;
	m_flNextFullThink = gpGlobals->time + m_flAttackAnimTime;

	switch (RANDOM_LONG(1, 3))
	{
	case 1: EMIT_SOUND(ENT(pev), CHAN_BODY, "zombi/zombi_attack_1.wav", VOL_NORM, ATTN_NORM); break;
	case 2: EMIT_SOUND(ENT(pev), CHAN_BODY, "zombi/zombi_attack_2.wav", VOL_NORM, ATTN_NORM); break;
	case 3: EMIT_SOUND(ENT(pev), CHAN_BODY, "zombi/zombi_attack_3.wav", VOL_NORM, ATTN_NORM); break;
	}

	return true;
}

bool CMonster::CheckSequence()
{
	// sequence settings.
	if (gpGlobals->time >= m_flFlinchTime)
	{
		if (pev->velocity.IsLengthGreaterThan(15))
		{
			SetAnimation(MONSTERANIM_WALK);
		}
		else
		{
			SetAnimation(MONSTERANIM_IDLE);
		}
	}
	return false;
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
}

void CMonster::SetAnimation(MonsterAnim anim) // similar to CBasePlayer::SetAnimation
{
	int animDesired = 0;
	float speed;
	char szAnim[64];

	if (!pev->modelindex)
		return;

	if (pev->health > 0)
	{
		if (m_flCustomSequenceUntilTime > gpGlobals->time && anim != MONSTERANIM_DIE)
		{
			if (m_iCustomSequence != -1)
			{
				if (pev->sequence != m_iCustomSequence)
				{
					pev->sequence = m_iCustomSequence;
					pev->frame = 0;

					ResetSequenceInfo();
				}
			}
			return;
		}
	}

	if (anim != MONSTERANIM_FLINCH && anim != MONSTERANIM_LARGE_FLINCH && m_flFlinchTime > gpGlobals->time && pev->health > 0.0f)
		return;

	// may be called from both CheckAttack and CheckSequence
	std::lock_guard<std::mutex> lock(m_mutexSetAnimation);

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
		m_pMonsterStrategy->DeathSound();
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
		{
			Q_strcpy(szAnim, "zbs_attack_walk");	//zombiedog dont have ref_shoot_knife
			animDesired = LookupSequence(szAnim);
			if (animDesired == -1)
			{
				animDesired = 0;
			}
		}
			
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
		{
			Q_strcpy(szAnim, "zbs_attack_walk");	//zombiedog dont have ref_shoot_knife
			animDesired = LookupSequence(szAnim);
			if (animDesired == -1)
			{
				animDesired = 0;
			}
		}

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

		break;
	}
	default:
	{
		if (m_Activity == m_IdealActivity)
			return;

		m_Activity = m_IdealActivity;
		animDesired = LookupActivity(m_IdealActivity);

		break;
	}
	}

	if (animDesired == -1)
		animDesired = 0;

	if (pev->sequence != animDesired)
	{
		pev->sequence = animDesired;
		pev->frame = 0;

		ResetSequenceInfo();
	}

}

int CMonster::LookupSequence(const char *label)
{
	// only called from SetAnimation, no need to lock...

	auto iter = m_mapLookupSequenceCache.find(label);
	if(iter == m_mapLookupSequenceCache.end())
	{
		iter = m_mapLookupSequenceCache.emplace(label, CBaseAnimating::LookupSequence(label)).first;
	}
	return iter->second;
}

CBaseEntity *CMonster::CheckTraceHullAttack(float flDist, int iDamage, int iDmgType)
{
	TraceResult tr;

	if (IsPlayer())
		UTIL_MakeVectors(pev->angles);
	else
		UTIL_MakeAimVectors(pev->angles);

	auto p = (CMonsterModStrategy_Default*)m_pMonsterStrategy.get();
	Vector vecStart = p->m_pClass->GetCenterid();
	Vector vecEnd = vecStart + (gpGlobals->v_forward * flDist);

	if (m_iClass == MONSTER_CLASS_ZOMBIEDOG) {
		UTIL_TraceLine(vecStart, vecEnd, dont_ignore_monsters, ENT(pev), &tr);
	} else {
		UTIL_TraceHull(vecStart, vecEnd, dont_ignore_monsters, head_hull, ENT(pev), &tr);
	}

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

bool CMonster::ShouldAttack(CBaseEntity *target) const
{
	if (target->pev->takedamage == DAMAGE_NO)
		return false;

	if (target->IsPlayer() && !m_pMonsterStrategy->IsTeamMate(target))
		return true;

	CZBSBreak *zbs_break = dynamic_cast<CZBSBreak *>(target);
	if (zbs_break)
		return zbs_break->m_flZombiDamageRatio > 0.0f;

	return false;
}

CBasePlayer *CMonster::GetClosestPlayer(bool bVisible) const
{
	if (!m_improv)
		return NULL;

	CBasePlayer *close = NULL;
	float closeRangeSq = 1e8f;

	for (int i = 1; i <= gpGlobals->maxClients; ++i)
	{
		CBasePlayer *player = static_cast<CBasePlayer *>(UTIL_PlayerByIndex(i));

		if (player == NULL)
			continue;

		if (FNullEnt(player->pev))
			continue;

		if (FStrEq(STRING(player->pev->netname), ""))
			continue;

		if (!player->IsAlive())
			continue;

		if (bVisible && !m_improv->IsVisible(m_improv->GetEyes(), true))
			continue;

		if (player->pev->team == pev->team)
			continue;

		float rangeSq = (m_improv->GetCentroid() - player->pev->origin).LengthSquared();

		if (rangeSq < closeRangeSq)
		{
			closeRangeSq = rangeSq;
			close = player;
		}
	}

	return close;
}

void CMonster::SetClass(MonsterClass iClass)
{
	auto p = (CMonsterModStrategy_Default*)m_pMonsterStrategy.get();
	p->SetClass(iClass);
}

void CMonster::SetSkill(std::unique_ptr<IBaseMonsterSkill> ptr)
{
	auto p = (CMonsterModStrategy_Default*)m_pMonsterStrategy.get();
	p->SetSkill(std::move(ptr));
}

void CMonsterModStrategy_Default::OnSpawn()
{
	if (m_pClass)
		m_pClass->OnSpawn();
}

void CMonsterModStrategy_Default::OnThink()
{
	if (m_pSkill) {
		m_pSkill->OnThink();
	}
}

void CMonsterModStrategy_Default::OnTouch(CBaseEntity* other)
{
	if (m_pSkill) {
		m_pSkill->OnTouch(other);
	}
}

void CMonsterModStrategy_Default::OnKilled(entvars_t* pKiller, int iGib)
{
	if (m_pSkill) {
		m_pSkill->OnKilled(pKiller, iGib);
	}
}

void CMonsterModStrategy_Default::DeathSound() const
{
	if (m_pClass) m_pClass->DeathSound();
}

void CMonsterModStrategy_Default::SetClass(MonsterClass iClass)
{
	m_pClass = MonsterClassFactory(iClass);
	m_pClass->OnInit();
}

void CBaseMonsterClass::OnInit()
{
	SET_MODEL(m_pMonster->edict(), "models/player/zombi_origin/zombi_origin.mdl");
	UTIL_SetSize(m_pMonster->pev, VEC_HULL_MIN, VEC_HULL_MAX);
}

const Vector CBaseMonsterClass::GetCenterid() const
{
	return m_pMonster->pev->origin + Vector(0, 0, 36);
}

void CBaseMonsterClass::DeathSound() const
{
	EMIT_SOUND(ENT(m_pMonster->pev), CHAN_VOICE, "zombi/zbs_death_1.wav", VOL_NORM, ATTN_NORM);
}

void CBaseMonsterSkill::OnThink()
{
	if (m_iSkillStatus == SKILL_STATUS_USING && gpGlobals->time > m_flTimeSkillEnd)
	{
		OnSkillEnd();
	}

	if (m_iSkillStatus == SKILL_STATUS_FREEZING && gpGlobals->time > m_flTimeSkillNext)
	{
		m_iSkillStatus = SKILL_STATUS_READY;
		OnSkillReady();
	}
}

void CMonsterClass_Normal::OnInit()
{
	SET_MODEL(m_pMonster->edict(), "models/player/zombi_origin/zombi_origin.mdl");
	UTIL_SetSize(m_pMonster->pev, VEC_HULL_MIN, VEC_HULL_MAX);
}

void CMonsterSkill_Dog::OnActivate(CBaseEntity* target)
{
	if (!(m_pMonster->pev->flags & FL_ONGROUND)) return;
	if (RANDOM_FLOAT(0.0f, 1.0f) > 0.3f) return;
	if (!target) return;
	if (!target->IsAlive()) return;
	if (!target->IsPlayer()) return;

	Vector vecDelta = target->pev->origin - m_pMonster->pev->origin;
	if (vecDelta.IsLengthGreaterThan(250.0f))
		return;

	const duration_t flSkillTime = 1.5s;
	m_pMonster->pev->sequence = 8;
	m_pMonster->pev->frame = 0;
	m_pMonster->ResetSequenceInfo();

	m_pMonster->m_flStop = gpGlobals->time + flSkillTime;

	m_iSkillStatus = SKILL_STATUS_USING;
	m_flTimeSkillEnd = m_pMonster->m_flStop;

	Vector angles = UTIL_VecToAngles(vecDelta);
	m_pMonster->pev->angles.y = angles.y;
	UTIL_MakeAimVectors(m_pMonster->pev->angles);
	Vector velocity = target->pev->velocity + gpGlobals->v_forward * 450.0f;
	velocity.z = vecDelta.Length2D() + 30.0f;
	m_pMonster->pev->velocity = velocity;

	EMIT_SOUND_DYN(ENT(m_pMonster->pev), CHAN_BODY, "zombi/zombiedog_skill1.wav", 1.0, ATTN_NORM, 0, PITCH_NORM);
}

void CMonsterSkill_Dog::OnSkillEnd()
{
	m_iSkillStatus = SKILL_STATUS_FREEZING;
	m_flTimeSkillNext = gpGlobals->time + 10s;
}

void CMonsterSkill_Dog::OnTouch(CBaseEntity* other)
{
	if (m_iSkillStatus != SKILL_STATUS_USING)
		return;

	if (!m_pMonster->ShouldAttack(other))
		return;

	other->TakeDamage(m_pMonster->pev, m_pMonster->pev, 10.0f, DMG_BULLET);

	m_pMonster->m_flStop = gpGlobals->time + 1s;
	m_pMonster->m_improv->Stop();
	m_iSkillStatus = SKILL_STATUS_FREEZING;
	m_flTimeSkillNext = gpGlobals->time + 10s;

	auto pPlayer = (CBasePlayer*)other;
	pPlayer->pev->velocity = g_vecZero;
	pPlayer->m_flVelocityModifier = 0.1;
	UTIL_ScreenFade(pPlayer, Vector(255, 0, 0), 1s, 1s, 150, 0);
}

void CMonsterClass_Dog::OnInit()
{
	SET_MODEL(m_pMonster->edict(), "models/monster/zombiedog.mdl");
	UTIL_SetSize(m_pMonster->pev, Vector(-10.0, -20.0, 0.0), Vector(10.0, 20.0, 36.0));
	m_pMonster->SetSkill(std::make_unique<CMonsterSkill_Dog>(m_pMonster));
}

void CMonsterClass_Dog::OnSpawn()
{
	m_pMonster->m_flAttackDist = 45.0f;
}

const Vector CMonsterClass_Dog::GetCenterid() const
{
	return m_pMonster->pev->origin + Vector(0, 0, 16);
}

void CMonsterClass_Dog::DeathSound() const
{
	switch (RANDOM_LONG(1, 2))
	{
	case 1: EMIT_SOUND(ENT(m_pMonster->pev), CHAN_VOICE, "zombi/zombiedog_death1.wav", VOL_NORM, ATTN_NORM); break;
	case 2: EMIT_SOUND(ENT(m_pMonster->pev), CHAN_VOICE, "zombi/zombiedog_death2.wav", VOL_NORM, ATTN_NORM); break;
	}
}

void CMonsterClass_Speed::OnInit()
{
	PRECACHE_MODEL("models/player/speed_zombi_host/speed_zombi_host.mdl");
	SET_MODEL(m_pMonster->edict(), "models/player/speed_zombi_host/speed_zombi_host.mdl");
	UTIL_SetSize(m_pMonster->pev, VEC_HULL_MIN, VEC_HULL_MAX);
	//m_pMonster->SetSkill(std::make_unique<CMonsterSkill_Dog>(m_pMonster));
}

void CMonsterClass_Speed::DeathSound() const
{
	EMIT_SOUND(ENT(m_pMonster->pev), CHAN_VOICE, "zombi/zbs_death_female_1.wav", VOL_NORM, ATTN_NORM);
}


void CMonsterSkill_Heavy::OnActivate(CBaseEntity* target)
{
	if (!(m_pMonster->pev->flags & FL_ONGROUND)) return;
	if (RANDOM_FLOAT(0.0f, 1.0f) > 0.3f) return;
	if (!target) return;
	if (!target->IsAlive()) return;
	if (!target->IsPlayer()) return;

	const duration_t flSkillTime = 1.0s;

	m_pMonster->m_flStop = gpGlobals->time + flSkillTime;

	Vector vecSrc = m_pMonster->pev->origin;
	CZombieTrap* pTrap = (CZombieTrap*)CBaseEntity::Create("zombietrap", vecSrc, g_vecZero, m_pMonster->edict());

	m_iSkillStatus = SKILL_STATUS_USING;
	m_flTimeSkillEnd = m_pMonster->m_flStop;
}

void CMonsterSkill_Heavy::OnSkillEnd()
{
	m_iSkillStatus = SKILL_STATUS_FREEZING;
	m_flTimeSkillNext = gpGlobals->time + 10s;
}

duration_t CMonsterSkill_Heavy::GetTrapTime()
{
	return 3.0s;
}

void CMonsterClass_Heavy::OnInit()
{
	PRECACHE_SOUND("zombi/zombi_trapsetup.wav");

	PRECACHE_MODEL("models/player/heavy_zombi_host/heavy_zombi_host.mdl");
	SET_MODEL(m_pMonster->edict(), "models/player/heavy_zombi_host/heavy_zombi_host.mdl");
	UTIL_SetSize(m_pMonster->pev, VEC_HULL_MIN, VEC_HULL_MAX);
	m_pMonster->SetSkill(std::make_unique<CMonsterSkill_Heavy>(m_pMonster));
}

void CMonsterClass_Thrower::OnInit()
{

	PRECACHE_MODEL("models/player/zombi_host/zombi_host.mdl");
	SET_MODEL(m_pMonster->edict(), "models/player/zombi_host/zombi_host.mdl");
	UTIL_SetSize(m_pMonster->pev, VEC_HULL_MIN, VEC_HULL_MAX);
	//m_pMonster->SetSkill(std::make_unique<CMonsterSkill_Dog>(m_pMonster));
}

void CMonsterSkill_Heal::OnActivate(CBaseEntity* target)
{
	if (RANDOM_FLOAT(0.0f, 1.0f) > 0.3f) return;
	if (!target) return;
	if (!target->IsAlive()) return;
	if (!target->IsPlayer()) return;

	const duration_t flSkillTime = 1.0s;

	m_pMonster->m_flStop = gpGlobals->time + flSkillTime;

	m_iSkillStatus = SKILL_STATUS_USING;
	m_flTimeSkillEnd = m_pMonster->m_flStop;

	m_pMonster->pev->health += GetHealBonus();

	MESSAGE_BEGIN(MSG_ALL, gmsgMPToCL, NULL, m_pMonster->pev);
	WRITE_BYTE(4);
	WRITE_SHORT(m_pMonster->entindex());
	WRITE_BYTE(2);
	MESSAGE_END();

	MESSAGE_BEGIN(MSG_ALL, gmsgMPToCL, NULL, m_pMonster->pev);
	WRITE_BYTE(4);
	WRITE_SHORT(m_pMonster->entindex());
	WRITE_BYTE(3);
	MESSAGE_END();

	CBaseEntity* pEntity = NULL;
	while ((pEntity = UTIL_FindEntityByClassname(pEntity, "monster_entity")) != NULL)
	{
		if (!pEntity->IsAlive()) continue;
		if ((pEntity->pev->origin - m_pMonster->pev->origin).IsLengthGreaterThan(250.0f)) continue;
		if (pEntity->pev->health >= pEntity->pev->max_health) continue;

		pEntity->pev->health += GetHealBonus();

		MESSAGE_BEGIN(MSG_ALL, gmsgMPToCL, NULL, pEntity->pev);
		WRITE_BYTE(4);
		WRITE_SHORT(pEntity->entindex());
		WRITE_BYTE(2);
		MESSAGE_END();

		MESSAGE_BEGIN(MSG_ALL, gmsgMPToCL, NULL, pEntity->pev);
		WRITE_BYTE(4);
		WRITE_SHORT(pEntity->entindex());
		WRITE_BYTE(3);
		MESSAGE_END();
	}

	/*CBaseEntity* pEntity = NULL;
	while ((pEntity = UTIL_FindEntityInSphere(pEntity, m_pMonster->pev->origin, 250.0)) != NULL)
	{
		if (pEntity->IsBSPModel())
			continue;

		if (pEntity->IsPlayer())
			continue;

		if (pEntity->IsDormant())
			continue;

		pEntity->pev->health += GetHealBonus();

		MESSAGE_BEGIN(MSG_ALL, gmsgMPToCL, NULL, pEntity->pev);
		WRITE_BYTE(4);
		WRITE_SHORT(pEntity->entindex());
		WRITE_BYTE(2);
		MESSAGE_END();

		MESSAGE_BEGIN(MSG_ALL, gmsgMPToCL, NULL, pEntity->pev);
		WRITE_BYTE(4);
		WRITE_SHORT(pEntity->entindex());
		WRITE_BYTE(3);
		MESSAGE_END();
	}*/
}

void CMonsterSkill_Heal::OnSkillEnd()
{
	m_iSkillStatus = SKILL_STATUS_FREEZING;
	m_flTimeSkillNext = gpGlobals->time + 10s;
}

float CMonsterSkill_Heal::GetHealBonus()
{
	return 100.0;
}

void CMonsterClass_Heal::OnInit()
{
	PRECACHE_MODEL("sprites/zombihealer.spr");
	PRECACHE_MODEL("sprites/zombiheal_head.spr");
	PRECACHE_MODEL("models/player/heal_zombi_host/heal_zombi_host.mdl");
	SET_MODEL(m_pMonster->edict(), "models/player/heal_zombi_host/heal_zombi_host.mdl");
	UTIL_SetSize(m_pMonster->pev, VEC_HULL_MIN, VEC_HULL_MAX);
	m_pMonster->SetSkill(std::make_unique<CMonsterSkill_Heal>(m_pMonster));
}

void CMonsterSkill_Boomer::OnKilled(entvars_t* pKiller, int iGib)
{
	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, m_pMonster->pev->origin);
	WRITE_BYTE(TE_ARROWMODEL);
	WRITE_COORD(m_pMonster->pev->origin.x);
	WRITE_COORD(m_pMonster->pev->origin.y);
	WRITE_COORD(m_pMonster->pev->origin.z);
	WRITE_ANGLE(0);
	WRITE_ANGLE(0.0f + rand() * (1.0f / 32767.0f) * (180.0f - 0.0f));
	WRITE_ANGLE(0);
	WRITE_SHORT(g_sModelIndexBoomerEffect);
	WRITE_BYTE(0);
	WRITE_BYTE(10);
	WRITE_BYTE(1);
	WRITE_BYTE(200);
	WRITE_BYTE(4);
	WRITE_SHORT(-1);
	WRITE_BYTE(17);
	MESSAGE_END();

	float z = (FBitSet(m_pMonster->pev->flags, FL_DUCKING)) ? (m_pMonster->pev->origin.z - 18.0 + 1.0) : (m_pMonster->pev->origin.z - 32.0 + 1.0);

	for (int i = 0; i < 5; i++)
	{
		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_TEMPSPRITE);
		WRITE_COORD(m_pMonster->pev->origin.x - 100 + rand() % (100 - (-100) + 1));
		WRITE_COORD(m_pMonster->pev->origin.y - 100 + rand() % (100 - (-100) + 1));
		WRITE_COORD(z + 60);
		WRITE_SHORT(g_sModelIndexBoomerSprite);
		WRITE_BYTE(15);
		WRITE_BYTE(200);
		WRITE_BYTE(20);
		MESSAGE_END();
	}

	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
	WRITE_BYTE(TE_TEMPSPRITE);
	WRITE_COORD(m_pMonster->pev->origin.x);
	WRITE_COORD(m_pMonster->pev->origin.y);
	WRITE_COORD(m_pMonster->pev->origin.z + 60);
	WRITE_SHORT(g_sModelIndexBoomerEffectEx);
	WRITE_BYTE(20);
	WRITE_BYTE(200);
	WRITE_BYTE(8);
	MESSAGE_END();

	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
	WRITE_BYTE(TE_ROTATEDMODEL);
	WRITE_COORD(m_pMonster->pev->origin.x - 55.0f + rand() * (1.0f / 32767.0f) * (55.0f - (-55.0f)));
	WRITE_COORD(m_pMonster->pev->origin.y - 55.0f + rand() * (1.0f / 32767.0f) * (55.0f - (-55.0f)));
	WRITE_COORD(z);
	WRITE_ANGLE(0.0f + rand() * (1.0f / 32767.0f) * (180.0f - 0.0f));
	WRITE_SHORT(g_sModelIndexPosionEffect);
	WRITE_BYTE(10);
	WRITE_BYTE(1);
	WRITE_BYTE(1);
	WRITE_BYTE(100);
	WRITE_BYTE(kRenderTransAdd);
	WRITE_SHORT(-1);
	MESSAGE_END();
	

	m_pMonster->pev->effects |= EF_NODRAW;

	float flDamage = GetDamage();
	float flRadius = 200.0f;

	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CBasePlayer* pPlayer = (CBasePlayer*)UTIL_PlayerByIndex(i);

		if (!pPlayer)
			continue;

		if (FNullEnt(pPlayer->pev))
			continue;

		if (pPlayer->pev->deadflag != DEAD_NO)
			continue;

		if (200.0f * 0.25f > -(pPlayer->pev->origin.z - m_pMonster->pev->origin.z))
		{
			Vector vecDelta = pPlayer->pev->origin - m_pMonster->pev->origin;

			if (flRadius * flRadius > vecDelta.LengthSquared())
			{
				MESSAGE_BEGIN(MSG_ONE, gmsgShake, NULL, pPlayer->pev);
				WRITE_SHORT(40960);
				WRITE_SHORT(0x2000);
				WRITE_SHORT(25600);
				MESSAGE_END();


				int falloff = flDamage / flRadius;

				float flCurrentDamage = max(0, flDamage - vecDelta.Length() * falloff);

				pPlayer->TakeDamage(m_pMonster->pev, m_pMonster->pev, flCurrentDamage, DMG_EXPLOSION);

				float flDist = vecDelta.NormalizeInPlace();

				pPlayer->pev->velocity += vecDelta * (1.0f - (flDist / 200.0f)) * 20 * 150;
			}
		}
	}
}

float CMonsterSkill_Boomer::GetDamage()
{
	return 20.0f;
}

void CMonsterClass_Boomer::OnInit()
{
	PRECACHE_MODEL("models/player/boomer_zombi_host/boomer_zombi_host.mdl");
	SET_MODEL(m_pMonster->edict(), "models/player/boomer_zombi_host/boomer_zombi_host.mdl");
	UTIL_SetSize(m_pMonster->pev, VEC_HULL_MIN, VEC_HULL_MAX);
	m_pMonster->SetSkill(std::make_unique<CMonsterSkill_Boomer>(m_pMonster));
}

void CMonsterClass_Boomer::DeathSound() const
{
	EMIT_SOUND(ENT(m_pMonster->pev), CHAN_VOICE, "zombi/boomer_death.wav", VOL_NORM, ATTN_NORM);
}

void CMonsterSkill_Resident::OnActivate(CBaseEntity* target)
{
	if (!(m_pMonster->pev->flags & FL_ONGROUND)) return;
	if (RANDOM_FLOAT(0.0f, 1.0f) > 0.3f) return;
	if (!target) return;
	if (!target->IsAlive()) return;
	if (!target->IsPlayer()) return;

	Vector vecDelta = target->pev->origin - m_pMonster->pev->origin;
	if (vecDelta.IsLengthGreaterThan(1500.0f))
		return;

	const duration_t flSkillTime = 1.9s;

	if (m_pMonster->pev->velocity.Length2D() > 135)
	{
		m_pMonster->m_iCustomSequence = m_pMonster->LookupSequence("zbs_skill1_run");
		m_pMonster->m_flCustomSequenceUntilTime = gpGlobals->time + 1.2s;
	}

	else if (m_pMonster->pev->velocity.Length2D() > 0)
	{
		m_pMonster->m_iCustomSequence = m_pMonster->LookupSequence("zbs_skill1_walk");
		m_pMonster->m_flCustomSequenceUntilTime = gpGlobals->time + 2.2s;
	}

	else
	{
		m_pMonster->m_iCustomSequence = m_pMonster->LookupSequence("zbs_skill1_idle");
		m_pMonster->m_flCustomSequenceUntilTime = gpGlobals->time + 1.4s;
	}

	m_pMonster->SetAnimation(MONSTERANIM_IDLE);

	m_pMonster->m_flStop = gpGlobals->time + flSkillTime;

	m_iSkillStatus = SKILL_STATUS_USING;
	m_flTimeSkillEnd = m_pMonster->m_flStop;

	Vector angles = UTIL_VecToAngles(vecDelta);
	m_pMonster->pev->angles.y = angles.y;
	UTIL_MakeAimVectors(m_pMonster->pev->angles);
	TraceResult tr;
	const float flDistance = 4.0f * 39.37f;
	Vector vecStart = m_pMonster->pev->origin;
	Vector vecEnd = vecStart + gpGlobals->v_forward * flDistance;

	for (int i = 5; i > 0; i--)
	{
		UTIL_TraceLine(vecStart, vecEnd, dont_ignore_monsters, m_pMonster->edict(), &tr);

		if (tr.flFraction >= 1.0f)
		{
			UTIL_TraceHull(vecStart, vecEnd, dont_ignore_monsters, human_hull, m_pMonster->edict(), &tr);

			if (tr.flFraction >= 1.0f)
				break;
		}

		if (FNullEnt(tr.pHit))
			break;

		CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);

		if (pEntity && pEntity->IsBSPModel())
			break;

		if (!pEntity->pev->takedamage)
			break;

		//float flFalloff = pow(tr.flFraction, (float)i);

		if (tr.flFraction * flDistance > flDistance)
			i = 0;

		if (pEntity->IsPlayer() && !((CBasePlayer*)pEntity)->m_bIsZombie)
		{
			ClearMultiDamage();
			pEntity->TraceAttack(m_pMonster->pev, GetDamage(), gpGlobals->v_forward, &tr, DMG_BULLET);
			ApplyMultiDamage(m_pMonster->pev, m_pMonster->pev);
		}

		vecStart = tr.vecEndPos + gpGlobals->v_forward * 32;
	}
}

void CMonsterSkill_Resident::OnSkillEnd()
{
	m_iSkillStatus = SKILL_STATUS_FREEZING;
	m_flTimeSkillNext = gpGlobals->time + 10s;
}

float CMonsterSkill_Resident::GetDamage()
{
	return 10.0;
}

void CMonsterClass_Resident::OnInit()
{
	PRECACHE_MODEL("models/player/resident_zombi_host/resident_zombi_host.mdl");
	SET_MODEL(m_pMonster->edict(), "models/player/resident_zombi_host/resident_zombi_host.mdl");
	UTIL_SetSize(m_pMonster->pev, VEC_HULL_MIN, VEC_HULL_MAX);
	m_pMonster->SetSkill(std::make_unique<CMonsterSkill_Resident>(m_pMonster));
}

void CMonsterClass_Resident::DeathSound() const
{
	EMIT_SOUND(ENT(m_pMonster->pev), CHAN_VOICE, "zombi/zbs_death_female_1.wav", VOL_NORM, ATTN_NORM);
}


void CMonsterSkill_Deimos::OnThink()
{
	if (m_flTimeTriggerZombieSkill != invalid_time_point && m_flTimeTriggerZombieSkill < gpGlobals->time)
	{
		FireZombieSkill();
	}
	CBaseMonsterSkill::OnThink();
}

void CMonsterSkill_Deimos::FireZombieSkill()
{
	m_flTimeTriggerZombieSkill = invalid_time_point;

	CDeimosTail* pTail = (CDeimosTail*)CBaseEntity::Create("deimostail", m_pMonster->pev->origin, m_pMonster->pev->angles, m_pMonster->edict());

	if (!pTail)
		return;

	UTIL_MakeAimVectors(m_pMonster->pev->angles);

	pTail->Initialize(1.0s);
	pTail->pev->velocity = gpGlobals->v_forward * 1200;
}

void CMonsterSkill_Deimos::OnActivate(CBaseEntity* target)
{
	if (!(m_pMonster->pev->flags & FL_ONGROUND)) return;
	if (!target) return;
	if (!target->IsAlive()) return;
	if (!target->IsPlayer()) return;

	Vector vecDelta = target->pev->origin - m_pMonster->pev->origin;
	if (vecDelta.IsLengthGreaterThan(1500.0f))
		return;

	const duration_t flSkillTime = 1.9s;

	EMIT_SOUND_DYN(m_pMonster->edict(), CHAN_VOICE, "zombi/deimos_skill_start.wav", 1.0, ATTN_NORM, 0, PITCH_NORM);

	if (m_pMonster->pev->velocity.Length2D() > 135)
	{
		m_pMonster->m_iCustomSequence = m_pMonster->LookupSequence("zbs_skill_run");
		m_pMonster->m_flCustomSequenceUntilTime = gpGlobals->time + 1.9s;
	}
		
	else if (m_pMonster->pev->velocity.Length2D() > 0)
	{
		m_pMonster->m_iCustomSequence = m_pMonster->LookupSequence("zbs_skill_walk");
		m_pMonster->m_flCustomSequenceUntilTime = gpGlobals->time + 3.0s;
	}
		
	else
	{
		m_pMonster->m_iCustomSequence = m_pMonster->LookupSequence("zbs_skill_idle");
		m_pMonster->m_flCustomSequenceUntilTime = gpGlobals->time + 1.5s;
	}
		
	m_pMonster->SetAnimation(MONSTERANIM_IDLE);

	m_pMonster->m_flStop = gpGlobals->time + flSkillTime;

	m_flTimeTriggerZombieSkill = gpGlobals->time + 0.5s;
	m_iSkillStatus = SKILL_STATUS_USING;
	m_flTimeSkillEnd = m_pMonster->m_flStop;
}

void CMonsterSkill_Deimos::OnSkillEnd()
{
	m_iSkillStatus = SKILL_STATUS_FREEZING;
	m_flTimeSkillNext = gpGlobals->time + 10s;
}

void CMonsterClass_Deimos::OnSpawn()
{
	m_pMonster->pev->max_health = 500.0f;
	m_pMonster->m_flAttackDamage = 10.0f;
	m_pMonster->m_flAttackRate = 2.0s;
	m_pMonster->m_flAttackDist = 72.0f;
}

void CMonsterClass_Deimos::OnInit()
{
	PRECACHE_SOUND("zombi/deimos_skill_start.wav");
	PRECACHE_MODEL("models/player/deimos_zombi_host/deimos_zombi_host.mdl");
	SET_MODEL(m_pMonster->edict(), "models/player/deimos_zombi_host/deimos_zombi_host.mdl");
	UTIL_SetSize(m_pMonster->pev, VEC_HULL_MIN, VEC_HULL_MAX);
	m_pMonster->SetSkill(std::make_unique<CMonsterSkill_Deimos>(m_pMonster));
}

void CMonsterClass_Helmet::OnInit()
{
	PRECACHE_MODEL("models/monster/zbs_bosss_jun00.mdl");
	SET_MODEL(m_pMonster->edict(), "models/monster/zbs_bosss_jun00.mdl");
	UTIL_SetSize(m_pMonster->pev, Vector(-32.0, -32.0, -36.0), Vector(32.0, 32.0, 100.0));
	//m_pMonster->SetSkill(std::make_unique<CMonsterSkill_Dog>(m_pMonster));
}

void CMonsterClass_Helmet::OnSpawn()
{
	m_pMonster->pev->max_health = 500.0f;
	m_pMonster->m_flAttackDamage = 10.0f;
	m_pMonster->m_flAttackRate = 2.0s;
	m_pMonster->m_flAttackDist = 72.0f;
}

void CMonsterSkill_Deimos2::OnThink()
{
	if (m_flTimeTriggerZombieSkill != invalid_time_point && m_flTimeTriggerZombieSkill < gpGlobals->time)
	{
		FireZombieSkill();
	}
	CBaseMonsterSkill::OnThink();
}

void CMonsterSkill_Deimos2::FireZombieSkill()
{
	UTIL_MakeAimVectors(m_pMonster->pev->angles);
	Vector velocity = gpGlobals->v_forward * 1000.0f;
	m_pMonster->pev->velocity = velocity;
	m_flTimeTriggerZombieSkill = invalid_time_point;
}

void CMonsterSkill_Deimos2::OnActivate(CBaseEntity* target)
{
	if (!(m_pMonster->pev->flags & FL_ONGROUND)) return;
	if (!target) return;
	if (!target->IsAlive()) return;
	if (!target->IsPlayer()) return;

	Vector vecDelta = target->pev->origin - m_pMonster->pev->origin;
	if (vecDelta.IsLengthGreaterThan(1500.0f))
		return;

	const duration_t flSkillTime = 1.9s;

	m_pMonster->m_iCustomSequence = m_pMonster->LookupSequence("zbs_skill_dash");
	m_pMonster->m_flCustomSequenceUntilTime = gpGlobals->time + 1.9s;

	m_pMonster->SetAnimation(MONSTERANIM_IDLE);

	m_pMonster->m_flStop = gpGlobals->time + flSkillTime;

	m_flTimeTriggerZombieSkill = gpGlobals->time + 0.65s;
	m_iSkillStatus = SKILL_STATUS_USING;
	m_flTimeSkillEnd = m_pMonster->m_flStop;
}

void CMonsterSkill_Deimos2::OnSkillEnd()
{
	m_iSkillStatus = SKILL_STATUS_FREEZING;
	m_flTimeSkillNext = gpGlobals->time + 10s;
}

void CMonsterSkill_Deimos2::OnTouch(CBaseEntity* other)
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

float CMonsterSkill_Deimos2::GetDamage()
{
	return 10.0;
}

void CMonsterClass_Deimos2::OnSpawn()
{
	m_pMonster->pev->max_health = 1000.0f;
	m_pMonster->m_flAttackDamage = 10.0f;
	m_pMonster->m_flAttackRate = 2.0s;
	m_pMonster->m_flAttackDist = 72.0f;
	m_pMonster->pev->maxspeed = 225.0f;
}

void CMonsterClass_Deimos2::OnInit()
{
	PRECACHE_MODEL("models/monster/zbs_bosss_jun01.mdl");
	SET_MODEL(m_pMonster->edict(), "models/monster/zbs_bosss_jun01.mdl");
	UTIL_SetSize(m_pMonster->pev, VEC_HULL_MIN, VEC_HULL_MAX);
	m_pMonster->SetSkill(std::make_unique<CMonsterSkill_Deimos2>(m_pMonster));
}

}
