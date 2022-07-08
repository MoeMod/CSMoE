#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif

namespace sv {
	LINK_ENTITY_TO_CLASS(balrog11_cannon, CBalrog11Cannon)

	void CBalrog11Cannon::Spawn()
	{
		Precache();
		
		CBaseEntity* pOwner = CBaseEntity::Instance(pev->owner);
		CBasePlayer* pEntityOwner = nullptr;
		if (pOwner && pOwner->IsPlayer())
			pEntityOwner = static_cast<CBasePlayer*>(pOwner);
		
		int iType = 0;
		CBasePlayerWeapon* pWeapon = (CBasePlayerWeapon*)pEntityOwner->m_rgpPlayerItems[PRIMARY_WEAPON_SLOT];
		if (pWeapon != NULL && !Q_strcmp(STRING(pWeapon->pev->classname), "weapon_balrog11b"))
		{
			SET_MODEL(this->edict(), "sprites/ef_hwater.spr");
			iType = 1;
		}
		else
			SET_MODEL(this->edict(), "sprites/flame_puff01.spr");

		pev->frame = 0.0;
		pev->effects = 0;
		pev->skin = 0;
		pev->body = 0;
		pev->solid = SOLID_TRIGGER; // 2	
		pev->rendermode = kRenderTransAdd; // 5;
		pev->renderfx = kRenderFxNone;
		pev->renderamt = 90.0;
		pev->movetype = MOVETYPE_FLYMISSILE; // 9
		pev->angles.z = (float)RANDOM_LONG(0, 180);

		SetThink(&CBalrog11Cannon::AnimationThink);
		SetTouch(&CBalrog11Cannon::BuckBcsTouch);
		m_vecOrigin = pev->origin;
		UTIL_SetOrigin(pev, pev->origin);
		UTIL_SetSize(pev, g_vecZero, g_vecZero);
		pev->nextthink = gpGlobals->time + 0.1s;
		pev->classname = MAKE_STRING("balrog11");

		m_flNumFrames = (float)(MODEL_FRAMES(pev->modelindex) - 1);
		pev->framerate = RANDOM_FLOAT(1.0, 30.0);

		MESSAGE_BEGIN(MSG_ALL, gmsgMPToCL, NULL);
		WRITE_BYTE(2);
		WRITE_COORD(m_vecOrigin.x);
		WRITE_COORD(m_vecOrigin.y);
		WRITE_COORD(m_vecOrigin.z);
		WRITE_SHORT(this->entindex());
		WRITE_BYTE(iType);
		MESSAGE_END();

		//PLAYBACK_EVENT_FULL(FEV_GLOBAL, this->edict(), PRECACHE_EVENT(1, "events/wpneffects.sc"), 0.0, m_vecOrigin, g_vecZero, 0.0, 0.0, 7, iType, FALSE, FALSE);
	}

	void CBalrog11Cannon::Init(Vector vecDir, float flSpeed, duration_t tTimeRemove, float flDamage, CUtlVector<CBaseEntity*>* pList, CUtlVector<CBaseEntity*>* pGroupList)
	{
		m_vecDir = vecDir;
		m_flSpeed = flSpeed;
		m_tTimeRemove = tTimeRemove + gpGlobals->time;
		m_vecVelocity = vecDir * m_flSpeed;
		m_flDamage = flDamage;
		pev->velocity = m_vecVelocity;
		m_pEnemyList = pList;
		m_pGroupList = pGroupList;
		m_pGroupList->AddToTail(this);
		pev->avelocity.z = (float)RANDOM_LONG(5, 50);
		if (RANDOM_LONG(0, 1))
			pev->avelocity.z *= -1;

	}

	void CBalrog11Cannon::Precache()
	{
		PRECACHE_MODEL("sprites/flame_puff01.spr");
		PRECACHE_MODEL("sprites/ef_hwater.spr");		//balrog11b
	}

	void CBalrog11Cannon::AnimationThink()
	{
		pev->scale = 0.8;

		pev->frame += 1.5;

		if (pev->frame > m_flNumFrames)
			pev->frame = fmod(pev->frame, m_flNumFrames);

		pev->nextthink = gpGlobals->time + 0.02s;

		// CalcVelocity(0.02);
		m_vecOrigin += m_vecVelocity * 0.02;

		UTIL_SetOrigin(pev, m_vecOrigin);

		if (gpGlobals->time >= m_tTimeRemove)
		{
			m_pGroupList->FindAndRemove(this);

			if (!m_pGroupList->Count())
			{
				delete m_pEnemyList;
				delete m_pGroupList;
			}

			UTIL_Remove(this);

			SetThink(NULL);
			SetTouch(NULL);
		}
	}

	void CBalrog11Cannon::Remove()
	{
		SetThink(nullptr);
		SetTouch(nullptr);
		pev->effects |= EF_NODRAW; // 0x80u

		return UTIL_Remove(this);
	}

	void CBalrog11Cannon::BuckBcsTouch(CBaseEntity* pOther)
	{
		if (pOther->edict() == pev->owner)
			return;

		if (m_pEnemyList->HasElement(pOther))
			return;

		CBaseEntity* pOwner = CBaseEntity::Instance(pev->owner);
		CBasePlayer* pEntityOwner = nullptr;
		if (pOwner && pOwner->IsPlayer())
			pEntityOwner = static_cast<CBasePlayer*>(pOwner);

		if (pEntityOwner)
		{
			if (!pEntityOwner->m_bIsZombie)
			{
				if (!pOther->IsDormant())
				{
					if (pOther->pev->takedamage != DAMAGE_NO)
					{
						pOther->TakeDamage(pev, pOwner->pev, m_flDamage, DMG_GENERIC);
						m_pEnemyList->AddToTail(pOther);
					}
				}
			}
		}

		if (pOther->IsBSPModel())
		{
			m_pGroupList->FindAndRemove(this);

			if (!m_pGroupList->Count())
			{
				delete m_pEnemyList;
				delete m_pGroupList;
			}

			UTIL_Remove(this);

			SetThink(NULL);
			SetTouch(NULL);
		}
	}
}
