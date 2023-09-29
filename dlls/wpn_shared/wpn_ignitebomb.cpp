#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "wpn_hegrenade.h"
#include "soundent.h"
#include "monsters.h"
#include "decals.h"
#include "gamerules.h"
#include "cbase/cbase_hash.h"
#include "weapons/KnifeAttack.h"

#ifndef CLIENT_DLL
#include "player/player_mod_strategy.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

enum ignitebomb_e
{
	IGNITEBOMB_IDLE,
	IGNITEBOMB_PULLPIN,
	IGNITEBOMB_THROW,
	IGNITEBOMB_DRAW,
	IGNITEBOMB_BMODE,

};

LINK_ENTITY_TO_CLASS(weapon_ignitebomb, CIgniteBomb)

void CIgniteBomb::Spawn(void)
{
	m_iGrenadeID = GRENADE_IGNITEBOMB;
	Precache();
	m_iId = WEAPON_HEGRENADE;
	SET_MODEL(ENT(pev), "models/w_ignitebomb.mdl");

	m_iDefaultAmmo = HEGRENADE_DEFAULT_GIVE;
	m_flStartThrow = invalid_time_point;
	m_flReleaseThrow = invalid_time_point;
	m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;

	FallInit();
}

void CIgniteBomb::Precache(void)
{
	PRECACHE_MODEL("models/v_ignitebomb.mdl");
	PRECACHE_MODEL("models/p_ignitebomb.mdl");
	PRECACHE_MODEL("models/w_ignitebomb.mdl");
	m_iEffectModel = PRECACHE_MODEL("models/ef_ignitebomb.mdl");

	PRECACHE_MODEL("sprites/muzzleflash331.spr");
	PRECACHE_MODEL("sprites/muzzleflash332.spr");
	PRECACHE_MODEL("sprites/muzzleflash340.spr");

	PRECACHE_SOUND("weapons/ignitebomb_exp2.wav");
	PRECACHE_SOUND("weapons/ignitebomb_exp.wav");

	m_usCreateExplosion = PRECACHE_EVENT(1, "events/createexplo.sc");
}

int CIgniteBomb::GetItemInfo(ItemInfo *p)
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

BOOL CIgniteBomb::Deploy(void)
{
	m_flReleaseThrow = invalid_time_point;
	m_fMaxSpeed = 250;

	if (DefaultDeploy("models/v_ignitebomb.mdl", "models/p_ignitebomb.mdl", IGNITEBOMB_DRAW, "grenade", UseDecrement() != FALSE))
		return TRUE;

	return FALSE;
}

BOOL CIgniteBomb::CanHolster(void)
{
	return CBasePlayerWeapon::CanHolster();
}

void CIgniteBomb::Holster(int skiplocal)
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

void CIgniteBomb::PrimaryAttack(void)
{
	if (m_flStartThrow == invalid_time_point && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] > 0)
	{
		m_flStartThrow = gpGlobals->time;
		m_flReleaseThrow = invalid_time_point;

		SendWeaponAnim(IGNITEBOMB_PULLPIN, UseDecrement() != FALSE);

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5s;
	}
}

float CIgniteBomb::GetDamageB() const
{
	float flDamage = 70.0f;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flDamage = 3000.0f;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 5000.0f;
#endif
	return flDamage;
}

void CIgniteBomb::DelaySecondaryAttack()
{
	EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_ITEM, "weapons/ignitebomb_exp2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

#ifndef CLIENT_DLL

	Vector vecSrc, vecEnd;
	TraceResult tr;
	CBaseEntity* pEntity = NULL;

	UTIL_MakeVectors(m_pPlayer->pev->v_angle);

	vecSrc = m_pPlayer->GetGunPosition();

	Vector vecForward = gpGlobals->v_forward;

	vecEnd = vecSrc + vecForward * 20.0;

	UTIL_TempModel(vecEnd, UTIL_VecToAngles(vecForward), g_vecZero, m_iEffectModel, 10, 0, 10, false, 0, 255, kRenderNormal, NULL, false, 0, 0, 256, 0x20000000);

	Vector2D vecForward2D = vecForward.Make2D().Normalize();

	float c = cos(360.0 * 0.5 * M_PI / 180.0);

	while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, 310.0)) != NULL)
	{
		if (pEntity == m_pPlayer)
			continue;

		if (g_pGameRules->PlayerRelationship(m_pPlayer, pEntity) == GR_TEAMMATE)
			continue;

		if (pEntity->IsDormant())
			continue;

		if (pEntity->pev->takedamage == DAMAGE_NO)
			continue;

		if (bIsWallBetweenEntity(pEntity, m_pPlayer))
			continue;

		if (!AngleCheck(pEntity, m_pPlayer, vecSrc, vecForward, vecForward2D, 310.0, c, &tr))
			continue;

		m_pPlayer->m_iWeaponVolume = 128;

		pEntity->TakeDamage(pev, m_pPlayer->pev, GetDamageB(), DMG_EXPLOSION);

		Vector vecDirection = (pEntity->pev->origin - m_pPlayer->pev->origin).Normalize();
		UTIL_AddKickRateBoost(pEntity, vecDirection.Make2D(), 1.0, 2000.0, 200);
	}
#endif

}

void CIgniteBomb::SecondaryAttack(void)
{
	if (!m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
		return;

	SendWeaponAnim(IGNITEBOMB_BMODE, UseDecrement() != FALSE);

	DelaySecondaryAttack();

#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

	m_flReleaseThrow = gpGlobals->time;
	m_flStartThrow = invalid_time_point;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.75s;

	m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;

	m_flNextSecondaryAttack = m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.0s;
}

void CIgniteBomb::WeaponIdle(void)
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
		SendWeaponAnim(IGNITEBOMB_THROW, UseDecrement() != FALSE);

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
			SendWeaponAnim(IGNITEBOMB_DRAW, UseDecrement() != FALSE);
			m_flReleaseThrow = invalid_time_point;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + RandomDuration<float>(10s, 15s);
		}
		else
			RetireWeapon();

		return;
	}

	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
	{
		SendWeaponAnim(IGNITEBOMB_IDLE, UseDecrement() != FALSE);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
	}
}

BOOL CIgniteBomb::CanDeploy(void)
{
	return m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] != 0;
}

}
