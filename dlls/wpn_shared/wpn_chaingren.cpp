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

#ifndef CLIENT_DLL
#include "player/player_mod_strategy.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif


#ifndef CLIENT_DLL
	class ChainGrenade : public CGrenade
	{
	public:
		void Spawn()
		{
			CGrenade::Spawn();

			Precache();
			m_iGrenadeID = GRENADE_CHAINGRENADE;

			SET_MODEL(ENT(pev), "models/w_chaingren.mdl");
			UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));
		}
		void Precache()
		{
			PRECACHE_MODEL("models/w_chaingren.mdl");

			m_usEvent = PRECACHE_EVENT(1, "events/createexplo.sc");
		}

	public:
		void EXPORT ChainGrenThink()
		{
			if (!IsInWorld())
			{
				UTIL_Remove(this);

				return;
			}

			CalculateFramerate();
			StudioFrameAdvance();

			pev->nextthink = gpGlobals->time + 0.1s;

			if (gpGlobals->time > pev->dmgtime - 1.0s)
			{
				CSoundEnt::InsertSound(bits_SOUND_DANGER, pev->origin + pev->velocity * ((pev->dmgtime - gpGlobals->time) / 1s), 400, 0.1s);
			}

			if (gpGlobals->time < pev->dmgtime)
			{
				if (gpGlobals->time >= m_flExplodeTime)
				{
					m_flExplodeTime = gpGlobals->time + 0.55s;

					Explode();
				}
			}
			else
				SetThink(&ChainGrenade::ChainGrenDetonate);

			if (pev->waterlevel != 0)
			{
				pev->velocity *= 0.5;
				pev->framerate = 0.2;
			}
		}
		void EXPORT ChainGrenDetonate()
		{
			TraceResult tr;

			UTIL_TraceLine(pev->origin + Vector(0, 0, 8), pev->origin - Vector(0, 0, 40), ignore_monsters, edict(), &tr);

			if (m_pevOwner)
				pev->owner = ENT(m_pevOwner);

			Explode3(&tr, DMG_EXPLOSION, EXPTYPE_NORMAL, 0);
		}

	public:
		void Explode()
		{
			TraceResult tr;

			UTIL_TraceLine(pev->origin + Vector(0, 0, 8), pev->origin - Vector(0, 0, 40), ignore_monsters, edict(), &tr);

			pev->solid = SOLID_NOT;
			pev->takedamage = DAMAGE_NO;

			if (tr.flFraction < 1.0)
			{
				pev->origin = tr.vecEndPos + tr.vecPlaneNormal * 76.0 * 0.6;
			}

			MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
			WRITE_BYTE(TE_EXPLOSION);
			WRITE_COORD(pev->origin.x);
			WRITE_COORD(pev->origin.y);
			WRITE_COORD(pev->origin.z + 20.0);
			WRITE_SHORT(g_sModelIndexFireball3);
			WRITE_BYTE(25);
			WRITE_BYTE(30);
			WRITE_BYTE(TE_EXPLFLAG_NONE);
			MESSAGE_END();

			MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
			WRITE_BYTE(TE_EXPLOSION);
			WRITE_COORD(pev->origin.x + RANDOM_FLOAT(-64.0, 64.0));
			WRITE_COORD(pev->origin.y + RANDOM_FLOAT(-64.0, 64.0));
			WRITE_COORD(pev->origin.z + RANDOM_FLOAT(30.0, 35.0));
			WRITE_SHORT(g_sModelIndexFireball2);
			WRITE_BYTE(25);
			WRITE_BYTE(30);
			WRITE_BYTE(TE_EXPLFLAG_NONE);
			MESSAGE_END();

			CSoundEnt::InsertSound(bits_SOUND_COMBAT, pev->origin, NORMAL_EXPLOSION_VOLUME, 3.0s);

			if (g_pModRunning->DamageTrack() == DT_ZB)
				m_flExplodeDamage *= 5.0f;
			ExplodeChainGrenade(pev, m_pevOwner, m_flExplodeDamage, CLASS_NONE, DMG_EXPLOSION);

			if (RANDOM_FLOAT(0.0, 1.0) < 0.5)
				UTIL_DecalTrace(&tr, DECAL_SCORCH1);
			else
				UTIL_DecalTrace(&tr, DECAL_SCORCH2);

			switch (RANDOM_LONG(0, 2))
			{
			case 0: EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/debris1.wav", 0.55, ATTN_NORM); break;
			case 1: EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/debris2.wav", 0.55, ATTN_NORM); break;
			case 2: EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/debris3.wav", 0.55, ATTN_NORM); break;
			}

			int sparkCount = RANDOM_LONG(0, 3);

			for (int i = 0; i < sparkCount; i++)
				Create("spark_shower", pev->origin, tr.vecPlaneNormal, NULL);
		}

		void ExplodeChainGrenade(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int iClassIgnore, int bitsDamageType)
		{
			if (g_pModRunning->DamageTrack() == DT_ZB)
				RadiusDamage4(pev->origin, pevInflictor, pevAttacker, flDamage, flDamage * 0.65, iClassIgnore, bitsDamageType);
			else
				RadiusDamage4(pev->origin, pevInflictor, pevAttacker, flDamage, flDamage * 0.65, iClassIgnore, bitsDamageType);
		}

	public:
		void Initialize(const Vector& vecVelocity, duration_t flTimeRemove, int iTeam)
		{
			m_bStaticFramerate = true;
			pev->velocity = vecVelocity;
			pev->dmgtime = gpGlobals->time + flTimeRemove;

			SetTouch(&CGrenade::BounceTouch);
			SetThink(&ChainGrenade::ChainGrenThink);

			m_flExplodeTime = gpGlobals->time + 0.55s;

			pev->nextthink = gpGlobals->time + 0.1s;
			pev->sequence = RANDOM_LONG(3, 6);
			pev->framerate = 1;

			m_bJustBlew = true;

			pev->gravity = 0.55;
			pev->friction = 0.7;

			m_iTeam = iTeam;

			if (pev->owner)
				m_pevOwner = VARS(pev->owner);
			else
				m_pevOwner = NULL;

			pev->owner = NULL;
			pev->dmg = 82;
			m_flExplodeDamage = pev->dmg * 0.27;
		}

	public:

		static void ShootTimed(const Vector& vecOrigin, const Vector& vecAngles, const Vector& vecVelocity, duration_t flTimeRemove, int iTeam, edict_t* peOwner)
		{
			ChainGrenade* pGrenade = (ChainGrenade*)CBaseEntity::Create("chaingrenade", vecOrigin, vecAngles, peOwner);

			if (!pGrenade)
				return;

			SET_MODEL(ENT(pGrenade->pev), "models/w_chaingren.mdl");
			pGrenade->Initialize(vecVelocity, flTimeRemove, iTeam);
		}


	public:
		float m_flExplodeDamage;
		time_point_t m_flExplodeTime;
		entvars_t* m_pevOwner;
	};

	LINK_ENTITY_TO_CLASS(chaingrenade, ChainGrenade);
#endif

LINK_ENTITY_TO_CLASS(weapon_chaingren, CChainGrenade)

void CChainGrenade::Spawn(void)
{
	m_iGrenadeID = GRENADE_CHAINGRENADE;
	Precache();
	m_iId = WEAPON_HEGRENADE;
	SET_MODEL(ENT(pev), "models/w_chaingren.mdl");

	m_iDefaultAmmo = HEGRENADE_DEFAULT_GIVE;
	m_flStartThrow = invalid_time_point;
	m_flReleaseThrow = invalid_time_point;
	m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;

	FallInit();
}

void CChainGrenade::Precache(void)
{
	PRECACHE_MODEL("models/v_chaingren.mdl");
	PRECACHE_MODEL("models/p_chaingren.mdl");
	PRECACHE_MODEL("models/w_chaingren.mdl");

	m_usCreateExplosion = PRECACHE_EVENT(1, "events/createexplo.sc");
}

int CChainGrenade::GetItemInfo(ItemInfo *p)
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

BOOL CChainGrenade::Deploy(void)
{
	m_flReleaseThrow = invalid_time_point;
	m_fMaxSpeed = 250;

	return DefaultDeploy("models/v_chaingren.mdl", "models/p_chaingren.mdl", HEGRENADE_DRAW, "grenade", UseDecrement() != FALSE);
}

BOOL CChainGrenade::CanHolster(void)
{
	return CBasePlayerWeapon::CanHolster();
}

void CChainGrenade::Holster(int skiplocal)
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

void CChainGrenade::PrimaryAttack(void)
{
	if (m_flStartThrow == invalid_time_point && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] > 0)
	{
		m_flStartThrow = gpGlobals->time;
		m_flReleaseThrow = invalid_time_point;
		SendWeaponAnim(HEGRENADE_PULLPIN, UseDecrement() != FALSE);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5s;
	}
}

void CChainGrenade::WeaponIdle(void)
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

		float flVel = (90 - angThrow.x) * 6;

		if (flVel > 750)
			flVel = 750;

		UTIL_MakeVectors(angThrow);
		Vector vecSrc = m_pPlayer->pev->origin + m_pPlayer->pev->view_ofs + gpGlobals->v_forward * 16;
		Vector vecThrow = gpGlobals->v_forward * flVel + m_pPlayer->pev->velocity;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.75s;

		auto time = 1.5s;

#ifndef CLIENT_DLL
		ChainGrenade::ShootTimed(vecSrc, m_pPlayer->pev->angles, vecThrow, time, m_pPlayer->m_iTeam, m_pPlayer->edict());

		m_pPlayer->m_pModStrategy->ShootGrenade(std::bind(ChainGrenade::ShootTimed, vecSrc, m_pPlayer->pev->angles, vecThrow, time, m_pPlayer->m_iTeam, m_pPlayer->edict()));
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

BOOL CChainGrenade::CanDeploy(void)
{
	return m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] != 0;
}

}
