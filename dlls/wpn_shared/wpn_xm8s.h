#pragma once

#include "weapons/WeaponTemplate.hpp"

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

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
	static constexpr const auto &DefaultReloadTime = 3.2s;
	static constexpr auto AccuracyDefault = 0.2;
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
	constexpr const CXM8SharpShooter &WeaponTemplateDataSource() const { return *this; }

#ifndef CLIENT_DLL
	virtual void ItemPostFrame() override;
	virtual void AttachToPlayer(CBasePlayer *pPlayer) override;
	~CXM8SharpShooter();
#endif

	int GetItemInfo(ItemInfo* p)
	{
		p->pszName = STRING(pev->classname);
		p->pszAmmo1 = "556Nato";
		p->iMaxAmmo1 = MAX_AMMO_556NATO;
		p->pszAmmo2 = NULL;
		p->iMaxAmmo2 = -1;
		p->iMaxClip = SG550_MAX_CLIP;
		p->iSlot = 0;
		p->iPosition = 16;
		p->iId = m_iId = WEAPON_XM8SHARPSHOOTER;
		p->iFlags = 0;
		p->iWeight = SG550_WEIGHT;

		return 1;
	}

	float GetArmorRatioModifier() override { return 1.45; }
#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return ::sv::GetBuyAmmoConfig(WEAPON_SG550); }
#endif

public:
	void XM8SharpShooterFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim);
	float GetDamage();

public:
	int m_iShell;

private:
	unsigned short m_usFireSG550;
};

}
