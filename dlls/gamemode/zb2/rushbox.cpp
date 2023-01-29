#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "client.h"
#include "cbase/cbase_hash.h"

#include "rushbox.h"
#include "gamemode/mod_zb2.h"
#include "vector.h"

namespace sv {

LINK_ENTITY_TO_CLASS(zombierushbox, CZombieRushBox);

void CZombieRushBox::Spawn(void)
{
	Precache();

	if (pev->classname)
	{
		RemoveEntityHashValue(pev, STRING(pev->classname), CLASSNAME);
	}

	MAKE_STRING_CLASS("zombierushbox", pev);
	AddEntityHashValue(pev, STRING(pev->classname), CLASSNAME);

	SetThink(&CZombieRushBox::BoxThink);

	m_nPullingPlayerIndex.clear();
	m_nOwnerIndex = 0;

	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_FOLLOW;
	pev->aiment = pev->owner;

	UTIL_SetSize(pev, Vector(-12.0, -12.0, -36.0), Vector(12.0, 12.0, 36.0));

	pev->nextthink = gpGlobals->time + 0.1s;
	m_flPullingSpeed = 370;
	m_iState = 0;

	EMIT_SOUND_DYN(pev->owner, CHAN_ITEM, "zombi/meatwallzombie_skill_dash_hold.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
}

void CZombieRushBox::Precache(void)
{
	PRECACHE_SOUND("zombi/meatwallzombie_skill_dash_finish.wav");
	PRECACHE_SOUND("zombi/meatwallzombie_skill_dash_hold.wav");
	m_iWaveModelIndex = PRECACHE_MODEL("models/ef_meatwall_wave.mdl");
	m_iGibsModelIndex = PRECACHE_MODEL("models/meatwall_gibs.mdl");
}

void EXPORT CZombieRushBox::BoxThink()
{
	pev->nextthink = gpGlobals->time + 0.1s;

	CBasePlayer* pOwner = (CBasePlayer*)UTIL_PlayerByIndex(m_nOwnerIndex);

	if (!pOwner)
	{
		SetThink(NULL);
		UTIL_Remove(this);;
		return;
	}

	if (m_iState && m_fKillDelay < gpGlobals->time)
	{
		DoDamage();
		SetThink(NULL);
		UTIL_Remove(this);
		return;
	}
	
	TraceResult tr;
	CBaseEntity* pList[100];
	Vector vecStart;

	vecStart = pev->origin + Vector(0.0, 0.0, -1.0);

	UTIL_MakeVectors(pOwner->pev->v_angle);

	Vector vecForward = gpGlobals->v_forward;

	Vector2D vecAngleCheckDir = gpGlobals->v_forward.Make2D().Normalize();

	float angCheck = cos(90.0 * 0.5 * M_PI / 180.0);
	const float flRange = 1.0 * 39.37;
	int iCount = UTIL_EntitiesInBox(pList, 100, vecStart - Vector(flRange, flRange, flRange), vecStart + Vector(flRange, flRange, flRange), FL_CLIENT | FL_MONSTER);

	for (int i = 0; i < iCount; i++)
	{
		CBaseEntity* pEntity = pList[i];

		if (!pEntity->IsPlayer())
			continue;

		if (pEntity->edict() == pev->owner)
			continue;

		if (DotProduct((pEntity->pev->origin.Make2D() - vecStart.Make2D()).Normalize(), vecAngleCheckDir) < angCheck)
			continue;

		if (g_pGameRules->PlayerRelationship(pOwner, pEntity) == GR_TEAMMATE)
			continue;

		if (std::find(m_nPullingPlayerIndex.begin(), m_nPullingPlayerIndex.end(), pEntity->entindex()) != m_nPullingPlayerIndex.end())
			continue;

		UTIL_TraceLine(vecStart, pEntity->BodyTarget(vecStart), dont_ignore_monsters, edict(), &tr);

		if (tr.flFraction < 1.0 && tr.pHit != pEntity->edict())
			continue;

		m_nPullingPlayerIndex.push_back(pEntity->entindex());
	}

	if (m_nPullingPlayerIndex.empty())
		return;

	if (!m_iState)
	{
		if (pOwner && pOwner->pev->deadflag == DEAD_NO)
		{
			int iSkillLoopSeq = pOwner->LookupSequence("skill_dash_hold");

			if (pOwner->pev->sequence != iSkillLoopSeq)
			{
				pOwner->m_iCustomSequence = iSkillLoopSeq;
				pOwner->m_flCustomSequenceUntilTime = gpGlobals->time + 8.0s;

				pOwner->SetAnimation(PLAYER_IDLE);

				CBasePlayerWeapon* pWeapon = (CBasePlayerWeapon*)pOwner->m_pActiveItem;

				if (pWeapon)
				{
					pWeapon->SendWeaponAnim(11, TRUE);
					pWeapon->m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 8.0s;
					pWeapon->m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 8.0s;
					pWeapon->m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 8.0s;
				}

			}
		}
	}

	for (auto iter = m_nPullingPlayerIndex.begin(); iter != m_nPullingPlayerIndex.end(); iter++)
	{
		CBasePlayer* pPlayer = (CBasePlayer*)UTIL_PlayerByIndex(*iter);
		Vector vecDelta = pPlayer->pev->origin - pOwner->pev->origin;
		Vector vecDis = pOwner->GetGunPosition() + vecForward * 70;
		vecDelta = vecDis - pPlayer->pev->origin;

		//pPlayer->pev->velocity.x += pOwner->pev->velocity.x;
		//pPlayer->pev->velocity.y += pOwner->pev->velocity.y;

		pPlayer->pev->velocity = vecDelta.Normalize() * m_flPullingSpeed;

		float length = pPlayer->pev->velocity.Length2D();

		if (length > m_flPullingSpeed)
		{
			pPlayer->pev->velocity.x = (pPlayer->pev->velocity.x * m_flPullingSpeed / length);
			pPlayer->pev->velocity.y = (pPlayer->pev->velocity.y * m_flPullingSpeed / length);
		}

		pOwner->pev->velocity.x = pPlayer->pev->velocity.x;
		pOwner->pev->velocity.y = pPlayer->pev->velocity.y;
	}
}

void CZombieRushBox::Kill(void)
{
	m_iState = 1;
	m_fKillDelay = gpGlobals->time + 0.5s;

	CBasePlayer* pPlayer = (CBasePlayer*)UTIL_PlayerByIndex(m_nOwnerIndex);

	if (pPlayer)
	{
	
		pPlayer->m_flCustomSequenceUntilTime = invalid_time_point;

		pPlayer->ResetMaxSpeed();
		pPlayer->SendFOV(90);

		CLIENT_COMMAND(pPlayer->edict(), "-forward\n");

		pPlayer->pev->velocity = g_vecZero;

		CBasePlayerWeapon* pWeapon = (CBasePlayerWeapon*)pPlayer->m_pActiveItem;

		if (pWeapon)
		{
			pWeapon->SendWeaponAnim(12, TRUE);
			pWeapon->m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.3s;
			pWeapon->m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.3s;
			pWeapon->m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.3s;
		}

		pPlayer->m_iCustomSequence = pPlayer->LookupSequence("skill_dash_finish");
		pPlayer->m_flCustomSequenceUntilTime = gpGlobals->time + 1.1s;

		pPlayer->SetAnimation(PLAYER_IDLE);

		EMIT_SOUND_DYN(pPlayer->edict(), CHAN_ITEM, "zombi/meatwallzombie_skill_dash_finish.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
	}

}

void CZombieRushBox::DoDamage()
{
	CBasePlayer* pOwner = (CBasePlayer*)UTIL_PlayerByIndex(m_nOwnerIndex);

	UTIL_MakeVectors(pOwner->pev->v_angle);

	Vector vecForward = gpGlobals->v_forward;

	Vector vecOrigin = pOwner->pev->origin + vecForward * 45;

	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, vecOrigin);
	WRITE_BYTE(TE_BREAKMODEL);
	WRITE_COORD(vecOrigin.x);
	WRITE_COORD(vecOrigin.y);
	WRITE_COORD(vecOrigin.z);
	WRITE_COORD(pev->size.x);
	WRITE_COORD(pev->size.y);
	WRITE_COORD(pev->size.z);
	WRITE_COORD(0.0);
	WRITE_COORD(0.0);
	WRITE_COORD(0.0);
	WRITE_BYTE(10);
	WRITE_SHORT(m_iGibsModelIndex);
	WRITE_BYTE(30);
	WRITE_BYTE(25);
	WRITE_BYTE(BREAK_WOOD);
	MESSAGE_END();

	UTIL_TempModel({ 0, 0, -16 }, g_vecZero, g_vecZero, m_iWaveModelIndex, 1, 0, 10, 0, 0, 255, kRenderTransAdd, pOwner, false, 0, 0, 256, 0);

	for (auto iter = m_nPullingPlayerIndex.begin(); iter != m_nPullingPlayerIndex.end(); iter++)
	{
		CBasePlayer* pPlayer = (CBasePlayer*)UTIL_PlayerByIndex(*iter);
		pPlayer->pev->velocity.z += 300.0f;
		pPlayer->TakeDamage(pOwner->pev, pOwner->pev, 250.0f, DMG_EXPLOSION | DMG_NEVERGIB);
	}
}

void CZombieRushBox::SetLifespan(duration_t flLifespawn)
{
	m_fKillTime = gpGlobals->time + flLifespawn;
}

time_point_t CZombieRushBox::GetLifespan()
{
	return m_fKillTime;
}


int CZombieRushBox::GetSkillState()
{
	return m_iState;
}

void CZombieRushBox::Init(void)
{
	m_nOwnerIndex = ENTINDEX(pev->owner);

	CBasePlayer* pPlayer = (CBasePlayer*)UTIL_PlayerByIndex(m_nOwnerIndex);

	if (!pPlayer)
		return;

	pPlayer->ResetMaxSpeed();
	pPlayer->SendFOV(120);
}


}