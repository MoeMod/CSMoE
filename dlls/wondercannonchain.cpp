#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif

namespace sv {
#define WONDERCANNON_CHAIN_DAMAGE_INTERVAL	0.3s
#define WONDERCANNON_CHAIN_DISTANCE 39.37 * 3.5
#define WONDERCANNON_TOTAL_EXP_COUNT	12
#define WONDERCANNON_CSO_DAMAGEDATA	0
#define WONDERCANNONEX_TOTAL_EXP_COUNT	8

	LINK_ENTITY_TO_CLASS(wondercannon_chain, CWonderCannonChain)

	CWonderCannonChain* CWonderCannonChain::Create(int iType, const Vector& vecOrigin, const Vector& vecAngles, edict_t* pentOwner)
	{
		edict_t* pent = CREATE_NAMED_ENTITY(MAKE_STRING("wondercannon_chain"));

		if (FNullEnt(pent))
		{
			ALERT(at_console, "NULL Ent in Create!\n");
			return NULL;
		}

		CWonderCannonChain* pChain = (CWonderCannonChain*)Instance(pent);

		if (pChain)
		{
			pChain->m_iType = iType;
			pChain->pev->owner = pentOwner;
			pChain->pev->origin = vecOrigin;
			pChain->pev->angles = vecAngles;
			pChain->Spawn();
		}

		return pChain;
	}

	void CWonderCannonChain::Spawn()
	{
		Precache();
	
		pev->solid = SOLID_NOT; // 2	
	
		pev->movetype = MOVETYPE_NOCLIP; // 9

		SetThink(&CWonderCannonChain::OnThink);
		
		m_iTotalExpCount = m_iType ? WONDERCANNONEX_TOTAL_EXP_COUNT : WONDERCANNON_TOTAL_EXP_COUNT;
		m_flNextDamage = gpGlobals->time + 0.1s;
		pev->nextthink = gpGlobals->time + 0.1s;
		m_tTimeRemove = gpGlobals->time + 5.0s;
	}

	void CWonderCannonChain::Init(CBasePlayer* pOwner, CWonderCannon* pWeapon, CBaseEntity* pAttachedEnt, CUtlVector<CBaseEntity*>* pEnemyList)
	{
		m_pAttachedEnt = pAttachedEnt;
		m_pWeapon = pWeapon;
		m_pOwner = pOwner;
		m_iTeam = m_pOwner->m_iTeam;
		m_pEnemyList = pEnemyList;
		if (m_pEnemyList->IsEmpty())
			m_pEnemyList->AddToTail(m_pAttachedEnt);

		if (g_pGameRules->PlayerRelationship(m_pOwner, m_pAttachedEnt) == GR_TEAMMATE)
			m_flNextDamage = invalid_time_point;
	}

	void CWonderCannonChain::Precache()
	{
		for (int i = 0; i < 4; i++)
		{
			char SzSpr[4][64]{};
			if(m_iType)
				sprintf(SzSpr[i], "sprites/ef_wondercannonex_hit%d.spr", i + 1);
			else
				sprintf(SzSpr[i], "sprites/ef_wondercannon_hit%d.spr", i + 1);

			m_iCache_Exp[i] = PRECACHE_MODEL(SzSpr[i]);
		}
		PRECACHE_MODEL("sprites/ef_wondercannon_chain.spr");
		PRECACHE_MODEL("sprites/ef_wondercannonex_chain.spr");
		PRECACHE_MODEL("sprites/ef_wondercannonex_light.spr");

		PRECACHE_SOUND("weapons/wondercannonex_bomd_exp.wav");
		PRECACHE_SOUND("weapons/wondercannon_bomd_exp.wav");
		PRECACHE_SOUND("weapons/wondercannon_bomd_exp2.wav");
	}

	void CWonderCannonChain::OnThink()
	{
		pev->nextthink = gpGlobals->time + 0.1s;

		if (!m_pOwner)
		{
			Remove();
			return;
		}

		for (int i = 0; i < m_pEnemyList->Count(); ++i)
		{
			if (!m_pEnemyList->Element(i))
			{
				m_pEnemyList->Remove(i);
				continue;
			}
				
			if (!m_pEnemyList->Element(i)->IsAlive())
			{
				m_pEnemyList->Remove(i);
				continue;
			}			
		}

		if (m_pOwner->m_bIsZombie || !m_pOwner->IsAlive())
		{
			Remove();
			return;
		}

		CBasePlayerWeapon* pWeapon = (CBasePlayerWeapon*)m_pOwner->m_rgpPlayerItems[PRIMARY_WEAPON_SLOT];

		if (!pWeapon)
		{
			Remove();
			return;
		}

		if (pWeapon->m_iId != GetWeaponsId())
		{
			Remove();
			return;
		}	
		
		if (!m_pEnemyList->Count())
		{
			Remove();
			return;
		}

		CBaseEntity* pNewEntity = NULL;
		for (int i = 0; i < m_pEnemyList->Count(); ++i)
		{
			while ((pNewEntity = UTIL_FindEntityInSphere(pNewEntity, m_pEnemyList->Element(i)->pev->origin, WONDERCANNON_CHAIN_DISTANCE)) != NULL)
			{
				if (m_pEnemyList->HasElement(pNewEntity))
					continue;

				if (pNewEntity->IsDormant())
					continue;

				if (pNewEntity == m_pAttachedEnt)
					continue;

				if (!pNewEntity->IsPlayer() && pNewEntity->Classify() != CLASS_PLAYER_ALLY)
					continue;

				if (pNewEntity == m_pOwner)
					continue;

				if (g_pGameRules->PlayerRelationship(m_pOwner, pNewEntity) == GR_TEAMMATE)
					continue;

				if(m_pEnemyList->Count() >= m_iTotalExpCount)
					continue;

				if (m_pAttachedEnt->IsPlayer() && !pev->iuser1)
				{
					CBasePlayer* player = dynamic_ent_cast<CBasePlayer*>(m_pAttachedEnt);
					if (g_pGameRules->PlayerRelationship(player, pNewEntity) != GR_TEAMMATE)
					{
						m_pEnemyList->RemoveMultipleFromHead(1);
						m_pEnemyList->AddToTail(pNewEntity);

						MESSAGE_BEGIN(MSG_PVS, gmsgMPToCL, NULL);
						WRITE_BYTE(19);
						WRITE_SHORT(m_pAttachedEnt->entindex());
						WRITE_SHORT(pNewEntity->entindex());
						WRITE_BYTE(0);
						MESSAGE_END();

						m_flNextDamage = gpGlobals->time + 0.1s;
						m_pAttachedEnt = pNewEntity;
						m_tTimeRemove = gpGlobals->time + 5.0s;
						pev->iuser1 = 1;
						continue;
					}
				}

				

				m_pEnemyList->AddToTail(pNewEntity);

				MESSAGE_BEGIN(MSG_PVS, gmsgMPToCL, NULL);
				WRITE_BYTE(19);
				WRITE_SHORT(m_pEnemyList->Element(i)->entindex());
				WRITE_SHORT(pNewEntity->entindex());
				WRITE_BYTE(m_iType ? 1 : 0);
				MESSAGE_END();			
			}
		}
		

		if (m_flNextDamage != invalid_time_point && m_flNextDamage < gpGlobals->time)
		{
			m_flNextDamage = gpGlobals->time + WONDERCANNON_CHAIN_DAMAGE_INTERVAL;

			m_iAttachedEntCount = m_pEnemyList->Count();

			if (m_iType)
			{
				if (m_iAttachedEntCount <= 1)
					m_iExpTime = m_iTotalExpCount - 1;
				else if (m_iAttachedEntCount < 3)
					m_iExpTime = m_iTotalExpCount / m_iAttachedEntCount;
				else if (m_iAttachedEntCount < 4)
					m_iExpTime = 3;	//3 people
				else if (m_iAttachedEntCount < 5)
					m_iExpTime = m_iTotalExpCount / m_iAttachedEntCount;	//4 people
				else if (m_iAttachedEntCount < 8)
					m_iExpTime = floor(double(m_iTotalExpCount) / double(m_iAttachedEntCount)) + 1;
				else if (m_iAttachedEntCount <= m_iTotalExpCount)
					m_iExpTime = 1;
			}
			else
			{
				if (m_iAttachedEntCount <= 1)
					m_iExpTime = m_iTotalExpCount - 1;
				else if (m_iAttachedEntCount < 5)
					m_iExpTime = m_iTotalExpCount / m_iAttachedEntCount;
				else if (m_iAttachedEntCount < 6)
					m_iExpTime = 3;
				else if (m_iAttachedEntCount < 10)
					m_iExpTime = floor(double(m_iTotalExpCount - 2) / double(m_iAttachedEntCount)) + 1;
				else if (m_iAttachedEntCount <= m_iTotalExpCount)
					m_iExpTime = 1;
			}
			
			
			int iMultiDamage = m_iCount * m_iAttachedEntCount;
				
			float flBaseDamage = GetDamage(EXPTYPE_BASE);
			float flFloatDamage = GetDamage(EXPTYPE_SINGLE) + GetDamage(EXPTYPE_MULTI);
			float flFinalDamage = GetDamage(EXPTYPE_FINAL);

			float flDamage = flBaseDamage + iMultiDamage * flFloatDamage;

			if (m_iCount >= m_iExpTime)
				flDamage += flFinalDamage;

			for (int i = 0; i < m_pEnemyList->Count(); ++i)
			{
				DoExpDamage(flDamage, m_pEnemyList->Element(i));

				if (m_iCount < m_iExpTime)
				{
					int iHalfExpTime = floor(double(m_iExpTime) / 2);

					if (m_iType)
					{
						if (m_iCount % 2)
						{
							//2 time is spr 4
							UTIL_TempModel(Vector(0, 0, RANDOM_LONG(-15, 15)), pev->angles, g_vecZero, m_iCache_Exp[3], 30, 0, 200, 0, 0, 195, kRenderTransAdd, m_pEnemyList->Element(i), false, 0, 8, 0, 0);
						}
						else
						{
							UTIL_TempModel(Vector(0, 0, RANDOM_LONG(-15, 15)), pev->angles, g_vecZero, m_iCache_Exp[0], 30, 0, 200, 0, 0, 195, kRenderTransAdd, m_pEnemyList->Element(i), false, 0, 8, 0, 0);
						}

						EMIT_SOUND_DYN(ENT(m_pEnemyList->Element(i)->pev), CHAN_ITEM, "weapons/wondercannon_bomd_exp.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
					}
					else
					{
						if (m_iCount < iHalfExpTime)
						{
							if (m_iCount % 2)
							{
								//2 time is spr 4
								UTIL_TempModel(Vector(0, 0, RANDOM_LONG(-7, 7)), pev->angles, g_vecZero, m_iCache_Exp[3], 30, 0, 200, 0, 0, 195, kRenderTransAdd, m_pEnemyList->Element(i), false, 0, 8, 0, 0);
							}
							else
							{
								UTIL_TempModel(Vector(0, 0, RANDOM_LONG(-7, 7)), pev->angles, g_vecZero, m_iCache_Exp[0], 30, 0, 200, 0, 0, 195, kRenderTransAdd, m_pEnemyList->Element(i), false, 0, 8, 0, 0);
							}
						}
						else
						{
							UTIL_TempModel(Vector(0, 0, -18), pev->angles, g_vecZero, m_iCache_Exp[1], 30, 0, 230, 0, 0, 195, kRenderTransAdd, m_pEnemyList->Element(i), false, 0, 8, 0, 0);
						}

						EMIT_SOUND_DYN(ENT(m_pEnemyList->Element(i)->pev), CHAN_ITEM, "weapons/wondercannon_bomd_exp.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
					}		
				}
				else
				{
					if (m_iType)
					{
						UTIL_TempModel(Vector(0, 0, -18), pev->angles, g_vecZero, m_iCache_Exp[2], 30, 0, 230, 0, 0, 195, kRenderTransAdd, m_pEnemyList->Element(i), false, 0, 8, 0, 0);

						EMIT_SOUND_DYN(ENT(m_pEnemyList->Element(i)->pev), CHAN_ITEM, "weapons/wondercannonex_bmod_exp.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
					}
					
					else
					{
						UTIL_TempModel(Vector(0, 0, -18), pev->angles, g_vecZero, m_iCache_Exp[2], 30, 0, 230, 0, 0, 195, kRenderTransAdd, m_pEnemyList->Element(i), false, 0, 8, 0, 0);

						EMIT_SOUND_DYN(ENT(m_pEnemyList->Element(i)->pev), CHAN_ITEM, "weapons/wondercannon_bomd_exp2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
					}
						
				}
					
			}

			if (m_iCount >= m_iExpTime)
			{
				SetThink(&CWonderCannonChain::Remove);
				pev->nextthink = gpGlobals->time + 0.1s;
				return;
			}

			m_iCount++;
		}

		if (gpGlobals->time >= m_tTimeRemove)
		{
			Remove();
			return;
		}
	}

	void CWonderCannonChain::DoExpDamage(float flDamage, CBaseEntity* pAttachedEnt)
	{
		if (pAttachedEnt && m_pOwner)
		{
			if (pAttachedEnt->pev->takedamage != DAMAGE_NO && pAttachedEnt->IsAlive())
			{
				Vector vecDirection = (pAttachedEnt->pev->origin - m_pOwner->pev->origin).Normalize();

				TraceResult tr;

				UTIL_TraceLine(m_pOwner->pev->origin, pAttachedEnt->pev->origin, dont_ignore_monsters, m_pOwner->edict(), &tr);

				ClearMultiDamage();
				tr.iHitgroup = HITGROUP_CHEST;
				pAttachedEnt->TraceAttack(m_pOwner->pev, flDamage, vecDirection, &tr, DMG_NEVERGIB | DMG_BULLET);
				ApplyMultiDamage(this->pev, m_pOwner->pev);
			}
		}
	}

	void CWonderCannonChain::Remove()
	{
		delete m_pEnemyList;
		
		UTIL_Remove(this);

		SetThink(NULL);
	}

	
	float CWonderCannonChain::GetDamage(DamageType iType) const
	{
		float flDamage = 1.0f;
#if WONDERCANNON_CSO_DAMAGEDATA
		switch (iType)
		{
		case sv::CWonderCannonChain::EXPTYPE_BASE:
			flDamage = 0.35;
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 18;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 38;
			break;
		case sv::CWonderCannonChain::EXPTYPE_SINGLE:
			flDamage = 0.5;
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 37;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 39;
			break;
		case sv::CWonderCannonChain::EXPTYPE_MULTI:
			flDamage = 0.35;
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 21;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 38;
			break;
		case sv::CWonderCannonChain::EXPTYPE_FINAL:
			flDamage = 0.35;
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 87;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 120;
			break;
		default:
			break;
		}
#else
		if (m_iType)
		{
			switch (iType)
			{
			case sv::CWonderCannonChain::EXPTYPE_BASE:
				flDamage = 0.35;
				if (g_pModRunning->DamageTrack() == DT_ZB)
					flDamage = 19;
				else if (g_pModRunning->DamageTrack() == DT_ZBS)
					flDamage = 38;
				break;
			case sv::CWonderCannonChain::EXPTYPE_SINGLE:
				flDamage = 0.5;
				if (g_pModRunning->DamageTrack() == DT_ZB)
					flDamage = 69;
				else if (g_pModRunning->DamageTrack() == DT_ZBS)
					flDamage = 101;
				break;
			case sv::CWonderCannonChain::EXPTYPE_MULTI:
				flDamage = 0.35;
				if (g_pModRunning->DamageTrack() == DT_ZB)
					flDamage = 37;
				else if (g_pModRunning->DamageTrack() == DT_ZBS)
					flDamage = 64;
				break;
			case sv::CWonderCannonChain::EXPTYPE_FINAL:
				flDamage = 0.35;
				if (g_pModRunning->DamageTrack() == DT_ZB)
					flDamage = 99;
				else if (g_pModRunning->DamageTrack() == DT_ZBS)
					flDamage = 165;
				break;
			default:
				break;
			}
		}
		else
		{
			switch (iType)
			{
			case sv::CWonderCannonChain::EXPTYPE_BASE:
				flDamage = 0.35;
				if (g_pModRunning->DamageTrack() == DT_ZB)
					flDamage = 13;
				else if (g_pModRunning->DamageTrack() == DT_ZBS)
					flDamage = 38;
				break;
			case sv::CWonderCannonChain::EXPTYPE_SINGLE:
				flDamage = 0.5;
				if (g_pModRunning->DamageTrack() == DT_ZB)
					flDamage = 24;
				else if (g_pModRunning->DamageTrack() == DT_ZBS)
					flDamage = 39;
				break;
			case sv::CWonderCannonChain::EXPTYPE_MULTI:
				flDamage = 0.35;
				if (g_pModRunning->DamageTrack() == DT_ZB)
					flDamage = 16;
				else if (g_pModRunning->DamageTrack() == DT_ZBS)
					flDamage = 38;
				break;
			case sv::CWonderCannonChain::EXPTYPE_FINAL:
				flDamage = 0.35;
				if (g_pModRunning->DamageTrack() == DT_ZB)
					flDamage = 65;
				else if (g_pModRunning->DamageTrack() == DT_ZBS)
					flDamage = 120;
				break;
			default:
				break;
			}
		}
#endif
		return flDamage;
	}
}
