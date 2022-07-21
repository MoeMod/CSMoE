#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

//kronos12
#define WPNSTATE_KRONOS12_SCAN_ACTIVATE	1
#define WPNSTATE_KRONOS12_ZOOM_ACTIVATE	2
#define KRONOS12_CHARGE_AMMO	3

class CKronos12 : public CBasePlayerWeapon
{
	enum kronos12_e
    {
        ANIM_IDLE,
		ANIM_IDLE2,
		ANIM_IDLE3,
		ANIM_SHOOT1,
		ANIM_SHOOT2,
		ANIM_SHOOT3,
		ANIM_RELOAD,
		ANIM_RELOAD2,
		ANIM_DRAW,
		ANIM_DRAW2,
		ANIM_SCAN_ACTIVATE,
		ANIM_ZOOM,
		ANIM_SCAN_DEACTIVATE,
    };
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override { return 240; }
	int iItemSlot() override { return PRIMARY_WEAPON_SLOT; }
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	void ItemPostFrame() override;
	bool HasSecondaryAttack() override { return true; }
	void Reload() override;
	void WeaponIdle() override;
	void Holster(int skiplocal) override;
	const char* GetCSModelName() override { return "models/w_kronos12.mdl"; }
	BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
	void PrimaryAttack_FindTargets();
	bool PrimaryAttack_CheckTargetAvailable(CBaseEntity* a2, Vector vecAngleDirection);
	void PrimaryAttack_InstantDamage();
	float GetDamage();
	float GetDamage_PrimaryAttack_Instant();
	KnockbackData GetKnockBackData() override { return { 1000.0f, 450.0f, 650.0f, 600.0f, 0.3f }; }

#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_buckshotk" , 160 }; }
#endif
	float GetArmorRatioModifier() override { return 1.3; }
	WeaponCrosshairData GetCrosshairData() override { return { 0, 8, 6, 0 }; }

public:
#ifndef CLIENT_DLL
	std::vector<EHANDLE> m_pEnemylist;
#endif
	time_point_t m_flNextThink;
	int m_iShell;

private:
	unsigned short m_usFireKronos12;
};

}
