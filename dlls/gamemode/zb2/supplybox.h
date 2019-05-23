#pragma once

namespace sv {

class CSupplyBox : public CBaseEntity
{
public:
	virtual void Spawn();
	virtual void Precache();

public:
	void EXPORT SupplyboxThink();
	void EXPORT SupplyboxTouch(CBaseEntity *pOther);

public:
	void SendPositionMsg();

public:
	float m_flNextRadarTime;
	int m_iSupplyboxIndex;
};

}
