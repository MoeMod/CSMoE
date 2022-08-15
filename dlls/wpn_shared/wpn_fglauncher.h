#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

#ifndef CLIENT_DLL

	class CFgLauncherEffect : public CBaseAnimating
	{
	public:
		static CFgLauncherEffect* Create( const Vector& vecOrigin, const Vector& vecAngles, edict_t* pentOwner, int iTeam);

		void Spawn() override;
		void Precache() override;

		void EXPORT OnThink(void);
	private:
		time_point_t m_flTimeDead;

		int m_iEffect;
		float m_flNumFrames;
		int m_iExpTimes;
	};

	class CFgLauncherRocket : public CGrenade
	{
	public:
		float GetArmorRatioModifier() { return 1.5; }
		int GetWeaponsId(void) { return WEAPON_FGLAUNCHER; }
		void Spawn(void);
		void Precache(void);
		
		void ForceParabolicMotion(Vector vecEnd, float flOffset);

		float GetExplodeDamage() const
		{
			float flDamage = 185;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
			{
				flDamage = 300;
			}
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
			{
				flDamage = 1300;
			}
#endif
			return flDamage;
		}

		float GetRadius() const
		{
			float flRadius = 1.2 * 39.37;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
			{
				flRadius = 4 * 39.37;
			}
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
			{
				flRadius = 4 * 39.37;
			}
#endif
			return flRadius;
		}



	public:
		void EXPORT EffectThink(void);
		void EXPORT FollowThink(void);
		void EXPORT IgniteThink(void);
		void EXPORT RocketTouch(CBaseEntity* pOther);
	public:
		
		int m_iEffect;
		int m_iExpTimes;
		time_point_t m_flNextTime;
		time_point_t m_flNextFadeTime;
		time_point_t m_flSprEffectTime;

	private:
		int m_iTrail;
		time_point_t m_flIgniteTime;
		time_point_t m_flRemoveTime;
	};
#endif

	class CFgLauncher : public CBasePlayerWeapon
	{
		enum fglauncher_e
		{
			FGLAUNCHER_IDLE,
			FGLAUNCHER_SHOOT,
			FGLAUNCHER_RELOAD,
			FGLAUNCHER_DRAW
		};
	public:
		void Spawn() override;
		void Precache() override;
		int GetItemInfo(ItemInfo* p) override;
		BOOL Deploy() override;
		float GetMaxSpeed() override
		{
			if (m_pPlayer->m_iFOV == 90)
				return 230;
			return 160;
		}

		int iItemSlot() override { return PRIMARY_WEAPON_SLOT; }
		void PrimaryAttack() override;
		void SecondaryAttack() override;
		bool HasSecondaryAttack(void) override { return true; }
		void Reload() override;
		void WeaponIdle() override;

		BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
			return TRUE;
#else
			return FALSE;
#endif
		}
#ifndef CLIENT_DLL
		WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_fglaunchergrenade" , 300 }; }
#endif
		KnockbackData GetKnockBackData() override { return { 700.0f, 450.0f, 600.0f, 450.0f, 0.4f }; }
		const char* GetCSModelName() override { return "models/w_fglauncher.mdl"; }

	public:
		duration_t m_flPumpTime;
		float GetDamage() const;

	private:
		unsigned short m_usFireFgLauncher;
	};

}
