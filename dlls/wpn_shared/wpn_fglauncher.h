#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

#ifndef CLIENT_DLL
	class CFGLauncherRocket : public CGrenade
	{
	public:

		CFGLauncherRocket()
		{
			m_flFGLauncherRocketVelBase = 800;
			m_flFGLauncherRocketDamage = 150;
			m_flFGLauncherRocketExplodeRadius = 3;
			m_flFGLauncherRocketExplodeKickRate = 0;
		}

		float GetArmorRatioModifier() { return 1.5; }
		int GetWeaponsId(void) { return WEAPON_FGLAUNCHER; }
		void Spawn(void);
		void Precache(void);
		
		void GetExplodeDamage(float& damage)
		{
			damage *= 0.2;
		}

	public:

		static CFGLauncherRocket* Create(Vector vecSrc, Vector vecAngles, Vector vecVelocity, CBasePlayer* pPlayer, float flDamage);

		void EXPORT FollowThink(void);
		void EXPORT IgniteThink(void);
		void EXPORT RocketTouch(CBaseEntity* pOther);

		void Explode(CBaseEntity* pOther);
	public:
		
		float m_flFGLauncherRocketVelBase;
		float m_flFGLauncherRocketDamage;
		float m_flFGLauncherRocketExplodeRadius;
		float m_flFGLauncherRocketExplodeKickRate;

	private:
		Vector m_vecLastPos;
		int m_iTrail;
		bool m_bBounce;
		int m_iBounce;
	};
#endif

	class CFGLauncher : public CBasePlayerWeapon
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
			return 220;
		}

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

	
#ifndef CLIENT_DLL
		WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_fglauncher" , 300 }; }
#endif
		KnockbackData GetKnockBackData() override { return { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }; }
		const char* GetCSModelName() override { return "models/w_fglauncher.mdl"; }

		WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 4, 3, 170 }; }

	public:
		float m_flFGLauncherRocketVelBase = 800;
		float m_flFGLauncherRocketDamage = 400;
		float m_flFGLauncherRocketExplodeRadius = 4;
		float m_flFGLauncherRocketExplodeKickRate = 0.5;

	private:
		unsigned short m_usFireFGLauncher;
	};

}
