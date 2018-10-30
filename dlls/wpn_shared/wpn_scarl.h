#ifndef WPN_GALIL_H
#define WPN_GALIL_H
#ifdef _WIN32
#pragma once
#endif

class CScarLight : public CBasePlayerWeapon
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual int GetItemInfo(ItemInfo *p);
	virtual BOOL Deploy();
	virtual float GetMaxSpeed() { return 230; }
	virtual int iItemSlot() { return PRIMARY_WEAPON_SLOT; }
	virtual void PrimaryAttack();
	virtual void SecondaryAttack() {}
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

	virtual KnockbackData GetKnockBackData() override { return { 350.0f, 250.0f, 300.0f, 100.0f, 0.6f }; }
	float GetArmorRatioModifier() override { return 1.4; }
	virtual const char *GetCSModelName() override { return "models/w_scar.mdl"; }

#ifndef CLIENT_DLL
	virtual void ItemPostFrame() override;
	~CScarLight();
	virtual void AttachToPlayer(CBasePlayer *pPlayer) override;
#endif

public:
	void GalilFire(float flSpread, float flCycleTime, BOOL fUseAutoAim);
	float GetDamage();

public:
	int m_iShell;
	int iShellOn;

private:
	unsigned short m_usFireGalil;
};

#endif