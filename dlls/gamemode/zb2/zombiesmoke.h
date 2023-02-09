#pragma once

namespace sv {
class CZombieSmoke : public CBaseEntity
{
public:
	virtual void Spawn();

public:
	void EXPORT SmokeThink();
	void SetLifespan(duration_t flLifespawn);
	void SetReduceRate(float rate);

private:
	time_point_t m_fKillTime;
	float m_flReduceRate;

};

}