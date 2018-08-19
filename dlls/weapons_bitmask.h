
#ifndef WEAPONS_BITMASK_H
#define WEAPONS_BITMASK_H
#ifdef _WIN32
#pragma once
#endif

// handler for pev->weapons
// Not all weapons should be set.

class WeaponsBitmask_Handler
{
private:
	int &bits;
public:
	constexpr WeaponsBitmask_Handler(int &weapons_bitmask) : bits(weapons_bitmask) {}

	static bool IsBitValid(int i)
	{
		return true;
		//return i == WEAPON_SUIT || i == WEAPON_KNIFE || i == WEAPON_C4 || i == WEAPON_HEGRENADE || i == WEAPON_FLASHBANG || i == WEAPON_SMOKEGRENADE || i == WEAPON_SHIELDGUN;
	}

	bool Set(int i) 
	{
		if (!IsBitValid(i)) return false;
		bits |= (1 << i);
		return true;
	}
	bool UnSet(int i)
	{
		if (!IsBitValid(i)) return false;
		bits &= ~(1 << i);
		return true;
	}
	bool Clear()
	{
		bits = 0;
	}
};

#endif
