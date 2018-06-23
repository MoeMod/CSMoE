#ifndef WPN_USP_H
#define WPN_USP_H
#ifdef _WIN32
#pragma once
#endif

//tmp
#define USP_MAX_SPEED		250
#define USP_DAMAGE		34
#define USP_DAMAGE_SIL		30
#define USP_RANGE_MODIFER	0.79
#define USP_RELOAD_TIME		2.7

class CUSP : public CBasePlayerWeapon
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual int GetItemInfo(ItemInfo *p);
	virtual BOOL Deploy();
	virtual float GetMaxSpeed() { return m_fMaxSpeed; }
	virtual int iItemSlot() { return PISTOL_SLOT; }
	virtual void PrimaryAttack();
	virtual void SecondaryAttack();
	virtual void Reload();
	virtual void WeaponIdle();
	virtual BOOL UseDecrement()
	{
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
	virtual BOOL IsPistol() { return TRUE; }

public:
	void USPFire(float flSpread, float flCycleTime, BOOL fUseSemi);
	NOXREF void MakeBeam();
	NOXREF void BeamUpdate();

	int m_iShell;

private:
	unsigned short m_usFireUSP;
};

#endif