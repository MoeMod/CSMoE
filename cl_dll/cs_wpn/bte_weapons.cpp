
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"

#include "com_weapons.h"

#include "parsemsg.h"

#include "bte_weapons.h"
#include "weapons_msg.h"

#include "wpn_shared.h"

#include <string>
#include <map>

/*
	Weapon Registers
*/
template<class T>
CBasePlayerWeapon *WeaponEntityPlaceHolderFactory() // Static
{
	static T w;
	static entvars_t ev;

	CBasePlayerWeapon *pEntity = &w;
	pEntity->pev = &ev;
	pEntity->Precache();
	pEntity->Spawn();

	return pEntity;
}

const std::map<std::string, CBasePlayerWeapon *(*)()> g_WeaponEntityFindList = {
	{ "weapon_ak47l", WeaponEntityPlaceHolderFactory<CAK47_Long> },
	{ "weapon_deagled", WeaponEntityPlaceHolderFactory<CDeagleD> },
	{ "weapon_mp7a1d", WeaponEntityPlaceHolderFactory<CMP7A1D> }
};

/*
	Entity Pools
*/
extern CBasePlayerWeapon *g_pWpns[MAX_WEAPONS]; // cs_weapons.cpp

/*
	CBTEClientWeapons impls
*/

void CBTEClientWeapons::PrepEntity(CBasePlayer *pWeaponOwner)
{
	for (auto &kv : g_WeaponEntityFindList)
	{
		CBasePlayerWeapon *pEntity = kv.second();

		if (pWeaponOwner)
		{
			((CBasePlayerWeapon *)pEntity)->m_pPlayer = pWeaponOwner;
		}
	}
	
}

void CBTEClientWeapons::ActiveWeapon(const char *name)
{
	m_pActiveWeapon = nullptr;

	auto iter = g_WeaponEntityFindList.find({ name });
	if (iter != g_WeaponEntityFindList.end())
	{
		m_pActiveWeapon = iter->second();

		ItemInfo info;
		memset(&info, 0, sizeof(ItemInfo));
		m_pActiveWeapon->GetItemInfo(&info);
		g_pWpns[info.iId] = m_pActiveWeapon;
	}
	
}

/*
	Message Handlers
*/

typedef void MsgDispatchFunction_t(BufferReader &reader);

void MsgDispatch_Active(BufferReader &reader)
{
	const char *name = reader.ReadString();
	BTEClientWeapons().ActiveWeapon(name);
}

MsgDispatchFunction_t *gMsgDispatchFunctions[BTE_Weapon_MaxMsgs] = {
	MsgDispatch_Active
};

int __MsgFunc_BTEWeapon(const char *pszName, int iSize, void *pbuf)
{
	BufferReader reader(pszName, pbuf, iSize);

	BTEWeaponMsgType type = static_cast<BTEWeaponMsgType>(reader.ReadByte());
	gMsgDispatchFunctions[type](reader);
	return 1;
}


void CBTEClientWeapons::Init()
{
	gEngfuncs.pfnHookUserMsg("BTEWeapon", __MsgFunc_BTEWeapon);
}

CBTEClientWeapons::CBTEClientWeapons() : m_pActiveWeapon(nullptr)
{
	
}

CBTEClientWeapons &BTEClientWeapons()
{
	static CBTEClientWeapons x;
	return x;
}