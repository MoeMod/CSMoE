/*
CheckAccuracyBoundary.hpp
Copyright (C) 2019 Moemod Haoyuan

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#pragma once

template<class CFinal, class CBase = CBaseTemplateWeapon>
class TCheckAccuracyBoundary : public CBase
{
public:
	void CheckAccuracyBoundary()
	{
		CFinal &wpn = static_cast<CFinal &>(*this);
		CheckAccuracyBoundaryMin(df::AccuracyMin::Has(wpn));
		CheckAccuracyBoundaryMax(df::AccuracyMax::Has(wpn));
	}


private:
	void CheckAccuracyBoundaryMin(std::true_type)
	{
		CFinal &wpn = static_cast<CFinal &>(*this);
		if (CBase::m_flAccuracy < df::AccuracyMin::Get(wpn))
			CBase::m_flAccuracy = df::AccuracyMin::Get(wpn);
	}
	constexpr void CheckAccuracyBoundaryMin(std::false_type) {}

	void CheckAccuracyBoundaryMax(std::true_type)
	{
		CFinal &wpn = static_cast<CFinal &>(*this);
		if (CBase::m_flAccuracy > df::AccuracyMax::Get(wpn))
			CBase::m_flAccuracy = df::AccuracyMax::Get(wpn);
	}
	constexpr void CheckAccuracyBoundaryMax(std::false_type) {}
};