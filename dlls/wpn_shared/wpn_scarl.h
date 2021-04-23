#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

class CScarLight : public CBasePlayerWeapon
{
	enum scar_e
    {
        SCARL_IDLE1,
        SCARL_RELOAD,
        SCARL_DRAW,
        SCARL_SHOOT1,
        SCARL_SHOOT2,
        SCARL_SHOOT3,
        CHANGE_SCARL,
        SCARH_IDLE1,
        SCARH_RELOAD,
        SCARH_DRAW,
        SCARH_SHOOT1,
        SCARH_SHOOT2,
        SCARH_SHOOT3,
        CHANGE_SCARH,
    };

public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override { return 230; }
	int iItemSlot() override { return PRIMARY_WEAPON_SLOT; }
	void PrimaryAttack() override;
	void SecondaryAttack() override {}
	void Reload() override;
	void WeaponIdle() override;
	BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}

	KnockbackData GetKnockBackData() override { return { 350.0f, 250.0f, 300.0f, 100.0f, 0.6f }; }
	float GetArmorRatioModifier() override { return 1.4; }
	const char *GetCSModelName() override { return "models/w_scar.mdl"; }

#ifndef CLIENT_DLL
	virtual void ItemPostFrame() override;
	~CScarLight();
	virtual void AttachToPlayer(CBasePlayer *pPlayer) override;
#endif

public:
	void GalilFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim);
	float GetDamage();

public:
	int m_iShell;
	int iShellOn;

private:
	unsigned short m_usFireGalil;
};

}
