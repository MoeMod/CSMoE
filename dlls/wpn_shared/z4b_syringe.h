#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

//syringe
#define syringe_MAX_SPEED		250
#define syringe_MAX_SPEED_SHIELD	180

class CSyringe : public CBasePlayerWeapon
{
	enum syringe_e
    {
        SYRINGE_IDLE,
        SYRINGE_SHOOT,
		SYRINGE_RUN,
        SYRINGE_DRAW,
    };
public:
	virtual void Spawn();
	virtual void Precache();
	virtual int GetItemInfo(ItemInfo *p);
	virtual BOOL CanDeploy();
	virtual BOOL CanDrop() { return FALSE; }
	virtual BOOL Deploy();
	virtual void Holster(int skiplocal);
	virtual float GetMaxSpeed() { return m_fMaxSpeed; }
	virtual int iItemSlot() { return PRIMARY_WEAPON_SLOT; }
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
	int Swing();
	void ResetRunIdle();
	void FindHullIntersection(const Vector& vecSrc, TraceResult& tr, const vec3_t pflMins, const vec3_t pfkMaxs, edict_t* pEntity);
public:
	duration_t m_flNextRunIdle;
	duration_t m_flNextIdle;
public:
	unsigned short m_usCreateSmoke;
};

}
