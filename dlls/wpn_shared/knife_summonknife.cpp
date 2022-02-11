/*
knife_summonknife.cpp
Copyright (C) 2021 Original Author 昔日の旧情 Modify by JustANoobQAQ , Entity Think by Rentro


This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#include <dlls/gamemode/mod_zb2.h>
#endif

#include "weapons/WeaponTemplate.hpp"
#include "weapons/KnifeAttack.h"




enum summoner_effect_state_e
{
	EFFECT_NONE,
	EFFECT_FADEIN,
	EFFECT_FADEOUT
};

enum summoner_anim
{
	ANIM_IDLE,
	ANIM_SLASH1,
	ANIM_SLASH2,
	ANIM_SLASH3,
	ANIM_SLASH4,
	ANIM_STAB,
	ANIM_SUMMON,
	ANIM_UNSUMMON
};

#ifndef CLIENT_DLL
namespace sv {
	class CKnifeSummonKnife;
	class CSummoner : public CBaseAnimating
	{
	public:
		void Spawn() override
		{
			Precache();

			SetThink(&CSummoner::OnThink);
			
			pev->solid = SOLID_NOT;
			pev->movetype = MOVETYPE_NOCLIP;
			pev->takedamage = DAMAGE_NO;
			pev->rendermode = kRenderTransAlpha;
			pev->renderamt = 0;
			pev->frame = 0;
			pev->sequence = 6;
			pev->animtime = gpGlobals->time;
			pev->framerate = 1.0;

			SET_MODEL(this->edict(), "models/summoner.mdl");

			pev->nextthink = gpGlobals->time + 0.6s;

			ResetSequenceInfo();

			m_flTimeAppearAttack = UTIL_WeaponTimeBase() + 0.01s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.86s;
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.4s;
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.6s;

			pev->classname = MAKE_STRING("d_summonknife");
		}

		void Precache() override
		{
			PRECACHE_MODEL("models/ef_summoner_summoning.mdl");
			//PRECACHE_MODEL("models/ef_summoner_swing.mdl");

			PRECACHE_MODEL("models/summoner.mdl");

			PRECACHE_MODEL("sprites/ef_summoner_summon01.spr");
			PRECACHE_MODEL("sprites/ef_summoner_summon02.spr");
			PRECACHE_MODEL("sprites/ef_summoner_unsummon01.spr");
			PRECACHE_MODEL("sprites/ef_summoner_unsummon02.spr");

			PRECACHE_MODEL("sprites/ef_summoner_smoke.spr");
			PRECACHE_MODEL("sprites/ef_summoner_stab1.spr");
			PRECACHE_MODEL("sprites/ef_summoner_stab2.spr");

			PRECACHE_SOUND("weapons/summon_summon.wav");
			PRECACHE_SOUND("weapons/summon_unsummon.wav");
			PRECACHE_SOUND("weapons/summon_stab_fire.wav");
			PRECACHE_SOUND("weapons/summon_stab.wav");
			PRECACHE_SOUND("weapons/summon_slash1.wav");
			/*PRECACHE_SOUND("weapons/summon_slash2.wav");
			PRECACHE_SOUND("weapons/summon_slash3.wav");
			PRECACHE_SOUND("weapons/summon_slash4.wav");*/
		}

		void Init(CBasePlayer* pOwner, CKnifeSummonKnife* pWeapon)
		{
			m_flSummonerHeight = 25.0;
			m_flFadeInTime =  2.26;
			m_flFadeOutTime = 0.3;

			m_pOwner = pOwner;
			m_pWeapon = pWeapon;

			m_flEffectTime = gpGlobals->time;
			m_iEffectState = EFFECT_FADEIN;

			PLAYBACK_EVENT_FULL(FEV_GLOBAL, m_pOwner->edict(), PRECACHE_EVENT(1, "events/wpneffects.sc"), 0.0, m_pOwner->pev->origin + Vector(0, 0, m_flSummonerHeight), g_vecZero, 0.0, 0.0, 17, 0, TRUE, FALSE);

			//PLAYBACK_EVENT_FULL(FEV_GLOBAL, m_pOwner->edict(), PRECACHE_EVENT(1, "events/wpneffects.sc"), 0.0, m_pOwner->pev->origin + Vector(0, 0, m_flSummonerHeight), g_vecZero, 0.0, 0.0, 17, 0, TRUE, FALSE);

			//PLAYBACK_EVENT_FULL(FEV_GLOBAL, m_pOwner->edict(), PRECACHE_EVENT(1, "events/wpneffects.sc"), 0.6, g_vecZero, m_pOwner->pev->origin + Vector(0, 0, m_flSummonerHeight), 0.0, 0.0, 17, 1, TRUE, FALSE);
		}

		void PrimaryAttack()
		{
			if (m_flNextPrimaryAttack > UTIL_WeaponTimeBase())
				return;

			EMIT_SOUND_DYN(ENT(pev), CHAN_BODY, "weapons/summon_slash1.wav", VOL_NORM, ATTN_NORM, 0, 94);

			
			pev->sequence = RANDOM_LONG(1, 4);

			//PLAYBACK_EVENT_FULL(FEV_GLOBAL, this->edict(), PRECACHE_EVENT(1, "events/wpneffects.sc"), 0.0, g_vecZero, g_vecZero, 0.0, 0.0, 16, pev->sequence - 1, TRUE, FALSE);

			pev->animtime = gpGlobals->time;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.37s;
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.1s;
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.9s;

			//float flSlashDamage = g_pModRunning->DamageTrack() == DT_NONE ? 10 : 840;
			float flSlashDamage = g_pModRunning->DamageTrack() == DT_NONE ? 10 : 420;

			CBaseEntity* pEntity = NULL;
			TraceResult tr;
			int bInWater = (UTIL_PointContents(pev->origin) == CONTENTS_WATER);

			while ((pEntity = UTIL_FindEntityInSphere(pEntity, m_pOwner->pev->origin, 235.0)) != NULL)
			{
				if (pEntity->edict() == m_pOwner->edict())
					continue;

				if (pEntity->IsDormant())
					continue;

				if (pEntity->pev->takedamage == DAMAGE_NO)
					continue;

				if (pEntity->Classify() == CLASS_NONE)
					continue;

				if (pEntity->IsPlayer())
				{
					CBasePlayer* target = static_cast<CBasePlayer*>(pEntity);

					if (m_pOwner->m_iTeam == target->m_iTeam)
						continue;
				}

				if ((bInWater && pEntity->pev->waterlevel == 0) || (!bInWater && pEntity->pev->waterlevel == 3))
					continue;

				Vector vecEnd = pEntity->BodyTarget(pev->origin);

				UTIL_TraceLine(pev->origin, vecEnd, ignore_monsters, pev->owner, &tr);

				if (tr.flFraction < 1.0 && tr.pHit != pEntity->edict())
					continue;

				int falloff = flSlashDamage / 235.0;
				float flCurrentDamage = max(0, flSlashDamage - (pev->origin - pEntity->pev->origin).Length() * falloff);
				pEntity->TakeDamage(m_pOwner->pev, m_pOwner->pev, flCurrentDamage, DMG_EXPLOSION);
			}
		}

		void SecondaryAttack()
		{
			if (m_flNextSecondaryAttack > UTIL_WeaponTimeBase())
				return;

			EMIT_SOUND_DYN(ENT(pev), CHAN_BODY, "weapons/summon_stab.wav", VOL_NORM, ATTN_NORM, 0, 94);

			pev->sequence = ANIM_STAB;
			pev->animtime = gpGlobals->time;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.04s;
			m_flDelaySecondaryAttack = gpGlobals->time + 0.76s;
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.9s;
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.1s;
		}

		void AppearAttack()
		{
			//float flAppearDamage = g_pModRunning->DamageTrack() == DT_NONE ? 200 : 4000;
			float flAppearDamage = g_pModRunning->DamageTrack() == DT_NONE ? 200 : 2000;

			CBaseEntity* pEntity = NULL;
			TraceResult tr;
			int bInWater = (UTIL_PointContents(pev->origin) == CONTENTS_WATER);

			while ((pEntity = UTIL_FindEntityInSphere(pEntity, m_pOwner->pev->origin, 171.0)) != NULL)
			{
				if (pEntity->edict() == m_pOwner->edict())
					continue;

				if (pEntity->IsDormant())
					continue;

				if (pEntity->pev->takedamage == DAMAGE_NO)
					continue;

				if (pEntity->Classify() == CLASS_NONE)
					continue;

				if (pEntity->IsPlayer())
				{
					CBasePlayer* target = static_cast<CBasePlayer*>(pEntity);

					if (m_pOwner->m_iTeam == target->m_iTeam)
						continue;
				}

				if ((bInWater && pEntity->pev->waterlevel == 0) || (!bInWater && pEntity->pev->waterlevel == 3))
					continue;

				Vector vecEnd = pEntity->BodyTarget(pev->origin);

				UTIL_TraceLine(pev->origin, vecEnd, ignore_monsters, pev->owner, &tr);

				if (tr.flFraction < 1.0 && tr.pHit != pEntity->edict())
					continue;

				int falloff = flAppearDamage / 171.0;
				float flCurrentDamage = max(0, flAppearDamage - (pev->origin - pEntity->pev->origin).Length() * falloff);
				pEntity->TakeDamage(m_pOwner->pev, m_pOwner->pev, flCurrentDamage, DMG_EXPLOSION);

				if (pEntity->IsPlayer())
				{
					CBasePlayer* player = (CBasePlayer*)pEntity;

					if (player->m_bIsZombie)
					{
						Vector vecDir = (player->pev->origin - m_pOwner->pev->origin);
						vecDir.z = 0;

						ApplyKnockbackData(player, vecDir, { 3000, 3000, 3000, 3000, 1.0f });
						player->pev->velocity.z = +200.0f;
					}
				}
			}
		}

		void HideAttack()
		{
			//float flAHideDamage = g_pModRunning->DamageTrack() == DT_NONE ? 10 : 3000;
			float flAHideDamage = g_pModRunning->DamageTrack() == DT_NONE ? 10 : 1500;

			CBaseEntity* pEntity = NULL;
			TraceResult tr;
			int bInWater = (UTIL_PointContents(pev->origin) == CONTENTS_WATER);

			while ((pEntity = UTIL_FindEntityInSphere(pEntity, m_pOwner->pev->origin, 235.0)) != NULL)
			{
				if (pEntity->edict() == m_pOwner->edict())
					continue;

				if (pEntity->IsDormant())
					continue;

				if (pEntity->pev->takedamage == DAMAGE_NO)
					continue;

				if (pEntity->Classify() == CLASS_NONE)
					continue;

				if (pEntity->IsPlayer())
				{
					CBasePlayer* target = static_cast<CBasePlayer*>(pEntity);

					if (m_pOwner->m_iTeam == target->m_iTeam)
						continue;
				}

				if ((bInWater && pEntity->pev->waterlevel == 0) || (!bInWater && pEntity->pev->waterlevel == 3))
					continue;

				Vector vecEnd = pEntity->BodyTarget(pev->origin);

				UTIL_TraceLine(pev->origin, vecEnd, ignore_monsters, pev->owner, &tr);

				if (tr.flFraction < 1.0 && tr.pHit != pEntity->edict())
					continue;

				int falloff = flAHideDamage / 235.0;
				float flCurrentDamage = max(0, flAHideDamage - (pev->origin - pEntity->pev->origin).Length() * falloff);
				pEntity->TakeDamage(m_pOwner->pev, m_pOwner->pev, flCurrentDamage, DMG_EXPLOSION);

				if (pEntity->IsPlayer())
				{
					CBasePlayer* player = (CBasePlayer*)pEntity;

					if (player->m_bIsZombie)
					{
						Vector vecDir = (player->pev->origin - m_pOwner->pev->origin).Normalize();

						player->pev->velocity.x = vecDir.x * 1.0 * 3000.0;
						player->pev->velocity.y = vecDir.y * 1.0 * 3000.0;
						player->pev->velocity.z += 1.0 * 200.0;
					}
				}
			}
		}

		void OnThink()
		{
			pev->nextthink = gpGlobals->time + 0.01s;

			if (!m_pOwner)
			{
				m_pWeapon = NULL;
				SetThink(nullptr);
				UTIL_Remove(this);
			}

			UTIL_SetOrigin(pev, m_pOwner->pev->origin + Vector(0, 0, m_flSummonerHeight));

			pev->angles = m_pOwner->pev->angles;
			pev->angles.x = 0;


			if (m_iEffectState != EFFECT_FADEOUT && m_pOwner && (!m_pOwner->IsAlive() || m_pOwner->m_bIsZombie))
			{
				RemoveSummoner();
			}

			if (m_flDelaySecondaryAttack != invalid_time_point && m_flDelaySecondaryAttack < gpGlobals->time)
				DelaySecondaryAttack();

			if (m_iEffectState == EFFECT_FADEIN)
				EF_Appear();

			if (m_iEffectState == EFFECT_FADEOUT)
				EF_Hide();

			m_flTimeWeaponIdle -= 0.01s;
			m_flNextPrimaryAttack -= 0.01s;
			m_flNextSecondaryAttack -= 0.01s;
			m_flTimeAppearAttack -= 0.01s;

			if (m_flTimeAppearAttack < UTIL_WeaponTimeBase())
			{
				AppearAttack();
				m_flTimeAppearAttack = UTIL_WeaponTimeBase() + 9999.0s;
			}

			if (m_flTimeWeaponIdle <= 0.0s)
				m_flTimeWeaponIdle = 0.0s;

			if (m_flNextPrimaryAttack <= 0.0s)
				m_flNextPrimaryAttack = 0.0s;

			if (m_flNextSecondaryAttack <= 0.0s)
				m_flNextSecondaryAttack = 0.0s;

			if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
				return;

			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
			pev->sequence = ANIM_IDLE;
			pev->animtime = gpGlobals->time;
		}

		void DelaySecondaryAttack()
		{
			m_flDelaySecondaryAttack = invalid_time_point;

			if (!m_pOwner)
				return;

			TraceResult tr;
			Vector vecEnd, vecDir;

			EMIT_SOUND_DYN(ENT(pev), CHAN_BODY, "weapons/summon_stab_fire.wav", VOL_NORM, ATTN_NORM, 0, 94);

			//float flTouchDamage = g_pModRunning->DamageTrack() == DT_NONE ? 80 : 3200;
			//float flExplodeDamage = g_pModRunning->DamageTrack() == DT_NONE ? 30 : 1200;
			float flTouchDamage = g_pModRunning->DamageTrack() == DT_NONE ? 80 : 1600;
			float flExplodeDamage = g_pModRunning->DamageTrack() == DT_NONE ? 30 : 600;
			float flDistance = 4096.0;

			Vector vecSrc = m_pOwner->GetGunPosition();
			UTIL_MakeVectors(m_pOwner->pev->v_angle + m_pOwner->pev->punchangle);
			vecDir = gpGlobals->v_forward;
			vecEnd = vecSrc + vecDir * flDistance;
	
			UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(m_pOwner->pev), &tr);
			//UTIL_TraceHull(vecSrc, vecEnd, dont_ignore_monsters, head_hull, ENT(m_pOwner->pev), &tr);	CSO maybe use Hull

			if (TheBots != NULL && tr.flFraction != 1.0f) {
				TheBots->OnEvent(EVENT_BULLET_IMPACT, this, (CBaseEntity*)&tr.vecEndPos);
			}

			CBaseEntity* pHitEntity = NULL;

			if (tr.flFraction != 1.0f) 
			{
				CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);
				pHitEntity = pEntity;
				if (pEntity->pev->takedamage != DAMAGE_NO)
				{
					tr.iHitgroup = HITGROUP_CHEST;
					ClearMultiDamage();
					pEntity->TraceAttack(m_pOwner->pev, flTouchDamage, vecDir, &tr, (DMG_BULLET | DMG_NEVERGIB));
					ApplyMultiDamage(m_pOwner->pev, m_pOwner->pev);
					if (pEntity->IsPlayer())
					{
						CBasePlayer* player = (CBasePlayer*)pEntity;

						if (player->m_bIsZombie)
						{
							Vector vecDir = (player->pev->origin - m_pOwner->pev->origin);
							vecDir.z = 0;

							ApplyKnockbackData(player, vecDir, { 1000, 1000, 1000, 1000, 1.0f });
							player->pev->velocity.z = +200.0f;
						}
					}
				}
			}
			
			CBaseEntity* pEntity = NULL;
			vecSrc = tr.vecEndPos;

			PLAYBACK_EVENT_FULL(FEV_GLOBAL, this->edict(), PRECACHE_EVENT(1, "events/wpneffects.sc"), 0.0, vecSrc, vecDir, 0.0, 0.0, 15, 0, TRUE, FALSE);

			int bInWater = (UTIL_PointContents(pev->origin) == CONTENTS_WATER);

			while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, 100.0)) != NULL)
			{
				if (pEntity->edict() == m_pOwner->edict())
					continue;

				if (pEntity->IsDormant())
					continue;

				if (pEntity->pev->takedamage == DAMAGE_NO)
					continue;

				if (pEntity->Classify() == CLASS_NONE)
					continue;

				if (pHitEntity && pEntity == pHitEntity)
					continue;

				if (pEntity->IsPlayer())
				{
					CBasePlayer* target = static_cast<CBasePlayer*>(pEntity);

					if (m_pOwner->m_iTeam == target->m_iTeam)
						continue;
				}

				if ((bInWater && pEntity->pev->waterlevel == 0) || (!bInWater && pEntity->pev->waterlevel == 3))
					continue;

				vecEnd = pEntity->BodyTarget(pev->origin);

				UTIL_TraceLine(pev->origin, vecEnd, ignore_monsters, pev->owner, &tr);

				if (tr.flFraction < 1.0 && tr.pHit != pEntity->edict())
					continue;

				int falloff = flExplodeDamage / 90.0;
				float flCurrentDamage = max(0, flExplodeDamage - (pev->origin - pEntity->pev->origin).Length() * falloff);
				pEntity->TakeDamage(m_pOwner->pev, m_pOwner->pev, flCurrentDamage, DMG_EXPLOSION);

				if (pEntity->IsPlayer())
				{
					CBasePlayer* player = (CBasePlayer*)pEntity;

					if (player->m_bIsZombie)
					{
						Vector vecDir = (player->pev->origin - m_pOwner->pev->origin);
						vecDir.z = 0;

						ApplyKnockbackData(player, vecDir, { 1000, 1000, 1000, 1000, 1.0f });
						player->pev->velocity.z = +200.0f;
					}
				}
			}	
		}

		void EF_Appear(void)
		{
			duration_t interval = gpGlobals->time - m_flEffectTime;

			if (interval / 1s > m_flFadeInTime)
			{
				m_flEffectTime = invalid_time_point;
				m_iEffectState = EFFECT_NONE;

				pev->rendermode = kRenderNormal;
				pev->renderfx = kRenderFxNone;
				pev->renderamt = 0;

				//PLAYBACK_EVENT_FULL(FEV_GLOBAL, this->edict(), PRECACHE_EVENT(1, "events/wpneffects.sc"), 0.0, g_vecZero, g_vecZero, 0.0, 0.0, 17, 3, TRUE, FALSE);
			}
			else
			{
				pev->rendermode = kRenderTransAlpha;
				pev->renderamt = (interval / (m_flFadeInTime * 1s)) * 255.0;
			}
		}

		void EF_Hide(void)
		{
			duration_t interval = gpGlobals->time - m_flEffectTime;

			if (interval / 1s > m_flFadeOutTime)
			{
				m_flEffectTime = invalid_time_point;
				m_iEffectState = EFFECT_NONE;

				HideAttack();
				PLAYBACK_EVENT_FULL(FEV_GLOBAL, m_pOwner->edict(), PRECACHE_EVENT(1, "events/wpneffects.sc"), 0.0, m_pOwner->pev->origin + Vector(0, 0, m_flSummonerHeight), pev->origin, 0.0, 0.0, 17, 2, TRUE, FALSE);

				pev->nextthink = invalid_time_point;
				m_pOwner = NULL;
				m_pWeapon = NULL;
				SetThink(nullptr);
				UTIL_Remove(this);
				
			}
			else
			{
				pev->rendermode = kRenderTransAlpha;
				pev->renderamt = (1.0 - interval / (m_flFadeOutTime * 1s) ) * 255.0;			
			}
		}

		void RemoveSummoner(void)
		{
			EMIT_SOUND_DYN(ENT(pev), CHAN_BODY, "weapons/summon_unsummon.wav", VOL_NORM, ATTN_NORM, 0, 94);

			pev->sequence = ANIM_UNSUMMON;
			pev->animtime = gpGlobals->time;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.7s;

			m_flEffectTime = gpGlobals->time;
			m_iEffectState = EFFECT_FADEOUT;
		}

		KnockbackData GetKnockBackData() override { return { 85.0f, 100.0f, 100.0f, 80.0f, 0.8f }; }

		time_point_t m_flEffectTime;
		float m_flFadeInTime;
		float m_flFadeOutTime;
		time_point_t m_flDelaySecondaryAttack;
		duration_t m_flTimeAppearAttack;
		duration_t m_flNextPrimaryAttack;
		duration_t m_flNextSecondaryAttack;
		duration_t m_flTimeWeaponIdle;
		float m_flSummonerHeight;

		CBasePlayer* m_pOwner;
		CKnifeSummonKnife* m_pWeapon;
		summoner_effect_state_e m_iEffectState;
	};

LINK_ENTITY_TO_CLASS(summoner, CSummoner)


class CSummonKnifeCannon : public CBaseEntity
{
public:
	void Spawn() override
	{
		Precache();

		m_fSequenceLoops = 0;
		//ph26 = 0;
		SetThink(&CSummonKnifeCannon::FollowThink);
		SetTouch(&CSummonKnifeCannon::OnTouch);
		SET_MODEL(this->edict(), "models/ef_summonknife_cannon.mdl");

		//ph32 = ?
		pev->rendermode = kRenderTransAdd;
		pev->renderfx = kRenderFxNone;
		pev->renderamt = 180;
		pev->framerate = 1.0;
		pev->scale = 1.0;
		pev->solid = SOLID_BBOX;
		pev->movetype = MOVETYPE_FLYMISSILE;
		pev->nextthink = gpGlobals->time + 0.1s;

		UTIL_SetOrigin(pev, pev->origin);
		UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));

		pev->classname = MAKE_STRING("d_summonknife");
	}

	void Precache() override
	{
		PRECACHE_MODEL("models/ef_summonknife_cannon.mdl");

		PRECACHE_MODEL("sprites/ef_summon_charging_exp.spr");
	}

	void EXPORT OnTouch(CBaseEntity* pOther)
	{
		if (pev->owner == pOther->edict())
		{
			SetTouch(NULL);
			SetThink(NULL);
			Remove();
			return;
		}

		if (pOther->IsBSPModel() || !pOther->IsPlayer())
		{
			PLAYBACK_EVENT_FULL(FEV_GLOBAL, this->edict(), PRECACHE_EVENT(1, "events/wpneffects.sc"), 0.0, pev->origin, g_vecZero, 0.0, 0.0, 15, 1, TRUE, FALSE);
			//MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
			//WRITE_BYTE(TE_SPRITE);
			//WRITE_COORD(pev->origin.x);
			//WRITE_COORD(pev->origin.y);
			//WRITE_COORD(pev->origin.z);
			//WRITE_SHORT(m_iSprExplo);	// sprite index
			//WRITE_BYTE(6);			// scale in 0.1's
			//WRITE_BYTE(255);			// brightness
			//MESSAGE_END();
		}

		CBaseEntity* pAttacker = CBaseEntity::Instance(pev->owner);
		CBasePlayer* pAttackePlayer = nullptr;
		if (pAttacker && pAttacker->IsPlayer())
			pAttackePlayer = static_cast<CBasePlayer*>(pAttacker);

		if (pAttackePlayer)
		{
			if (pOther->pev->takedamage != DAMAGE_NO && pOther->IsAlive())
			{
				Vector vecDirection = (pOther->pev->origin - pev->origin).Normalize();

				TraceResult tr;
				UTIL_TraceLine(pev->origin, pOther->pev->origin, missile, ENT(pAttackePlayer->pev), &tr);
				tr.iHitgroup = HITGROUP_CHEST; // ...

				ClearMultiDamage();
				pOther->TraceAttack(pAttackePlayer->pev, m_flTouchDamage, vecDirection, &tr, DMG_BULLET);
				ApplyMultiDamage(pAttackePlayer->pev, pAttackePlayer->pev);

				if (pOther->IsPlayer())
				{
					CBasePlayer* player = (CBasePlayer*)pOther;

					if (player->m_bIsZombie)
					{
						player->pev->velocity = m_vecStartVelocity;
					}
				}
			}

			TraceResult tr;
			CBaseEntity* pEntity = NULL;

			Vector vecEnd;
			int bInWater = (UTIL_PointContents(pev->origin) == CONTENTS_WATER);

			while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, 90.0)) != NULL)
			{
				if (pEntity->edict() == pev->owner)
					continue;

				if (pEntity->IsDormant())
					continue;

				if (pEntity->pev->takedamage == DAMAGE_NO)
					continue;

				if (pEntity->Classify() == CLASS_NONE)
					continue;

				if (pEntity->IsPlayer())
				{
					CBasePlayer* target = static_cast<CBasePlayer*>(pEntity);

					if (pAttackePlayer->m_iTeam == target->m_iTeam)
						continue;
				}

				if ((bInWater && pEntity->pev->waterlevel == 0) || (!bInWater && pEntity->pev->waterlevel == 3))
					continue;

				vecEnd = pEntity->BodyTarget(pev->origin);

				UTIL_TraceLine(pev->origin, vecEnd, ignore_monsters, pev->owner, &tr);

				if (tr.flFraction < 1.0 && tr.pHit != pEntity->edict())
					continue;

				int falloff = RadiusDamageAmount / 90.0;

				float flCurrentDamage = max(0, RadiusDamageAmount - (pev->origin - pEntity->pev->origin).Length() * falloff);

				pEntity->TakeDamage(pev, pAttackePlayer->pev, flCurrentDamage, DMG_EXPLOSION);

				if (pEntity->IsPlayer())
				{
					CBasePlayer* player = (CBasePlayer*)pEntity;

					if (player->m_bIsZombie)
					{
						Vector vecDir = (player->pev->origin - pAttackePlayer->pev->origin);
						vecDir.z = 0;

						ApplyKnockbackData(player, vecDir, { 1200, 1200, 1200, 1200, 0.85f });
						player->pev->velocity.z = +200.0f;
					}
				}
			}
		}


		SetTouch(NULL);
		SetThink(NULL);
		Remove();
	}

	void EXPORT FollowThink()
	{
		pev->frame += pev->framerate * gpGlobals->frametime / 1s;

		if (pev->frame > m_flMaxFrames)
			pev->frame = fmod(pev->frame, m_flMaxFrames);

		pev->nextthink = gpGlobals->time + 0.01s;
	}

	void Init(Vector vecVelocity, float flTouchDamage, float flExplodeDamage, TeamName iTeam)
	{
		std::tie(m_flTouchDamage, RadiusDamageAmount, m_iTeam) = std::make_tuple(flTouchDamage, flExplodeDamage, iTeam);
		m_vecStartVelocity = pev->velocity = std::move(vecVelocity);
	}

	KnockbackData GetKnockBackData() override { return { 85.0f, 100.0f, 100.0f, 80.0f, 1.0f }; }

	int m_fSequenceLoops;
	time_point_t m_flRemoveTime;
	float m_flMaxFrames;
	float m_flTouchDamage;
	float RadiusDamageAmount;
	TeamName m_iTeam;
	int m_iSprEffect;
	Vector m_vecStartVelocity;

protected:
	void Remove()
	{
		SetThink(nullptr);
		SetTouch(nullptr);
		pev->effects |= EF_NODRAW; // 0x80u
		return UTIL_Remove(this);
	}
};

LINK_ENTITY_TO_CLASS(summonknife_cannon, CSummonKnifeCannon)
}
#endif


#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif
	
#define KNIFE_BODYHIT_VOLUME 128
#define KNIFE_WALLHIT_VOLUME 512

	class CKnifeSummonKnife : public LinkWeaponTemplate<CKnifeSummonKnife,
		TGeneralData,
		BuildTGetItemInfoFromCSW<WEAPON_KNIFE>::template type
	>
	{
	public:
		static constexpr const char* ClassName = "knife_summonknife";
		static constexpr const char* V_Model = "models/v_summonknife.mdl";
		static constexpr const char* P_Model = "models/p_summonknife_charging.mdl";
		static constexpr const char* W_Model = "models/w_knife.mdl";

		static constexpr int MaxClip = 100;
		static constexpr auto ItemSlot = KNIFE_SLOT;
		static constexpr const char* AnimExtension = "dragonsword";
		KnockbackData KnockBack = { 1.0f, 1.0f, 1.0f, 1.0f, 0.0f };

	public:

		enum summonknife_e
		{
			ANIM_IDLE_A,
			ANIM_SLASH1,
			ANIM_SLASH2,
			ANIM_SLASH3,
			ANIM_SLASH4,
			ANIM_SLASH_END,
			ANIM_DRAW_A,
			ANIM_IDLE_B,
			ANIM_STAB1,
			ANIM_STAB2,
			ANIM_STAB_END,
			ANIM_DRAW_B,
			ANIM_CHARGING_START,
			ANIM_CHARGING_LOOP,
			ANIM_CHARGING_RELEASE,
			ANIM_SUMMONING_A,
		};

		void Precache() override
		{
			m_usFireSummonKnife = PRECACHE_EVENT(1, "events/summonknife.sc");

			PRECACHE_MODEL("models/v_summonknife.mdl");
			PRECACHE_MODEL("models/p_summonknife_a.mdl");
			PRECACHE_MODEL("models/p_summonknife_b.mdl");
			PRECACHE_MODEL("models/p_summonknife_charging.mdl");
			
		
			PRECACHE_MODEL("models/ef_summonknife_cannon.mdl");

			PRECACHE_MODEL("sprites/muzzleflash97.spr");
			PRECACHE_MODEL("sprites/muzzleflash98.spr");
			PRECACHE_MODEL("sprites/muzzleflash99.spr");
			PRECACHE_MODEL("sprites/muzzleflash100.spr");
			
			PRECACHE_MODEL("sprites/ef_summon_charging.spr");
			PRECACHE_MODEL("sprites/ef_summon_charging_left.spr");
			
			PRECACHE_MODEL("sprites/ef_summon_charging_loop.spr");
			PRECACHE_MODEL("sprites/ef_summon_stab1.spr");
			PRECACHE_MODEL("sprites/ef_summon_stab1_left.spr");
				
			PRECACHE_MODEL("sprites/ef_summonl.spr");
			PRECACHE_MODEL("sprites/ef_summonl_left.spr");
			PRECACHE_MODEL("sprites/ef_summonr.spr");
			PRECACHE_MODEL("sprites/ef_summonr_left.spr");
			
			PRECACHE_SOUND("weapons/summonknife1.wav");
			PRECACHE_SOUND("weapons/summonknife2.wav");
			PRECACHE_SOUND("weapons/summonknife3.wav");
			PRECACHE_SOUND("weapons/summonknife4.wav");

			PRECACHE_SOUND("weapons/summonknife_charging_loop.wav");
			PRECACHE_SOUND("weapons/summonknife_charging_relese.wav");
			PRECACHE_SOUND("weapons/summonknife_stab.wav");
			PRECACHE_SOUND("weapons/summoning_ready.wav");
					
			PRECACHE_SOUND("weapons/mastercombat_wall.wav");
			PRECACHE_SOUND("weapons/mastercombat_hit1.wav");
			PRECACHE_SOUND("weapons/plasmagun_exp.wav");
		}
		
		BOOL Deploy(void) override
		{
			m_fMaxSpeed = 250;
			m_iState = 0;
			m_iState2 = 0;
			m_iCheck = 0;
			m_iThinkType = 0;
			m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;
			m_tNextThink = m_tTimeSoundEmit = m_flTimeCheckCharge = invalid_time_point;
			m_pPlayer->m_bShieldDrawn = false;

			RemoveSummoner();

#ifndef CLIENT_DLL
			if (m_flTimeholster != invalid_time_point)
			{
				m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] += (gpGlobals->time - m_flTimeholster) / 0.25s;
			}
			if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] >= 100)
				m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] = 100;
#endif
			int iAnim = ANIM_DRAW_B;
			const char* szAnim = "sfsword";
			const char* szP_Model = "models/p_summonknife_b.mdl";
			duration_t delay = 0.17s;
			if (m_iLastAnim < 7 || m_iLastAnim == 15)
			{
				iAnim = ANIM_DRAW_A;
				szAnim = "katanad";
				szP_Model = "models/p_summonknife_a.mdl";
				delay = 0.1s;
			}

			m_iLastAnim = iAnim;
			return DefaultDeploy(V_Model, szP_Model, iAnim, szAnim, UseDecrement() != FALSE, delay);
		}

		int GetItemInfo(ItemInfo* p) override;
		void PrimaryAttack() override;	
		void SecondaryAttack() override;
		void Spawn() override;
		void WeaponIdle() override;
		float GetMaxSpeed() override;
		void Holster(int skiplocal) override;
		int ExtractAmmo(CBasePlayerWeapon* pWeapon) override;
		void DelayPrimaryAttack();
		void DelaySecondaryAttack();
		void OnAttack2Released();
#ifndef CLIENT_DLL
		hit_result_t KnifeAttack(float flDamage, float flRadius, float flAngleDegrees, int bitsDamageType, entvars_t* pevInflictor, CBasePlayer* pAttackPlayer, bool IsPrimaryAttack);
		void MaterialSound(float flRadius);
# endif
		BOOL CanDrop() override { return false; }
		void ItemPostFrame() override;
		void RemoveSummoner(void);

	public:
		float GetPrimaryAttackDamage() const
		{
			float flDamage = 28;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
			{
				//flDamage = 644.0f;
				flDamage = 190.0f;
				if (m_pSummoner)
					flDamage *= 0.85;
			}
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
			{
				flDamage = 1148.0f;
				if (m_pSummoner)
					flDamage *= 0.85;
			}
#endif
			return flDamage;
		}

		float GetSlashAttackDamage() const
		{
			float flDamage = 60;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
			{
				//flDamage = 1380.0f;
				flDamage = 360.0f;
				if (m_pSummoner)
					flDamage *= 0.85;
			}
			
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
			{
				flDamage = 2460.0f;
				if (m_pSummoner)
					flDamage *= 0.85;
			}
#endif
			return flDamage;
		}

		float GetSecondaryAttackDamage() const
		{
			float flDamage = 100;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				//flDamage = 2000.0f;
				flDamage = 800.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 3700.0f;
#endif
			return flDamage;
		}

		float GetChargeSecondaryAttackDamage() const
		{
			float flDamage = 115;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				//flDamage = 2300.0f;
				flDamage = 1000.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 4255.0f;
#endif
			return flDamage;
		}

#ifndef CLIENT_DLL
		CSummoner* m_pSummoner;
		time_point_t m_flTimeNextCharge;
		time_point_t m_flTimeholster;
#endif
		int m_usFireSummonKnife;
		time_point_t m_tTimeSoundEmit;
		time_point_t m_flTimeCheckCharge;
		int m_iDamageType;
		int m_iState;	//pev_iuser1
		int m_iState2;	//pev_iuser4
		int m_iCheck;	//g_iCheck
		int m_iLastAnim;
		time_point_t m_tNextThink;
		int m_iThinkType;
	};

	LINK_ENTITY_TO_CLASS(knife_summonknife, CKnifeSummonKnife)

	void CKnifeSummonKnife::Spawn()
	{
		pev->classname = MAKE_STRING("knife_summonknife");
		m_iId = WEAPON_KNIFE;
		Precache();
		SET_MODEL(ENT(pev), "models/w_knife.mdl");
		m_tNextThink = m_tTimeSoundEmit = m_flTimeCheckCharge = invalid_time_point;
#ifndef CLIENT_DLL
		m_flTimeholster = gpGlobals->time;
#endif
		m_iClip = WEAPON_NOCLIP;
		m_iDefaultAmmo = 0;
		m_iState = 0;
		m_iState2 = 0;
		m_iCheck = 0;
		m_iLastAnim = 0;
		m_iThinkType = 0;
#ifndef CLIENT_DLL
		m_pSummoner = NULL;
#endif
		FallInit();
	}

	int CKnifeSummonKnife::GetItemInfo(ItemInfo* p)
	{
		p->pszName = STRING(pev->classname);
		p->pszAmmo1 = "summoncharge";
		p->iMaxAmmo1 = 100;
		p->pszAmmo2 = NULL;
		p->iMaxAmmo2 = -1;
		p->iMaxClip = WEAPON_NOCLIP;
		p->iSlot = 2;
		p->iPosition = 1;
		p->iId = WEAPON_KNIFE;
		p->iFlags = 0;
		p->iWeight = KNIFE_WEIGHT;

		return 1;
	}

	int CKnifeSummonKnife::ExtractAmmo(CBasePlayerWeapon* pWeapon)
	{
		if (m_iDefaultAmmo)
		{
			m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] = m_iDefaultAmmo;
			m_iClip = WEAPON_NOCLIP;
			m_iDefaultAmmo = 0;
			return TRUE;
		}
		return CBasePlayerWeapon::ExtractAmmo(pWeapon);
	}

	void CKnifeSummonKnife::RemoveSummoner(void)
	{
#ifndef CLIENT_DLL
		if (m_pSummoner && m_pSummoner->m_iEffectState != EFFECT_FADEOUT)
		{
			m_pSummoner->RemoveSummoner();
			m_pSummoner = NULL;
		}
#endif
	}

#ifndef CLIENT_DLL
	hit_result_t CKnifeSummonKnife::KnifeAttack(float flDamage, float flRadius, float flAngleDegrees, int bitsDamageType, entvars_t* pevInflictor, CBasePlayer* pAttackPlayer, bool IsPrimaryAttack)
	{
		CBaseEntity* pEntity = NULL;
		hit_result_t result = HIT_NONE;
		TraceResult tr;
		UTIL_MakeVectors(pAttackPlayer->pev->v_angle);
		Vector vecEnd;
		Vector vecSrc = pAttackPlayer->GetGunPosition();

		Vector vecForward = gpGlobals->v_forward;
		Vector2D vecForward2D = vecForward.Make2D().Normalize();

		float c = cos(flAngleDegrees * 0.5 * M_PI / 180.0);

		while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, flRadius)) != NULL)
		{
			if (pEntity == pAttackPlayer)
				continue;

			if (pEntity->IsDormant())
				continue;

			if (pEntity->pev->takedamage == DAMAGE_NO)
				continue;

			if (!AngleCheck(pEntity, pAttackPlayer, vecSrc, vecForward, vecForward2D, flRadius, c, &tr))
				continue;

			float flRatio = 1.0;

			if (pEntity && pEntity->IsPlayer())
			{
				UTIL_MakeVectors(pEntity->pev->angles);

				if (DotProduct((pEntity->pev->origin.Make2D() - pAttackPlayer->pev->origin.Make2D()).Normalize(), gpGlobals->v_forward.Make2D()) > 0.8)
					flRatio = 3.0;
			}

			pAttackPlayer->m_iWeaponVolume = 128;

			ClearMultiDamage();

			pEntity->TraceAttack(pAttackPlayer->pev, flDamage * flRatio, (tr.vecEndPos - vecSrc).Normalize(), &tr, bitsDamageType);

			ApplyMultiDamage(pevInflictor, pAttackPlayer->pev);

			result = HIT_PLAYER;

			if (pEntity->IsPlayer())
			{
				CBasePlayer* player = (CBasePlayer*)pEntity;

				if (player->m_bIsZombie)
				{
					Vector vecDir = (player->pev->origin - pev->origin);
					vecDir.z = 0;
					if (IsPrimaryAttack)
					{
						/*auto strategy = dynamic_cast<CPlayerModStrategy_ZB1*>(player->m_pModStrategy.get());
						if (strategy)
						{
							strategy->SetStunSpeedTime(2.0s);
							strategy->SetStunGravityTime(2.0s);
						}*/
						//if (player->m_bIsZombie && player->m_ZombieClass)
						//{
						//	if (stricmp(player->m_ZombieClass, "revival") || !player->m_iZombieRevivalEnabled)
						//	{
						//		//need stun
						//		


								auto strategy = dynamic_cast<CPlayerModStrategy_ZB2*>(player->m_pModStrategy.get());
								if (strategy)
								{
									strategy->SetStunSpeedTime(2.0s);
									strategy->SetStunGravityTime(2.0s);
								}


						//		player->pev->velocity.x = 0;
						//		player->pev->velocity.y = 0;
						//		player->m_flVelocityModifier = 0.1;
						//	}
						//}			
					}
					else
					{
						if (player->m_bIsZombie && player->m_ZombieClass)
						{
							if (stricmp(player->m_ZombieClass, "revival") || !player->m_iZombieRevivalEnabled)
							{
								ApplyKnockbackData(player, vecDir, { 1200, 1200, 1200, 1200, 0.85f });

								/*auto strategy = dynamic_cast<CPlayerModStrategy_ZB1*>(player->m_pModStrategy.get());
								if (strategy)
								{
									strategy->SetStunSpeedTime(2.0s);
									strategy->SetStunGravityTime(2.0s);
								}*/
							}
						}
						
					}
				}
			}
		}

		if (!result)
		{
			static int calced = 0;
			static float calcRes = 0.0f;

			if (!(calced & 1))
			{
				calced |= 1;
				calcRes = sqrt(512.0);
			}

			vecEnd = vecSrc + vecForward * (flRadius - calcRes);

			UTIL_TraceHull(vecSrc, vecEnd, dont_ignore_monsters, human_hull, pAttackPlayer->edict(), &tr);

			if (tr.flFraction < 1.0f)
				result = HIT_WALL;
		}

		return result;
	}
#endif

	void CKnifeSummonKnife::DelayPrimaryAttack(void)
	{	
#ifndef CLIENT_DLL		
		UTIL_MakeVectors(m_pPlayer->pev->v_angle);
		Vector vecSrc = m_pPlayer->GetGunPosition();
		
		float flDamage = GetPrimaryAttackDamage();
		float flDistance = 143;
		float flAngle = 180.0;

		if (m_pSummoner)
		{
			flDistance = 235.0;
			flAngle = 360.0;
		}

		if(m_iLastAnim == ANIM_SLASH3 || m_iLastAnim == ANIM_SLASH3);
		{
			float flDamage = GetSlashAttackDamage();
		}

		switch (KnifeAttack(flDamage, flDistance, flAngle, DMG_NEVERGIB | DMG_BULLET, this->pev, m_pPlayer, true))
		{
		case HIT_PLAYER:
		{
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/mastercombat_hit1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			break;
		}
		case HIT_WALL:
		{
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/mastercombat_wall.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			//MaterialSound();
			break;
		}
		}
#endif

		SetThink(nullptr);
	}

	void CKnifeSummonKnife::PrimaryAttack(void)
	{
		m_pPlayer->pev->weaponmodel = MAKE_STRING("models/p_summonknife_a.mdl");
		Q_strcpy(m_pPlayer->m_szAnimExtention, "katanad");
		if (m_iCheck == 3 && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] >= 100)
		{

			m_iCheck = 0;
			m_iState = 0;
			SendWeaponAnim(ANIM_SUMMONING_A, UseDecrement() != FALSE);
			m_iLastAnim = ANIM_SUMMONING_A;

			PLAYBACK_EVENT_FULL(FEV_HOSTONLY, m_pPlayer->edict(), m_usFireSummonKnife, 0.0, g_vecZero, g_vecZero, 0.0, 0.0, ANIM_SUMMONING_A, 0, FALSE, FALSE);
			
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.2s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.4s;
			m_iThinkType = 2;
			m_tNextThink = gpGlobals->time + 0.1s;

#ifndef CLIENT_DLL

			if (m_pSummoner != NULL)
				return;

			m_pSummoner = static_cast<CSummoner*>(CBaseEntity::Create("summoner", m_pPlayer->pev->origin + Vector(0, 0, 25), g_vecZero, m_pPlayer->edict()));

			if (m_pSummoner)
				m_pSummoner->Init(m_pPlayer, this);
#endif
			return;
		}

		switch (m_iState)
		{
			case 0:
			{
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/summonknife1.wav", VOL_NORM, ATTN_NORM, 0, 94);
				
				m_iState = 1;
				SendWeaponAnim(ANIM_SLASH1, UseDecrement() != FALSE);
				m_iLastAnim = ANIM_SLASH1;
				SetThink(&CKnifeSummonKnife::DelayPrimaryAttack);
				pev->nextthink = gpGlobals->time + 0.1s;

				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0s;				
				m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.2s;
				m_tNextThink = gpGlobals->time + 0.5s;
				m_iThinkType = 1;

#ifndef CLIENT_DLL
				m_pPlayer->SetAnimation(PLAYER_ATTACK1);

				if (m_pSummoner)
					m_pSummoner->PrimaryAttack();
#endif
				if (!m_iCheck)
					m_iCheck = 1;
				else if (m_iCheck == 3)
					m_iCheck = 3;
				else
					m_iCheck = 1;
				break;
			}
			case 1:
			{
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/summonknife2.wav", VOL_NORM, ATTN_NORM, 0, 94);

				m_iState = 2;
				SendWeaponAnim(ANIM_SLASH2, UseDecrement() != FALSE);
				m_iLastAnim = ANIM_SLASH2;

				SetThink(&CKnifeSummonKnife::DelayPrimaryAttack);
				pev->nextthink = gpGlobals->time + 0.1s;

				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0s;
				m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.4s;
				m_tNextThink = gpGlobals->time + 0.5s;
				m_iThinkType = 1;

#ifndef CLIENT_DLL	
				m_pPlayer->SetAnimation(PLAYER_ATTACK1);

				if (m_pSummoner)
					m_pSummoner->PrimaryAttack();
#endif

				if (!m_iCheck)
					m_iCheck = 1;
				else if (m_iCheck == 3)
					m_iCheck = 3;
				else
					m_iCheck = 1;
				break;
			}
			case 2:
			{

				if (RANDOM_LONG(0, 1))
				{
					EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/summonknife3.wav", VOL_NORM, ATTN_NORM, 0, 94);

#ifndef CLIENT_DLL	
					SendWeaponAnim(ANIM_SLASH3, UseDecrement() != FALSE);
#endif
					m_iLastAnim = ANIM_SLASH3;

				}
				else
				{
					EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/summonknife4.wav", VOL_NORM, ATTN_NORM, 0, 94);
#ifndef CLIENT_DLL	
					SendWeaponAnim(ANIM_SLASH4, UseDecrement() != FALSE);
#endif
					m_iLastAnim = ANIM_SLASH4;

				}

#ifndef CLIENT_DLL	
				if (m_pSummoner)
					m_pSummoner->PrimaryAttack();
#endif

				m_iState = 0;
				SetThink(&CKnifeSummonKnife::DelayPrimaryAttack);
				pev->nextthink = gpGlobals->time + 0.1s;

				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0s;
				m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.43s;
				m_tNextThink = gpGlobals->time + 0.5s;
				m_iThinkType = 1;

				if (!m_iCheck)
					m_iCheck = 1;
				else if (m_iCheck == 3)
					m_iCheck = 3;
				else
					m_iCheck = 1;
				break;
			}
		}
	}

	void CKnifeSummonKnife::ItemPostFrame()
	{
#ifndef CLIENT_DLL
		if (!m_pSummoner)
		{
			if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] < 100)
			{
				if (m_flTimeNextCharge <= gpGlobals->time)
				{
					m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]++;
					m_flTimeNextCharge = gpGlobals->time + 0.25s;

					if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] == 100)
						EMIT_SOUND_DYN(pev->owner, CHAN_WEAPON, "weapons/summoning_ready.wav", VOL_NORM, ATTN_NORM, 0, 94);
				}
			}
		}
		else
		{
			if (m_flTimeNextCharge <= gpGlobals->time)
			{
				m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;
				m_flTimeNextCharge = gpGlobals->time + 0.13s;

				if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
				{
					m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] = 0;
					RemoveSummoner();
				}		
			}
		}
#endif


		if (m_tNextThink != invalid_time_point && m_tNextThink < gpGlobals->time)
		{
			switch (m_iThinkType)
			{
			case 1:
				{
					SendWeaponAnim(ANIM_SLASH_END, UseDecrement() != FALSE);
					m_iLastAnim = ANIM_SLASH_END;
					m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.9s;
					m_tNextThink = gpGlobals->time + 0.43s;

					m_iThinkType = 2;
					break;
				}
			case 2:
				{
					//reset slash anim
					m_iThinkType = 0;
					m_iState = 0;
					m_tNextThink = invalid_time_point;
					break;
				}
			case 3:
				{
					SendWeaponAnim(ANIM_STAB_END, UseDecrement() != FALSE);
					m_iLastAnim = ANIM_STAB_END;
					m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.9s;
					m_pPlayer->pev->weaponmodel = MAKE_STRING("models/p_summonknife_b.mdl");
					Q_strcpy(m_pPlayer->m_szAnimExtention, "sfsword");
					m_tNextThink = invalid_time_point;
					m_iState = 0;
					m_iThinkType = 0;
					break;
				}
			default:
			{
				m_tNextThink = invalid_time_point;
				break;
			}
			}
		}


		/*if (m_iState2 == 4)
		{
			if (m_tTimeSoundEmit != invalid_time_point && m_tTimeSoundEmit <= gpGlobals->time)
			{
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/summonknife_field_attack_inoperation_spark.wav", VOL_NORM, ATTN_NORM, 0, 94);
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/summonknife_field_attack_charge.wav", VOL_NORM, ATTN_NORM, 0, 94);

				m_tTimeSoundEmit = gpGlobals->time + 4s;
			}
		}*/

		return CBasePlayerWeapon::ItemPostFrame();
	}

	void CKnifeSummonKnife::SecondaryAttack(void)
	{
		if (!m_iState2)
		{
			if (m_iCheck == 1)
				m_iCheck = 2;
			else if (m_iCheck == 2)
				m_iCheck = 3;
			else
				m_iCheck = 0;

			m_pPlayer->pev->weaponmodel = MAKE_STRING("models/p_summonknife_b.mdl");
			Q_strcpy(m_pPlayer->m_szAnimExtention, "sfsword");

			SetThink(&CKnifeSummonKnife::DelaySecondaryAttack);
			if (m_iLastAnim < 7 || m_iLastAnim == 15)
			{
				//change anim

				SendWeaponAnim(ANIM_STAB1, UseDecrement() != FALSE);
				m_iLastAnim = ANIM_STAB1;

				PLAYBACK_EVENT_FULL(FEV_HOSTONLY, m_pPlayer->edict(), m_usFireSummonKnife, 0.466, g_vecZero, g_vecZero, 0.0, 0.0, ANIM_STAB1, 0, FALSE, FALSE);

				pev->nextthink = gpGlobals->time + 0.466s;
				m_tNextThink = gpGlobals->time + 0.86s;
				m_flTimeCheckCharge = gpGlobals->time + 0.8s;
				m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.5s;
			}
			else
			{
				SendWeaponAnim(ANIM_STAB2, UseDecrement() != FALSE);
				m_iLastAnim = ANIM_STAB2;

				PLAYBACK_EVENT_FULL(FEV_HOSTONLY, m_pPlayer->edict(), m_usFireSummonKnife, 0.266, g_vecZero, g_vecZero, 0.0, 0.0, ANIM_STAB2, 0, FALSE, FALSE);

				pev->nextthink = gpGlobals->time + 0.266s;
				m_tNextThink = gpGlobals->time + 0.86s;
				m_flTimeCheckCharge = gpGlobals->time + 0.63s;
				m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.3s;
			}

#ifndef CLIENT_DLL
			m_pPlayer->SetAnimation(PLAYER_ATTACK1);

			if (m_pSummoner)
				m_pSummoner->SecondaryAttack();
#endif

			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/summonknife_stab.wav", VOL_NORM, ATTN_NORM, 0, 94);

			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.9s;
			

			m_iThinkType = 3;
			m_iState2 = 1;
			m_iState = 0;
		}

		if (m_flTimeCheckCharge < gpGlobals->time)
		{
			switch (m_iState2)
			{
			case 1:
			{
				m_pPlayer->pev->weaponmodel = MAKE_STRING(P_Model);
				SendWeaponAnim(ANIM_CHARGING_START, UseDecrement() != FALSE);
				m_iLastAnim = ANIM_CHARGING_START;
				m_iState2 = 2;

				PLAYBACK_EVENT_FULL(FEV_HOSTONLY, m_pPlayer->edict(), m_usFireSummonKnife, 0.7, g_vecZero, g_vecZero, 0.0, 0.0, ANIM_CHARGING_START, 0, FALSE, FALSE);

				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/summonknife_charging_start.wav", VOL_NORM, ATTN_NORM, 0, 94);

				m_tNextThink = gpGlobals->time + 999999s;
				m_flTimeCheckCharge = gpGlobals->time + 1.3s;
				m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.4s;
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.4s;

				break;
			}
			case 2:
			{
				SendWeaponAnim(ANIM_CHARGING_LOOP, UseDecrement() != FALSE);
				m_iLastAnim = ANIM_CHARGING_LOOP;
				//EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_AUTO, "weapons/summonknife_charging_loop.wav", VOL_NORM, ATTN_NORM, 0, 94);

				PLAYBACK_EVENT_FULL(FEV_HOSTONLY, m_pPlayer->edict(), m_usFireSummonKnife, 0.0, g_vecZero, g_vecZero, 0.0, 0.0, ANIM_CHARGING_LOOP, 0, FALSE, FALSE);

				m_iState2 = 3;

				m_tNextThink = gpGlobals->time + 999999s;
				m_flTimeCheckCharge = gpGlobals->time + 0.01s;
				m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 999999s;
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.1s;
				break;
			}
			case 3:
			{
				m_tTimeSoundEmit = gpGlobals->time + 0.01s;
				m_flTimeCheckCharge = gpGlobals->time + 999999s;
				m_tNextThink = gpGlobals->time + 999999s;
				break;
			}
			}
			return;
		}
		
	}

	void CKnifeSummonKnife::OnAttack2Released(void)
	{
		switch (m_iState2)
		{
		case 1:
		{
			m_flNextSecondaryAttack = m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.9s;
			break;
		}
		case 2:
		{
			SendWeaponAnim(ANIM_STAB_END, UseDecrement() != FALSE);
			m_iLastAnim = ANIM_STAB_END;
			m_tNextThink = invalid_time_point;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.9s;
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.73s;
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.43s;
			break;
		}
		case 3:
		{
			Q_strcpy(m_pPlayer->m_szAnimExtention, "katana");
#ifndef CLIENT_DLL	
			m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
			SendWeaponAnim(ANIM_CHARGING_RELEASE, UseDecrement() != FALSE);
			m_iLastAnim = ANIM_CHARGING_RELEASE;

			//PLAYBACK_EVENT_FULL(FEV_HOSTONLY, m_pPlayer->edict(), m_usFireSummonKnife, 0.0, (float*)&g_vecZero, (float*)&g_vecZero, 0.0, 0.0, ANIM_CHARGING_RELEASE, 0, FALSE, FALSE);

			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/summonknife_charging_relese.wav", VOL_NORM, ATTN_NORM, 0, 94);

			m_flNextSecondaryAttack = m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.9s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.9s;

			SetThink(&CKnifeSummonKnife::DelaySecondaryAttack);
			m_tNextThink = gpGlobals->time + 0.9s;
			m_iThinkType = 3;
			pev->nextthink = gpGlobals->time + 0.4s;
			break;
		}
		}
		m_iState2 = 0;
	}

	void CKnifeSummonKnife::DelaySecondaryAttack(void)
	{
		UTIL_MakeVectors(m_pPlayer->pev->v_angle);
		Vector vecSrc = m_pPlayer->GetGunPosition();

#ifndef CLIENT_DLL	
		float flDamage = GetSecondaryAttackDamage();
		if (m_iLastAnim == ANIM_CHARGING_RELEASE);
		{
			float flDamage = GetChargeSecondaryAttackDamage();
		}

		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
		switch (KnifeAttack(flDamage, 171, 120, DMG_NEVERGIB | DMG_BULLET, this->pev, m_pPlayer, false))
		{
		case HIT_PLAYER:
		{
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/mastercombat_hit1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			break;
		}
		case HIT_WALL:
		{
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/mastercombat_wall.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			//MaterialSound();
			break;
		}
		}
#endif

		if (m_iLastAnim == ANIM_CHARGING_RELEASE)
		{
			//射实体
#ifndef CLIENT_DLL
			CSummonKnifeCannon* pEnt = static_cast<CSummonKnifeCannon*>(CBaseEntity::Create("summonknife_cannon", vecSrc, m_pPlayer->pev->v_angle, ENT(m_pPlayer->pev)));
			if (pEnt)
			{
				float flTouchDamage = g_pModRunning->DamageTrack() == DT_NONE ? 20 : 3960;
				float flExplodeDamage = g_pModRunning->DamageTrack() == DT_NONE ? 10 : 3300;
				pEnt->Init(gpGlobals->v_forward * 2000, flTouchDamage, flExplodeDamage, m_pPlayer->m_iTeam);
			}
#endif
		}

		SetThink(nullptr);
	}

	float CKnifeSummonKnife::GetMaxSpeed(void)
	{
			return 252;
	}

	void CKnifeSummonKnife::Holster(int skiplocal)
	{
		SetThink(nullptr);
		m_iState = 0;
		m_iState2 = 0;
		m_iCheck = 0;
		m_iThinkType = 0;
		m_tNextThink = m_tTimeSoundEmit = m_flTimeCheckCharge = invalid_time_point;
		
#ifndef CLIENT_DLL
		m_flTimeholster = gpGlobals->time;
		if(m_pSummoner)
			m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] = 0;
#endif
		RemoveSummoner();
		return CBasePlayerWeapon::Holster(skiplocal);
	}

	void CKnifeSummonKnife::WeaponIdle(void)
	{
		ResetEmptySound();
		m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

		if (m_iState2 && !(m_pPlayer->pev->button & IN_ATTACK2))
			OnAttack2Released();

		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;

		m_iCheck = 0;

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;

		int iAnim = ANIM_IDLE_B;
		
		if (m_iLastAnim < 7 || m_iLastAnim == 15)
			iAnim = ANIM_IDLE_A;
		
		SendWeaponAnim(iAnim, UseDecrement() != FALSE);
		m_iLastAnim = iAnim;
		
	}

}
