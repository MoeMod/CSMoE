#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

//sfsmg
#define SFSMG_MAX_SPEED			260
#define SFSMG_MAX_CLIP			35
#define SFSMG_RELOAD_TIME		2.95s


class CSFSMG : public CBasePlayerWeapon
{
	enum sfsmg_e
    {
		SFSMG_IDLE1,
		SFSMG_RELOAD,
		SFSMG_DRAW,
		SFSMG_SHOOT1,
		SFSMG_SHOOT2,
		SFSMG_SHOOT3
    };
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override { return SFSMG_MAX_SPEED; }
	int iItemSlot() override { return PRIMARY_WEAPON_SLOT; }
	void PrimaryAttack() override;
	void Reload() override;
	void WeaponIdle() override;
	BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
	KnockbackData GetKnockBackData() override { return { 450.0f, 300.0f, 350.0f, 250.0f, 0.7f }; }
	const char* GetCSModelName() override { return "models/w_sfsmg.mdl"; }
	float GetArmorRatioModifier() override { return 1.55; }
#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return ::sv::GetBuyAmmoConfig(WEAPON_MAC10); }
#endif
	WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 6, 2, 170 }; }
public:
	void SFSMGFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim);

	int m_iShell;
	float GetDamage() const;

private:
	unsigned short m_usFireSFSMG;
};

#ifndef CLIENT_DLL
class CEF_Y23S1SFSMG;
#endif

class CY23S1SFSMG : public CSFSMG
{
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo* p) override;
	BOOL Deploy() override;
	int ExtractAmmo(CBasePlayerWeapon* pWeapon) override;
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	void Reload() override;
	void WeaponIdle() override;
	void Holster(int skiplocal) override;
#ifndef CLIENT_DLL
	void FireBullet3CallBack(int iPenetration, Vector vecDir, TraceResult* pTrace) override;
#endif
	BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}

	const char* GetCSModelName() override { return "models/w_y23s1sfsmg.mdl"; }

public:
	void Y23S1SFSMGFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim);

	float GetDamage() const;
	float GetDamageB() const;
	float GetDamageC() const;

private:
#ifndef CLIENT_DLL
	EntityHandle<CEF_Y23S1SFSMG> m_pEffect;
#endif
	int m_iDefaultAmmo2;
	unsigned short m_usFireY23S1SFSMG;
};

}
