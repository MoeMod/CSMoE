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
#include "wpn_gatlingex.h"

#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

#ifndef CLIENT_DLL
		class CGatlingexProjectile : public CBaseEntity
		{
		public:
			void Spawn() override
			{
				Precache();

				SET_MODEL(this->edict(), "sprites/ef_gatlingex_fireball.spr");
				SetThink(&CGatlingexProjectile::OnThink);
				SetTouch(&CGatlingexProjectile::OnTouch);
				pev->nextthink = gpGlobals->time + 0.01s;
				m_flExplodeTime = gpGlobals->time + 3s;
				pev->rendermode = kRenderTransAdd; // 5;
				pev->renderfx = kRenderFxNone;
				pev->renderamt = 255.0;
				m_freq = 0;
				pev->framerate = 15.0;
				UTIL_SetSize(pev, { -14, -14, -14 }, { 14, 14, 14 });
				pev->solid = SOLID_TRIGGER; // 2
				pev->movetype = MOVETYPE_FLYMISSILE; // 9
				m_flMaxFrames = 30.0;
				pev->classname = MAKE_STRING("gatlingex");
			}

			void EXPORT OnTouch(CBaseEntity* pOther)
			{
				if (pev->owner == pOther->edict())
					return;

				if (pOther->IsPlayer())
					return;

				Explode();
			}

			void Precache() override
			{
				PRECACHE_MODEL("sprites/ef_gatlingex_fireball.spr");
				PRECACHE_MODEL("sprites/ef_gatlingex_explosion.spr");
			}

			void OnThink()
			{

				if (gpGlobals->time < m_flExplodeTime)
				{
					this->pev->frame = (float)(this->pev->framerate * gpGlobals->frametime / 1s) + this->pev->frame;
					if (pev->frame > m_flMaxFrames)
					{
						pev->frame = fmod(pev->frame, m_flMaxFrames);
					}
					pev->nextthink = gpGlobals->time + 0.0099999998s;
					CBaseEntity* pEntity = NULL;
					while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, 20)) != NULL)
					{
						if (pEntity->pev->takedamage != DAMAGE_NO)
						{
							if (pEntity->pev == VARS(pev->owner))
								continue;

							if (pEntity->IsBSPModel())
								continue;

							if (pEntity->IsPlayer())
							{
								pev->velocity = m_vecNormalVeclocity * 20;
								break;
							}
						}
					}
					if (pEntity == NULL)
					{
						pev->velocity = m_vecNormalVeclocity * 100;
					}
					m_freq++;
					if (m_freq == 28)
					{
						m_freq = 0;
						RadiusDamage(pev->origin, m_fFireBallDamage, FALSE);
					}					
				}
				else
				{
					Explode();
				}
			}
			void Explode()
			{
				RadiusDamage(pev->origin, DetonationDamage(), TRUE);

				PLAYBACK_EVENT_FULL(FEV_GLOBAL, ENT(pev), m_usFireGatlingex, 0, pev->origin, (float*)&g_vecZero, 0.0, 0.0, 0, 0, TRUE, TRUE);

				Remove();
			}

			void RadiusDamage(Vector vecAiming, float flDamage, bool IsExp)
			{
				float flRadius;
				if (!IsExp)
				{
					flRadius = 40.0f;
					if (g_pModRunning->DamageTrack() == DT_ZBS)
						flRadius = 40.0f;
					if (g_pModRunning->DamageTrack() == DT_ZB)
						flRadius = 40.0f;
				}
				else
				{
					flRadius = 60.0f;
					if (g_pModRunning->DamageTrack() == DT_ZBS)
						flRadius = 60.0f;
					if (g_pModRunning->DamageTrack() == DT_ZB)
						flRadius = 60.0f;
				}

				const Vector vecSrc = pev->origin;
				entvars_t* const pevAttacker = VARS(pev->owner);
				entvars_t* const pevInflictor = this->pev;
				int bitsDamageType = DMG_BULLET;

				TraceResult tr;
				const float falloff = flRadius ? flDamage / flRadius : 1;
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
							float flAdjustedDamage = flDamage - (vecSrc - pEntity->pev->origin).Length() * falloff;
							flAdjustedDamage = Q_max(0, flAdjustedDamage);

							if (tr.flFraction == 1.0f)
							{
								pEntity->TakeDamage(pevInflictor, pevAttacker, flAdjustedDamage, bitsDamageType);
							}
							else
							{
								tr.iHitgroup = HITGROUP_CHEST;
								ClearMultiDamage();
								pEntity->TraceAttack(pevInflictor, flAdjustedDamage, (tr.vecEndPos - vecSrc).Normalize(), &tr, bitsDamageType);
								ApplyMultiDamage(pevInflictor, pevAttacker);
								CBasePlayer* player = dynamic_ent_cast<CBasePlayer*>(pEntity);
								if (player && player->m_bIsZombie)
								{
									ApplyKnockbackData(player, player->Center() - vecSrc, { 7000.0, 7000.0, 7000.0, 7000.0, 0.01f });
								}
							}

							/*CBasePlayer *pVictim = dynamic_cast<CBasePlayer *>(pEntity);
							if (pVictim->m_bIsZombie) // Zombie Knockback...
							{
								ApplyKnockbackData(pVictim, vecSpot - vecSrc, GetKnockBackData());
							}*/
						}
					}
				}
			}

			float DetonationDamage() const
			{
				float flDamage = 300.0f;
#ifndef CLIENT_DLL
				if (g_pModRunning->DamageTrack() == DT_ZB)
					flDamage = 3000.0f;
				//flDamage = 750.0f;
				else if (g_pModRunning->DamageTrack() == DT_ZBS)
					flDamage = 3000.0f;
				//flDamage = 1500.0f;
#endif
				return flDamage;
			}

			void Init(Vector vecVelocity, float flDamage, int eventindex, CBasePlayer* player)
			{
				std::tie(m_fFireBallDamage, m_iTeam) = std::make_tuple(flDamage, player->m_iTeam);
				m_vecNormalVeclocity = vecVelocity;
				m_usFireGatlingex = eventindex;
				pev->velocity = m_vecNormalVeclocity * 100;
				VEC_TO_ANGLES(vecVelocity.Normalize(), pev->angles);
			}

			int m_freq;
			int m_usFireGatlingex;
			time_point_t m_flExplodeTime;
			float m_fFireBallDamage;
			time_point_t m_flAnimEndTime;
			float m_flMaxFrames;
			Vector m_vecNormalVeclocity;
			TeamName m_iTeam;


		protected:
			void Remove()
			{
				SetTouch(nullptr);
				SetThink(nullptr);
				pev->effects |= EF_NODRAW; // 0x80u
				return UTIL_Remove(this);
			}

		};

		LINK_ENTITY_TO_CLASS(gatlingex_projectile, CGatlingexProjectile)
#endif

enum gatlingex_e
{
	GATLINGEX_IDLE,
	GATLINGEX_FIRE1,
	GATLINGEX_FIRE2,
	GATLINGEX_RELOAD,
	GATLINGEX_DRAW,
	GATLINGEX_FIREBALL
};

LINK_ENTITY_TO_CLASS(weapon_gatlingex, CGatlingex)

void CGatlingex::Spawn(void)
{
	Precache();
	m_iId = WEAPON_XM1014;
	SET_MODEL(ENT(pev), "models/w_gatlingex.mdl");

	m_iDefaultAmmo = GATLING_DEFAULT_GIVE;
	m_iDefaultAmmo2 = 0;
	FallInit();
}

void CGatlingex::Precache(void)
{
	PRECACHE_MODEL("models/v_gatlingex.mdl");
	PRECACHE_MODEL("models/w_gatlingex.mdl");
	PRECACHE_MODEL("models/p_gatlingex.mdl");

	PRECACHE_MODEL("sprites/ef_gatlingex_star.spr");

	m_iShellId = m_iShell = PRECACHE_MODEL("models/shotgunshell.mdl");

	PRECACHE_SOUND("weapons/gatlingex-1.wav");
	PRECACHE_SOUND("weapons/gatlingex-2.wav");
	PRECACHE_SOUND("weapons/gatlingex-2_amb.wav");
	PRECACHE_SOUND("weapons/gatlingex-2_exp.wav");
	PRECACHE_SOUND("weapons/gatlingex_clipin1.wav");
	PRECACHE_SOUND("weapons/gatlingex_clipin2.wav");
	PRECACHE_SOUND("weapons/gatlingex_clipout1.wav");
	PRECACHE_SOUND("weapons/gatlingex_clipout2.wav");
	PRECACHE_SOUND("weapons/gatlingex_boltpull.wav");
	PRECACHE_SOUND("weapons/gatlingex_draw.wav");
	PRECACHE_SOUND("weapons/gatlingex_draw.wav");

	m_usFireGatlingex = PRECACHE_EVENT(1, "events/gatlingex.sc");
}

int CGatlingex::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "12gauge";
	p->iMaxAmmo1 = 80;
	p->pszAmmo2 = "GatlingexProjectile";
	p->iMaxAmmo2 = 0;
	p->iMaxClip = GATLING_MAX_CLIP;
	p->iSlot = 0;
	p->iPosition = 12;
	p->iId = m_iId = WEAPON_XM1014;
	p->iFlags = 0;
	p->iWeight = XM1014_WEIGHT;

	return 1;
}

BOOL CGatlingex::Deploy(void)
{
	return DefaultDeploy("models/v_gatlingex.mdl", "models/p_gatlingex.mdl", GATLINGEX_DRAW, "m249", UseDecrement() != FALSE);
}

void CGatlingex::SecondaryAttack(void)
{
	if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] <= 0)
	{
		return;
	}

	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	PLAYBACK_EVENT_FULL(flags, ENT(m_pPlayer->pev), m_usFireGatlingex, 0, (float*)&g_vecZero, (float*)&g_vecZero, m_vVecAiming.x, m_vVecAiming.y, 7, m_vVecAiming.x * 100, FALSE, TRUE);
	
	m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]--;
#ifndef CLIENT_DLL
	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
	CGatlingexProjectile* pEnt = static_cast<CGatlingexProjectile*>(CBaseEntity::Create("gatlingex_projectile", m_pPlayer->GetGunPosition() + 50.0 * gpGlobals->v_forward, m_pPlayer->pev->v_angle, ENT(m_pPlayer->pev)));
	if (pEnt)
	{
		pEnt->Init(gpGlobals->v_forward, FireBallDamage(), m_usFireGatlingex, m_pPlayer);
	}
#endif

	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 3.0s;
	m_flNextPrimaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0s;
}

void CGatlingex::PrimaryAttack(void)
{
	if (m_pPlayer->pev->waterlevel == 3)
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15s;
		return;
	}
	
	if (m_iClip <= 0)
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2s;
		}

		return;
	}
	m_iBulletFired++;
	if(m_iBulletFired > 15 && m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] <= 2)
	{
		m_iBulletFired = 0;
		m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]++;
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
	m_pPlayer->FireBullets2(7, m_pPlayer->GetGunPosition(), gpGlobals->v_forward, Vector(0.03, 0.03, 0.0), 3048, BULLET_PLAYER_BUCKSHOT, 0, GetPrimaryAttackDamage(), NULL, 1);
#endif
	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, ENT(m_pPlayer->pev), m_usFireGatlingex, 0, (float *)&g_vecZero, (float *)&g_vecZero, m_vVecAiming.x, m_vVecAiming.y, 7, m_vVecAiming.x * 100, m_iClip != 0, FALSE);


#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.23s;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.23s;

	if (m_iClip)
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.25s;
	else
		m_flTimeWeaponIdle = 0.75s;


	if (m_pPlayer->pev->velocity.Length2D() > 0)
		m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomFloat(m_pPlayer->random_seed + 1, 3.0, 4.0);
	else if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 5.0, 6.0);
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 1.5, 2.0);
	else
		m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 2.0, 3.0);
}

void CGatlingex::Reload(void)
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		return;

	if (DefaultReload(GATLING_MAX_CLIP, GATLINGEX_RELOAD, 4.55s))
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
	}
}

void CGatlingex::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
	SendWeaponAnim(GATLINGEX_IDLE, UseDecrement() != FALSE);
	
}

int CGatlingex::GetPrimaryAttackDamage() const
{
	int iDamage = 18;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		iDamage = 26;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		iDamage = 26;
#endif
	return iDamage;
}

float CGatlingex::FireBallDamage() const
{
	float flDamage = 80.0f;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flDamage = 1100.0f;
	//flDamage = 375.0f;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 1100.0f;
	//flDamage = 750.0f;
#endif
	return flDamage;
}

int CGatlingex::ExtractAmmo(CBasePlayerWeapon* pWeapon)
{
	if (m_iDefaultAmmo2)
	{
		m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = m_iDefaultAmmo2;
		m_iDefaultAmmo2 = 0;
	}
	return CBasePlayerWeapon::ExtractAmmo(pWeapon);
}

}
