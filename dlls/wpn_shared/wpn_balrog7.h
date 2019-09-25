#ifndef WPN_BALROG7_H
#define WPN_BALROG7_H
#ifdef _WIN32
#pragma once
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

//Balrog7
#define M249_MAX_SPEED            220
#define M249_DAMAGE            32
#define M249_RANGE_MODIFER        0.97
#define M249_RELOAD_TIME        4.7
#define BALROG7_MAX_CLIP            120
#define	BALROG7_DEFAULT_GIVE		120


class CBalrog7 : public CBasePlayerWeapon
{
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	void SecondaryAttack() override;
	void ItemPostFrame() override;
	BOOL Deploy() override;
	float GetMaxSpeed() override;
	int iItemSlot() override { return PRIMARY_WEAPON_SLOT; }
	void PrimaryAttack() override;
	void Reload() override;
	void WeaponIdle() override;
	bool HasSecondaryAttack() override { return true; }
	
	BOOL UseDecrement() override
	{
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
	KnockbackData GetKnockBackData() override { return {350.0f, 250.0f, 300.0f, 100.0f, 0.6f}; }
	const char *GetCSModelName() override { return "models/w_balrog7.mdl"; }

public:
	void Balrog7Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim);
	void RadiusDamage(Vector vecAiming, float flDamage);
	float GetDamage();
	float BalrogDamage();
	int m_iShell;
	int iShellOn;
	int m_iModelExplo;
	int m_iBalrog7Explo;

private:
	unsigned short m_usFireBalrog7;
};

}

#endif