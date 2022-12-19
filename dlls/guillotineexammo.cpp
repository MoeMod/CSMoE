#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "studio.h"
#include "wpn_shared/wpn_guillotine.h"

#include "gamemode/mods.h"
#include "effects.h"
#include "customentity.h"

#define BTEVER 0

namespace sv {
#if BTEVER
TL_ADD(CGuillotineexAmmo)
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
#endif


#define GUILLOTINE_AMMO_MAX_DISTANCE	20.0 * 39.37
#define GUILLOTINE_AMMO_HEADCUT_TIME	10
#define GUILLOTINE_AMMO_RADIUS	255
#define GUILLOTINE_AMMO_SPEED	1000.0
#define GUILLOTINE_AMMO2_HEADCUT_TIME	14
#define GUILLOTINE_AMMO2_RADIUS	258

LINK_ENTITY_TO_CLASS(guillotineexAmmo, CGuillotineExAmmo);

enum guillotineexammo_e
{
	GUILLOTINEAMMO_IDLE,
	GUILLOTINEAMMO_HEADHIT
};

void CGuillotineExAmmo::SetAnimation(int sequence, bool bResetFrame)
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

void CGuillotineExAmmo::Precache(void)
{
	m_iModelIndex = PRECACHE_MODEL("models/guillotineex_projectile1.mdl");
	PRECACHE_MODEL("models/guillotineex_projectile2.mdl");
	m_iLostSpriteIndex = PRECACHE_MODEL("sprites/guillotine_lost.spr");
	PRECACHE_MODEL("sprites/ef_elec.spr");

	
	PRECACHE_SOUND("weapons/guillotine_explode.wav");
	PRECACHE_SOUND("weapons/guillotineex_shoot-2_exp.wav");
	PRECACHE_SOUND("weapons/guillotineex_shoot-1_exp.wav");

	PRECACHE_SOUND("weapons/janus9_wood1.wav");
	PRECACHE_SOUND("weapons/janus9_wood2.wav");
	PRECACHE_SOUND("weapons/janus9_metal1.wav");
	PRECACHE_SOUND("weapons/janus9_metal2.wav");
	PRECACHE_SOUND("weapons/janus9_stone1.wav");
	PRECACHE_SOUND("weapons/janus9_stone2.wav");
}


CGuillotineExAmmo* CGuillotineExAmmo::CreateAmmo(const Vector& vecOrigin, const Vector& vecAngles, CBasePlayer* pOwner, int iType)
{
	edict_t* pent = CREATE_NAMED_ENTITY(MAKE_STRING("guillotineexAmmo"));

	if (FNullEnt(pent))
	{
		ALERT(at_console, "NULL Ent in Create!\n");
		return NULL;
	}

	CGuillotineExAmmo* pAmmo = (CGuillotineExAmmo*)Instance(pent);

	if (pAmmo)
	{
		pAmmo->m_iType = iType;
		pAmmo->m_pOwner = pOwner;
		pAmmo->pev->owner = pOwner->edict();
		pAmmo->pev->origin = vecOrigin;
		pAmmo->pev->angles = vecAngles;
		pAmmo->Spawn();
	}

	return pAmmo;
}

void CGuillotineExAmmo::Spawn(void)
{
	Precache();

	pev->movetype = MOVETYPE_FLY;
	pev->solid = SOLID_TRIGGER;

	if(m_iType)
		SET_MODEL(ENT(pev), "models/guillotineex_projectile2.mdl");
	else
		SET_MODEL(ENT(pev), "models/guillotineex_projectile1.mdl");

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
	m_pTarget = NULL;

	pBeam = CBeam::BeamCreate("sprites/lgtning.spr", 60);

	if (m_iType)
	{
		pBeam->SetColor(255, 185, 0);
		pBeam->SetScrollRate(60);
		pBeam->SetBrightness(0);
		pBeam->SetNoise(0);
	}
		
	else
	{
		pBeam->SetColor(0, 100, 255);
		pBeam->SetScrollRate(0);
		pBeam->SetBrightness(0);
		pBeam->SetNoise(60);
	}

	pBeam->pev->effects |= EF_NODRAW;


	if (m_iType)
	{	
		float flMinDist = 9999.0f;
		CBaseEntity* pEntity = NULL;
		while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, GUILLOTINE_AMMO_MAX_DISTANCE)) != nullptr)
		{
			if (!pEntity->IsPlayer() && pEntity->Classify() != CLASS_PLAYER_ALLY)
				continue;

			if (!pEntity->IsAlive())
				continue;

			if (g_pGameRules->PlayerRelationship(m_pOwner, pEntity) == GR_TEAMMATE)
				continue;

			if (pEntity == m_pOwner)
				continue;

			Vector vecSrc = m_pOwner->GetGunPosition();
			Vector vecDelta = (pEntity->Center() - vecSrc).Normalize();
			float flDelta = vecDelta.Length();

			UTIL_MakeVectors(m_pOwner->pev->v_angle);

			if (DotProduct(gpGlobals->v_forward, vecDelta) < 0.7)
				continue;

			TraceResult tr;
			UTIL_TraceLine(m_pOwner->pev->origin, pEntity->pev->origin, missile, m_pOwner->edict(), &tr);

			if (tr.flFraction < 1.0)
			{
				if (!tr.pHit)
					continue;

				if (tr.pHit != pEntity->edict())
					continue;
			}

			if (flDelta < flMinDist)
			{
				flMinDist = flDelta;
				m_pTarget = pEntity;
			}
			continue;
		}

		SetThink(&CGuillotineExAmmo::ChargeAttackThink);
	}
		
	else
		SetThink(&CGuillotineExAmmo::FireThink);

	SetTouch(&CGuillotineExAmmo::FireTouch);
	pev->nextthink = gpGlobals->time;

	SetAnimation(GUILLOTINEAMMO_IDLE, true);
}

void CGuillotineExAmmo::ClearBeam()
{
	pBeam->SetBrightness(0);
	pBeam->pev->effects |= EF_NODRAW;
}

void CGuillotineExAmmo::ChargeAttackThink(void)
{
	if (ShouldCrashHalfway())
	{
		Crash();

		return;
	}

	if (m_pTarget != nullptr)
	{
		if (m_pTarget->IsAlive())
		{
			Vector vecDir = (m_pTarget->pev->origin - pev->origin).Normalize();
			pev->velocity = vecDir * GUILLOTINE_AMMO_SPEED;
		}
	}

	float flDistance = (pev->origin - m_vecInitialPos).Length();

	if (flDistance > 1000.0)
	{
		Crash();
	}

	pev->nextthink = gpGlobals->time + 0.05s;
}

void CGuillotineExAmmo::FireThink(void)
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

void CGuillotineExAmmo::FireTouch(CBaseEntity* pOther)
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
		if (m_iType)
		{
			MaterialSound(&tr);
			Crash();

			return;		
		}
		else
		{
			m_vecImpactPos = tr.vecEndPos;

			MaterialSound(&tr);
			Return(false);
		}
	}
}

bool CGuillotineExAmmo::ShouldCrashHalfway(void)
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

	if ((pev->origin - pevOwner->origin).Length() > GUILLOTINE_AMMO_MAX_DISTANCE * (m_iType ? 4 : 2))
		return true;

	return false;
}


void CGuillotineExAmmo::Crash(void)
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

	UTIL_Remove(pBeam);
	UTIL_Remove(this);
}

bool CGuillotineExAmmo::RecursiveWorldTrace(float delta, Vector vecOrigin, edict_t* pentIgnore, bool bMaterialSound)
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

void CGuillotineExAmmo::StartHeadCut(CBaseEntity* pOther)
{
	if (pOther->IsPlayer() || pOther->Classify() == CLASS_PLAYER_ALLY)
		m_pWeapon->m_iHitCount++;

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
	SetThink(&CGuillotineExAmmo::HeadCutThink);

	pev->nextthink = gpGlobals->time;

	SetAnimation(GUILLOTINEAMMO_HEADHIT, true);
}

void CGuillotineExAmmo::HeadCutThink(void)
{
	if (ShouldCrashHalfway())
	{
		pev->movetype = MOVETYPE_FLY;
		pev->solid = SOLID_TRIGGER;
		Crash();

		return;
	}
	
	CBaseEntity* pCutting = m_hHeadCutting;

	if (m_iType)
	{
		if (!m_hHeadCutting.Get() || !pCutting)
		{
			pev->movetype = MOVETYPE_NONE;
			pev->solid = SOLID_NOT;
			pev->aiment = nullptr;
		}
		else
		{
			pev->movetype = MOVETYPE_FOLLOW;
			pev->solid = SOLID_NOT;
			pev->aiment = pCutting->edict();
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

			CBaseEntity* pEntity = NULL;
			while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, GUILLOTINE_AMMO2_RADIUS)) != nullptr)
			{
				if (!pEntity->IsPlayer() && pEntity->Classify() != CLASS_PLAYER_ALLY)
					continue;

				if (!pEntity->IsAlive())
					continue;

				if (g_pGameRules->PlayerRelationship(m_pOwner, pEntity) == GR_TEAMMATE)
					continue;

				if (pEntity == pCutting)
					continue;

				if (pEntity == m_pOwner)
					continue;

				TraceResult tr;
				UTIL_TraceLine(m_pOwner->pev->origin, pEntity->pev->origin, missile, m_pOwner->edict(), &tr);

				pEntity->m_LastHitGroup = HITGROUP_CHEST;

				pEntity->TakeDamage(pev, VARS(pev->owner), flDamage, DMG_BULLET | DMG_NEVERGIB);
				break;
			}

			if (pEntity)
			{
				pBeam->SetType(BEAM_ENTS);
				pBeam->SetStartEntity(this->entindex());
				pBeam->SetEndEntity(pEntity->entindex());
				pBeam->SetStartAttachment(0);
				pBeam->SetEndAttachment(0);
				pBeam->RelinkBeam();
				pBeam->SetBrightness(230);
				pBeam->pev->effects &= ~EF_NODRAW;

				MESSAGE_BEGIN(MSG_ALL, gmsgMPToCL, nullptr, pEntity->pev);
				WRITE_BYTE(4);
				WRITE_SHORT(pEntity->entindex());
				WRITE_BYTE(20);
				MESSAGE_END();
			}
			else
				ClearBeam();

			if (flDamage > 0 && m_hHeadCutting.Get() && pCutting)
			{
				m_hHeadCutting->m_LastHitGroup = HITGROUP_HEAD;

				m_hHeadCutting->TakeDamage(pev, VARS(pev->owner), flDamage, DMG_BULLET | DMG_NEVERGIB);
			}
		}

		int iMaxTimes = GUILLOTINE_AMMO2_HEADCUT_TIME;

		if (m_iHeadCutCounter >= iMaxTimes)
		{
			SetThink(NULL);

			UTIL_Remove(pBeam);
			UTIL_Remove(this);
		}
		else
			pev->nextthink = gpGlobals->time + 0.01s;
	}
	else
	{

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

			CBaseEntity* pEntity = NULL;
			while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, GUILLOTINE_AMMO_RADIUS)) != nullptr)
			{
				if (!pEntity->IsPlayer() && pEntity->Classify() != CLASS_PLAYER_ALLY)
					continue;

				if (!pEntity->IsAlive())
					continue;

				if (g_pGameRules->PlayerRelationship(m_pOwner, pEntity) == GR_TEAMMATE)
					continue;

				if (pEntity == pCutting)
					continue;

				if (pEntity == m_pOwner)
					continue;

				TraceResult tr;
				UTIL_TraceLine(m_pOwner->pev->origin, pEntity->pev->origin, missile, m_pOwner->edict(), &tr);

				pEntity->m_LastHitGroup = HITGROUP_CHEST;

				pEntity->TakeDamage(pev, VARS(pev->owner), flDamage, DMG_BULLET | DMG_NEVERGIB);
				break;
			}

			if (pEntity)
			{
				pBeam->SetType(BEAM_ENTS);
				pBeam->SetStartEntity(this->entindex());
				pBeam->SetEndEntity(pEntity->entindex());
				pBeam->SetStartAttachment(0);
				pBeam->SetEndAttachment(0);
				pBeam->RelinkBeam();
				pBeam->SetBrightness(230);
				pBeam->pev->effects &= ~EF_NODRAW;

				MESSAGE_BEGIN(MSG_ALL, gmsgMPToCL, nullptr, pEntity->pev);
				WRITE_BYTE(4);
				WRITE_SHORT(pEntity->entindex());
				WRITE_BYTE(19);
				MESSAGE_END();
			}
			else
				ClearBeam();


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
}


void CGuillotineExAmmo::Fire(TraceResult* ptr, CBaseEntity* pOther, Vector vecAbsMins, Vector vecAbsMaxs)
{
	if (pOther->edict() == pev->owner)
		return;

	if (ShouldHeadCut(ptr, pOther, vecAbsMins, vecAbsMaxs))
		StartHeadCut(pOther);
	else
		Attack(ptr, pOther);
	
}

void CGuillotineExAmmo::Attack(TraceResult* ptr, CBaseEntity* pOther)
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
		if (pOther->IsPlayer() || pOther->Classify() == CLASS_PLAYER_ALLY)
			m_pWeapon->m_iHitCount++;

		UTIL_MakeVectors(pev->angles);

		ClearMultiDamage();

		pOther->TraceAttack(VARS(pev->owner), m_flDirectDamage, gpGlobals->v_forward, ptr, DMG_BULLET | DMG_NEVERGIB);

		ApplyMultiDamage(pev, VARS(pev->owner));
	}

	m_TargetVector.push_back(pOther->entindex());
}

bool CGuillotineExAmmo::IsHeadCutting(void)
{
	return m_iHeadCutCounter > 0;
}

bool CGuillotineExAmmo::ShouldHeadCut(TraceResult* ptr, CBaseEntity* pOther, Vector vecAbsMins, Vector vecAbsMaxs)
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

	if (m_iType)
	{
		m_hHeadCutting = pOther;
		return true;
	}
		

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

bool CGuillotineExAmmo::CanAttack(CBaseEntity* pOther)
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

void CGuillotineExAmmo::MaterialSound(TraceResult* ptr)
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

bool CGuillotineExAmmo::RangeAttack(TraceResult* ptr, Vector vecTraceDelta)
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

void CGuillotineExAmmo::Return(bool bResetFrame)
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

	SetTouch(&CGuillotineExAmmo::ReturnTouch);
	SetThink(&CGuillotineExAmmo::ReturnThink);

	pev->nextthink = gpGlobals->time;

	SetAnimation(GUILLOTINEAMMO_IDLE, bResetFrame);
}

void CGuillotineExAmmo::ReturnThink(void)
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

void CGuillotineExAmmo::ReturnTouch(CBaseEntity* pOther)
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

	CGuillotineex* pWeapon = (CGuillotineex*)((CBasePlayer*)pOther)->m_pActiveItem;

	if (pWeapon && pWeapon->m_iId == WEAPON_GUILLOTINEEX)
	{
		pWeapon->CatchAmmo();

		pev->velocity = g_vecZero;

		SetTouch(NULL);
		SetThink(NULL);

		UTIL_Remove(pBeam);
		UTIL_Remove(this);

		return;
	}

	Crash();

	return;
}

}
