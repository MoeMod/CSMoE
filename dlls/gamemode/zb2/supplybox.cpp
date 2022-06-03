#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "client.h"
#include "cbase/cbase_hash.h"

#include "supplybox.h"
#include "gamemode/mods.h"
#include "player/player_mod_strategy.h"
#include "weapons_moe_buy.h"
#include "newmenus.h"
#include "client/ghosthuntermenu.h"
#include <utility>

namespace sv {

static std::pair<const char *, void(*)(CBasePlayer *p)> g_SupplyboxItems[]=
{
	{ "NightVision && Grenades", [](CBasePlayer *p) {
		for (auto iSlot : { PRIMARY_WEAPON_SLOT, PISTOL_SLOT})
			{
				p->m_pModStrategy->GiveSlotAmmo(p, iSlot);
			}

		p->GiveNamedItem("weapon_hegrenade");
		p->GiveNamedItem("weapon_smokegrenade");
		p->GiveNamedItem("weapon_flashbang");
		p->GiveNamedItem("weapon_flashbang");

		if (!(p->m_flDisplayHistory & DHF_NIGHTVISION))
		{
			p->HintMessage("#Hint_use_nightvision");
			p->m_flDisplayHistory |= DHF_NIGHTVISION;
		}
		EMIT_SOUND(ENT(p->pev), CHAN_ITEM, "items/equip_nvg.wav", VOL_NORM, ATTN_NORM);
		p->m_bHasNightVision = true;
		SendItemStatus(p);

		}
	},
	{ "Dual MP7A1", [](CBasePlayer* p) {
		for (auto iSlot : { PRIMARY_WEAPON_SLOT, PISTOL_SLOT})
		{
			p->m_pModStrategy->GiveSlotAmmo(p, iSlot);
		}
		DropPrimary(p);
		p->GiveNamedItem("weapon_mp7a1d");
		int iAmount = p->m_pModStrategy->ComputeMaxAmmo("46mm", MAX_AMMO_46MM);
		p->GiveAmmo(iAmount, "46mm", iAmount);
		}
	},
	{ "AK-47 60R", [](CBasePlayer* p) {
			for (auto iSlot : { PRIMARY_WEAPON_SLOT, PISTOL_SLOT})
			{
				p->m_pModStrategy->GiveSlotAmmo(p, iSlot);
			}
			DropPrimary(p);
			p->GiveNamedItem("weapon_ak47l");
			int iAmount = p->m_pModStrategy->ComputeMaxAmmo("762Nato", MAX_AMMO_762NATO);
			p->GiveAmmo(iAmount, "762Nato", iAmount);
		}
	},
	{ "Dual Desert Eagle", [](CBasePlayer* p) {
			for (auto iSlot : { PRIMARY_WEAPON_SLOT, PISTOL_SLOT})
			{
				p->m_pModStrategy->GiveSlotAmmo(p, iSlot);
			}
			DropSecondary(p);
			p->GiveNamedItem("weapon_deagled");
			int iAmount = p->m_pModStrategy->ComputeMaxAmmo("50ae", MAX_AMMO_50AE);
			p->GiveAmmo(iAmount, "50AE", iAmount);
		}
	}
};

LINK_ENTITY_TO_CLASS(supplybox, CSupplyBox);

void CSupplyBox::Precache()
{
	PRECACHE_SOUND("zombi/get_box.wav");
	PRECACHE_MODEL("models/supplybox.mdl");
}

void CSupplyBox::Spawn()
{
	Precache();

	if (pev->classname)
	{
		RemoveEntityHashValue(pev, STRING(pev->classname), CLASSNAME);
	}

	MAKE_STRING_CLASS("supplybox", pev);
	AddEntityHashValue(pev, STRING(pev->classname), CLASSNAME);

	pev->movetype = MOVETYPE_TOSS;
	pev->solid = SOLID_TRIGGER;

	UTIL_SetSize(pev, Vector(-16, -16, 0), Vector(16, 16, 16));
	UTIL_SetOrigin(pev, pev->origin);
	SetTouch(&CSupplyBox::SupplyboxTouch);
	SetThink(&CSupplyBox::SupplyboxThink);

	SET_MODEL(edict(), "models/supplybox.mdl");

	m_flNextRadarTime = gpGlobals->time + RandomDuration(0.0s, 1.0s);
}

void CSupplyBox::SupplyboxTouch(CBaseEntity *pOther)
{
	if (!pOther->IsPlayer())
		return;

	CBasePlayer *p = static_cast<CBasePlayer *>(pOther);

	if (p->m_bIsVIP || p->m_bIsZombie || p->m_iBuff & BUFF_MASTERHUNTER)
		return;

	if (p->m_iBuff & BUFF_GHOSTHUNTER)
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgZBZMsg, nullptr, p->edict());
		WRITE_BYTE(7);
		WRITE_BYTE(4);
		WRITE_BYTE(5);
		MESSAGE_END();
		
		ghosthuntermenu::ShowGhostHunterMenu(p);
	
		p->m_iBuff |= BUFF_MASTERHUNTER;
		pev->effects |= EF_NODRAW;
		SendPositionMsg();
		SUB_Remove();
		return;
	}

	const char *mod_str = CVAR_GET_STRING("mp_gamemode");
	if(strcmp(mod_str, "zb3f2p"))
	{
		for (auto iSlot : { PRIMARY_WEAPON_SLOT, PISTOL_SLOT })
		{
			p->m_pModStrategy->GiveSlotAmmo(p, iSlot);
		}

		DropPrimary(p);
		DropSecondary(p);
		p->RemoveAllItems(FALSE, false);
		p->m_bHasPrimary = false;

		const char* weapon_names_view[5];

		for (auto iSlot : { PRIMARY_WEAPON_SLOT, PISTOL_SLOT, KNIFE_SLOT, GRENADE_SLOT })
		{
			std::vector<MoEWeaponBuyInfo_s> candidate_list;
#ifndef XASH_DEDICATED
			std::copy_if(std::begin(g_MoEWeaponBuyInfoLocal), std::end(g_MoEWeaponBuyInfoLocal), std::back_inserter(candidate_list), [iSlot](const MoEWeaponBuyInfo_s& info) { return info.iSlot == iSlot; });
#else
			std::copy_if(std::begin(g_MoEWeaponBuyInfo), std::end(g_MoEWeaponBuyInfo), std::back_inserter(candidate_list), [iSlot](const MoEWeaponBuyInfo_s& info) { return info.iSlot == iSlot; });
#endif // !XASH_DEDICATED
			std::shuffle(candidate_list.begin(), candidate_list.end(), std::random_device());
			const MoEWeaponBuyInfo_s& info = candidate_list.front();
			if (iSlot == KNIFE_SLOT) {
				p->m_iKnifeID = info.iID;
			} else if (iSlot == GRENADE_SLOT) {
				p->m_iGrenadeID = info.iID;
			}
			p->GiveNamedItem(info.pszClassName);
			p->m_pModStrategy->GiveSlotAmmo(p, iSlot);
			weapon_names_view[iSlot] = info.pszDisplayName;
		}

		//p->GiveNamedItem("weapon_hegrenade");
		p->GiveNamedItem("weapon_smokegrenade");
		p->GiveNamedItem("weapon_flashbang");
		p->GiveNamedItem("weapon_flashbang");

		if (!(p->m_flDisplayHistory & DHF_NIGHTVISION))
		{
			p->HintMessage("#Hint_use_nightvision");
			p->m_flDisplayHistory |= DHF_NIGHTVISION;
		}
		EMIT_SOUND(ENT(p->pev), CHAN_ITEM, "items/equip_nvg.wav", VOL_NORM, ATTN_NORM);
		p->m_bHasNightVision = true;
		SendItemStatus(p);

		UTIL_ClientPrintAll(HUD_PRINTCENTER, "%s1 获得补给物品 (%s2, %s3, %s4).", STRING(p->pev->netname), weapon_names_view[PRIMARY_WEAPON_SLOT], weapon_names_view[PISTOL_SLOT], weapon_names_view[KNIFE_SLOT]);
		ClientPrint(p->pev, HUD_PRINTCENTER, "你获得了补给物品 (%s1, %s2, %s3).", weapon_names_view[PRIMARY_WEAPON_SLOT], weapon_names_view[PISTOL_SLOT], weapon_names_view[KNIFE_SLOT]);
	}
	else
	{
		auto &nf = g_SupplyboxItems[RANDOM_LONG(0, std::extent<decltype(g_SupplyboxItems)>::value - 1)];
		nf.second(p);

		UTIL_ClientPrintAll(HUD_PRINTCENTER, "%s1 获得补给物品 (%s2).", STRING(p->pev->netname), nf.first);
		ClientPrint(p->pev, HUD_PRINTCENTER, "你获得了补给物品 (%s1).", nf.first);
	}
	
	EMIT_SOUND(ENT(p->pev), CHAN_BODY, "zombi/get_box.wav", VOL_NORM, ATTN_NORM);

	pev->effects |= EF_NODRAW;
	SendPositionMsg();
	SUB_Remove();
}

void CSupplyBox::SupplyboxThink()
{
	if (pev->deadflag != DEAD_DEAD && !(pev->effects & EF_NODRAW))
	{
		if (m_flNextRadarTime <= gpGlobals->time)
		{
			SendPositionMsg();
			m_flNextRadarTime = gpGlobals->time + 1s;
		}
	}
}

void CSupplyBox::SendPositionMsg()
{
	CBaseEntity *pEntity = NULL;

	while ((pEntity = UTIL_FindEntityByClassname(pEntity, "player")) != NULL)
	{
		if (FNullEnt(pEntity->edict()))
			break;

		if (!pEntity->IsPlayer())
			continue;

		if (pEntity->pev->flags == FL_DORMANT)
			continue;

		CBasePlayer *pTempPlayer = static_cast<CBasePlayer *>(pEntity);

		if (pTempPlayer->pev->deadflag == DEAD_NO && pTempPlayer->m_iTeam == CT)
		{
			if (pev->effects & EF_NODRAW)
			{
				MESSAGE_BEGIN(MSG_ONE, gmsgHostageK, NULL, pTempPlayer->pev);
				WRITE_BYTE(m_iSupplyboxIndex);
				MESSAGE_END();
			}
			else
			{
				MESSAGE_BEGIN(MSG_ONE, gmsgHostagePos, NULL, pTempPlayer->pev);
				WRITE_BYTE(0);
				WRITE_BYTE(m_iSupplyboxIndex);
				WRITE_COORD(pev->origin.x);
				WRITE_COORD(pev->origin.y);
				WRITE_COORD(pev->origin.z);
				MESSAGE_END();
			}
			
		}
	}
}

}
