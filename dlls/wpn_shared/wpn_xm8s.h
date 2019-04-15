#ifndef WPN_SG550_H
#define WPN_SG550_H
#ifdef _WIN32
#pragma once
#endif

#include "weapons/WeaponTemplate.hpp"

/*class CXM8SharpShooter : public
		TReloadDefault<CXM8SharpShooter,
		TSecondaryAttackSniperZoom2<CXM8SharpShooter,
		TWeaponIdleDefault<CXM8SharpShooter,
		CBasePlayerWeapon>>>*/
class CXM8SharpShooter : public LinkWeaponTemplate<CXM8SharpShooter,
		TReloadDefault,
		TSecondaryAttackSniperZoom1,
		TWeaponIdleDefault,
		BuildTGetItemInfoFromCSW<WEAPON_SG550>::template type
		>
{
private:
	enum xm8_e
	{
		CARBINE_XM8_IDLE1,
		CARBINE_XM8_RELOAD,
		CARBINE_XM8_DRAW,
		CARBINE_XM8_SHOOT1,
		CARBINE_XM8_SHOOT2,
		CARBINE_XM8_SHOOT3,
		XM8_CHANGE_SHARPSHOOTER,
		SHARPSHOOTER_XM8_IDLE1,
		SHARPSHOOTER_XM8_RELOAD,
		SHARPSHOOTER_XM8_DRAW,
		SHARPSHOOTER_XM8_SHOOT1,
		SHARPSHOOTER_XM8_SHOOT2,
		SHARPSHOOTER_XM8_SHOOT3,
		XM8_CHANGE_CARBINE,
	};


public:
	static constexpr auto ClassName = "weapon_xm8s";
	static constexpr auto MaxClip = 30;
	static constexpr auto DefaultReloadTime = 3.2;
	static constexpr auto DefaultAccuracy = 0.2;
	static constexpr auto ZoomFOV = 25;

	enum
	{
		ANIM_IDLE1 = SHARPSHOOTER_XM8_IDLE1,
		ANIM_RELOAD = SHARPSHOOTER_XM8_RELOAD,
		ANIM_DRAW = SHARPSHOOTER_XM8_DRAW,
	};

public:
	void Spawn() override;
	void Precache() override;
	BOOL Deploy() override;
	float GetMaxSpeed() override;
	int iItemSlot() override { return PRIMARY_WEAPON_SLOT; }
	void PrimaryAttack() override;
	BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
	KnockbackData GetKnockBackData() override { return { 450.0f, 400.0f, 400.0f, 200.0f, 0.5f }; }
	const char *GetCSModelName() override { return "models/w_xm8.mdl"; }

#ifndef CLIENT_DLL
	virtual void ItemPostFrame() override;
	virtual void AttachToPlayer(CBasePlayer *pPlayer) override;
	~CXM8SharpShooter();
#endif

public:
	void XM8SharpShooterFire(float flSpread, float flCycleTime, BOOL fUseAutoAim);
	float GetDamage();

public:
	int m_iShell;

private:
	unsigned short m_usFireSG550;
};

#endif