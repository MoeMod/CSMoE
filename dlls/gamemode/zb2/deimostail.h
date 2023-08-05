#pragma once
namespace sv {

class CDeimosTail : public CBaseEntity
{
public:
	virtual void Spawn();
	virtual void Precache();

public:
	void EXPORT TailTouch(CBaseEntity* pOther);

public:
	void EXPORT Kill();
	void Initialize(duration_t tLifespan);
	void DropGunEX();

public:
	int m_iBossBig00Index;
	int m_bIsZombieSkill;
	int m_iSmokeTrail;
	bool m_bHitHuman;
};
}
