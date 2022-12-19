#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif

namespace sv {
	LINK_ENTITY_TO_CLASS(sgmissile_cannon, CSGMissileCannon)

	void CSGMissileCannon::Spawn()
	{
		Precache();
		
		CBaseEntity* pOwner = CBaseEntity::Instance(pev->owner);
		CBasePlayer* pEntityOwner = nullptr;
		if (pOwner && pOwner->IsPlayer())
			pEntityOwner = static_cast<CBasePlayer*>(pOwner);
		
		int iType = 2;
		pev->classname = MAKE_STRING("sgmissile");
		SET_MODEL(this->edict(), "sprites/ef_sgmissile_line.spr");
		CBasePlayerWeapon* pWeapon = (CBasePlayerWeapon*)pEntityOwner->m_rgpPlayerItems[PRIMARY_WEAPON_SLOT];
		if (pWeapon != NULL)
		{
			if (!Q_strcmp(STRING(pWeapon->pev->classname), "weapon_sgmissilem"))
			{
				SET_MODEL(this->edict(), "sprites/ef_sgmissilem_line.spr");
				pev->classname = MAKE_STRING("sgmissilem");
				iType = 3;
			}
			else if(!Q_strcmp(STRING(pWeapon->pev->classname), "weapon_sgmissileex"))
			{
				SET_MODEL(this->edict(), "sprites/ef_sgmissileex_line.spr");
				pev->classname = MAKE_STRING("sgmissileex");
				iType = 4;
			}
		}
			

		pev->frame = 0.0;
		pev->effects = 0;
		pev->skin = 0;
		pev->body = 0;
		pev->solid = SOLID_TRIGGER; // 2	
		pev->rendermode = kRenderTransAdd; // 5;
		pev->renderfx = kRenderFxNone;
		pev->renderamt = 90.0;
		pev->movetype = MOVETYPE_FLYMISSILE; // 9
		pev->angles.z = (float)RANDOM_LONG(5, 180);

		SetThink(&CBalrog11Cannon::AnimationThink);
		SetTouch(&CSGMissileCannon::SGMissileBcsTouch);
		m_vecOrigin = pev->origin;
		UTIL_SetOrigin(pev, pev->origin);
		UTIL_SetSize(pev, g_vecZero, g_vecZero);
		pev->nextthink = gpGlobals->time + 0.1s;
		

		m_flNumFrames = (float)(MODEL_FRAMES(pev->modelindex) - 1);
		pev->framerate = RANDOM_FLOAT(1.0, 30.0);

		/*MESSAGE_BEGIN(MSG_ALL, gmsgMPToCL, NULL);
		WRITE_BYTE(2);
		WRITE_COORD(m_vecOrigin.x);
		WRITE_COORD(m_vecOrigin.y);
		WRITE_COORD(m_vecOrigin.z);
		WRITE_SHORT(this->entindex());
		WRITE_BYTE(iType);
		MESSAGE_END();*/

		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_BEAMPARTICLE); //defined as 32
		WRITE_SHORT(this->entindex());  // short (entity:attachment to follow)
		WRITE_SHORT(m_iSprIndex[iType - 2]);// short (sprite index)
		WRITE_SHORT(m_iSprIndex[iType - 2]);// short (sprite index)
		WRITE_SHORT(m_iSprIndex[iType - 2]);// short (sprite index)
		WRITE_BYTE(255); //r
		WRITE_BYTE(255); //g
		WRITE_BYTE(255); //b
		WRITE_BYTE(0); //randomcolor
		WRITE_BYTE(100); //scale in 0.01's
		WRITE_BYTE(6); //freq(time of one cycle) in 0.01's
		WRITE_BYTE(3); //fadetime in 0.1s
		WRITE_BYTE(1);  //count
		WRITE_BYTE(0);  //random in org
		WRITE_BYTE(0);  //speed (if>0 do slow gravity)
		MESSAGE_END();
	}

	void CSGMissileCannon::Init(Vector vecDir, float flSpeed, duration_t tTimeRemove, float flDamage, CUtlVector<CBaseEntity*>* pList, CUtlVector<CBaseEntity*>* pGroupList, float flExplodeDamage, float flExplodeRadius, float flDirectForce, int iType)
	{
		switch (iType)
		{
		case 3:
			m_iEffectSprite = PRECACHE_MODEL("sprites/muzzleflash115.spr"); break;
		case 4:
			m_iEffectSprite = PRECACHE_MODEL("sprites/ef_sgmissileex.spr"); break;
		}
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
		m_flExplodeRadius = flExplodeRadius;
		m_flExplodeDamage = flExplodeDamage;
		m_flDirectForce = flDirectForce;
	}

	void CSGMissileCannon::Precache()
	{
		m_iSprIndex[0] = PRECACHE_MODEL("sprites/ef_sgmissile_line.spr");
		m_iSprIndex[1] = PRECACHE_MODEL("sprites/ef_sgmissilem_line.spr");
		m_iSprIndex[2] = PRECACHE_MODEL("sprites/ef_sgmissileex_line.spr");
		PRECACHE_MODEL("sprites/ef_sgmissileex.spr");
		m_iEffectSprite = PRECACHE_MODEL("sprites/ef_sgmissile.spr");
		m_iExplosionSprite = PRECACHE_MODEL("sprites/dart_explosion.spr");

		PRECACHE_SOUND("weapons/sgmissile_exp.wav");
	}

	void CSGMissileCannon::Remove()
	{
		SetThink(nullptr);
		SetTouch(nullptr);
		pev->effects |= EF_NODRAW; // 0x80u

		return UTIL_Remove(this);
	}

	void CSGMissileCannon::SGMissileBcsTouch(CBaseEntity* pOther)
	{
		if (ENT(pOther->pev) == pev->owner)
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
						TraceResult tr;
						Vector vecStart = pev->origin;
						Vector vecEnd = pev->origin + pev->velocity.Normalize() * 42;
						UTIL_TraceLine(vecStart, vecEnd, dont_ignore_monsters, pev->owner, &tr);
				
						if (g_pGameRules->PlayerRelationship(pEntityOwner, pOther) != GR_TEAMMATE)
						{
							entvars_t* pevOwner = VARS(pev->owner);

							ClearMultiDamage();

							pOther->TraceAttack(pevOwner, m_flDamage, pev->velocity, &tr, DMG_BULLET | DMG_NEVERGIB);

							ApplyMultiDamage(pev, pOwner->pev);

							if (pOther->IsPlayer() && ((CBasePlayer*)pOther)->m_bIsZombie)
							{
								UTIL_AddKickRateBoost(pOther, (pOther->pev->origin.Make2D() - pOwner->pev->origin.Make2D()).Normalize(), 1.0, m_flDirectForce, 0.0);
							}

							m_pEnemyList->AddToTail(pOther);
							Explode();
						}
					}
				}
			}
		}

		if (pOther->IsBSPModel())
		{
			Explode();

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

	
	void CSGMissileCannon::Explode(void)
	{
		CBaseEntity* pOwner = CBaseEntity::Instance(pev->owner);
		CBasePlayer* pEntityOwner = nullptr;
		if (pOwner && pOwner->IsPlayer())
			pEntityOwner = static_cast<CBasePlayer*>(pOwner);

		if (!pEntityOwner)
			return;

		if (FNullEnt(pEntityOwner->edict()))
			return;

		Vector vecEffectPos = pev->origin - pev->velocity.Normalize() * 10;

		MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
		WRITE_BYTE(TE_EXPLOSION);
		WRITE_COORD(vecEffectPos.x);
		WRITE_COORD(vecEffectPos.y);
		WRITE_COORD(vecEffectPos.z);
		WRITE_SHORT(m_iEffectSprite);
		WRITE_BYTE(5);
		WRITE_BYTE(15);
		WRITE_BYTE(TE_EXPLFLAG_NODLIGHTS | TE_EXPLFLAG_NOSOUND | TE_EXPLFLAG_NOPARTICLES);
		MESSAGE_END();

		MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
		WRITE_BYTE(TE_EXPLOSION);
		WRITE_COORD(vecEffectPos.x);
		WRITE_COORD(vecEffectPos.y);
		WRITE_COORD(vecEffectPos.z);
		WRITE_SHORT(m_iExplosionSprite);
		WRITE_BYTE(10);
		WRITE_BYTE(15);
		WRITE_BYTE(TE_EXPLFLAG_NODLIGHTS | TE_EXPLFLAG_NOSOUND | TE_EXPLFLAG_NOPARTICLES);
		MESSAGE_END();

		EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "weapons/sgmissile_exp.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

		CBaseEntity *pEntity = NULL;

		while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, m_flExplodeRadius)) != NULL)
		{
			if (pEntityOwner->m_bIsZombie)
				break;

			if (pEntity->pev->takedamage == DAMAGE_NO)
				continue;

			if (UTIL_PointContents(pev->origin) == CONTENTS_WATER)
				continue;

			if (pEntity->pev == pEntityOwner->pev)
				continue;

			if (g_pGameRules->PlayerRelationship(pEntityOwner, pEntity) == GR_TEAMMATE)
				continue;

			float flDamage = UTIL_CalcDamage(pev->origin, pEntity->pev->origin, m_flExplodeDamage, m_flExplodeRadius, NULL);

			pEntity->TakeDamage(pev, pEntityOwner->pev, flDamage, DMG_EXPLOSION);
		}
	}
}
