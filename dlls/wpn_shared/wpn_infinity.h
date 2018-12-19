#ifndef WPN_DEAGLED_H
#define WPN_DEAGLED_H
#ifdef _WIN32
#pragma once
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
	virtual void Spawn();
	virtual void Precache();
	virtual int GetItemInfo(ItemInfo *p);
	virtual BOOL Deploy();
	virtual float GetMaxSpeed() { return 250; }
	virtual int iItemSlot() { return PISTOL_SLOT; }
	virtual void PrimaryAttack();
	virtual void SecondaryAttack();
	virtual void Reload();
	virtual void WeaponIdle();
	virtual BOOL UseDecrement()
	{
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
	virtual BOOL IsPistol() { return TRUE; }

	KnockbackData GetKnockBackData() override 
	{ 
		return m_iSpecialAttack ?
			KnockbackData{ 450.0f, 400.0f, 400.0f, 200.0f, 0.5f } : 
			KnockbackData{ 300.0f, 200.0f, 200.0f, 90.0f, 0.8f }; 
	}
	virtual bool HasSecondaryAttack() override { return true; }
	float GetArmorRatioModifier() override { return 1.0; }
	const char *GetCSModelName() override { return "models/w_infinity.mdl"; }
#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() { return ::GetBuyAmmoConfig(WEAPON_USP); }
#endif

public:
	void InfinityFire(float flSpread, float flCycleTime, BOOL fUseSemi);
	void InfinityFire2(float flSpread, float flCycleTime, BOOL fUseSemi);

protected:
	int m_iMaxClip;
	int m_iSpecialAttack;
	virtual float GetDamage() const;

public:
	int m_iShell;
	unsigned short m_usFireELITE_LEFT;
	unsigned short m_usFireELITE_RIGHT;
};

#endif