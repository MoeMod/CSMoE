#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "wpn_sbmine.h"

#include "monsters.h"
#include "decals.h"
#include "gamerules.h"
#include "cbase/cbase_hash.h"

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

LINK_ENTITY_TO_CLASS(weapon_sbmine, CSbmine)

void CSbmine::Spawn(void)
{
	Precache();
	m_iId = WEAPON_HEGRENADE;
	SET_MODEL(ENT(pev), "models/w_sbmine.mdl");

	m_iDefaultAmmo = HEGRENADE_DEFAULT_GIVE;
	m_flStartThrow = invalid_time_point;
	m_flReleaseThrow = invalid_time_point;
	m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;

	FallInit();
}

void CSbmine::Precache(void)
{
	PRECACHE_MODEL("models/v_sbmine.mdl");
	PRECACHE_MODEL("models/p_sbmine.mdl");
	PRECACHE_MODEL("models/w_sbmine.mdl");

	PRECACHE_SOUND("weapons/pump_exp.wav");
	PRECACHE_SOUND("weapons/pump_deploy.wav");
	PRECACHE_SOUND("weapons/pump_throw.wav");
	PRECACHE_SOUND("weapons/pump_in.wav");
}

int CSbmine::GetItemInfo(ItemInfo *p)
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

BOOL CSbmine::Deploy(void)
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
		return DefaultDeploy("models/v_sbmine.mdl", "models/p_sbmine.mdl", SBMINE_DRAW, "grenade", UseDecrement() != FALSE);
}

BOOL CSbmine::CanHolster(void)
{
	return CBasePlayerWeapon::CanHolster();
}

void CSbmine::Holster(int skiplocal)
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

void CSbmine::PrimaryAttack(void)
{
	if (m_flStartThrow == invalid_time_point && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] > 0)
	{
		m_flStartThrow = gpGlobals->time;
		m_flReleaseThrow = invalid_time_point;
		SendWeaponAnim(HEGRENADE_PULLPIN, UseDecrement() != FALSE);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5s;
	}
}


void CSbmine::SecondaryAttack(void)
{
	
}

void CSbmine::WeaponIdle(void)
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

		CSbmineGrenade::ShootGrenade(m_pPlayer->pev, vecSrc, vecThrow, m_pPlayer->m_iTeam, GetDamage());

		SendWeaponAnim(SBMINE_SHOOT2, UseDecrement() != FALSE);
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
			SendWeaponAnim(SBMINE_DRAW, UseDecrement() != FALSE);
			m_flReleaseThrow = invalid_time_point;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + RandomDuration<float>(10s, 15s);
		}
		else
			RetireWeapon();

		return;
	}

	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
	{
		SendWeaponAnim(SBMINE_IDLE, UseDecrement() != FALSE);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + RandomDuration<float>(10s, 15s);
	}
}

BOOL CSbmine::CanDeploy(void)
{
	return m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] != 0;
}

float CSbmine::GetDamage()
{
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZBS)
		return 500.0f;
	if (g_pModRunning->DamageTrack() == DT_ZB)
		return 500.0f;
#endif
	return 100.0f;
}

float CSbmine::GetRange()
{
	return 100.0f;
}

#ifndef CLIENT_DLL
LINK_ENTITY_TO_CLASS(grenade_sbmine, CSbmineGrenade);

void CSbmineGrenade::Precache(void)
{
	m_sModelIndexExp = PRECACHE_MODEL("sprites/heart_bombexp.spr");

}

void CSbmineGrenade::Spawn()
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

	SET_MODEL(ENT(pev), "models/w_sbmine.mdl");
	UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));
}

CSbmineGrenade* CSbmineGrenade::ShootGrenade(entvars_t* pevOwner, Vector vecStart, Vector vecVelocity, int iTeam, float flDamage)
{
	CSbmineGrenade* pGrenade = CreateClassPtr<CSbmineGrenade>();
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
	pGrenade->SetTouch(&CSbmineGrenade::BounceTouch);
	pGrenade->SetThink(&CSbmineGrenade::TumbleThink);

	return pGrenade;
}

void CSbmineGrenade::BounceTouch(CBaseEntity* pOther)
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

void CSbmineGrenade::TumbleThink()
{
	if (!IsInWorld())
	{
		UTIL_Remove(this);
		return;
	}

	if(!m_pOwner)
	{
		UTIL_Remove(this);
		return;
	}

	if(m_pOwner->m_bIsZombie)
	{
		UTIL_Remove(this);
		return;
	}

	float flRadius = GetRange();
	while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, flRadius)) != NULL)
	{
		if (pEntity == pAttackPlayer)
			continue;

		if (pEntity->IsDormant())
			continue;

		if (pEntity->pev->takedamage == DAMAGE_NO)
			continue;

		if (pEntity->IsPlayer() && bIsWallBetweenEntity(pEntity, pAttackPlayer))
			continue;

		if (!AngleCheck(pEntity, pAttackPlayer, vecSrc, vecForward, vecForward2D, flRadius, c, &tr))
			continue;

		float flRatio = 1.0;

		if (pEntity && pEntity->IsPlayer())
		{
			UTIL_MakeVectors(pEntity->pev->angles);

			if (DotProduct((pEntity->pev->origin.Make2D() - pAttackPlayer->pev->origin.Make2D()).Normalize(), gpGlobals->v_forward.Make2D()) > 0.8)
			{
				flRatio = 3.0;
				bitsDamageType |= DMG_BACKATK;
			}

			if (g_pGameRules->PlayerRelationship(m_pPlayer, pEntity) != GR_TEAMMATE)
			{
				if (!FBitSet(pEntity->pev->flags, FL_DUCKING))
					CreateHitFX(pEntity->pev->origin + Vector(0, 0, -10), iAttackType);
				else
					CreateHitFX(pEntity->pev->origin + Vector(0, 0, 10), iAttackType);
			}
		}

		pAttackPlayer->m_iWeaponVolume = 128;

		ClearMultiDamage();

		pEntity->TraceAttack(pAttackPlayer->pev, flDamage * flRatio, (tr.vecEndPos - vecSrc).Normalize(), &tr, bitsDamageType);

		ApplyMultiDamage(pevInflictor, pAttackPlayer->pev);

		result = HIT_PLAYER;

		if (pEntity->IsPlayer())
		{
			CBasePlayer* player = (CBasePlayer*)pEntity;

			if (player->m_bIsZombie)
			{
				Vector vecDir = (player->pev->origin - m_pPlayer->pev->origin);
				vecDir.z = 0;
				vecDir = vecDir.Normalize();

				switch (iAttackType)
				{
				case 0:
				{
					//left 3
					ApplyKnockbackData(player, vecDir, { 0, 0, 0, 0, 0.6f });

					auto strategy = dynamic_cast<CPlayerModStrategy_ZB1*>(player->m_pModStrategy.get());
					if (strategy)
					{
						strategy->SetStunSpeedTime(1.5s);
						strategy->SetStunGravityTime(1.5s);
					}
					break;
				}
				case 1:
				{
					//left 1
					ApplyKnockbackData(player, vecDir, { 0, 0, 0, 0, 0.3f });

					auto strategy = dynamic_cast<CPlayerModStrategy_ZB1*>(player->m_pModStrategy.get());
					if (strategy)
					{
						strategy->SetStunSpeedTime(1.5s);
						strategy->SetStunGravityTime(1.5s);
					}
					break;

				}
				case 2:
				{
					//left 2
					ApplyKnockbackData(player, vecDir, { 0, 0, 0, 0, 0.3f });

					auto strategy = dynamic_cast<CPlayerModStrategy_ZB1*>(player->m_pModStrategy.get());
					if (strategy)
					{
						strategy->SetStunSpeedTime(1.5s);
						strategy->SetStunGravityTime(1.5s);
					}
					break;

				}
				case 3:
				{
					//charge
					ApplyKnockbackData(player, vecDir, { 1500, 1000, 1000, 1500, 1.0f });

					if (!(player->m_iBuff & BUFF_IGNORE_KNOCKBACK))
					{
						player->pev->velocity.z = 260.0;
					}
					break;
				}
				case 4:
				{
					//parying
					ApplyKnockbackData(player, vecDir, { 1500, 1000, 1000, 1500, 1.0f });

					if (!(player->m_iBuff & BUFF_IGNORE_KNOCKBACK))
					{
						player->pev->velocity.z = 260.0;
					}
					break;
				}
				}
			}
		}

}

void CSbmineGrenade::Detonate()
{
	TraceResult tr;
	Vector vecSpot;

	vecSpot = pev->origin + Vector(0, 0, 8);
	UTIL_TraceLine(vecSpot, vecSpot + Vector(0, 0, -40), ignore_monsters, ENT(pev), &tr);
	Explode(&tr, DMG_EXPLOSION);
}

void CSbmineGrenade::Explode(TraceResult* pTrace, int bitsDamageType)
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
	SetThink(&CSbmineGrenade::Smoke);
	pev->velocity = g_vecZero;
	pev->nextthink = gpGlobals->time + 0.55s;
	int sparkCount = RANDOM_LONG(0, 3);

	for (int i = 0; i < sparkCount; ++i)
		Create("spark_shower", pev->origin, pTrace->vecPlaneNormal, NULL);
}

void CSbmineGrenade::Smoke()
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
