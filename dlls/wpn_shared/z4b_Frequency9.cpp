/*
z4b_Frequency9.cpp
Copyright (C) 2019 Moemod Hymei

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "weapons/KnifeAttack.h"
#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif

#include "weapons/WeaponTemplate.hpp"
#include "wpn_knife.h"

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

class CKnifeFrequency9: public LinkWeaponTemplate<CKnifeFrequency9,
	TGeneralData,
	BuildTGetItemInfoFromCSW<WEAPON_KNIFE>::template type,
	TWeaponIdleDefault
>
{
public:
	static constexpr const char *ClassName = "z4b_Frequency9";
	static constexpr const char *V_Model = "models/z4b/v_Frequency9.mdl";
	static constexpr const char *P_Model = "models/z4b/p_Frequency9.mdl";
	static constexpr const char *W_Model = "models/w_knife.mdl";

	static constexpr int MaxClip = -1;
	static constexpr auto ItemSlot = KNIFE_SLOT;
	static constexpr const char *AnimExtension = "katana";
	KnockbackData KnockBack = { .0f, .0f, .0f, .0f, 1.0f };

public:

	enum
	{
		ANIM_IDLE1 = 0,
		ANIM_SLASH1,
		ANIM_SLASH2,
		ANIM_DRAW,
		ANIM_STAB,
		ANIM_STAB_MISS,
		ANIM_MIDSLASH1,
		ANIM_MIDSLASH2,
		ANIM_SKILL_START,
		ANIM_SKILL_LOOP,
		ANIM_SKILL_END
	};

	enum
	{
		GUILLOTINE_START,
		GUILLOTINE_BACK,
	};

	void Precache() override
	{
		Base::Precache();

		PRECACHE_SOUND("weapons/frequency9_draw.wav");
		PRECACHE_SOUND("weapons/frequency9_hit.wav");
		PRECACHE_SOUND("weapons/frequency9_slash1.wav");
		PRECACHE_SOUND("weapons/frequency9_slash2.wav");
		PRECACHE_SOUND("weapons/frequency9_wall.wav");
		PRECACHE_SOUND("weapons/skullaxe_stab.wav");
		PRECACHE_SOUND("weapons/thanatos5_explode3.wav");
		PRECACHE_SOUND("weapons/frequency9_knife_wall.wav");
		PRECACHE_SOUND("weapons/frequency9_knife_create.wav");
		PRECACHE_SOUND("weapons/frequency9_skill_start.wav");
		PRECACHE_SOUND("weapons/frequency9_skill_end.wav");
	}

	BOOL Deploy() override
	{
		EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/frequency9_draw.wav", 0.3, 2.4);

		m_fMaxSpeed = 250;
		m_iSwing = 0;
		m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;
		m_pPlayer->m_bShieldDrawn = false;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0s;

		if(m_pFlyingEntity)
			return DefaultDeploy(V_Model, P_Model, ANIM_SKILL_LOOP, AnimExtension, UseDecrement() != FALSE);

		return DefaultDeploy(V_Model, P_Model, ANIM_DRAW, AnimExtension, UseDecrement() != FALSE);
	}

	float GetMaxSpeed() override { return m_fMaxSpeed; }
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	BOOL CanDrop() override { return false; }

public:
	void DelayPrimaryAttack();
	void DelaySecondaryAttack();
	float GetPrimaryAttackDamage() const
	{
		float flDamage = 76;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage *= 9.5f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage *= 5.5f;
#endif
		return flDamage;
	}
	float GetSecondaryAttackDamage() const
	{
		float flDamage = 40;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage *= 9.5f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage *= 5.5f;
#endif
		return flDamage;
	}

	void Catch();
	void Lost();

private:
	EntityHandle<CBaseEntity> m_pFlyingEntity; // pev_euser1
};

LINK_ENTITY_TO_CLASS(z4b_Frequency9, CKnifeFrequency9)

void CKnifeFrequency9::PrimaryAttack(void)
{
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
	if(m_iSwing & 1)
		SendWeaponAnim(ANIM_MIDSLASH2, UseDecrement() != FALSE);
	else
		SendWeaponAnim(ANIM_MIDSLASH1, UseDecrement() != FALSE);

	SetThink(&CKnifeFrequency9::DelayPrimaryAttack);
	pev->nextthink = gpGlobals->time + 0.2s;

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.6s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;
}

void CKnifeFrequency9::DelayPrimaryAttack()
{
	BOOL fDidHit = FALSE;
	UTIL_MakeVectors(m_pPlayer->pev->v_angle);
	Vector vecSrc = m_pPlayer->GetGunPosition();

#ifndef CLIENT_DLL
	m_iSwing++;
	switch (KnifeAttack3(vecSrc, gpGlobals->v_forward, GetPrimaryAttackDamage(), 100, 210, DMG_NEVERGIB | DMG_BULLET, m_pPlayer->pev, m_pPlayer->pev))
	{
	case HIT_NONE:
	{
		if(m_iSwing & 1)
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/frequency9_slash1.wav", VOL_NORM, ATTN_NORM, 0, 94);
		else
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/frequency9_slash2.wav", VOL_NORM, ATTN_NORM, 0, 94);
		break;
	}
	case HIT_PLAYER:
	{
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/frequency9_hit.wav", VOL_NORM, ATTN_NORM, 0, 94);
		m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;
		fDidHit = TRUE;
		break;
	}
	case HIT_WALL:
	{
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/frequency9_wall.wav", VOL_NORM, ATTN_NORM, 0, 94);
		m_pPlayer->m_iWeaponVolume = KNIFE_WALLHIT_VOLUME * 0.5;
		fDidHit = TRUE;
		break;
	}
	}
#endif
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;
	SetThink(nullptr);

	//return fDidHit;
}

void CKnifeFrequency9::SecondaryAttack(void)
{
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
	SendWeaponAnim(ANIM_SKILL_START, UseDecrement() != FALSE);

	SetThink(&CKnifeFrequency9::DelaySecondaryAttack);
	pev->nextthink = gpGlobals->time + 0.5s;

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 2.3s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;
}

void CKnifeFrequency9::Catch()
{
	SendWeaponAnim(ANIM_SKILL_END, UseDecrement() != FALSE);
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = 0.5s;
	m_flTimeWeaponIdle = 1.03s;

	m_pFlyingEntity = nullptr;

#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
	EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "weapons/frequency9_skill_end.wav", 0.8, ATTN_NORM, 0, PITCH_NORM);
}

void CKnifeFrequency9::Lost()
{
	SendWeaponAnim(ANIM_DRAW, UseDecrement() != FALSE);
	m_flNextPrimaryAttack = 1.76s;
	m_flTimeWeaponIdle = 1.8s;

	m_pFlyingEntity = nullptr;
}

#ifndef CLIENT_DLL
class CRailKnife : public CBaseAnimating
{
public:
	void Spawn() override
	{
		Precache();

		m_fSequenceLoops = 0;
		//ph26 = 0;
		SetTouch(&CRailKnife::OnTouch);
		SetThink(&CRailKnife::FlyThink);

		m_flReturnTime = gpGlobals->time + 0.73s;

		pev->solid = SOLID_CUSTOM; // 5
		pev->movetype = MOVETYPE_FLY; // 5
		pev->nextthink = gpGlobals->time + 0.0099999998s;
		SET_MODEL(edict(), "models/z4b/Frequency9_scythe.mdl");
		UTIL_SetSize(pev, { -1.44f, -1.45f, -1.3f }, { 1.44f, 1.45f, 4.3f });

		pev->animtime = gpGlobals->time;
		pev->sequence = 0;
		pev->framerate = 1;

		m_bReturning = false;
	}

	void Precache() override
	{
		m_iModelExplo = PRECACHE_MODEL("sprites/z4b/m4a1mw_hit.spr");
	}

	KnockbackData GetKnockBackData()
	{
		return { 1100.0f, 500.0f, 700.0f, 400.0f, 0.89999998f };
	}

	void EXPORT FlyThink()
	{
		if (gpGlobals->time < m_flReturnTime)
		{
			this->pev->nextthink = gpGlobals->time + 0.0099999998s;

			if (pev->solid == SOLID_NOT)
			{
				PenetrateEnd();
			}
		}
		else
		{
			Return();
		}
	}

	void EXPORT FireThink()
	{
		if(!m_pPlayer || !m_pPlayer->IsAlive())
			return Remove();
		Vector vecDelta = m_pPlayer->pev->origin - this->pev->origin;
		if(vecDelta.Length() < 42)
		{
			CKnifeFrequency9 *pWeapon = dynamic_cast<CKnifeFrequency9 *>(m_pPlayer->m_pActiveItem);
			if(pWeapon)
			{
				pWeapon->Catch();
				Remove();
			}
			else
			{
				Explode();
			}
		}

		pev->velocity = vecDelta.Normalize() * 1000;
		pev->nextthink = gpGlobals->time + 10ms;
	}

	void EXPORT OnTouch(CBaseEntity *pOther)
	{
		LaserEffect(pev->origin);
		EMIT_SOUND_DYN(edict(), CHAN_WEAPON, "weapons/thanatos5_explode3.wav", 1.0, ATTN_STATIC, 0, PITCH_NORM);

		if (pOther && pOther->IsBSPModel())
		{
			if(m_bReturning)
				Explode();
			else
				Return();
			MaterialSound();
		}
		else if (pOther->pev->pContainingEntity != this->pev->owner)
		{
			TouchEntity(pOther);
		}

	}

	void LaserEffect(Vector vecOrigin)
	{
		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_EXPLOSION);
		WRITE_COORD(vecOrigin[0]);
		WRITE_COORD(vecOrigin[1]);
		WRITE_COORD(vecOrigin[2]);
		WRITE_SHORT(m_iModelExplo);
		WRITE_BYTE(4);
		WRITE_BYTE(15);
		WRITE_BYTE(TE_EXPLFLAG_NODLIGHTS | TE_EXPLFLAG_NOPARTICLES | TE_EXPLFLAG_NOSOUND);
		MESSAGE_END();
	}

	void Explode()
	{
		LaserEffect(pev->origin);
		if(m_pPlayer)
		{
			CKnifeFrequency9 *pWeapon = dynamic_cast<CKnifeFrequency9 *>(m_pPlayer->m_pActiveItem);
			if(pWeapon)
				pWeapon->Lost();
		}
		Remove();
	}

	void Return()
	{
		SetThink(&CRailKnife::FireThink);
		pev->nextthink = gpGlobals->time + 10ms;
		m_bReturning = true;
	}

	void TouchEntity(CBaseEntity *pOther)
	{
		if (pev->owner == pOther->edict())
			return;

		CBaseEntity *pAttacker = CBaseEntity::Instance(pev->owner);
		CBasePlayer *pAttackePlayer = nullptr;
		if (pAttacker && pAttacker->IsPlayer())
			pAttackePlayer = static_cast<CBasePlayer *>(pAttacker);

		if (pAttackePlayer &&
		    pOther->pev->takedamage != DAMAGE_NO &&
		    pOther->IsAlive()
				)
		{
			Vector vecDirection = (pOther->pev->origin - pev->origin).Normalize();

			TraceResult tr;
			UTIL_TraceLine(pev->origin, pOther->pev->origin, missile, ENT(pAttackePlayer->pev), &tr);
			tr.iHitgroup = HITGROUP_CHEST; // ...

			ClearMultiDamage();
			pOther->TraceAttack(pAttackePlayer->pev, m_flTouchDamage, vecDirection, &tr, DMG_BULLET);
			ApplyMultiDamage(pAttackePlayer->pev, pAttackePlayer->pev);

			PenetrateStart();
		}
		pev->velocity = {};
	}

	void PenetrateStart()
	{
		pev->origin = pev->origin + m_vecStartVelocity.Normalize() * 42;
		pev->velocity = {};

		pev->solid = SOLID_NOT;

		pev->nextthink = gpGlobals->time + 0.05s;
	}

	void PenetrateEnd()
	{
		pev->velocity = m_vecStartVelocity;
		pev->solid = SOLID_CUSTOM;

		pev->nextthink = gpGlobals->time + 0.001s;
	}

	void MaterialSound()
	{
		if(RANDOM_LONG(0, 1))
			UTIL_EmitAmbientSound(edict(), pev->origin, "weapons/frequency9_slash2.wav", VOL_NORM, ATTN_NORM, 0, 96 + RANDOM_LONG(0, 15));
		else
			UTIL_EmitAmbientSound(edict(), pev->origin, "weapons/frequency9_wall.wav", VOL_NORM, ATTN_NORM, 0, 96 + RANDOM_LONG(0, 15));
	}

	void Init(Vector vecVelocity, float flTouchDamage, CBasePlayer *owner)
	{
		std::tie(m_flTouchDamage, m_pPlayer) = std::make_tuple(flTouchDamage, owner);
		pev->owner = m_pPlayer->edict();
		m_vecStartVelocity = pev->velocity = std::move(vecVelocity);
		m_vecStartOrigin = pev->origin;
		VEC_TO_ANGLES(vecVelocity.Normalize(), pev->angles);
	}

	float m_flTouchDamage;
	EntityHandle<CBasePlayer> m_pPlayer;

	Vector m_vecStartOrigin;
	Vector m_vecStartVelocity;
	int m_iModelExplo;

	time_point_t m_flReturnTime;
	bool m_bReturning;

protected:
	void Remove()
	{
		SetThink(nullptr);
		SetTouch(nullptr);
		pev->effects |= EF_NODRAW; // 0x80u
		return UTIL_Remove(this);
	}
};
LINK_ENTITY_TO_CLASS(rail_knife, CRailKnife)
#endif

void CKnifeFrequency9::DelaySecondaryAttack()
{
#ifndef CLIENT_DLL
	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
	Vector vecSrcA = m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 10 + gpGlobals->v_right * 5;
	Vector vecSrcB = m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 10 + -gpGlobals->v_right * (-5);
	CRailKnife *pEnt = static_cast<CRailKnife *>(CBaseEntity::Create("rail_knife", vecSrcA, m_pPlayer->pev->v_angle, ENT(m_pPlayer->pev)));
	if (pEnt)
	{
		pEnt->Init(gpGlobals->v_forward * 800, GetSecondaryAttackDamage(), m_pPlayer);
	}
#endif

	m_iSwing = 0;
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
	EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "weapons/frequency9_knife_create.wav", 0.8, ATTN_NORM, 0, PITCH_NORM);
	SendWeaponAnim(ANIM_SKILL_LOOP, UseDecrement() != FALSE);

	m_flNextPrimaryAttack = 1.76s;
	m_flTimeWeaponIdle = 1.8s;


}

}
