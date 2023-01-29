#pragma once

namespace sv {
#if 0
class CGhostHunterSupplyBox : public CBaseAnimating
{
public:
	virtual void Spawn();
	virtual void Precache();

	void EXPORT SupplyboxTouch(CBaseEntity* pOther);
	void EXPORT SupplyBoxThink();
public:

	bool m_bCanOpen;
	int m_iLandingEffectIndex;
};


class CMechSupplyBox : public CBaseAnimating
{
public:
	virtual void Spawn();
	virtual void Precache();

	int ObjectCaps(void)
	{
		return CBaseEntity::ObjectCaps() | FCAP_IMPULSE_USE;
	}

	virtual void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

	void EXPORT SupplyBoxThink();

public:

	bool m_bCanOpen;
	int m_iLandingEffectIndex;

};
#endif

enum SuperSupplyBoxType
{
	GHOSTHHUNTER_SUPPLYBOX,
	MECH_SUPPLYBOX,
};

class CSuperSupplyBox : public CBaseAnimating
{
public:
	virtual void Spawn();
	virtual void Precache();

	int ObjectCaps(void)
	{
		return CBaseEntity::ObjectCaps() | FCAP_IMPULSE_USE;
	}

	virtual void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

	void EXPORT SupplyboxTouch(CBaseEntity* pOther);
	void EXPORT SupplyBoxThink();
public:

	bool m_bCanOpen;
	int m_iType;
	int m_iLandingEffectIndex;
};


}
