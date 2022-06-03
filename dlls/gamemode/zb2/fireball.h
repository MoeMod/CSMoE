#pragma once
namespace sv {

class CAkshaFireBall : public CBaseEntity
{
public:
	virtual void Spawn();
	virtual void Precache();

public:
	void EXPORT OnTouch(CBaseEntity* pOther);
	void EXPORT Onthink();

public:
	void Explosion();
	void Remove();
	void RadiusDamage();

public:
	int m_iSmokeTrail;
	int m_iFireBallDestroy;
	int m_iFireBallExplosion;

private:
	duration_t GetAliveTime() const { return 4s; }
	float GetRange() const { return 180.0f; }
	float GetDamage() const { return 800.0f; }
	float GetKnockBack() const { return 500.0f; }

	time_point_t m_flRemoveTime;
};
}
