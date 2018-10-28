#ifndef WPN_SG550_H
#define WPN_SG550_H
#ifdef _WIN32
#pragma once
#endif

class CXM8SharpShooter : public CBasePlayerWeapon
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual int GetItemInfo(ItemInfo *p);
	virtual BOOL Deploy();
	virtual float GetMaxSpeed();
	virtual int iItemSlot() { return PRIMARY_WEAPON_SLOT; }
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
	virtual KnockbackData GetKnockBackData() override { return { 450.0f, 400.0f, 400.0f, 200.0f, 0.5f }; }
	virtual const char *GetCSModelName() override { return "models/w_xm8.mdl"; }

#ifndef CLIENT_DLL
	virtual void ItemPostFrame() override;
	virtual void AttachToPlayer(CBasePlayer *pPlayer) override;
#endif

public:
	void XM8SharpShooterFire(float flSpread, float flCycleTime, BOOL fUseAutoAim);
	float GetDamage();

public:
	int m_iShell;

private:
	unsigned short m_usFireSG550;
};

#endif