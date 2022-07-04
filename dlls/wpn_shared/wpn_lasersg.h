#pragma once



#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

#define LaserSgPrimaryAttack 0
#define LaserSgSecondaryAttack 1
	class CLaserSg : public CBasePlayerWeapon
	{
		enum m3_e
		{
			LASERSG_IDLE = 0,
			LASERSG_RELOAD,
			LASERSG_DRAW,
			LASERSG_SHOOT2_START,
			LASERSG_SHOOT2,
			LASERSG_SHOOT2_END
		};

		enum lasersg_bodygroup_e
		{
			GROUP_HANDS,
			GROUP_WEAPON,
			GROUP_SKINGROUP,
			GROUP_SKINGROUP2,
			GROUP_LED,
			GROUP_FX1,
			GROUP_FX2,
			GROUP_FX3,
			GROUP_FX4,
			GROUP_FX5,
			GROUP_FX6
		};

	public:
#ifndef CLIENT_DLL
		void ItemPostFrame(void);
#endif

		void Spawn() override;
		void Precache() override;
		int GetItemInfo(ItemInfo* p) override;
		BOOL Deploy() override;
		float GetMaxSpeed() override { return 240; }
		int iItemSlot() override { return PRIMARY_WEAPON_SLOT; }
		void PrimaryAttack() override;
		void SecondaryAttack() override;
		bool HasSecondaryAttack(void) override { return true; }
		void Reload() override;
		void Holster(int skiplocal) override;
		void WeaponIdle() override;
		
		int ExtractAmmo(CBasePlayerWeapon* pWeapon) override;
#ifndef CLIENT_DLL
		WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_lasersg_buckshot" , 150 }; }
#endif
		float GetArmorRatioModifier() override { return 1.3; }
		WeaponCrosshairData GetCrosshairData() override { return { 0, 9, 4, 0 }; }

		BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
			return TRUE;
#else
			return FALSE;
#endif
		}
		KnockbackData GetKnockBackData() override { return { 750.0f, 500.0f, 650.0f, 500.0f, 0.4f }; }
		const char* GetCSModelName() override { return "models/w_lasersg.mdl"; }

	public:
		float GetDamage() const;

		void InitChargeState(void);
		void ResetChargeTimer(void);
		void SetChargeSkin(void);
		void SendChargeState(void);

		double GetDamage_SecondaryAttack();
		double GetDamage_PrimaryAttack();

		void Attack_FindTargets(bool bFireAttack);
		void Attack_ApplyDamage(bool bFireAttack);
		bool Attack_CheckTargetAvailable(CBaseEntity* a2, Vector vecAngleDirection, bool bFireAttack);


	public:
		
		int m_iShell;
		std::vector<EHANDLE> m_pEnemyList;
		std::vector<EHANDLE> m_pFieldEnemyList;
		time_point_t m_tSecondaryAttackStart;


	private:
		int m_iDefaultAmmo2;
		int m_iModelIndex;
		unsigned short m_usFireLaserSg;
		time_point_t m_flTimeNextCharge;
	};

}
