/*
knife_skullt9.cpp
Copyright (C) 2019 Moemod Hyakuya

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

#include "weapons/WeaponTemplate.hpp"
#include "weapons/KnifeAttack.h"


#ifndef CLIENT_DLL
namespace sv {
	class CSkullT9Storm : public CBaseEntity
	{
	public:
		void Spawn() override
		{
			Precache();

			SET_MODEL(this->edict(), "models/ef_skullt9.mdl");
			pev->animtime = gpGlobals->time;
			pev->sequence = 0;
			pev->framerate = 1.0;
			pev->renderamt = 255.0;
			pev->solid = SOLID_NOT; // 2	
			pev->rendermode = kRenderTransTexture; // 5;
			UTIL_SetSize(pev, { -1.0, -1.0, -1.0 }, { 1.0, 1.0, 1.0 });
			pev->movetype = MOVETYPE_FLY; // 9
			SetThink(&CSkullT9Storm::OnThink);
			m_tRemoveTime = gpGlobals->time + 1.5s;
			pev->nextthink = gpGlobals->time + 0.01s;
			pev->classname = MAKE_STRING("skullt9");
			m_flMaxFrames = 31.0;
		}

		void OnThink()
		{
			this->pev->frame = (float)(this->pev->framerate * gpGlobals->frametime / 1s) + this->pev->frame;
			if (pev->frame > m_flMaxFrames)
			{
				pev->frame = fmod(pev->frame, m_flMaxFrames);
			}

			if(m_tRemoveTime < gpGlobals->time);
			{
				SetThink(&CSkullT9Storm::StartFadeOut);
			}
			pev->nextthink = gpGlobals->time + 0.01s;
				
		}

		void RadiusDamage(Vector vecAiming, float flDamage)
		{
			float flRadius = 240.0f;
			if (g_pModRunning->DamageTrack() == DT_ZBS)
				flRadius = 240.0f;
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flRadius = 240.0f;
			const Vector vecSrc = vecAiming;

			CBaseEntity* pAttacker = CBaseEntity::Instance(pev->owner);
			CBasePlayer* pAttackePlayer = nullptr;
			if (pAttacker && pAttacker->IsPlayer())
				pAttackePlayer = static_cast<CBasePlayer*>(pAttacker);

			if (!pAttackePlayer)
				return;

			entvars_t* const pevInflictor = this->pev;
			int bitsDamageType = DMG_BULLET;

			TraceResult tr;
			const int bInWater = (UTIL_PointContents(vecSrc) == CONTENTS_WATER);

			CBaseEntity* pEntity = NULL;
			while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, flRadius)) != NULL)
			{
				if (pEntity->pev->takedamage != DAMAGE_NO)
				{
					if (pEntity->pev == pAttackePlayer->pev)
						continue;

					if (bInWater && !pEntity->pev->waterlevel)
						continue;

					if (!bInWater && pEntity->pev->waterlevel == 3)
						continue;

					if (pEntity->IsBSPModel())
						continue;

					if ((pEntity->pev->origin - vecAiming).z < -50.0)
						continue;

					if (!pAttackePlayer->m_bIsZombie)
					{
						Vector vecSpot = pEntity->BodyTarget(vecSrc);
						UTIL_TraceLine(vecSrc, vecSpot, missile, ENT(pevInflictor), &tr);

						if (tr.flFraction == 1.0f || tr.pHit == pEntity->edict())
						{
							if (tr.fStartSolid)
							{
								tr.vecEndPos = vecSrc;
								tr.flFraction = 0;
							}

							if (tr.flFraction == 1.0f)
							{
								pEntity->TakeDamage(pevInflictor, pAttackePlayer->pev, flDamage, bitsDamageType);
							}
							else
							{
								tr.iHitgroup = HITGROUP_CHEST;
								ClearMultiDamage();
								pEntity->TraceAttack(pAttackePlayer->pev, flDamage, (tr.vecEndPos - vecSrc).Normalize(), &tr, bitsDamageType);
								ApplyMultiDamage(pevInflictor, pAttackePlayer->pev);
							}	
						}
					}
				}
			}
		}

		KnockbackData GetKnockBackData()
		{
			return { 30.0f, 85.0f, 100.0f, 100.0f, 0.1f };
		}

		void Precache() override
		{
			PRECACHE_MODEL("models/ef_skullt9.mdl");
		}

		float m_flMaxFrames;
		time_point_t m_tRemoveTime;

	protected:
		void StartFadeOut()
		{
			if (pev->rendermode == kRenderNormal) {
				pev->renderamt = 255.0f;
				pev->rendermode = kRenderTransTexture;
			}

			pev->solid = SOLID_NOT;
			pev->avelocity = g_vecZero;
			pev->nextthink = gpGlobals->time + 0.03s;

			SetThink(&CSkullT9Storm::FadeOut);
		}

		void FadeOut()
		{
			if (pev->renderamt > 50)
			{
				pev->renderamt -= 52.0f;
				pev->nextthink = gpGlobals->time + 0.03s;
			}
			else
			{
				pev->renderamt = 0.0f;
				SUB_Remove();
			}
		}
	};

	LINK_ENTITY_TO_CLASS(skullt9_storm, CSkullT9Storm);
}
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

class CKnifeSkullT9: public LinkWeaponTemplate<CKnifeSkullT9,
	TGeneralData,
	BuildTGetItemInfoFromCSW<WEAPON_KNIFE>::template type
>
{
public:
	static constexpr const char *ClassName = "knife_skullt9";
	static constexpr const char *V_Model = "models/v_skullt9.mdl";
	static constexpr const char* V_Model_1 = "models/v_skullt9_1.mdl";
	static constexpr const char *P_Model = "models/p_skullt9.mdl";
	static constexpr const char *W_Model = "models/w_knife.mdl";

	static constexpr int MaxClip = -1;
	static constexpr auto ItemSlot = KNIFE_SLOT;
	static constexpr const char *AnimExtension = "skullaxe";
	KnockbackData KnockBack = { 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };

public:

	enum skullaxe_e
	{
		ANIM_IDLE1 = 0,
		ANIM_SLASH_HIT,
		ANIM_STAB,
		ANIM_DRAW,
		ANIM_NONE1,
		ANIM_SLASH_MISS,
		ANIM_NONE2,
		ANIM_NONE3,
		ANIM_SLASH,
		ANIM_SKILL,
	};

	enum skullt9_crosshairstate_e
	{
		SKULLT9_AIM_NO = 11,
		SKULLT9_AIM_ON,
	};

	void Precache() override
	{
		PRECACHE_MODEL("models/v_skullt9_1.mdl");
		PRECACHE_MODEL("models/v_skullt9.mdl");
		PRECACHE_MODEL("models/p_skullt9.mdl");

		PRECACHE_SOUND("weapons/skullaxe_draw.wav");
		PRECACHE_SOUND("weapons/skullaxe_hit.wav");
		PRECACHE_SOUND("weapons/skullaxe_miss.wav");
		PRECACHE_SOUND("weapons/skullaxe_slash1.wav");
		PRECACHE_SOUND("weapons/skullaxe_slash2.wav");
		PRECACHE_SOUND("weapons/skullaxe_wall.wav");
		PRECACHE_SOUND("weapons/skullt9_skill_wind.wav");
		PRECACHE_SOUND("weapons/skullt9_fx.wav");
	}

	BOOL Deploy() override
	{
		EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/skullaxe_draw.wav", 0.3, 2.4);

		m_fMaxSpeed = 250;
		m_iSwing = 0;
		m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;
		m_pPlayer->m_bShieldDrawn = false;
		
#ifndef CLIENT_DLL
		MESSAGE_BEGIN(MSG_ONE, gmsgSpecialCrossHair, NULL, m_pPlayer->pev);
		WRITE_BYTE(2);
		WRITE_BYTE(m_CrossHairState);
		MESSAGE_END();
#endif

		if (m_flTimeholster != invalid_time_point)
		{
			m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] += (gpGlobals->time - m_flTimeholster) / 0.33s;
		}
		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] >= 100)
			m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] = 100;
			
		return DefaultDeploy(m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] >= 100 ? V_Model_1 : V_Model, P_Model, ANIM_DRAW, "skullaxe", UseDecrement() != FALSE);
	}

	void Spawn() override;
	float GetMaxSpeed() override { return m_fMaxSpeed; }
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	int GetItemInfo(ItemInfo* p) override;
	BOOL CanDrop() override { return false; }
    WeaponBotStrategy GetBotStrategy() const override;

public:
#ifndef CLIENT_DLL
	hit_result_t KnifeAttack(float flDamage, float flRadius, float flAngleDegrees, int bitsDamageType, entvars_t* pevInflictor, CBasePlayer* pAttackPlayer, bool IsPrimaryAttack);
#endif
	void DelayPrimaryAttack();
	void DelaySecondaryAttack();
	float GetPrimaryAttackDamage() const
	{
		float flDamage = 100;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage *= 20.5f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 9760.0f;
#endif
		return flDamage;
	}

	float GetSecondaryAttackDamage() const
	{
		float flDamage = 100;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage *= 27.5f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 9760.0f;
#endif
		return flDamage;
	}

	float GetEffectAttackDamage() const
	{
		float flDamage = 100;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage *= 35.5f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 9760.0f;
#endif
		return flDamage;
	}

	void Holster(int skiplocal) override;
	int ExtractAmmo(CBasePlayerWeapon* pWeapon) override;
	BOOL IsSignal();
	void ItemPostFrame() override;

	time_point_t m_flTimeNextCharge;
	time_point_t m_flTimeNextSkill1;
	time_point_t m_flTimeNextMessage;
	time_point_t m_flTimeholster;
	skullt9_crosshairstate_e m_CrossHairState;
	skullt9_crosshairstate_e m_CrossHairStateStored;
	int m_iButton;
	int m_iUseSkill;
};

LINK_ENTITY_TO_CLASS(knife_skullt9, CKnifeSkullT9)

int CKnifeSkullT9::ExtractAmmo(CBasePlayerWeapon* pWeapon)
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

int CKnifeSkullT9::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "skullt9ammo";
	p->iMaxAmmo1 = 100;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = 100;
	p->iSlot = 2;
	p->iPosition = 1;
	p->iId = WEAPON_KNIFE;
	p->iFlags = 0;
	p->iWeight = KNIFE_WEIGHT;

	return 1;
}

void CKnifeSkullT9::Spawn()
{
	m_iKnifeID = KNIFE_SKULLT9;
	pev->classname = MAKE_STRING("knife_skullt9");
	m_iId = WEAPON_KNIFE;
	Precache();
	SET_MODEL(ENT(pev), "models/w_knife.mdl");
	m_iClip = WEAPON_NOCLIP;
	m_flTimeholster = gpGlobals->time;
	m_iDefaultAmmo = 0;
	m_iUseSkill = 0;
	m_CrossHairState = m_CrossHairStateStored = SKULLT9_AIM_NO;
	m_flTimeNextMessage = invalid_time_point;
	FallInit();
}

void CKnifeSkullT9::PrimaryAttack(void)
{
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
	SendWeaponAnim(ANIM_SLASH, UseDecrement() != FALSE);

	SetThink(&CKnifeSkullT9::DelayPrimaryAttack);
	pev->nextthink = gpGlobals->time + 0.9s;

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.2s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;
}

void CKnifeSkullT9::SecondaryAttack(void)
{
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] >= 100)
	{
		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] = 0;
		m_iUseSkill = 1;
	}
	SendWeaponAnim(ANIM_STAB, UseDecrement() != FALSE);

	SetThink(&CKnifeSkullT9::DelaySecondaryAttack);
	pev->nextthink = gpGlobals->time + 0.9s;

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.4s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;
}

void CKnifeSkullT9::DelayPrimaryAttack()
{
	BOOL fDidHit = FALSE;

#ifndef CLIENT_DLL
	m_iSwing++;
	switch (KnifeAttack(GetPrimaryAttackDamage(), 117, 60, DMG_NEVERGIB | DMG_BULLET, m_pPlayer->pev, m_pPlayer, true))
	{
	case HIT_NONE:
	{
		SendWeaponAnim(ANIM_SLASH_MISS, UseDecrement() != FALSE);
		if(m_iSwing & 1)
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/skullaxe_slash1.wav", VOL_NORM, ATTN_NORM, 0, 94);
		else
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/skullaxe_slash2.wav", VOL_NORM, ATTN_NORM, 0, 94);
		break;
	}
	case HIT_PLAYER:
	{
		SendWeaponAnim(ANIM_SLASH_HIT, UseDecrement() != FALSE);
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/skullaxe_hit.wav", VOL_NORM, ATTN_NORM, 0, 94);
		m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;
		fDidHit = TRUE;
		break;
	}
	case HIT_WALL:
	{
		SendWeaponAnim(ANIM_SLASH_HIT, UseDecrement() != FALSE);
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/skullaxe_wall.wav", VOL_NORM, ATTN_NORM, 0, 94);
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

void CKnifeSkullT9::DelaySecondaryAttack()
{
	BOOL fDidHit = FALSE;
#ifndef CLIENT_DLL
	switch (KnifeAttack(GetSecondaryAttackDamage(), 130, 180, DMG_NEVERGIB | DMG_BULLET, m_pPlayer->pev, m_pPlayer, false))
	{
	case HIT_NONE:
	{
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/skullaxe_miss.wav", VOL_NORM, ATTN_NORM, 0, 94);
		break;
	}
	case HIT_PLAYER:
	{
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/skullaxe_hit.wav", VOL_NORM, ATTN_NORM, 0, 94);
		m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;
		fDidHit = TRUE;
		break;
	}
	case HIT_WALL:
	{
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/skullaxe_wall.wav", VOL_NORM, ATTN_NORM, 0, 94);
		m_pPlayer->m_iWeaponVolume = KNIFE_WALLHIT_VOLUME * 0.1;
		fDidHit = TRUE;
		break;
	}
	}

	if (m_iUseSkill)
	{
		UTIL_MakeVectors(m_pPlayer->pev->v_angle);
		Vector vecSrc = m_pPlayer->GetGunPosition();
		TraceResult tr;

		Vector vecEnd = vecSrc + gpGlobals->v_forward * 8192;
		UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr);
		CSkullT9Storm* pEnt = static_cast<CSkullT9Storm*>(CBaseEntity::Create("skullt9_storm", tr.vecEndPos, { 0,0,0 }, ENT(m_pPlayer->pev)));
		if (pEnt)
		{
			pEnt->RadiusDamage(tr.vecEndPos, GetEffectAttackDamage());
		}
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_AUTO, "weapons/skullt9_skill_wind.wav", VOL_NORM, ATTN_NORM, 0, 94);
		m_iUseSkill = 0;
	}
#endif

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;
	SetThink(nullptr);

	//return fDidHit;
}

BOOL CKnifeSkullT9::IsSignal()
{
#ifndef CLIENT_DLL
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] == 100)
	{
		m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_skullt9_1.mdl");
		return TRUE;
	}
	else
	{
		m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_skullt9.mdl");
		m_CrossHairState = SKULLT9_AIM_NO;
		return FALSE;
	}
#endif
	return TRUE;
}

#ifndef CLIENT_DLL
hit_result_t CKnifeSkullT9::KnifeAttack(float flDamage, float flRadius, float flAngleDegrees, int bitsDamageType, entvars_t* pevInflictor, CBasePlayer* pAttackPlayer, bool IsPrimaryAttack)
{
	CBaseEntity* pEntity = NULL;
	hit_result_t result = HIT_NONE;
	TraceResult tr;
	UTIL_MakeVectors(pAttackPlayer->pev->v_angle);
	Vector vecEnd;
	Vector vecSrc = pAttackPlayer->GetGunPosition();

	Vector vecForward = gpGlobals->v_forward;
	Vector2D vecForward2D = vecForward.Make2D().Normalize();

	float c = cos(flAngleDegrees * 0.5 * M_PI / 180.0);

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
				Vector vecDir = (player->pev->origin - pev->origin);
				vecDir.z = 0;
				if (!IsPrimaryAttack)
				{
					if (!(player->m_iBuff & BUFF_IGNORE_KNOCKBACK))
					{
						Vector2D vecDir2D = (player->pev->origin.Make2D() - m_pPlayer->pev->origin.Make2D()).Normalize();

						player->pev->velocity.x = vecDir2D.x * 600;
						player->pev->velocity.y = vecDir2D.y * 600;
						player->pev->velocity.z = 310;

						player->m_flVelocityModifier = 1.0;
					}
				}
				else
				{
					Vector2D vecDir2D = (player->pev->origin.Make2D() - m_pPlayer->pev->origin.Make2D()).Normalize();

					player->pev->velocity.x = vecDir2D.x * 600;
					player->pev->velocity.y = vecDir2D.y * 600;

					player->m_flVelocityModifier = 1.0;
				}
			}
		}
	}

	if (!result)
	{
		static int calced = 0;
		static float calcRes = 0.0f;

		if (!(calced & 1))
		{
			calced |= 1;
			calcRes = sqrt(512.0);
		}

		vecEnd = vecSrc + vecForward * (flRadius - calcRes);

		UTIL_TraceHull(vecSrc, vecEnd, dont_ignore_monsters, human_hull, pAttackPlayer->edict(), &tr);

		if (tr.flFraction < 1.0f)
			result = HIT_WALL;
	}

	return result;
}
#endif

void CKnifeSkullT9::Holster(int skiplocal)
{
#ifndef CLIENT_DLL
	MESSAGE_BEGIN(MSG_ONE, gmsgSpecialCrossHair, NULL, m_pPlayer->pev);
	WRITE_BYTE(2);
	WRITE_BYTE(3);
	MESSAGE_END();
#endif

	m_flTimeholster = gpGlobals->time;
	return CBasePlayerWeapon::Holster(skiplocal);
}

void CKnifeSkullT9::ItemPostFrame()
{
#ifndef CLIENT_DLL
	m_iButton = m_pPlayer->pev->button;

	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] < 100)
	{
		if (m_flTimeNextCharge <= gpGlobals->time)
		{
			m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]++;
			m_flTimeNextCharge = gpGlobals->time + 0.3s;

			if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] == 100)
				EMIT_SOUND_DYN(pev->owner, CHAN_WEAPON, "weapons/skullt9_fx.wav", VOL_NORM, ATTN_NORM, 0, 94);
		}
	}
	if (IsSignal())
	{
		m_CrossHairState = SKULLT9_AIM_ON;
	}
	else
		m_CrossHairState = SKULLT9_AIM_NO;
	

	if (m_flTimeNextMessage < gpGlobals->time)
	{
		if (m_CrossHairState != m_CrossHairStateStored)
		{
			
#ifndef CLIENT_DLL
			MESSAGE_BEGIN(MSG_ONE, gmsgSpecialCrossHair, NULL, m_pPlayer->pev);
			WRITE_BYTE(2);
			WRITE_BYTE(m_CrossHairState);
			MESSAGE_END();
#endif
		}

#ifndef CLIENT_DLL
		MESSAGE_BEGIN(MSG_ONE_UNRELIABLE, gmsgAmmoX, NULL, m_pPlayer->pev);
		WRITE_BYTE(m_iPrimaryAmmoType);
		WRITE_SHORT(m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]);
		MESSAGE_END();
#endif

		m_flTimeNextMessage = gpGlobals->time + 0.1s;
		m_CrossHairStateStored = m_CrossHairState;
	}
#endif
	return CBasePlayerWeapon::ItemPostFrame();
}

WeaponBotStrategy CKnifeSkullT9::GetBotStrategy() const
{
    return BOT_STRATEGY_SKULLAXE;
}


}
