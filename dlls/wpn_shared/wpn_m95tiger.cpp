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
#include "wpn_m95tiger.h"

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
class CM95TigerEFTiger : public CBaseEntity
{
public:
	CM95TigerEFTiger()
	{
		m_flAnimTime = invalid_time_point;
		m_flSoundTime = invalid_time_point;
		m_flRemoveTime = invalid_time_point;
		m_flDamageTime = invalid_time_point;
	}

	float GetDamage() const
	{
		float flDamage = 150.0f;
		if (g_pModRunning->DamageTrack() == DT_ZB)
			//flDamage = 7450.0f;
			flDamage = 3725.0f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 14900.0f;
		return flDamage;
	}

	void Spawn() override
	{
		Precache();

		SetThink(&CM95TigerEFTiger::FlyThink);
		SET_MODEL(this->edict(), "models/ef_m95tiger.mdl");

		pev->rendermode = kRenderTransAdd;
		pev->renderfx = kRenderFxNone;
		pev->renderamt = 255;
		pev->solid = SOLID_TRIGGER;
		pev->movetype = MOVETYPE_NOCLIP;
		pev->sequence = 0;
		pev->animtime = gpGlobals->time;
		pev->framerate = 1.0;

		m_flAnimTime = gpGlobals->time + 1.12s;
		m_flRemoveTime = gpGlobals->time + 12.0s;
		m_flDamageTime = gpGlobals->time + 0.3s;
		UTIL_SetSize(pev, g_vecZero, g_vecZero);

		pev->classname = MAKE_STRING("weapon_m95tiger");

		pev->nextthink = gpGlobals->time + 0.01s;

		PLAYBACK_EVENT_FULL(FEV_GLOBAL, this->edict(), PRECACHE_EVENT(1, "events/wpneffects.sc"), 0.0, pev->origin, g_vecZero, 0.0, 0.0, 24, 0, TRUE, FALSE);
	}

	void Precache() override
	{
		PRECACHE_MODEL("models/ef_m95tiger.mdl");

		PRECACHE_SOUND("weapons/m95tiger_tiger.wav");
	}

	void RadiusDamage()
	{
		CBaseEntity* pAttacker = CBaseEntity::Instance(pev->owner);
		CBasePlayer* pAttackePlayer = nullptr;
		if (pAttacker && pAttacker->IsPlayer())
			pAttackePlayer = static_cast<CBasePlayer*>(pAttacker);

		TraceResult tr;
		CBaseEntity* pEntity = NULL;

		Vector vecEnd;
		int bInWater = (UTIL_PointContents(pev->origin) == CONTENTS_WATER);
		float flDamage = GetDamage();
		while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, 150.0)) != NULL)
		{
			if (!pEntity->IsPlayer())
				continue;

			CBasePlayer* target = static_cast<CBasePlayer*>(pEntity);

			if (pAttackePlayer->m_iTeam == target->m_iTeam)
				continue;

			if (pev->iuser1 & (1 << pEntity->entindex()))
				continue;

			pev->iuser1 |= (1 << pEntity->entindex());

			pEntity->TakeDamage(pev, pAttackePlayer->pev, flDamage, DMG_EXPLOSION);

			PLAYBACK_EVENT_FULL(FEV_GLOBAL, this->edict(), PRECACHE_EVENT(1, "events/wpneffects.sc"), 0.0, pEntity->pev->origin, g_vecZero, 0.0, 0.0, 24, pEntity->entindex(), FALSE, FALSE);

			if (pEntity->IsPlayer())
			{
				CBasePlayer* player = (CBasePlayer*)pEntity;

				if (player->m_bIsZombie)
				{
					auto strategy = dynamic_cast<CPlayerModStrategy_ZB1*>(player->m_pModStrategy.get());
					if (strategy)
					{
						strategy->SetStunSpeedTime(3.0s);
						strategy->SetStunGravityTime(3.0s);
					}
				}
			}
		}
	}

	void EXPORT FlyThink()
	{
		pev->nextthink = gpGlobals->time + 0.01s;
		if (m_flRemoveTime <= gpGlobals->time)
		{
			Remove();
			return;
		}

		if (m_flSoundTime <= gpGlobals->time)
		{
			EMIT_SOUND_DYN(this->edict(), CHAN_ITEM, "weapons/m95tiger_tiger.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			m_flSoundTime = gpGlobals->time + 3.0s;
		}

		if (m_flAnimTime != invalid_time_point && m_flAnimTime < gpGlobals->time)
		{
			pev->sequence = 1;
			m_flAnimTime = invalid_time_point;
		}

		if (m_flDamageTime < gpGlobals->time)
		{
			RadiusDamage();
			m_flDamageTime = gpGlobals->time + 0.1s;
		}
	}

	KnockbackData GetKnockBackData() override { return { 0.0f, 0.0f, 0.0f, 0.0f, 1.0f }; }

	time_point_t m_flRemoveTime;
	time_point_t m_flAnimTime;
	time_point_t m_flDamageTime;
	time_point_t m_flSoundTime;

protected:
	void Remove()
	{
		SetThink(nullptr);
		SetTouch(nullptr);
		pev->effects |= EF_NODRAW; // 0x80u
		return UTIL_Remove(this);
	}
};

LINK_ENTITY_TO_CLASS(m95tiger_tiger, CM95TigerEFTiger)

class CM95TigerNet : public CBaseEntity
{
public:
	void Spawn() override
	{
		Precache();

		SetThink(&CM95TigerNet::FlyThink);
		SetTouch(&CM95TigerNet::OnTouch);
		SET_MODEL(this->edict(), "models/ef_m95tiger_net.mdl");

		pev->rendermode = kRenderTransAdd;
		pev->renderfx = kRenderFxNone;
		pev->renderamt = 255;
		pev->framerate = 1.0;
		pev->solid = SOLID_TRIGGER;
		pev->movetype = MOVETYPE_FLY;

		m_flRemoveTime = invalid_time_point;
		UTIL_SetSize(pev, Vector(-16.0, -16.0, -16.0), Vector(16.0, 16.0, 16.0));
		pev->classname = MAKE_STRING("m95tiger_net");//nodamage

		pev->nextthink = gpGlobals->time + 0.1s;
	}

	void Precache() override
	{
		PRECACHE_MODEL("models/ef_m95tiger_net.mdl");
	}

	void EXPORT OnTouch(CBaseEntity* pOther)
	{
		if (!pOther)
			return;

		if (pev->owner == pOther->edict())
			return;

		if(pOther->IsBSPModel())
		{
			m_flRemoveTime = gpGlobals->time + 0.1s;
			return;
		}

		CBaseEntity* pAttacker = CBaseEntity::Instance(pev->owner);
		CBasePlayer* pAttackPlayer = nullptr;
		if (pAttacker && pAttacker->IsPlayer())
			pAttackPlayer = static_cast<CBasePlayer*>(pAttacker);

		CBaseEntity* pEntity = NULL;
		if (pAttackPlayer && !pAttackPlayer->m_bIsZombie)
		{			
			TraceResult tr;

			Vector vecEnd;
			int bInWater = (UTIL_PointContents(pev->origin) == CONTENTS_WATER);

			while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, 75.0)) != NULL)
			{
				if (pEntity->IsBSPModel())
					continue;

				if (pEntity->edict() == pev->owner)
					continue;

				if (pEntity->IsDormant())
					continue;

				if (pEntity->pev->takedamage == DAMAGE_NO)
					continue;

				if (g_pGameRules->PlayerRelationship(pAttackPlayer, pEntity) == GR_TEAMMATE)
					continue;

				vecEnd = pEntity->BodyTarget(pev->origin);

				UTIL_TraceLine(pev->origin, vecEnd, ignore_monsters, pev->owner, &tr);

				if (tr.flFraction < 1.0 && tr.pHit != pEntity->edict())
					continue;

				if (pEntity->IsPlayer())
				{
					CBasePlayer* player = (CBasePlayer*)pEntity;
					player->pev->velocity = g_vecZero;
					if (player->m_bIsZombie)
					{
						auto strategy = dynamic_cast<CPlayerModStrategy_ZB1*>(player->m_pModStrategy.get());
						if (strategy)
						{
							strategy->SetStunSpeedTime(1.5s, 1);
							strategy->SetStunGravityTime(1.5s, 2.0);
						}
					}
				}

				PLAYBACK_EVENT_FULL(FEV_GLOBAL, this->edict(), PRECACHE_EVENT(1, "events/wpneffects.sc"), 0.0, pEntity->pev->origin, g_vecZero, 0.0, 0.0, 23, pEntity->entindex(), TRUE, FALSE);
			}
		}

		Remove();
	}

	void EXPORT FlyThink()
	{
		pev->nextthink = gpGlobals->time + 0.1s;
		if (m_flRemoveTime != invalid_time_point && m_flRemoveTime <= gpGlobals->time)
		{
			Remove();
			return;
		}
	}

	time_point_t m_flRemoveTime;

protected:
	void Remove()
	{
		SetThink(nullptr);
		SetTouch(nullptr);
		pev->effects |= EF_NODRAW; // 0x80u
		return UTIL_Remove(this);
	}
};

LINK_ENTITY_TO_CLASS(m95tiger_net, CM95TigerNet)
#endif
LINK_ENTITY_TO_CLASS(weapon_m95tiger, CM95Tiger)

void CM95Tiger::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_m95tiger");

	Precache();
	m_iId = WEAPON_M95TIGER;
	SET_MODEL(ENT(pev), "models/w_m95tiger.mdl");
	m_iDefaultAmmo2 = 7;
#ifndef CLIENT_DLL
	m_iHitCount = 0;
	m_flNextCheckHitCount = invalid_time_point;
#endif
	m_iDefaultAmmo = M95TIGER_MAX_CLIP;
	FallInit();
}

void CM95Tiger::Precache(void)
{
	PRECACHE_MODEL("models/v_m95tiger.mdl");
	PRECACHE_MODEL("models/p_m95tiger.mdl");
	PRECACHE_MODEL("models/w_m95tiger.mdl");

	PRECACHE_SOUND("weapons/m95tiger-3.wav");

	PRECACHE_MODEL("sprites/muzzleflash80.spr");
	PRECACHE_MODEL("sprites/muzzleflash59.spr");

	m_iShellId = m_iShell = PRECACHE_MODEL("models/rshell_big.mdl");
	m_usFireM95Tiger = PRECACHE_EVENT(1, "events/m95tiger.sc");
}

int CM95Tiger::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "50BMG";
	p->iMaxAmmo1 = MAX_AMMO_50BMG;
	p->pszAmmo2 = "M95TigerNet";
	p->iMaxAmmo2 = 7;
	p->iMaxClip = M95TIGER_MAX_CLIP;
	p->iSlot = 0;
	p->iPosition = 2;
	p->iId = m_iId = WEAPON_M95TIGER;
	p->iFlags = 0;
	p->iWeight = AWP_WEIGHT;

	return 1;
}

int CM95Tiger::ExtractAmmo(CBasePlayerWeapon* pWeapon)
{
	if (m_iDefaultAmmo2)
	{
		m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = m_iDefaultAmmo2;
		m_iDefaultAmmo2 = 0;
	}
	return CBasePlayerWeapon::ExtractAmmo(pWeapon);
}

BOOL CM95Tiger::Deploy(void)
{
#ifndef CLIENT_DLL
	PLAYBACK_EVENT_FULL(FEV_HOSTONLY, ENT(m_pPlayer->pev), m_usFireM95Tiger, 0, g_vecZero, g_vecZero, 0.0, 0.0, m_iHitCount >= M95TIGER_CHARGECOUNT ? 2 : 1, 0, TRUE, TRUE);

	m_pPlayer->UpdateShieldCrosshair(false);
#endif
	if (DefaultDeploy("models/v_m95tiger.mdl", "models/p_m95tiger.mdl", M95TIGER_DRAW, "rifle", UseDecrement() != FALSE, 0.75s, 1.0s))
	{
		m_flNextPrimaryAttack = m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 1.25s;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3s;
		return TRUE;
	}

	return FALSE;
}

void CM95Tiger::SecondaryAttack(void)
{
	switch (m_pPlayer->m_iFOV)
	{
		case 90: 
			m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 35; 
#ifndef CLIENT_DLL
			m_pPlayer->UpdateShieldCrosshair(true);
#endif
			break;
		default: 
			m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 90; 
#ifndef CLIENT_DLL
			m_pPlayer->UpdateShieldCrosshair(false);
#endif
			break;
	}

	EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/zoom.wav", 0.2, 2.4);
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3s;
}

void CM95Tiger::PrimaryAttack(void)
{
	if (!m_pPlayer->m_bResumeZoom && m_pPlayer->pev->fov == 90)
	{
#ifndef CLIENT_DLL
		if (m_flNextShootNetAttack <= UTIL_WeaponTimeBase())
		{
			M95TigerFireNet();
			m_flNextShootNetAttack = UTIL_WeaponTimeBase() + 1.9s;
		}
#endif
	}
	else
	{
		if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			M95TigerFire(0.01, 1.83s, FALSE);
		else if (m_pPlayer->pev->velocity.Length2D() > 120)
			M95TigerFire(0.01, 1.83s, FALSE);
		else if (m_pPlayer->pev->velocity.Length2D() > 10)
			M95TigerFire(0.005, 1.83s, FALSE);
		else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
			M95TigerFire(0.0, 1.83s, FALSE);
		else
			M95TigerFire(0.0005, 1.83s, FALSE);	
	}
}

void CM95Tiger::M95TigerFireNet()
{
	if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] <= 0)
		return;

	m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]--;
	m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	Vector vecSrc = m_pPlayer->GetGunPosition();
	CM95TigerNet* pEnt = static_cast<CM95TigerNet*>(CBaseEntity::Create("m95tiger_net", vecSrc, UTIL_VecToAngles(gpGlobals->v_forward), ENT(m_pPlayer->pev)));
	if (pEnt)
	{
		pEnt->pev->velocity = gpGlobals->v_forward * 1000;
	}
#endif

	PLAYBACK_EVENT_FULL(0, ENT(m_pPlayer->pev), m_usFireM95Tiger, 0, g_vecZero, g_vecZero, 0.0, 0.0, 0, 0, FALSE, TRUE);
}

void CM95Tiger::M95TigerFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
{
	if (m_pPlayer->pev->fov != 90)
	{
		m_pPlayer->m_bResumeZoom = true;
#ifndef CLIENT_DLL
		m_pPlayer->UpdateShieldCrosshair(m_pPlayer->m_bResumeZoom);
#endif
		m_pPlayer->m_iLastZoom = m_pPlayer->m_iFOV;
		m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 90;
	}
	else // not zoomed, so bullets diverts more
		flSpread += 0.01;

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

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	m_pPlayer->m_flEjectBrass = gpGlobals->time + 0.55s;
	m_pPlayer->m_iWeaponVolume = BIG_EXPLOSION_VOLUME;
	m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
	if(m_iHitCount < M95TIGER_CHARGECOUNT)
		AddHitCountToLineTargets(m_pPlayer->GetGunPosition(), gpGlobals->v_forward);
#endif
	
	Vector vecDir = m_pPlayer->FireBullets3(m_pPlayer->GetGunPosition(), gpGlobals->v_forward, flSpread, 8192, 9, BULLET_PLAYER_338MAG, GetDamage(), M95TIGER_ARMOR_RATIO_MODIFIER, m_pPlayer->pev, TRUE, m_pPlayer->random_seed);

	PLAYBACK_EVENT_FULL(0, ENT(m_pPlayer->pev), m_usFireM95Tiger, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.x * 100), FALSE, FALSE);
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;
	m_pPlayer->pev->punchangle.x -= 12;
}

#ifndef CLIENT_DLL
void CM95Tiger::AddHitCountToLineTargets(Vector vecSrc, Vector vecDir)
{
	TraceResult tr;

	UTIL_TraceLine(vecSrc, vecSrc + vecDir * 8192.0, dont_ignore_monsters, m_pPlayer->edict(), &tr);

	if (tr.flFraction < 1.0 && !FNullEnt(tr.pHit))
	{
		CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);
		if (pEntity && pEntity->IsPlayer() && g_pGameRules->PlayerRelationship(m_pPlayer, pEntity) != GR_TEAMMATE)
		{
			m_iHitCount++;
			if (m_iHitCount >= M95TIGER_CHARGECOUNT)
			{
				PLAYBACK_EVENT_FULL(FEV_HOSTONLY, ENT(m_pPlayer->pev), m_usFireM95Tiger, 0, g_vecZero, g_vecZero, 0.0, 0.0, 2, 0, TRUE, TRUE);

				MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->pev);
				WRITE_BYTE(1);
				WRITE_BYTE(1);
				MESSAGE_END();
			}	
		}
	}
}
#endif

void CM95Tiger::ItemPostFrame(void)
{
	m_flNextShootNetAttack -= 0.01s;

	if (m_flNextShootNetAttack <= 0.0s)
		m_flNextShootNetAttack = 0.0s;

#ifndef CLIENT_DLL
	if (m_pPlayer->pev->button & IN_ATTACK && m_pPlayer->pev->button & IN_ATTACK2 && m_iHitCount >= M95TIGER_CHARGECOUNT)
	{
		m_pPlayer->pev->effects |= EF_MUZZLEFLASH;

		m_pPlayer->SetAnimation(PLAYER_ATTACK1);

		UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
		Vector vecDir = gpGlobals->v_forward;
		Vector vecSrc = m_pPlayer->pev->origin - vecDir * 100.0;

		CM95TigerEFTiger* pEnt = static_cast<CM95TigerEFTiger*>(CBaseEntity::Create("m95tiger_tiger", vecSrc, UTIL_VecToAngles(gpGlobals->v_forward), ENT(m_pPlayer->pev)));
		if (pEnt)
		{
			pEnt->pev->velocity = gpGlobals->v_forward * 500;
		}

		m_iHitCount = 0;
		PLAYBACK_EVENT_FULL(FEV_HOSTONLY, ENT(m_pPlayer->pev), m_usFireM95Tiger, 0, g_vecZero, g_vecZero, 0.0, 0.0, 1, 0, TRUE, TRUE);

		MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->pev);
		WRITE_BYTE(1);
		WRITE_BYTE(0);
		MESSAGE_END();

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0s;
		SendWeaponAnim(M95TIGER_RELOADB, UseDecrement() != FALSE);

		EMIT_SOUND_DYN(this->edict(), CHAN_ITEM, "weapons/m95tiger-3.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

		m_flNextShootNetAttack = UTIL_WeaponTimeBase() + 1.9s;
	}
#endif
	return CBasePlayerWeapon::ItemPostFrame();
}

void CM95Tiger::Reload(void)
{
	if (DefaultReload(M95TIGER_MAX_CLIP, M95TIGER_RELOADA, M95TIGER_RELOAD_TIME))
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
		if (m_pPlayer->pev->fov != 90)
		{
			m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 10;
			SecondaryAttack();
		}
	}
}

void CM95Tiger::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	if (m_iClip)
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 60s;
		SendWeaponAnim(M95TIGER_IDLE, UseDecrement() != FALSE);
	}
}

float CM95Tiger::GetMaxSpeed(void)
{
	return M95TIGER_MAX_SPEED;
}

float CM95Tiger::GetDamage() const
{
	float flDamage = 150.0f;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		//flDamage = 3100.0f;
		flDamage = 1100.0f;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 6200.0f;
#endif
	return flDamage;
}
}
