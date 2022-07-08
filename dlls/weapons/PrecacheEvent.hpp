/*
PrecacheEvent.hpp - part of CSMoE template weapon framework, to auto-gen Precache() function to precache events
Copyright (C) 2019 Moemod Hyakuya

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

template<class CFinal, class CBase = CBaseTemplateWeapon>
class TPrecacheEvent : public CBase
{
public:
	void Precache(void) override
	{
		auto &wpn = static_cast<CFinal &>(*this);
		auto &&data = wpn.WeaponTemplateDataSource();
		m_usFire = PRECACHE_EVENT(1, df::EventFile::Get(data));
		CBase::Precache();
	}

	unsigned short m_usFire;
};