#pragma once

namespace sv {

class CZombibomb : public CBaseEntity
{
public:
	virtual void Spawn();
	virtual void Precache();

public:
	void EXPORT OnThink();
	void EXPORT OnTouch(CBaseEntity *pOther);
	void Kill();
};

}
