#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "client.h"

#include "supplybox.h"
#include "gamemode/mods.h"
#include "player/player_mod_strategy.h"

#include <utility>

namespace sv {

static std::pair<const char *, void(*)(CBasePlayer *p)> g_SupplyboxItems[]=
{
	{ "NightVision", [](CBasePlayer *p) {
			p->AddAccount(16000);
			BuyItem(p, MENU_SLOT_ITEM_NVG);
			p->AddAccount(16000);
			p->m_bNightVisionOn = false;
			p->ClientCommand("nightvision");
		}
	},
	{ "Grenades, Ammo Set", [](CBasePlayer *p) {
			p->AddAccount(16000);

			while (BuyAmmo(p, PRIMARY_WEAPON_SLOT, false))
				;
			while (BuyAmmo(p, PISTOL_SLOT, false))
				;
			BuyItem(p, MENU_SLOT_ITEM_FLASHGREN);
			BuyItem(p, MENU_SLOT_ITEM_FLASHGREN);
			BuyItem(p, MENU_SLOT_ITEM_HEGREN);
			BuyItem(p, MENU_SLOT_ITEM_SMOKEGREN);

			p->AddAccount(16000);
		}
	},
	{ "Dual MP7A1", [](CBasePlayer *p) {
			DropPrimary(p);
			p->GiveNamedItem("weapon_mp7a1d");
			int iAmount = p->m_pModStrategy->ComputeMaxAmmo("46mm", MAX_AMMO_46MM);
			p->GiveAmmo(iAmount, "46mm", iAmount);
		}
	},
	{ "AK-47 60R", [](CBasePlayer *p) {
			DropPrimary(p);
			p->GiveNamedItem("weapon_ak47l");
			int iAmount = p->m_pModStrategy->ComputeMaxAmmo("762Nato", MAX_AMMO_762NATO);
			p->GiveAmmo(iAmount, "762Nato", iAmount);
		}
	},
	{ "Dual Desert Eagle", [](CBasePlayer *p) {
			DropSecondary(p);
			p->GiveNamedItem("weapon_deagled");
			int iAmount = p->m_pModStrategy->ComputeMaxAmmo("50ae", MAX_AMMO_50AE);
			p->GiveAmmo(iAmount, "762Nato", iAmount);
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

	m_flNextRadarTime = gpGlobals->time + RANDOM_FLOAT(0, 1);
}

void CSupplyBox::SupplyboxTouch(CBaseEntity *pOther)
{
	if (!pOther->IsPlayer())
		return;

	CBasePlayer *p = static_cast<CBasePlayer *>(pOther);

	if (p->m_bIsVIP || p->m_bIsZombie)
		return;

	auto &nf = g_SupplyboxItems[RANDOM_LONG(0, std::extent<decltype(g_SupplyboxItems)>::value - 1)];
	nf.second(p);

	UTIL_ClientPrintAll(HUD_PRINTCENTER, "%s1 obtained supply item (%s2).", STRING(p->pev->netname), nf.first);
	ClientPrint(p->pev, HUD_PRINTCENTER, "You obtained supply item (%s1).", nf.first);
	
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
			m_flNextRadarTime = gpGlobals->time + 1;
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
