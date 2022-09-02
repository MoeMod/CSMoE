#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

	//0->Do Pinpoint Bombing.  1->Endless.

#define BunkerBusterPersonalTime 60s
#define BunkerBusterGlobalTime 100s


#define BunkerBusterMode 0
#define BunkerBusterMissileCounts 10
//Just Think Rate ,Real Rate Depends On Distance Calculation.
#define BunkerBuster_BombingInterval 0.2s
#define BunkerBuster_BombingDistance 50.0

#define BunkerBuster_PlaneSpeed 800.0
#define BunkerBuster_PlaneOffSet 512.0
#define BunkerBuster_PlaneRemoveTime 15.0s
#define BunkerBuster_MissileRemoveTime 60.0s
#define BunkerBuster_MissileSpeed 800.0

#define BunkerBuster_FireRate 0.5s
#define BunkerBuster_FireTime 10.0s


//10 Walls Or Solids Will Be Destroyed.
#define BunkerBuster_MissilePenetration 10

	class CBunkerBuster : public CBasePlayerWeapon
	{

	public:
		virtual void Spawn();
		virtual void Precache();
		virtual int GetItemInfo(ItemInfo* p);
		virtual BOOL CanDeploy() { return TRUE; };
		virtual BOOL CanDrop() { return FALSE; }
		//virtual BOOL CanHolster();
		virtual BOOL Deploy();
		virtual void Holster(int skiplocal);
		virtual float GetMaxSpeed() { return m_fMaxSpeed; }
		virtual int iItemSlot() { return GRENADE_SLOT; }
		virtual void PrimaryAttack();
		virtual void SecondaryAttack();
		virtual void ItemPostFrame();
		virtual void WeaponIdle();
		virtual BOOL UseDecrement()
		{
#ifdef CLIENT_WEAPONS
			return TRUE;
#else
			return FALSE;
#endif
		}

	public:
		void GetHitPoint(Vector hOrigin);
		Vector GetAimingOrg();
		Vector Get_ShootPosition(CBaseEntity* pevAttacker, Vector Start);
		float GetDamage();

		void EXPORT OnSoundThink(void);

	public:
		bool m_bSightMode;
		bool m_bStartedAiming;
		int m_iAimedState;

		int m_iSoundCounts;
		time_point_t m_flChangeView;
		time_point_t m_fAimedTime;

		Vector m_vecStart;
		Vector m_vecEnd;
		unsigned short m_usFireBunkerBuster;
	private:
		time_point_t m_flCoolDownTime;

	};


	class CBunkerBusterTarget : public CBaseAnimating
	{
	public:
		void Spawn() override;
		void Precache() override;
		void EXPORT OnThink();

	public:
		static CBunkerBusterTarget* CreateTarget(const Vector& vecOrigin, const Vector& vecAngles, edict_t* pentOwner, CBunkerBuster* pWeapon, bool IsSpecial = false);

	public:
		time_point_t m_flRemoveTime;
		bool m_bFastRemove;

		CBunkerBuster* m_pWeapon;

	private:
		duration_t GetLifeTime() const { return 10.0s; }

	};

}
