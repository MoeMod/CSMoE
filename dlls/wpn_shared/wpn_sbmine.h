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

	class CSbMine;
	class CSbMineEffect : public CBaseAnimating
	{
	public:
		static CSbMineEffect* Create(const Vector& vecOrigin, const Vector& vecAngles, edict_t* pentOwner, int iType = 0);

		void Spawn(void);
		void Precache(void);
		void Init(CBasePlayer* pOwner, CSbMine* pWeapon);

		void EXPORT EffectThink();
		void StartFadeOut(void);
		void FadeOut(void);

		int GetWeaponsId(void) { return WEAPON_SBMINE; }
	private:
		CBasePlayer* m_pOwner;
		CSbMine* m_pWeapon;

		int m_iType;
		time_point_t m_flAliveTime;
	};
	class CSbMineNade : public CGrenade
	{
	public:
		CSbMineNade()
		{
			m_iState = 0;
		}

		static CSbMineNade* Create(const Vector& vecOrigin, const Vector& vecAngles, edict_t* pentOwner);
		void Init(CBasePlayer* pOwner, CSbMine* pWeapon);

		void Spawn(void);
		void Precache(void);
		void Remove();

		void EXPORT MineThink();
		void EXPORT OnExplosion();
		void EXPORT LightningThink();
		void DetectionThink();
		void DebuffThink();

		void CreateWarningField(CBaseEntity* pEntity, int iType);
		void Explode(bool IsManual);
		void ExplodeEffect(Vector vecOrigin);

		void RadiusDamage(float flDamage, float flRadius, bool IsAttract, bool IsDistanceModified, bool IsSuper, Vector vecOrigin = Vector{ 0,0,0 });

		float GetDamage() const;
		float GetDamageDebuff(int iStage) const;

		int GetWeaponsId(void) { return WEAPON_SBMINE; }
		KnockbackData GetKnockBackData() override { return { 0.0f, 0.0f, 0.0f, 0.0f, -1.0f }; }
	public:
		void SetIsManualExp(bool IsManual) { m_bIsManualExp = IsManual; }

		CBasePlayer* m_pOwner;
		CSbMine* m_pWeapon;
		int m_iState;

	private:
		int m_iDebuffIcon;
		int m_iDebuffField;

		int m_iExp[2];
		int m_iLightning;

		int m_iLightReady;

		bool m_bIsManualExp;

		time_point_t m_flNextAnim;
		time_point_t m_flWarningTime;
		time_point_t m_flDelayExplosion;

	};

	class CSbMine : public CBasePlayerWeapon
	{
		enum
		{
			SBMINE_IDLE,
			SBMINE_SHOOT_READY,
			SBMINE_SHOOT2,
			SBMINE_SHOOT1,
			SBMINE_READY_IDLE,
			SBMINE_DRAW,
			SBMINE_IDLE_TRIGGER,
			SBMINE_TRIGEER_SHOOT,
			SBMINE_TRIGGER_DRAW
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
#ifndef CLIENT_DLL
		virtual void ItemPostFrame();
#endif
		virtual void WeaponIdle();
		void SetFirePost();
		virtual BOOL UseDecrement()
		{
#ifdef CLIENT_WEAPONS
			return TRUE;
#else
			return FALSE;
#endif
		}

		void EXPORT AnimThink(void);
		void EXPORT DelayThink(void);
	public:
#ifndef CLIENT_DLL
		EntityHandle <CSbMineNade> m_pMine;
#endif
		void SetTimeOut(duration_t flTime) { m_flTimeOut = gpGlobals->time + flTime; }
		bool GetTimeOut() { return m_flTimeOut > gpGlobals->time ? true : false; }
	public:
		int m_iModelIndex;
		bool m_pDeployRemoteControl;
		time_point_t m_flTimeOut;
	};

}
