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
	auto GetDamage_ZB(ClassToCheck *p) -> decltype(static_cast<ClassToCheck &>(*this).DamageZB)
	{
		return static_cast<CFinal &>(*this).DamageZB;
	}
	template<class ClassToCheck = CFinal>
	auto GetDamage_ZB(...) -> decltype(static_cast<ClassToCheck &>(*this).DamageDefault)
	{
		return static_cast<CFinal &>(*this).DamageDefault;
	}

	template<class ClassToCheck>
	auto GetDamage_ZBS(ClassToCheck *p) -> decltype(static_cast<ClassToCheck &>(*this).DamageZBS)
	{
		return static_cast<CFinal &>(*this).DamageZBS;
	}
	template<class ClassToCheck = CFinal>
	auto GetDamage_ZBS(...) -> decltype(static_cast<ClassToCheck &>(*this).DamageDefault)
	{
		return static_cast<CFinal &>(*this).DamageDefault;
	}

public:
#ifndef CLIENT_DLL
	template<class ClassToFind = CFinal>
	auto GetDamage(void) -> decltype
	(
		true ? this->GetDamage_ZB(static_cast<ClassToFind *>(this)) :
		true ? this->GetDamage_ZBS(static_cast<ClassToFind *>(this)) :
		static_cast<ClassToFind &>(*this).DamageDefault
		)
	{
		return	g_pModRunning->DamageTrack() == DT_ZB ? GetDamage_ZB(static_cast<CFinal *>(this)) : (
			g_pModRunning->DamageTrack() == DT_ZBS ? GetDamage_ZBS(static_cast<CFinal *>(this)) :
			static_cast<CFinal &>(*this).DamageDefault);
	}

#else
	template<class ClassToFind = CFinal>
	auto GetDamage(void) -> decltype(static_cast<ClassToFind &>(*this).DamageDefault)
	{
		return static_cast<CFinal &>(*this).DamageDefault;
	}
#endif

};
