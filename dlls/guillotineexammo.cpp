#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "studio.h"
#include "wpn_shared/wpn_guillotine.h"
#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif


namespace sv {

// Made by Sh@de(Xiaobaibai)
// pev_euser1 = Gun<->GuillotineAmmo
// pev_euser2 = GuillotineAmmo->AttackingPlayer
// pev_fuser1 = GuillotineAmmo->TimeReturn
#define GUILLOTINE_AMMO_MAX_DISTANCE	18.0 * 39.37
#define GUILLOTINE_AMMO_HEADCUT_TIME	10
#define GUILLOTINE_AMMO_SPEED	1000.0


	LINK_ENTITY_TO_CLASS(guillotineexAmmo, CGuillotineexAmmo);

	enum guillotineexAmmo_e
	{
		GUILLOTINE_START,
		GUILLOTINE_BACK
	};

	void CGuillotineexAmmo::Precache(void)
	{
		m_iModelIndex = PRECACHE_MODEL("models/guillotineex_projectile1.mdl");
		m_iLostSpriteIndex = PRECACHE_MODEL("sprites/guillotine_lost.spr");

		PRECACHE_SOUND("weapons/guillotine_explode.wav");
		PRECACHE_SOUND("weapons/janus9_wood1.wav");
		PRECACHE_SOUND("weapons/janus9_wood2.wav");
		PRECACHE_SOUND("weapons/janus9_metal1.wav");
		PRECACHE_SOUND("weapons/janus9_metal2.wav");
		PRECACHE_SOUND("weapons/janus9_stone1.wav");
		PRECACHE_SOUND("weapons/janus9_stone2.wav");
	}

	void CGuillotineexAmmo::Spawn(void)
	{
		Precache();

		pev->movetype = MOVETYPE_FLY;
		pev->solid = SOLID_TRIGGER;

		SET_MODEL(ENT(pev), "models/guillotineex_projectile1.mdl");
		UTIL_SetOrigin(pev, pev->origin);
		//UTIL_SetSize(pev, g_vecZero, g_vecZero);
		UTIL_SetSize(pev, { -1.44, -1.45, -1.3 }, { 1.44, 1.45, 4.3 });

		SetTouch(&CGuillotineexAmmo::FireTouch);
		SetThink(&CGuillotineexAmmo::ReturnThink);

		pev->sequence = 0;

		ResetSequenceInfo();

		pev->nextthink = gpGlobals->time;
		m_iState = GUILLOTINE_START;
		m_iHeadCutCounter = 0;	//m_iHeadCutCounter
		pev->iuser3 = 0;	//m_TargetVector

		m_flTimeReturn = gpGlobals->time + 0.73s;
		
	}

	void CGuillotineexAmmo::ReturnThink(void)
	{
		if (gpGlobals->time > m_flTimeReturn)
		{
			pev->sequence = 0;

			ResetSequenceInfo();

			m_iState = GUILLOTINE_BACK;

			m_TargetVector.clear();
			pev->iuser4 = 0;

			SetThink(&CGuillotineexAmmo::FireThink);
			pev->nextthink = gpGlobals->time;
			return;
		}

		pev->nextthink = gpGlobals->time + 0.01s;
		CheckDamage();
	}

	void CGuillotineexAmmo::FireThink(void)
	{
		if (ShouldCrashHalfway())
		{
			Crash();

			return;
		}

		Vector vecOrigin, vecOrigin2, vecVelocity;
		vecOrigin = pev->origin;
		vecOrigin2 = pev->owner->v.origin;
		vecVelocity = vecOrigin2 - vecOrigin;

		if (vecVelocity.Length() < 42.0)
		{
			Catched();
			return;
		}

		pev->velocity = vecVelocity.Normalize() * 1000.0;
		pev->nextthink = gpGlobals->time + 0.01s;
		CheckDamage();
	}


	void CGuillotineexAmmo::CheckDamage()
	{
		Vector vecOrigin, vecDirection, vecVelocity;
		vecOrigin = pev->origin;
		vecVelocity = pev->velocity;
		vecDirection = pev->velocity.Normalize();

		Vector vecEnd;
		vecEnd = vecOrigin + vecDirection * 39.37 * 1.5;

		TraceResult tr;

		UTIL_TraceLine(vecOrigin, vecEnd, missile, pev->owner, &tr);

		CBaseEntity* pHit = Instance(tr.pHit);

		if (tr.flFraction < 1.0 && tr.pHit && tr.pHit != ENT(pev))
		{
			int index = pHit->entindex();

			auto iter = m_TargetVector.begin();
			for (iter; iter != m_TargetVector.end(); ++iter)
			{
				if (*iter == index)
					break;
			}

			if (iter != m_TargetVector.end())
			{
				return;
			}

			if(CanAttack(pHit))
			{
				ClearMultiDamage();

				pHit->TraceAttack(VARS(pev->owner), m_flDirectDamage, gpGlobals->v_forward, &tr, DMG_BULLET | DMG_NEVERGIB);

				ApplyMultiDamage(pev, VARS(pev->owner));
			}

			m_TargetVector.push_back(pHit->entindex());	
		}

		if (pHit)
		{
			if (!pHit->IsAlive())
				return;

			if (m_iState == GUILLOTINE_START && tr.iHitgroup == HITGROUP_HEAD && !m_iHeadCutCounter && m_iTeam != pHit->m_iTeam)
			{
				m_hHeadCutting = pHit;
				SetTouch(NULL);
				SetThink(&CGuillotineexAmmo::HeadCutThink);
				m_flHeadCutTime = gpGlobals->time;

				pev->sequence = 1;

				ResetSequenceInfo();

				Vector vecOrigin2;
				vecOrigin2 = pHit->pev->origin;
				m_vecDelta = tr.vecEndPos - vecOrigin2;

				pev->nextthink = gpGlobals->time + 0.001s;
			}
		}
	}


	void CGuillotineexAmmo::FireTouch(CBaseEntity* pOther)
	{
		if(ShouldCrashHalfway())
		{
			Crash();

			return;
		}

		if(pOther->edict() == pev->owner)
			return;

		if (!pOther->IsAlive())
		{
			//CGuillotineAmmo_MaterialSound
			if (m_iState == GUILLOTINE_BACK)
			{
				Crash();

				return;
			}
			else
			{
				SetTouch(&CGuillotineexAmmo::FireTouch);
				SetThink(&CGuillotineexAmmo::ReturnThink);
				pev->nextthink = gpGlobals->time;
				m_flTimeReturn = gpGlobals->time;
			}
		}
	}

	bool CGuillotineexAmmo::ShouldCrashHalfway(void)
	{
		if (!pev->owner)
			return true;

		if (OFFSET(pev->owner) == eoNullEntity)
			return true;

		entvars_t* pevOwner = VARS(pev->owner);

		if (pevOwner->deadflag != DEAD_NO)
			return true;

		if (!(pevOwner->origin.x || pevOwner->origin.y || pevOwner->origin.z))
			return true;

		if ((pev->origin - pevOwner->origin).Length() > GUILLOTINE_AMMO_MAX_DISTANCE * 2)
			return true;

		return false;
	}


	void CGuillotineexAmmo::Crash(void)
	{
		pev->velocity = g_vecZero;

		Vector vecEffectPos = pev->origin + (pev->maxs + pev->mins) * 0.5;

		MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
		WRITE_BYTE(TE_BREAKMODEL);
		WRITE_COORD(vecEffectPos.x);
		WRITE_COORD(vecEffectPos.y);
		WRITE_COORD(vecEffectPos.z);
		WRITE_COORD(pev->size.x);
		WRITE_COORD(pev->size.y);
		WRITE_COORD(pev->size.z);
		WRITE_COORD(pev->velocity.x);
		WRITE_COORD(pev->velocity.y);
		WRITE_COORD(pev->velocity.z);
		WRITE_BYTE(10);
		WRITE_SHORT(g_sModelIndexGuillotineGibs);
		WRITE_BYTE(20);
		WRITE_BYTE(13);
		WRITE_BYTE(0);
		MESSAGE_END();

		MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
		WRITE_BYTE(TE_TEMPSPRITE);
		WRITE_COORD(vecEffectPos.x);
		WRITE_COORD(vecEffectPos.y);
		WRITE_COORD(vecEffectPos.z);
		WRITE_SHORT(m_iLostSpriteIndex);
		WRITE_BYTE(5);
		WRITE_BYTE(100);
		WRITE_BYTE(40);
		MESSAGE_END();

		/*MESSAGE_BEGIN(MSG_ALL, SVC_TEMPENTITY);
		WRITE_BYTE(152);
		WRITE_SHORT(entindex());
		MESSAGE_END();*/

		EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "weapons/guillotine_explode.wav", 0.55, ATTN_NORM, 0, PITCH_NORM);

		SetThink(NULL);
		SetTouch(NULL);

		UTIL_Remove(this);
	}
	
	void CGuillotineexAmmo::Catched()
	{

		CBaseEntity* pOwner = CBaseEntity::Instance(pev->owner);
		CBasePlayer* pEntityOwner = static_cast<CBasePlayer*>(pOwner);


		CGuillotine* pWeapon = (CGuillotine*)((CBasePlayer*)pEntityOwner)->m_pActiveItem;

		if (pWeapon && pWeapon->m_iId == WEAPON_GUILLOTINEEX)
		{
			pWeapon->CatchAmmo();

			pev->velocity = g_vecZero;

			SetTouch(NULL);
			SetThink(NULL);

			UTIL_Remove(this);

			return;
		}

		Crash();

		return;
	}

	void CGuillotineexAmmo::HeadCutThink(void)
	{
		if (ShouldCrashHalfway())
		{
			Crash();

			return;
		}

		CBaseEntity* pCutting = m_hHeadCutting;

		if (!m_hHeadCutting.Get() || !pCutting)
		{
			SetTouch(&CGuillotineexAmmo::FireTouch);
			SetThink(&CGuillotineexAmmo::ReturnThink);
			pev->nextthink = gpGlobals->time;
			m_flTimeReturn = gpGlobals->time;
			//set_pdata_float(iEnt, m_flTimeWeaponIdle, 0.1);
			return;
		}

		if (gpGlobals->time > m_flHeadCutTime)
		{
			if (m_iHeadCutCounter < 18 && pCutting->IsAlive() && pCutting->m_iTeam != pev->owner->v.team)
			{
				m_hHeadCutting->m_LastHitGroup = HITGROUP_HEAD;

				m_hHeadCutting->TakeDamage(pev, VARS(pev->owner), m_flHeadCutDamage, DMG_BULLET | DMG_NEVERGIB);
				m_iHeadCutCounter++;
				m_flHeadCutTime = gpGlobals->time + 0.2s;
			}
			else
			{
				SetTouch(&CGuillotineexAmmo::FireTouch);
				SetThink(&CGuillotineexAmmo::ReturnThink);
				pev->nextthink = gpGlobals->time;
				m_flTimeReturn = gpGlobals->time;
				//set_pdata_float(iEnt, m_flTimeWeaponIdle, 0.1);
				return;
			}
		}

		Vector vecHeadOrigin;
		vecHeadOrigin = m_vecDelta + pCutting->pev->origin;
		pev->origin = vecHeadOrigin;
		pev->velocity = pCutting->pev->velocity;
		pev->nextthink = gpGlobals->time + 0.001s;
	}

	bool CGuillotineexAmmo::IsHeadCutting(void)
	{
		return m_iHeadCutCounter > 0;
	}

	bool CGuillotineexAmmo::CanAttack(CBaseEntity* pOther)
	{
		if (FNullEnt(pOther->edict()))
			return false;

		if (pOther->IsDormant())
			return false;

		if (pOther->pev->takedamage == DAMAGE_NO)
			return false;

		if (pOther->IsPlayer() || pOther->Classify() == CLASS_PLAYER_ALLY)
			return true;

		return false;
	}

	void CGuillotineexAmmo::MaterialSound(TraceResult* ptr)
	{
	
	}

}
