#pragma once

class CBasePlayerWeapon;

// make this as a global var, to auto register...
// check it in util.h -> LINK_ENTITY_TO_CLASS

void InitializeWeaponEntity(CBasePlayerWeapon *pEntity, entvars_t *pev); // bte_weapons.cpp

template<class T>
class CBTEClientWeapons_AutoRegister
{
public:
	CBTEClientWeapons_AutoRegister(const char *name)
	{
		BTEClientWeapons().AddToFindList(name, WeaponEntityPlaceHolderFactory);
	}

private:
	static CBasePlayerWeapon *WeaponEntityPlaceHolderFactory() // Static
	{
		static T w;
		static entvars_t ev;

		CBasePlayerWeapon *pEntity = &w;
		InitializeWeaponEntity(pEntity, &ev);

		return pEntity;
	}
};