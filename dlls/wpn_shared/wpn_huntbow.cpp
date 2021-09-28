/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "wpn_huntbow.h"
#ifndef CLIENT_DLL
#include "effects.h"
#include "customentity.h"
#include "monsters.h"
#endif

#include <vector>
#include <array>

#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

#ifndef CLIENT_DLL
	class CHuntbowArrow : public CBaseEntity
	{
	public:
		void Spawn() override
		{
			Precache();

			SET_MODEL(this->edict(), "models/huntbow_arrow.mdl");
			SetThink(&CHuntbowArrow::OnThink);
			SetTouch(&CHuntbowArrow::OnTouch);
			pev->mins = { -0.1, -0.1, -0.1 };
			pev->maxs = { 0.1, 0.1, 0.1 };
			pev->rendermode = kRenderTransAdd; // 5;
			pev->renderfx = kRenderFxNone;
			pev->renderamt = 230.0;		
			pev->nextthink = gpGlobals->time + 0.2s;
			pev->framerate = 1;
			pev->gravity = 0.5;
			pev->solid = SOLID_BBOX; // 2
			pev->movetype = MOVETYPE_FLY; // 9

			pev->classname = MAKE_STRING("d_arrow");
		}

		void EXPORT OnTouch(CBaseEntity *pOther)
		{
			if (pev->owner == pOther->edict())
				return;
			pev->solid = SOLID_NOT;

			int DECAL_SHOT[5];
			DECAL_SHOT[0] = DECAL_INDEX("{shot1");
			DECAL_SHOT[1] = DECAL_INDEX("{shot2");
			DECAL_SHOT[2] = DECAL_INDEX("{shot3");
			DECAL_SHOT[3] = DECAL_INDEX("{shot4");
			DECAL_SHOT[4] = DECAL_INDEX("{shot5");

			Vector vecStart,vecAngle,vecForw,vecEnd;
			vecStart = pev->origin;
			vecAngle = Get_Velocity_Angle(this, vecAngle);

			UTIL_MakeVectors(vecAngle);		
			vecForw = gpGlobals->v_forward * 20.0;
			vecEnd = vecStart + vecForw;
			vecForw = gpGlobals->v_forward * -0.5;
			vecStart += vecForw;

			CBaseEntity *pAttacker = CBaseEntity::Instance(pev->owner);
			CBasePlayer *pAttackePlayer = nullptr;
			CBasePlayer *pInflictorPlayer = nullptr;

			if (pAttacker && pAttacker->IsPlayer())
				pAttackePlayer = static_cast<CBasePlayer *>(pAttacker);
		
			TraceResult tr;
			UTIL_TraceLine(vecStart, vecEnd, dont_ignore_monsters, ENT(pAttackePlayer->pev), &tr);
			
			if (pAttackePlayer &&
				pOther->pev->takedamage != DAMAGE_NO &&
				pOther->IsAlive()
				)
			{
				Vector vecDirection = (pev->velocity).Normalize();
				if (g_pModRunning->DamageTrack() == DT_ZB)
				{
					if (pOther->IsPlayer() && g_pGameRules->PlayerRelationship(pAttackePlayer, pOther) != GR_TEAMMATE)
					{
						pInflictorPlayer = static_cast<CBasePlayer*>(pOther);
						if(pInflictorPlayer->tlLastMarkTime[ENTINDEX(pAttacker->edict())] + 5s < gpGlobals->time)
							pInflictorPlayer->iRecorder[ENTINDEX(pAttacker->edict())] = 0;
						pInflictorPlayer->iRecorder[ENTINDEX(pAttacker->edict())] += 1;
						pInflictorPlayer->tlLastMarkTime[ENTINDEX(pAttacker->edict())] = gpGlobals->time;

						/*
						MESSAGE_BEGIN(MSG_ONE, gmsgHeadIcon, NULL, pAttacker->pev);
						WRITE_BYTE(5);	//type
						WRITE_SHORT(ENTINDEX(pInflictorPlayer->edict())); //entity
						WRITE_BYTE(pInflictorPlayer->iRecorder[ENTINDEX(pAttacker->edict())]);
						MESSAGE_END();*/

						m_ArrowDamage = m_ArrowDamage + (pInflictorPlayer->iRecorder[ENTINDEX(pAttacker->edict())] * 100);	//original 300
						if (pInflictorPlayer->iRecorder[ENTINDEX(pAttacker->edict())] == 5)
						{
							pInflictorPlayer->iRecorder[ENTINDEX(pAttacker->edict())] = 0;
						}
					}
				}

				ClearMultiDamage();
				pOther->TraceAttack(pAttackePlayer->pev, m_ArrowDamage, vecDirection, &tr, DMG_BULLET | DMG_NEVERGIB);
				ApplyMultiDamage(pAttackePlayer->pev, pAttackePlayer->pev);				

				pev->renderamt = 0.0;
				EMIT_SOUND_DYN(this->edict(), CHAN_AUTO, "weapons/xbow_hitbod1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			}

			SetThink(&CBaseEntity::SUB_StartFadeOut);
			pev->nextthink = gpGlobals->time + 6.0s;
			pev->rendermode = kRenderNormal;
			pev->renderamt = 255.0;

			if (pOther->IsBSPModel())
			{
				MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY, pev->origin);
				WRITE_BYTE(TE_GUNSHOTDECAL);
				WRITE_COORD(pev->origin.x);
				WRITE_COORD(pev->origin.y);
				WRITE_COORD(pev->origin.z);
				WRITE_SHORT(0);
				WRITE_BYTE(DECAL_SHOT[RANDOM_LONG(0, 4)]);
				MESSAGE_END();

				EMIT_SOUND_DYN(this->edict(), CHAN_AUTO, "weapons/xbow_hit1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
				pev->animtime = gpGlobals->time;
				pev->framerate = 1.0;
				pev->sequence = 0;
				SetThink(&CBaseEntity::SUB_StartFadeOut);
				pev->nextthink = gpGlobals->time + 3.5s;
				pev->renderamt = 255.0;
				pev->movetype = MOVETYPE_NONE;
				return;
			}
			else
				pev->flags |= FL_KILLME;

			pev->movetype = MOVETYPE_NONE;
		}

		void Precache() override
		{
			m_iBeamModelIndex = PRECACHE_MODEL("sprites/ef_huntbow_trail.spr");
			PRECACHE_MODEL("models/huntbow_arrow.mdl");
		}

		void OnThink()
		{
			if (pev->waterlevel)
				SetThink(&CHuntbowArrow::BubbleThink);
			else
			{
				MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
				WRITE_BYTE(TE_BEAMFOLLOW);
				WRITE_SHORT(entindex());
				WRITE_SHORT(m_iBeamModelIndex);
				WRITE_BYTE(6);
				WRITE_BYTE(2);
				WRITE_BYTE(225);
				WRITE_BYTE(150);
				WRITE_BYTE(150);
				WRITE_BYTE(255);
				MESSAGE_END();

				SetThink(&CHuntbowArrow::FollowThink);
			}

			pev->nextthink = gpGlobals->time + 0.1s;
		}

		void FollowThink(void)
		{
			if (pev->waterlevel)
				SetThink(&CHuntbowArrow::BubbleThink);

			pev->nextthink = gpGlobals->time + 0.1s;
		}

		void BubbleThink(void)
		{
			UTIL_BubbleTrail(pev->origin - pev->velocity * 0.1, pev->origin, 1);

			if (!pev->waterlevel)
				SetThink(&CHuntbowArrow::FollowThink);

			pev->nextthink = gpGlobals->time + 0.1s;
		}

		void Init(Vector vecVelocity, float flDamage, TeamName iTeam, float avelocity, int iMode)
		{
			pev->avelocity.z = avelocity;
			m_ArrowDamage = flDamage;
			pev->velocity = std::move(vecVelocity);
			m_iTeam = iTeam;
			m_iMode = iMode;

		}

		int m_iBeamModelIndex;
		int m_iMode;
		float m_ArrowDamage;
		TeamName m_iTeam;
	};

	LINK_ENTITY_TO_CLASS(huntbow_arrow, CHuntbowArrow);
#endif

LINK_ENTITY_TO_CLASS(weapon_huntbow, CHuntbow)

void CHuntbow::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_huntbow");

	Precache();
	m_iId = WEAPON_AWP;
	SET_MODEL(ENT(pev), "models/w_huntbow.mdl");
	m_flAccuracy = 0.2;
	m_iShotsFired = 0;

	m_iClip = -1;
	m_iDefaultAmmo = 150;

	m_flAccuracy = 0.05;
	m_iShotsFired = 0;
	m_bDelayFire = false;
	m_flTimeCheckCharge = gpGlobals->time + 999999s;

	FallInit();
}

void CHuntbow::Precache(void)
{
	PRECACHE_MODEL("models/v_huntbow.mdl");
	PRECACHE_MODEL("models/w_huntbow.mdl");
	PRECACHE_MODEL("models/p_huntbow.mdl");
	PRECACHE_MODEL("models/p_huntbow_empty.mdl");
	PRECACHE_MODEL("sprites/muzzleflash207.spr");
	PRECACHE_MODEL("sprites/muzzleflash208.spr");
	PRECACHE_MODEL("sprites/muzzleflash209.spr");
	PRECACHE_MODEL("sprites/muzzleflash210.spr");
	PRECACHE_MODEL("sprites/muzzleflash211.spr");
	PRECACHE_MODEL("sprites/muzzleflash212.spr");
	PRECACHE_MODEL("sprites/muzzleflash213.spr");
	PRECACHE_MODEL("sprites/dmgreiteration01.spr");
	PRECACHE_MODEL("sprites/dmgreiteration02.spr");
	PRECACHE_MODEL("sprites/dmgreiteration03.spr");
	PRECACHE_MODEL("sprites/dmgreiteration04.spr");
	PRECACHE_MODEL("sprites/dmgreiteration05.spr");
	PRECACHE_MODEL("sprites/ef_huntbow_explo.spr");
	PRECACHE_MODEL("sprites/ef_huntbow_trail.spr");

	PRECACHE_SOUND("weapons/xhuntbow_hit1.wav");
	PRECACHE_SOUND("weapons/xhuntbow_hitbod1.wav");
	PRECACHE_SOUND("weapons/failnaught_charge_loop_fx.wav");
	PRECACHE_SOUND("weapons/failnaught_charge_shoot.wav");
	PRECACHE_SOUND("weapons/failnaught_charge_shoot1.wav");
	PRECACHE_SOUND("weapons/failnaught_charge_shoot2.wav");
	PRECACHE_SOUND("weapons/failnaught_charge_start_fx.wav");
	PRECACHE_SOUND("weapons/failnaught_charge_start1.wav");
	PRECACHE_SOUND("weapons/failnaught_draw.wav");
	PRECACHE_SOUND("weapons/failnaught_draw_empty.wav");
	PRECACHE_SOUND("weapons/failnaught_shoot1_empty.wav");
	PRECACHE_SOUND("weapons/failnaught_shoot1.wav");
	PRECACHE_SOUND("weapons/failnaught_loop_fx.wav");
	PRECACHE_SOUND("weapons/failnaught-1.wav");
	PRECACHE_SOUND("weapons/failnaught-2.wav");
	PRECACHE_SOUND("weapons/failnaught-2_exp.wav");

	m_usFireHuntbow = PRECACHE_EVENT(1, "events/huntbow.sc");
}

int CHuntbow::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "bowarrow";
	p->iMaxAmmo1 = 150;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = -1;
	p->iSlot = 0;
	p->iPosition = 4;
	p->iId = m_iId = WEAPON_AWP;
	p->iFlags = 0;
	p->iWeight = M249_WEIGHT;

	return 1;
}

BOOL CHuntbow::Deploy(void)
{
	m_fInCharge = false;
	m_iChargeState = 0;
	m_flTimeCheckCharge = gpGlobals->time + 999999s;
	m_flAccuracy = 0.05;
	m_iShotsFired = 0;
	m_bDelayFire = false;


#ifndef CLIENT_DLL
	MESSAGE_BEGIN(MSG_ONE, gmsgDgaxe, NULL, m_pPlayer->pev);
	WRITE_BYTE(1);
	WRITE_BYTE(HUNTBOW_AIM_IDLE);
	MESSAGE_END();
#endif
	return DefaultDeploy("models/v_huntbow.mdl", m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 ? "models/p_huntbow_empty.mdl" : "models/p_huntbow.mdl", BOW_DRAW + (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0), "slingshot", UseDecrement() != FALSE, 0.2s, 1.5s);
}

int CHuntbow::ExtractAmmo(CBasePlayerWeapon *pWeapon)
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

#ifndef CLIENT_DLL
void CHuntbow::RadiusDamage(Vector vecAiming, float flDamage, float flRadius)
{
	const Vector vecSrc = vecAiming;
	entvars_t* const pevAttacker = VARS(pev->owner);
	entvars_t* const pevInflictor = this->pev;
	int bitsDamageType = DMG_BULLET;

	TraceResult tr;
	const int bInWater = (UTIL_PointContents(vecSrc) == CONTENTS_WATER);

	CBaseEntity* pEntity = NULL;
	while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, flRadius)) != NULL)
	{
		if (pEntity->pev->takedamage != DAMAGE_NO)
		{
			if (bInWater && !pEntity->pev->waterlevel)
				continue;

			if (!bInWater && pEntity->pev->waterlevel == 3)
				continue;

			if (pEntity->IsBSPModel())
				continue;

			if (pEntity->pev == pevAttacker)
				continue;

			Vector vecSpot = pEntity->BodyTarget(vecSrc);
			UTIL_TraceLine(vecSrc, vecSpot, missile, ENT(pevInflictor), &tr);

			if (tr.flFraction == 1.0f || tr.pHit == pEntity->edict())
			{
				if (tr.fStartSolid)
				{
					tr.vecEndPos = vecSrc;
					tr.flFraction = 0;
				}

				if (tr.flFraction == 1.0f)
				{
					pEntity->TakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType);
				}
				else
				{
					tr.iHitgroup = HITGROUP_CHEST;
					ClearMultiDamage();
					pEntity->TraceAttack(pevInflictor, flDamage, (tr.vecEndPos - vecSrc).Normalize(), &tr, bitsDamageType);
					ApplyMultiDamage(pevInflictor, pevAttacker);
				}
			}
		}
	}
}
#endif

void CHuntbow::PrimaryAttack(void)
{

	if (m_iChargeState)
	{
		OnAttack2Released();
	}
	else
	{
		HuntbowFire(GetSpread(), 0.3s, FALSE);
	}
}

void CHuntbow::SetWeaponModel(void)
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->pev->weaponmodel = MAKE_STRING("models/p_huntbow_empty.mdl");
	else
		m_pPlayer->pev->weaponmodel = MAKE_STRING("models/p_huntbow.mdl");
}

void CHuntbow::ChargedFire(void)
{
	SetInChargeState(false);

	m_iChargeState = 0;

	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
	{
		PlayEmptySound();

		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3s;

		return;
	}

	m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

	UTIL_MakeVectors(m_pPlayer->pev->punchangle + m_pPlayer->pev->v_angle);
	m_pPlayer->m_iWeaponVolume = QUIET_GUN_VOLUME;
	m_iMode = 1;
#ifndef CLIENT_DLL
	int iEffect = 3;
	Vector vecDir = m_pPlayer->FireBullets4(m_pPlayer->GetGunPosition(), gpGlobals->v_forward, 0, 8192, 7, 16, GetSecondaryAttackDamage(), 0.99, m_pPlayer->pev, true, m_pPlayer->random_seed, &iEffect, m_pPlayer);
	PLAYBACK_EVENT_FULL(0, m_pPlayer->edict(), m_usFireHuntbow, 0, (float*)&g_vecZero, (float*)&g_vecZero, vecDir.x, vecDir.y, int(m_pPlayer->pev->punchangle.x * 100), 1, m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] > 0, TRUE);
#endif
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.3s;
	m_pPlayer->pev->punchangle.x -= 2.0;
}

float CHuntbow::GetSpread(void)
{
	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		return m_flAccuracy * 0.035 + 0.1;
	else if (m_pPlayer->pev->velocity.Length2D() > 120)
		return m_flAccuracy * 0.006 + 0.02;
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		return m_flAccuracy * 0.001 + 0.0075;
	else
		return m_flAccuracy * 0.001 + 0.01;
}

void CHuntbow::HuntbowFire(float flSpread, duration_t flCycleTime, BOOL fChargeInterrupted)
{
	SetInChargeState(false);

	m_iChargeState = 0;
	m_bDelayFire = true;
	m_iShotsFired++;

	m_flAccuracy = (m_iShotsFired * m_iShotsFired * m_iShotsFired) / 240.0 + 0.1;

	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		return;

	m_pPlayer->m_iWeaponVolume = QUIET_GUN_VOLUME;
	m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
	m_pPlayer->pev->effects &= ~EF_MUZZLEFLASH;

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	Vector vecSrc = m_pPlayer->GetGunPosition() - gpGlobals->v_up * 2;
	Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 0, BULLET_PLAYER_556MM, 0, 0.0, m_pPlayer->pev, false, m_pPlayer->random_seed);

#ifndef CLIENT_DLL
	int bHit = 0;
	Vector vecArrowDir = CalcPositon(vecSrc, gpGlobals->v_forward, &bHit);
	if (bHit == 0 || bHit == 3)
		vecArrowDir = gpGlobals->v_forward + gpGlobals->v_right * vecDir.x + gpGlobals->v_up * vecDir.y;
	m_iMode = 0;
	CHuntbowArrow* pEnt = static_cast<CHuntbowArrow*>(CBaseEntity::Create("huntbow_arrow", vecSrc, UTIL_VecToAngles(vecArrowDir), m_pPlayer->edict()));
	if (pEnt)
	{
		pEnt->Init(vecArrowDir * 2000, GetPrimaryAttackDamage(), m_pPlayer->m_iTeam, 10.0, m_iMode);
	}
#endif

	int flags;

#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFireHuntbow, 0, (float*)&g_vecZero, (float*)&g_vecZero, 0, 0, 0, 0, m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] > 0, fChargeInterrupted);

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + flCycleTime;

	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		KickBack(1.0, 1.0, 0.5, 0.5, 5.0, 5.0, 5);
	else if (m_pPlayer->pev->velocity.Length2D() > 0)
		KickBack(0.3, 0.3, 0.07, 0.07, 2.0, 2.0, 4);
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		KickBack(0.2, 0.2, 0.01, 0.01, 0.5, 0.5, 4);
	else
		KickBack(0.2, 0.2, 0.01, 0.01, 0.7, 0.7, 3);
}

void CHuntbow::OnAttack2Released(void)
{
	if (m_iChargeState < 3)
	{
		HuntbowFire(GetSpread(), 1.3s, TRUE);
	}
	else
	{
		ChargedFire();
	}
}

void CHuntbow::Holster(int skiplocal)
{
#ifndef CLIENT_DLL
	MESSAGE_BEGIN(MSG_ONE, gmsgDgaxe);
	WRITE_BYTE(0);
	WRITE_BYTE(3);
	MESSAGE_END();

	for (int iIndex = 1; iIndex <= gpGlobals->maxClients; ++iIndex)
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgHeadIcon, nullptr, m_pPlayer->pev);
		WRITE_BYTE(6);
		WRITE_SHORT(iIndex);
		WRITE_BYTE(0);
		MESSAGE_END();
	}
	/*
	for (int i = 0; i < 33; i++)
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgHeadIcon, nullptr, m_pPlayer->pev);
		WRITE_BYTE(6);
		WRITE_SHORT(i);
		WRITE_BYTE(0);
		MESSAGE_END();
	}*/
#endif
	return CBasePlayerWeapon::Holster(skiplocal);
}

void CHuntbow::SecondaryAttack(void)
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
	{
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3s;

		return;
	}

	if (m_iChargeState)
	{
		if (gpGlobals->time > m_flTimeCheckCharge)
		{
			if (m_iChargeState > 1)
			{
				if (m_iChargeState > 2)
				{
					if (m_iChargeState == 3)
					{
						SendWeaponAnim(BOW_CHARGE_IDLE2);

						m_iChargeState = 4;
						m_flTimeCheckCharge = gpGlobals->time + 999999s;
						m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.43s;
					}
				}
				else
				{
					SendWeaponAnim(BOW_CHARGE_FINISH1);
					m_iChargeState = 3;
					EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_AUTO, "weapons/failnaught_charge_start_fx.wav", VOL_NORM, ATTN_NORM, 0, 94);
					m_flTimeCheckCharge = gpGlobals->time + 0.43s;
					m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.43s;
				}
			}
			else
			{
				SendWeaponAnim(BOW_CHARGE_IDLE1);

				m_iChargeState = 2;
				m_flTimeCheckCharge = gpGlobals->time + (1.0s - 0.63s);
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + (1.0s - 0.63s);
			}
		}
	}
	else
	{
		SetInChargeState(true);
		SendWeaponAnim(BOW_CHARGE_START1);

		m_flTimeCheckCharge = gpGlobals->time + 0.63s;
		m_iChargeState = 1;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.63s;
	}
}

void CHuntbow::SetInChargeState(bool state)
{
	if (m_fInCharge != state)
	{
		m_fInCharge = state;

		m_pPlayer->ResetMaxSpeed();
	}
}

#ifndef CLIENT_DLL
Vector CHuntbow::Get_ShootPosition(CBaseEntity* pevAttacker, Vector Start, int* bHit = 0)
{
	Vector end, vecDir;
	
	end = pevAttacker->pev->v_angle;
	UTIL_MakeVectors(end);
	vecDir = gpGlobals->v_forward;
	end = vecDir * 8192.0;
	end = Start + end;

	TraceResult tr;
	UTIL_TraceHull(Start, end, dont_ignore_monsters, 0, pevAttacker->edict(), &tr);
	
	if (tr.flFraction != 1.0f)
	{
		end = tr.vecEndPos;
		CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);
		{
			if (pEntity->pev->takedamage != DAMAGE_NO)
			{
				if (pEntity->IsBSPModel())
					return end;

				if (pEntity->IsPlayer() && g_pGameRules->PlayerRelationship(m_pPlayer, pEntity) == GR_TEAMMATE)
					return end;
					

				Vector vecSpot = pEntity->BodyTarget(Start);
				vecSpot.z = end.z;
				if (AngleBetweenVectors(end - Start, vecDir) > 30)
					return end;

				*bHit = 1;
			}
		}
	}

		
	return end;
}

Vector CHuntbow::CalcPositon(Vector vecSrc, Vector vecDir, int *bHit = 0)
{
	float dis = 9999.0;
	int target = 0;
	Vector vecRealDir = { 0, 0, 0 };
	for (int iIndex = 1; iIndex <= gpGlobals->maxClients; ++iIndex)
	{
		CBaseEntity* entity = UTIL_PlayerByIndex(iIndex);

		if (!entity)
			continue;

		CBasePlayer* player = static_cast<CBasePlayer*>(entity);

		if (FNullEnt(player->pev))
			continue;

		if (player == m_pPlayer)
			continue;

		if (player->IsDormant())
			continue;

		if (g_pGameRules->PlayerRelationship(m_pPlayer, player) == GR_TEAMMATE)
			continue;

		Vector vecPlayerOrigin = player->pev->origin;

		TraceResult tr;
		UTIL_TraceLine(vecSrc, vecPlayerOrigin, ignore_monsters, m_pPlayer->edict(), &tr);
		if (tr.flFraction < 1.0 )
			continue;

		if (AngleBetweenVectors(tr.vecEndPos - vecSrc, vecDir) > 6)
			continue;


		
		float fDelta = (vecPlayerOrigin - m_pPlayer->pev->origin).Length();
		if (fDelta < dis)
		{
			target = iIndex;
			dis = fDelta;
		}	
	}
	if (target)
	{
		CBaseEntity* entity = UTIL_PlayerByIndex(target);

		if(!entity)
			return vecRealDir;

		CBasePlayer* player = static_cast<CBasePlayer*>(entity);

		TraceResult tr;
		UTIL_TraceLine(vecSrc, player->pev->origin, ignore_monsters, m_pPlayer->edict(), &tr);
		if(tr.pHit != NULL && tr.pHit == player->edict())
			*bHit = 3;				//don't use auto aim 

		vecRealDir = (player->pev->origin - vecSrc).Normalize();

		if(vecRealDir.x - vecDir.Normalize().x < 0)	//LEFT
			*bHit = 1;
		else
			*bHit = 2;			//right
	}

	return vecRealDir;
}

void CHuntbow::ScannerFunc(Vector vecSrc, Vector vecDir, float flRadius, float flAngleDegrees, entvars_t* pevPlayer, int m_iChargeState)
{
	TraceResult tr;

	vecSrc.z += 1;

	Vector vecEnd = vecSrc + vecDir.Normalize() * flRadius;
	UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(pevPlayer), &tr);

	if (tr.flFraction >= 1) {
		UTIL_TraceHull(vecSrc, vecEnd, dont_ignore_monsters, head_hull, ENT(pevPlayer), &tr);

		if (tr.flFraction < 1) {
			CBaseEntity* pHit = CBaseEntity::Instance(tr.pHit);
			vecEnd = tr.vecEndPos;
		}
	}

	CBaseEntity* pEntity = nullptr;
	while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, flRadius)) != nullptr)
	{
		if (pEntity->pev->takedamage != DAMAGE_NO) 
		{
			if (pEntity->IsBSPModel())
				continue;

			if (pEntity->pev == pevPlayer)
				continue;

			Vector vecSpot = pEntity->BodyTarget(vecSrc);
			vecSpot.z = vecEnd.z;

			if (AngleBetweenVectors(vecSpot - vecSrc, vecDir) > flAngleDegrees)
				continue;

			if (!pEntity->IsPlayer())
				continue;
			
			CBasePlayer* pVictim = dynamic_cast<CBasePlayer*>(pEntity);
			if (m_iChargeState == 3 || m_iChargeState == 4)
			{
				if (pVictim->m_bIsZombie && pVictim->IsAlive()) // Zombie Scanner
				{
					MESSAGE_BEGIN(MSG_ONE, gmsgHeadIcon, nullptr, m_pPlayer->pev);
					WRITE_BYTE(6);
					WRITE_SHORT(ENTINDEX(pVictim->edict()));
					WRITE_BYTE(1);
					MESSAGE_END();
				}
			}
			else
			{
				MESSAGE_BEGIN(MSG_ONE, gmsgHeadIcon, nullptr, m_pPlayer->pev);
				WRITE_BYTE(6);
				WRITE_SHORT(ENTINDEX(pVictim->edict()));
				WRITE_BYTE(0);
				MESSAGE_END();
			}
			continue;
		}
	}
}

void CHuntbow::ItemPostFrame(void)
{
	SetWeaponModel();
	int bHit = 0;
	Vector vecSrc = m_pPlayer->GetGunPosition();
	CBaseEntity* pevAttacker = this->m_pPlayer;
	//auto vecShootPosition = Get_ShootPosition(pevAttacker, vecSrc, &bHit);
	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
	CalcPositon(vecSrc, gpGlobals->v_forward, &bHit);

	switch (bHit)
	{
	case 1:m_CrossHairState = HUNTBOW_HIT_LEFT; break;
	case 2:m_CrossHairState = HUNTBOW_HIT_RIGHT; break;
	case 3:m_CrossHairState = HUNTBOW_AIM_HIT; break;
	default:
		m_CrossHairState = HUNTBOW_AIM_IDLE;
	}
	if (m_iChargeState == 3 || m_iChargeState == 4)
	{
		m_CrossHairState = HUNTBOW_AIM_CHARGIN;
		if (m_iChargeState == 4 && m_flNextTimeCheck < gpGlobals->time)
		{
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/failnaught_charge_loop_fx.wav", VOL_NORM, ATTN_NORM, 0, 94);
			m_flNextTimeCheck = gpGlobals->time + 4.0s;
		}
	}

	UTIL_MakeVectors(m_pPlayer->pev->v_angle);
	ScannerFunc(vecSrc, gpGlobals->v_forward, 900, 180, m_pPlayer->pev, m_iChargeState);

	MESSAGE_BEGIN(MSG_ONE, gmsgDgaxe, NULL, m_pPlayer->pev);
	WRITE_BYTE(1);
	WRITE_BYTE(m_CrossHairState);
	MESSAGE_END();

	return CBasePlayerWeapon::ItemPostFrame();
}
#endif

void CHuntbow::WeaponIdle(void)
{
	ResetEmptySound();

	if (m_iChargeState && !(m_pPlayer->pev->button & IN_ATTACK2))
	{
		OnAttack2Released();
	}
		

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	SendWeaponAnim(m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 ? BOW_IDLE_EMPTY : BOW_IDLE);
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20.0s;
}

float CHuntbow::GetPrimaryAttackDamage() const
{
	float flDamage = 38;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flDamage = 72;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 72;
#endif
	return flDamage;
}

float CHuntbow::GetSecondaryAttackDamage() const
{
	float flDamage = 152.0f;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flDamage = 1555.0f;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 1555.0f;
#endif
	return flDamage;
}

}
