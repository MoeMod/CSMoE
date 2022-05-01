#pragma once
namespace sv {

class CWebBomb : public CGrenade
{
public:
	void Spawn() override;
	void Precache() override;

	void EXPORT OnThink(void);
	void EXPORT OnTouch(CBaseEntity* pOther);
	void Slowdown();

public:
	int m_iWebTrap;

private:
	time_point_t m_flSlowdownTime;
};
}
