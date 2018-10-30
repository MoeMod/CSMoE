#pragma once

/*
	template<class T>
	concept SecondaryAttackZoom_c requires
	{
		T::ZoomFOV;
	};
*/

template<class CFinal, class CBase = CBasePlayerWeapon>
class TSecondaryAttackZoom : public CBase
{
public:
	
	static constexpr auto Rec_SecondaryAttack_HasZoom = true;
	static constexpr int Ref_GetMinZoomFOV() { return CFinal::ZoomFOV; }

public:
	void SecondaryAttack(void) override
	{
		if (m_pPlayer->m_iFOV != 90)
			m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 90;
		else
			m_pPlayer->pev->fov = m_pPlayer->m_iFOV = CFinal::ZoomFOV;

		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3;

		return CBase::SecondaryAttack(); // pass over
	}

	bool HasSecondaryAttack() override { return true || CBase::HasSecondaryAttack(); }
};