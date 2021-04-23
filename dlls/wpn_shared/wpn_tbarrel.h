#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

//xm1014
#define MAX_SPEED	240
#define XM1014_CONE_VECTOR	Vector(0.0725, 0.0725, 0.0)	// special shotgun spreads
#define	DEFAULT_GIVE 3
#define	MAX_CLIP 3

class CTbarrel : public CBasePlayerWeapon
{
	enum tbarrel_e
    {
        ANIM_IDLE,
        ANIM_FIRE1,
        ANIM_FIRE2,
        ANIM_RELOAD,
        ANIM_DRAW
    };
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override { return MAX_SPEED; }
	int iItemSlot() override { return PRIMARY_WEAPON_SLOT; }
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	bool HasSecondaryAttack() override { return true; }
	int GetPrimaryAttackDamage() const;
	void Reload() override;
	void WeaponIdle() override;
	BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
	KnockbackData GetKnockBackData() override { return { 5000.0f, 850.0f, 1450.0f, 850.0f, 0.8f }; }
	const char *GetCSModelName() override { return "models/w_tbarrel.mdl"; }
#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_12gauge" , 65 }; }
#endif

public:
	int m_iShell;

private:
	unsigned short m_usFireTbarrel;
};

}
