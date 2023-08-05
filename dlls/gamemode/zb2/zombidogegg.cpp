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

	pev->classname = MAKE_STRING("zombi_dog_egg");

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
	m_iBossBig01Index = PRECACHE_MODEL("models/monster/zbs_bossl_big01.mdl");
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
	pev->velocity = g_vecZero;

	CMonster* monster = GetClassPtr<CMonster>(nullptr);

	if (!monster)
		return;

	edict_t* pent = monster->edict();

	monster->pev->origin = vecOrigin;
	monster->pev->angles = pev->angles;
	monster->SetClass(MONSTER_CLASS_ZOMBIEDOG);
	pent->v.spawnflags |= SF_NORESPAWN;
	DispatchSpawn(pent);

	// default settings
	monster->pev->health = monster->pev->max_health = 500;
	monster->pev->maxspeed = 160.0f;
	monster->m_flAttackDamage = 100.0f;


	if (!RANDOM_LONG(0, 5))
	{
		UTIL_TempModel(pev->origin, g_vecZero, g_vecZero, m_iBossBig01Index, 30, 9, 10, true, 64, 255, kRenderNormal, NULL, false, 0, 0, 256, 0);

		SetThink(&CZombiDogEgg::DropGunEX);
		pev->nextthink = gpGlobals->time + 2.1s;
	}
	else
	{
		SetThink(nullptr);
		UTIL_Remove(this);
	}
}

void CZombiDogEgg::DropGunEX()
{
	SetThink(nullptr);

	TraceResult tr;
	float flDamage;
	CBaseEntity* pList[100];
	Vector vecStart;

	vecStart = pev->origin + Vector(0.0, 0.0, 5.0);

	const float flRange = 2.5 * 39.37;
	int iCount = UTIL_EntitiesInBox(pList, 100, vecStart - Vector(flRange, flRange, flRange), vecStart + Vector(flRange, flRange, flRange), FL_CLIENT | FL_MONSTER);

	flDamage = 500.0;

	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, vecStart);
	WRITE_BYTE(TE_BEAMCYLINDER);
	WRITE_COORD(vecStart[0]);
	WRITE_COORD(vecStart[1]);
	WRITE_COORD(vecStart[2]);
	WRITE_COORD(vecStart[0]);
	WRITE_COORD(vecStart[1]);
	WRITE_COORD(vecStart[2] + 200);
	WRITE_SHORT(g_sModelIndexShockWave);
	WRITE_BYTE(0); // Start Frame
	WRITE_BYTE(0); // Framerate
	WRITE_BYTE(5); // Live Time
	WRITE_BYTE(60); // Width
	WRITE_BYTE(0); // Noise
	WRITE_BYTE(130); // R
	WRITE_BYTE(139); // G
	WRITE_BYTE(253); // B
	WRITE_BYTE(150); // Bright
	WRITE_BYTE(0); // Speed
	MESSAGE_END();
	
	for (int i = 0; i < iCount; i++)
	{
		CBaseEntity* pEntity = pList[i];

		if (pEntity == this)
			continue;

		if (!pEntity->IsPlayer())
			continue;

		if (pEntity->pev->takedamage == DAMAGE_NO)
			continue;

		if (pEntity->edict() == pev->owner)
			continue;

		UTIL_TraceLine(vecStart, pEntity->BodyTarget(vecStart), dont_ignore_monsters, edict(), &tr);

		if (tr.flFraction < 1.0 && tr.pHit != pEntity->edict())
			continue;

		pEntity->TakeDamage(pev, VARS(pev->owner), flDamage, DMG_EXPLOSION);

		((CBasePlayer*)pEntity)->ReactToDeimosTail();
	}

	UTIL_Remove(this);
}

}
