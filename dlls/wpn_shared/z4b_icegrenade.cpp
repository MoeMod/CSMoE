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

LINK_ENTITY_TO_CLASS(z4b_icegrenade, CZ4B_IceGrenade)

enum icegrenade_e
{
	ICEGRENADE_IDLE,
	ICEGRENADE_PULLPIN,
	ICEGRENADE_THROW,
	ICEGRENADE_DRAW,
	ICEGRENADE_RUN,
};

void CZ4B_IceGrenade::Spawn(void)
{
	m_iGrenadeID = GRENADE_ICEGRENADE;
	Precache();
	m_iId = WEAPON_HEGRENADE;
	SET_MODEL(ENT(pev), "models/z4b/pw_icegrenade.mdl");

	m_iDefaultAmmo = HEGRENADE_DEFAULT_GIVE;
	m_flStartThrow = invalid_time_point;
	m_flReleaseThrow = invalid_time_point;
	m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;

	FallInit();
}

void CZ4B_IceGrenade::Precache(void)
{
	PRECACHE_MODEL("models/z4b/v_icegrenade.mdl");
	PRECACHE_MODEL("models/z4b/pw_icegrenade.mdl");

	m_usCreateExplosion = PRECACHE_EVENT(1, "events/createexplo.sc");
}

int CZ4B_IceGrenade::GetItemInfo(ItemInfo *p)
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

BOOL CZ4B_IceGrenade::Deploy(void)
{
	m_flReleaseThrow = invalid_time_point;
	m_fMaxSpeed = 250;
	ResetRunIdle();

	return DefaultDeploy("models/z4b/v_icegrenade.mdl", "models/z4b/pw_icegrenade.mdl", ICEGRENADE_DRAW, "grenade", UseDecrement() != FALSE);
}

BOOL CZ4B_IceGrenade::CanHolster(void)
{
	return CBasePlayerWeapon::CanHolster();
}

void CZ4B_IceGrenade::Holster(int skiplocal)
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

void CZ4B_IceGrenade::PrimaryAttack(void)
{
	if (m_flStartThrow == invalid_time_point && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] > 0)
	{
		m_flStartThrow = gpGlobals->time;
		m_flReleaseThrow = invalid_time_point;
		SendWeaponAnim(ICEGRENADE_PULLPIN, UseDecrement() != FALSE);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5s;
		ResetRunIdle();
	}
}

void CZ4B_IceGrenade::WeaponIdle(void)
{
	if (m_flReleaseThrow == invalid_time_point && m_flStartThrow != invalid_time_point)
		m_flReleaseThrow = gpGlobals->time;

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	if (m_flStartThrow != invalid_time_point)
	{	
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

		SendWeaponAnim(ICEGRENADE_THROW, UseDecrement() != FALSE);
		ResetRunIdle();
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
			SendWeaponAnim(ICEGRENADE_DRAW, UseDecrement() != FALSE);
			m_flReleaseThrow = invalid_time_point;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + RandomDuration<float>(10s, 15s);
			ResetRunIdle();
		}
		else
			RetireWeapon();

		return;
	}

	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
	{
		if (m_flTimeWeaponIdle < UTIL_WeaponTimeBase())
		{
			if (m_pPlayer->pev->button & (IN_FORWARD | IN_MOVELEFT | IN_MOVERIGHT | IN_BACK))
			{
				if (m_flNextRunIdle <= UTIL_WeaponTimeBase())
				{
					SendWeaponAnim(ICEGRENADE_RUN, UseDecrement() != FALSE);

					m_flNextRunIdle = UTIL_WeaponTimeBase() + 0.6s;
				}

				m_flNextIdle = UTIL_WeaponTimeBase();
			}
			else
			{
				if (m_flNextIdle <= UTIL_WeaponTimeBase())
				{
					SendWeaponAnim(ICEGRENADE_IDLE, UseDecrement() != FALSE);

					m_flNextIdle = UTIL_WeaponTimeBase() + 20s;
				}

				m_flNextRunIdle = UTIL_WeaponTimeBase();
			}

			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.1s;
		}
	}
}

BOOL CZ4B_IceGrenade::CanDeploy(void)
{
	return m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] != 0;
}

}
