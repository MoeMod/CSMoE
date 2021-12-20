#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

class CY21s1Jetgunmd : public CBasePlayerWeapon
{
	enum y21s1jetgunmd_e
    {
		ANIM_IDLE,
		ANIM_RELOAD,
		ANIM_DRAW,
		ANIM_SHOOT_START,
		ANIM_SHOOT_LOOP,
		ANIM_SHOOT_END,
		ANIM_DASH_START,
		ANIM_DASH_LOOP,
		ANIM_DASH_END,
    };
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override { return 250; }
	int iItemSlot() override { return PISTOL_SLOT; }
	void PrimaryAttack() override;
	bool HasSecondaryAttack(void) override { return true; }
	void SecondaryAttack() override;
	void Reload() override;
	void WeaponIdle() override;
	void Holster(int skiplocal) override;
	void ItemPostFrame() override;
	BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
	BOOL IsPistol() override { return TRUE; }

	KnockbackData GetKnockBackData() override { return { 450.0f, 350.0f, 300.0f, 250.0f, 0.3f }; }
	int ExtractAmmo(CBasePlayerWeapon* pWeapon) override;
	float GetArmorRatioModifier() override { return 1.35; }
	const char *GetCSModelName() override { return "models/w_y21s1jetgunmd.mdl"; }
#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_energycells" , 100 }; }
#endif
	double GetDamage_PrimaryAttack_Instant();
	double GetDamage_PrimaryAttack_Field();
	void PrimaryAttack_FindTargets();
	bool PrimaryAttack_CheckTargetAvailable(CBaseEntity* a2, Vector vecAngleDirection, bool bFireAttack);
	void PrimaryAttack_InstantDamage();
	void PrimaryAttack_FieldDamage();
	int DashCount();

public:
	std::vector<EHANDLE> m_pEnemyList;
	std::vector<EHANDLE> m_pFieldEnemyList;
	time_point_t m_tPrimaryAttackStart;
	time_point_t m_tDashStart;
	time_point_t m_tDashTime;
	int m_iDefaultAmmo2;
	int m_iChargeShotsFired;
	int m_iShell;
	unsigned short m_usFireY21s1Jetgunmd;
};

}
