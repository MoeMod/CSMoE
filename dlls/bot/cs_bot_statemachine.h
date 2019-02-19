#ifndef CS_BOT_STATEMACHINE_H
#define CS_BOT_STATEMACHINE_H
#ifdef _WIN32
#pragma once
#endif

class CCSBot;

class BotState
{
public:
	virtual void OnEnter(CCSBot *me) {}
	virtual void OnUpdate(CCSBot *me) {}
	virtual void OnExit(CCSBot *me) {}
	virtual const char *GetName() const = 0;
};

class IdleState : public BotState
{
public:
	virtual void OnEnter(CCSBot *me);
	virtual void OnUpdate(CCSBot *me);
	virtual const char *GetName() const { return "Idle"; }
};

class HuntState : public BotState
{
public:
	virtual void OnEnter(CCSBot *me);
	virtual void OnUpdate(CCSBot *me);
	virtual void OnExit(CCSBot *me);
	virtual const char *GetName() const { return "Hunt"; }

	void ClearHuntArea() { m_huntArea = NULL; }
private:
	CNavArea *m_huntArea;
};

class AttackState : public BotState
{
public:
	virtual void OnEnter(CCSBot *me);
	virtual void OnUpdate(CCSBot *me);
	virtual void OnExit(CCSBot *me);
	virtual const char *GetName() const { return "Attack"; }

	void SetCrouchAndHold(bool crouch) { m_crouchAndHold = crouch; }
	void StopAttacking(CCSBot *me);

protected:
	enum DodgeStateType
	{
		STEADY_ON,
		SLIDE_LEFT,
		SLIDE_RIGHT,
		JUMP,
		NUM_ATTACK_STATES
	} m_dodgeState;

	float m_nextDodgeStateTimestamp;
	CountdownTimer m_repathTimer;
	float m_scopeTimestamp;
	bool m_haveSeenEnemy;
	bool m_isEnemyHidden;
	float m_reacquireTimestamp;
	float m_shieldToggleTimestamp;
	bool m_shieldForceOpen;
	float m_pinnedDownTimestamp;
	bool m_crouchAndHold;
	bool m_didAmbushCheck;
	bool m_dodge;
	bool m_firstDodge;
	bool m_isCoward;
	CountdownTimer m_retreatTimer;
};

class InvestigateNoiseState : public BotState
{
public:
	virtual void OnEnter(CCSBot *me);
	virtual void OnUpdate(CCSBot *me);
	virtual void OnExit(CCSBot *me);
	virtual const char *GetName() const { return "InvestigateNoise"; }

private:
	void AttendCurrentNoise(CCSBot *me);
	Vector m_checkNoisePosition;
};

class BuyState : public BotState
{
public:
	virtual void OnEnter(CCSBot *me);
	virtual void OnUpdate(CCSBot *me);
	virtual void OnExit(CCSBot *me);
	virtual const char *GetName() const { return "Buy"; }

private:
	bool m_isInitialDelay;
	int m_prefRetries;
	int m_prefIndex;
	int m_retries;
	bool m_doneBuying;
	bool m_buyDefuseKit;
	bool m_buyGrenade;
	bool m_buyShield;
	bool m_buyPistol;
};

class MoveToState : public BotState
{
public:
	virtual void OnEnter(CCSBot *me);
	virtual void OnUpdate(CCSBot *me);
	virtual void OnExit(CCSBot *me);
	virtual const char *GetName() const { return "MoveTo"; }

	void SetGoalPosition(const Vector &pos) { m_goalPosition = pos; }
	void SetRouteType(RouteType route) { m_routeType = route; }

private:
	Vector m_goalPosition;
	RouteType m_routeType;
	bool m_radioedPlan;
	bool m_askedForCover;
};

class FetchBombState : public BotState
{
public:
	virtual void OnEnter(CCSBot *me);
	virtual void OnUpdate(CCSBot *me);
	virtual const char *GetName() const { return "FetchBomb"; }
};

class PlantBombState : public BotState
{
public:
	virtual void OnEnter(CCSBot *me);
	virtual void OnUpdate(CCSBot *me);
	virtual void OnExit(CCSBot *me);
	virtual const char *GetName() const { return "PlantBomb"; }
};

class DefuseBombState : public BotState
{
public:
	virtual void OnEnter(CCSBot *me);
	virtual void OnUpdate(CCSBot *me);
	virtual void OnExit(CCSBot *me);
	virtual const char *GetName() const { return "DefuseBomb"; }
};

class HideState : public BotState
{
public:
	virtual void OnEnter(CCSBot *me);
	virtual void OnUpdate(CCSBot *me);
	virtual void OnExit(CCSBot *me);
	virtual const char *GetName() const { return "Hide"; }

public:
	void SetHidingSpot(const Vector &pos) { m_hidingSpot = pos; }
	const Vector &GetHidingSpot() const { return m_hidingSpot; }

	void SetSearchArea(CNavArea *area) { m_searchFromArea = area; }
	void SetSearchRange(float range) { m_range = range; }

	void SetDuration(float time) { m_duration = time; }
	void SetHoldPosition(bool hold) { m_isHoldingPosition = hold; }

	bool IsAtSpot() const { return m_isAtSpot; }

private:
	CNavArea *m_searchFromArea;
	float m_range;

	Vector m_hidingSpot;
	bool m_isAtSpot;
	float m_duration;
	bool m_isHoldingPosition;
	float m_holdPositionTime;
	bool m_heardEnemy;

	float m_firstHeardEnemyTime;
	int m_retry;
	Vector m_leaderAnchorPos;
};

class EscapeFromBombState : public BotState
{
public:
	virtual void OnEnter(CCSBot *me);
	virtual void OnUpdate(CCSBot *me);
	virtual void OnExit(CCSBot *me);
	virtual const char *GetName() const { return "EscapeFromBomb"; }
};

class FollowState : public BotState
{
public:
	virtual void OnEnter(CCSBot *me);
	virtual void OnUpdate(CCSBot *me);
	virtual void OnExit(CCSBot *me);
	virtual const char *GetName() const { return "Follow"; }

	void SetLeader(CBasePlayer *leader) { m_leader = leader; }

private:
	void ComputeLeaderMotionState(float leaderSpeed);

	EntityHandle<CBasePlayer> m_leader;
	Vector m_lastLeaderPos;
	bool m_isStopped;
	float m_stoppedTimestamp;

	enum LeaderMotionStateType
	{
		INVALID,
		STOPPED,
		WALKING,
		RUNNING

	} m_leaderMotionState;

	IntervalTimer m_leaderMotionStateTime;

	bool m_isSneaking;
	float m_lastSawLeaderTime;
	CountdownTimer m_repathInterval;

	IntervalTimer m_walkTime;
	bool m_isAtWalkSpeed;

	float m_waitTime;
	CountdownTimer m_idleTimer;
};

class UseEntityState : public BotState
{
public:
	virtual void OnEnter(CCSBot *me);
	virtual void OnUpdate(CCSBot *me);
	virtual void OnExit(CCSBot *me);
	virtual const char *GetName() const { return "UseEntity"; }

	void SetEntity(CBaseEntity *entity) { m_entity = entity; }

private:
	EHANDLE m_entity;
};

#endif