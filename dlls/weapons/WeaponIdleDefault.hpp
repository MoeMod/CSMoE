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
	void WeaponIdle(void) override
	{
		CFinal &wpn = static_cast<CFinal &>(*this);
		auto &&data = wpn.WeaponTemplateDataSource();
		wpn.ResetEmptySound();

		CBase::m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

		if (CBase::m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return CBase::WeaponIdle();


		CBase::m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + df::WeaponIdleTime::Get(data);
		wpn.SendWeaponAnim(df::ANIM_IDLE1::Get(data), wpn.UseDecrement() != FALSE);

		return CBase::WeaponIdle();
	}
};