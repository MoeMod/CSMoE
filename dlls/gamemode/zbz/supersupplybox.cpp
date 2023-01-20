#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "client.h"
#include "cbase/cbase_hash.h"

#include "supersupplybox.h"
#include "gamemode/mods.h"
#include "player/player_mod_strategy.h"
#include "newmenus.h"
#include "client/ghosthuntermenu.h"
#include <utility>

namespace sv {

LINK_ENTITY_TO_CLASS(ghosthunter_supplybox, CGhostHunterSupplyBox);

void CGhostHunterSupplyBox::Precache()
{
	PRECACHE_MODEL("models/ghosthunter_supplybox.mdl");
}

void CGhostHunterSupplyBox::Spawn()
{
	Precache();

	if (pev->classname)
	{
		RemoveEntityHashValue(pev, STRING(pev->classname), CLASSNAME);
	}

	MAKE_STRING_CLASS("ghosthunter_supplybox", pev);
	AddEntityHashValue(pev, STRING(pev->classname), CLASSNAME);

	pev->movetype = MOVETYPE_TOSS;
	pev->solid = SOLID_TRIGGER;

	SetThink(&CMechSupplyBox::SupplyBoxThink);

	UTIL_SetSize(pev, Vector(-16, -16, 0), Vector(16, 16, 32));
	UTIL_SetOrigin(pev, pev->origin);

	SET_MODEL(edict(), "models/ghosthunter_supplybox.mdl");

	m_bCanOpen = false;

	pev->frame = 0;
	pev->sequence = 1;

	ResetSequenceInfo();

	pev->nextthink = gpGlobals->time + 2.1s;
}

void  CGhostHunterSupplyBox::SupplyBoxThink()
{
	pev->sequence = 0;
	m_bCanOpen = true;

	UTIL_TempModel(pev->origin, g_vecZero, g_vecZero, m_iLandingEffectIndex, 10, 0, 10, 0, 0, 255, kRenderNormal, NULL, false, 0, 0, 256, 0);
}


void CGhostHunterSupplyBox::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (!m_bCanOpen)
		return;

	if (!pCaller->IsPlayer())
		return;

	CBasePlayer* p = static_cast<CBasePlayer*>(pCaller);

	if (p->m_bIsZombie)
		return;

	if (!p->m_bIsVIP)
		return;

	if (p->m_iBuff & BUFF_LASTHERO)
		return;

	if (p->m_iBuff & BUFF_MASTERHUNTER)
		return;

	MESSAGE_BEGIN(MSG_ONE, gmsgZBZMsg, nullptr, p->edict());
	WRITE_BYTE(8);
	WRITE_BYTE(4);
	WRITE_BYTE(5);
	MESSAGE_END();

	ghosthuntermenu::ShowGhostHunterMenu(p);

	p->m_iBuff |= BUFF_MASTERHUNTER;
	pev->effects |= EF_NODRAW;
	SUB_Remove();
	return;
}

LINK_ENTITY_TO_CLASS(mech_supplybox, CMechSupplyBox);

void CMechSupplyBox::Precache()
{
	PRECACHE_MODEL("models/mech_supplybox.mdl");
	m_iLandingEffectIndex =  PRECACHE_MODEL("models/zs_allout/ef_ground_zero.mdl");
}

void CMechSupplyBox::Spawn()
{
	Precache();

	if (pev->classname)
	{
		RemoveEntityHashValue(pev, STRING(pev->classname), CLASSNAME);
	}

	MAKE_STRING_CLASS("mech_supplybox", pev);
	AddEntityHashValue(pev, STRING(pev->classname), CLASSNAME);

	pev->movetype = MOVETYPE_TOSS;
	pev->solid = SOLID_TRIGGER;

	UTIL_SetSize(pev, Vector(-16, -16, 0), Vector(16, 16, 32));
	UTIL_SetOrigin(pev, pev->origin);

	SetThink(&CMechSupplyBox::SupplyBoxThink);

	SET_MODEL(edict(), "models/mech_supplybox.mdl");

	m_bCanOpen = false;

	pev->frame = 0;
	pev->sequence = 1;

	ResetSequenceInfo();

	pev->nextthink = gpGlobals->time + 2.1s;
}

void  CMechSupplyBox::SupplyBoxThink()
{
	pev->sequence = 0;
	m_bCanOpen = true;

	UTIL_TempModel(pev->origin, g_vecZero, g_vecZero, m_iLandingEffectIndex, 10, 0, 10, 0, 0, 255, kRenderNormal, NULL, false, 0, 0, 256, 0);
}

void CMechSupplyBox::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (!m_bCanOpen)
		return;

	if (!pCaller->IsPlayer())
		return;

	CBasePlayer* p = static_cast<CBasePlayer*>(pCaller);

	if (p->m_bIsZombie)
		return;

	if (!p->m_bIsVIP)
		return;
	
	if (p->m_iBuff & (BUFF_GHOSTHUNTER | BUFF_MASTERHUNTER))
		return;

	if (p->m_iBuff & BUFF_LASTHERO)
		return;

	p->m_iBuff |= BUFF_LASTHERO;

	p->RemoveAllItems(FALSE);
	p->m_bHasPrimary = false;
	p->m_bHolsterDisabled = true;

	p->SetNewPlayerModel("models/player/lasthero/lasthero.mdl");
	p->GiveNamedItem("weapon_herochainsaw");

	UTIL_ClientPrintAll(HUD_PRINTCENTER, "动力机甲 %s 已投放完毕", STRING(p->pev->netname));

	EMIT_SOUND(ENT(p->pev), CHAN_BODY, "zombi/get_box.wav", VOL_NORM, ATTN_NORM);

	pev->effects |= EF_NODRAW;
	SUB_Remove();
}

}
