#pragma once

#include "weapons/WeaponTemplate.hpp"

class CCannon : public LinkWeaponTemplate<CCannon,
	TGeneralData,
	BuildTGetItemInfoFromCSW<WEAPON_AK47>::template type,
	TGetItemInfoDefault,
	TWeaponIdleDefault,
	TDeployDefault
>
{
public:
	static constexpr const char *ClassName = "weapon_cannon";
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

	struct ItemInfoData_t : Base::ItemInfoData_t
	{
		static constexpr int iFlags = ITEM_FLAG_EXHAUSTIBLE;
		static constexpr const char *szAmmo1 = "CannonAmmo";
		static constexpr int iMaxAmmo1 = 20;
	};

	static constexpr float MaxSpeed = 230;
	static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;

	static constexpr auto &&KnockBack = { 1100.f, 500.f, 700.f, 400.f, 0.9f };
	static constexpr float ArmorRatioModifier = 1.5f;

public:
	virtual void Spawn();
	virtual void Precache();
	virtual void PrimaryAttack();
	virtual int ExtractAmmo(CBasePlayerWeapon *pWeapon) override; // sync Clip -> BpAmmo
#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_cannon" , 200 }; }
#endif

public:
	void CannonFire(float flSpread, float flCycleTime, BOOL fUseAutoAim);
	float GetDamage();

private:
	unsigned short m_usFire;
};
