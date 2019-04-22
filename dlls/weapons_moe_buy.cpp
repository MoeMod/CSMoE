#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "gamerules.h"
#include "client.h"
#include "weapons_moe_buy.h"

#include <algorithm>

struct MoEWeaponBuyInfo_s
{
	const char *pszClassName;
	int iCost;
	InventorySlotType iSlot;
	TeamName team;
};

static MoEWeaponBuyInfo_s g_MoEWeaponBuyInfo[] = {
	{ "weapon_infinity", 1500, PISTOL_SLOT, UNASSIGNED },
	{ "weapon_infinityss", 600, PISTOL_SLOT, UNASSIGNED },
	{ "weapon_infinitysr", 600, PISTOL_SLOT, UNASSIGNED },
	{ "weapon_infinitysb", 600, PISTOL_SLOT, UNASSIGNED }

};

bool HasPlayerItem(CBasePlayer *pPlayer, const char *pszCheckItem)
{
	for (CBasePlayerItem *pItem : pPlayer->m_rgpPlayerItems)
	{
		while (pItem != nullptr)
		{
			if (FClassnameIs(pItem->pev, pszCheckItem))
				return true;

			pItem = pItem->m_pNext;
		}
	}
	return false;
}

bool MoE_HandleBuyCommands(CBasePlayer *pPlayer, const char *pszCommand)
{
	if (!pPlayer->CanPlayerBuy(true))
		return false;

	if (HasPlayerItem(pPlayer, pszCommand))
	{
		ClientPrint(pPlayer->pev, HUD_PRINTCENTER, "#Cstrike_Already_Own_Weapon");
		return false;
	}
		
	auto iter = std::find_if(std::begin(g_MoEWeaponBuyInfo), std::end(g_MoEWeaponBuyInfo), 
		[pszCommand](const MoEWeaponBuyInfo_s &info) {
			return !strcmp(info.pszClassName, pszCommand);
		}
	);

	if (iter != std::end(g_MoEWeaponBuyInfo))
	{
		if (pPlayer->m_iAccount < iter->iCost)
		{
			ClientPrint(pPlayer->pev, HUD_PRINTCENTER, "#Not_Enough_Money");
			BlinkAccount(pPlayer, 2);

			return false;
		}

		switch (iter->iSlot)
		{
		case PRIMARY_WEAPON_SLOT:
			DropPrimary(pPlayer);
			break;
		case PISTOL_SLOT :
			DropSecondary(pPlayer);
			break;
		case KNIFE_SLOT:
			if (pPlayer->m_rgpPlayerItems[KNIFE_SLOT])
			{
				pPlayer->RemovePlayerItem(pPlayer->m_rgpPlayerItems[KNIFE_SLOT]);
			}
			break;
		default:
			break;
		}

		pPlayer->GiveNamedItem(iter->pszClassName);
		pPlayer->AddAccount(-iter->iCost);
		
		return true;
	}

	return false;
}