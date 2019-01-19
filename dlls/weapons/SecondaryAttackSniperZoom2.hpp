#pragma once

/*
	template<class T>
	concept SecondaryAttackZoom_c requires
	{
		T::ZoomFOV1;
		T::ZoomFOV2;
	};
*/

template<class CFinal, class CBase = CBaseTemplateWeapon>
class TSecondaryAttackSniperZoom2 : public CBase
{
public:

	static constexpr auto Rec_SecondaryAttack_HasZoom = true;
	constexpr int Ref_GetMinZoomFOV() { return static_cast<CFinal &>(*this).ZoomFOV2; }

public:
	void SecondaryAttack(void) override
	{
		CFinal &wpn = static_cast<CFinal &>(*this);
		const int fov1 = wpn.ZoomFOV1;
		const int fov2 = wpn.ZoomFOV2;

		if(CBase::m_pPlayer->m_iFOV == 90)
			CBase::m_pPlayer->m_iFOV = CBase::m_pPlayer->pev->fov = fov1;
		else if(CBase::m_pPlayer->m_iFOV == fov1)
			CBase::m_pPlayer->m_iFOV = CBase::m_pPlayer->pev->fov = fov2;
		else
			CBase::m_pPlayer->m_iFOV = CBase::m_pPlayer->pev->fov = 90;

		CBase::m_pPlayer->ResetMaxSpeed();
		EMIT_SOUND(ENT(CBase::pev), CHAN_ITEM, "weapons/zoom.wav", 0.2, 2.4);
		CBase::m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3;

		return CBase::SecondaryAttack(); // pass over
	}

	bool HasSecondaryAttack() override { return true || CBase::HasSecondaryAttack(); }
};