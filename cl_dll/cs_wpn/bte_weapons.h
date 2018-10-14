#pragma once

class CBasePlayer;
class CBasePlayerWeapon;

// make this as a global var, to auto register...
// check it in util.h -> LINK_ENTITY_TO_CLASS
template<class T>
class CBTEClientWeapons_AutoRegister
{
public:
	// I am a SFINAE static checker!
	template<class = typename std::enable_if<std::is_base_of<CBasePlayerWeapon, T>::value>::type>
	CBTEClientWeapons_AutoRegister(const char *name)
	{
		BTEClientWeapons().AddToFindList(name, WeaponEntityPlaceHolderFactory);
	}
	CBTEClientWeapons_AutoRegister(...)
	{
		//static_assert(0, "not a weapon!");
	}

private:
	static CBasePlayerWeapon *WeaponEntityPlaceHolderFactory() // Static
	{
		static T w;
		static entvars_t ev;

		CBasePlayerWeapon *pEntity = &w;
		pEntity->pev = &ev;
		pEntity->Precache();
		pEntity->Spawn();

		return pEntity;
	}
};

class CBTEClientWeapons
{
private:
	CBTEClientWeapons();

public:
	void Init();

	void PrepEntity(CBasePlayer *pWeaponOwner);
	void ActiveWeapon(const char *name);
	CBasePlayerWeapon *GetActiveWeaponEntity()
	{
		return m_pActiveWeapon;
	}

private:
	CBasePlayerWeapon *m_pActiveWeapon;

private:
	template<class T>
	friend class CBTEClientWeapons_AutoRegister;

	void AddToFindList(const char *name, CBasePlayerWeapon *(*pfn)());

public:
	class WeaponEntityFindList_t;
	static WeaponEntityFindList_t staticWeaponEntityFindList;

public:
	// singleton accessor
	friend CBTEClientWeapons &BTEClientWeapons();
};