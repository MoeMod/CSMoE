#ifndef WPN_XM1014_H
#define WPN_XM1014_H
#ifdef _WIN32
#pragma once
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

//xm1014
#define XM1014_MAX_SPEED	220
#define XM1014_CONE_VECTOR	Vector(0.0725, 0.0725, 0.0)	// special shotgun spreads
#define	GATLING_DEFAULT_GIVE 45
#define	GATLING_MAX_CLIP 45

class CGatlingex : public CBasePlayerWeapon
{
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override { return XM1014_MAX_SPEED; }
	int iItemSlot() override { return PRIMARY_WEAPON_SLOT; }
	bool HasSecondaryAttack() override { return true; }
	void SecondaryAttack() override;
	void PrimaryAttack() override;
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
	KnockbackData GetKnockBackData() override { return { 750.0f, 550.0f, 650.0f, 500.0f, 0.4f }; }
	const char *GetCSModelName() override { return "models/w_gatlingex.mdl"; }
#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_12gauge" , 65 }; }
#endif

public:
	int m_iShell;
	int m_iDefaultAmmo2;
	int ExtractAmmo(CBasePlayerWeapon* pWeapon) override;
	float FireBallDamage() const;
	int m_iBulletFired;

private:
	unsigned short m_usFireGatlingex;
};

}

#endif