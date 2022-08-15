#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

	//m4a1
#define M4A1_MAX_SPEED		230
#define M4A1_RANGE_MODIFER      0.97
#define M4A1_RANGE_MODIFER_SIL  0.95


	class CACRAMETHYST : public CBasePlayerWeapon
	{
	public:
		void Spawn() override;
		void Precache() override;
		int GetItemInfo(ItemInfo* p) override;
		BOOL Deploy() override;
		float GetMaxSpeed() override { return M4A1_MAX_SPEED; }
		int iItemSlot() override { return PRIMARY_WEAPON_SLOT; }
		void PrimaryAttack() override;
		void SecondaryAttack() override;
		void Reload() override;
		void WeaponIdle() override;
		float GetDamageA();
		float GetDamageB();
		BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
			return TRUE;
#else
			return FALSE;
#endif
		}
		KnockbackData GetKnockBackData() override { return { 350.0f, 250.0f, 300.0f, 100.0f, 0.6f }; }

	public:
		void ACRAMETHYSTFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim);

		int m_iShell;
		time_point_t m_NextInspect;
		int iShellOn;
		int m_iShots;

	private:
		unsigned short m_usFireACRAMETHYST;
	};

}
