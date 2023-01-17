#pragma once

namespace sv {

class CGhostHunterSupplyBox : public CBaseEntity
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	int ObjectCaps(void)
	{
		return CBaseEntity::ObjectCaps() | FCAP_IMPULSE_USE;
	}
};


class CMechSupplyBox : public CGhostHunterSupplyBox
{
public:
	virtual void Spawn();
	virtual void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	virtual void Precache();

public:

	int m_iLandingEffectIndex;
};


}
