#pragma once
namespace sv {

class CTeleportEntity : public CBaseEntity
{
public:
	virtual void Spawn();
	virtual void Precache();

public:
	void EXPORT OnThink();
	void EXPORT OnTouch(CBaseEntity *pOther);

public:
	void Kill();
	void Play_EfTeleport();

private:
	int m_iTeleportZombieOut;
};

}
