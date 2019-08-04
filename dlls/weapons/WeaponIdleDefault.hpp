#pragma once

/*
	template<class T>
	concept WeaponIdleDefault_c requires
	{
		T::WeaponIdleTime = 20; // optional
		T::ANIM_IDLE1
	};
*/

template<class CFinal, class CBase = CBaseTemplateWeapon>
class TWeaponIdleDefault : public CBase
{
public:
	static constexpr const auto & WeaponIdleTime = 20s;

public:
	void WeaponIdle(void)
	{
		CFinal &wpn = static_cast<CFinal &>(*this);
		wpn.ResetEmptySound();

		CBase::m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

		if (CBase::m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return CBase::WeaponIdle();


		CBase::m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + wpn.WeaponIdleTime;
		wpn.SendWeaponAnim(wpn.ANIM_IDLE1, wpn.UseDecrement() != FALSE);

		return CBase::WeaponIdle();
	}
};