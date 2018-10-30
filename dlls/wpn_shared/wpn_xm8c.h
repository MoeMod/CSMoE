
#pragma once

#include "weapons/WeaponTemplate.hpp"

class CXM8Carbine : public 
	TReloadDefault<CXM8Carbine, 
	TSecondaryAttackZoom<CXM8Carbine, 
	CBasePlayerWeapon>>
{

public:
	static constexpr auto MaxClip = 30;
	static constexpr auto DefaultReloadTime = 3.2;
	static constexpr auto DefaultAccuracy = 0.2;
	static constexpr auto ZoomFOV = 55;

	static constexpr const char *V_Model = "models/v_xm8.mdl";
	static constexpr const char *P_Model = "models/p_xm8.mdl";
	static constexpr const char *W_Model = "models/w_xm8.mdl";

	enum
	{
		ANIM_IDLE1,
		ANIM_RELOAD,
		ANIM_DRAW,
		ANIM_SHOOT1,
		ANIM_SHOOT2,
		ANIM_SHOOT3,
	};

public:
	virtual void Spawn();
	virtual void Precache();
	virtual int GetItemInfo(ItemInfo *p);
	virtual BOOL Deploy();
	virtual float GetMaxSpeed();
	virtual int iItemSlot() { return PRIMARY_WEAPON_SLOT; }
	virtual void PrimaryAttack();
	virtual void WeaponIdle();
	virtual BOOL UseDecrement()
	{
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
	virtual KnockbackData GetKnockBackData() override { return { 350.0f, 250.0f, 300.0f, 100.0f, 0.6f }; }
	
	virtual const char *GetCSModelName() override { return "models/w_xm8.mdl"; }

#ifndef CLIENT_DLL
	virtual void ItemPostFrame() override;
	~CXM8Carbine();
	virtual void AttachToPlayer(CBasePlayer *pPlayer) override;
#endif

public:
	void XM8CarbineFire(float flSpread, float flCycleTime, BOOL fUseAutoAim);
	float GetDamage();

	int m_iShell;
	int iShellOn;

private:
	unsigned short m_usFireSG552;
};
