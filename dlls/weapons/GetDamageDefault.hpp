//
// Created by 小白白 on 2019-01-19.
//

#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif

#include <type_traits>

template<class CFinal, class CBase = CBaseTemplateWeapon>
class TGetDamageDefault : public CBase
{
private:
	template<class ClassToCheck>
	auto GetDamage_ZB(ClassToCheck *p) -> decltype(ClassToCheck::DamageZB)
	{
		return static_cast<CFinal &>(*this).DamageZB;
	}
	template<class ClassToCheck = CFinal>
	auto GetDamage_ZB(...) -> decltype(ClassToCheck::DamageDefault)
	{
		return static_cast<CFinal &>(*this).DamageDefault;
	}

	template<class ClassToCheck>
	auto GetDamage_ZBS(ClassToCheck *p) -> decltype(ClassToCheck::DamageZBS)
	{
		return static_cast<CFinal &>(*this).DamageZBS;
	}
	template<class ClassToCheck = CFinal>
	auto GetDamage_ZBS(...) -> decltype(ClassToCheck::DamageDefault)
	{
		return static_cast<CFinal &>(*this).DamageDefault;
	}

public:
#ifndef CLIENT_DLL
	template<class ClassToFind = CFinal>
	auto GetDamage() -> typename std::common_type<
			decltype(this->GetDamage_ZB(static_cast<ClassToFind *>(this))),
			decltype(this->GetDamage_ZBS(static_cast<ClassToFind *>(this))) ,
			decltype(ClassToFind::DamageDefault)
		>::type
	{
		return	g_pModRunning->DamageTrack() == DT_ZB ? GetDamage_ZB(static_cast<CFinal *>(this)) : (
			g_pModRunning->DamageTrack() == DT_ZBS ? GetDamage_ZBS(static_cast<CFinal *>(this)) :
			static_cast<CFinal &>(*this).DamageDefault);
	}

#else
	template<class ClassToFind = CFinal>
	auto GetDamage(void) -> decltype(ClassToFind::DamageDefault)
	{
		return static_cast<CFinal &>(*this).DamageDefault;
	}
#endif

};
