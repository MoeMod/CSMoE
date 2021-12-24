#pragma once

namespace sv {
	//Code From CS:AE
class CZombieHook : public CBaseAnimating
{
public:
	virtual void Spawn();
	virtual void Precache();

public:
	void EXPORT Kill();
	void EXPORT Pulling();
	void EXPORT TailTouch(CBaseEntity* pOther);
	void SetLifespan(duration_t flLifespawn);
	void Init();

private:
	int m_iWitchBatModelIndex;
	int m_iBatEffectModelIndex;
	time_point_t m_fKillTime;
	int m_nPullingPlayerIndex;
	int m_nOwnerIndex;
	float m_flPullingSpeed;
};

}