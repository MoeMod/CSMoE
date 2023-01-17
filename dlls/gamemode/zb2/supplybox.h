#pragma once

namespace sv {

class CSupplyBox : public CBaseEntity
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	int ObjectCaps(void)
	{
		return CBaseEntity::ObjectCaps() | FCAP_IMPULSE_USE;
	}

public:
	void EXPORT SupplyboxThink();
	void EXPORT SupplyboxTouch(CBaseEntity *pOther);

public:
	void SendPositionMsg();

public:
	time_point_t m_flNextRadarTime;
	int m_iSupplyboxIndex;
};

}
