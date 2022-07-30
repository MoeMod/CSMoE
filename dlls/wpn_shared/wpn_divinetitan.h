#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

	class CDivineTitan;
	class CDivineTitanBase : public CBaseAnimating
	{
	public:
		void Spawn() override;
		void Precache() override;
		void ClearEnt(int iClear);
		int GetWeaponsId(void) override { return WEAPON_DIVINETITAN; }

	public:
		static CDivineTitanBase* CreateBeam(int iType, const Vector& vecStart, const Vector& vecOrigin, edict_t* pentOwner);
		static CDivineTitanBase* Create(int iType, const Vector& vecOrigin, const Vector& vecAngles, edict_t* pentOwner, int iTeam, int iData, CDivineTitan* pWeapon);

		void EXPORT OnThink(void);
		void EXPORT OnTouch(CBaseEntity* pOther);

	public:
		void MakeLoopGenerator();
		void CheckDistanceBetweenEnemies();
		void LoopInstantDamage(CBasePlayer* pAttackPlayer);
		float GetDamage_Missile(int IsTouch) const;
		float GetRange_Missile() const;
		float GetDamage_Instance() const;
		float GetRange_Instance() const;

	public:
		CBasePlayer* m_pOwner;
		CDivineTitan* m_pWeapon;

		int m_iType;
		int m_iSaveWpnState;
		Vector m_cStartEntity;
		CBaseEntity* m_pFirstEnemy;
		CBaseEntity* m_iSaveEnemies[3];
		Vector m_vEndPosition;

		time_point_t m_flTimeNextCharge;
		std::vector<EHANDLE> phs9_10_11;
		std::vector<EHANDLE> m_pGroupList;




	};

	class CDivineTitanSummon : public CBaseAnimating
	{
	public:
		void Spawn() override;
		void Precache() override;
		int GetWeaponsId(void) override { return WEAPON_DIVINETITAN; }

	public:
		static CDivineTitanSummon* Create(int iType, bool iIsEffect, int iEffectType, const Vector& vecOrigin, const Vector& vecAngles, edict_t* pentOwner, int iTeam);
		void EXPORT OnThink(void);
		void EXPORT OnTouch(CBaseEntity* pOther);

	public:
		float GetDamage_SummonerAttack(int IsAttraction) const;
		float GetRange_SummonerAttack() const;
	public:

		int m_iType;
		int m_iEffectType;
		int m_iEffect[3];

		//Used For Summoner Effects
		time_point_t m_flNextThinkPoint[2];
		//Used For Damage taking and attraction.
		time_point_t m_flNextDamagePoint[2];
		//Unused
		time_point_t m_flEffRemoveTime;
		Vector m_vSaveOrigin;
	};

	class CDivineTitan : public CBasePlayerWeapon
	{
		// From model: Frames/FPS
#define WEAPON_ANIM_IDLE1_TIME 		/*91.0f/30.0f*/3.03s
#define WEAPON_ANIM_IDLE2_TIME 		/*60.0f/30.0f*/2.00s
#define WEAPON_ANIM_IDLE3_TIME		/*61.0f/30.0f*/2.03s
#define WEAPON_ANIM_DRAW_TIME 		/*31.0f/30.0f*/1.03s
#define WEAPON_ANIM_DRAW3_TIME 		/*12.0f/30.0f*/0.40s

#define WEAPON_ANIM_DEFAULTTOA_TIME 		/*21.0f/30.0f*/0.70s
#define WEAPON_ANIM_DEFAULTTOB_TIME 		/*21.0f/30.0f*/0.70s
#define WEAPON_ANIM_ATODEFAULT_TIME 		/*15.0f/30.0f*/0.50s
#define WEAPON_ANIM_BTODEFAULT_TIME 		/*15.0f/30.0f*/0.50s
#define WEAPON_ANIM_ATOB_TIME 		/*16.0f/30.0f*/0.53s
#define WEAPON_ANIM_BTOA_TIME 		/*16.0f/30.0f*/0.53s

		//Time base（实际时间
#define WEAPON_TIME_DRAW		0.2s
#define WEAPON_TIMEA_ON		0.6s
#define WEAPON_TIMEB_ON		0.6s
#define WEAPON_TIMEC_ON		0.37s

#define WEAPON_TIMEA_OFF		0.5s
#define WEAPON_TIMEB_OFF		0.5s
#define WEAPON_TIMEABSWITCH	0.5s

		//Weapon Animations
		enum
		{
			WEAPON_ANIM_IDLE1,
			WEAPON_ANIM_PULLPIN,
			WEAPON_ANIM_IDLE_PULLPIN,
			WEAPON_ANIM_THROW,
			WEAPON_ANIM_DRAW1,
			WEAPON_ANIM_DEFAULTTOA,
			WEAPON_ANIM_IDLE2,
			WEAPON_ANIM_ATODEFAULT,
			WEAPON_ANIM_DRAW2,
			WEAPON_ANIM_ATOB,
			WEAPON_ANIM_IDLE3,
			WEAPON_ANIM_BTODEFAULT,
			WEAPON_ANIM_DRAW3,
			WEAPON_ANIM_BTOA,
			WEAPON_ANIM_DEFAULTTOB
		};

	public:
		void Spawn() override;
		void Precache() override;
		int GetItemInfo(ItemInfo* p) override;
		int ExtractAmmo(CBasePlayerWeapon* pWeapon) override;
		BOOL CanDeploy() override { return TRUE; };
		BOOL CanDrop() override { return FALSE; }
		BOOL Deploy() override;
		void Holster(int skiplocal) override;
		float GetMaxSpeed() override { return m_fMaxSpeed; }
		int iItemSlot() override { return GRENADE_SLOT; }
		void PrimaryAttack() override;
		void SecondaryAttack() override;
		void ItemPostFrame() override;
		void WeaponIdle() override;
		void UpdateOnRemove(void);
		void RemoveEntity(void);

		BOOL IsSignal();
		
		BOOL UseDecrement() override
		{
#ifdef CLIENT_WEAPONS
			return TRUE;
#else
			return FALSE;
#endif
		}
	public:

#ifndef CLIENT_DLL
		CDivineTitanBase* m_pTitan;
#endif
		int m_iButton;
		int m_iWpnState;
		int m_iFade;
		int m_iSaveSpecialAmmo;

		int m_CrossHairState;
		int m_CrossHairStateStored;
		time_point_t m_flTimeNextMessage;
		time_point_t m_flTimeNextCharge;
		time_point_t m_flPlayerThink;
		time_point_t m_flTimeholster;

	private:
		
		int m_iSaveShoots;
		int m_iAutoAttackStage;
		bool m_bCanThrow;
	};

}
