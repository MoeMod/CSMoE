#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

//sfgun
#define SFGUN_MAX_SPEED			221
#define SFGUN_RELOAD_TIME		2.8s
#define SFGUN_MAX_CLIP			45

class CSFGUN : public CBasePlayerWeapon
{
	enum sfgun_e
    {
		SFGUN_IDLE1,
		SFGUN_RELOAD,
		SFGUN_DRAW,
		SFGUN_SHOOT1,
		SFGUN_SHOOT2,
		SFGUN_SHOOT3
    };
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override { return SFGUN_MAX_SPEED; }
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
	const char* GetCSModelName() override { return "models/w_sfgun.mdl"; }
	float GetArmorRatioModifier() override { return 1.55; }
public:
	void SFGUNFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim);
	float GetDamage() const;
public:
	int m_iShell;

private:
	unsigned short m_usFireSFGUN;
};

}
