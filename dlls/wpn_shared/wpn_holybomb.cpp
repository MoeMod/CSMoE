#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "wpn_holybomb.h"

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

LINK_ENTITY_TO_CLASS(weapon_holybomb, CHolyBomb)

void CHolyBomb::Spawn(void)
{
	Precache();
	m_iId = WEAPON_HEGRENADE;
	SET_MODEL(ENT(pev), "models/w_holybomb.mdl");

	m_iDefaultAmmo = HEGRENADE_DEFAULT_GIVE;
	m_flStartThrow = invalid_time_point;
	m_flReleaseThrow = invalid_time_point;
	m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;

	FallInit();
}

void CHolyBomb::Precache(void)
{
	PRECACHE_MODEL("models/v_holybomb.mdl");
	PRECACHE_MODEL("models/p_holybomb.mdl");
	PRECACHE_MODEL("models/w_holybomb.mdl");

	PRECACHE_SOUND("weapons/pump_exp.wav");
	PRECACHE_SOUND("weapons/pump_deploy.wav");
	PRECACHE_SOUND("weapons/pump_throw.wav");
	PRECACHE_SOUND("weapons/pump_in.wav");
}

int CHolyBomb::GetItemInfo(ItemInfo *p)
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

BOOL CHolyBomb::Deploy(void)
{
	m_flReleaseThrow = invalid_time_point;
	m_fMaxSpeed = 250;
	m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;
	m_pPlayer->m_bShieldDrawn = false;
#ifdef ENABLE_SHIELD
	if (m_pPlayer->HasShield() != false)
		return DefaultDeploy("models/shield/v_shield_hegrenade.mdl", "models/shield/p_shield_hegrenade.mdl", HEGRENADE_DRAW, "shieldgren", UseDecrement() != FALSE);
	else
#endif
		return DefaultDeploy("models/v_holybomb.mdl", "models/p_holybomb.mdl", HEGRENADE_DRAW, "grenade", UseDecrement() != FALSE);
}

BOOL CHolyBomb::CanHolster(void)
{
	return CBasePlayerWeapon::CanHolster();
}

void CHolyBomb::Holster(int skiplocal)
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

void CHolyBomb::PrimaryAttack(void)
{
	if (m_flStartThrow == invalid_time_point && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] > 0)
	{
		m_flStartThrow = gpGlobals->time;
		m_flReleaseThrow = invalid_time_point;
		SendWeaponAnim(HEGRENADE_PULLPIN, UseDecrement() != FALSE);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5s;
	}
}

bool CHolyBomb::ShieldSecondaryFire(int up_anim, int down_anim)
{
	if (m_pPlayer->HasShield() == false)
		return false;

	if (m_flStartThrow != invalid_time_point)
		return false;

	if (m_iWeaponState & WPNSTATE_SHIELD_DRAWN)
	{
		m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;
		SendWeaponAnim(down_anim, UseDecrement() != FALSE);
		strcpy(m_pPlayer->m_szAnimExtention, "shieldgren");
		m_fMaxSpeed = 250;
		m_pPlayer->m_bShieldDrawn = false;
	}
	else
	{
		m_iWeaponState |= WPNSTATE_SHIELD_DRAWN;
		SendWeaponAnim(up_anim, UseDecrement() != FALSE);
		strcpy(m_pPlayer->m_szAnimExtention, "shielded");
		m_fMaxSpeed = 180;
		m_pPlayer->m_bShieldDrawn = true;
	}

#ifndef CLIENT_DLL
	m_pPlayer->UpdateShieldCrosshair((m_iWeaponState & WPNSTATE_SHIELD_DRAWN) == 0);
	m_pPlayer->ResetMaxSpeed();
#endif
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.4s;
	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.4s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.6s;
	return true;
}

void CHolyBomb::SecondaryAttack(void)
{
	ShieldSecondaryFire(SHIELDGUN_DRAW, SHIELDGUN_DRAWN_IDLE);
}

void CHolyBomb::WeaponIdle(void)
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

#ifndef CLIENT_DLL
		m_pPlayer->m_pModStrategy->ShootGrenade(std::bind(CHolyBombGrenade::ShootGrenade, m_pPlayer->pev, vecSrc, vecThrow, m_pPlayer->m_iTeam, GetDamage()));
#endif
		CHolyBombGrenade::ShootGrenade(m_pPlayer->pev, vecSrc, vecThrow, m_pPlayer->m_iTeam, GetDamage());

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

BOOL CHolyBomb::CanDeploy(void)
{
	return m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] != 0;
}

float CHolyBomb::GetDamage()
{
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZBS)
		return 500.0f;
	if (g_pModRunning->DamageTrack() == DT_ZB)
		return 500.0f;
#endif
	return 100.0f;
}

float CHolyBomb::GetRange()
{
	return 100.0f;
}

#ifndef CLIENT_DLL
LINK_ENTITY_TO_CLASS(grenade_holybomb, CHolyBombGrenade);

void CHolyBombGrenade::Precache(void)
{
	m_sModelIndexExp = PRECACHE_MODEL("sprites/heart_bombexp.spr");
}

void CHolyBombGrenade::Spawn()
{
	Precache();
	pev->movetype = MOVETYPE_BOUNCE;

	if (pev->classname)
	{
		RemoveEntityHashValue(pev, STRING(pev->classname), CLASSNAME);
	}

	MAKE_STRING_CLASS("grenade", pev);
	AddEntityHashValue(pev, STRING(pev->classname), CLASSNAME);
	pev->solid = SOLID_BBOX;

	SET_MODEL(ENT(pev), "models/w_holybomb.mdl");
	UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));
}

CHolyBombGrenade* CHolyBombGrenade::ShootGrenade(entvars_t* pevOwner, Vector vecStart, Vector vecVelocity, int iTeam, float flDamage)
{
	CHolyBombGrenade* pGrenade = CreateClassPtr<CHolyBombGrenade>();
	pGrenade->Spawn();
	UTIL_SetOrigin(pGrenade->pev, vecStart);
	pGrenade->pev->velocity = vecVelocity;
	pGrenade->pev->angles = UTIL_VecToAngles(-pGrenade->pev->velocity);
	pGrenade->pev->owner = ENT(pevOwner);
	pGrenade->pev->sequence = 1;
	pGrenade->pev->frame = 0;
	pGrenade->ResetSequenceInfo();
	pGrenade->m_fSequenceLoops = TRUE;
	pGrenade->pev->framerate = 2.0f;
	pGrenade->pev->gravity = 1.0f;
	pGrenade->pev->friction = 0.7f;
	pGrenade->pev->dmg = flDamage;
	pGrenade->pev->dmgtime = gpGlobals->time + pGrenade->GetExplosionTime();
	pGrenade->pev->nextthink = gpGlobals->time + 0.1s;
	pGrenade->m_iTeam = iTeam;
	pGrenade->m_iGrenadeID = GRENADE_HOLYBOMB;
	pGrenade->SetTouch(&CHolyBombGrenade::BounceTouch);
	pGrenade->SetThink(&CHolyBombGrenade::TumbleThink);

	return pGrenade;
}

void CHolyBombGrenade::BounceTouch(CBaseEntity* pOther)
{
	if (pOther->edict() == pev->owner)
		return;

	if (FClassnameIs(pOther->pev, "func_breakable") && pOther->pev->rendermode != kRenderNormal)
	{
		pev->velocity = pev->velocity * -2.0f;
		return;
	}

	Vector vecTestVelocity;
	vecTestVelocity = pev->velocity;
	vecTestVelocity.z *= 0.7f;

	if (pev->flags & FL_ONGROUND)
	{
		pev->velocity = pev->velocity * 0.965f;
	}
}

void CHolyBombGrenade::TumbleThink()
{
	if (!IsInWorld())
	{
		UTIL_Remove(this);
		return;
	}

	if (pev->velocity.LengthSquared() > 1.0f)
	{
		auto vecAngles = UTIL_VecToAngles(-pev->velocity);
		vecAngles[0] = vecAngles[2] = 0.0f;
		pev->angles = vecAngles;
	}
	else
		pev->angles.x = 0.0f;

	if (pev->waterlevel != 0)
	{
		pev->velocity = pev->velocity * 0.5f;
		pev->framerate = 0.2f;
	}
	else
	{
		pev->framerate = pev->velocity.Length() / 200.0f;
	}

	StudioFrameAdvance();
	pev->nextthink = gpGlobals->time + 0.1s;

	if (pev->dmgtime <= gpGlobals->time && pev->flags & FL_ONGROUND)
	{
		SetThink(&CHolyBombGrenade::Detonate);
	}
}

void CHolyBombGrenade::Detonate()
{
	TraceResult tr;
	Vector vecSpot;

	vecSpot = pev->origin + Vector(0, 0, 8);
	UTIL_TraceLine(vecSpot, vecSpot + Vector(0, 0, -40), ignore_monsters, ENT(pev), &tr);
	Explode(&tr, DMG_EXPLOSION);
}

void CHolyBombGrenade::Explode(TraceResult* pTrace, int bitsDamageType)
{
	pev->model = iStringNull; // invisible
	pev->solid = SOLID_NOT; // intangible
	pev->takedamage = DAMAGE_NO;

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
	WRITE_SHORT(m_sModelIndexExp);
	WRITE_BYTE(30);			// scale * 10
	WRITE_BYTE(30);		// framerate
	WRITE_BYTE(TE_EXPLFLAG_NONE);	// flags
	MESSAGE_END();

	entvars_t* pevOwner;
	if (pev->owner)
		pevOwner = VARS(pev->owner);
	else
		pevOwner = NULL;

	if (TheBots != NULL)
	{
		TheBots->OnEvent(EVENT_HE_GRENADE_EXPLODED, CBaseEntity::Instance(pev->owner));
	}

	pev->owner = NULL;
	::sv::RadiusDamage(pev->origin, pev, pevOwner, pev->dmg, GetRange(), CLASS_NONE, bitsDamageType);

	if (RANDOM_FLOAT(0, 1) < 0.5f)
		UTIL_DecalTrace(pTrace, DECAL_SCORCH1);
	else
		UTIL_DecalTrace(pTrace, DECAL_SCORCH2);

	switch (RANDOM_LONG(0, 2))
	{
	case 0: EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/debris1.wav", 0.55, ATTN_NORM); break;
	case 1: EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/debris2.wav", 0.55, ATTN_NORM); break;
	case 2: EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/debris3.wav", 0.55, ATTN_NORM); break;
	}

	pev->effects |= EF_NODRAW;
	SetThink(&CHolyBombGrenade::Smoke);
	pev->velocity = g_vecZero;
	pev->nextthink = gpGlobals->time + 0.55s;
	int sparkCount = RANDOM_LONG(0, 3);

	for (int i = 0; i < sparkCount; ++i)
		Create("spark_shower", pev->origin, pTrace->vecPlaneNormal, NULL);
}

void CHolyBombGrenade::Smoke()
{
	if (UTIL_PointContents(pev->origin) == CONTENTS_WATER)
	{
		UTIL_Bubbles(pev->origin - Vector(64, 64, 64), pev->origin + Vector(64, 64, 64), 100);
	}
	else
	{
		MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
		WRITE_BYTE(TE_SMOKE);
		WRITE_COORD(pev->origin.x);
		WRITE_COORD(pev->origin.y);
		WRITE_COORD(pev->origin.z - 5.0f);
		WRITE_SHORT(g_sModelIndexSmoke);
		WRITE_BYTE(35 + RANDOM_FLOAT(0, 10)); // scale * 10
		WRITE_BYTE(5); // framerate
		MESSAGE_END();
	}

	UTIL_Remove(this);
}
#endif
}
