/*
zb3_morale.h - Zombie Hero Human Morale System (Server)
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

#pragma once

enum ZB3HumanMoraleType_e : byte;

class CZB3HumanMorale
{
public:
	constexpr CZB3HumanMorale() : m_iHumanMoraleLevel(0) {}

	constexpr float DamageModifier(ZB3HumanMoraleType_e type) const 
	{ 
		if(type == ZB3_MORALE_STRENGTHEN)
			return 1.3f + m_iHumanMoraleLevel * 0.1f;
		return 1.0f + m_iHumanMoraleLevel * 0.1f; 
	}

	constexpr int GetMoraleLevel() const { return m_iHumanMoraleLevel; }

	bool LevelUp() 
	{
		if (m_iHumanMoraleLevel < 10)
		{
			++m_iHumanMoraleLevel;
			return true;
		}
		return false;
	}
	void ResetLevel() { m_iHumanMoraleLevel = 0; }
	void UpdateHUD(CBasePlayer * pPlayer = nullptr, ZB3HumanMoraleType_e type = ZB3_MORALE_DEFAULT) const;

protected:
	int m_iHumanMoraleLevel;
};