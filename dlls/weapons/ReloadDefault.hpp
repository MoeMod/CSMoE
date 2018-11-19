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
		CFinal &wpn = static_cast<CFinal &>(*this);
		if (wpn.DefaultReload(
				wpn.MaxClip,
				wpn.ANIM_RELOAD,
				wpn.DefaultReloadTime
				))
		{
#ifndef CLIENT_DLL
			CBase::m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
			CBase::m_flAccuracy = wpn.DefaultAccuracy;
			CBase::m_iShotsFired = 0;
			CBase::m_bDelayFire = false;

			ReloadCheckZoom();
		}
		return CBase::Reload();
	}
private:
	void ReloadCheckZoom()
	{
		ReloadCheckZoom_impl();
	}

	// fucking sfinae
	void ReloadCheckZoom_impl(...) { /* default impl*/}
	template<class ClassToFind = CFinal>
	void ReloadCheckZoom_impl(bool = ClassToFind::Rec_SecondaryAttack_HasZoom)
	{
		CFinal &wpn = static_cast<CFinal &>(*this);
		if (CBase::m_pPlayer->pev->fov != 90)
		{
			CBase::m_pPlayer->pev->fov = CBase::m_pPlayer->m_iFOV = wpn.Ref_GetMinZoomFOV();
			wpn.SecondaryAttack();
		}
	}
};