#pragma once

/*
	template<class T>
	concept RelaodDefault_c requires
	{
		T::MaxClip;
		T::DefaultReloadTime;
		T::ANIM_RELOAD;
		T::DefaultAccuracy;
	};
*/

#include "WeaponTemplate.hpp"

template<class CFinal, class CBase = CBasePlayerWeapon>
class TReloadDefault : public CBase
{
public:
	void Reload(void) override
	{
		if (DefaultReload(CFinal::MaxClip, CFinal::ANIM_RELOAD, CFinal::DefaultReloadTime))
		{
#ifndef CLIENT_DLL
			m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
			m_flAccuracy = CFinal::DefaultAccuracy;
			m_iShotsFired = 0;
			m_bDelayFire = false;

			ReloadCheckZoom();
		}
		return CBase::Reload();
	}
private:

	void ReloadCheckZoom()
	{
		ReloadCheckZoom_impl(1);
	}

	// fucking sfinae
	void ReloadCheckZoom_impl(...) { /* default impl*/}
	template<class Enabled = typename std::enable_if<CFinal::Rec_SecondaryAttack_HasZoom>::type>
	Enabled ReloadCheckZoom_impl(int)
	{
		if (m_pPlayer->pev->fov != 90)
		{
			m_pPlayer->pev->fov = m_pPlayer->m_iFOV = CFinal::Ref_GetMinZoomFOV();
			SecondaryAttack();
		}
	}

	
};