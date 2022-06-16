#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "wpn_shared/wpn_cannonex.h"
#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif

namespace sv {

	LINK_ENTITY_TO_CLASS(cannonex_dragon, CCannonEXDragon)

	class CCannonEX;

		enum cannonexdragon_effect_state_e
	{
		EFFECT_NONE,
		EFFECT_FADEIN,
		EFFECT_FADEOUT
	};

	void CCannonEXDragon::ApplyDamageForce(CBaseEntity* pOther)
	{
		if (!pOther)
			return;

		if (!m_pOwner)
			return;

		if (!pOther->IsPlayer())
			return;

		if (pOther->IsPlayer() && !((CBasePlayer*)pOther)->m_bIsZombie)
			return;

		if (g_pGameRules->PlayerRelationship(m_pOwner, pOther) == GR_TEAMMATE)
			return;

		const float force = GetKickRateMagnitude();

		UTIL_AddKickRateBoost(pOther, (pOther->pev->origin.Make2D() - m_pOwner->pev->origin.Make2D()).Normalize(), 1.0, force, 0.0);
		//((CBasePlayer*)pOther)->m_flVelocityModifier = 0.25; +6
	}

	void CCannonEXDragon::CCannonEXDragonThink(void)
	{
		pev->nextthink = gpGlobals->time + 0.01s;

		DispatchAnimEvents(StudioFrameAdvance());

		if (m_pOwner)
		{
			UTIL_SetOrigin(pev, m_pOwner->pev->origin + Vector(0, 0, m_flDragonHeight));

			pev->angles = m_pOwner->pev->angles;
			pev->angles.x = 0;
		}

		if (m_iEffectState != EFFECT_FADEOUT && m_pOwner && (!m_pOwner->IsAlive() || m_pOwner->m_bIsZombie))
		{
			RemoveDragon();
		}

		if (gpGlobals->time > m_flAttackTime)
		{
			Attack();

			m_flAttackTime = gpGlobals->time + m_flAttackInterval;
		}

		if (gpGlobals->time > m_flSoundTime)
		{
			if (m_iEffectState != EFFECT_FADEOUT)
			{
				EMIT_SOUND_DYN(edict(), CHAN_BODY,
					"weapons/flamegun-1.wav",
					VOL_NORM,
					0.52,
					0,
					(94 + RANDOM_LONG(0, 15)));
			}

			m_flSoundTime = gpGlobals->time + m_flSoundInterval;
		}

		if (m_iEffectState == EFFECT_FADEIN)
			EF_Appear();

		if (m_iEffectState == EFFECT_FADEOUT)
			EF_Hide();
	}

	void CCannonEXDragon::DragonInit(CBasePlayer* pOwner, CCannonEX* pWeapon)
	{
		m_pOwner = pOwner;
		m_pWeapon = pWeapon;

		m_flAttackTime = invalid_time_point;
		m_flEffectTime = gpGlobals->time;
		m_iEffectState = EFFECT_FADEIN;

		if (pOwner)
		{
			UTIL_SetOrigin(pev, pOwner->pev->origin + Vector(0, 0, m_flDragonHeight));

			pev->angles = pOwner->pev->angles;
			pev->angles.x = 0;
		}
	}

	void CCannonEXDragon::Attack(void)
	{
		Vector vecOrigin, vecAngles;

		vecAngles = pev->angles;
		vecAngles.x += -20.0;

		UTIL_MakeVectors(vecAngles);

		Vector vecForward = gpGlobals->v_forward;
		vecForward.z *= -1.0;

		GetAttachment(0, vecOrigin, vecAngles);

		auto pTargetSet = new std::set<CBaseEntity*>;

		UTIL_MakeVectors(pev->angles);

		DragonFire(vecOrigin, gpGlobals->v_up, vecForward, pTargetSet);
		DragonFire(vecOrigin, -gpGlobals->v_up, vecForward, pTargetSet);
		DragonFire(vecOrigin, -gpGlobals->v_right, vecForward, pTargetSet);
		DragonFire(vecOrigin, gpGlobals->v_right, vecForward, pTargetSet);

		//create flame_puff
		PLAYBACK_EVENT_FULL(FEV_GLOBAL, ENT(pev), m_usFireCannonEx, 0.0, g_vecZero, g_vecZero, 0.0, 0.0, 4, 0, TRUE, FALSE);

		delete pTargetSet;
	}

	void CCannonEXDragon::DragonFire(Vector vecSrc, Vector vecOffset, Vector vecDir, std::set<CBaseEntity*>* pSet)
	{
		Vector vecStart, vecEnd;
		CBaseEntity* pEntity;
		TraceResult tr;

		if (!m_pOwner)
			return;

		vecStart = vecSrc + vecOffset.Normalize() * m_flFireOffsetStrength;
		vecEnd = vecStart + vecDir * m_flAttackRange;

		for (int i = 1; i <= 32; i++)
		{
			UTIL_TraceLine(vecStart, vecEnd, dont_ignore_monsters, m_pOwner->edict(), &tr);

			if (tr.iHitgroup == HITGROUP_HEAD)
				tr.iHitgroup = HITGROUP_CHEST;

			if (tr.flFraction >= 1.0)
				break;

			pEntity = CBaseEntity::Instance(tr.pHit);

			if (pEntity == m_pOwner)
				break;

			if (pSet->find(pEntity) != pSet->end())
				continue;

			ClearMultiDamage();

			pEntity->TraceAttack(m_pOwner->pev, GetDamageAmount(), vecDir, &tr, DMG_BULLET | DMG_BURN);

			ApplyMultiDamage(m_pOwner->pev, m_pOwner->pev);

			pSet->insert(pEntity);

			ApplyDamageForce(pEntity);

			if (!tr.pHit)
				break;

			if (pEntity->IsBSPModel())
				break;

			vecStart = vecStart + (vecEnd - vecStart) * tr.flFraction;
		}
	}

	float CCannonEXDragon::GetDamageAmount(void)
	{
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			return 150;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			return 600;
#endif
		return 5.0;
	}

	float CCannonEXDragon::GetKickRateMagnitude(void)
	{
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			return 300;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			return 0.0;
#endif
		return 0.0;
	}

	void CCannonEXDragon::InitializeData(void)
	{
		m_flDragonHeight = 35.0;
		m_flFadeInTime = 0.5s;
		m_flFadeOutTime = 0.5s;
		m_flDamage = GetDamageAmount();
		m_flAttackInterval = 0.09s;
		m_flAttackRange = 500.0;
		m_flFireOffsetStrength = 20.0;
		m_flFlameDirectionOffset = -20.0;
		m_flFlameInterval = 0.07s;
		m_flFlameVelocity = 250.0;
		m_flForceMagnitude = GetKickRateMagnitude();
		m_flSoundInterval = 0.9s;
	}

	void CCannonEXDragon::EF_Appear(void)
	{
		duration_t interval = gpGlobals->time - m_flEffectTime;

		if (interval > m_flFadeInTime)
		{
			m_flEffectTime = invalid_time_point;
			m_iEffectState = EFFECT_NONE;

			pev->rendermode = kRenderNormal;
			pev->renderfx = kRenderFxNone;
			pev->renderamt = 0;
		}
		else
		{
			pev->rendermode = kRenderTransAlpha;
			pev->renderamt = (interval / m_flFadeInTime) * 255.0;
		}
	}

	void CCannonEXDragon::EF_Hide(void)
	{
		duration_t interval = gpGlobals->time - m_flEffectTime;

		if (interval > m_flFadeOutTime)
		{
			m_flEffectTime = invalid_time_point;
			m_iEffectState = EFFECT_NONE;

			pev->nextthink = invalid_time_point;
			UTIL_Remove(this);
		}
		else
		{
			pev->rendermode = kRenderTransAlpha;
			pev->renderamt = (1.0 - interval / m_flFadeOutTime) * 255.0;
		}
	}

	void CCannonEXDragon::Precache(void)
	{
		PRECACHE_MODEL("models/cannonexdragon.mdl");
		//PRECACHE_MODEL("models/cannonexdragon_d3a3.mdl");

		PRECACHE_SOUND("weapons/cannonex_dragon_ice.wav");
		PRECACHE_SOUND("weapons/cannonex_dragon_ice_end.wav");
		PRECACHE_SOUND("weapons/cannonex_dragon_fire_end.wav");
		PRECACHE_SOUND("weapons/flamegun-1.wav");

		m_usFireCannonEx = PRECACHE_EVENT(1, "events/cannonex.sc");
	}

	void CCannonEXDragon::RemoveDragon(void)
	{
		if (m_pWeapon == NULL)
		{
			EMIT_SOUND_DYN(ENT(pev), CHAN_BODY, "weapons/cannonex_dragon_fire_end.wav", VOL_NORM, ATTN_NORM, 0, 94);
		}
		else
		{
			EMIT_SOUND_DYN(ENT(pev),
				CHAN_BODY,
				"weapons/cannonex_dragon_fire_end.wav",
				VOL_NORM,
				ATTN_NORM,
				0,
				(94 + RANDOM_LONG(0, 15)));
		}

		m_flEffectTime = gpGlobals->time;
		m_iEffectState = EFFECT_FADEOUT;
	}

	void CCannonEXDragon::Spawn(void)
	{
		duration_t interval = 0.16s;

		Precache();
		pev->classname = MAKE_STRING("cannonex");
		pev->solid = SOLID_NOT;
		pev->movetype = MOVETYPE_NOCLIP;
		pev->takedamage = DAMAGE_NO;
		pev->rendermode = kRenderTransAlpha;
		pev->renderamt = 0;

		SET_MODEL(ENT(pev), "models/cannonexdragon.mdl");

		pev->frame = 0;
		pev->sequence = 0;

		ResetSequenceInfo();

		SetThink(&CCannonEXDragon::CCannonEXDragonThink);
		pev->nextthink = gpGlobals->time + interval;
	}

	void CCannonEXDragon::UpdateOnRemove(void)
	{
		if (m_pWeapon)
			m_pWeapon->m_pDragon = NULL;

		CBaseEntity::UpdateOnRemove();
	}

}
