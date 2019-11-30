/*
GetDamageDefault.hpp - part of CSMoE template weapon framework, to auto-gen GetDamage() functions
Copyright (C) 2018 Moemod Hyakuya

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#include <type_traits>

template<class CFinal, class CBase = CBaseTemplateWeapon>
class TGetDamageDefault : public CBase
{
private:
	float GetDamage_ZB()
	{
		auto &wpn = static_cast<CFinal &>(*this);
		return df::DamageZB::Try(wpn, df::DamageDefault::Get(wpn));
	}

	float GetDamage_ZBS()
	{
		auto &wpn = static_cast<CFinal &>(*this);
		return df::DamageZB::Try(wpn, df::DamageDefault::Get(wpn));
	}

	float GetDamage_Default()
	{
		auto &wpn = static_cast<CFinal &>(*this);
		return df::DamageDefault::Get(wpn);
	}

public:
#ifndef CLIENT_DLL
	float GetDamage()
	{
		return	g_pModRunning->DamageTrack() == DT_ZB ? GetDamage_ZB() : (
			g_pModRunning->DamageTrack() == DT_ZBS ? GetDamage_ZBS() :
			GetDamage_Default());
	}

#else
	float GetDamage()
	{
		return GetDamage_Default();
	}
#endif

};
