#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif


	const duration_t flMineReadyTime = 0.5s;//地雷落地后多久开始监测目标
	const duration_t flMineDetectionRate = 0.1s;//监测频率

	const float flMineDetectionRadius = 39.37 * 3.5;//地雷监测范围
	const duration_t flMineDebuffRate = 0.5s;//爆炸前持续伤害频率
	const duration_t flMineDebuffTime = 3.0s;//爆炸前持续伤害持续时间（自动
	const duration_t flMineDebuffTimeManual = 1.0s;//爆炸前持续伤害持续时间（手动

	const duration_t flMineLightningRate = 0.5s;//爆炸后闪电伤害频率
	const int iMineLightningCount = 3;//爆炸后闪电伤害次数
	const float flMineLightningRadius = 39.37 * 5;//爆炸后闪电伤害范围

	const float flMineExplosionRadius = 39.37 * 5;//爆炸范围
	const int iMinePenetration = 10;

	class CSfTurret;

	class CSfTurretNade : public CGrenade
	{
	public:
		CSfTurretNade()
		{
			m_iState = 0;
		}

		static CSfTurretNade* Create(int iType, const Vector& vecOrigin, const Vector& vecAngles, edict_t* pentOwner);
		void Init(CBasePlayer* pOwner, CSfTurret* pWeapon);
		void SetState(int iState) { m_iState = iState; };

		void Spawn(void);
		void Precache(void);
		void Remove();

		void EXPORT TurretThink();
		void EXPORT TurretTouch(CBaseEntity* pOther);

		void OnExplosion();


		std::vector<CBaseEntity*> RadiusDamage(float flDamage, float flRadius, bool IsDamage, Vector vecOrigin = Vector{ 0,0,0 });

		float GetDamage(int iType, int iStage) const;
		float GetRange(int iType, int iStage) const;

		int GetWeaponsId(void) { return WEAPON_SFTURRET; }
		KnockbackData GetKnockBackData() override { return { 0.0f, 0.0f, 0.0f, 0.0f, -1.0f }; }
	public:

		CBasePlayer* m_pOwner;
		CSfTurret* m_pWeapon;

		std::vector<int> m_vecEffect;

	private:

		int m_iState;
		int m_iAnim;

		time_point_t m_flNextAnim;
		time_point_t m_flDelayExplosion;
		time_point_t m_flReadyExplosion;
		time_point_t m_flSpawnTime;

	};

	class CSfTurret : public CBasePlayerWeapon
	{
		enum
		{
			SFTURRET_IDLE_A,
			SFTURRET_IDLE_B,
			SFTURRET_PULLPIN,
			SFTURRET_IDLE_PULLPIN,
			SFTURRET_THROW,
			SFTURRET_DRAW,
			SFTURRET_MODECHANGE_FIRE,
			SFTURRET_MODECHANGE_LIGHTENING

		};
	public:
		virtual void Spawn();
		virtual void Precache();
		virtual int GetItemInfo(ItemInfo* p);
		virtual BOOL CanDeploy();
		virtual BOOL CanDrop() { return FALSE; }
		virtual BOOL CanHolster();
		virtual BOOL Deploy();
		virtual void Holster(int skiplocal);
		virtual float GetMaxSpeed() { return m_fMaxSpeed; }
		virtual int iItemSlot() { return GRENADE_SLOT; }
		virtual void PrimaryAttack();
		virtual void SecondaryAttack();

		virtual void WeaponIdle();
		virtual BOOL UseDecrement()
		{
#ifdef CLIENT_WEAPONS
			return TRUE;
#else
			return FALSE;
#endif
		}

		void EXPORT AnimThink(void);
		void EXPORT ChangeState(void);

	public:
#ifndef CLIENT_DLL
		EntityHandle <CSfTurretNade> m_pMine;
#endif

	public:
		int m_iState;
	};

}
