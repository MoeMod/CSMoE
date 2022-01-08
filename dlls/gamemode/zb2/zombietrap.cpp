#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "client.h"
#include "cbase/cbase_hash.h"
#include "gamemode/mod_zb2.h"

#include "zombietrap.h"
#include "gamemode/mod_zb2.h"

namespace sv {

LINK_ENTITY_TO_CLASS(zombietrap, CZombieTrap);

void CZombieTrap::Spawn(void)
{
	Precache();

	if (pev->classname)
	{
		RemoveEntityHashValue(pev, STRING(pev->classname), CLASSNAME);
	}

	MAKE_STRING_CLASS("zombietrap", pev);
	AddEntityHashValue(pev, STRING(pev->classname), CLASSNAME);

	pev->movetype = MOVETYPE_PUSHSTEP;
	pev->solid = SOLID_TRIGGER;
	m_flRemoveTime = gpGlobals->time + GetAliveTime();
	pev->nextthink = gpGlobals->time + 0.2s;

	SET_MODEL(ENT(pev), "models/zombitrap.mdl");
	UTIL_SetSize(pev, Vector(-4, -4, -4), Vector(4, 4, 4));

	SetTouch(&CZombieTrap::OnTouch);
	SetThink(&CZombieTrap::OnThink);
}

void CZombieTrap::Precache(void)
{
	PRECACHE_MODEL("models/zombitrap.mdl");
	PRECACHE_SOUND("zombi/zombi_trapped.wav");
	PRECACHE_SOUND("zombi/zombi_trapped_female.wav");
	
}

void EXPORT CZombieTrap::OnThink()
{
	if (m_flRemoveTime <= gpGlobals->time)
	{
		if (m_hTarget && m_hTarget->IsAlive() && !m_hTarget->m_bIsZombie)
		{
			m_hTarget->ResetMaxSpeed();
		}
		m_hTarget = nullptr;
		SetThink(NULL);
		SetTouch(NULL);
		UTIL_Remove(this);

		return;
	}

	pev->nextthink = gpGlobals->time + 0.2s;

	if (m_hTarget)
		return;

	if (!(pev->flags & FL_ONGROUND))
		return;

	if (!m_hFollowTarget || !m_hFollowTarget->IsAlive() || m_hFollowTarget->m_bIsZombie)
		m_hFollowTarget = GetClosestPlayer();

	if (!m_hFollowTarget)
		return;

	Vector vecVelocity = (m_hFollowTarget->pev->origin - pev->origin).Normalize() * GetMoveSpeed();
	vecVelocity[2] = 0.0f;
	pev->velocity = vecVelocity;
}

void EXPORT CZombieTrap::OnTouch(CBaseEntity* pOther)
{
	if (!pOther->IsPlayer())
		return;

	if (m_hTarget != nullptr)
	{
		if (!m_hTarget->IsAlive() || m_hTarget->m_bIsZombie)
		{
			m_flRemoveTime = gpGlobals->time;
		}
		else
		{
			m_hTarget->pev->velocity = Vector(0, 0, -200);
		}

		if (pev->frame < 230.0) pev->frame += 0.5;
		else pev->frame = 14.0;
		return;
	}

	CBasePlayer* p = static_cast<CBasePlayer*>(pOther);

	if (p->m_bIsZombie)
		return;

	m_hTarget = p;
	pev->origin.x = p->pev->origin.x;
	pev->origin.y = p->pev->origin.y;
	pev->sequence = 1;
	pev->frame = 0.0;
	m_flRemoveTime = gpGlobals->time + GetTrappedTime();

	MESSAGE_BEGIN(MSG_ONE_UNRELIABLE, gmsgShake, NULL, pOther->pev);
	WRITE_SHORT((1 << 14));
	WRITE_SHORT((1 << 14));
	WRITE_SHORT((1 << 14));
	MESSAGE_END();

	EMIT_SOUND(ENT(p->pev), CHAN_BODY, p->m_bIsFemale ? "zombi/zombi_trapped_female.wav" : "zombi/zombi_trapped.wav", VOL_NORM, ATTN_NORM);

	auto strategy = dynamic_cast<CPlayerModStrategy_ZB2*>(p->m_pModStrategy.get());
	if (strategy)
	{
		strategy->SetTrappedTime(GetTrappedTime());
	}

	CBasePlayer* owner = dynamic_cast<CBasePlayer*>(CBasePlayer::Instance(pev->owner));
	if (!owner)
		return;

	ClientPrint(owner->pev, HUD_PRINTCENTER, "你的陷阱抓住了人类!");
}

CBasePlayer* CZombieTrap::GetClosestPlayer() const
{
	CBasePlayer* close = NULL;
	float closeRangeSq = 1e8f;

	for (int i = 1; i <= gpGlobals->maxClients; ++i)
	{
		CBasePlayer* player = static_cast<CBasePlayer*>(UTIL_PlayerByIndex(i));

		if (player == NULL)
			continue;

		if (FNullEnt(player->pev))
			continue;

		if (FStrEq(STRING(player->pev->netname), ""))
			continue;

		if (!player->IsAlive())
			continue;

		if (player->m_bIsZombie)
			continue;

		float rangeSq = (pev->origin - player->pev->origin).LengthSquared();

		if (rangeSq < closeRangeSq)
		{
			closeRangeSq = rangeSq;
			close = player;
		}
	}

	return close;
}

}
