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
public:
	void SFSMGFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim);

	int m_iShell;
	float GetDamage() const;

private:
	unsigned short m_usFireSFSMG;
};

}
