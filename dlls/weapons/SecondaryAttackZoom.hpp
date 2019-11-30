#pragma once

/*
	template<class T>
	concept SecondaryAttackZoom_c requires
	{
		T::ZoomFOV;
	};
*/

template<class CFinal, class CBase = CBaseTemplateWeapon>
class TSecondaryAttackZoom : public CBase
{
public:

	static constexpr auto Rec_SecondaryAttack_HasZoom = true;
	int Ref_GetMinZoomFOV() { return static_cast<CFinal &>(*this).ZoomFOV; }

public:
	void SecondaryAttack(void) override
	{
		CFinal &wpn = static_cast<CFinal &>(*this);
		const int fov1 = df::ZoomFOV::Get(wpn);

		if (CBase::m_pPlayer->m_iFOV != 90)
			CBase::m_pPlayer->pev->fov = CBase::m_pPlayer->m_iFOV = 90;
		else
			CBase::m_pPlayer->pev->fov = CBase::m_pPlayer->m_iFOV = fov1;

		CBase::m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3s;

		return CBase::SecondaryAttack(); // pass over
	}

	bool HasSecondaryAttack() override { return true || CBase::HasSecondaryAttack(); }
};
