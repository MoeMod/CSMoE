#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

#define CHAINSR_MAX_SPEED		220
#define CHAINSR_MAX_CLIP		20
#define CHAINSR_MAX_SPEED_ZOOM	140
#define CHAINSR_RANGE_MODIFER	1.0
#define CHAINSR_RELOAD_TIME		2.0s
#define CHAINSR_ARMOR_RATIO_MODIFIER	1.9

	const float ChainSr_ShadowSearchRadius = 39.37 * 20;

	class CChainSr : public CBasePlayerWeapon
	{
		enum chainsr_e
		{
			CHAINSR_IDLE,
			CHAINSR_SHOOT1,
			CHAINSR_SHOOT2,
			CHAINSR_RELOAD,
			CHAINSR_DRAW
		};
	public:
		void Spawn() override;
		void Precache() override;
		int GetItemInfo(ItemInfo* p) override;
		BOOL Deploy() override;
		float GetMaxSpeed() override;
		int iItemSlot() override { return PRIMARY_WEAPON_SLOT; }
		void PrimaryAttack() override;
		void SecondaryAttack() override;
		void Reload() override;
		void WeaponIdle() override;
		BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
			return TRUE;
#else
			return FALSE;
#endif
		}
		KnockbackData GetKnockBackData() override { return { 5000.0f, 500.0f, 1200.0f, 800.0f, 0.3f }; }
		float GetArmorRatioModifier() override { return CHAINSR_ARMOR_RATIO_MODIFIER; }
		const char* GetCSModelName() override { return "models/w_chainsr.mdl"; }
		void ItemPostFrame() override;

#ifndef CLIENT_DLL
		WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_50bmg" , AMMO_50BMG_PRICE }; }
#endif
		WeaponCrosshairData GetCrosshairData() override { return { 0, 4, 3, 120 }; }

	public:
		void ChainSrFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim);

#ifndef CLIENT_DLL
		void FireBullet3CallBack(int iPenetration, Vector vecDir, TraceResult* pTrace) override;
#endif
		void EntityTakeDamageCallBack(int iWpnID, CBaseEntity* pVictim, entvars_t* pevInflictor, CBasePlayer* pAttacker, float flDamage, int bitsDamageType) override;
		void ShadowSearchForEnemy(CBaseEntity* pIgnored, Vector vecOrigin, int iType, float flDamage, int iLastHitGroup);
		void WpnDeathCallBack(CBaseEntity* pVictim,entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) override;
		void ReloadingWaveThink();
		float GetDamage() const;

		int m_iShell;

	private:
		unsigned short m_usFireChainSr;
	};

	class CChainSrShadow : public CBaseEntity
	{
	public:
		static CChainSrShadow* Create(const Vector& vecOrigin, const Vector& vecAngles, edict_t* pentOwner, int iType, int iLastHitGroup, float flDamage);

		void Spawn(void);
		void Precache(void);
		void Init(CBasePlayer* pOwner, CChainSr* pWeapon, CBaseEntity* pEnemy);

		void EXPORT OnThink();
		void StartFadeOut(void);
		void FadeOut(void);
		void FadeIn(void);

		void DoSingleDamage(CBaseEntity* pEnemy);
		KnockbackData GetKnockBackData() override { return { 5000.0f, 500.0f, 1200.0f, 800.0f, 0.3f }; }
		float GetArmorRatioModifier() override { return CHAINSR_ARMOR_RATIO_MODIFIER; }

		int GetWeaponsId(void) { return WEAPON_CHAINSR; }
		float GetDamage(int iType) const;
	private:
		CBasePlayer* m_pOwner;
		CChainSr* m_pWeapon;

		int m_iType;
		int m_iState;

		int m_pLastHitGroup;
		float m_pLastDamage;
		CBaseEntity* m_pEnemy;

		time_point_t m_flAnimTime;

		int m_iSkill[2];
		int m_iShadowMuzzleFlash[2];
	};

}
