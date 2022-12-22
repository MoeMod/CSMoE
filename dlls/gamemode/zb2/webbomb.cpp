#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "client.h"
#include "cbase/cbase_hash.h"

#include "webbomb.h"
#include "gamemode/mods.h"
#include "player/player_mod_strategy.h"
#include "gamemode/mod_zb2.h"

#include <utility>

namespace sv {

LINK_ENTITY_TO_CLASS(webbomb, CWebBomb);

void CWebBomb::Spawn(void)
{
	Precache();
	pev->movetype = MOVETYPE_BOUNCE;

	pev->classname = MAKE_STRING("webbomb");

	m_bIsC4 = false;
	pev->solid = SOLID_BBOX;

	SET_MODEL(ENT(pev), "models/zb5_za_webbomb.mdl");
	UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));

	pev->dmg = 30;

	SetTouch(&CWebBomb::OnTouch);
	SetThink(&CWebBomb::OnThink);
}

void CWebBomb::Precache(void)
{
	PRECACHE_MODEL("models/zb5_za_webbomb.mdl");
	m_iWebTrap = PRECACHE_MODEL("models/zb5_za_webtrap.mdl");
}

void EXPORT CWebBomb::OnThink(void)
{
	if (!IsInWorld())
	{
		SetThink(NULL);
		UTIL_Remove(this);
		return;
	}

	if (m_flSlowdownTime != invalid_time_point && m_flSlowdownTime < gpGlobals->time)
	{
		SetThink(NULL);
		UTIL_Remove(this);
		return;
	}
	
	if (m_flSlowdownTime > gpGlobals->time)
	{
		Slowdown();
	}

	pev->nextthink = gpGlobals->time + 0.1s;
}

void EXPORT CWebBomb::OnTouch(CBaseEntity* pOther)
{
	if (ENT(pOther->pev) == pev->owner)
		return;

	CBaseEntity* pEntity = NULL;

	//has bug
	//float flRadius;
	//float flExplodeDamage = 300;
	//entvars_t* pevOwner = VARS(pev->owner);
	//while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, 200)) != NULL)
	//{
	//	if (pEntity->IsDormant())
	//		continue;

	//	if (pEntity->pev->takedamage == DAMAGE_NO)
	//		continue;

	//	if (ENT(pEntity->pev) == pev->owner)
	//		continue;

	//	int falloff = flExplodeDamage / flRadius;

	//	float flCurrentDamage = max(0, flExplodeDamage - (pev->origin - pEntity->pev->origin).Length() * falloff);

	//	pEntity->TakeDamage(pev, pevOwner, flCurrentDamage, DMG_NEVERGIB | DMG_EXPLOSION);
	//}

	m_flSlowdownTime = gpGlobals->time + 5.0s;

	pev->model = iStringNull;
	pev->solid = SOLID_NOT;
	pev->takedamage = DAMAGE_NO;
	Vector vecOrigin = pev->origin;
	Vector vecAngles = pev->angles;
	Vector vecVelocity = g_vecZero;

	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
	WRITE_BYTE(TE_TEMPMODEL);
	WRITE_COORD(vecOrigin.x);
	WRITE_COORD(vecOrigin.y);
	WRITE_COORD(vecOrigin.z);
	WRITE_COORD(vecAngles.x);
	WRITE_COORD(vecAngles.y);
	WRITE_COORD(vecAngles.z);
	WRITE_COORD(vecVelocity.x);
	WRITE_COORD(vecVelocity.y);
	WRITE_COORD(vecVelocity.z);
	WRITE_SHORT(m_iWebTrap);
	WRITE_BYTE(50);	//life
	WRITE_SHORT(0);
	WRITE_BYTE(1);
	WRITE_BYTE(false);
	WRITE_BYTE(255);
	WRITE_BYTE(kRenderNormal);
	WRITE_SHORT(-1);
	WRITE_BYTE(0);
	WRITE_BYTE(false);
	WRITE_BYTE(0);
	WRITE_BYTE(0);
	WRITE_SHORT(2);
	WRITE_LONG(0);
	MESSAGE_END();

	SetTouch(NULL);
	pev->effects |= EF_NODRAW;
}

void CWebBomb::Slowdown()
{
	CBaseEntity* pVictim = NULL;
	while ((pVictim = UTIL_FindEntityInSphere(pVictim, pev->origin, 200)) != NULL)
	{
		if (!pVictim->IsPlayer())
			continue;
		if (!pVictim->IsAlive())
			continue;

		CBasePlayer* pPlayer = (CBasePlayer*)pVictim;
		if (pPlayer->m_bIsZombie)
			continue;

		auto strategy = dynamic_cast<CPlayerModStrategy_ZB2*>(pPlayer->m_pModStrategy.get());
		if (strategy)
		{
			strategy->SetSlowdownTime(1.5s);
		}
	}
}


}
