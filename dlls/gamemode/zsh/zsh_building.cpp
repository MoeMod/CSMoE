
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "schedule.h"
#include "player.h"
#include "monsters.h"
#include "trains.h"

#include "cbase/cbase_hash.h"

#include "zsh_building.h"
#include "gamemode/mods.h"

namespace sv
{
	LINK_ENTITY_TO_CLASS(zsh_building, CZSHBuilding);

	CZSHBuilding::CZSHBuilding()
	{

	}

	void CZSHBuilding::Precache()
	{
		m_iMetalGibs = PRECACHE_MODEL("models/metalplategibs.mdl");
	}

	void CZSHBuilding::Create(CBasePlayer *pPlayer, zsh::ZSHBuildingClass iClass, std::shared_ptr<CBaseBuildingClass> pClass)
	{
		m_hBuilder = pPlayer;
		m_iClass = iClass;
		m_pClass = pClass;
		m_pClass->OnCreate();

		pev->spawnflags |= SF_NORESPAWN;
		DispatchSpawn(edict());

		Vector vecAngle = pPlayer->pev->v_angle;
		UTIL_MakeVectors(vecAngle);
		UTIL_SetOrigin(pev, pPlayer->pev->origin + pPlayer->pev->view_ofs + gpGlobals->v_forward * 150);
	}

	bool CZSHBuilding::Settle()
	{
		if (m_hBuilder && m_hBuilder.IsValid()) {
			auto strategy = dynamic_cast<CPlayerModStrategy_ZSH*>(m_hBuilder->m_pModStrategy.get());
			if (strategy && strategy->m_iRotate % 2)
			{
				Vector mins = m_pClass->GetMins();
				Vector maxs = m_pClass->GetMaxs();
				UTIL_SetSize(pev, Vector(mins.y, mins.x, mins.z), Vector(maxs.y, maxs.x, maxs.z));
			}
			else
			{
				UTIL_SetSize(pev, m_pClass->GetMins(), m_pClass->GetMaxs());
			}
		} else {
			UTIL_SetSize(pev, m_pClass->GetMins(), m_pClass->GetMaxs());
		}

		TraceResult tr;
		TRACE_MONSTER_HULL(edict(), pev->origin, pev->origin, dont_ignore_monsters, edict(), &tr);
		if (tr.fStartSolid || tr.fAllSolid || !tr.fInOpen)
		{
			UTIL_SetSize(pev, g_vecZero, g_vecZero);
			return false;
		}

		Vector to(pev->origin);
		to.z -= 9999.9f;
		UTIL_TraceLine(pev->origin, to, dont_ignore_monsters, dont_ignore_glass, edict(), &tr);

		if(!FNullEnt(tr.pHit))
		{
			UTIL_SetSize(pev, g_vecZero, g_vecZero);
			return false;
		}

		pev->takedamage = DAMAGE_AIM;
		pev->flags |= FL_MONSTER;
		pev->deadflag = DEAD_NO;
		pev->health = 1.0f;
		pev->max_health = m_pClass->GetHealth();
		pev->rendermode = kRenderTransColor;
		pev->renderamt = 100.0f;
		pev->gravity = 1.0f;
		m_iProgress = 0;
		m_iTotalProgress = m_pClass->GetTotalProgress();

		UTIL_TE_Sprite(g_sModelIndexSmokePuff, pev->origin.x + pev->maxs.x, pev->origin.y, pev->origin.z, 35, 70);
		UTIL_TE_Sprite(g_sModelIndexSmokePuff, pev->origin.x + pev->mins.x, pev->origin.y, pev->origin.z, 35, 70);
		UTIL_TE_Sprite(g_sModelIndexSmokePuff, pev->origin.x, pev->origin.y + pev->maxs.y, pev->origin.z, 35, 70);
		UTIL_TE_Sprite(g_sModelIndexSmokePuff, pev->origin.x, pev->origin.y + pev->mins.y, pev->origin.z, 35, 70);

		m_pClass->OnSettle();
		return true;
	}

	void CZSHBuilding::Spawn()
	{
		Precache();
		if (pev->classname)
		{
			RemoveEntityHashValue(pev, STRING(pev->classname), CLASSNAME);
		}

		MAKE_STRING_CLASS("zsh_building", pev);
		AddEntityHashValue(pev, STRING(pev->classname), CLASSNAME);

		pev->movetype = MOVETYPE_PUSHSTEP;
		pev->solid = SOLID_SLIDEBOX;
		pev->takedamage = DAMAGE_NO;
		pev->flags |= FL_MONSTER;
		pev->deadflag = DEAD_RESPAWNABLE;
		pev->rendermode = kRenderTransColor;
		pev->renderamt = 100.0f;
		pev->gravity = 0.0f;
		m_iProgress = m_iTotalProgress = 0;

		pev->sequence = 0;
		pev->frame = 0;
		ResetSequenceInfo();
		SetBoneController(0, 0);
		SetBoneController(1, 0);
		SetBoneController(2, 0);
		SetBoneController(3, 0);
		SetBoneController(4, 0);

		pev->nextthink = gpGlobals->time + 0.1s;
		SetThink(&CZSHBuilding::OnThink);
		SetTouch(&CZSHBuilding::OnTouch);
	}

	void CZSHBuilding::Complete()
	{
		EMIT_SOUND(ENT(pev), CHAN_STATIC, zsh::SOUND_BUILDING_COMPLETE, VOL_NORM, ATTN_NORM);
		pev->renderfx = 0;
		pev->rendercolor = { 255, 255, 255 };
		pev->rendermode = kRenderNormal;
		pev->renderamt = 16.0;
		m_iProgress = -1;
		m_pClass->OnCompleted();
	}

	void CZSHBuilding::TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType)
	{
		if (!pev->takedamage)
			return;

		CBasePlayer* pPlayerAttacker = dynamic_ent_cast<CBasePlayer*>(pevAttacker);
		if (!pPlayerAttacker)
			return;

		if (!pPlayerAttacker->m_pActiveItem)
			return;

		if (pPlayerAttacker->m_pActiveItem->m_iId != WEAPON_KNIFE)
			return;

		if (!isSettled() && !isCompleted())
			return;

		UTIL_Sparks(ptr->vecEndPos);
		bool bDestory = !(bitsDamageType & DMG_KNIFE_SLASH);

		if (isSettled())
		{
			if (bDestory)
			{
				Killed(pevAttacker, GIB_ALWAYS);
				return;
			}

			auto strategy = dynamic_cast<CPlayerModStrategy_ZSH*>(pPlayerAttacker->m_pModStrategy.get());
			if (strategy)
			{
				int iOldProress = m_iProgress;
				m_iProgress += strategy->m_iBuildProgress;
				pev->health = min(pev->health + pev->max_health * ((float)(m_iProgress - iOldProress) / (float)m_iTotalProgress), pev->max_health);
			}

			if (m_iProgress >= m_iTotalProgress)
				Complete();
		}
		else
		{
			if (bDestory)
			{
				flDamage = (m_hBuilder && m_hBuilder.Get() == ENT(pevAttacker)) ? pev->max_health / 4 : pev->max_health / 10;
			}
			else
			{
				auto strategy = dynamic_cast<CPlayerModStrategy_ZSH*>(pPlayerAttacker->m_pModStrategy.get());
				if (strategy)
				{
					pev->health = min(strategy->m_iFixHealth + pev->health, pev->max_health);
				}
				return;
			}
		}

		CBaseMonster::TraceAttack(pevAttacker, flDamage, vecDir, ptr, bitsDamageType);
	}

	int CZSHBuilding::TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
	{
		if (!pev->takedamage)
			return 0;

		return CBaseMonster::TakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType);
	}

	void CZSHBuilding::Killed(entvars_t* pevAttacker, int iGib)
	{
		if (!IsAlive())
			return;

		iGib = GIB_ALWAYS;
		UTIL_ClientPrintAll(HUD_PRINTTALK, "#CSO_zsht_Destroy_Build_Message", m_pClass->GetName());
		EMIT_SOUND(ENT(pev), CHAN_STATIC, zsh::SOUND_BUILDING_CRASH, VOL_NORM, ATTN_NORM);
		return CBaseMonster::Killed(pevAttacker, iGib);
	}

	void CZSHBuilding::GibMonster()
	{
		Vector vecSpot = pev->origin + (pev->mins + pev->maxs) * 0.5;
		Vector vecVelocity(RANDOM_FLOAT(-50.0, 50.0), RANDOM_FLOAT(-50.0, 50.0), RANDOM_FLOAT(20.0, 50.0));

		MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, vecSpot);
		WRITE_BYTE(TE_BREAKMODEL);
		WRITE_COORD(vecSpot.x);		// position
		WRITE_COORD(vecSpot.y);
		WRITE_COORD(vecSpot.z);
		WRITE_COORD(pev->size.x);	// size
		WRITE_COORD(pev->size.y);
		WRITE_COORD(pev->size.z);
		WRITE_COORD(vecVelocity.x);	// velocity
		WRITE_COORD(vecVelocity.y);
		WRITE_COORD(vecVelocity.z);
		WRITE_BYTE(10);			// randomization
		WRITE_SHORT(m_iMetalGibs);		// model id#
		WRITE_BYTE(6);			// # of shards, let client decide
		WRITE_BYTE(25);			// duration, 2.5 seconds
		WRITE_BYTE(BREAK_METAL);		// flags
		MESSAGE_END();
	}

	void CZSHBuilding::BecomeDead(void)
	{
		pev->health = 0;
		pev->movetype = MOVETYPE_NONE;
		pev->flags &= ~FL_ONGROUND;

		pev->takedamage = DAMAGE_NO;
		pev->deadflag = DEAD_DEAD;
		pev->solid = SOLID_NOT;
	}

	void CZSHBuilding::OnThink()
	{
		pev->nextthink = gpGlobals->time + 0.1s;
		//StudioFrameAdvance();
		m_pClass->OnThink();
	}

	void CZSHBuilding::OnTouch(CBaseEntity* pOther)
	{
		m_pClass->OnTouch(pOther);
	}

	void CBuilding_Generator_Diesel::OnCreate()
	{
		PRECACHE_MODEL("models/zsh_deadcity/zsh_generator_1.mdl");
		SET_MODEL(m_pBuilding->edict(), "models/zsh_deadcity/zsh_generator_1.mdl");
		m_vecMins = Vector(-22.0, -15.0, 0.0);
		m_vecMaxs = Vector(22.0, 15.0, 37.0);
	}

	void CBuilding_Generator_Diesel::OnCompleted()
	{
	}

	void CBuilding_Generator_Diesel::OnThink()
	{
		m_pBuilding->pev->nextthink = gpGlobals->time + 5s;

		auto mod = (CMod_ZombieShelter*)g_pGameRules;
		mod->AddResource(zsh::RT_EPOWER, 1);
	}
}