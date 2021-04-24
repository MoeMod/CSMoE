#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

//voidpistol
#define VOIDPISTOL_MAX_SPEED	260
#define VOIDPISTOL_DAMAGE		54
#define VOIDPISTOL_RANGE_MODIFER	0.81
#define VOIDPISTOL_RELOAD_TIME	3.36s
#define	VOIDPISTOL_INSPECT_TIME 5.7s
#define	VOIDPISTOL_SCAN_RADIUS 600
#define	VOIDPISTOL_MAX_CLIP	50

class CVoidpistol : public CBasePlayerWeapon
{
	enum voidpistol_e
    {
        VOIDPISTOL_IDLEA,
        VOIDPISTOL_IDLEB,
        VOIDPISTOL_IDLEC,
        VOIDPISTOL_SHOOTA,
        VOIDPISTOL_SHOOTB,
        VOIDPISTOL_SHOOTC,
        VOIDPISTOL_SHOOT_BLACKHOLE_A,
        VOIDPISTOL_SHOOT_BLACKHOLE_B,
        VOIDPISTOL_RELOADA,
        VOIDPISTOL_RELOADB,
        VOIDPISTOL_RELOADC,
        VOIDPISTOL_SCANNING_ON,
        VOIDPISTOL_SCANNING_OFF,
        VOIDPISTOL_CHANGEAC,
        VOIDPISTOL_CHANGEBC,
        VOIDPISTOL_DRAWA,
        VOIDPISTOL_DRAWB,
        VOIDPISTOL_DRAWC
    };
	enum voidpistol_mode
    {
        VOIDPISTOL_MODEA,
        VOIDPISTOL_MODEB,
    };
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override { return m_fMaxSpeed; }
	int iItemSlot() override { return PISTOL_SLOT; }
	void PrimaryAttack() override;
	bool IsTargetAvailable;
	bool HasSecondaryAttack() override { return true; }
	void SecondaryAttack() override;
	void Reload() override;
	void ItemPostFrame() override;
	void WeaponIdle() override;
	BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
	BOOL IsPistol() override { return TRUE; }
	KnockbackData GetKnockBackData() override { return { 350.0f, 250.0f, 350.0f, 100.0f, 0.6f }; }

public:
	void VoidpistolFireA(float flSpread, duration_t flCycleTime, BOOL fUseSemi);
	void VoidpistolFireB(float flSpread, duration_t flCycleTime, BOOL fUseSemi);
	void VoidpistolFireC(void);
	bool IsModeCEnabled(int);
#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_voidpistol" , 200 }; }
#endif
	const char* GetCSModelName() override { return "models/w_voidpistol.mdl"; }
	float GetDamage() const;
	int ExtractAmmo(CBasePlayerWeapon* pWeapon) override;
	float BlackholeDamage() const;
	int m_iShell;
	int m_iMode;
	int m_iCharging;
	int m_iDefaultAmmo2;
	int m_iCountPlayer;
	unsigned short m_usFireVoidpistol;

	

};

}
