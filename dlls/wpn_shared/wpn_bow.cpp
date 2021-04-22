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
#include "wpn_bow.h"
#ifndef CLIENT_DLL
#include "effects.h"
#include "customentity.h"
#include "monsters.h"
#endif

#include <vector>
#include <array>

#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

#ifndef CLIENT_DLL
	Vector Get_Velocity_Angle(CBaseEntity *pEntity, Vector Output)
	{
		pEntity->pev->velocity;
		Output = UTIL_VecToAngles(pEntity->pev->velocity);
		if (Output[0] > 90.0)
			Output[0] = -(360.0 - Output[0]);
		return Output;
	}

	Vector Stock_Get_Velocity_Angle(Vector In, Vector Output)
	{
		Output = UTIL_VecToAngles(In);
		if (Output[0] > 90.0)
			Output[0] = -(360.0 - Output[0]);
		return Output;
	}
#endif



#ifndef CLIENT_DLL
	class CBowArrow : public CBaseEntity
	{
	public:
		void Spawn() override
		{
			Precache();

			SET_MODEL(this->edict(), "models/arrow.mdl");
			SetThink(&CBowArrow::OnThink);
			SetTouch(&CBowArrow::OnTouch);
			pev->mins = { -0.1, -0.1, -0.1 };
			pev->maxs = { 0.1, 0.1, 0.1 };
			pev->rendermode = kRenderTransAdd; // 5;
			pev->renderfx = kRenderFxNone;
			pev->renderamt = 230.0;		
			pev->nextthink = gpGlobals->time + 0.15s;
			pev->framerate = 1;
			pev->solid = SOLID_BBOX; // 2
			pev->movetype = MOVETYPE_FLY; // 9

			pev->classname = MAKE_STRING("d_arrow");
		}

		void EXPORT OnTouch(CBaseEntity *pOther)
		{
			if (pev->owner == pOther->edict())
				return;
			pev->solid = SOLID_NOT;

			int DECAL_SHOT[5];
			DECAL_SHOT[0] = DECAL_INDEX("{shot1");
			DECAL_SHOT[1] = DECAL_INDEX("{shot2");
			DECAL_SHOT[2] = DECAL_INDEX("{shot3");
			DECAL_SHOT[3] = DECAL_INDEX("{shot4");
			DECAL_SHOT[4] = DECAL_INDEX("{shot5");

			Vector vecStart,vecAngle,vecForw,vecEnd;
			vecStart = pev->origin;
			vecAngle = Get_Velocity_Angle(this, vecAngle);

			UTIL_MakeVectors(vecAngle);		
			vecForw = gpGlobals->v_forward * 20.0;
			vecEnd = vecStart + vecForw;
			vecForw = gpGlobals->v_forward * -0.5;
			vecStart += vecForw;

			CBaseEntity *pAttacker = CBaseEntity::Instance(pev->owner);
			CBasePlayer *pAttackePlayer = nullptr;

			if (pAttacker && pAttacker->IsPlayer())
				pAttackePlayer = static_cast<CBasePlayer *>(pAttacker);
		
			TraceResult tr;
			UTIL_TraceLine(vecStart, vecEnd, dont_ignore_monsters, ENT(pAttackePlayer->pev), &tr);
			
			if (pAttackePlayer &&
				pOther->pev->takedamage != DAMAGE_NO &&
				pOther->IsAlive()
				)
			{
				Vector vecDirection = (pev->velocity).Normalize();

				ClearMultiDamage();
				pOther->TraceAttack(pAttackePlayer->pev, m_ArrowDamage, vecDirection, &tr, DMG_BULLET | DMG_NEVERGIB);
				ApplyMultiDamage(pAttackePlayer->pev, pAttackePlayer->pev);

				/*CBasePlayer *pVictim = dynamic_cast<CBasePlayer *>(pOther);
				if (pVictim->m_bIsZombie) // Zombie Knockback...
				{
					ApplyKnockbackData(pVictim, vecDirection, { 1500, 500, 700, 600, 0.4f });
					
				}*/
				pev->renderamt = 0.0;
				EMIT_SOUND_DYN(this->edict(), CHAN_AUTO, "weapons/xbow_hitbod1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			}

			SetThink(&CBaseEntity::SUB_StartFadeOut);
			pev->nextthink = gpGlobals->time + 6.0s;
			pev->rendermode = kRenderNormal;
			pev->renderamt = 255.0;

			if (pOther->IsBSPModel())
			{
				MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY, pev->origin);
				WRITE_BYTE(TE_GUNSHOTDECAL);
				WRITE_COORD(pev->origin.x);
				WRITE_COORD(pev->origin.y);
				WRITE_COORD(pev->origin.z);
				WRITE_SHORT(0);
				WRITE_BYTE(DECAL_SHOT[RANDOM_LONG(0, 4)]);
				MESSAGE_END();

				EMIT_SOUND_DYN(this->edict(), CHAN_AUTO, "weapons/xbow_hit1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
				pev->animtime = gpGlobals->time;
				pev->framerate = 1.0;
				pev->sequence = 0;
				SetThink(&CBaseEntity::SUB_StartFadeOut);
				pev->nextthink = gpGlobals->time + 3.5s;
				pev->renderamt = 255.0;
				pev->movetype = MOVETYPE_NONE;
				return;
			}
			else
				pev->flags |= FL_KILLME;

			pev->movetype = MOVETYPE_NONE;
		}

		void Precache() override
		{
			m_iModelLight1 = PRECACHE_MODEL("sprites/smoke.spr");
			PRECACHE_MODEL("models/arrow.mdl");
		}

		void OnThink()
		{
			if (pev->flags & FL_KILLME)
				return;
			MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
			WRITE_BYTE(TE_BEAMFOLLOW);
			WRITE_SHORT(this->entindex());  // short (entity:attachment to follow)
			WRITE_SHORT(m_iModelLight1);// short (sprite index)
			WRITE_BYTE(5);
			WRITE_BYTE(1);
			WRITE_BYTE(244);
			WRITE_BYTE(244);
			WRITE_BYTE(244);
			WRITE_BYTE(100);
			MESSAGE_END();
			pev->nextthink = gpGlobals->time + 10.0s;
		}

		void Init(Vector vecVelocity, float flDamage, TeamName iTeam, int iMode, float flGravity)
		{
			pev->gravity = flGravity;
			m_iMode = iMode;
			m_ArrowDamage = flDamage;
			pev->velocity = std::move(vecVelocity);
			m_iTeam = iTeam;
		}

		int m_iModelLight1;
		int m_iMode;
		float m_ArrowDamage;
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

	LINK_ENTITY_TO_CLASS(bow_arrow, CBowArrow);
#endif

enum CBow::bow_e
{
	ANIM_IDLE,
	ANIM_IDLE_EMPTY,
	ANIM_SHOOT1,
	ANIM_SHOOT1_EMPTY,
	ANIM_DRAW,
	ANIM_DRAW_EMPTY,
	ANIM_CHARGE_START1,
	ANIM_CHARGE_FINISH1,
	ANIM_CHARGE_IDLE1,
	ANIM_CHARGE_IDLE2,
	ANIM_CHARGE_SHOOT1,
	ANIM_CHARGE_SHOOT1_EMPTY,
	ANIM_CHARGE_SHOOT2,
	ANIM_CHARGE_SHOOT2_EMPTY
};

LINK_ENTITY_TO_CLASS(weapon_bow, CBow)

void CBow::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_bow");

	Precache();
	m_iId = WEAPON_M249;
	SET_MODEL(ENT(pev), "models/w_bow.mdl");
	m_flAccuracy = 0.2;
	m_iShotsFired = 0;

	m_iClip = -1;
	m_iDefaultAmmo = 60;

	FallInit();
}

void CBow::Precache(void)
{
	PRECACHE_MODEL("models/v_bow.mdl");
	PRECACHE_MODEL("models/w_bow.mdl");
	PRECACHE_MODEL("models/p_bow.mdl");
	PRECACHE_SOUND("weapons/xbow_hit1.wav");
	PRECACHE_SOUND("weapons/xbow_hitbod1.wav");
	PRECACHE_SOUND("weapons/bow_charge_finish1.wav");
	PRECACHE_SOUND("weapons/bow_charge_shoot1_empty.wav");
	PRECACHE_SOUND("weapons/bow_charge_shoot2.wav");
	PRECACHE_SOUND("weapons/bow_charge_start1.wav");
	PRECACHE_SOUND("weapons/bow_charge_start2.wav");
	PRECACHE_SOUND("weapons/bow_draw.wav");
	PRECACHE_SOUND("weapons/bow-shoot1.wav");

	m_usFireBow = PRECACHE_EVENT(1, "events/bow.sc");
}

int CBow::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "bowarrow";
	p->iMaxAmmo1 = 60;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = -1;
	p->iSlot = 0;
	p->iPosition = 4;
	p->iId = m_iId = WEAPON_M249;
	p->iFlags = 0;
	p->iWeight = M249_WEIGHT;

	return 1;
}

BOOL CBow::Deploy(void)
{
	m_flAccuracy = 0.2;
	m_iShotsFired = 0;
	iShellOn = 1;
	pev->iuser1 = 0;
	fFinishTime = invalid_time_point;

	return DefaultDeploy("models/v_bow.mdl", "models/p_bow.mdl", m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] ? ANIM_DRAW : ANIM_DRAW_EMPTY, "m249", UseDecrement() != FALSE);
}

int CBow::ExtractAmmo(CBasePlayerWeapon *pWeapon)
{
	if (m_iDefaultAmmo)
	{
		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] = m_iDefaultAmmo;
		m_iClip = WEAPON_NOCLIP;
		m_iDefaultAmmo = 0;
		return TRUE;
	}
	return CBasePlayerWeapon::ExtractAmmo(pWeapon);
}

void CBow::PrimaryAttack(void)
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
	{
		return;
	}
	m_iShotsFired++;
	m_pPlayer->pev->punchangle.x -= 2.0;
	m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
	SendWeaponAnim(m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] ? ANIM_SHOOT1 : ANIM_SHOOT1_EMPTY, UseDecrement() != FALSE);

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.425s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.925s;

	PLAYBACK_EVENT_FULL(FEV_GLOBAL, m_pPlayer->edict(), m_usFireBow, 0, m_pPlayer->pev->origin, (float *)&g_vecZero, 0.0, 0.0, (1 << 1), 0, FALSE, TRUE);

#ifndef CLIENT_DLL
	if (!m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
	m_iMode = 0;
#ifndef CLIENT_DLL
	Vector vecArrowAngle;
	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
	//Vector vecSrcB = m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 10 + -gpGlobals->v_right * (-5);
	CBowArrow *pEnt = static_cast<CBowArrow *>(CBaseEntity::Create("bow_arrow", m_pPlayer->GetGunPosition(), Stock_Get_Velocity_Angle(gpGlobals->v_forward * 2000, vecArrowAngle), ENT(m_pPlayer->pev)));
	if (pEnt)
	{
		pEnt->Init(gpGlobals->v_forward * 2000, GetPrimaryAttackDamage(), m_pPlayer->m_iTeam, m_iMode, 0.15);
	}
#endif

	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		KickBack(1.0, 1.0, 0.5, 0.5, 5.0, 5.0, 5.0);
	else if (m_pPlayer->pev->velocity.Length2D() > 0)
		KickBack(0.3, 0.3, 0.07, 0.07, 2.0, 2.0, 4.0);
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		KickBack(0.2, 0.2, 0.01, 0.01, 0.5, 0.5, 4.0);
	else
		KickBack(0.2, 0.2, 0.01, 0.01, 0.7, 0.7, 3.0);

	pev->iuser1 = 0;
	fFinishTime = invalid_time_point;
}

void CBow::SecondaryAttack(void)
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
	{
		return;
	}
}

void CBow::ItemPostFrame(void)
{
	if (m_flNextPrimaryAttack > 0.0s)
		return CBasePlayerWeapon::ItemPostFrame();
	if (!m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
		return CBasePlayerWeapon::ItemPostFrame();

	if (m_pPlayer->pev->button & IN_ATTACK && !(m_pPlayer->pev->button & IN_ATTACK2) && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
	{
		PrimaryAttack();
		m_pPlayer->pev->button &= ~IN_ATTACK;
		return CBasePlayerWeapon::ItemPostFrame();
	}
	else if (m_pPlayer->pev->button & IN_ATTACK2 && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
	{
		if (!pev->iuser1)
		{
			pev->iuser1 = 1;
			fFinishTime = gpGlobals->time + 1.2s;
			SendWeaponAnim(ANIM_CHARGE_START1, UseDecrement() != FALSE);
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.425s + 0.15s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.425s + 0.15s;
			m_pPlayer->pev->maxspeed = 185;
		}
		else if (gpGlobals->time > fFinishTime && pev->iuser1 != 2)
		{
			pev->iuser1 = 2;
			SendWeaponAnim(ANIM_CHARGE_FINISH1, UseDecrement() != FALSE);
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.43s;
		}
		m_pPlayer->pev->button &= ~IN_ATTACK2;
	}
	else if (!(m_pPlayer->pev->button & IN_RELOAD))
	{
		switch (pev->iuser1)
		{
		case 1:
		{
			m_pPlayer->pev->maxspeed = BOW_MAX_SPEED;
			m_iShotsFired++;
#ifndef CLIENT_DLL
			m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
			m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;
			SendWeaponAnim(m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] ? ANIM_CHARGE_SHOOT1 : ANIM_CHARGE_SHOOT1_EMPTY, UseDecrement() != FALSE);
			if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
			{
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.28s;
				m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.36s;
			}
			else
			{
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.7s;
				m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.7s;
			}
			EMIT_SOUND_DYN(this->edict(), CHAN_AUTO, m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] ? "weapons/bow-shoot1.wav" : "weapons/bow-shoot1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			m_iMode = 0;
#ifndef CLIENT_DLL
			Vector vecArrowAngle;
			UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
			//Vector vecSrcB = m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 10 + -gpGlobals->v_right * (-5);
			CBowArrow *pEnt = static_cast<CBowArrow *>(CBaseEntity::Create("bow_arrow", m_pPlayer->GetGunPosition(), Stock_Get_Velocity_Angle(gpGlobals->v_forward * 2200, vecArrowAngle), ENT(m_pPlayer->pev)));
			if (pEnt)
			{
				pEnt->Init(gpGlobals->v_forward * 2200, GetPrimaryAttackDamage(), m_pPlayer->m_iTeam, m_iMode, 0.15);
			}
#endif
			pev->punchangle[0] -= 3.0;
			m_pPlayer->pev->button &= ~IN_ATTACK;
			m_pPlayer->pev->button &= ~IN_ATTACK2;
			pev->iuser1 = 0;

			if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
				KickBack(1.0, 1.0, 0.5, 0.5, 5.0, 5.0, 5.0);
			else if (m_pPlayer->pev->velocity.Length2D() > 0)
				KickBack(0.3, 0.3, 0.07, 0.07, 2.0, 2.0, 4.0);
			else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
				KickBack(0.2, 0.2, 0.01, 0.01, 0.5, 0.5, 4.0);
			else
				KickBack(0.2, 0.2, 0.01, 0.01, 0.7, 0.7, 3.0);

			return CBasePlayerWeapon::ItemPostFrame();
		}
		case 2:
		{
			m_pPlayer->pev->maxspeed = BOW_MAX_SPEED;
			m_iShotsFired++;
#ifndef CLIENT_DLL
			m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
			UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

			m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
			m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

			Vector vecSrc = m_pPlayer->GetGunPosition();
			m_iMode = 1;
			Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, 0.0f, 8192, 7, BULLET_PLAYER_556MM, GetSecondaryAttackDamage(), 0.97, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);
			m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;
			PLAYBACK_EVENT_FULL(FEV_GLOBAL, m_pPlayer->edict(), m_usFireBow, 0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType], 0, FALSE, FALSE);

			SendWeaponAnim(m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] ? ANIM_CHARGE_SHOOT2 : ANIM_CHARGE_SHOOT2_EMPTY, UseDecrement() != FALSE);

			if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
			{
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.28s;
				m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.28s;
			}
			else
			{
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.7s;
				m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.7s;
			}
			pev->punchangle[0] -= 5.0;
			pev->iuser1 = 0;

			if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
				KickBack(1.0, 1.0, 0.5, 0.5, 5.0, 5.0, 5.0);
			else if (m_pPlayer->pev->velocity.Length2D() > 0)
				KickBack(0.3, 0.3, 0.07, 0.07, 2.0, 2.0, 4.0);
			else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
				KickBack(0.2, 0.2, 0.01, 0.01, 0.5, 0.5, 4.0);
			else
				KickBack(0.2, 0.2, 0.01, 0.01, 0.7, 0.7, 3.0);

			m_pPlayer->pev->button &= ~IN_ATTACK;
			m_pPlayer->pev->button &= ~IN_ATTACK2;
			return CBasePlayerWeapon::ItemPostFrame();
		}
		}
		
	}
	m_pPlayer->pev->button &= ~IN_ATTACK;
	m_pPlayer->pev->button &= ~IN_ATTACK2;
	return CBasePlayerWeapon::ItemPostFrame();
}

void CBow::Reload(void)
{
		return;
}

void CBow::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
	SendWeaponAnim(pev->iuser1 >= 1 ? (pev->iuser1 == 2 ? ANIM_CHARGE_IDLE2 : ANIM_CHARGE_IDLE1) : (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] ? ANIM_IDLE : ANIM_IDLE_EMPTY), UseDecrement() != FALSE);
}

float CBow::GetPrimaryAttackDamage() const
{
	float flDamage = 38;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flDamage = 72;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 72;
#endif
	return flDamage;
}

float CBow::GetSecondaryAttackDamage() const
{
	float flDamage = 152.0f;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flDamage = 755.0f;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 755.0f;
#endif
	return flDamage;
}

}
