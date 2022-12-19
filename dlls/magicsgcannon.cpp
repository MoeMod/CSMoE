#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

#include "weapons/KnifeAttack.h"
#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif

namespace sv {
	LINK_ENTITY_TO_CLASS(magicsg_cannon, CMagicSgCannon)

		void CMagicSgCannon::Spawn()
	{
		Precache();

		CBaseEntity* pOwner = CBaseEntity::Instance(pev->owner);
		CBasePlayer* pEntityOwner = nullptr;
		if (pOwner && pOwner->IsPlayer())
			pEntityOwner = static_cast<CBasePlayer*>(pOwner);

		pev->classname = MAKE_STRING("magicsg_cannon");
		SET_MODEL(this->edict(), "models/w_usp.mdl");


		pev->frame = 0.0;
		//pev->effects |= EF_NODRAW;
		pev->solid = SOLID_TRIGGER; // 2	
		pev->rendermode = kRenderTransAdd; // 5;
		pev->renderfx = kRenderFxNone;
		pev->renderamt = 0.0;
		pev->movetype = MOVETYPE_FLYMISSILE; // 9
		pev->angles.z = (float)RANDOM_LONG(5, 180);

		SetThink(&CBalrog11Cannon::AnimationThink);
		SetTouch(&CMagicSgCannon::MagicSgBcsTouch);
		m_vecOrigin = pev->origin;
		UTIL_SetOrigin(pev, pev->origin);
		UTIL_SetSize(pev, g_vecZero, g_vecZero);
		pev->nextthink = gpGlobals->time + 0.1s;


		m_flNumFrames = (float)(MODEL_FRAMES(pev->modelindex) - 1);
		pev->framerate = RANDOM_FLOAT(1.0, 30.0);
	}

	void CMagicSgCannon::Init(Vector vecDir, float flSpeed, duration_t tTimeRemove, float flDamage, CUtlVector<CBaseEntity*>* pList, CUtlVector<CBaseEntity*>* pGroupList, float flExplodeDamage, float flExplodeRadius, float flDirectForce, int iType, int iBuffType)
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
		m_flExplodeRadius = flExplodeRadius;
		m_flExplodeDamage = flExplodeDamage;
		m_flDirectForce = flDirectForce;

		m_iColor = iType;
		m_iBuffType = iBuffType;

		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_BEAMPARTICLE); //defined as 32
		WRITE_SHORT(this->entindex());  // short (entity:attachment to follow)
		WRITE_SHORT(m_iSprIndex[7]);// short (sprite index)
		WRITE_SHORT(m_iSprIndex[7]);// short (sprite index)
		WRITE_SHORT(m_iSprIndex[7]);// short (sprite index)
		WRITE_BYTE(255); //r
		WRITE_BYTE(255); //g
		WRITE_BYTE(255); //b
		WRITE_BYTE(1); //randomcolor
		WRITE_BYTE(1); //scale in 0.01's
		WRITE_BYTE(5); //freq(time of one cycle) in 0.01's
		WRITE_BYTE(5); //fadetime in 0.1s
		WRITE_BYTE(3);  //count
		WRITE_BYTE(5);  //random in org
		WRITE_BYTE(10);  //speed (if>0 do slow gravity)
		MESSAGE_END();

	}

	void CMagicSgCannon::Precache()
	{

		m_iSprIndex[0] = PRECACHE_MODEL("sprites/ef_magicsg_hit.spr");
		m_iSprIndex[1] = PRECACHE_MODEL("sprites/ef_magicsg_hit2.spr");
		m_iSprIndex[2] = PRECACHE_MODEL("sprites/ef_magicsg_hit_blue.spr");
		m_iSprIndex[3] = PRECACHE_MODEL("sprites/ef_magicsg_hit_fink.spr");
		m_iSprIndex[4] = PRECACHE_MODEL("sprites/ef_magicsg_hit_green.spr");
		m_iSprIndex[5] = PRECACHE_MODEL("sprites/ef_magicsg_hit_red.spr");
		m_iSprIndex[6] = PRECACHE_MODEL("sprites/ef_magicsg_hit_yellow.spr");
		m_iSprIndex[7] = PRECACHE_MODEL("sprites/ef_magicsg_star.spr");

		PRECACHE_MODEL("sprites/skillban_head.spr");
		PRECACHE_MODEL("sprites/zbt_stone.spr");

		PRECACHE_SOUND("weapons/magicsg_shoot2_exp.wav");
	}

	void CMagicSgCannon::Remove()
	{
		SetThink(nullptr);
		SetTouch(nullptr);
		pev->effects |= EF_NODRAW; // 0x80u

		return UTIL_Remove(this);
	}

	void CMagicSgCannon::MagicSgBcsTouch(CBaseEntity* pOther)
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

							if (m_iBuffType == 4)
								tr.iHitgroup = HITGROUP_HEAD;

							pOther->TraceAttack(pevOwner, m_flDamage, pev->velocity, &tr, DMG_BULLET | DMG_NEVERGIB);

							ApplyMultiDamage(pev, pOwner->pev);

							if (m_iBuffType >= -1)
							{
								//MAGICSG_GREEN
								if (pOther->IsPlayer() && ((CBasePlayer*)pOther)->m_bIsZombie)
								{
									UTIL_AddKickRateBoost(pOther, (pOther->pev->origin.Make2D() - pOwner->pev->origin.Make2D()).Normalize(), 1.0, m_flDirectForce, 0.0);
								}
							}
							if (m_iBuffType > 0)
							{
								//MAGICSG_BUFF_FORCESTOP
								if (pOther->IsPlayer())
								{
									CBasePlayer* pPlayer = (CBasePlayer*)pOther;
									if (pPlayer->m_bIsZombie && !(pPlayer->m_iBuff & BUFF_IGNORE_KNOCKBACK))
									{
										pPlayer->m_flVelocityModifier = 0.9;

										auto pStrategy = dynamic_cast<CPlayerModStrategy_ZB1*>(pPlayer->m_pModStrategy.get());
										if (pStrategy)
										{
											pStrategy->SetStunSpeedTime(3.0s);
											pStrategy->SetStunGravityTime(3.0s);
										}
									}
								}

								//MAGICSG_BUFF_FORCESTOP ->DRAW SPR
								if (m_iBuffType == 1)
								{
									if (pOther->IsPlayer() && ((CBasePlayer*)pOther)->m_bIsZombie)
									{
										MESSAGE_BEGIN(MSG_ALL, gmsgMPToCL, NULL, pOther->pev);
										WRITE_BYTE(4);
										WRITE_SHORT(pOther->entindex());
										WRITE_BYTE(22);
										MESSAGE_END();
									}
								}
							}
							if (m_iBuffType >= 2)
							{
								//MAGICSG_BUFF_BANSKILLS
								if (pOther->IsPlayer() && ((CBasePlayer*)pOther)->m_bIsZombie)
								{
									MESSAGE_BEGIN(MSG_ALL, gmsgMPToCL, NULL, pOther->pev);
									WRITE_BYTE(4);
									WRITE_SHORT(pOther->entindex());
									WRITE_BYTE(21);
									MESSAGE_END();
								}

								CBasePlayer* pPlayer = (CBasePlayer*)pOther;
								if (pPlayer->m_bIsZombie)
								{
									auto pStrategy = dynamic_cast<CPlayerModStrategy_ZB1*>(pPlayer->m_pModStrategy.get());
									if (pStrategy)
									{
										pStrategy->SetBanSkillsTime(5.0s);
									}
								}
							}
							if (m_iBuffType >= 3)
							{
								float fMaxHealth, fHealth;

								fMaxHealth = pOwner->pev->max_health;
								fHealth = pOwner->pev->health;

								fHealth = min(fHealth + fMaxHealth * 0.1, fMaxHealth);
								pOwner->pev->health = fHealth;
							}

							m_pEnemyList->AddToTail(pOther);
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


	void CMagicSgCannon::Explode(void)
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

		if (m_iColor == 1)
		{
			MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
			WRITE_BYTE(TE_EXPLOSION);
			WRITE_COORD(vecEffectPos.x);
			WRITE_COORD(vecEffectPos.y);
			WRITE_COORD(vecEffectPos.z);
			WRITE_SHORT(m_iSprIndex[1]);
			WRITE_BYTE(5);
			WRITE_BYTE(20);
			WRITE_BYTE(TE_EXPLFLAG_NODLIGHTS | TE_EXPLFLAG_NOSOUND | TE_EXPLFLAG_NOPARTICLES);
			MESSAGE_END();
		}
		MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
		WRITE_BYTE(TE_EXPLOSION);
		WRITE_COORD(vecEffectPos.x);
		WRITE_COORD(vecEffectPos.y);
		WRITE_COORD(vecEffectPos.z);
		WRITE_SHORT(m_iSprIndex[0]);
		WRITE_BYTE(3);
		WRITE_BYTE(20);
		WRITE_BYTE(TE_EXPLFLAG_NODLIGHTS | TE_EXPLFLAG_NOSOUND | TE_EXPLFLAG_NOPARTICLES);
		MESSAGE_END();

		EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "weapons/magicsg_shoot2_exp.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		PLAYBACK_EVENT_FULL(FEV_GLOBAL, ENT(this->pev), PRECACHE_EVENT(1, "events/wpneffects.sc"), 0.1, this->pev->origin, g_vecZero, m_iColor, 2.0, 27, 0, TRUE, TRUE);

		CBaseEntity* pEntity = NULL;

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
