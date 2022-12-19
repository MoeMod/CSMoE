#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "weapons/RadiusDamage.hpp"
#include "gamemode/mods.h"


namespace sv {

	LINK_ENTITY_TO_CLASS(voidpistol_blackhole, CVoidPistolBlackHole)

	CVoidPistolBlackHole* CVoidPistolBlackHole::Create(int iType, const Vector& vecOrigin, const Vector& vecAngles, edict_t* pentOwner, int iTeam)
	{
		edict_t* pent = CREATE_NAMED_ENTITY(MAKE_STRING("voidpistol_blackhole"));

		if (FNullEnt(pent))
		{
			ALERT(at_console, "NULL Ent in Create!\n");
			return NULL;
		}

		CVoidPistolBlackHole* pProjectile = (CVoidPistolBlackHole*)Instance(pent);

		if (pProjectile)
		{
			pProjectile->m_iType = iType;
			pProjectile->pev->owner = pentOwner;
			pProjectile->pev->origin = vecOrigin;
			pProjectile->pev->angles = vecAngles;
			pProjectile->m_iTeam = iTeam;
			pProjectile->Spawn();
		}

		return pProjectile;
	}

	void CVoidPistolBlackHole::Spawn()
	{
		Precache();

		SetTouch(&CVoidPistolBlackHole::FlyingTouch);
		SetThink(&CVoidPistolBlackHole::FlyingThink);

		pev->nextthink = gpGlobals->time + 0.1s;
		pev->solid = SOLID_BBOX;
		pev->movetype = MOVETYPE_FLY;
		pev->renderamt = 255;

		if (m_iType)
			SET_MODEL(ENT(pev), "models/ef_voidpistolex_projectile.mdl");
		else
			SET_MODEL(ENT(pev), "models/ef_blackhole_projectile.mdl");

		pev->frame = 0;
		pev->sequence = 0;

		ResetSequenceInfo();

		UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));
		m_flExplodeTime = gpGlobals->time + 4s;

		UTIL_TempModel(Vector(0, 0, 0), pev->angles, g_vecZero, m_iBlackHoleProjectileIndex[m_iType], 40, 0, 255, 0, 0, 255, kRenderTransAdd, this, false, 0, 5, 30, 0x00010000 | 0x00000100);
	}

	void CVoidPistolBlackHole::Precache()
	{
		PRECACHE_SOUND("weapons/voidpistol_blackhole_start.wav");
		PRECACHE_SOUND("weapons/voidpistol_blackhole_exp.wav");
		PRECACHE_SOUND("weapons/voidpistol_blackhole_idle.wav");
		PRECACHE_SOUND("weapons/voidpistol_blackhole_c_idle.wav");
		PRECACHE_SOUND("weapons/voidpistol_blackhole_c_idle2.wav");

		PRECACHE_MODEL("models/ef_blackhole_projectile.mdl");
		m_iBlackHoleProjectileIndex[0] = PRECACHE_MODEL("sprites/ef_blackhole_projectile.spr");	//30
		PRECACHE_MODEL("models/ef_blackhole.mdl");
		m_iBlackHoleStartIndex[0] = PRECACHE_MODEL("sprites/ef_blackhole_start.spr");	//45
		m_iBlackHoleLoopIndex[0] = PRECACHE_MODEL("sprites/ef_blackhole_loop.spr");	//35
		m_iBlackHoleEndIndex[0] = PRECACHE_MODEL("sprites/ef_blackhole_end.spr");	//46

		PRECACHE_SOUND("weapons/voidpistolex_blackhole_exp.wav");
		PRECACHE_SOUND("weapons/voidpistolex_blackhole_idle.wav");
		PRECACHE_MODEL("models/ef_voidpistolex_projectile.mdl");
		m_iBlackHoleProjectileIndex[1] = PRECACHE_MODEL("sprites/ef_voidpistolex_projectile.spr");
		PRECACHE_MODEL("models/ef_voidpistolex.mdl");
		m_iBlackHoleStartIndex[1] = PRECACHE_MODEL("sprites/ef_voidpistolex_start.spr");
		m_iBlackHoleLoopIndex[1] = PRECACHE_MODEL("sprites/ef_voidpistolex_loop.spr");
		m_iBlackHoleEndIndex[1] = PRECACHE_MODEL("sprites/ef_voidpistolex_end.spr");
	}

	int CVoidPistolBlackHole::GetWeaponsId(void) { return m_iType ? WEAPON_VOIDPISTOLEX : WEAPON_VOIDPISTOL; }


	void CVoidPistolBlackHole::FlyingThink()
	{
		pev->nextthink = gpGlobals->time + 0.1s;

		if (m_flExplodeTime < gpGlobals->time)
		{
			Explode();
		}

		if (!IsInWorld())
		{
			SetTouch(NULL);
			SetThink(NULL);

			UTIL_Remove(this);
			return;
		}
	}

	void CVoidPistolBlackHole::FlyingTouch(CBaseEntity* pOther)
	{
		if (!pOther)
			return;

		if (pOther->edict() == pev->owner)
			return;

		Vector vecDir = pev->velocity.Normalize();
		pev->origin -= vecDir * 40;
		SetTouch(NULL);
		Explode();
	}

	void CVoidPistolBlackHole::Explode()
	{
		pev->solid = SOLID_NOT;
		pev->movetype = MOVETYPE_NONE;

		MESSAGE_BEGIN(MSG_ALL, SVC_TEMPENTITY, pev->origin);
		WRITE_BYTE(TE_KILLENTITYATTACHMENTS);
		WRITE_SHORT(entindex());
		MESSAGE_END();

		if (m_iType)
		{
			SET_MODEL(ENT(pev), "models/ef_voidpistolex.mdl");
		}
		else
		{
			SET_MODEL(ENT(pev), "models/ef_blackhole.mdl");
		}

		pev->frame = 0;
		pev->sequence = 0;

		ResetSequenceInfo();

		SetThink(&CVoidPistolBlackHole::BlackHoleThink);

		m_bIsExp = false;
		pev->nextthink = gpGlobals->time + 1.5s;
		m_flExplodeTime = gpGlobals->time + 6.0s;
		m_flDamageTime = gpGlobals->time + 1.5s;

		EMIT_SOUND_DYN(ENT(this->pev), CHAN_ITEM, "weapons/voidpistol_blackhole_start.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_TEMPSPRITE);
		WRITE_COORD(pev->origin.x);
		WRITE_COORD(pev->origin.y);
		WRITE_COORD(pev->origin.z);
		WRITE_SHORT(m_iBlackHoleStartIndex[m_iType]);
		WRITE_BYTE(5);
		WRITE_BYTE(255);
		WRITE_BYTE(30);
		MESSAGE_END();
	}

	void CVoidPistolBlackHole::BlackHoleThink()
	{
		pev->nextthink = gpGlobals->time + 0.1s;

		if (m_flSoundTime != invalid_time_point && m_flSoundTime < gpGlobals->time)
		{
			EMIT_SOUND_DYN(ENT(this->pev), CHAN_ITEM, m_iType ? "weapons/voidpistolex_blackhole_idle.wav" : "weapons/voidpistol_blackhole_idle.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

			m_flSoundTime = gpGlobals->time + 1.0s;
		}

		if (!m_bIsExp && m_flExplodeTime < gpGlobals->time)
		{
			m_bIsExp = true;
			m_flDamageTime = gpGlobals->time + 1.0s;
		}
			
		if (!m_bIsExp && pev->sequence != 1)
		{
			EMIT_SOUND_DYN(ENT(this->pev), CHAN_ITEM, m_iType ? "weapons/voidpistolex_blackhole_idle.wav" : "weapons/voidpistol_blackhole_idle.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

			m_flSoundTime = gpGlobals->time + 1.0s;

			pev->frame = 0;
			pev->sequence = 1;

			UTIL_TempModel(pev->origin, g_vecZero, g_vecZero, m_iBlackHoleLoopIndex[m_iType], 50, 0, 255, false, 0, 255, kRenderTransAdd, NULL, false, 0, 5, 35, 0x00010000 | 0x00000100);
		}
		
		if (m_bIsExp && pev->sequence != 2)
		{
			pev->frame = 0;
			pev->sequence = 2;

			MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
			WRITE_BYTE(TE_TEMPSPRITE);
			WRITE_COORD(pev->origin.x);
			WRITE_COORD(pev->origin.y);
			WRITE_COORD(pev->origin.z);
			WRITE_SHORT(m_iBlackHoleEndIndex[m_iType]);
			WRITE_BYTE(5);
			WRITE_BYTE(255);
			WRITE_BYTE(30);
			MESSAGE_END();

			m_flSoundTime = invalid_time_point;

			EMIT_SOUND_DYN(ENT(this->pev), CHAN_ITEM, m_iType ? "weapons/voidpistolex_blackhole_exp.wav" : "weapons/voidpistol_blackhole_exp.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		}
			
		if (m_flDamageTime != invalid_time_point && gpGlobals->time > m_flDamageTime)
		{
			float flDamage = m_bIsExp ? GetExpDamage() : GetBlackHoleDamage();
			float flRadius = GetExpRadius();

			if(m_pOwner->IsAlive() && !m_pOwner->m_bIsZombie)
				RadiusDamage(m_bIsExp, flDamage, flRadius);

			if(!m_bIsExp)
				m_flDamageTime = gpGlobals->time + 0.3s;
			else
			{
				m_flDamageTime = invalid_time_point;
				pev->nextthink = gpGlobals->time + 0.5s;
				SetThink(&CVoidPistolBlackHole::Remove);
			}
		}
	}

	void CVoidPistolBlackHole::RadiusDamage(bool IsExp, float flDamage, float flRadius)
	{
		entvars_t* pevOwner = VARS(pev->owner);

		float flPullRadius = GetBlackHoleRadius();
		CBaseEntity* pEntity = NULL;
		TraceResult tr;
		Vector vecOrigin = pev->origin;

		while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, flPullRadius)) != NULL)
		{
			if (!IsExp && !(pEntity->IsPlayer() || pEntity->Classify() == CLASS_PLAYER_ALLY))
				continue;

			if (m_iTeam == pEntity->m_iTeam)
				continue;

			if (pEntity->IsDormant())
				continue;

			if (pEntity->pev->takedamage == DAMAGE_NO)
				continue;

			if (pEntity->pev == pevOwner)
				continue;

			UTIL_TraceLine(pev->origin, pEntity->pev->origin , missile, pev->owner, &tr);

			if (tr.flFraction < 1.0 && tr.pHit != pEntity->edict())
				continue;
	
	
			Vector vecDelta = pEntity->pev->origin - vecOrigin;
		
			if (vecDelta.IsLengthLessThan(flRadius))
			{
				pEntity->m_LastHitGroup = HITGROUP_CHEST;
				pEntity->TakeDamage(pev, pevOwner, flDamage, DMG_BULLET | DMG_NEVERGIB);
				if (IsExp)
				{
					UTIL_AddKickRateBoost(pEntity, vecDelta.Make2D(), 1.0, 1000.0, 0);
				}
			}
				
			if (!IsExp)
			{	
				Vector vecPullVelocity;
				const float power = 400;
				vecOrigin[0] += RANDOM_LONG(-20, 20);
				vecOrigin[1] += RANDOM_LONG(-15, 15);
				vecOrigin[2] += RANDOM_LONG(-10, 10);


				vecPullVelocity = (vecOrigin - pEntity->pev->origin).Normalize() * power;
				pEntity->pev->velocity = vecPullVelocity;
			}
		}
	}

	void CVoidPistolBlackHole::Remove()
	{
		SetThink(NULL);

		UTIL_Remove(this);
	}

	float CVoidPistolBlackHole::GetExpDamage()
	{
		float flDamage = 30.0f;
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 1000.0f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 3000.0f;
		return flDamage;
	}

	float CVoidPistolBlackHole::GetBlackHoleDamage()
	{
		float flDamage = 6.0f;
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 110.0f;
		//flDamage = 500.0f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 1500.0f;
		return flDamage;
	}

	float CVoidPistolBlackHole::GetExpRadius()
	{
		float flDamage = 5.0f;
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 5.0f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 5.0f;
		return flDamage * 39.37;
	}

	float CVoidPistolBlackHole::GetBlackHoleRadius()
	{
		float flDamage = 10.8f;
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 10.8f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 12.0f;
		return flDamage * 39.37;
	}
}
