#pragma once

//AK47
#define AK47_MAX_SPEED			221
#define AK47_RANGE_MODIFER		0.98


#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

	class CAR57AMETHYST : public CBasePlayerWeapon
	{

	public:
		void Spawn() override;
		void Precache() override;
		int GetItemInfo(ItemInfo* p) override;
		BOOL Deploy() override;
		float GetMaxSpeed() override { return AK47_MAX_SPEED; }
		int iItemSlot() override { return PRIMARY_WEAPON_SLOT; }
		void PrimaryAttack() override;
		void SecondaryAttack() override {}
		void Reload() override;
		const char* GetCSModelName() override { return "models/w_ar57_amethyst.mdl"; }
		void WeaponIdle()override;
		BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
			return TRUE;
#else
			return FALSE;
#endif
		}

		KnockbackData GetKnockBackData() override { return { 350.0f, 250.0f, 200.0f, 100.0f, 0.6f }; }
		float GetArmorRatioModifier() override { return 1.55; }
#ifndef CLIENT_DLL
		WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return ::sv::GetBuyAmmoConfig(WEAPON_AK47); }
#endif
		WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 4, 4, 140 }; }

	public:
		//void Killed(entvars_t* pevAttacker, int iGib) override;
		void AR57AMETHYSTFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim);
		float GetDamage() const;
		int m_iShell;
		int iShellOn;

	private:
		unsigned short m_usFireAR57AMETHYST;
	};

}
