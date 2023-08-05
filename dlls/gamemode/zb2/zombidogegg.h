#pragma once
namespace sv {

class CZombiDogEgg : public CGrenade
{
public:
	void Spawn() override;
	void Precache() override;

	void EXPORT OnThink(void);
	void EXPORT OnTouch(CBaseEntity* pOther);
	void DropGunEX();

private:
	int m_iExplodeEffect;
	int m_iBossBig01Index;
};
}
