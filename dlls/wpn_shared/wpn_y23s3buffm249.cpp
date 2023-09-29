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
#include "wpn_buffm249.h"
#ifndef CLIENT_DLL
#include "gamemode/mod_zb1.h"
#include "gamemode/mods.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

#ifndef CLIENT_DLL
	class CY23S3BuffM249Grenade : public CBaseEntity
	{
	public:
		float GetArmorRatioModifier() { return 1.5; }
		int GetWeaponsId(void) { return WEAPON_Y23S3BUFFM249; }
		void Spawn() override
		{
			Precache();

			SetThink(&CY23S3BuffM249Grenade::FollowThink);
			SetTouch(&CY23S3BuffM249Grenade::OnTouch);
			SET_MODEL(this->edict(), "sprites/ef_y23s3buffm249_fireball.spr");

			pev->rendermode = kRenderTransAdd;
			pev->renderfx = kRenderFxNone;
			pev->renderamt = 200;
			pev->framerate = 30;
			pev->scale = 0.8;
			pev->solid = SOLID_BBOX;
			pev->movetype = MOVETYPE_FLYMISSILE;
			pev->nextthink = gpGlobals->time + 0.1s;

			m_flMaxFrames = float(MODEL_FRAMES(pev->modelindex) - 1);

			UTIL_SetOrigin(pev, pev->origin);
			UTIL_SetSize(pev, Vector(-1.0, -1.0, -1.0), Vector(1.0, 1.0, 1.0));
		}

		void Precache() override
		{
			PRECACHE_MODEL("sprites/ef_y23s3buffm249_fireball.spr");
			PRECACHE_SOUND("weapons/y23s3buffm249_exp.wav");
	
			m_iSprExplo = PRECACHE_MODEL("sprites/ef_y23s4buffm249_explosion.spr");
		}

		void EXPORT OnTouch(CBaseEntity* pOther)
		{
			CBaseEntity* pAttacker = CBaseEntity::Instance(pev->owner);
			CBasePlayer* pAttackePlayer = nullptr;
			if (pAttacker && pAttacker->IsPlayer())
				pAttackePlayer = static_cast<CBasePlayer*>(pAttacker);

			if (!pAttackePlayer)
				return;
			
			if (pOther->pev->takedamage != DAMAGE_NO && pOther->IsAlive())
			{
				Vector vecDirection = (pOther->pev->origin - pev->origin).Normalize();

				TraceResult tr;
				UTIL_TraceLine(pev->origin, pOther->pev->origin, missile, ENT(pAttackePlayer->pev), &tr);
				tr.iHitgroup = HITGROUP_CHEST; // ...

				ClearMultiDamage();
				pOther->TraceAttack(pAttackePlayer->pev, m_flTouchDamage, vecDirection, &tr, DMG_BULLET);
				ApplyMultiDamage(pev, pAttackePlayer->pev);

				if (pOther->IsPlayer())
				{
					CBasePlayer* player = (CBasePlayer*)pOther;

					if (player->m_bIsZombie)
					{
						auto strategy = dynamic_cast<CPlayerModStrategy_ZB1*>(player->m_pModStrategy.get());
						if (strategy)
						{
							strategy->SetStunSpeedTime(1.5s, 150.0);
							strategy->SetStunGravityTime(1.5s);
						}
					}
				}
			}		

			Explode(pOther);
		}

		void Explode(CBaseEntity* pOther)
		{
			EMIT_SOUND_DYN(this->edict(), CHAN_ITEM, "weapons/y23s3buffm249_exp.wav", 0.8, ATTN_NORM, 0, PITCH_NORM);

			MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
			WRITE_BYTE(TE_TEMPSPRITE);
			WRITE_COORD(pev->origin.x);
			WRITE_COORD(pev->origin.y);
			WRITE_COORD(pev->origin.z);
			WRITE_SHORT(m_iSprExplo);
			WRITE_BYTE(8);
			WRITE_BYTE(200);
			WRITE_BYTE(30);
			MESSAGE_END();

			CBaseEntity* pAttacker = CBaseEntity::Instance(pev->owner);
			CBasePlayer* pAttackePlayer = nullptr;
			if (pAttacker && pAttacker->IsPlayer())
				pAttackePlayer = static_cast<CBasePlayer*>(pAttacker);

			if (!pAttackePlayer)
				return;

			TraceResult tr;
			CBaseEntity* pEntity = NULL;

			Vector vecEnd;
			int bInWater = (UTIL_PointContents(pev->origin) == CONTENTS_WATER);
			float flRadius = 130.0;
			while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, flRadius)) != NULL)
			{
				if (pEntity->edict() == pev->owner)
					continue;

				if (pEntity == pOther)
					continue;

				if (pEntity->IsDormant())
					continue;

				if (pEntity->pev->takedamage == DAMAGE_NO)
					continue;

				if (pEntity->Classify() == CLASS_NONE)
					continue;

				if (g_pGameRules->PlayerRelationship(pAttackePlayer, pEntity) == GR_TEAMMATE)
					continue;

				vecEnd = pEntity->BodyTarget(pev->origin);

				UTIL_TraceLine(pev->origin, vecEnd, ignore_monsters, pev->owner, &tr);

				if (tr.flFraction < 1.0 && tr.pHit != pEntity->edict())
					continue;

				int falloff = RadiusDamageAmount / flRadius;

				float flCurrentDamage = max(0, RadiusDamageAmount - (pev->origin - pEntity->pev->origin).Length() * falloff);

				pEntity->TakeDamage(pev, pAttackePlayer->pev, flCurrentDamage, DMG_EXPLOSION);

				if (pEntity->IsPlayer())
				{
					CBasePlayer* player = (CBasePlayer*)pEntity;

					if (player->m_bIsZombie)
					{
						auto strategy = dynamic_cast<CPlayerModStrategy_ZB1*>(player->m_pModStrategy.get());
						if (strategy)
						{
							strategy->SetStunSpeedTime(1.5s, 150.0);
							strategy->SetStunGravityTime(1.5s);
						}
					}
				}
			}

			SetThink(nullptr);
			SetTouch(nullptr);
			UTIL_Remove(this);
		}

		void EXPORT FollowThink()
		{
			pev->frame += pev->framerate * gpGlobals->frametime / 1s;

			if (pev->frame > m_flMaxFrames)
				pev->frame = fmod(pev->frame, m_flMaxFrames);

			if (m_flRemoveTime < gpGlobals->time)
				Explode(NULL);

			pev->nextthink = gpGlobals->time + 0.01s;
		}

		void Init(Vector vecVelocity, float flTouchDamage, float flExplodeDamage, int iTeam)
		{
			m_flRemoveTime = gpGlobals->time + 3.0s;
			std::tie(m_flTouchDamage, RadiusDamageAmount, m_iTeam) = std::make_tuple(flTouchDamage, flExplodeDamage, iTeam);
			pev->velocity = std::move(vecVelocity);
		}

		KnockbackData GetKnockBackData() override { return { 0.0f, 0.0f, 0.0f, 0.0f, 0.5f }; }

		time_point_t m_flRemoveTime;
		float m_flMaxFrames;
		float m_flTouchDamage;
		float RadiusDamageAmount;
		int m_iSprExplo;

	};
	LINK_ENTITY_TO_CLASS(y23s3buffm249_grenade, CY23S3BuffM249Grenade)
#endif

LINK_ENTITY_TO_CLASS(weapon_y23s3buffm249, CY23S3BUFFM249)

#define Y23S3BUFFM249_AMMO_GIVE 100
#define Y23S3BUFFM249_MAX_CLIP2	5

void CY23S3BUFFM249::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_y23s3buffm249");

	Precache();
	m_iId = WEAPON_Y23S3BUFFM249;
	SET_MODEL(ENT(pev), "models/w_y23s3buffm249.mdl");
	m_flTimeNextCharge = gpGlobals->time;
	m_iDefaultAmmo = Y23S3BUFFM249_AMMO_GIVE;
	m_flAccuracy = 0.2;
	m_iShotsFired = 0;

	FallInit();
}

void CY23S3BUFFM249::ItemPostFrame()
{
	if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] < Y23S3BUFFM249_MAX_CLIP2)
	{
		if (m_flTimeNextCharge <= gpGlobals->time)
		{
			m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]++;
			
			if(!pev->skin)
				pev->skin = 1;

			m_flTimeNextCharge = gpGlobals->time + GetChargeTime();
		}
	}
	else
		m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = Y23S3BUFFM249_MAX_CLIP2;

	CBasePlayerWeapon::ItemPostFrame();
}

void CY23S3BUFFM249::Precache(void)
{
	PRECACHE_MODEL("models/v_y23s3buffm249.mdl");
	PRECACHE_MODEL("models/w_y23s3buffm249.mdl");
	PRECACHE_MODEL("models/p_y23s3buffm249.mdl");
	PRECACHE_MODEL("sprites/muzzleflash309.spr");
	PRECACHE_MODEL("sprites/muzzleflash337.spr");
	PRECACHE_MODEL("sprites/muzzleflash338.spr");
	PRECACHE_MODEL("sprites/muzzleflash339.spr");
	PRECACHE_MODEL("sprites/ef_y23s3buffm249_fire.spr");

	m_iShell = PRECACHE_MODEL("models/rshell.mdl");
	m_usFireY23S3BUFFM249 = PRECACHE_EVENT(1, "events/y23s3buffm249.sc");
}

int CY23S3BUFFM249::ExtractAmmo(CBasePlayerWeapon* pWeapon)
{
	m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 0;

	return CBasePlayerWeapon::ExtractAmmo(pWeapon);
}

int CY23S3BUFFM249::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "50az";
	p->iMaxAmmo1 = 200;
	p->pszAmmo2 = "y23s3buffm249";
	p->iMaxAmmo2 = Y23S3BUFFM249_MAX_CLIP2;
	p->iMaxClip = Y23S3BUFFM249_AMMO_GIVE;
	p->iSlot = 0;
	p->iPosition = 4;
	p->iId = m_iId = WEAPON_Y23S3BUFFM249;
	p->iFlags = 0;
	p->iWeight = M249_WEIGHT;

	return 1;
}

void CY23S3BUFFM249::Holster(int skiplocal)
{
	m_flTimeNextCharge = gpGlobals->time;

	return CBasePlayerWeapon::Holster(skiplocal);
}

BOOL CY23S3BUFFM249::Deploy(void)
{
	m_flAccuracy = 0.2;
	m_iShotsFired = 0;

	m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] += (gpGlobals->time - m_flTimeNextCharge) / GetChargeTime();
	m_flTimeNextCharge = gpGlobals->time + GetChargeTime();
	if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] >= Y23S3BUFFM249_MAX_CLIP2)
		m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = Y23S3BUFFM249_MAX_CLIP2;
	
	if (DefaultDeploy("models/v_y23s3buffm249.mdl", "models/p_y23s3buffm249.mdl", BUFFM249_DRAW, "m249", UseDecrement() != FALSE))
	{
		if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] > 0)
			pev->skin = 1;
		else
			pev->skin = 0;
		return TRUE;
	}

	return FALSE;
}

void CY23S3BUFFM249::PrimaryAttack(void)
{
	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		Y23S3BUFFM249Fire(0.04 + 0.4 * m_flAccuracy, 0.1s, FALSE);
	else if (m_pPlayer->pev->velocity.Length2D() > 195)
		Y23S3BUFFM249Fire(0.025 + 0.07 * m_flAccuracy, 0.1s, FALSE);
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		Y23S3BUFFM249Fire(0.0 + 0.0 * m_flAccuracy, 0.1s, FALSE);
	else
		Y23S3BUFFM249Fire(0.02 * m_flAccuracy, 0.1s, FALSE);
}

void CY23S3BUFFM249::Y23S3BUFFM249Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
{
	m_bDelayFire = true;
	m_iShotsFired++;
	m_flAccuracy = ((m_iShotsFired * m_iShotsFired * m_iShotsFired) / 190.0) + 0.3;

	if (m_flAccuracy > 0.9)
		m_flAccuracy = 0.9;

	if (m_iClip <= 0)
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2s;
		}

		return;
	}

	m_iClip--;
	m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 4, BULLET_PLAYER_556MM, GetDamage(), 0.97, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);

	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFireY23S3BUFFM249, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), FALSE, FALSE);
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.6s;


	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		KickBack(1.1, 0.47, 0.27, 0.135, 3.0, 1.5, 5);
	else
	{
		if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
			KickBack(0.4, 0.2, 0.15, 0.01, 3.0, 1.5, 5);
		else
			KickBack(0.5, 0.3, 0.2, 0.01, 3.0, 1.5, 5);
	}
	
}

void CY23S3BUFFM249::Reload(void)
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		return;

	if (DefaultReload(Y23S3BUFFM249_AMMO_GIVE, BUFFM249_RELOAD, 3.6s))
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif

		m_flAccuracy = 0.2;
		m_bDelayFire = false;
		m_iShotsFired = 0;
	}
}

void CY23S3BUFFM249::SecondaryAttack(void)
{
	if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] <= 0)
	{
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.2s;
		return;
	}

	m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]--;

	if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] < 1)
		pev->skin = 0;
	m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;

	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecDir = m_pPlayer->FireBullets3(vecSrc - gpGlobals->v_up * 2.0, gpGlobals->v_forward, 0.0, 8192.0, 0, BULLET_PLAYER_556MM, 0, 0.0, m_pPlayer->pev, false, m_pPlayer->random_seed);

#ifndef CLIENT_DLL
	Vector vecProjDir = gpGlobals->v_forward + gpGlobals->v_right * vecDir.x + gpGlobals->v_up * vecDir.y;

	CY23S3BuffM249Grenade* pEnt = static_cast<CY23S3BuffM249Grenade*>(CBaseEntity::Create("y23s3buffm249_grenade", vecSrc, m_pPlayer->pev->v_angle, ENT(m_pPlayer->pev)));
	if (pEnt)
	{
		pEnt->Init(vecProjDir * 1000, GetTouchDamage(), GetRadiusDamage(), m_pPlayer->m_iTeam);
	}
#endif

	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFireY23S3BUFFM249, 0, g_vecZero, g_vecZero, 0.0, 0.0, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), TRUE, FALSE);
	
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0s;
	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.1s;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5s;
}


void CY23S3BUFFM249::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
	SendWeaponAnim(BUFFM249_IDLE1, UseDecrement() != FALSE);
}

float CY23S3BUFFM249::GetTouchDamage() const
{
	float flDamage = 25.0f;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flDamage = 750.0f;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 1182.0f;
#endif
	return flDamage;
}

float CY23S3BUFFM249::GetRadiusDamage() const
{
	float flDamage = 75.0f;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flDamage = 2230.0f;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 4563.0f;
#endif
	return flDamage;
}

float CY23S3BUFFM249::GetDamage() const
{
	float flDamage = 25.0f;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flDamage = 145.0f;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 197.0f;
#endif
	return flDamage;
}

duration_t CY23S3BUFFM249::GetChargeTime()
{
	duration_t flTime = 3.0s;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flTime = 3.0s;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flTime = 3.0s;
#endif
	return flTime;
}

KnockbackData CY23S3BUFFM249::GetKnockBackData()
{
	return { 350.0f, 250.0f, 300.0f, 100.0f, 0.6f };
}

}
