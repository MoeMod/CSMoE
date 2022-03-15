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
#include "wpn_voidpistol.h"


#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif


#ifndef CLIENT_DLL
namespace sv {
	class CVoidpistolBlackhole : public CBaseEntity
	{
		enum blackhole_anim
		{
			BLACKHOLE_START,
			BLACKHOLE_LOOP,
			BLACKHOLE_END
		};
	public:
		void Spawn() override
		{
			Precache();

			SET_MODEL(this->edict(), "models/ef_blackhole.mdl");
			pev->animtime = gpGlobals->time;
			pev->sequence = 0;
			pev->framerate = 1.0;	
			pev->solid = SOLID_BBOX; // 2	
			pev->rendermode = kRenderTransAlpha; // 5;
			pev->renderfx = kRenderFxNone;
			pev->renderamt = 255.0;
			pev->movetype = MOVETYPE_FLY; // 9
			pev->light_level = 255.0;
			UTIL_SetSize(pev, Vector(-0.1, -0.1, -0.1), Vector(0.1, 0.1, 0.1));
			
			m_iState = BLACKHOLE_START;
			m_flLoopTime = invalid_time_point;
			m_flStartTime = gpGlobals->time + 0.01s;
			m_flDamageTime = gpGlobals->time + 0.3s;
			m_flEndTime = invalid_time_point;
			SetThink(&CVoidpistolBlackhole::OnThink);
			pev->nextthink = gpGlobals->time + 0.1s;
			pev->classname = MAKE_STRING("voidpistol");

			PLAYBACK_EVENT_FULL(FEV_GLOBAL, ENT(pev), m_usFireVoidpistol, 0.0, pev->origin, g_vecZero, 0.0, 0.0, 0, m_iState, FALSE, TRUE);
		}

		void OnThink()
		{
			pev->nextthink = gpGlobals->time + 0.1s;
			switch (m_iState)
			{
			case BLACKHOLE_START:
			{
				if (m_flStartTime != invalid_time_point && gpGlobals->time > m_flStartTime)
				{
					m_iState = BLACKHOLE_LOOP;
					m_flStartTime = invalid_time_point;
					m_flLoopTime = gpGlobals->time + 1.5s;
					m_tSoundRepeat = gpGlobals->time + 1.5s;
				}

			}
			case BLACKHOLE_LOOP:
			{
				if (m_tSoundRepeat != invalid_time_point && gpGlobals->time > m_tSoundRepeat)
				{
					EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/voidpistol_blackhole_idle.wav", VOL_NORM, ATTN_NORM);
					m_tSoundRepeat = gpGlobals->time + 1s;
				}

				if (m_flLoopTime != invalid_time_point && gpGlobals->time > m_flLoopTime)
				{
					pev->sequence = 1;
					pev->animtime = gpGlobals->time;
					m_iState = BLACKHOLE_LOOP;
					m_flLoopTime = invalid_time_point;
					m_flEndTime = gpGlobals->time + 4.0s;
					PLAYBACK_EVENT_FULL(FEV_GLOBAL, ENT(pev), m_usFireVoidpistol, 0.0, pev->origin, g_vecZero, 0.0, 0.0, 0, m_iState, FALSE, TRUE);
				}

				if (m_flDamageTime != invalid_time_point && gpGlobals->time > m_flDamageTime)
				{
					RadiusDamage(BlackholeDamage(), true);
					m_flDamageTime = gpGlobals->time + 0.3s;
				}

				if (m_flEndTime != invalid_time_point && gpGlobals->time > m_flEndTime)
				{
					m_tSoundRepeat = invalid_time_point;
					pev->sequence = 2;
					pev->animtime = gpGlobals->time;
					m_iState = BLACKHOLE_END;
					m_freq = 0;
					PLAYBACK_EVENT_FULL(FEV_GLOBAL, ENT(pev), m_usFireVoidpistol, 0.0, pev->origin, g_vecZero, 0.0, 0.0, 0, m_iState, FALSE, TRUE);
					m_flEndTime = gpGlobals->time + 1.6s;
					m_iState = BLACKHOLE_END;
				}
			}
			case BLACKHOLE_END:
			{
				if (m_flDamageTime != invalid_time_point && gpGlobals->time > m_flDamageTime)
				{
					//exp
					RadiusDamage(DetonationDamage(), false);
					m_flDamageTime = invalid_time_point;
				}

				if (m_flEndTime != invalid_time_point && gpGlobals->time > m_flEndTime)
				{
					SetThink(&CVoidpistolBlackhole::Remove);
				}
			}
			}
		}

		void RadiusDamage(float flDamage, bool IsLoop)
		{
			CBaseEntity* pAttacker = CBaseEntity::Instance(pev->owner);
			CBasePlayer* pAttackPlayer = nullptr;
			if (pAttacker && pAttacker->IsPlayer())
				pAttackPlayer = static_cast<CBasePlayer*>(pAttacker);

			if (pAttackPlayer && !pAttackPlayer->m_bIsZombie)
			{
				TraceResult tr;
				CBaseEntity* pEntity = NULL;

				Vector vecDelta;
				Vector vecEnd;
				int bInWater = (UTIL_PointContents(pev->origin) == CONTENTS_WATER);

				while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, 600.0)) != NULL)
				{
					if (pEntity->edict() == pev->owner)
						continue;

					if (pEntity->IsDormant())
						continue;

					if (pEntity->pev->takedamage == DAMAGE_NO)
						continue;

					if (pEntity->Classify() == CLASS_NONE)
						continue;

					if ((bInWater && pEntity->pev->waterlevel == 0) || (!bInWater && pEntity->pev->waterlevel == 3))
						continue;

					if (pEntity->IsPlayer())
					{
						CBasePlayer* player = (CBasePlayer*)pEntity;

						if (!player)
							continue;

						if (pAttackPlayer->m_iTeam == player->m_iTeam)
							continue;

						vecEnd = player->BodyTarget(pev->origin);

						UTIL_TraceLine(pev->origin, vecEnd, ignore_monsters, pev->owner, &tr);

						if (tr.flFraction < 1.0 && tr.pHit != pEntity->edict())
							continue;

						vecDelta = pEntity->pev->origin - pev->origin;
						if (IsLoop)
						{
							Vector vecPullVelocity, vecOrigin;
							const float power = 400;
							vecOrigin = pev->origin;

							vecOrigin[0] += RANDOM_LONG(-20, 20);
							vecOrigin[1] += RANDOM_LONG(-15, 15);
							vecOrigin[2] += RANDOM_LONG(-10, 10);


							vecPullVelocity = (vecOrigin - player->pev->origin).Normalize() * power;
							player->pev->velocity = vecPullVelocity;
						}
						else
						{
							ApplyKnockbackData(player, vecDelta.Normalize(), {500, 500, 500, 500, 0.6f});
						}

						
						if (vecDelta.Length() > 190)
							continue;

						player->TakeDamage(pev, pAttackPlayer->pev, flDamage, DMG_EXPLOSION | DMG_NEVERGIB);
					}
					else
					{
						vecEnd = pEntity->BodyTarget(pev->origin);

						UTIL_TraceLine(pev->origin, vecEnd, ignore_monsters, pev->owner, &tr);

						if (tr.flFraction < 1.0 && tr.pHit != pEntity->edict())
							continue;

						vecDelta = pEntity->pev->origin - pev->origin;

						if (vecDelta.Length() > 190)
							continue;

						pEntity->TakeDamage(pev, pAttackPlayer->pev, flDamage, DMG_EXPLOSION | DMG_NEVERGIB);
					}
				}
			}
		}

		float DetonationDamage() const
		{
			float flDamage = 40.0f;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 750.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 1500.0f;
			//flDamage = 1500.0f;
#endif
			return flDamage;
		}

		float BlackholeDamage() const
		{
			float flDamage = 10.0f;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 110.0f;
			//flDamage = 375.0f;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 750.0f;
			//flDamage = 750.0f;
#endif
			return flDamage;
		}

		void Init(int eventindex, CBasePlayer* pPlayer)
		{
			m_usFireVoidpistol = eventindex;
			m_pPlayer = pPlayer;
		}

		int m_usFireVoidpistol;
		int m_freq;
		int m_SoundRepeat;
		CBasePlayer* m_pPlayer;
		int m_iState;
		time_point_t m_flDamageTime;
		time_point_t m_flLoopTime;
		time_point_t m_flStartTime;
		time_point_t m_flEndTime;
		time_point_t m_tSoundRepeat;

	protected:
		void Remove()
		{
			SetThink(nullptr);
			SetTouch(nullptr);
			pev->effects |= EF_NODRAW; // 0x80u

			return UTIL_Remove(this);
		}

	};
	LINK_ENTITY_TO_CLASS(voidpistol_blackhole, CVoidpistolBlackhole)

	class CVoidpistolProjectile : public CBaseEntity
	{
	public:
		void Spawn() override
		{
			Precache();

			SET_MODEL(this->edict(), "models/ef_blackhole_projectile.mdl");
			SetThink(&CVoidpistolProjectile::OnThink);
			SetTouch(&CVoidpistolProjectile::OnTouch);
			m_bCreateSpr = FALSE;
			pev->solid = SOLID_BBOX; // 2
			pev->nextthink = gpGlobals->time + 0.01s;
			m_flExplodeTime = gpGlobals->time + 4s;
			pev->rendermode = kRenderTransAdd; // 5;
			pev->renderfx = kRenderFxNone;
			pev->renderamt = 255.0;
			pev->framerate = 1.0;
			UTIL_SetSize(pev, { -14, -14, -14 }, { 14, 14, 14 });
			pev->solid = SOLID_BBOX; // 2
			pev->movetype = MOVETYPE_FLY; // 9
		}

		void EXPORT OnTouch(CBaseEntity* pOther)
		{
			if (pev->owner == pOther->edict())
				return;
			Explode();
		}

		void Precache() override
		{
			PRECACHE_MODEL("models/ef_blackhole_projectile.spr");
		}

		void OnThink()
		{
			if (gpGlobals->time < m_flExplodeTime)
			{
				if (!m_bCreateSpr)
				{
					m_bCreateSpr = TRUE;
					PLAYBACK_EVENT_FULL(FEV_GLOBAL, ENT(pev), m_usFireVoidpistol, 0.0, pev->origin, vecForward, 0.0, 0.0, 0, 4, FALSE, TRUE);
				}
				
			}
			else
			{
				Explode();
			}
			

			pev->nextthink = gpGlobals->time + 0.01s;
		}
		void Explode()
		{
			CVoidpistolBlackhole* pEnt = static_cast<CVoidpistolBlackhole*>(CBaseEntity::Create("voidpistol_blackhole", pev->origin, pev->angles, pev->owner));
			if (pEnt)
			{
				pEnt->Init(m_usFireVoidpistol, m_pPlayer);
			}
			Remove();
		}
		void Init(Vector vecVelocity, CBasePlayer* player, int eventindex)
		{
			std::tie(m_iTeam) = std::make_tuple(player->m_iTeam);
			vecForward = vecVelocity;
			pev->velocity = vecVelocity * 600;
			m_vecStartOrigin = pev->origin;
			VEC_TO_ANGLES(vecVelocity.Normalize(), pev->angles);
			pev->owner = player->edict();
			m_usFireVoidpistol = eventindex;
			m_pPlayer = player;
		}

		int m_usFireVoidpistol;
		time_point_t m_flExplodeTime;
		bool m_bCreateSpr;
		float m_BlackholeDamage;
		CBasePlayer* m_pPlayer;
		Vector vecForward;
		Vector m_vecStartOrigin;
		Vector m_vecStartVelocity;
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

	LINK_ENTITY_TO_CLASS(voidpistol_projectile, CVoidpistolProjectile)
}
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

LINK_ENTITY_TO_CLASS(weapon_voidpistol, CVoidpistol)

void CVoidpistol::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_voidpistol");

	Precache();
	m_iId = WEAPON_DEAGLE;
	SET_MODEL(ENT(pev), "models/w_voidpistol.mdl");
	m_iCharging = 0;
	m_iDefaultAmmo = 150;
	m_iDefaultAmmo2 = 0;
	m_flAccuracy = 0.9;
	m_iMode = VOIDPISTOL_MODEA;
	m_fMaxSpeed = 260;

	FallInit();
}

void CVoidpistol::Precache(void)
{
	PRECACHE_MODEL("models/v_voidpistol.mdl");
#ifdef ENABLE_SHIELD
	PRECACHE_MODEL("models/shield/v_shield_voidpistol.mdl");
#endif
	PRECACHE_MODEL("models/w_voidpistol.mdl");
	PRECACHE_MODEL("models/p_voidpistol.mdl");

	PRECACHE_MODEL("sprites/ef_blackhole01.spr"); //charing spr attached to model
	PRECACHE_MODEL("sprites/ef_blackhole02.spr");
	PRECACHE_MODEL("sprites/ef_blackhole03.spr");
	PRECACHE_MODEL("sprites/ef_blackhole04.spr");//infront of the gun charing finished
	PRECACHE_MODEL("models/ef_blackhole_projectile.mdl");

	PRECACHE_MODEL("sprites/ef_blackhole_star.spr"); //muzzleflash

	PRECACHE_MODEL("models/ef_blackhole.mdl");
	
	PRECACHE_MODEL("sprites/ef_blackhole_start.spr");//shooting project
	PRECACHE_MODEL("sprites/ef_blackhole_loop.spr");
	PRECACHE_MODEL("sprites/ef_blackhole_end.spr");
	PRECACHE_MODEL("sprites/muzzleflash140.spr");
	PRECACHE_MODEL("sprites/muzzleflash141.spr");



	PRECACHE_SOUND("weapons/voidpistol-1.wav");
	PRECACHE_SOUND("weapons/voidpistol-2.wav");


	m_iShell = PRECACHE_MODEL("models/pshell.mdl");
	m_usFireVoidpistol = PRECACHE_EVENT(1, "events/voidpistol.sc");
}

int CVoidpistol::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "voidpistolammo";
	p->iMaxAmmo1 = 100;
	p->pszAmmo2 = "VoidpistolProjectile";
	p->iMaxAmmo2 = 0;
	p->iMaxClip = VOIDPISTOL_MAX_CLIP;
	p->iSlot = 1;
	p->iPosition = 1;
	p->iId = m_iId = WEAPON_DEAGLE;
	p->iFlags = 0;
	p->iWeight = DEAGLE_WEIGHT;

	return 1;
}

BOOL CVoidpistol::Deploy(void)
{
	m_flAccuracy = 0.9;

	m_fMaxSpeed = 260;
#ifndef CLIENT_DLL
	if (IsModeCEnabled(m_iCharging))
	{
		DefaultDeploy("models/v_voidpistol.mdl", "models/p_voidpistol.mdl", VOIDPISTOL_DRAWC, "onehanded", UseDecrement() != FALSE, 0.2s);	
	}
	else
	{
		switch (m_iMode)
		{
		case VOIDPISTOL_MODEA:
		{
			DefaultDeploy("models/v_voidpistol.mdl", "models/p_voidpistol.mdl", VOIDPISTOL_DRAWA, "onehanded", UseDecrement() != FALSE, 0.2s);
		}
		case VOIDPISTOL_MODEB:
		{
			DefaultDeploy("models/v_voidpistol.mdl", "models/p_voidpistol.mdl", VOIDPISTOL_DRAWB, "onehanded", UseDecrement() != FALSE, 0.2s);
		}
		default:
			return FALSE;

		}
	}
#endif
	return TRUE;
}

void CVoidpistol::ItemPostFrame()
{
	Vector vecPlayerOrigin;
	vecPlayerOrigin = m_pPlayer->pev->origin;
#ifndef CLIENT_DLL

	CBaseEntity* pEntity = NULL;
	while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecPlayerOrigin, VOIDPISTOL_SCAN_RADIUS)) != NULL)
	{
		if (pEntity->pev->takedamage != DAMAGE_NO)
		{
			if (pEntity->pev == m_pPlayer->pev)
				continue;

			if (pEntity->IsBSPModel())
				continue;

			TraceResult tr;
			UTIL_TraceLine(m_pPlayer->pev->origin, pEntity->pev->origin, missile, m_pPlayer->edict(), &tr);

			if (tr.flFraction < 1.0)
			{
				if (!tr.pHit)
					continue;

				if (tr.pHit != pEntity->edict())
					continue;
			}

			if (pEntity->IsPlayer())
			{
				if(g_pGameRules->PlayerRelationship(m_pPlayer, pEntity) != GR_TEAMMATE)
				{
					if (!IsModeCEnabled(m_iCharging))
					{
						if (m_iMode == VOIDPISTOL_MODEB)
						{
							return CBasePlayerWeapon::ItemPostFrame();	//already MODE B
						}

						m_iMode = VOIDPISTOL_MODEB;
						SendWeaponAnim(VOIDPISTOL_SCANNING_ON, 0);
						m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.8s;
						pev->iuser1 = 0;
						return CBasePlayerWeapon::ItemPostFrame();
					}
					else
					{
						if (pev->iuser1)
						{				
							m_iMode = VOIDPISTOL_MODEB;
							return CBasePlayerWeapon::ItemPostFrame();	//already MODE B
						}
						pev->iuser1 = 1;
						m_iMode = VOIDPISTOL_MODEB;
						SendWeaponAnim(VOIDPISTOL_CHANGEBC, 0);
						//m_flNextSecondaryAttack = m_flNextPrimaryAttack = 0.5s;
						m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.8s;
						return CBasePlayerWeapon::ItemPostFrame();
					}
				}
			}
			
		}
	}

	if (!IsModeCEnabled(m_iCharging))
	{
		if (m_iMode == VOIDPISTOL_MODEA)
		{
			return CBasePlayerWeapon::ItemPostFrame();	//already MODE B
		}
		m_iMode = VOIDPISTOL_MODEA;
		SendWeaponAnim(VOIDPISTOL_SCANNING_OFF, 0);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.8s;
		pev->iuser1 = 0;
	}
	else
	{
		if (pev->iuser1)
		{		
			m_iMode = VOIDPISTOL_MODEA;
			return CBasePlayerWeapon::ItemPostFrame();	//already MODE B
		}
		pev->iuser1 = 1;
		m_iMode = VOIDPISTOL_MODEA;
		SendWeaponAnim(VOIDPISTOL_CHANGEAC, 0);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.8s;
	}
	return CBasePlayerWeapon::ItemPostFrame();
	
#endif
	

}


void CVoidpistol::PrimaryAttack(void)
{
#ifndef CLIENT_DLL
	switch (m_iMode)
	{
	case VOIDPISTOL_MODEA:
	{
		if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			VoidpistolFireA((1.5) * (1 - m_flAccuracy), 0.227s, FALSE);
		else if (m_pPlayer->pev->velocity.Length2D() > 0)
			VoidpistolFireA((0.25) * (1 - m_flAccuracy), 0.227s, FALSE);
		else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
			VoidpistolFireA((0.115) * (1 - m_flAccuracy), 0.227s, FALSE);
		else
			VoidpistolFireA((0.13) * (1 - m_flAccuracy), 0.227s, FALSE);
		break;
	}
	case VOIDPISTOL_MODEB:
	{
		VoidpistolFireB((0.13) * (1 - m_flAccuracy), 0.227s, FALSE); break;
	}
	}
#endif
}
bool CVoidpistol::IsModeCEnabled(int iCharging)
{
	return iCharging > 70;
}

void CVoidpistol::VoidpistolFireC(void)
{
	Vector vecPlayerOrigin;
	vecPlayerOrigin = m_pPlayer->pev->origin;
	m_iShotsFired++;
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
	m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]--;
#ifndef CLIENT_DLL
	m_iCharging = 0;
	CBaseEntity* pEntity = NULL;

	while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecPlayerOrigin, VOIDPISTOL_SCAN_RADIUS)) != NULL)
	{
		if (pEntity->pev->takedamage != DAMAGE_NO)
		{
			if (pEntity->pev == m_pPlayer->pev)
				continue;

			if (pEntity->IsBSPModel())
				continue;

			if (pEntity->IsPlayer() && g_pGameRules->PlayerRelationship(m_pPlayer, pEntity) != GR_TEAMMATE)
			{
				SendWeaponAnim(VOIDPISTOL_SHOOT_BLACKHOLE_B, UseDecrement() != FALSE);
				m_flNextSecondaryAttack = m_flNextPrimaryAttack = 1.03s;
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.1s;
				m_iMode = VOIDPISTOL_MODEB;
				break;
				
			}
			else
			{
				SendWeaponAnim(VOIDPISTOL_SHOOT_BLACKHOLE_A, UseDecrement() != FALSE);
				m_flNextSecondaryAttack = m_flNextPrimaryAttack = 1.03s;
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.1s;
				m_iMode = VOIDPISTOL_MODEA;
				break;
			}
		}
	}
	if (pEntity == NULL)
	{
		SendWeaponAnim(VOIDPISTOL_SHOOT_BLACKHOLE_A, UseDecrement() != FALSE);
		m_flNextSecondaryAttack = m_flNextPrimaryAttack = 1.03s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.1s;
		m_iMode = VOIDPISTOL_MODEA;
	}
#endif
#ifndef CLIENT_DLL
	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
	CVoidpistolProjectile* pEnt = static_cast<CVoidpistolProjectile*>(CBaseEntity::Create("voidpistol_projectile", m_pPlayer->GetGunPosition() + 50.0 * gpGlobals->v_forward, m_pPlayer->pev->v_angle, ENT(m_pPlayer->pev)));
	if (pEnt)
	{
		pEnt->Init(gpGlobals->v_forward, m_pPlayer, m_usFireVoidpistol);
	}
#endif
}

void CVoidpistol::VoidpistolFireB(float flSpread, duration_t flCycleTime, BOOL fUseSemi)
{
	Vector vecPlayerOrigin;
	TraceResult tr;
	vecPlayerOrigin = m_pPlayer->pev->origin;

	m_iShotsFired++;

	if (m_flLastFire != invalid_time_point)
	{
		m_flAccuracy -= (0.4 - ((gpGlobals->time - m_flLastFire) / 1s)) * 0.35;

		if (m_flAccuracy > 0.9)
			m_flAccuracy = 0.9;
		else if (m_flAccuracy < 0.55)
			m_flAccuracy = 0.55;
	}

	m_flLastFire = gpGlobals->time;

	if (m_iClip <= 0)
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2s;
		}

		return;
	}

#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
		
	Vector vecSrc = m_pPlayer->GetGunPosition();
#ifndef CLIENT_DLL
		CBaseEntity* pEntity = NULL;
		while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecPlayerOrigin, VOIDPISTOL_SCAN_RADIUS)) != NULL)
		{
			if (pEntity->pev->takedamage != DAMAGE_NO)
			{
				if (pEntity->pev == m_pPlayer->pev)
					continue;

				if (pEntity->IsBSPModel())
					continue;

				if (m_iCountPlayer > 10)
					continue;

				TraceResult tr;
				UTIL_TraceLine(m_pPlayer->pev->origin, pEntity->pev->origin, missile, m_pPlayer->edict(), &tr);

				if (tr.flFraction < 1.0)
				{
					if (!tr.pHit)
						continue;

					if (tr.pHit != pEntity->edict())
						continue;
				}

				if (pEntity->IsPlayer())
				{
					if (g_pGameRules->PlayerRelationship(m_pPlayer, pEntity) == GR_TEAMMATE)
						continue;

					

					Vector vecSpot = pEntity->BodyTarget(vecSrc);
					UTIL_TraceLine(vecSrc, vecSpot, missile, ENT(m_pPlayer->pev), &tr);
					if (tr.flFraction < 1.0f)
					{
						if (m_iClip > 0)
						{
							m_iClip--;
							m_iCharging++;
							m_iCountPlayer++;
							tr.iHitgroup = HITGROUP_CHEST;
							ClearMultiDamage();
							pEntity->TraceAttack(m_pPlayer->pev, GetDamage(), (tr.vecEndPos - vecSrc).Normalize(), &tr, DMG_BULLET);
							ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);
						}
						else
							break;
						
					}
				}
			}
		}
		m_iCountPlayer = 0;

		//SendWeaponAnim(m_iMode == VOIDPISTOL_MODEC ? VOIDPISTOL_SHOOTC : VOIDPISTOL_SHOOTB, UseDecrement() != FALSE);
		if (IsModeCEnabled(m_iCharging))
		{
			m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 1;
		}

		m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME; // 600
		m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH; // 512
	
#endif
		m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
		PLAYBACK_EVENT_FULL(0, m_pPlayer->edict(), m_usFireVoidpistol, 0, g_vecZero, g_vecZero, 2.0, 0.0, 0, 0, IsModeCEnabled(m_iCharging), FALSE);

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

#ifndef CLIENT_DLL
		if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
		m_pPlayer->pev->punchangle.x -= 2;
		ResetPlayerShieldAnim();

}

void CVoidpistol::SecondaryAttack(void)
{
#ifndef CLIENT_DLL
	if (IsModeCEnabled(m_iCharging))
		VoidpistolFireC();
	else
		return;
#endif
}


void CVoidpistol::VoidpistolFireA(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
{
	m_iShotsFired++;


	if (m_flLastFire != invalid_time_point)
	{
		m_flAccuracy -= (0.4 - ((gpGlobals->time - m_flLastFire) / 1s)) * 0.35;

		if (m_flAccuracy > 0.9)
			m_flAccuracy = 0.9;
		else if (m_flAccuracy < 0.55)
			m_flAccuracy = 0.55;
	}

	m_flLastFire = gpGlobals->time;
	
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
	SetPlayerShieldAnim();
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	m_pPlayer->m_iWeaponVolume = BIG_EXPLOSION_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	Vector vecSrc = m_pPlayer->GetGunPosition();
	

#ifndef CLIENT_DLL	
	int iEffect = 1;
	Vector vecDir = m_pPlayer->FireBullets4(vecSrc, gpGlobals->v_forward, flSpread, 4096, 2, BULLET_PLAYER_50AE, GetDamage(), 0.81, m_pPlayer->pev, TRUE, iEffect, m_pPlayer->random_seed, m_pPlayer);
	TraceResult tr;
	UTIL_TraceLine(vecSrc, vecDir, missile, ENT(m_pPlayer->pev), &tr);
	{
		if (tr.flFraction < 1.0)
		{
			CBaseEntity* pHit = CBaseEntity::Instance(tr.pHit);
			
			if (pHit->IsPlayer())
			{
				//PLAYBACK_EVENT_FULL(FEV_GLOBAL, ENT(pHit->pev), PRECACHE_EVENT(1, "events/wpneffects.sc"), 0.0, pHit->pev->origin, g_vecZero, 0.0, 0.0, 2, 0, TRUE, FALSE);
				if (g_pGameRules->PlayerRelationship(m_pPlayer, pHit) != GR_TEAMMATE)
					m_iCharging++;
			}
				//PLAYBACK_EVENT_FULL(FEV_GLOBAL, ENT(pHit->pev), PRECACHE_EVENT(1, "events/wpneffects.sc"), 0.0, tr.vecEndPos, g_vecZero, 0.0, 0.0, 2, 0, TRUE, FALSE);
		}
	}
	if (IsModeCEnabled(m_iCharging))
	{
		m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 1;
	}

	PLAYBACK_EVENT_FULL(0, m_pPlayer->edict(), m_usFireVoidpistol, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), 1, IsModeCEnabled(m_iCharging), FALSE);
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;
#endif

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
	
	m_pPlayer->pev->punchangle.x -= 2;
	ResetPlayerShieldAnim();
}

void CVoidpistol::Reload(void)
{
#ifndef CLIENT_DLL
	if (IsModeCEnabled(m_iCharging))
	{
		if (DefaultReload(VOIDPISTOL_MAX_CLIP, VOIDPISTOL_RELOADC, VOIDPISTOL_RELOAD_TIME))
		{
			m_pPlayer->SetAnimation(PLAYER_RELOAD);
			m_iShotsFired = 0;
		}
	}
	else
	{
		switch (m_iMode)
		{
		case VOIDPISTOL_MODEA:
		{
			if (DefaultReload(VOIDPISTOL_MAX_CLIP, VOIDPISTOL_RELOADA, VOIDPISTOL_RELOAD_TIME))
			{
				m_pPlayer->SetAnimation(PLAYER_RELOAD);
				m_iShotsFired = 0;
			}
			break;
		}
		case VOIDPISTOL_MODEB:
		{
			if (DefaultReload(VOIDPISTOL_MAX_CLIP, VOIDPISTOL_RELOADB, VOIDPISTOL_RELOAD_TIME))
			{
				m_pPlayer->SetAnimation(PLAYER_RELOAD);
				m_iShotsFired = 0;
				m_iMode = VOIDPISTOL_MODEA;
			}
			break;
		}
		}
	}
#endif
}

void CVoidpistol::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);
#ifndef CLIENT_DLL
	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	if (IsModeCEnabled(m_iCharging))
	{
		/*if (gpGlobals->time > m_tNextPlayEvent && m_tNextPlayEvent != invalid_time_point)
		{
			PLAYBACK_EVENT_FULL(FEV_HOSTONLY, m_pPlayer->edict(), PRECACHE_EVENT(1, "events/wpneffects.sc"), 0.0, g_vecZero, g_vecZero, 0.0, 0.0, 19, 4, TRUE, FALSE);
			m_tNextPlayEvent = gpGlobals->time + 1.2s;
		}*/
		SendWeaponAnim(VOIDPISTOL_IDLEC, UseDecrement() != FALSE);
		
	}
	else
	{
		switch (m_iMode)
		{
		case VOIDPISTOL_MODEA:
		{
			SendWeaponAnim(VOIDPISTOL_IDLEA, UseDecrement() != FALSE); 
			break;
		}
		case VOIDPISTOL_MODEB:
		{
			SendWeaponAnim(VOIDPISTOL_IDLEB, UseDecrement() != FALSE);
			break;
		}
		}
	}
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 6s;
#endif
	
}

float CVoidpistol::GetDamage() const
{
	float flDamage = 24.0f;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flDamage = 56.0f;
		//flDamage = 225.0f;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 390.0f;
#endif
	return flDamage;
}

int CVoidpistol::ExtractAmmo(CBasePlayerWeapon* pWeapon)
{
	if (m_iDefaultAmmo2)
	{
		m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = m_iDefaultAmmo2;
		m_iDefaultAmmo2 = 0;
	}
	return CBasePlayerWeapon::ExtractAmmo(pWeapon);
}

}
