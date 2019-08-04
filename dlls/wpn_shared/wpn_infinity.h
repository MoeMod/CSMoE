#ifndef WPN_DEAGLED_H
#define WPN_DEAGLED_H
#ifdef _WIN32
#pragma once
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

class CInfinity : public CBasePlayerWeapon
{
protected:
	enum infinity_e
	{
		ANIM_IDLE = 0,
		ANIM_IDLE_LEFTEMPTY,
		ANIM_SHOOT_LEFT,
		ANIM_SHOOT_LEFTLAST,
		ANIM_SHOOT_RIGHT,
		ANIM_SHOOT_RIGHTLAST,
		ANIM_SP_SHOOT_LEFT1,
		ANIM_SP_SHOOT_LEFT2,
		ANIM_SP_SHOOT_RIGHT1,
		ANIM_SP_SHOOT_RIGHT2,
		ANIM_SP_SHOOT_LEFTLAST,
		ANIM_SP_SHOOT_RIGHTLAST,
		ANIM_RELOAD,
		ANIM_DRAW,
	};

public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override { return 250; }
	int iItemSlot() override { return PISTOL_SLOT; }
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	void Reload() override;
	void WeaponIdle() override;
	BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
	BOOL IsPistol() override { return TRUE; }

	KnockbackData GetKnockBackData() override
	{
		return m_iSpecialAttack ?
			KnockbackData{ 450.0f, 400.0f, 400.0f, 200.0f, 0.5f } :
			KnockbackData{ 300.0f, 200.0f, 200.0f, 90.0f, 0.8f };
	}
	bool HasSecondaryAttack() override { return true; }
	float GetArmorRatioModifier() override { return 1.0; }
	const char *GetCSModelName() override { return "models/w_infinity.mdl"; }
#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return ::sv::GetBuyAmmoConfig(WEAPON_USP); }
#endif

public:
	void InfinityFire(float flSpread, duration_t flCycleTime, BOOL fUseSemi);
	void InfinityFire2(float flSpread, duration_t flCycleTime, BOOL fUseSemi);

protected:
	int m_iMaxClip;
	int m_iSpecialAttack;
	virtual float GetDamage() const;

public:
	int m_iShell;
	unsigned short m_usFireELITE_LEFT;
	unsigned short m_usFireELITE_RIGHT;
};

}

#endif