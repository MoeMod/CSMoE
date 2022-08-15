#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

	//xm1014
#define XM1014_MAX_SPEED	235
#define XM1014_CONE_VECTOR	Vector(0.0725, 0.0725, 0.0)	// special shotgun spreads
#define	SGDRILLEX_DEFAULT_GIVE 270
#define	SGDRILLEX_MAX_CLIP 50

	class CSgdrillEx : public CBasePlayerWeapon
	{
		enum sgdrillex_e
		{
			SGDRILLEX_IDLE,
			SGDRILLEX_FIRE,
			SGDRILLEX_SLASH,
			SGDRILLEX_RELOAD,
			SGDRILLEX_DRAW,
			SGDRILLEX_CHARGESLASH
		};
	public:

		void Spawn() override;
		void Precache() override;
		int GetItemInfo(ItemInfo* p) override;
		int ExtractAmmo(CBasePlayerWeapon* pWeapon) override;
		BOOL Deploy() override;
		float GetMaxSpeed() override { return XM1014_MAX_SPEED; }
		int iItemSlot() override { return PRIMARY_WEAPON_SLOT; }
		void PrimaryAttack() override;
		void SecondaryAttack() override;
		bool HasSecondaryAttack() override { return true; }
		void DelaySecondaryAttack();
		void DelayChargeAttack();
		
		void Reload() override;
		void WeaponIdle() override;
		void ItemPostFrame() override;
		time_point_t m_flNextResetModel;
		int GetPrimaryAttackDamage() const;
		void AddKickRate(CBasePlayer* pPlayer, Vector vecSrc, float amount, float boost);
#ifndef CLIENT_DLL
		BOOL KnifeAttack(bool IsCharged, Vector vecSrc, Vector vecDir, float flDamage, float flRadius, float flAngleDegrees, int bitsDamageType,
			entvars_t* pevInflictor, entvars_t* pevAttacker);
#endif
		void FindHullIntersection(const Vector& vecSrc, TraceResult& tr, const vec3_t pflMins, const vec3_t pfkMaxs, edict_t* pEntity);
		float GetSecondaryAttackDamage() const;
		float GetChargeAttackDamage() const;
		void ResetSkin();
		void Holster(int skiplocal) override;
		void InitChargeState(void);
		void ResetChargeTimer(void);
#ifndef CLIENT_DLL
		WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_sgdrill_buckshot" , 150 }; }
#endif
		BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
			return TRUE;
#else
			return FALSE;
#endif
		}
		KnockbackData GetKnockBackData() override
		{
			if (m_bSlash)
				return { 600.0f, 500.0f, 650.0f, 500.0f, 0.9f };
			else
				return { 600.0f, 500.0f, 650.0f, 500.0f, 0.35f };
		}

		float GetArmorRatioModifier() override { return 1.25; }
		WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 4, 3, 0 }; }

		const char* GetCSModelName() override { return "models/w_sgdrillex.mdl"; }
	public:
		int m_iShell;
		int m_bSlash;
		int m_iSpecialAmmo;
		bool m_bCanAttack;
		bool m_bShowSmoke;
		time_point_t m_flTimeNextCharge;
	private:
		int m_iDefaultAmmo2;
		unsigned short m_usFireSgdrillEx;
	};

}
