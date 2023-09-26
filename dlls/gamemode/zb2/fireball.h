#pragma once
namespace sv {

class CAkshaFireBall : public CBaseEntity
{
public:
	virtual void Spawn();
	virtual void Precache();

public:
	void EXPORT OnTouch(CBaseEntity* pOther);
	void EXPORT IgniteThink();
	void EXPORT FollowThink();

public:
	void Explosion();
	void Remove();
	void RadiusDamage();

public:
	int m_iSmokeTrail[2];
	int m_iFireBallDestroy[2];
	int m_iFireBallExplosion[2];

private:
	int m_iSkin;
	float m_flMaxFrames;
	CBasePlayer* m_pOwner;
	duration_t GetAliveTime() const { return 4s; }
	float GetRange() const { return 180.0f; }
	float GetDamage() const { return 800.0f; }
	float GetKnockBack() const { return 500.0f; }

	time_point_t m_flRemoveTime;
};
}
