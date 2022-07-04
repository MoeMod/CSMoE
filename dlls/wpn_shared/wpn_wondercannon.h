#pragma once

#ifdef CLIENT_DLL
	namespace cl {
#else
	namespace sv {
#endif

#define WONDERCANNON_MAXMINE 3
#define WONDERCANNON_CHECKBOMBTIME	0.35s
		class CWonderCannon;
#ifndef CLIENT_DLL
		class CWonderCannonMine : public CGrenade
		{
		public:
			CWonderCannonMine()
			{
				m_iState = 0;
				m_iSequence = 0;
				m_flNextAnim = invalid_time_point;
			}

			void Spawn(void);
			void Precache(void);
			void EXPORT MineThink();
			void Init(CBasePlayer* pOwner, CWonderCannon* pWeapon);
			void Remove();
			void Explode(bool IsManual);
			void DoBombExp(bool IsManual);
			float GetDamage(bool IsManual) const;
			int GetWeaponsId(void) { return WEAPON_WONDERCANNON; }
		public:

			CBasePlayer* m_pOwner;
			CWonderCannon* m_pWeapon;
			int m_iState;
		private:
			int m_iExp[2];
			int m_iSequence;
			time_point_t m_flNextAnim;
		
		};
#endif
	class CWonderCannon : public CBasePlayerWeapon
	{
		enum wondercannon_e
		{
			WONDERCANNON_IDLE1,
			WONDERCANNON_RELOAD,
			WONDERCANNON_DRAW,
			WONDERCANNON_SHOOT1,
			WONDERCANNON_SHOOT2,
			WONDERCANNON_SHOOT3,
			WONDERCANNON_BMOD_ON,
			WONDERCANNON_CMOD_START,
			WONDERCANNON_CMOD_IDLE,
			WONDERCANNON_CMOD_SHOOT
		};
	public:

		void Spawn() override;
		void Precache() override;
		int GetItemInfo(ItemInfo* p) override;
		BOOL Deploy() override;
		float GetMaxSpeed() override;
		int iItemSlot() override { return PRIMARY_WEAPON_SLOT; }
		bool HasSecondaryAttack() override { return true; }
		void Reload() override;
		void ItemPostFrame() override;
		void WeaponIdle() override;
		void PrimaryAttack() override;
		void SecondaryAttack() override;
		void Holster(int skiplocal) override;
		int ExtractAmmo(CBasePlayerWeapon* pWeapon) override;
		BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
			return TRUE;
#else
			return FALSE;
#endif
		}
		KnockbackData GetKnockBackData() override {	return { 550.0f, 800.0f, 450.0f, 350.0f, 0.4f };}
		float GetArmorRatioModifier() override { return 1.3; }
		const char* GetCSModelName() override { return "models/w_wondercannon.mdl"; }
#ifndef CLIENT_DLL
		WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_wondercannon" , 150 }; }
#endif
		WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 6, 3, 195 }; }
	public:
		void WonderCannonFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim);
		void ShootBomb();
		float GetDamageA() const;
		void DoExpAttachVicStart(CBaseEntity* pVictim);
		int CheckBombActive();
#ifndef CLIENT_DLL
		EntityHandle <CWonderCannonMine> m_pWonderCannonMine[WONDERCANNON_MAXMINE];
#endif
		void ExpCurrentBomb();
	private:

		int m_iDefaultAmmo2;
		unsigned short m_usFireWonderCannon;	//phs[2]
		int m_iBombState;
		int m_iHitEffect;

		time_point_t m_tNextPlayEvent;	//phs[4]
		time_point_t m_flSecondaryAttackTimer;
		time_point_t m_flSecondaryAttackTimer2;
	};

}
