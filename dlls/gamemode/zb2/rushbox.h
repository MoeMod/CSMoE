#pragma once

namespace sv {
class CZombieRushBox : public CBaseEntity
{
public:
	virtual void Spawn();
	virtual void Precache();

public:
	void DoDamage();
	void EXPORT BoxThink();
	void Kill();
	int GetSkillState();
	time_point_t GetLifespan();
	void SetLifespan(duration_t flLifespawn);
	void Init();


private:

	time_point_t m_fKillDelay;
	time_point_t m_fKillTime;
	std::vector<int> m_nPullingPlayerIndex;
	int m_nOwnerIndex;
	int m_iState;
	int m_iGibsModelIndex;
	int m_iWaveModelIndex;
	float m_flPullingSpeed;
};

}