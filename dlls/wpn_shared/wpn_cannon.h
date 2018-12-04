#pragma once

#include "weapons/WeaponTemplate.hpp"

class CCannon : public LinkWeaponTemplate<CCannon,
	TWeaponIdleDefault,
	TDeployDefault
>
{
public:
	enum cannon_e
	{
		ANIM_IDLE1,
		ANIM_SHOOT,
		ANIM_DRAW,
	};
	static constexpr const char *V_Model = "models/v_cannon.mdl";
	static constexpr const char *P_Model = "models/p_cannon.mdl";
	static constexpr const char *W_Model = "models/w_cannon.mdl";
	static constexpr const char *AnimExtension = "carbine";

public:
	virtual void Spawn();
	virtual void Precache();
	virtual int GetItemInfo(ItemInfo *p);
	virtual float GetMaxSpeed() { return 235; }
	virtual int iItemSlot() { return PRIMARY_WEAPON_SLOT; }
	virtual void PrimaryAttack();
	virtual BOOL UseDecrement()
	{
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}

	virtual KnockbackData GetKnockBackData() override { return { 1100.f, 500.f, 700.f, 400.f, 0.9f }; }
	virtual const char *GetCSModelName() override { return W_Model; }

public:
	void CannonFire(float flSpread, float flCycleTime, BOOL fUseAutoAim);
	float GetDamage();

private:
	unsigned short m_usFire;
};
