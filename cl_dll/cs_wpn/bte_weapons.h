#pragma once

class CBasePlayer;
class CBasePlayerWeapon;

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
	static WeaponEntityFindList_t &WeaponEntityFindList();

public:
	// singleton accessor
	friend CBTEClientWeapons &BTEClientWeapons();
};

// non-msvc needs this...
CBTEClientWeapons &BTEClientWeapons();