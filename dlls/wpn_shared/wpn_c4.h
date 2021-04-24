#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

//C4
#define C4_MAX_AMMO		1
#define C4_MAX_SPEED		250.0
#define C4_ARMING_ON_TIME	3.0

class CC4 : public CBasePlayerWeapon
{
	enum c4_e
    {
        C4_IDLE1,
        C4_DRAW,
        C4_DROP,
        C4_ARM
    };
public:
	virtual void Spawn();
	virtual void Precache();
	virtual void KeyValue(KeyValueData *pkvd);
	virtual void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	virtual int GetItemInfo(ItemInfo *p);
	virtual BOOL Deploy();
	virtual void Holster(int skiplocal);
	virtual float GetMaxSpeed() { return C4_MAX_SPEED; }
	virtual int iItemSlot() { return C4_SLOT; }
	virtual void PrimaryAttack();
	virtual void WeaponIdle();
	virtual BOOL UseDecrement()
	{
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}

public:
	bool m_bStartedArming;
	bool m_bBombPlacedAnimation;
	time_point_t m_fArmedTime;

private:
	bool m_bHasShield;
};

}
