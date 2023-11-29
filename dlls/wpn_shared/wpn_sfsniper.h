#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

#define SFSNIPER_MAX_SPEED            190

class CSFSniper : public CBasePlayerWeapon
{
	enum sfsniper_e
    {
        ANIM_IDLE,
        ANIM_SHOOT1,
        ANIM_DRAW,
    };

public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	bool HasSecondaryAttack() override { return true; }
	float GetMaxSpeed() override 	{ return SFSNIPER_MAX_SPEED; }
	int iItemSlot() override { return PRIMARY_WEAPON_SLOT; }
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	void ItemPostFrame() override;
	float GetPrimaryAttackDamage() const;
	void Reload() override;
	void WeaponIdle() override;
	BOOL UseDecrement() override
	{
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
	int m_iMode;
	KnockbackData GetKnockBackData() override 
	{ 
			return { 5000, 900, 1000, 800, 0.16f };
	}
	const char *GetCSModelName() override { return "models/w_sfsniper.mdl"; }
	float GetArmorRatioModifier() override { return 2.0; }
#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_railbeam" , 180 }; }
#endif

	void SFSniperFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim);
	int m_iShell;
	int iShellOn;
	bool m_bSearchingTarget;
	bool m_bIsZoomed;

private:
	unsigned short m_usFireSFSniper;
};

}
