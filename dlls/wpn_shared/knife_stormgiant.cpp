/*
z4b_stormgiant.cpp
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
#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif
#include "weapons/KnifeAttack.h"
#include "weapons/WeaponTemplate.hpp"
#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

class CKnifeStormgiant: public LinkWeaponTemplate<CKnifeStormgiant,
	TGeneralData,
	BuildTGetItemInfoFromCSW<WEAPON_KNIFE>::template type
>
{
public:
	static constexpr const char *ClassName = "knife_stormgiant";
	static constexpr const char *V_Model = "models/v_stormgiant.mdl";
	static constexpr const char *P_Model = "models/p_stormgiant.mdl";
	static constexpr const char *W_Model = "models/w_knife.mdl";

	static constexpr int MaxClip = -1;
	static constexpr auto ItemSlot = KNIFE_SLOT;
	static constexpr const char *AnimExtension = "skullaxe";
	KnockbackData KnockBack = { 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };

public:

	enum stormgiant_e
	{
		ANIM_IDLE1,
		ANIM_DRAW_ATTACK,
		ANIM_DRAW_ATTACK_HIT,
		ANIM_DRAW,
		ANIM_MIDSLASH1,
		ANIM_MIDSLASH1_HIT,
		ANIM_MIDSLASH1_FAIL,
		ANIM_MIDSLASH2,
		ANIM_MIDSLASH2_HIT,
		ANIM_MIDSLASH2_FAIL
	};

	void Precache() override
	{
		Base::Precache();

		PRECACHE_SOUND("weapons/stormgiant_hit1.wav");
		PRECACHE_SOUND("weapons/stormgiant_hit2.wav");
		PRECACHE_SOUND("weapons/stormgiant_draw.wav");
		PRECACHE_SOUND("weapons/stormgiant_draw_attack.wav");
		PRECACHE_SOUND("weapons/stormgiant_attack_hit2.wav");
		PRECACHE_SOUND("weapons/stormgiant_midslash1.wav");
		PRECACHE_SOUND("weapons/stormgiant_midslash1_fail.wav");
		PRECACHE_SOUND("weapons/stormgiant_midslash2.wav");
		PRECACHE_SOUND("weapons/stormgiant_midslash2_fail.wav");
		PRECACHE_SOUND("weapons/stormgiant_stab_miss.wav");
		PRECACHE_SOUND("weapons/stormgiant_stone1.wav");
		PRECACHE_SOUND("weapons/stormgiant_stone2.wav");
		PRECACHE_SOUND("weapons/stormgiant_midslash1_hit.wav");
		PRECACHE_SOUND("weapons/stormgiant_midslash2_hit.wav");
		PRECACHE_SOUND("weapons/stormgiant_idle.wav");
		PRECACHE_SOUND("weapons/stormgiant_metal1.wav");
		PRECACHE_SOUND("weapons/stormgiant_metal2.wav");
		PRECACHE_SOUND("weapons/stormgiant_wood1.wav");
		PRECACHE_SOUND("weapons/stormgiant_wood2.wav");

		m_iSprEffect = PRECACHE_MODEL("sprites/ef_coilmg.spr");
	}

	void Spawn() override;
	float GetMaxSpeed() override { return m_fMaxSpeed; }
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	void WeaponIdle() override;
	void ItemPostFrame() override;
	BOOL CanDrop() override { return false; }

public:
	void DelayPrimaryAttack();
	void DelaySecondaryAttack();
	void ExtraAttack();
	int GetDeployAnim();
	bool AngularCheck(CBaseEntity* pEntity, CBaseEntity* pAttacker, Vector vecSrc, Vector vecDir, Vector2D vecAngleCheckDir, float flRange, float flMinimumCosine, TraceResult* ptr);

	void Holster(int skiplocal) override
	{
		m_iSwing = 0;
		SetThink(NULL);
		pev->nextthink = invalid_time_point;
	}

	BOOL Deploy() override
	{
		//EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/skullaxe_draw.wav", 0.3, 2.4);

		m_fMaxSpeed = 260;
		m_flTimeWeaponIdle2 = invalid_time_point;
		m_pPlayer->m_bShieldDrawn = false;

		if (m_iSwing == -1)
		{
			m_iSwing = 0;

			if (DefaultDeploy("models/v_stormgiant.mdl", "models/p_stormgiant.mdl", ANIM_DRAW, "skullaxe", UseDecrement() != FALSE))
				return TRUE;

			return FALSE;
		}

		m_iSwing = 1;
		m_pPlayer->m_bShieldDrawn = false;

		Q_strcpy(m_pPlayer->m_szAnimExtention, "stormgiant");
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
		SetThink(&CKnifeStormgiant::DelayPrimaryAttack);
		pev->nextthink = gpGlobals->time + 0.23s;

		if (!DefaultDeploy("models/v_stormgiant.mdl", "models/p_stormgiant.mdl", GetDeployAnim(), "skullaxe", UseDecrement() != FALSE, 0.0s))
			return FALSE;

		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.23s;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase();

		return TRUE;
	}

	float GetDeployAttackDamage() const	
	{
		float flDamage = 35;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 420.0f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage *= 5.5f;
#endif
		return flDamage;
	}
	float GetSecondaryAttackDamage() const
	{
		float flDamage = 110;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 1928.0f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage *= 5.5f;
#endif
		return flDamage;
	}
	float GetPrimaryAttackDamage() const
	{
		float flDamage = 110;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 1265.0f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage *= 5.5f;
#endif
		return flDamage;
	}
	float GetDeployExtraAttackDamage() const
	{
		float flDamage = 75;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 862.0f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage *= 5.5f;
#endif
		return flDamage;
	}

	void MaterialSound(TraceResult *ptr, Vector vecStart, Vector vecEnd);
	void CreateHitFX(Vector pos);

private:
	time_point_t m_flTimeWeaponIdle2;
	int m_iSprEffect;
};

LINK_ENTITY_TO_CLASS(knife_stormgiant, CKnifeStormgiant)

bool CKnifeStormgiant::AngularCheck(CBaseEntity* pEntity, CBaseEntity* pAttacker, Vector vecSrc, Vector vecDir, Vector2D vecAngleCheckDir, float flRange, float flMinimumCosine, TraceResult* ptr)
{
	bool bResult = false;
#ifndef CLIENT_DLL

	if (pEntity->IsBSPModel())
	{
		UTIL_TraceLine(vecSrc, vecSrc + vecDir * flRange, ignore_monsters, pAttacker->edict(), ptr);

		if (ptr->flFraction < 1.0)
		{
			if (ptr->pHit)
			{
				CBaseEntity* pHit = CBaseEntity::Instance(ptr->pHit);

				if (pHit && pHit == pEntity)
					bResult = true;
			}
		}
	}
	else
	{
		UTIL_TraceHull(vecSrc, vecSrc + vecDir * flRange, dont_ignore_monsters, head_hull, pAttacker->edict(), ptr);

		if (ptr->flFraction < 1.0 && ptr->pHit != NULL && CBaseEntity::Instance(ptr->pHit) == pEntity)
		{
			UTIL_TraceLine(vecSrc, vecSrc + vecDir * flRange, dont_ignore_monsters, pAttacker->edict(), ptr);

			bResult = true;
		}
		else
		{
			Vector vecEnd = pEntity->BodyTarget(vecSrc);

			if (DotProduct(vecAngleCheckDir, (vecEnd.Make2D() - vecSrc.Make2D()).Normalize()) > flMinimumCosine)
			{
				edict_t* peAttacker = pAttacker->edict();

				for (int i = 5; i > 0; i--)
				{
					UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, peAttacker, ptr);

					if (!ptr->pHit || !GET_PRIVATE(ptr->pHit))
					{
						break;
					}

					CBaseEntity* pHit = CBaseEntity::Instance(ptr->pHit);

					if (ptr->flFraction < 1.0 || pHit == pEntity)
					{
						bResult = true;

						break;
					}

					if (pHit->IsBSPModel())
					{
						break;
					}

					vecSrc = ptr->vecEndPos + vecDir * 5.0;
				}
			}
		}
	}

	//if ((!friendlyfire.value && pEntity->m_iTeam == pAttacker->m_iTeam) || !bResult)
	if (!bResult)
		return bResult;

	if (pEntity->IsPlayer())
	{
		CKnifeStormgiant::CreateHitFX(pEntity->pev->origin);
		CKnifeStormgiant::CreateHitFX(pEntity->pev->origin + Vector(0, 0, -10));

		if (!FBitSet(pEntity->pev->flags, FL_DUCKING))
			CKnifeStormgiant::CreateHitFX(pEntity->pev->origin + Vector(0, 0, 10));
	}
	else
	{
		CKnifeStormgiant::CreateHitFX(ptr->vecEndPos);
		CKnifeStormgiant::CreateHitFX(ptr->vecEndPos + Vector(0, 0, -10));

		if (!FBitSet(pEntity->pev->flags, FL_DUCKING))
			CKnifeStormgiant::CreateHitFX(ptr->vecEndPos + Vector(0, 0, 10));
	}
#endif

	return bResult;
}


void CKnifeStormgiant::Spawn()
{
	m_iKnifeID = KNIFE_STORMGIANT;
	pev->classname = MAKE_STRING("knife_stormgiant");
	m_iId = WEAPON_KNIFE;
	Precache();
	SET_MODEL(ENT(pev), "models/w_knife.mdl");
	m_iClip = WEAPON_NOCLIP;
	FallInit();
}


int CKnifeStormgiant::GetDeployAnim(void)
{
	Vector vecSrc, vecEnd;
	TraceResult tr;

	UTIL_MakeVectors(m_pPlayer->pev->v_angle);

	vecSrc = m_pPlayer->GetGunPosition();
	vecEnd = vecSrc + gpGlobals->v_forward * 105;

	UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, m_pPlayer->edict(), &tr);

	if (tr.flFraction < 1.0f)
	{
		EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "weapons/stormgiant_attack_hit2.wav", 0.3, 2.4, 0, PITCH_NORM);

		return ANIM_DRAW_ATTACK_HIT;
	}

	EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "weapons/stormgiant_draw_attack.wav", 0.3, 2.4, 0, PITCH_NORM);

	return ANIM_DRAW_ATTACK;
}

void CKnifeStormgiant::PrimaryAttack(void)
{
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
	SendWeaponAnim(ANIM_MIDSLASH1, UseDecrement() != FALSE);

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.3s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.3s;

	SetThink(&CKnifeStormgiant::DelayPrimaryAttack);
	pev->nextthink = gpGlobals->time + 1.033s;
}

void CKnifeStormgiant::SecondaryAttack(void)
{
	if(m_iSwing == 1)
	{
		m_iSwing = 2;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.53s;
		return;
	}

	SendWeaponAnim(ANIM_MIDSLASH2, UseDecrement() != FALSE);

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.17s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.17s;

	SetThink(&CKnifeStormgiant::DelaySecondaryAttack);
	pev->nextthink = gpGlobals->time + 0.9s;
}


void CKnifeStormgiant::DelayPrimaryAttack()
{
#ifndef CLIENT_DLL
	int bitsDamageType = DMG_BULLET | DMG_NEVERGIB;
	if (m_iSwing != 1 && m_iSwing != 2)
	{
		m_iSwing = 5;
		
		CBaseEntity* pEntity = NULL;
		BOOL fDidHit = FALSE;
		TraceResult tr;
		UTIL_MakeVectors(m_pPlayer->pev->v_angle);
		Vector vecEnd;
		Vector vecSrc = m_pPlayer->GetGunPosition();

		Vector vecForward = gpGlobals->v_forward;
		Vector2D vecForward2D = vecForward.Make2D().Normalize();

		float c = cos(360.0 * 0.5 * M_PI / 180.0);

		while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, 110.0)) != NULL)
		{
			if (pEntity == m_pPlayer)
				continue;

			if (pEntity->IsDormant())
				continue;

			if (pEntity->pev->takedamage == DAMAGE_NO)
				continue;

			if (pEntity->IsPlayer() && bIsWallBetweenEntity(pEntity, m_pPlayer))
				continue;

			if (!AngularCheck(pEntity, m_pPlayer, vecSrc, vecForward, vecForward2D, 110.0, c, &tr))
				continue;

			float flRatio = 1.0;

			if (pEntity && pEntity->IsPlayer())
			{
				UTIL_MakeVectors(pEntity->pev->angles);

				if (DotProduct((pEntity->pev->origin.Make2D() - m_pPlayer->pev->origin.Make2D()).Normalize(), gpGlobals->v_forward.Make2D()) > 0.8)
				{
					flRatio = 3.0;
					bitsDamageType |= DMG_BACKATK;
				}
			}

			m_pPlayer->m_iWeaponVolume = 128;

			EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_ITEM, RANDOM_LONG(0, 1) ? "weapons/stormgiant_hit1.wav" : "weapons/stormgiant_hit2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

			ClearMultiDamage();

			pEntity->TraceAttack(m_pPlayer->pev, GetPrimaryAttackDamage() * flRatio, (tr.vecEndPos - vecSrc).Normalize(), &tr, bitsDamageType);

			ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);

			fDidHit = TRUE;
		}

		if (!fDidHit)
		{
			vecEnd = vecSrc + vecForward * 110.0;

			UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, m_pPlayer->edict(), &tr);

			if (tr.flFraction < 1.0f)
			{
				MaterialSound(&tr, vecSrc, vecEnd);

				EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "weapons/stormgiant_midslash1_hit.wav", VOL_NORM, ATTN_NORM, 0, 94);

				SendWeaponAnim(ANIM_MIDSLASH1_HIT, UseDecrement() != FALSE);
			}
			else
			{
				EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "weapons/stormgiant_midslash1_fail.wav", VOL_NORM, ATTN_NORM, 0, 94);

				SendWeaponAnim(ANIM_MIDSLASH1_FAIL, UseDecrement() != FALSE);
			}
		}
		else
		{
			SendWeaponAnim(ANIM_MIDSLASH1_HIT, UseDecrement() != FALSE);
		}

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.23s;
		m_flTimeWeaponIdle2 = gpGlobals->time + 0.1s;

		SetThink(NULL);
	}
	else
	{
		BOOL fDidHit = FALSE;
		TraceResult tr;
		CBaseEntity* pEntity = NULL;
		UTIL_MakeVectors(m_pPlayer->pev->v_angle);
		Vector vecSrc = m_pPlayer->GetGunPosition();
		Vector vecEnd;

		Vector vecForward = gpGlobals->v_forward;
		Vector2D vecForward2D = vecForward.Make2D().Normalize();

		float c = cos(360.0 * 0.5 * M_PI / 180.0);

		while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, 105.0)) != NULL)
		{
			if (pEntity == m_pPlayer)
				continue;

			if (pEntity->IsDormant())
				continue;

			if (pEntity->pev->takedamage == DAMAGE_NO)
				continue;

			if (pEntity->IsPlayer() && bIsWallBetweenEntity(pEntity, m_pPlayer))
				continue;

			if (!AngularCheck(pEntity, m_pPlayer, vecSrc, vecForward, vecForward2D, 105.0, c, &tr))
				continue;

			float flRatio = 1.0;

			if (pEntity && pEntity->IsPlayer())
			{
				UTIL_MakeVectors(pEntity->pev->angles);

				if (DotProduct((pEntity->pev->origin.Make2D() - m_pPlayer->pev->origin.Make2D()).Normalize(), gpGlobals->v_forward.Make2D()) > 0.8)
				{
					flRatio = 3.0;
					bitsDamageType |= DMG_BACKATK;
				}
			}

			m_pPlayer->m_iWeaponVolume = 128;

			EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_ITEM, RANDOM_LONG(0, 1) ? "weapons/stormgiant_hit1.wav" : "weapons/stormgiant_hit2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

			ClearMultiDamage();

			pEntity->TraceAttack(m_pPlayer->pev, GetDeployAttackDamage() * flRatio, (tr.vecEndPos - vecSrc).Normalize(), &tr, bitsDamageType);

			ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);

			fDidHit = TRUE;
		}

		if (!fDidHit)
		{
			vecEnd = vecSrc + vecForward * 105.0;

			UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, m_pPlayer->edict(), &tr);

			if (tr.flFraction < 1.0f)
			{
				MaterialSound(&tr, vecSrc, vecEnd);
			}
		}

		SetThink(&CKnifeStormgiant::DelaySecondaryAttack);
		pev->nextthink = gpGlobals->time + 0.3s;
	}
#endif
}

void CKnifeStormgiant::DelaySecondaryAttack()
{
	if (m_iSwing != 1 && m_iSwing != 2)
	{
		ExtraAttack();
	}
	else if (m_iSwing == 2)
	{
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.17s;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.17s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.17s;

		m_iSwing = 3;

		ExtraAttack();
		SetThink(NULL);
	}
	else
	{
		m_flNextSecondaryAttack = m_flNextPrimaryAttack;

		m_iSwing = 4;
		SetThink(NULL);
		m_flTimeWeaponIdle2 = gpGlobals->time + 0.1s;
	}
}

void CKnifeStormgiant::ExtraAttack()
{

#ifndef CLIENT_DLL
	Vector vecSrc, vecEnd;
	TraceResult tr;
	CBaseEntity* pEntity = NULL;
	BOOL fHit = FALSE;

	Q_strcpy(m_pPlayer->m_szAnimExtention, "stormgiant");
	m_pPlayer->SetAnimation(PLAYER_ATTACK2);

	Q_strcpy(m_pPlayer->m_szAnimExtention, "skullaxe");

	if (m_iSwing != 3)
		m_iSwing = 6;

	float flDamage;
	int bitsDamageType = DMG_BULLET | DMG_NEVERGIB;

	if (m_iSwing == 3)
		flDamage = GetDeployExtraAttackDamage();
	else
		flDamage = GetSecondaryAttackDamage();

	UTIL_MakeVectors(m_pPlayer->pev->v_angle);

	vecSrc = m_pPlayer->GetGunPosition();

	Vector vecForward = gpGlobals->v_forward;
	Vector2D vecForward2D = vecForward.Make2D().Normalize();

	float c = cos(180.0 * 0.5 * M_PI / 180.0);

	while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, 110.0)) != NULL)
	{
		if (pEntity == m_pPlayer)
			continue;

		if (pEntity->IsDormant())
			continue;

		if (pEntity->pev->takedamage == DAMAGE_NO)
			continue;

		if (pEntity->IsPlayer() && bIsWallBetweenEntity(pEntity, m_pPlayer))
			continue;

		if (!AngularCheck(pEntity, m_pPlayer, vecSrc, vecForward, vecForward2D, 110.0, c, &tr))
			continue;

		float flRatio = 1.0;

		if (pEntity && pEntity->IsPlayer())
		{
			UTIL_MakeVectors(pEntity->pev->angles);

			if (DotProduct((pEntity->pev->origin.Make2D() - m_pPlayer->pev->origin.Make2D()).Normalize(), gpGlobals->v_forward.Make2D()) > 0.8)
			{
				flRatio = 3.0;
				bitsDamageType |= DMG_BACKATK;
			}
		}

		m_pPlayer->m_iWeaponVolume = 128;

		EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_ITEM, RANDOM_LONG(0, 1) ? "weapons/stormgiant_hit1.wav" : "weapons/stormgiant_hit2.wav", VOL_NORM, ATTN_NORM, 0, 94);

		ClearMultiDamage();

		pEntity->TraceAttack(m_pPlayer->pev, flDamage * flRatio, (tr.vecEndPos - vecSrc).Normalize(), &tr, bitsDamageType);

		ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);

		CBasePlayer* pPlayer = (CBasePlayer*)pEntity;
		if (pPlayer->IsPlayer() && pPlayer->m_bIsZombie)
		{
			if (m_iSwing == 3)
			{
				Vector2D vecDir2D = (pPlayer->pev->origin.Make2D() - m_pPlayer->pev->origin.Make2D()).Normalize();

				pPlayer->pev->velocity.x = vecDir2D.x * 300;
				pPlayer->pev->velocity.y = vecDir2D.y * 300;
				pPlayer->pev->velocity.z = 400;
			}
			else
			{
				pPlayer->m_flVelocityModifier = 0.35;
			}
		}

		fHit = TRUE;
	}

	if (!fHit)
	{
		vecEnd = vecSrc + vecForward * 110.0;

		UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, m_pPlayer->edict(), &tr);

		if (tr.flFraction < 1.0f)
		{
			MaterialSound(&tr, vecSrc, vecEnd);
			EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "weapons/stormgiant_midslash2_hit.wav", VOL_NORM, ATTN_NORM, 0, 94);

			SendWeaponAnim(ANIM_MIDSLASH2_HIT, UseDecrement() != FALSE);
		}
		else
		{
			EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "weapons/stormgiant_midslash2_fail.wav", VOL_NORM, ATTN_NORM, 0, 94);

			SendWeaponAnim(ANIM_MIDSLASH2_FAIL, UseDecrement() != FALSE);
		}
	}
	else
	{
		SendWeaponAnim(ANIM_MIDSLASH2_HIT, UseDecrement() != FALSE);
	}

	SetThink(NULL);
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.36s;
	m_flTimeWeaponIdle2 = gpGlobals->time + 0.1s;
#endif
}

void CKnifeStormgiant::MaterialSound(TraceResult *ptr, Vector vecStart, Vector vecEnd)
{
#ifndef CLIENT_DLL
	const char *rgsz[2];

	switch (UTIL_TextureHit(ptr, vecStart, vecEnd))
	{
		case CHAR_TEX_WOOD:
			rgsz[0] = "weapons/stormgiant_wood1.wav";
			rgsz[1] = "weapons/stormgiant_wood2.wav";

			break;

		case CHAR_TEX_GRATE:
		case CHAR_TEX_METAL:
		case CHAR_TEX_COMPUTER:
			rgsz[0] = "weapons/stormgiant_metal1.wav";
			rgsz[1] = "weapons/stormgiant_metal2.wav";

			break;

		default:
			rgsz[0] = "weapons/stormgiant_stone1.wav";
			rgsz[1] = "weapons/stormgiant_stone2.wav";

			break;
	}

	UTIL_EmitAmbientSound(ENT(0), ptr->vecEndPos, rgsz[RANDOM_LONG(0, 1)], 0.9, ATTN_NORM, 0, 96 + RANDOM_LONG(0, 15));
	CreateHitFX(ptr->vecEndPos);
#endif
}

void CKnifeStormgiant::CreateHitFX(Vector pos)
{
#ifndef CLIENT_DLL
	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
	WRITE_BYTE(TE_EXPLOSION);
	WRITE_COORD(pos.x);
	WRITE_COORD(pos.y);
	WRITE_COORD(pos.z);
	WRITE_SHORT(m_iSprEffect);
	WRITE_BYTE(3);
	WRITE_BYTE(15);
	WRITE_BYTE(TE_EXPLFLAG_NODLIGHTS | TE_EXPLFLAG_NOPARTICLES | TE_EXPLFLAG_NOSOUND);
	MESSAGE_END();
#endif
}


void CKnifeStormgiant::ItemPostFrame()
{
	if (m_flTimeWeaponIdle2 != invalid_time_point && m_flTimeWeaponIdle2 <= gpGlobals->time)
	{
		m_flTimeWeaponIdle2 = invalid_time_point;

		switch (m_iSwing)
		{
		case 4:
			SendWeaponAnim(ANIM_DRAW, UseDecrement() != FALSE);

			m_iSwing = 0;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;
			break;

		case 3:
		case 5:
		case 6:
			m_iSwing = 0;

			break;
		}
	}

	return CBasePlayerWeapon::ItemPostFrame();
}

void CKnifeStormgiant::WeaponIdle()
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	SendWeaponAnim(ANIM_IDLE1, UseDecrement() != FALSE);

	m_iSwing = 0;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1000s;
}

}
