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
#include "wpn_m3dragonex.h"
#include "weapons/KnifeAttack.h"

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

LINK_ENTITY_TO_CLASS(weapon_m3dragonex, CM3DragonEX)

void CM3DragonEX::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_m3dragonex");

	Precache();
	m_iId = WEAPON_M3DRAGONEX;
	SET_MODEL(ENT(pev), "models/w_m3dragonex.mdl");
	m_iDefaultAmmo = 15;
	m_flNextPlaySound = invalid_time_point;
	FallInit();
}

void CM3DragonEX::Precache(void)
{
	PRECACHE_MODEL("models/v_m3dragonex.mdl");
	PRECACHE_MODEL("models/w_m3dragonex.mdl");
	PRECACHE_MODEL("models/p_m3dragonex.mdl");

	PRECACHE_SOUND("weapons/m3dragonex_charge.wav");
	PRECACHE_SOUND("weapons/m3dragonex_charge_attack.wav");
	PRECACHE_SOUND("weapons/m3dragonex_charge_idle.wav");
	PRECACHE_SOUND("weapons/m3dragonex_charge_idle2.wav");

	PRECACHE_MODEL("sprites/ef_m3dragonex_L.spr");
	PRECACHE_MODEL("sprites/ef_m3dragonex_R.spr");

	PRECACHE_MODEL("sprites/ef_m3dragonex.spr");	//DRAGON mdl
	PRECACHE_MODEL("sprites/ef_m3dragonex_sign.spr");	//sign mdl

	PRECACHE_MODEL("sprites/ef_m3dragonex_shoot.spr");
	PRECACHE_MODEL("sprites/ef_m3dragonex_smoke.spr");

	PRECACHE_MODEL("sprites/muzzleflash327.spr");
	PRECACHE_MODEL("sprites/muzzleflash328.spr");
	PRECACHE_MODEL("sprites/muzzleflash329.spr");
	PRECACHE_MODEL("sprites/muzzleflash330.spr");

	m_iSignExp[0] = PRECACHE_MODEL("sprites/ef_m3dragonex_exp01.spr");
	m_iSignExp[1] = PRECACHE_MODEL("sprites/ef_m3dragonex_exp02.spr");
	m_iSignExp[2] = PRECACHE_MODEL("sprites/ef_m3dragonex_exp03.spr");
	m_iSignStart = PRECACHE_MODEL("sprites/ef_m3dragonex_start.spr");	//29
	m_iSignLoop = PRECACHE_MODEL("sprites/ef_m3dragonex_loop.spr");
	m_iSignEnd = PRECACHE_MODEL("sprites/ef_m3dragonex_end.spr");

	m_iShellId = m_iShell = PRECACHE_MODEL("models/shotgunshell.mdl");

	m_usFire = PRECACHE_EVENT(1, "events/m3dragonex.sc");
}

int CM3DragonEX::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "buckshot";
	p->iMaxAmmo1 = MAX_AMMO_BUCKSHOT;
	p->pszAmmo2 = "m3dragonex_b";
	p->iMaxAmmo2 = 2;
	p->iMaxClip = 15;
	p->iSlot = 0;
	p->iPosition = 5;
	p->iId = m_iId = WEAPON_M3DRAGONEX;
	p->iFlags = 0;
	p->iWeight = M3_WEIGHT;

	return 1;
}

int CM3DragonEX::ExtractAmmo(CBasePlayerWeapon* pWeapon)
{
	if (m_iDefaultAmmo2)
	{
		m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = m_iDefaultAmmo2;
		m_iDefaultAmmo2 = 0;
	}
	return CBasePlayerWeapon::ExtractAmmo(pWeapon);
}

void CM3DragonEX::Holster(int skiplocal)
{
	m_iState = 0;
	

#ifndef CLIENT_DLL
	auto iter = m_pEnemylist.begin();
	for (iter; iter != m_pEnemylist.end(); ++iter)
	{
		if (iter->Get() != NULL)
		{
			CBaseEntity* pEntity = *iter;

			ClearSign(pEntity, false);
		}
	}
	m_pEnemylist.clear();
	m_iTargetID.clear();
#endif
	StopIdleSound();

	return CBasePlayerWeapon::Holster(skiplocal);
}


BOOL CM3DragonEX::Deploy(void)
{
	if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType])
		m_flNextPlaySound = gpGlobals->time + 0.1s;
	return DefaultDeploy("models/v_m3dragonex.mdl", "models/p_m3dragonex.mdl", m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] > 0 ? M3DRAGONEX_SECONDARY_DRAW : M3DRAGONEX_DRAW, "shotgun", UseDecrement() != FALSE);
}

#ifndef CLIENT_DLL
void CM3DragonEX::ClearSign(CBaseEntity* pEntity, bool bExplode)
{
	MESSAGE_BEGIN(MSG_ALL, SVC_TEMPENTITY);
	WRITE_BYTE(TE_KILLENTITYATTACHMENTS);
	WRITE_SHORT(pEntity->entindex());
	WRITE_SHORT(m_iSignStart);
	MESSAGE_END();

	MESSAGE_BEGIN(MSG_ALL, SVC_TEMPENTITY);
	WRITE_BYTE(TE_KILLENTITYATTACHMENTS);
	WRITE_SHORT(pEntity->entindex());
	WRITE_SHORT(m_iSignLoop);
	MESSAGE_END();

	Vector vecDir = (pEntity->pev->origin - m_pPlayer->pev->origin).Normalize();
	Vector angle = UTIL_VecToAngles(vecDir);
	angle.x *= -1.0;

	UTIL_TempModel(pEntity->pev->origin, pev->angles, g_vecZero, bExplode ? m_iSignExp[RANDOM_LONG(0, 2)] : m_iSignEnd, 10, 0, 255, 0, 0, 255, kRenderTransAdd, NULL, false, 0, 5, bExplode ? 20 : 29, 0);
}

void CM3DragonEX::CheckEnemylist()
{
	m_pEnemylist.erase(std::remove_if(m_pEnemylist.begin(), m_pEnemylist.end(), [=](CBaseEntity* pEntity) {

		if(!pEntity)
		{
			ClearSign(pEntity, false);
			return true;
		}

		if(!pEntity->IsAlive())
		{
			ClearSign(pEntity, false);
			return true;
		}

		if (bIsWallBetweenEntity(pEntity, m_pPlayer))
		{
			ClearSign(pEntity, false);
			return true;
		}
			
		if ((m_pPlayer->pev->origin - pEntity->pev->origin).Length() > 318.96)
		{
			ClearSign(pEntity, false);
			return true;
		}

		return false;
		}), m_pEnemylist.end());
}

bool CM3DragonEX::SecondaryAttack_CheckTargetAvailable(CBaseEntity* a2)
{
	if (!a2->IsPlayer() && a2->Classify() != CLASS_PLAYER_ALLY)
		return false;

	if (!a2->IsAlive())
		return false;

	if (g_pGameRules->PlayerRelationship(m_pPlayer, a2) == GR_TEAMMATE)
		return false;

	if (a2->pev == m_pPlayer->pev)
		return false;

	if (bIsWallBetweenEntity(a2, m_pPlayer))
		return false;

	return true;
}

void CM3DragonEX::SecondaryAttack_FindTargets()
{
	EHANDLE eh;
	CBaseEntity* pEntity = NULL;
	CBaseEntity* pNearest = NULL;
	while ((pEntity = UTIL_FindEntityInSphere(pEntity, m_pPlayer->pev->origin, 314.96)) != nullptr)
	{

		eh.Set(pEntity->edict());

		if (std::find(m_pEnemylist.begin(), m_pEnemylist.end(), pEntity) != m_pEnemylist.end())
			continue;

		if (SecondaryAttack_CheckTargetAvailable(pEntity))
		{
			if (!pNearest)
				pNearest = pEntity;

			if ((pNearest->pev->origin - m_pPlayer->pev->origin).Length() > (pEntity->pev->origin - m_pPlayer->pev->origin).Length())
				pNearest = pEntity;
		}
	}

	
	if (pNearest)
	{
		eh.Set(pNearest->edict());

		m_pEnemylist.push_back(eh);

		//MpToCl
		MESSAGE_BEGIN(MSG_ALL, gmsgMPToCL, NULL);
		WRITE_BYTE(39);
		WRITE_BYTE(1);
		WRITE_SHORT(pNearest->entindex());
		WRITE_BYTE(m_pPlayer->entindex());
		MESSAGE_END();

		EMIT_SOUND(ENT(pev), CHAN_ITEM, "weapons/m3dragonex_charge.wav", VOL_NORM, ATTN_NORM);

		if (std::find(m_iTargetID.begin(), m_iTargetID.end(), pNearest->entindex()) != m_iTargetID.end())
			return;

		m_iTargetID.push_back(pNearest->entindex());

		if (pNearest->IsPlayer())
		{
			CBasePlayer* player = (CBasePlayer*)pNearest;
			if (player->m_bIsZombie && !(player->m_iBuff & BUFF_IGNORE_KNOCKBACK))
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
#endif

void CM3DragonEX::M3DragonEXShootCannon()
{
	if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] <= 0)
	{
		PrimaryAttack();
		return;
	}
		
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

	SendWeaponAnim(M3DRAGONEX_SHOOT_SPECIAL, UseDecrement() != FALSE);

	EMIT_SOUND_DYN(ENT(this->pev), CHAN_AUTO, "weapons/m3dragonex-3.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

	m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]--;

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.875s;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.875s;

	if (m_iClip)
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.5s;
	else
		m_flTimeWeaponIdle = 0.875s;

	m_flNextPlaySound = invalid_time_point;
#ifndef CLIENT_DLL
	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
	Vector vecSrc = m_pPlayer->GetGunPosition();
	CM3DragonCannon* pCannon = CM3DragonCannon::Create(2, vecSrc, m_pPlayer->pev->v_angle, ENT(m_pPlayer->pev), m_pPlayer->m_iTeam);

	if (pCannon)
	{
		pCannon->pev->velocity = gpGlobals->v_forward * 1000;
	}
#endif
}

void CM3DragonEX::SecondaryAttack_SignExplode()
{
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

	SendWeaponAnim(m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] > 0  ? M3DRAGONEX_CHARGE_ATTACKB : M3DRAGONEX_CHARGE_ATTACK, UseDecrement() != FALSE);

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.875s;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.875s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;

	EMIT_SOUND(ENT(pev), CHAN_ITEM, "weapons/m3dragonex_charge_attack.wav", VOL_NORM, ATTN_NORM);

	EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "weapons/m3dragonex-3.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

#ifndef CLIENT_DLL
	auto iter = m_pEnemylist.begin();
	for (iter; iter != m_pEnemylist.end(); ++iter)
	{
		if (iter->Get() != NULL)
		{
			CBaseEntity* pEntity = *iter;

			ClearSign(pEntity, true);

			Vector vecDirection = (pEntity->pev->origin - m_pPlayer->pev->origin).Normalize();

			TraceResult tr;

			UTIL_TraceLine(m_pPlayer->pev->origin, pEntity->pev->origin, dont_ignore_monsters, m_pPlayer->edict(), &tr);

			ClearMultiDamage();
			tr.iHitgroup = HITGROUP_CHEST;
			pEntity->TraceAttack(m_pPlayer->pev, GetDamageC(), vecDirection, &tr, DMG_NEVERGIB | DMG_BULLET);
			ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);
		}
	}
	m_pEnemylist.clear();
	m_iTargetID.clear();
#endif
}

void CM3DragonEX::ItemPostFrame(void)
{
	if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] && m_flNextPlaySound != invalid_time_point && m_flNextPlaySound < gpGlobals->time)
	{
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/m3dragon_fire_loop.wav", 0.8, ATTN_NORM, 0, PITCH_NORM);
		m_flNextPlaySound = gpGlobals->time + 1.0s;
	}

	return CBasePlayerWeapon::ItemPostFrame();
}

void CM3DragonEX::SecondaryAttack()
{

	if (!m_iState)
	{
		SendWeaponAnim(m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] > 0 ? M3DRAGONEX_CHARGE_STARTB : M3DRAGONEX_CHARGE_START, UseDecrement() != FALSE);
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 9999s;
		m_flTimeCheckCharge = gpGlobals->time + 0.37s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.7s;
		m_iState = 1;
	}
	else
	{
#ifndef CLIENT_DLL
		CheckEnemylist();
#endif
		if (m_flTimeCheckCharge < gpGlobals->time)
		{
			switch (m_iState)
			{
			case 1:
			{
				SendWeaponAnim(m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] > 0 ? M3DRAGONEX_CHARGE_IDLEB : M3DRAGONEX_CHARGE_IDLE, UseDecrement() != FALSE);
				m_iState = 2;

				m_flTimeCheckCharge = gpGlobals->time + 0.33s;
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 9999s;
				break;
			}
			case 2:
			{
				m_flTimeCheckCharge = gpGlobals->time + 0.1s;
#ifndef CLIENT_DLL
				SecondaryAttack_FindTargets();
#endif
				break;
			}
			}
		}
	}
}

void CM3DragonEX::OnAttack2Released(void)
{
	switch (m_iState)
	{
	case 1:
	{
#ifndef CLIENT_DLL	
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

		M3DragonEXShootCannon();
		break;
	}
	case 2:
	{
#ifndef CLIENT_DLL	
		SecondaryAttack_SignExplode();
#endif
		break;
	}
	}
	m_iState = 0;
}

void CM3DragonEX::PrimaryAttack(void)
{
	if (m_pPlayer->pev->waterlevel == 3)
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15s;
		return;
	}

	if (m_iClip <= 0)
	{
		Reload();

		if (m_iClip == 0)
			PlayEmptySound();

		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.0s;
		return;
	}

	m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	m_iClip--;
	m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
#ifndef CLIENT_DLL
	m_pPlayer->FireBullets(10, m_pPlayer->GetGunPosition(), gpGlobals->v_forward, Vector(0.0675, 0.0675, 0), 3000, BULLET_PLAYER_BUCKSHOT, 0, GetDamage());

	if (CBaseEntity* pEntity = gMultiDamage.pEntity)
	{
		if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] < 2)
		{
			if (pEntity->IsPlayer() || pEntity->Classify() == CLASS_PLAYER_ALLY)
			{
				pev->iuser1 += 1;
			}
		}	
	}
#endif

	if (pev->iuser1 >= 8)
	{
		m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]++;
#ifndef CLIENT_DLL
		MESSAGE_BEGIN(MSG_ONE, gmsgAmmoPickup, NULL, m_pPlayer->pev);
		WRITE_BYTE(CBasePlayer::GetAmmoIndex("m3dragonexshot"));
		WRITE_BYTE(1);
		MESSAGE_END();
#endif
		pev->iuser1 = 0;
		m_flNextPlaySound = gpGlobals->time + 0.1s;
	}

	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

#ifndef CLIENT_DLL
	Vector vecDir = KnifeAttack2(m_pPlayer->GetGunPosition(), gpGlobals->v_forward, GetDamageB(), 393.7f, 45.0f, DMG_BULLET, this->pev, m_pPlayer->pev);
#else
	Vector vecDir = gpGlobals->v_forward;
#endif

	PLAYBACK_EVENT_FULL(0, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), FALSE, m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] > 0);

	if (m_iClip)
		m_flPumpTime = UTIL_WeaponTimeBase() + 0.5s;

#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
	if (m_iClip)
		m_flPumpTime = UTIL_WeaponTimeBase() + 0.5s;

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.875s;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.875s;

	if (m_iClip)
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.5s;
	else
		m_flTimeWeaponIdle = 0.875s;

	m_fInSpecialReload = 0;

	if (m_pPlayer->pev->flags & FL_ONGROUND)
		m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 4, 6);
	else
		m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 8, 11);

	m_pPlayer->m_flEjectBrass = gpGlobals->time + 0.45s;
}

void CM3DragonEX::StopIdleSound()
{
#ifndef CLIENT_DLL
	int player = m_pPlayer->entindex();

	MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->edict());
	WRITE_BYTE(79);
	WRITE_SHORT(player);
	WRITE_SHORT(CHAN_WEAPON);
	WRITE_STRING("weapons/m3dragonex_charge_idle.wav");
	MESSAGE_END();

	MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->edict());
	WRITE_BYTE(79);
	WRITE_SHORT(player);
	WRITE_SHORT(CHAN_WEAPON);
	WRITE_STRING("weapons/m3dragonex_charge_idle2.wav");
	MESSAGE_END();
#endif
}

void CM3DragonEX::Reload(void)
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 || m_iClip == M3DRAGONEX_MAX_CLIP)
		return;

	if (m_flNextPrimaryAttack > UTIL_WeaponTimeBase())
		return;

	if (!m_fInSpecialReload)
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
		SendWeaponAnim(m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] > 0 ? M3DRAGONEX_SECONDARY_BEFORE_RELOAD : M3DRAGONEX_BEFORE_RELOAD, UseDecrement() != FALSE);

		m_fInSpecialReload = 1;
		m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.53s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.53s;
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.53s;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.53s;
	}
	else if (m_fInSpecialReload == 1)
	{
		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;

		m_fInSpecialReload = 2;
		SendWeaponAnim(m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] > 0 ? M3DRAGONEX_SECONDARY_INSERT : M3DRAGONEX_INSERT, UseDecrement() != FALSE);

		m_flNextReload = UTIL_WeaponTimeBase() + 0.3s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.3s;
	}
	else
	{
		m_iClip++;
		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;
		m_fInSpecialReload = 1;
		m_pPlayer->ammo_buckshot--;
	}
}

void CM3DragonEX::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);

	if (m_iState && !(m_pPlayer->pev->button & IN_ATTACK2))
		OnAttack2Released();

	if (m_flPumpTime != zero_duration && m_flPumpTime < UTIL_WeaponTimeBase())
		m_flPumpTime = zero_duration;

	if (m_flTimeWeaponIdle < UTIL_WeaponTimeBase())
	{
		if (m_iClip == 0 && m_fInSpecialReload == 0 && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
		{
			Reload();
		}
		else if (m_fInSpecialReload != 0)
		{
			if (m_iClip != M3DRAGONEX_MAX_CLIP && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
			{
				Reload();
			}
			else
			{
				SendWeaponAnim(m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] > 0 ? M3DRAGONEX_SECONDARY_AFTER_RELOAD : M3DRAGONEX_AFTER_RELOAD, UseDecrement() != FALSE);

				m_fInSpecialReload = 0;
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;
			}
		}
		else
		{
			SendWeaponAnim(m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] > 0 ? M3DRAGONEX_SECONDARY_READY : M3DRAGONEX_IDLE, UseDecrement() != FALSE);
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20.0s;
		}
	}
}

int CM3DragonEX::GetDamage()
{
	int flDamage = 6;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
	{
		if (enhancedzombie.value)
			flDamage = 140;
		else
			flDamage = 110;
	}		
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 200;
#endif
	return flDamage;
}

float CM3DragonEX::GetDamageB()
{
	float flDamage = 5;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
	{
		flDamage = 900;
	}
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 900;
#endif
	return flDamage;
}

float CM3DragonEX::GetDamageC()
{
	float flDamage = 5;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
	{
		flDamage = 5000;
	}
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 5000;
#endif
	return flDamage;
}

}
