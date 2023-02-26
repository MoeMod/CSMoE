#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "wpn_claymore.h"

#include "monsters.h"
#include "gamerules.h"

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

	LINK_ENTITY_TO_CLASS(weapon_claymore, CClayMore)

		void CClayMore::Spawn(void)
	{
		Precache();
		m_iId = WEAPON_CLAYMORE;
		SET_MODEL(ENT(pev), "models/w_claymore.mdl");

		pev->classname = MAKE_STRING("weapon_claymore");
		m_iDefaultAmmo = HEGRENADE_DEFAULT_GIVE;
		m_bDelayFire = false;


		m_bAlreadySetUp = false;
		m_bDeployRemoteControl = false;
		m_bIsManual = false;
		m_iTriggerLaserMode = 0;
		m_iSetUpType = 0;

		m_flSetUpTime = invalid_time_point;
		m_flDeployRemoteControl = invalid_time_point;

		m_flDelayBoom = invalid_time_point;


		FallInit();
	}

	void CClayMore::Precache(void)
	{
		PRECACHE_MODEL("models/v_claymore.mdl");
		PRECACHE_MODEL("models/p_claymore.mdl");
		PRECACHE_MODEL("models/w_claymore.mdl");
	}

	int CClayMore::GetItemInfo(ItemInfo* p)
	{
		p->pszName = STRING(pev->classname);
		p->pszAmmo1 = "HEGrenade";
		p->iMaxAmmo1 = MAX_AMMO_HEGRENADE;
		p->pszAmmo2 = NULL;
		p->iMaxAmmo2 = -1;
		p->iMaxClip = WEAPON_NOCLIP;
		p->iSlot = 3;
		p->iPosition = 1;
		p->iId = m_iId = WEAPON_CLAYMORE;
		p->iWeight = HEGRENADE_WEIGHT;
		p->iFlags = 0;


		return 1;
	}
	BOOL CClayMore::CanDeploy(void)
	{
		return m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] != 0;
	}
	BOOL CClayMore::Deploy(void)
	{
		m_fMaxSpeed = 260;
		m_bDelayFire = true;

		if (!m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
		{
			m_pPlayer->RemoveWeapon(m_iId);
			DestroyItem();

			return false;
		}

		if (m_bAlreadySetUp == false)
		{
			m_bDeployRemoteControl = false;
			m_iTriggerLaserMode = 0;
			m_iSetUpType = 0;

		}

		m_bIsManual = false;
		m_flDelayBoom = invalid_time_point;
		m_flDeployRemoteControl = invalid_time_point;
		m_flSetUpTime = invalid_time_point;

		int iAnim = CLAYMORE_DRAW;
		if (m_bAlreadySetUp == true)
		{
			if (m_iTriggerLaserMode == 0)
				iAnim = CLAYMORE_TRIGGER_DRAW_OFF;
			else
				iAnim = CLAYMORE_TRIGGER_DRAW_ON;
		}

		return DefaultDeploy("models/v_claymore.mdl", m_bAlreadySetUp == true ? "models/p_claymore_2.mdl" : "models/p_claymore.mdl", iAnim, "grenade", UseDecrement() != FALSE);
	}

	void CClayMore::Holster(int skiplocal)
	{
		m_fInReload = 0;

		if (!m_bAlreadySetUp)
		{
			m_bDeployRemoteControl = false;
			m_iTriggerLaserMode = 0;
			m_iSetUpType = 0;
		}

		m_bIsManual = false;
		m_flDelayBoom = invalid_time_point;
		m_flDeployRemoteControl = invalid_time_point;
		m_flSetUpTime = invalid_time_point;

		if (!m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
		{
			m_pPlayer->RemoveWeapon(m_iId);
			DestroyItem();
		}

		return CBasePlayerWeapon::Holster(skiplocal);
	}


	void CClayMore::PrimaryAttack(void)
	{
		if (m_bAlreadySetUp == false)
		{
			//Shoot:39.9 / 30.0 = 1.33s

			SendWeaponAnim(CLAYMORE_SHOOT, UseDecrement() != FALSE);

			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.8s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0s;

			m_flSetUpTime = invalid_time_point;
			m_flSetUpTime = gpGlobals->time + 1.36s;

			m_iSetUpType = 1;
			m_bDelayFire = true;
		}
		else
		{
#ifndef CLIENT_DLL
			if (!m_pClayMoreMine)
				return;
#endif
			OnAttack_Shoot();
		}
	}
	void CClayMore::OnAttack_Shoot(void)
	{
		int iAnim = m_iTriggerLaserMode ? CLAYMORE_SHOOT_ON : CLAYMORE_SHOOT_OFF;
		SendWeaponAnim(iAnim, UseDecrement() != FALSE);

		//36 / 30.0 = 1.2
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.2s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;

		m_flDelayBoom = gpGlobals->time + 0.66s;
		m_bIsManual = true;
		m_bDelayFire = true;
	}


	void CClayMore::SecondaryAttack(void)
	{
#ifndef CLIENT_DLL
		if (!m_pClayMoreMine)
			return;
#endif
		if (m_bAlreadySetUp == true && m_bDeployRemoteControl == true)
		{
			int iAnim = CLAYMORE_TRIGGER_ON;
			switch (m_iTriggerLaserMode)
			{
			case 0:
			{
				m_iTriggerLaserMode = 1;
				m_flSwitchLaserTime = gpGlobals->time + 1.0s;

				iAnim = CLAYMORE_TRIGGER_ON;
				break;
			}
			case 1:
			{
				m_iTriggerLaserMode = 0;
				m_flSwitchLaserTime = gpGlobals->time + 1.0s;

				iAnim = CLAYMORE_TRIGGER_OFF;
				break;
			}
			default:break;
			}

			SendWeaponAnim(iAnim, UseDecrement() != FALSE);

			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.03s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.2s;
		}
	}

	void CClayMore::ItemPostFrame()
	{
		if (m_bAlreadySetUp == false)
		{
			if (m_iSetUpType == 1)
			{
				if (m_flSetUpTime != invalid_time_point && m_flSetUpTime < gpGlobals->time)
				{

					m_flSetUpTime = invalid_time_point;
					m_iSetUpType = 2;

					m_bAlreadySetUp = true;

					Vector vecSrc = m_pPlayer->pev->origin;
					Vector vecAngle = m_pPlayer->pev->v_angle;
					vecAngle.x = 0;
					UTIL_MakeVectors(vecAngle);

#ifndef CLIENT_DLL
					m_pClayMoreMine = CClayMoreMine::Create(0, vecSrc, vecAngle, ENT(m_pPlayer->pev));

					if (m_pClayMoreMine)
					{
						m_pClayMoreMine->Init(m_pPlayer, this);
					}
#endif
					if (m_flDeployRemoteControl == invalid_time_point)
						m_flDeployRemoteControl = gpGlobals->time + 0.5s;
				}
			}
		}
		else
		{
			if (m_bDeployRemoteControl && m_flSwitchLaserTime != invalid_time_point && m_flSwitchLaserTime < gpGlobals->time)
			{
				m_flSwitchLaserTime = invalid_time_point;

#ifndef CLIENT_DLL
				if (m_pClayMoreMine)
					m_pClayMoreMine->ChangeMode(m_pClayMoreMine, m_iTriggerLaserMode ? true : false);
#endif
			}

			if (m_iSetUpType == 2 && m_flDeployRemoteControl != invalid_time_point)
			{
				if (m_flDeployRemoteControl < gpGlobals->time)
				{
					if (!m_bDeployRemoteControl)
					{
						SendWeaponAnim(CLAYMORE_TRIGGER_DRAW_OFF, UseDecrement() != FALSE);

						m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.03s;
						m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.2s;

						m_flDeployRemoteControl = invalid_time_point;
						m_bDeployRemoteControl = true;
						m_iSetUpType = 3;
					}
				}
			}

#ifndef CLIENT_DLL
			if (!m_bIsManual || !m_pClayMoreMine)
				return CBasePlayerWeapon::ItemPostFrame();
#endif

			if (m_bDeployRemoteControl && m_flDelayBoom != invalid_time_point && m_flDelayBoom < gpGlobals->time)
			{
				m_flDelayBoom = invalid_time_point;

#ifndef CLIENT_DLL
				if (m_pClayMoreMine)
				{
					m_pClayMoreMine->CheckCanBoom(true, false);
				}
#endif

				if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
					--m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType];

				m_bAlreadySetUp = false;
				Holster(false);
				RetireWeapon();
				m_bIsManual = false;
			}
		}


		return CBasePlayerWeapon::ItemPostFrame();
	}


	void CClayMore::WeaponIdle(void)
	{
		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;

		int iAnim = CLAYMORE_IDLE;
		if (m_bAlreadySetUp)
		{
			switch (m_iTriggerLaserMode)
			{
			case 0:
			{
				iAnim = CLAYMORE_IDLE_TRIGGER_OFF;
				break;
			}
			case 1:
			{
				iAnim = CLAYMORE_IDLE_TRIGGER_ON;
				break;
			}
			default:break;
			}
		}




		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
#ifndef CLIENT_DLL
		SendWeaponAnim(iAnim, UseDecrement() != FALSE);
#endif
	}


}
