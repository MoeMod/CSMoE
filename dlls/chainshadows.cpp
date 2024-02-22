#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "weapons/RadiusDamage.hpp"
#include "gamemode/mods.h"
#include "gamemode/mod_zb1.h"
#include "decals.h"

namespace sv {

	//static constexpr const auto & ShadowModelLifeTime = 0.32s;
	static constexpr const auto& ShadowModelFadeInTime = 0.32s;
	static constexpr const auto& ShadowModelFadeOutTime = 0.32s;

	//static constexpr float ShadowModelFadeInStartAlpha = 10.0;
	static constexpr float ShadowModelBaseAlpha = 160.0;
	//static constexpr float ShadowModelFadeOutEndAlpha = 10.0;

	static constexpr float ShadowKnockbackGround = 0.0;
	static constexpr float ShadowKnockbackJump = 0.0;
	static constexpr float ShadowKnockbackJumpForward = 0.0;
	static constexpr float ShadowKnockbackDucking = 0.0;
	static constexpr float ShadowVelocityModifier = 0.0;

	static constexpr float ShadowChainMGDamage = 3.0;
	static constexpr float ShadowChainMGDamageZombie = 200.0;
	static constexpr float ShadowChainMGDamageScenario = 430.0;

	//static constexpr const auto& ShadowChainMGEnableDelay = 1.0s;
	static constexpr const auto& ShadowChainMGLifeTime = 2.4s;
	static constexpr const auto& ShadowChainMGCycleTime = 0.06s;
	static constexpr const auto& ShadowChainMGFixDuration = 0.1s;
	static constexpr float ShadowChainMGFixSpeed = 150.0;

	LINK_ENTITY_TO_CLASS(chainmg_shadowchainmg, CChainMG_ShadowChainMG);

	KnockbackData CChainMG_ShadowChainMG::GetKnockBackData()
	{
		return { ShadowKnockbackGround, ShadowKnockbackJump, ShadowKnockbackJumpForward, ShadowKnockbackDucking, ShadowVelocityModifier };
	}

	float CChainMG_ShadowChainMG::GetShadowChainMGDamage() const
	{
		if (g_pModRunning->DamageTrack() == DT_ZB)
			return ShadowChainMGDamageZombie;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			return ShadowChainMGDamageScenario;
		return ShadowChainMGDamage;
	};

	void CChainMG_ShadowChainMG::Init(CBasePlayer* pOwner, CChainMG* pWeapon, CBaseEntity* pEnemy, int iLastHitGroup)
	{
		m_pOwner = pOwner;
		m_pWeapon = pWeapon;
		m_iTeam = m_pOwner->m_iTeam;

		m_pEnemy = pEnemy;

		//Set Aiming AT?
		if (m_pEnemy)
		{
			Vector vecOrigin = pev->origin;
			Vector vecAngles;

			vecOrigin = m_pEnemy->pev->origin - vecOrigin;
			vecAngles = UTIL_VecToAngles(vecOrigin);
			vecAngles.x = vecAngles.z = 0.0;

			pev->angles = vecAngles;
		}
	
		if (m_pEnemy)
		{
			pev->sequence = 1;
		}
		else
		{
			pev->sequence = 0;
		}

		pev->frame = 0;

		ResetSequenceInfo();

		m_pLastHitGroup = iLastHitGroup;
		m_iEffectState = EFFECT_FADEIN;
		m_flEffectTime = gpGlobals->time;
		m_flRemoveTime = gpGlobals->time + ShadowChainMGLifeTime;
		m_flDamageTime = gpGlobals->time;
		SetThink(&CChainMG_ShadowChainMG::ShadowThink);
		pev->nextthink = gpGlobals->time + 0.1s;
	}

	void CChainMG_ShadowChainMG::Spawn(void)
	{
		Precache();

		SET_MODEL(ENT(pev), "models/ef_chainmg_summon_chainmg.mdl");
		UTIL_SetOrigin(pev, pev->origin);

		pev->solid = SOLID_NOT;
		pev->movetype = MOVETYPE_TOSS;
		pev->takedamage = DAMAGE_NO;
		pev->rendermode = kRenderTransAlpha;
		pev->renderamt = 0;

		m_flFadeInTime = ShadowModelFadeInTime;
		m_flFadeOutTime = ShadowModelFadeOutTime;
		m_flEffectTime = invalid_time_point;
		m_flRemoveTime = invalid_time_point;
		m_flDamageTime = invalid_time_point;
	
		pev->frame = 0;
		pev->sequence = 0;

		ResetSequenceInfo();
	}

	void CChainMG_ShadowChainMG::Precache(void)
	{
		PRECACHE_MODEL("models/ef_chainmg_summon_chainmg.mdl");
	}

	void CChainMG_ShadowChainMG::EF_Appear(void)
	{
		duration_t interval = gpGlobals->time - m_flEffectTime;

		if (interval > m_flFadeInTime)
		{
			m_flEffectTime = invalid_time_point;
			m_iEffectState = EFFECT_NONE;

			pev->rendermode = kRenderTransAlpha;
			pev->renderfx = kRenderFxNone;
			pev->renderamt = ShadowModelBaseAlpha;

		}
		else
		{
			pev->rendermode = kRenderTransAlpha;
			pev->renderamt = (interval / m_flFadeInTime) * ShadowModelBaseAlpha;
		}
	}

	void CChainMG_ShadowChainMG::EF_Hide(void)
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
			pev->renderamt = (1.0 - interval / m_flFadeOutTime) * ShadowModelBaseAlpha;
		}
	}

	void CChainMG_ShadowChainMG::RemoveShadow(void)
	{
		m_flRemoveTime = invalid_time_point;
		m_flEffectTime = gpGlobals->time;
		m_iEffectState = EFFECT_FADEOUT;
	}


	void EXPORT CChainMG_ShadowChainMG::ShadowThink()
	{
		pev->nextthink = gpGlobals->time + ShadowChainMGCycleTime;

		if (!m_pWeapon)
		{
			RemoveShadow();
		}

		if (m_iEffectState != EFFECT_FADEOUT && m_pOwner && (!m_pOwner->IsAlive() || m_pOwner->m_bIsZombie))
		{
			RemoveShadow();
		}

		if (m_flRemoveTime != invalid_time_point && m_flRemoveTime < gpGlobals->time)
		{
			RemoveShadow();
		}

		if (m_iEffectState != EFFECT_FADEOUT && m_pEnemy && m_flDamageTime != invalid_time_point && m_flDamageTime < gpGlobals->time)
		{
			if (m_pEnemy)
			{
				DoSingleDamage(m_pEnemy);
			}

			m_flDamageTime = gpGlobals->time + ShadowChainMGCycleTime;
		}


		if (m_iEffectState == EFFECT_FADEIN)
			EF_Appear();

		if (m_iEffectState == EFFECT_FADEOUT)
			EF_Hide();
	}

	void CChainMG_ShadowChainMG::DoSingleDamage(CBaseEntity* pEnemy)
	{
		pev->frame = 0;
		pev->sequence = 1;

		ResetSequenceInfo();

		UTIL_EMIT_SOUND(this->edict(), CHAN_ITEM, "weapons/chainsr-1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

		TraceResult tr;
		Vector vecStart = pev->origin;
		Vector vecEnd = pEnemy->BodyTarget(vecEnd);
		Vector vecDirection = (vecEnd - vecStart).Normalize();
		UTIL_TraceLine(vecStart, vecEnd, dont_ignore_monsters, edict(), &tr);

		if (tr.flFraction < 1.0 && tr.pHit != pEnemy->edict())
			return;

		ClearMultiDamage();
		tr.iHitgroup = m_pLastHitGroup;
		pEnemy->TraceAttack(m_pOwner->pev, GetShadowChainMGDamage(), vecDirection, &tr, DMG_NEVERGIB | DMG_BULLET);
		ApplyMultiDamage(this->pev, m_pOwner->pev);

		if (pEnemy->IsPlayer())
		{
			CBasePlayer* player = dynamic_cast<CBasePlayer*>(pEnemy);

			if (player->m_bIsZombie)
			{
				auto strategy = dynamic_cast<CPlayerModStrategy_ZB1*>(player->m_pModStrategy.get());
				if (strategy)
				{
					strategy->SetStunSpeedTime(ShadowChainMGFixDuration, ShadowChainMGFixSpeed);
				}
			}
			else
			{
				//support other mode
				player->m_tLockBuffTime = gpGlobals->time + ShadowChainMGFixDuration;
				player->m_flLockSpeed = ShadowChainMGFixSpeed;
				player->ResetMaxSpeed();
			}
		}

	}

	static constexpr float ShadowMP5Damage = 2.0;
	static constexpr float ShadowMP5DamageZombie = 190.0;
	static constexpr float ShadowMP5DamageScenario = 410.0;

	static constexpr const auto& ShadowMP5EnableDelay = 1.0s;
	static constexpr const auto& ShadowMP5LifeTime = 2.4s;
	static constexpr const auto& ShadowMP5CycleTime = 0.6s;
	static constexpr const auto& ShadowMP5FixDuration = 0.1s;
	static constexpr float ShadowMP5FixSpeed = 150.0;

	LINK_ENTITY_TO_CLASS(chainmg_shadowmp5, CChainMG_ShadowMP5);

	KnockbackData CChainMG_ShadowMP5::GetKnockBackData()
	{
		return { ShadowKnockbackGround, ShadowKnockbackJump, ShadowKnockbackJumpForward, ShadowKnockbackDucking, ShadowVelocityModifier };
	}

	float CChainMG_ShadowMP5::GetShadowMP5Damage() const
	{
		if (g_pModRunning->DamageTrack() == DT_ZB)
			return ShadowMP5DamageZombie;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			return ShadowMP5DamageScenario;
		return ShadowMP5Damage;
	};

	void CChainMG_ShadowMP5::Init(CBasePlayer* pOwner, CChainMG* pWeapon, CBaseEntity* pEnemy, int iLastHitGroup)
	{
		m_pOwner = pOwner;
		m_pWeapon = pWeapon;
		m_iTeam = m_pOwner->m_iTeam;

		m_pEnemy = pEnemy;

		//Set Aiming AT?
		if (m_pEnemy)
		{
			Vector vecOrigin = pev->origin;
			Vector vecAngles;

			vecOrigin = m_pEnemy->pev->origin - vecOrigin;
			vecAngles = UTIL_VecToAngles(vecOrigin);
			vecAngles.x = vecAngles.z = 0.0;

			pev->angles = vecAngles;
		}

		if (m_pEnemy)
		{
			pev->sequence = 1;
		}
		else
		{
			pev->sequence = 0;
		}

		pev->frame = 0;

		ResetSequenceInfo();

		m_pLastHitGroup = iLastHitGroup;
		m_iEffectState = EFFECT_FADEIN;
		m_flEffectTime = gpGlobals->time;
		m_flRemoveTime = gpGlobals->time + ShadowMP5LifeTime;
		m_flDamageTime = gpGlobals->time;
		SetThink(&CChainMG_ShadowMP5::ShadowThink);
		pev->nextthink = gpGlobals->time + 0.1s;
	}

	void CChainMG_ShadowMP5::Spawn(void)
	{
		Precache();

		SET_MODEL(ENT(pev), "models/ef_chainmg_summon_mp5.mdl");
		UTIL_SetOrigin(pev, pev->origin);

		pev->solid = SOLID_NOT;
		pev->movetype = MOVETYPE_TOSS;
		pev->takedamage = DAMAGE_NO;
		pev->rendermode = kRenderTransAlpha;
		pev->renderamt = 0;

		m_flFadeInTime = ShadowModelFadeInTime;
		m_flFadeOutTime = ShadowModelFadeOutTime;
		m_flEffectTime = invalid_time_point;
		m_flRemoveTime = invalid_time_point;
		m_flDamageTime = invalid_time_point;

		pev->frame = 0;
		pev->sequence = 0;

		ResetSequenceInfo();
	}

	void CChainMG_ShadowMP5::Precache(void)
	{
		PRECACHE_MODEL("models/ef_chainmg_summon_mp5.mdl");
	}

	void CChainMG_ShadowMP5::EF_Appear(void)
	{
		duration_t interval = gpGlobals->time - m_flEffectTime;

		if (interval > m_flFadeInTime)
		{
			m_flEffectTime = invalid_time_point;
			m_iEffectState = EFFECT_NONE;

			pev->rendermode = kRenderTransAlpha;
			pev->renderfx = kRenderFxNone;
			pev->renderamt = ShadowModelBaseAlpha;

		}
		else
		{
			pev->rendermode = kRenderTransAlpha;
			pev->renderamt = (interval / m_flFadeInTime) * ShadowModelBaseAlpha;
		}
	}

	void CChainMG_ShadowMP5::EF_Hide(void)
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
			pev->renderamt = (1.0 - interval / m_flFadeOutTime) * ShadowModelBaseAlpha;
		}
	}

	void CChainMG_ShadowMP5::RemoveShadow(void)
	{
		m_flRemoveTime = invalid_time_point;
		m_flEffectTime = gpGlobals->time;
		m_iEffectState = EFFECT_FADEOUT;
	}


	void EXPORT CChainMG_ShadowMP5::ShadowThink()
	{
		pev->nextthink = gpGlobals->time + ShadowMP5CycleTime;

		if (!m_pWeapon)
		{
			RemoveShadow();
		}

		if (m_iEffectState != EFFECT_FADEOUT && m_pOwner && (!m_pOwner->IsAlive() || m_pOwner->m_bIsZombie))
		{
			RemoveShadow();
		}

		if (m_flRemoveTime != invalid_time_point && m_flRemoveTime < gpGlobals->time)
		{
			RemoveShadow();
		}

		if (m_iEffectState != EFFECT_FADEOUT && m_pEnemy && m_flDamageTime != invalid_time_point && m_flDamageTime < gpGlobals->time)
		{
			if (m_pEnemy)
			{
				DoSingleDamage(m_pEnemy);
			}

			m_flDamageTime = gpGlobals->time + ShadowChainMGCycleTime;
		}


		if (m_iEffectState == EFFECT_FADEIN)
			EF_Appear();

		if (m_iEffectState == EFFECT_FADEOUT)
			EF_Hide();
	}

	void CChainMG_ShadowMP5::DoSingleDamage(CBaseEntity* pEnemy)
	{
		pev->frame = 0;
		pev->sequence = 1;

		ResetSequenceInfo();

		UTIL_EMIT_SOUND(this->edict(), CHAN_ITEM, "weapons/chainsr-1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

		TraceResult tr;
		Vector vecStart = pev->origin;
		Vector vecEnd = pEnemy->BodyTarget(vecEnd);
		Vector vecDirection = (vecEnd - vecStart).Normalize();
		UTIL_TraceLine(vecStart, vecEnd, dont_ignore_monsters, edict(), &tr);

		if (tr.flFraction < 1.0 && tr.pHit != pEnemy->edict())
			return;

		ClearMultiDamage();
		tr.iHitgroup = m_pLastHitGroup;
		pEnemy->TraceAttack(m_pOwner->pev, GetShadowMP5Damage(), vecDirection, &tr, DMG_NEVERGIB | DMG_BULLET);
		ApplyMultiDamage(this->pev, m_pOwner->pev);

		if (pEnemy->IsPlayer())
		{
			CBasePlayer* player = dynamic_cast<CBasePlayer*>(pEnemy);

			if (player->m_bIsZombie)
			{
				auto strategy = dynamic_cast<CPlayerModStrategy_ZB1*>(player->m_pModStrategy.get());
				if (strategy)
				{
					strategy->SetStunSpeedTime(ShadowMP5FixDuration, ShadowMP5FixSpeed);
				}
			}
			else
			{
				//support other mode
				player->m_tLockBuffTime = gpGlobals->time + ShadowMP5FixDuration;
				player->m_flLockSpeed = ShadowMP5FixSpeed;
				player->ResetMaxSpeed();
			}
		}

	}

	//static constexpr const auto& ShadowAT4EnableDelay = 1.0s;
	static constexpr float ShadowAT4RocketSpeed = 2000.0;

	LINK_ENTITY_TO_CLASS(chainmg_shadowat4, CChainMG_ShadowAT4);

	KnockbackData CChainMG_ShadowAT4::GetKnockBackData()
	{
		return { ShadowKnockbackGround, ShadowKnockbackJump, ShadowKnockbackJumpForward, ShadowKnockbackDucking, ShadowVelocityModifier };
	}

	float CChainMG_ShadowAT4::GetShadowAT4Damage() const
	{
		if (g_pModRunning->DamageTrack() == DT_ZB)
			return ShadowMP5DamageZombie;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			return ShadowMP5DamageScenario;
		return ShadowMP5Damage;
	};

	void CChainMG_ShadowAT4::Init(CBasePlayer* pOwner, CChainMG* pWeapon, Vector vecPredictOrigin, int iAnim)
	{
		m_pOwner = pOwner;
		m_pWeapon = pWeapon;
		m_iTeam = m_pOwner->m_iTeam;

		pev->sequence = iAnim;
		pev->frame = 0;

		ResetSequenceInfo();

		m_iEffectState = EFFECT_FADEIN;
		m_flEffectTime = gpGlobals->time;
		m_flRemoveTime = gpGlobals->time + ShadowChainMGLifeTime;
		m_flDamageTime = gpGlobals->time;
		SetThink(&CChainMG_ShadowAT4::ShadowThink);
		pev->nextthink = gpGlobals->time + 0.01s;

		Vector vecAttachment, vecAttachmentAngles;
		GET_ATTACHMENT(ENT(pev), 0, vecAttachment, vecAttachmentAngles);

		Vector vecDir = (vecPredictOrigin - vecAttachment).Normalize();
		Vector vecAngles = UTIL_VecToAngles(vecDir);

		CChainMG_ShadowAT4Rocket* pRocket = static_cast<CChainMG_ShadowAT4Rocket*>(CBaseEntity::Create("chainmg_shadowat4_rocket", vecAttachment, vecAngles, pev->owner));
		if (pRocket)
		{
			pRocket->pev->velocity = vecDir * ShadowAT4RocketSpeed;
		}

	}

	void CChainMG_ShadowAT4::Spawn(void)
	{
		Precache();

		SET_MODEL(ENT(pev), "models/ef_chainmg_summon_at4.mdl");
		UTIL_SetOrigin(pev, pev->origin);

		pev->solid = SOLID_NOT;
		pev->movetype = MOVETYPE_NOCLIP;
		pev->takedamage = DAMAGE_NO;
		pev->rendermode = kRenderTransAlpha;
		pev->renderamt = 0;

		m_flFadeInTime = ShadowModelFadeInTime;
		m_flFadeOutTime = ShadowModelFadeOutTime;
		m_flEffectTime = invalid_time_point;
		m_flRemoveTime = invalid_time_point;
		
		pev->frame = 0;
		pev->sequence = 0;

		ResetSequenceInfo();
	}

	void CChainMG_ShadowAT4::Precache(void)
	{
		PRECACHE_MODEL("models/ef_chainmg_summon_at4.mdl");
	}

	void CChainMG_ShadowAT4::EF_Appear(void)
	{
		duration_t interval = gpGlobals->time - m_flEffectTime;

		if (interval > m_flFadeInTime)
		{
			m_flEffectTime = invalid_time_point;
			m_iEffectState = EFFECT_NONE;

			pev->rendermode = kRenderTransAlpha;
			pev->renderfx = kRenderFxNone;
			pev->renderamt = ShadowModelBaseAlpha;
		}
		else
		{
			pev->rendermode = kRenderTransAlpha;
			pev->renderamt = (interval / m_flFadeInTime) * ShadowModelBaseAlpha;
		}
	}

	void CChainMG_ShadowAT4::EF_Hide(void)
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
			pev->renderamt = (1.0 - interval / m_flFadeOutTime) * ShadowModelBaseAlpha;
		}
	}

	void CChainMG_ShadowAT4::RemoveShadow(void)
	{
		m_flRemoveTime = invalid_time_point;
		m_flEffectTime = gpGlobals->time;
		m_iEffectState = EFFECT_FADEOUT;
	}


	void EXPORT CChainMG_ShadowAT4::ShadowThink()
	{
		pev->nextthink = gpGlobals->time + 0.1s;

		if (!m_pWeapon)
		{
			RemoveShadow();
		}

		if (m_iEffectState != EFFECT_FADEOUT && m_pOwner && (!m_pOwner->IsAlive() || m_pOwner->m_bIsZombie))
		{
			RemoveShadow();
		}

		if (m_flRemoveTime != invalid_time_point && m_flRemoveTime < gpGlobals->time)
		{
			RemoveShadow();
		}

		if (m_iEffectState == EFFECT_FADEIN)
			EF_Appear();

		if (m_iEffectState == EFFECT_FADEOUT)
			EF_Hide();
	}


	static constexpr float ShadowAT4DirectDamage = 20.0;
	static constexpr float ShadowAT4DirectDamageZombie = 2000.0;
	static constexpr float ShadowAT4DirectDamageScenario = 9300.0;

	static constexpr float ShadowAT4ExplodeDamage = 20.0;
	static constexpr float ShadowAT4ExplodeDamageZombie = 6800.0;
	static constexpr float ShadowAT4ExplodeDamageScenario = 9300.0;

	static constexpr float ShadowAT4ExplodeRadius = 5.0;

	static constexpr const auto& ShadowAT4FixDuration = 1.25s;
	static constexpr float ShadowAT4FixSpeed = 70.0;
	static constexpr float ShadowAT4FixGravity = 100.0;


	LINK_ENTITY_TO_CLASS(chainmg_shadowat4_rocket, CChainMG_ShadowAT4Rocket)

	void CChainMG_ShadowAT4Rocket::Spawn(void)
	{
		Precache();

		pev->movetype = MOVETYPE_FLY;
		pev->solid = SOLID_BBOX;

		SET_MODEL(ENT(pev), "models/shell_svdex.mdl");
		UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));
		UTIL_SetOrigin(pev, pev->origin);

		SetTouch(&CChainMG_ShadowAT4Rocket::RocketTouch);
		SetThink(&CChainMG_ShadowAT4Rocket::IgniteThink);

		pev->velocity = gpGlobals->v_forward * ShadowAT4RocketSpeed;
		pev->nextthink = gpGlobals->time + 0.1s;
		pev->dmg = GetExplodeDamage();

	}

	void CChainMG_ShadowAT4Rocket::Precache(void)
	{
		m_iTrail = PRECACHE_MODEL("sprites/black_smoke3.spr");

		PRECACHE_MODEL("models/shell_svdex.mdl");
	}

	int CChainMG_ShadowAT4Rocket::GetWeaponsId(void)
	{
		return WEAPON_CHAINMG;
	}

	void CChainMG_ShadowAT4Rocket::IgniteThink()
	{
		pev->movetype = MOVETYPE_FLY;

		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_BEAMPARTICLE); //defined as 32
		WRITE_SHORT(this->entindex());  // short (entity:attachment to follow)
		WRITE_SHORT(m_iTrail);// short (sprite index)
		WRITE_SHORT(m_iTrail);// short (sprite index)
		WRITE_SHORT(m_iTrail);// short (sprite index)
		WRITE_BYTE(255); //r
		WRITE_BYTE(255); //g
		WRITE_BYTE(255); //b
		WRITE_BYTE(0); //randomcolor
		WRITE_BYTE(100); //scale in 0.01's
		WRITE_BYTE(6); //freq(time of one cycle) in 0.01's
		WRITE_BYTE(3); //fadetime in 0.1s
		WRITE_BYTE(1);  //count
		WRITE_BYTE(0);  //random in org
		WRITE_BYTE(0);  //speed (if>0 do slow gravity)
		MESSAGE_END();

		m_flIgniteTime = gpGlobals->time;

		SetThink(&CChainMG_ShadowAT4Rocket::FollowThink);
		pev->nextthink = gpGlobals->time + 0.1s;
	}

	void CChainMG_ShadowAT4Rocket::FollowThink()
	{
		if (!IsInWorld())
		{
			SetThink(NULL);
			SetTouch(NULL);

			UTIL_Remove(this);
			return;
		}

		UTIL_MakeAimVectors(pev->angles);

		pev->angles = UTIL_VecToAngles(gpGlobals->v_forward);

		pev->nextthink = gpGlobals->time + 0.1s;
	}

	void CChainMG_ShadowAT4Rocket::RocketTouch(CBaseEntity* pOther)
	{
		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_KILLBEAM);
		WRITE_SHORT(this->entindex());
		MESSAGE_END();

		MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
		WRITE_BYTE(TE_EXPLOSION);	// This makes a dynamic light and the explosion sprites/sound
		WRITE_COORD(pev->origin.x);		// Send to PAS because of the sound
		WRITE_COORD(pev->origin.y);
		WRITE_COORD(pev->origin.z + 20.0f);
		WRITE_SHORT(g_sModelIndexFireball3);
		WRITE_BYTE(25);			// scale * 10
		WRITE_BYTE(30);		// framerate
		WRITE_BYTE(TE_EXPLFLAG_NONE);	// flags
		MESSAGE_END();

		MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
		WRITE_BYTE(TE_EXPLOSION);	// This makes a dynamic light and the explosion sprites/sound
		WRITE_COORD(pev->origin.x + RANDOM_FLOAT(-64, 64));	// Send to PAS because of the sound
		WRITE_COORD(pev->origin.y + RANDOM_FLOAT(-64, 64));
		WRITE_COORD(pev->origin.z + RANDOM_FLOAT(30, 35));
		WRITE_SHORT(g_sModelIndexFireball2);
		WRITE_BYTE(30);			// scale * 10
		WRITE_BYTE(30);		// framerate
		WRITE_BYTE(TE_EXPLFLAG_NONE);	// flags
		MESSAGE_END();

		TraceResult tr = UTIL_GetGlobalTrace();

		if (RANDOM_FLOAT(0.0, 1.0) < 0.5)
			UTIL_DecalTrace(&tr, DECAL_SCORCH1);
		else
			UTIL_DecalTrace(&tr, DECAL_SCORCH2);

		switch (RANDOM_LONG(0, 2))
		{
		case 0: EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "weapons/debris1.wav", 0.55, ATTN_NORM, 0, PITCH_NORM); break;
		case 1: EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "weapons/debris2.wav", 0.55, ATTN_NORM, 0, PITCH_NORM); break;
		case 2: EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "weapons/debris3.wav", 0.55, ATTN_NORM, 0, PITCH_NORM); break;
		}

		CBasePlayer* pPlayer = (CBasePlayer*)GET_PRIVATE(pev->owner);

		if (!pPlayer || FNullEnt(pPlayer->edict()) || !pPlayer->IsPlayer() || pPlayer->m_bIsZombie)
		{
			SetTouch(NULL);
			SetThink(NULL);

			UTIL_Remove(this);
			return;
		}

		if (!pOther->IsDormant() && pOther->pev->takedamage != DAMAGE_NO)
		{
			entvars_t* pevOwner = VARS(pev->owner);

			ClearMultiDamage();

			pOther->TraceAttack(pevOwner, GetDirectDamage(), pev->velocity.Normalize(), &tr, DMG_BULLET | DMG_NEVERGIB);

			ApplyMultiDamage(pev, pevOwner);

			if (pOther->IsPlayer())
			{
				CBasePlayer* player = dynamic_cast<CBasePlayer*>(pOther);

				if (player->m_bIsZombie)
				{
					auto strategy = dynamic_cast<CPlayerModStrategy_ZB1*>(player->m_pModStrategy.get());
					if (strategy)
					{
						strategy->SetStunSpeedTime(ShadowAT4FixDuration, ShadowAT4FixSpeed);
						strategy->SetStunGravityTime(ShadowAT4FixDuration, ShadowAT4FixGravity);
					}
				}
				else
				{
					//support other mode
					player->m_tLockBuffTime = gpGlobals->time + ShadowAT4FixDuration;
					player->m_flLockSpeed = ShadowAT4FixSpeed;
					player->m_flLockGravity = ShadowAT4FixGravity;
					player->ResetMaxSpeed();
				}
			}
		}

		CBaseEntity* pEntity = NULL;

		Vector vecEnd;
		int bInWater = (UTIL_PointContents(pev->origin) == CONTENTS_WATER);
		float flRadius = ShadowAT4ExplodeRadius * 39.37;
		while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, flRadius)) != NULL)
		{
			if (pEntity->edict() == pev->owner)
				continue;

			if (pEntity->IsDormant())
				continue;

			if (pEntity->pev->takedamage == DAMAGE_NO)
				continue;

			if (pEntity->Classify() == CLASS_NONE)
				continue;

			if (g_pGameRules->PlayerRelationship(pPlayer, pEntity) == GR_TEAMMATE)
				continue;

			vecEnd = pEntity->BodyTarget(pev->origin);

			UTIL_TraceLine(pev->origin, vecEnd, ignore_monsters, pev->owner, &tr);

			if (tr.flFraction < 1.0 && tr.pHit != pEntity->edict())
				continue;

			int falloff = GetExplodeDamage() / flRadius;

			float flCurrentDamage = max(0, GetExplodeDamage() - (pev->origin - pEntity->pev->origin).Length() * falloff);

			pEntity->TakeDamage(pev, pPlayer->pev, flCurrentDamage, DMG_BULLET | DMG_NEVERGIB);

			if (pEntity->IsPlayer())
			{
				CBasePlayer* player = dynamic_cast<CBasePlayer*>(pEntity);

				if (player->m_bIsZombie)
				{
					auto strategy = dynamic_cast<CPlayerModStrategy_ZB1*>(player->m_pModStrategy.get());
					if (strategy)
					{
						strategy->SetStunSpeedTime(ShadowAT4FixDuration, ShadowAT4FixSpeed);
						strategy->SetStunGravityTime(ShadowAT4FixDuration, ShadowAT4FixGravity);
					}
				}
				else
				{
					//support other mode
					player->m_tLockBuffTime = gpGlobals->time + ShadowAT4FixDuration;
					player->m_flLockSpeed = ShadowAT4FixSpeed;
					player->m_flLockGravity = ShadowAT4FixGravity;
					player->ResetMaxSpeed();
				}
			}
		}

		SetThink(nullptr);
		SetTouch(nullptr);
		UTIL_Remove(this);
	}

	float CChainMG_ShadowAT4Rocket::GetDirectDamage()
	{
		if (g_pModRunning->DamageTrack() == DT_ZB)
			return ShadowAT4DirectDamageZombie;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			return ShadowAT4DirectDamageScenario;
		return ShadowAT4DirectDamage;
	}

	float CChainMG_ShadowAT4Rocket::GetExplodeDamage()
	{
		if (g_pModRunning->DamageTrack() == DT_ZB)
			return ShadowAT4ExplodeDamageZombie;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			return ShadowAT4ExplodeDamageScenario;
		return ShadowAT4ExplodeDamage;


	}
	
}
