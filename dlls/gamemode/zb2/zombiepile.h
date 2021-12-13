#pragma once

namespace sv {

class CZombiePile : public CBaseAnimating
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual int TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType);
	virtual void Killed(entvars_t* pevAttacker, int iGib);

public:
	void EXPORT OnGround();
	void EXPORT OnThink();

public:
	static CZombiePile* Create(CBasePlayer* player);
	void Explosion();
	void Slowdown();

private:
	duration_t GetAliveTime() const { return 10s; }
	duration_t GetSlowdownTime() const { return 5s; }
	float GetRange() const { return 150.0f; }
	float GetDamage() const { return 50.0f; }
	float GetKnockBack() const { return 500.0f; }

	EntityHandle <CBasePlayer> m_hOwner;
	time_point_t m_flRemoveTime;
	int m_idShard;
	unsigned short m_usEvent;
};

}