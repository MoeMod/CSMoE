#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif


	class CMagicSg : public CBasePlayerWeapon
	{
		enum xm1014_e
		{
			MAGICSG_IDLE,
			MAGICSG_IDLE_CHARGE,
			MAGICSG_SHOOT1,
			MAGICSG_SHOOT2,
			MAGICSG_RELOAD,
			MAGICSG_DRAW
		};

		enum MAGICSG_COLOR
		{
			MAGICSG_NONE = -1,
			MAGICSG_GREEN,
			MAGICSG_YELLOW,
			MAGICSG_RED,
			MAGICSG_BLUE,
			MAGICSG_PINK,
		};

		enum MAGICSG_BUFFTYPE
		{
			//Nothing happened,maybe it should be Type(MAGICSG_BUFF_HEADSHOT),but it failed.
			MAGICSG_BUFF_NONE = -1,
			MAGICSG_BUFF_KNOCKBACK,
			MAGICSG_BUFF_FORCESTOP,
			MAGICSG_BUFF_BANSKILLS,
			MAGICSG_BUFF_RECOVERY,//10% current health is recovered.
			MAGICSG_BUFF_HEADSHOT,
		};

		enum magicsg_bodygroup_e
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

		void ItemPreFrame() override;

		void Spawn() override;
		void Precache() override;
		int GetItemInfo(ItemInfo* p) override;
		BOOL Deploy() override;
		float GetMaxSpeed() override { return 245; }
		int iItemSlot() override { return PRIMARY_WEAPON_SLOT; }
		void PrimaryAttack() override;
		void SecondaryAttack() override;
		bool HasSecondaryAttack(void) override { return true; }
		void Reload() override;
		void Holster(int skiplocal) override;
		void WeaponIdle() override;
		void DoShoot2(int iCount);
		int ExtractAmmo(CBasePlayerWeapon* pWeapon) override;
#ifndef CLIENT_DLL
		void MagicSgBcsFire(float fYaw, CUtlVector<CBaseEntity*>* pList, CUtlVector<CBaseEntity*>* pGroupList, int iColor, int iBuffType);
		WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_magicsg_buckshot" , 144 }; }
#endif
		float GetArmorRatioModifier() override { return 1.1; }
		WeaponCrosshairData GetCrosshairData() override { return { 0, 9, 4, 0 }; }

		BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
			return TRUE;
#else
			return FALSE;
#endif
		}
		KnockbackData GetKnockBackData() override { return { 850.0f, 450.0f, 600.0f, 450.0f, 0.30f }; }
		const char* GetCSModelName() override { return "models/w_magicsg.mdl"; }

	public:
		float GetDamage() const;
		float GetDamageBcs() const;

		float GetDirectDamageForce(void);
		float GetExplodeDamage(void);
		float GetExplodeRadius(void);

	public:

		bool m_bStartedCharge;
		bool m_bIsCharging;
		int m_iChargeState;
		time_point_t m_fChargeTime;


	private:
		int m_iModelIndex;
		unsigned short m_usFireMagicSg;


	};

}
