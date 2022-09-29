#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

#include "gamemode/mods.h"
#include "gamemode/mod_zb1.h"

namespace sv {

	LINK_ENTITY_TO_CLASS(wondercannon_mine, CWonderCannonMine)
#define WONDERCANNON_MINE_EXP_RADIUS	39.37 * 5
#define WONDERCANNONEX_MINE_EXP_RADIUS	39.37 * 6.5
	CWonderCannonMine* CWonderCannonMine::Create(int iType, const Vector& vecOrigin, const Vector& vecAngles, edict_t* pentOwner)
	{
		edict_t* pent = CREATE_NAMED_ENTITY(MAKE_STRING("wondercannon_mine"));

		if (FNullEnt(pent))
		{
			ALERT(at_console, "NULL Ent in Create!\n");
			return NULL;
		}

		CWonderCannonMine* pMine = (CWonderCannonMine*)Instance(pent);

		if (pMine)
		{
			pMine->m_iType = iType;
			pMine->pev->owner = pentOwner;
			pMine->pev->origin = vecOrigin;
			pMine->pev->angles = vecAngles;
			pMine->Spawn();
		}

		return pMine;
	}

	void CWonderCannonMine::Spawn(void)
	{
		Precache();

		SET_MODEL(ENT(pev), m_iType ? "models/bomb_wondercannonex.mdl" : "models/bomb_wondercannon.mdl");
		UTIL_SetOrigin(pev, pev->origin);

		pev->angles = g_vecZero;
		pev->gravity = 2.5;
		pev->solid = SOLID_BBOX;
		pev->movetype = MOVETYPE_TOSS;
		pev->frame = 0;

		pev->classname = MAKE_STRING("wondercannon_mine");
		pev->nextthink = gpGlobals->time + 0.01s;
		SetThink(&CWonderCannonMine::MineThink);
	}

	void CWonderCannonMine::Init(CBasePlayer* pOwner, CWonderCannon* pWeapon)
	{
		m_pOwner = pOwner;
		m_pWeapon = pWeapon;
		m_iTeam = m_pOwner->m_iTeam;
	}

	void CWonderCannonMine::Precache(void)
	{
		PRECACHE_MODEL("sprites/ef_wondercannon_bomb_set.spr");
		PRECACHE_MODEL("sprites/ef_wondercannonex_bomb_set.spr");

		PRECACHE_MODEL("models/ef_wondercannon_area.mdl");
		PRECACHE_MODEL("models/bomb_wondercannon.mdl");

		PRECACHE_MODEL("models/ef_wondercannonex_area.mdl");
		PRECACHE_MODEL("models/bomb_wondercannonex.mdl");

		if (m_iType)
		{
			m_iExp[0] = PRECACHE_MODEL("sprites/ef_wondercannonex_hit3.spr");
			m_iExp[1] = PRECACHE_MODEL("sprites/ef_wondercannonex_bomb_set.spr");
		}
		else
		{
			m_iExp[0] = PRECACHE_MODEL("sprites/ef_wondercannon_hit3.spr");
			m_iExp[1] = PRECACHE_MODEL("sprites/ef_wondercannon_bomb_ex.spr");
		}
		
		PRECACHE_SOUND("weapons/wondercannon_comd_drop.wav");
		PRECACHE_SOUND("weapons/wondercannon_comd_exp.wav");
		PRECACHE_SOUND("weapons/wondercannonex_cmod_exp.wav");
	}

	void CWonderCannonMine::Remove()
	{
		MESSAGE_BEGIN(MSG_ALL, SVC_TEMPENTITY, pev->origin);
		WRITE_BYTE(TE_KILLENTITYATTACHMENTS);
		WRITE_SHORT(entindex());
		MESSAGE_END();

		SetThink(nullptr);
		pev->effects |= EF_NODRAW; // 0x80u
		return UTIL_Remove(this);
	}

	void CWonderCannonMine::Explode(bool IsManual)
	{
		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_TEMPSPRITE);
		WRITE_COORD(pev->origin.x);
		WRITE_COORD(pev->origin.y);
		WRITE_COORD(pev->origin.z);
		WRITE_SHORT(m_iExp[0]);
		WRITE_BYTE(10);
		WRITE_BYTE(255);
		WRITE_BYTE(30);
		MESSAGE_END();

		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_TEMPSPRITE);
		WRITE_COORD(pev->origin.x);
		WRITE_COORD(pev->origin.y);
		WRITE_COORD(pev->origin.z + 5.0);
		WRITE_SHORT(m_iExp[1]);
		WRITE_BYTE(5);
		WRITE_BYTE(255);
		WRITE_BYTE(30);
		MESSAGE_END();

		if(m_iType)
			EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/wondercannonex_cmod_exp.wav", VOL_NORM, ATTN_NORM);
		else
			EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/wondercannon_comd_exp.wav", VOL_NORM, ATTN_NORM);

		DoBombExp(IsManual);

		Remove();
	}
	void CWonderCannonMine::DoBombExp(bool IsManual)
	{
		CBaseEntity* pEntity = NULL;
		float flRadius = m_iType ? WONDERCANNONEX_MINE_EXP_RADIUS : WONDERCANNON_MINE_EXP_RADIUS;
		while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, flRadius)) != NULL)
		{
			if (pEntity->IsDormant())
				continue;

			if (pEntity->pev->takedamage == DAMAGE_NO)
				continue;

			if (pEntity == m_pOwner)
			{
				if (IsManual && m_iType)
				{
					if ((pEntity->pev->origin - pev->origin).Length() < 39.37 * 2.2)
					{
						Vector2D vecVelocity = pEntity->pev->velocity.Make2D();
						float flSpeed = vecVelocity.Length();

						vecVelocity = vecVelocity.Normalize();
						pEntity->pev->velocity.x = vecVelocity.x * flSpeed * 2.5;
						pEntity->pev->velocity.y = vecVelocity.y * flSpeed * 2.5;
						pEntity->pev->velocity.z = 600;
					}
					
				}	
				continue;
			}

			if (g_pGameRules->PlayerRelationship(m_pOwner, pEntity) == GR_TEAMMATE)
				continue;

			pEntity->TakeDamage(pev, m_pOwner->pev, GetDamage(IsManual), DMG_NEVERGIB | DMG_EXPLOSION | DMG_NOMORALE);

			if (pEntity->IsPlayer())
			{
				CBasePlayer* player = (CBasePlayer*)pEntity;

				if (player->m_bIsZombie)
				{
					auto strategy = dynamic_cast<CPlayerModStrategy_ZB1*>(player->m_pModStrategy.get());
					if (strategy)
					{
						strategy->SetStunSpeedTime(3.5s, 55.0);
						strategy->SetStunGravityTime(3.5s);
					}
				}
			}
		}
	}

	float CWonderCannonMine::GetDamage(bool IsManual) const
	{
		float flDamage = 1.0f;
		if (m_iType)
		{
			flDamage = IsManual ? 110.0f : 100.0f;
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = IsManual ? 29200.0f : 19200.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = IsManual ? 40000.0f : 25000.0f;
		}
		else
		{
			flDamage = IsManual ? 110.0f : 100.0f;
			if (g_pModRunning->DamageTrack() == DT_ZB)
				//flDamage = IsManual ? 28600.0f : 18600.0f;
				flDamage = IsManual ? 16900.0f : 10900.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = IsManual ? 39400.0f : 24400.0f;
		}
		return flDamage;
	}

	void EXPORT CWonderCannonMine::MineThink(void)
	{
		pev->nextthink = gpGlobals->time + 0.01s;

		if (!m_pOwner)
		{
			Remove();
			return;
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

		if (pev->flags & FL_ONGROUND)
		{
			if (m_iState == 2)
			{
				CBaseEntity* pEntity = NULL;
				if (!m_iType)
				{
					while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, 39.37 * 1.5)) != NULL)
					{
						//a lazy way
						if (pEntity != m_pOwner)
							continue;

						Vector2D vecVelocity = pEntity->pev->velocity.Make2D();
						float flSpeed = vecVelocity.Length();
						vecVelocity = vecVelocity.Normalize();
						pEntity->pev->velocity.x = vecVelocity.x * flSpeed * 2;
						pEntity->pev->velocity.y = vecVelocity.y * flSpeed * 2;
						pEntity->pev->velocity.z = 400;

						Explode(false);
						return;
					}
				}

				while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, 39.37 * 2.8)) != NULL)
				{
					if (pEntity->IsDormant())
						continue;

					if (pEntity->pev->takedamage == DAMAGE_NO)
						continue;

					if (pEntity == m_pOwner)
						continue;

					if (g_pGameRules->PlayerRelationship(m_pOwner, pEntity) == GR_TEAMMATE)
						continue;

					Explode(false);

				}
			}

			if (m_flNextAnim != invalid_time_point && m_flNextAnim < gpGlobals->time)
			{
				pev->sequence = 1;
				//play model & spr

				MESSAGE_BEGIN(MSG_ALL, gmsgMPToCL, NULL, this->pev);
				WRITE_BYTE(18);
				WRITE_BYTE(m_iType ? 2 : 0);
				WRITE_SHORT(this->entindex());
				MESSAGE_END();

				m_iState = 2;
				m_flNextAnim = invalid_time_point;
			}

			if (!m_iState)
			{
				pev->sequence = 0;
				pev->frame = 0;
				ResetSequenceInfo();

				MESSAGE_BEGIN(MSG_ALL, gmsgMPToCL, NULL, this->pev);
				WRITE_BYTE(18);
				WRITE_BYTE(m_iType ? 3 : 1);
				WRITE_SHORT(this->entindex());
				MESSAGE_END();

				EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/wondercannon_comd_drop.wav", VOL_NORM, ATTN_NORM);

				m_flNextAnim = gpGlobals->time + 0.5s;

				m_iState = 1;
			}
		}
	}
}
