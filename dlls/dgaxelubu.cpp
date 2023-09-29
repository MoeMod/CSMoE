#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "weapons/RadiusDamage.hpp"
#include "gamemode/mods.h"
#include "gamemode/mod_zb1.h"

namespace sv {
	LINK_ENTITY_TO_CLASS(knifedgaxe_lubu, CKnifeDGAXE_LuBu);

	CKnifeDGAXE_LuBu* CKnifeDGAXE_LuBu::Create(int iType, const Vector& vecOrigin, const Vector& vecAngles, edict_t* pentOwner, CBasePlayer* pOwner, int iTeam, int iSkill)
	{
		edict_t* pent = CREATE_NAMED_ENTITY(MAKE_STRING("knifedgaxe_lubu"));

		if (FNullEnt(pent))
		{
			ALERT(at_console, "NULL Ent in Create!\n");
			return NULL;
		}

		CKnifeDGAXE_LuBu* pLuBu = (CKnifeDGAXE_LuBu*)Instance(pent);

		if (pLuBu)
		{
			pLuBu->m_iType = iType;
			pLuBu->pev->owner = pentOwner;
			pLuBu->pev->origin = vecOrigin;
			pLuBu->pev->angles = vecAngles;
			pLuBu->m_iTeam = iTeam;
			pLuBu->m_iSkill = iSkill;
			pLuBu->m_pOwner = pOwner;
			pLuBu->pev->iuser1 = iType;
			pLuBu->Spawn();
		}

		return pLuBu;
	}

	void CKnifeDGAXE_LuBu::Spawn()
	{
		Precache();

		if (m_iType)
			SET_MODEL(ENT(pev), "models/dgaxeex_summon.mdl");
		else
			SET_MODEL(ENT(pev), "models/dgaxe_summon.mdl");

		UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));

		pev->solid = SOLID_NOT;
		pev->movetype = MOVETYPE_NONE;
		pev->rendermode = kRenderTransTexture;
		pev->renderamt = 230.0;
		SetThink(&CKnifeDGAXE_LuBu::LuBuThink);
	
		pev->frame = 0;

		if (m_iSkill)
		{
			pev->sequence = 2;

			EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/dgaxe_skill2.wav", VOL_NORM, ATTN_NORM);
		}
			
		else
		{
			DROP_TO_FLOOR(edict());

			pev->sequence = 0;

			EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/dgaxe_skill1.wav", VOL_NORM, ATTN_NORM);
		}
			

		ResetSequenceInfo();

		m_flDamageTime = gpGlobals->time + 0.25s;;
		m_flLoopTime = invalid_time_point;

		pev->nextthink = gpGlobals->time + 0.37s;

		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_TEMPSPRITE);
		WRITE_COORD(pev->origin.x);
		WRITE_COORD(pev->origin.y);
		WRITE_COORD(pev->origin.z - 16.0);
		WRITE_SHORT(m_iSmoke);
		WRITE_BYTE(10);
		WRITE_BYTE(255);
		WRITE_BYTE(30);
		MESSAGE_END();
	}

	void CKnifeDGAXE_LuBu::Precache()
	{
		m_iExplo[0] = PRECACHE_MODEL("sprites/ef_dgaxe_explo.spr");
		m_iSmoke = PRECACHE_MODEL("sprites/ef_dgaxe_smoke.spr");
		PRECACHE_MODEL("models/dgaxe_summon.mdl");
		PRECACHE_MODEL("models/dgaxeex_summon.mdl");
		PRECACHE_MODEL("sprites/ef_dgaxeex_shockwave.spr");
		m_iExplo[1] = PRECACHE_MODEL("sprites/ef_dgaxeex_explo.spr");
		m_iShockWaveIndex = PRECACHE_MODEL("sprites/ef_dgaxeex_shockwave.spr");

		PRECACHE_SOUND("weapons/dgaxe_skill1_exp.wav");
		PRECACHE_SOUND("weapons/dgaxe_skill2_exp.wav");


		PRECACHE_SOUND("weapons/dgaxeex_skill1_exp.wav");
		PRECACHE_SOUND("weapons/dgaxeex_skill2_exp.wav");
		//unknown
		//PRECACHE_SOUND("weapons/dgaxeex_fx_start.wav");
		//PRECACHE_SOUND("weapons/dgaxeex_fx_loop.wav");
		//PRECACHE_SOUND("weapons/dgaxeex_fx_end.wav");
	}

	void CKnifeDGAXE_LuBu::FadeOut()
	{
		if (pev->renderamt > 50) {
			pev->renderamt -= 60.0f;
			pev->nextthink = gpGlobals->time + 0.1s;
		}
		else {
			pev->renderamt = 0.0f;
			pev->nextthink = gpGlobals->time + 0.2s;
			SetThink(&CBaseEntity::SUB_Remove);
		}
	}

	void CKnifeDGAXE_LuBu::LuBuThink()
	{
		pev->nextthink = gpGlobals->time + 0.1s;

		if (m_iSkill)
		{
			if (m_flLoopTime != invalid_time_point && gpGlobals->time > m_flLoopTime)
			{
				pev->frame = 0;
				pev->sequence = 4;

				ResetSequenceInfo();

				m_flDamageTime = invalid_time_point;

				SetThink(&CKnifeDGAXE_LuBu::FadeOut);
			}

			if (m_flDamageTime != invalid_time_point && gpGlobals->time > m_flDamageTime)
			{
				RadiusDamage();
				m_flDamageTime = gpGlobals->time + 0.25s;
			}

			if (pev->sequence == 2)
			{
				m_flLoopTime = gpGlobals->time + 5.0s;

				pev->frame = 0;
				pev->sequence = 3;

				ResetSequenceInfo();

				if (m_iType)
					EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/dgaxeex_skill2_exp.wav", VOL_NORM, ATTN_NORM);
				else
					EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/dgaxe_skill2_exp.wav", VOL_NORM, ATTN_NORM);
			}	
		}
		else
		{
			if (pev->sequence == 0)
			{
				pev->frame = 0;
				pev->sequence = 1;

				ResetSequenceInfo();

				if(m_iType)
					EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/dgaxeex_skill1_exp.wav", VOL_NORM, ATTN_NORM);
				else
					EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/dgaxe_skill1_exp.wav", VOL_NORM, ATTN_NORM);
			}

			if (m_flDamageTime != invalid_time_point && gpGlobals->time > m_flDamageTime)
			{
				RadiusDamage();
				m_flDamageTime = invalid_time_point;

				if (m_iType)
				{
					Vector angle = UTIL_VecToAngles({ 0, 0, 1 });

					UTIL_TempModel(pev->origin, angle, g_vecZero, m_iShockWaveIndex, 20, 0, 255, 0, 0, 255, kRenderNormal, NULL, false, 0, 15, 256, 0);

					UTIL_TempModel(pev->origin, g_vecZero, g_vecZero, m_iExplo[m_iType], 20, 0, 255, 0, 0, 255, kRenderNormal, NULL, false, 0, 15, 256, 0);
				}
				else
				{
					MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
					WRITE_BYTE(TE_TEMPSPRITE);
					WRITE_COORD(pev->origin.x);
					WRITE_COORD(pev->origin.y);
					WRITE_COORD(pev->origin.z);
					WRITE_SHORT(m_iExplo[m_iType]);
					WRITE_BYTE(15);
					WRITE_BYTE(255);
					WRITE_BYTE(30);
					MESSAGE_END();

				}
	
				pev->nextthink = gpGlobals->time + 1.0s;

				SetThink(&CKnifeDGAXE_LuBu::FadeOut);
			}
		}
		
	}

	float CKnifeDGAXE_LuBu::GetDamage()
	{
		float flDamage;

		if (m_iType)
		{
			flDamage = m_iSkill ? 4.0f : 100.0f;
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = m_iSkill ? 837.0f : 16170.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = m_iSkill ? 1320.0f : 22467.0f;
		}
		else
		{
			flDamage = m_iSkill ? 4.0f : 100.0f;
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = m_iSkill ? 761.0f : 14700.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = m_iSkill ? 1200.0f : 20425.0f;
		}

		return flDamage;
	}

	float CKnifeDGAXE_LuBu::GetRadius()
	{
		float flRadius = 4.0f;
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flRadius = 4.0f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flRadius = 5.0f;
		return flRadius * 39.37;
	}

	void CKnifeDGAXE_LuBu::RadiusDamage()
	{
		if (!m_pOwner)
			return;

		if (m_pOwner->m_bIsZombie)
			return;

		entvars_t* pevOwner = VARS(pev->owner);

		CBaseEntity* pEntity = NULL;
		TraceResult tr;
		Vector vecOrigin = pev->origin;

		Vector vecEnd;
		int bInWater = (UTIL_PointContents(pev->origin) == CONTENTS_WATER);

		while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecOrigin, GetRadius())) != NULL)
		{
			if (pEntity->edict() == pev->owner)
				continue;

			if (pEntity->IsDormant())
				continue;

			if (pEntity->pev->takedamage == DAMAGE_NO)
				continue;

			if (g_pGameRules->PlayerRelationship(m_pOwner, pEntity) == GR_TEAMMATE)
				continue;

			vecEnd = pEntity->BodyTarget(pev->origin);

			UTIL_TraceLine(pev->origin, vecEnd, missile, pev->owner, &tr);

			if (tr.flFraction < 1.0 && tr.pHit != pEntity->edict())
				continue;


			Vector vecDelta = pEntity->pev->origin - vecOrigin;


			pEntity->m_LastHitGroup = HITGROUP_CHEST;
			pEntity->TakeDamage(pev, pevOwner, GetDamage(), DMG_BULLET | DMG_NEVERGIB);

			if (pEntity->IsPlayer())
			{
				CBasePlayer* player = (CBasePlayer*)pEntity;

				if (!player)
					continue;
		
				if (player->m_bIsZombie && ~(player->m_iBuff) & BUFF_IGNORE_KNOCKBACK)
				{
					auto strategy = dynamic_cast<CPlayerModStrategy_ZB1*>(player->m_pModStrategy.get());
					if (strategy)
					{
						if (m_iSkill)
						{
							strategy->SetStunSpeedTime(1.0s, 150.0f);	//150
							strategy->SetStunGravityTime(1.0s);
						}
						else
						{
							strategy->SetStunSpeedTime(2.0s, 60.0f);	//60
							strategy->SetStunGravityTime(2.0s);
						}

					}
				}
			}
		}

	}

	
}
