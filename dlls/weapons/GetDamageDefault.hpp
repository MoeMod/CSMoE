//
// Created by 小白白 on 2019-01-19.
//

#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif

template<class CFinal, class CBase = CBaseTemplateWeapon>
class TGetDamageDefault : public CBase
{
public:
	constexpr auto GetDamage(void)
	{
		CFinal &wpn = static_cast<CFinal &>(*this);
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			return GetDamage_ZB(&wpn);
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			return GetDamage_ZBS(&wpn);
		else
#endif
		return wpn.DamageDefault;
	}

private:
	template<class ClassToCheck, class = decltype( &ClassToCheck::DamageZB )>
	constexpr auto GetDamage_ZB(ClassToCheck *p)
	{
		return static_cast<CFinal &>(*this).DamageZB;
	}
	constexpr auto GetDamage_ZB(...)
	{
		return static_cast<CFinal &>(*this).DamageDefault;
	}

	template<class ClassToCheck, class = decltype( &ClassToCheck::DamageZBS )>
	constexpr auto GetDamage_ZBS(ClassToCheck *p)
	{
		return static_cast<CFinal &>(*this).DamageZBS;
	}
	constexpr auto GetDamage_ZBS(...)
	{
		return static_cast<CFinal &>(*this).DamageDefault;
	}
};