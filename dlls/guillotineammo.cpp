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

#define GUILLOTINE_AMMO_MAX_DISTANCE	18.0 * 39.37
#define GUILLOTINE_AMMO_HEADCUT_TIME	10
#define GUILLOTINE_AMMO_SPEED	1000.0


	LINK_ENTITY_TO_CLASS(guillotineAmmo, CGuillotineAmmo);

	enum guillotineammo_e
	{
		GUILLOTINEAMMO_IDLE,
		GUILLOTINEAMMO_HEADHIT
	};

	void CGuillotineAmmo::SetAnimation(int sequence, bool bResetFrame)
	{
		if (sequence < 0)
			sequence = 0;

		if (bResetFrame)
			pev->frame = 0;

		pev->framerate = 1;

		if (m_nCurrentAnimation != sequence)
		{
			pev->sequence = m_nCurrentAnimation = sequence;

			ResetSequenceInfo();
		}
	}

	void CGuillotineAmmo::Precache(void)
	{
		m_iModelIndex = PRECACHE_MODEL("models/guillotine_projectile.mdl");
		m_iLostSpriteIndex = PRECACHE_MODEL("sprites/guillotine_lost.spr");

		PRECACHE_SOUND("weapons/guillotine_explode.wav");
		PRECACHE_SOUND("weapons/janus9_wood1.wav");
		PRECACHE_SOUND("weapons/janus9_wood2.wav");
		PRECACHE_SOUND("weapons/janus9_metal1.wav");
		PRECACHE_SOUND("weapons/janus9_metal2.wav");
		PRECACHE_SOUND("weapons/janus9_stone1.wav");
		PRECACHE_SOUND("weapons/janus9_stone2.wav");
	}

	void CGuillotineAmmo::Spawn(void)
	{
		Precache();

		pev->movetype = MOVETYPE_FLY;
		pev->solid = SOLID_TRIGGER;

		SET_MODEL(ENT(pev), "models/guillotine_projectile.mdl");
		UTIL_SetOrigin(pev, pev->origin);
		UTIL_SetSize(pev, g_vecZero, g_vecZero);
		//UTIL_SetSize(pev, { -1.44, -1.45, -1.3 }, { 1.44, 1.45, 4.3 });


		m_vecMins = Vector(-14.4, -14.5, -1.3);
		m_vecMaxs = Vector(14.4, 14.5, 4.3);

		m_nCurrentAnimation = -1;
		m_iHeadCutCounter = 0;

		m_flDirectDamage = 0;
		m_flHeadCutDamage = 0;
		m_flHeadCutTime = invalid_time_point;

		m_vecInitialPos = pev->origin;
		m_vecImpactPos = g_vecZero;

		m_hHeadCutting = NULL;

		SetTouch(&CGuillotineAmmo::FireTouch);
		SetThink(&CGuillotineAmmo::FireThink);

		pev->nextthink = gpGlobals->time;

		SetAnimation(GUILLOTINEAMMO_IDLE, true);
	}

	void CGuillotineAmmo::FireThink(void)
	{
		if (ShouldCrashHalfway())
		{
			Crash();

			return;
		}

		float flDistance = (pev->origin - m_vecInitialPos).Length();

		if (flDistance < GUILLOTINE_AMMO_MAX_DISTANCE && !RecursiveWorldTrace(1000.0 * 0.05, pev->origin, pev->owner, flDistance >= 1000.0 * 0.05))
		{
			pev->nextthink = gpGlobals->time + 0.05s;
		}
		else
		{
			Return(false);
		}
	}

	void CGuillotineAmmo::FireTouch(CBaseEntity* pOther)
	{
		if (pOther->edict() == pev->owner)
			return;

		TraceResult tr;

		UTIL_TraceLine(pev->origin, pev->origin + pev->velocity.Normalize() * 10, dont_ignore_monsters, pev->owner, &tr);

		if (CanAttack(pOther))
		{
			Fire(&tr, pOther, pev->origin + m_vecMins, pev->origin + pev->maxs);
		}
		else if (pOther->IsBSPModel())
		{
			m_vecImpactPos = tr.vecEndPos;

			MaterialSound(&tr);
			Return(false);
		}
	}

	bool CGuillotineAmmo::ShouldCrashHalfway(void)
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


	void CGuillotineAmmo::Crash(void)
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

	bool CGuillotineAmmo::RecursiveWorldTrace(float delta, Vector vecOrigin, edict_t* pentIgnore, bool bMaterialSound)
	{
		if (!(pev->velocity.x || pev->velocity.y || pev->velocity.z))
			return false;

		if (!pentIgnore)
			return false;

		if (OFFSET(pentIgnore) == eoNullEntity)
			return false;

		if (delta <= 0.0)
			return false;

		Vector vecPositions[5] =
		{
			vecOrigin,
			vecOrigin + m_vecMins + Vector(3.5, 3.5, 0.5),
			vecOrigin + m_vecMaxs - Vector(3.5, 3.5, 0.5),
			vecOrigin + Vector(m_vecMins.x + 3.5, m_vecMaxs.y - 3.5, m_vecMins.z + 0.5),
			vecOrigin + Vector(m_vecMaxs.x - 3.5, m_vecMins.y + 3.5, m_vecMaxs.z - 0.5)
		};

		TraceResult tr;

		for (int i = 0; i < 5; i++)
		{
			Vector vecStart = vecPositions[i];
			Vector vecEnd = vecStart + pev->velocity.Normalize() * delta;

			UTIL_TraceLine(vecStart, vecEnd, dont_ignore_monsters, pentIgnore, &tr);

			if (tr.flFraction < 1.0 && tr.pHit && tr.pHit != ENT(pev))
			{
				if (tr.pHit == pev->owner)
					return false;

				Vector vecDelta = tr.vecEndPos - vecStart;

				if (!RangeAttack(&tr, vecDelta))
				{
					if (bMaterialSound)
					{
						m_vecImpactPos = vecOrigin + vecDelta;

						MaterialSound(&tr);

						return true;
					}
				}

				if (tr.iHitgroup == HITGROUP_HEAD)
					return false;

				return RecursiveWorldTrace(delta - 10.0, pev->origin + pev->velocity * 10, tr.pHit, bMaterialSound);
			}
		}

		return false;
	}

	void CGuillotineAmmo::StartHeadCut(CBaseEntity* pOther)
	{
		pev->velocity = g_vecZero;
		pev->angles = g_vecZero;

		/*MESSAGE_BEGIN(MSG_ALL, SVC_TEMPENTITY);
		WRITE_BYTE(151);
		WRITE_BYTE(2);
		WRITE_SHORT(entindex());
		WRITE_SHORT(pOther->entindex());
		WRITE_SHORT(m_iModelIndex);
		WRITE_BYTE(1);
		WRITE_COORD(m_vecInitialPos.x);
		WRITE_COORD(m_vecInitialPos.y);
		WRITE_COORD(m_vecInitialPos.z);
		WRITE_COORD(pev->angles.x);
		WRITE_COORD(pev->angles.y);
		WRITE_COORD(pev->angles.z);
		WRITE_FLOAT((10 + int(s_flHeadCutTimesEnhanced[m_iDamageLevel]) - 1) * 0.2);
		MESSAGE_END();*/

		//pev->effects |= EF_NODRAW;

		SetTouch(NULL);
		SetThink(&CGuillotineAmmo::HeadCutThink);

		pev->nextthink = gpGlobals->time;

		SetAnimation(GUILLOTINEAMMO_HEADHIT, true);
	}

	void CGuillotineAmmo::HeadCutThink(void)
	{
		
		if (ShouldCrashHalfway())
		{
			pev->movetype = MOVETYPE_FLY;
			pev->solid = SOLID_TRIGGER;
			Crash();

			return;
		}

		CBaseEntity* pCutting = m_hHeadCutting;

		if (!m_hHeadCutting.Get() || !pCutting)
		{
			pev->movetype = MOVETYPE_FLY;
			pev->solid = SOLID_TRIGGER;
			Return(true);

			return;
		}

		pev->movetype = MOVETYPE_NOCLIP;
		pev->solid = SOLID_NOT;

		if (pCutting->pev->origin.x || pCutting->pev->origin.y || pCutting->pev->origin.z)
		{
			pev->origin = pCutting->pev->origin + m_vecInitialPos;
		}

		if (!m_iHeadCutCounter)
			m_flHeadCutTime = gpGlobals->time;

		if (gpGlobals->time >= m_flHeadCutTime)
		{
			float flDamage = m_flHeadCutDamage;

			m_iHeadCutCounter++;
			m_flHeadCutTime += 0.2s;

			while (gpGlobals->time >= m_flHeadCutTime)
			{
				m_iHeadCutCounter++;

				m_flHeadCutTime += 0.2s;
				flDamage += m_flHeadCutDamage;
			}

			if (flDamage > 0)
			{
				m_hHeadCutting->m_LastHitGroup = HITGROUP_HEAD;

				m_hHeadCutting->TakeDamage(pev, VARS(pev->owner), flDamage, DMG_BULLET | DMG_NEVERGIB);
			}
		}

		int iMaxTimes = GUILLOTINE_AMMO_HEADCUT_TIME;
		if (!m_hHeadCutting->IsAlive() || m_iHeadCutCounter >= iMaxTimes)
		{
			/*MESSAGE_BEGIN(MSG_ALL, SVC_TEMPENTITY);
			WRITE_BYTE(152);
			WRITE_SHORT(entindex());
			MESSAGE_END();*/
			pev->movetype = MOVETYPE_FLY;
			pev->solid = SOLID_TRIGGER;
			Return(true);

			m_TargetVector.push_back(m_hHeadCutting->entindex());
		}
		else
			pev->nextthink = gpGlobals->time + 0.01s;
	}


	void CGuillotineAmmo::Fire(TraceResult* ptr, CBaseEntity* pOther, Vector vecAbsMins, Vector vecAbsMaxs)
	{
		if (pOther->edict() == pev->owner)
			return;

		if (ShouldHeadCut(ptr, pOther, vecAbsMins, vecAbsMaxs))
			StartHeadCut(pOther);
		else
			Attack(ptr, pOther);
	}

	void CGuillotineAmmo::Attack(TraceResult* ptr, CBaseEntity* pOther)
	{
		int index = pOther->entindex();

		auto iter = m_TargetVector.begin();
		for (iter; iter != m_TargetVector.end(); ++iter)
		{
			if (*iter == index)
				break;
		}

		if (iter != m_TargetVector.end())
		{
		/*	if (pOther->Classify() == CLASS_BREAKABLE)
				Crash();*/

			return;
		}

		/*if (pOther->Classify() == CLASS_BREAKABLE)
		{
			pOther->TakeDamage(pev, VARS(pev->owner), m_flDirectDamage, DMG_BULLET | DMG_NEVERGIB);

			m_vecImpactPos = ptr->vecEndPos;
			Return(false);
		}
		else*/
		{
			UTIL_MakeVectors(pev->angles);

			ClearMultiDamage();

			pOther->TraceAttack(VARS(pev->owner), m_flDirectDamage, gpGlobals->v_forward, ptr, DMG_BULLET | DMG_NEVERGIB);

			ApplyMultiDamage(pev, VARS(pev->owner));
		}

		m_TargetVector.push_back(pOther->entindex());
	}

	bool CGuillotineAmmo::IsHeadCutting(void)
	{
		return m_iHeadCutCounter > 0;
	}

	bool CGuillotineAmmo::ShouldHeadCut(TraceResult* ptr, CBaseEntity* pOther, Vector vecAbsMins, Vector vecAbsMaxs)
	{
		/*
		if (UTIL_IsZombieScenario())
			return false;
		*/
		if (g_pGameRules->PlayerRelationship(m_pOwner, pOther) == GR_TEAMMATE && !friendlyfire.value)
			return false;

		/*if (!friendlyfire.value && m_iTeam == pOther->m_iTeam)
			return false;*/

		if (IsHeadCutting())
			return false;

		if (!pOther->IsPlayer())
			return false;

		auto iter = m_TargetVector.begin();
		int index = pOther->entindex();

		for (iter; iter != m_TargetVector.end(); ++iter)
		{
			if (*iter == index)
				break;
		}

		if (iter != m_TargetVector.end())
			return false;

		studiohdr_t* phdr = (studiohdr_t*)GET_MODEL_PTR(pOther->edict());

		if (!phdr)
			return false;

		mstudiobbox_t* phitboxes = (mstudiobbox_t*)((byte*)phdr + phdr->hitboxindex);
		Vector vecOrigin, vecAngles;
		const float flTolerance = 0.025f * 39.37f;

		for (int i = 0; i < phdr->numhitboxes; i++)
		{
			if (phitboxes[i].group == HITGROUP_HEAD)
			{
				GetBonePosition(phitboxes[i].bone, vecOrigin, vecAngles);

				Vector vecMins = vecOrigin - Vector(flTolerance, flTolerance, flTolerance);
				Vector vecMaxs = vecOrigin + Vector(flTolerance, flTolerance, flTolerance);

				if (ptr->iHitgroup != HITGROUP_HEAD)
				{
					for (int j = 0; j < 3; j++)
					{
						if (vecMaxs[j] + (vecAbsMins[j] - vecMaxs[j]) > vecMaxs[j])
							return false;

						if (vecMins[j] > vecMaxs[j] + (vecAbsMaxs[j] - vecMaxs[j]))
							return false;
					}
				}

				ptr->iHitgroup = HITGROUP_HEAD;
				m_hHeadCutting = pOther;
				m_vecInitialPos = (vecAbsMins + vecAbsMaxs) * 0.5 - pOther->pev->origin;

				return true;
			}
		}

		return false;
	}

	bool CGuillotineAmmo::CanAttack(CBaseEntity* pOther)
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

	void CGuillotineAmmo::MaterialSound(TraceResult* ptr)
	{
		Vector vecSrc = m_vecImpactPos;

		if (!(m_vecImpactPos.x || m_vecImpactPos.y || m_vecImpactPos.z))
			vecSrc = pev->origin;

		Vector vecEnd = vecSrc + pev->velocity;

		const char* rgsz[2];

		switch (UTIL_TextureHit(ptr->pHit, vecSrc, vecEnd))
		{
		case CHAR_TEX_WOOD:
			rgsz[0] = "weapons/janus9_wood1.wav";
			rgsz[1] = "weapons/janus9_wood2.wav";
			break;

		case CHAR_TEX_GRATE:
		case CHAR_TEX_METAL:
		case CHAR_TEX_COMPUTER:
			rgsz[0] = "weapons/janus9_metal1.wav";
			rgsz[1] = "weapons/janus9_metal2.wav";
			break;

		default:
			rgsz[0] = "weapons/janus9_stone1.wav";
			rgsz[1] = "weapons/janus9_stone2.wav";
			break;
		}

		UTIL_EmitAmbientSound(ENT(eoNullEntity), ptr->vecEndPos, rgsz[RANDOM_LONG(0, 1)], 0.9, ATTN_NORM, 0, 96 + RANDOM_LONG(0, 15));
	}

	bool CGuillotineAmmo::RangeAttack(TraceResult* ptr, Vector vecTraceDelta)
	{
		if (ptr->pHit == ENT(pev))
			return false;

		if (FNullEnt(ptr->pHit))
			return false;

		CBaseEntity* pHit = Instance(ptr->pHit);

		if (pHit == NULL)
			return false;

		// if (pHit->ReflectGauss())
		//	return false;

		if (pHit->pev->takedamage == DAMAGE_NO)
			return false;

		Fire(ptr, pHit, pev->origin + vecTraceDelta + m_vecMins, pev->origin + vecTraceDelta + m_vecMaxs);

		return true;
	}

	void CGuillotineAmmo::Return(bool bResetFrame)
	{
		if (m_vecImpactPos.x || m_vecImpactPos.y || m_vecImpactPos.z)
		{
			Vector vecDir = pev->velocity.Normalize();

			float flMax = 0.0;

			if (vecDir.x > flMax)
				flMax = vecDir.x;

			if (vecDir.y > flMax)
				flMax = vecDir.y;

			if (vecDir.z > flMax)
				flMax = vecDir.z;

			if (flMax < 1.0)
				flMax = 1.0;

			pev->origin.x = m_vecImpactPos.x - (m_vecMaxs.x - m_vecMins.x) * (vecDir.x / flMax) * 0.5 * 1.1;
			pev->origin.y = m_vecImpactPos.y - (m_vecMaxs.y - m_vecMins.y) * (vecDir.y / flMax) * 0.5 * 1.1;
			pev->origin.z = m_vecImpactPos.z - (m_vecMaxs.z - m_vecMins.z) * (vecDir.z / flMax) * 0.5 * 1.1;

			m_vecImpactPos = g_vecZero;
		}

		//pev->effects &= ~EF_NODRAW;
		pev->velocity = g_vecZero;

		m_TargetVector.clear();

		SetTouch(&CGuillotineAmmo::ReturnTouch);
		SetThink(&CGuillotineAmmo::ReturnThink);

		pev->nextthink = gpGlobals->time;

		SetAnimation(GUILLOTINEAMMO_IDLE, bResetFrame);
	}

	void CGuillotineAmmo::ReturnThink(void)
	{
		if (ShouldCrashHalfway())
		{
			Crash();

			return;
		}

		float flSpeed = GUILLOTINE_AMMO_SPEED;
		Vector vecDelta = VARS(pev->owner)->origin + VARS(pev->owner)->view_ofs - Vector(0, 0, 10) - pev->origin;

		pev->velocity = vecDelta.Normalize() * flSpeed;
		pev->angles = UTIL_VecToAngles(pev->velocity);

		if (RecursiveWorldTrace(flSpeed * 0.05, pev->origin, pev->owner, vecDelta.LengthSquared() >= flSpeed * 0.05 + 70.0))
		{
			pev->origin = m_vecImpactPos;

			Crash();

			return;
		}

		pev->nextthink = gpGlobals->time + 0.05s;
	}

	void CGuillotineAmmo::ReturnTouch(CBaseEntity* pOther)
	{
		if (!(pev->velocity.x || pev->velocity.y || pev->velocity.z))
			return;

		TraceResult tr;

		UTIL_TraceLine(pev->origin, pev->origin + pev->velocity.Normalize() * 10, dont_ignore_monsters, pev->owner, &tr);

		if (!CanAttack(pOther))
		{
			if (!pOther->ReflectGauss())
				return;

			MaterialSound(&tr);
			Crash();

			return;
		}

		if (pOther->edict() != pev->owner)
		{
			Fire(&tr, pOther, pev->origin + m_vecMins, pev->origin + m_vecMaxs);

			return;
		}

		CGuillotine* pWeapon = (CGuillotine*)((CBasePlayer*)pOther)->m_pActiveItem;

		if (pWeapon && pWeapon->m_iId == WEAPON_GUILLOTINE)
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
}
