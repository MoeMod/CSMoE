#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "wpn_hegrenade.h"

#include "monsters.h"
#include "decals.h"
#include "gamerules.h"
#include "cbase/cbase_hash.h"

#ifndef CLIENT_DLL
#include "player/player_mod_strategy.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

LINK_ENTITY_TO_CLASS(weapon_waterbomb, CWaterBomb)

void CWaterBomb::Spawn(void)
{
	Precache();
	m_iId = WEAPON_HEGRENADE;
	SET_MODEL(ENT(pev), "models/w_waterbomb.mdl");

	m_iDefaultAmmo = HEGRENADE_DEFAULT_GIVE;
	m_flStartThrow = invalid_time_point;
	m_flReleaseThrow = invalid_time_point;
	m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;

	FallInit();
}

void CWaterBomb::Precache(void)
{
	PRECACHE_MODEL("models/v_waterbomb.mdl");
	PRECACHE_MODEL("models/p_waterbomb.mdl");
	PRECACHE_MODEL("models/w_waterbomb.mdl");

	PRECACHE_SOUND("weapons/waterbomb_exp.wav");

	PRECACHE_MODEL("sprites/ef_waterbomb1.spr");
	PRECACHE_MODEL("sprites/ef_waterbomb2.spr");

	m_usCreateExplosion = PRECACHE_EVENT(1, "events/createexplo.sc");
}

int CWaterBomb::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "HEGrenade";
	p->iMaxAmmo1 = MAX_AMMO_HEGRENADE;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 3;
	p->iPosition = 1;
	p->iId = m_iId = WEAPON_HEGRENADE;
	p->iWeight = HEGRENADE_WEIGHT;
	p->iFlags = ITEM_FLAG_LIMITINWORLD | ITEM_FLAG_EXHAUSTIBLE;

	return 1;
}

BOOL CWaterBomb::Deploy(void)
{
	m_flReleaseThrow = invalid_time_point;
	m_fMaxSpeed = 250;

	return DefaultDeploy("models/v_waterbomb.mdl", "models/p_waterbomb.mdl", HEGRENADE_DRAW, "grenade", UseDecrement() != FALSE);
}

BOOL CWaterBomb::CanHolster(void)
{
	return CBasePlayerWeapon::CanHolster();
}

void CWaterBomb::Holster(int skiplocal)
{
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5s;

	if (!m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
	{
		m_pPlayer->RemoveWeapon(m_iId);
		DestroyItem();
	}

	m_flStartThrow = invalid_time_point;
	m_flReleaseThrow = invalid_time_point;
}

void CWaterBomb::PrimaryAttack(void)
{
	if (m_flStartThrow == invalid_time_point && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] > 0)
	{
		m_flStartThrow = gpGlobals->time;
		m_flReleaseThrow = invalid_time_point;
		SendWeaponAnim(HEGRENADE_PULLPIN, UseDecrement() != FALSE);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5s;
	}
}

void CWaterBomb::WeaponIdle(void)
{
	if (m_flReleaseThrow == invalid_time_point && m_flStartThrow != invalid_time_point)
		m_flReleaseThrow = gpGlobals->time;

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	if (m_flStartThrow != invalid_time_point)
	{	
#ifndef CLIENT_DLL
		m_pPlayer->Radio("%!MRAD_FIREINHOLE", "#Fire_in_the_hole");
#endif
		Vector angThrow = m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle;

		if (angThrow.x < 0)
			angThrow.x = -10 + angThrow.x * ((90 - 10) / 90.0);
		else
			angThrow.x = -10 + angThrow.x * ((90 + 10) / 90.0);

		float flVel = 750;

		UTIL_MakeVectors(angThrow);
		Vector vecSrc = m_pPlayer->pev->origin + m_pPlayer->pev->view_ofs + gpGlobals->v_forward * 16;
		Vector vecThrow = gpGlobals->v_forward * flVel + m_pPlayer->pev->velocity;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.75s;

		auto time = 1.5s;
		CGrenade::ShootTimed2(m_pPlayer->pev, vecSrc, vecThrow, time, m_pPlayer->m_iTeam, m_usCreateExplosion);

#ifndef CLIENT_DLL
		m_pPlayer->m_pModStrategy->ShootGrenade(std::bind(CGrenade::ShootTimed2, m_pPlayer->pev, vecSrc, vecThrow, time, m_pPlayer->m_iTeam, m_usCreateExplosion));
#endif
		SendWeaponAnim(HEGRENADE_THROW, UseDecrement() != FALSE);
		SetPlayerShieldAnim();

#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
		m_flStartThrow = invalid_time_point;
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.5s;
		
		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;

		if (!m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
			m_flNextSecondaryAttack = m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.5s;

		return;
	}
	else if (m_flReleaseThrow != invalid_time_point)
	{
		m_flStartThrow = invalid_time_point;

		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
		{
			SendWeaponAnim(HEGRENADE_DRAW, UseDecrement() != FALSE);
			m_flReleaseThrow = invalid_time_point;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + RandomDuration<float>(10s, 15s);
		}
		else
			RetireWeapon();

		return;
	}

	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
	{
		if (m_pPlayer->HasShield() != false)
		{
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20.0s;

			if (m_iWeaponState & WPNSTATE_SHIELD_DRAWN)
				SendWeaponAnim(SHIELDREN_IDLE, UseDecrement() != FALSE);

			return;
		}

		SendWeaponAnim(HEGRENADE_IDLE, UseDecrement() != FALSE);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + RandomDuration<float>(10s, 15s);
	}
}

BOOL CWaterBomb::CanDeploy(void)
{
	return m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] != 0;
}

}
