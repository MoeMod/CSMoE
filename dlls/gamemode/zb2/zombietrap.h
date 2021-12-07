#pragma once

namespace sv {

class CZombieTrap : public CBaseEntity
{
public:
	virtual void Spawn();
	virtual void Precache();

public:
	void EXPORT OnThink();
	void EXPORT OnTouch(CBaseEntity *pOther);

private:
	float GetMoveSpeed() const { return 50.0f; }
	duration_t GetAliveTime() const { return 20s; }
	duration_t GetTrappedTime() const { return 10s; }
	CBasePlayer* GetClosestPlayer() const;

	EntityHandle <CBasePlayer> m_hTarget;
	EntityHandle <CBasePlayer> m_hFollowTarget;
	time_point_t m_flRemoveTime;
};

}