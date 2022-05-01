#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "client.h"
#include "cbase/cbase_hash.h"

#include "zombidogegg.h"
#include "gamemode/mods.h"
#include "player/player_mod_strategy.h"
#include "gamemode/mod_zb2.h"
#include "gamemode/zbs/monster_entity.h"

#include <utility>

namespace sv {

LINK_ENTITY_TO_CLASS(zombi_dog_egg, CZombiDogEgg);

void CZombiDogEgg::Spawn(void)
{
	Precache();
	pev->movetype = MOVETYPE_BOUNCE;

	pev->classname = MAKE_STRING("webbomb");

	m_bIsC4 = false;
	pev->solid = SOLID_BBOX;

	SET_MODEL(ENT(pev), "models/meatwall_egg.mdl");
	UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));

	pev->dmg = 30;

	SetTouch(&CZombiDogEgg::OnTouch);
	SetThink(&CZombiDogEgg::OnThink);
}

void CZombiDogEgg::Precache(void)
{
	PRECACHE_MODEL("models/meatwall_egg.mdl");
	m_iExplodeEffect = PRECACHE_MODEL("sprites/ef_meatwall_egg.spr");
}

void EXPORT CZombiDogEgg::OnThink(void)
{
	if (!IsInWorld())
	{
		SetThink(NULL);
		UTIL_Remove(this);
		return;
	}

	pev->nextthink = gpGlobals->time + 0.1s;
}

void EXPORT CZombiDogEgg::OnTouch(CBaseEntity* pOther)
{
	if (ENT(pOther->pev) == pev->owner)
		return;

	pev->model = iStringNull;
	pev->solid = SOLID_NOT;
	pev->takedamage = DAMAGE_NO;
	Vector vecOrigin = pev->origin;
	Vector vecAngles = pev->angles;
	Vector vecVelocity = g_vecZero;

	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
	WRITE_BYTE(TE_TEMPSPRITE);
	WRITE_COORD(vecOrigin.x);
	WRITE_COORD(vecOrigin.y);
	WRITE_COORD(vecOrigin.z);
	WRITE_SHORT(m_iExplodeEffect);
	WRITE_BYTE(10);
	WRITE_BYTE(255);
	WRITE_BYTE(30);
	MESSAGE_END();

	SetTouch(NULL);
	pev->effects |= EF_NODRAW;

	UTIL_Remove(this);

	CMonster* monster = GetClassPtr<CMonster>(nullptr);

	if (!monster)
		return;

	edict_t* pent = monster->edict();

	vecOrigin.z += 36;
	monster->pev->origin = vecOrigin;
	monster->pev->angles = pev->angles;

	DispatchSpawn(pent);

	// default settings
	monster->pev->health = monster->pev->max_health = 500;
	monster->pev->maxspeed = 160.0f;
	monster->m_flAttackDamage = 100.0f;

	SET_MODEL(monster->edict(), "models/monster/zombiedog.mdl");
	UTIL_SetSize(monster->pev, VEC_HULL_MIN, VEC_HULL_MAX);
}

}
