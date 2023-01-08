#pragma once


#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif
	class CClayMore : public CBasePlayerWeapon
	{
		enum claymore_e
		{
			CLAYMORE_IDLE,
			CLAYMORE_SHOOT,
			CLAYMORE_DRAW,
			CLAYMORE_IDLE_TRIGGER_ON,
			CLAYMORE_IDLE_TRIGGER_OFF,
			CLAYMORE_TRIGGER_ON,
			CLAYMORE_TRIGGER_OFF,
			CLAYMORE_SHOOT_ON,
			CLAYMORE_SHOOT_OFF,
			CLAYMORE_TRIGGER_DRAW_ON,
			CLAYMORE_TRIGGER_DRAW_OFF
		};

	public:
		virtual void Spawn();
		virtual void Precache();
		virtual int GetItemInfo(ItemInfo* p);
		virtual BOOL CanDeploy() { return TRUE; };
		virtual BOOL CanDrop() { return FALSE; }
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
#ifndef CLIENT_DLL
		EntityHandle <CClayMoreMine> m_pClayMoreMine;
#endif

	public:
		void OnAttack_Shoot();

	private:

		bool m_bAlreadySetUp;
		bool m_bDeployRemoteControl;
		int m_iTriggerLaserMode;
		int m_iSetUpType;

		time_point_t m_flSetUpTime;
		time_point_t m_flDeployRemoteControl;
		time_point_t m_flSwitchLaserTime;
		time_point_t m_flDelayBoom;

		unsigned short m_usFireClayMore;

	};
}



