#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

	//mg3
#define MG3_MAX_SPEED			235
#define MG3_RANGE_MODIFER		0.79

	class CM60AMETHYST : public CBasePlayerWeapon
	{

	public:
		void Spawn() override;
		void Precache() override;
		int GetItemInfo(ItemInfo* p) override;
		BOOL Deploy() override;
		float GetMaxSpeed() override { return MG3_MAX_SPEED; }
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
		KnockbackData GetKnockBackData() override { return { 350.0f, 250.0f, 300.0f, 100.0f, 0.6f }; }
		const char* GetCSModelName() override { return "models/w_m60_amethyst.mdl"; }
#ifndef CLIENT_DLL
		WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_amethyst" , 60 }; }
#endif
		float GetArmorRatioModifier() override { return 1.5; }
		WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 6, 3, 140 }; }

	public:
		void M60AMETHYSTFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim);
		void ExpToLineTargets(Vector vecSrc, Vector vecDir);
		float GetDamage() const;
		float GetExpDamage() const;
		
		int m_iShell;
		int iShellOn;
		int m_iRequiredShoots;

	private:
		unsigned short m_usFireM60AMETHYST;
	};

}
