/*
z4b_oppovivo.cpp
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

#include "weapons/WeaponTemplate.hpp"
#include "weapons/KnifeAttack.h"

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif


#ifndef CLIENT_DLL
	class CMechanicHandGrenade : public CBaseEntity
	{
	public:
		float GetArmorRatioModifier() { return 1.5; }
		int GetWeaponsId(void) { return WEAPON_KNIFE; }
		void Spawn() override
		{
			Precache();

			SetThink(&CMechanicHandGrenade::FollowThink);
			SetTouch(&CMechanicHandGrenade::OnTouch);
			SET_MODEL(this->edict(), "models/z4b/ef_mechanic_arcane.mdl");

			pev->rendermode = kRenderTransAdd;
			pev->renderfx = kRenderFxNone;
			pev->renderamt = 200;
			pev->framerate = 30;
			pev->scale = 1.0;
			pev->solid = SOLID_BBOX;
			pev->movetype = MOVETYPE_FLYMISSILE;
			pev->nextthink = gpGlobals->time + 0.1s;

			UTIL_SetOrigin(pev, pev->origin);
			UTIL_SetSize(pev, Vector(-1.0, -1.0, -1.0), Vector(1.0, 1.0, 1.0));
		}

		void Precache() override
		{
			PRECACHE_MODEL("models/z4b/ef_mechanic_arcane.mdl");
			m_iSprExplo = PRECACHE_MODEL("sprites/z4b/ef_fexplo.spr");
			PRECACHE_SOUND("CK_WPN/WEAPON/Mechanic/arcane_explosion.wav");
		}

		void EXPORT OnTouch(CBaseEntity* pOther)
		{
			Explode(pOther);
		}

		void Explode(CBaseEntity* pOther)
		{
			EMIT_SOUND_DYN(this->edict(), CHAN_ITEM, "CK_WPN/WEAPON/Mechanic/arcane_explosion.wav", 0.8, ATTN_NORM, 0, PITCH_NORM);

			MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
			WRITE_BYTE(TE_TEMPSPRITE);
			WRITE_COORD(pev->origin.x);
			WRITE_COORD(pev->origin.y);
			WRITE_COORD(pev->origin.z);
			WRITE_SHORT(m_iSprExplo);
			WRITE_BYTE(8);
			WRITE_BYTE(200);
			WRITE_BYTE(30);
			MESSAGE_END();

			CBaseEntity* pAttacker = CBaseEntity::Instance(pev->owner);
			CBasePlayer* pAttackePlayer = nullptr;
			if (pAttacker && pAttacker->IsPlayer())
				pAttackePlayer = static_cast<CBasePlayer*>(pAttacker);

			if (!pAttackePlayer)
				return;

			TraceResult tr;
			CBaseEntity* pEntity = NULL;

			Vector vecEnd;
			int bInWater = (UTIL_PointContents(pev->origin) == CONTENTS_WATER);
			float flRadius = 130.0;
			while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, flRadius)) != NULL)
			{
				if (pEntity->edict() == pev->owner)
					continue;

				if (pEntity->IsDormant())
					continue;

				if (pEntity->pev->takedamage == DAMAGE_NO)
					continue;

				if (pEntity->Classify() == CLASS_NONE)
					continue;

				if (g_pGameRules->PlayerRelationship(pAttackePlayer, pEntity) == GR_TEAMMATE)
					continue;

				vecEnd = pEntity->BodyTarget(pev->origin);

				UTIL_TraceLine(pev->origin, vecEnd, ignore_monsters, pev->owner, &tr);

				if (tr.flFraction < 1.0 && tr.pHit != pEntity->edict())
					continue;

				int falloff = RadiusDamageAmount / flRadius;

				float flCurrentDamage = max(0, RadiusDamageAmount - (pev->origin - pEntity->pev->origin).Length() * falloff);

				pEntity->TakeDamage(pev, pAttackePlayer->pev, flCurrentDamage, DMG_EXPLOSION);

				Vector vecDirection = (pEntity->pev->origin - pAttackePlayer->pev->origin).Normalize();
				UTIL_AddKickRateBoost(pEntity, vecDirection.Make2D(), 1.0, 1500.0, 500);			
			}

			SetThink(nullptr);
			SetTouch(nullptr);
			UTIL_Remove(this);
		}

		void EXPORT FollowThink()
		{
			if (m_flRemoveTime < gpGlobals->time)
				Explode(NULL);

			pev->nextthink = gpGlobals->time + 0.1s;
		}

		void Init(Vector vecVelocity, float flExplodeDamage, int iTeam)
		{
			m_flRemoveTime = gpGlobals->time + 5.0s;
			std::tie(RadiusDamageAmount, m_iTeam) = std::make_tuple(flExplodeDamage, iTeam);
			pev->velocity = std::move(vecVelocity);
		}

		KnockbackData GetKnockBackData() override { return { 500.0f, 650.0f, 800.0f, 300.0f, 0.5f }; }

		time_point_t m_flRemoveTime;;
		float RadiusDamageAmount;
		int m_iSprExplo;

	};
	LINK_ENTITY_TO_CLASS(mechanichand_grenade, CMechanicHandGrenade)
#endif

class CKnifeMechanicHand: public LinkWeaponTemplate<CKnifeMechanicHand,
	TGeneralData,
	BuildTGetItemInfoFromCSW<WEAPON_KNIFE>::template type,
	TDeployDefault
>
{
public:
	static constexpr const char *ClassName = "z4b_mechanichand";
	static constexpr const char *V_Model = "models/z4b/v_mechanichand.mdl";
	static constexpr const char *P_Model = "models/null.mdl";
	static constexpr const char *W_Model = "models/w_knife.mdl";

	static constexpr int MaxClip = -1;
	static constexpr auto ItemSlot = KNIFE_SLOT;
	static constexpr const char *AnimExtension = "knife";
	KnockbackData KnockBack = { 0.0f, 0.0f, 0.0f, 0.0f, 0.7f };

public:

	enum skullaxe_e
	{
		ANIM_IDLE1 = 0,
		ANIM_NONE,
		ANIM_WSSB,
		ANIM_DRAW,
		ANIM_STAB1,
		ANIM_STAB2,
		ANIM_SLASH1,
		ANIM_SLASH2,
		ANIM_RUN,
		ANIM_SKILL,
	};
	void Precache() override
	{
		Base::Precache();

		PRECACHE_SOUND("CK_WPN/WEAPON/Mechanic/Mechanic_Swing.WAV");
		PRECACHE_SOUND("CK_WPN/WEAPON/Mechanic/Attack_11.wav");
		PRECACHE_SOUND("CK_WPN/WEAPON/Mechanic/Attack_22.WAV");
		PRECACHE_SOUND("CK_WPN/WEAPON/Mechanic/Attack_33.WAV");
		PRECACHE_SOUND("CK_WPN/WEAPON/Mechanic/Mechanic_Stab.WAV");
		PRECACHE_SOUND("CK_WPN/WEAPON/Mechanic/Mechanic_Hit.WAV");
		PRECACHE_SOUND("CK_WPN/WEAPON/Mechanic/Arcane_Fire.WAV");

	}

	BOOL Deploy() override
	{
		m_fMaxSpeed = 250;
		m_iSwing = 0;
		pev->iuser1 = 0;
		m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;
		m_pPlayer->m_bShieldDrawn = false;
		ResetRunIdle();

		if (m_flTimeHolster != invalid_time_point)
		{
			m_iClip += (gpGlobals->time - m_flTimeHolster) / 0.125s;
			m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] += (gpGlobals->time - m_flTimeHolster) / 0.125s;
		}
		if (m_iClip >= 100)
			m_iClip = 100;

		if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] >= 100)
			m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 100;

		return Base::Deploy();
	}

	void ItemPostFrame() override;
	void Reload() override;
	int GetItemInfo(ItemInfo* p) override;
	void Holster(int skiplocal) override;
	void Spawn() override;
	float GetMaxSpeed() override { return m_fMaxSpeed; }
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	void SkillAttack();
	BOOL CanDrop() override { return false; }
	void WeaponIdle() override;
	int ExtractAmmo(CBasePlayerWeapon* pWeapon) override
	{
		if (m_iDefaultAmmo)
		{
			m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = m_iDefaultAmmo;
		}

		return CBasePlayerWeapon::ExtractAmmo(pWeapon);
	}
#ifndef CLIENT_DLL
	hit_result_t KnifeAttack(float flDamage, float flRadius, float flAngleDegrees, int bitsDamageType, entvars_t* pevInflictor, CBasePlayer* pAttackPlayer);
#endif

	time_point_t m_flTimeHolster;
	time_point_t m_flTimeNextCharge;
	duration_t m_flNextRunIdle;
	duration_t m_flNextIdle;

public:
	void ResetRunIdle()
	{
		m_flNextIdle = m_flNextRunIdle = UTIL_WeaponTimeBase();
	}

	void DelayPrimaryAttack();
	void DelaySecondaryAttack();

	float GetPrimaryAttackDamage() const
	{
		float flDamage = 47;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 280 + (g_pGameRules->m_iTotalRoundsPlayed * 50);
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage *= 5.5f;
#endif
		return flDamage;
	}
	float GetSecondaryAttackDamage() const
	{
		float flDamage = 120;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 1105 + (g_pGameRules->m_iTotalRoundsPlayed * 100);
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage *= 5.5f;
#endif
		return flDamage;
	}

	float GetRadiusDamage() const
	{
		float flDamage = 75.0f;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 2230.0f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 4563.0f;
#endif
		return flDamage;
	}
};

LINK_ENTITY_TO_CLASS(z4b_mechanichand, CKnifeMechanicHand)

int CKnifeMechanicHand::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "mechanichandammo";
	p->iMaxAmmo1 = -1;
	p->pszAmmo2 = "mechanichandammo2";
	p->iMaxAmmo2 = 100;
	p->iMaxClip = 100;
	p->iSlot = 2;
	p->iPosition = 1;
	p->iId = WEAPON_KNIFE;
	p->iFlags = 0;
	p->iWeight = KNIFE_WEIGHT;

	return 1;
}

void CKnifeMechanicHand::Spawn()
{
	m_iKnifeID = KNIFE_MECHANICHAND;
	pev->classname = MAKE_STRING("z4b_mechanichand");
	m_iId = WEAPON_KNIFE;
	Precache();
	SET_MODEL(ENT(pev), "models/w_knife.mdl");
	m_iDefaultAmmo = 100;
	m_flTimeHolster = gpGlobals->time;
	m_flTimeNextCharge = invalid_time_point;
	FallInit();
}

void CKnifeMechanicHand::Holster(int skiplocal)
{
	SetThink(nullptr);

	m_flTimeHolster = gpGlobals->time;
	return CBasePlayerWeapon::Holster(skiplocal);
}


void CKnifeMechanicHand::ItemPostFrame()
{
	if (m_flTimeNextCharge <= gpGlobals->time)
	{
		if (m_iClip < 100)
		{
			m_iClip++;
		}

		if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] < 100)
		{
			m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]++;
		}

		m_flTimeNextCharge = gpGlobals->time + 0.125s;
	}

	if (m_pPlayer->pev->button & IN_USE && m_pPlayer->m_flNextAttack < 0s)
	{
		CBasePlayerWeapon* pWeapon = (CBasePlayerWeapon*)m_pPlayer->m_rgpPlayerItems[GRENADE_SLOT];

		if (pWeapon)
			return CBasePlayerWeapon::ItemPostFrame();

		if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] >= 100)
		{
			m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 0;
#ifndef CLIENT_DLL
			m_pPlayer->GiveNamedItem("z4b_mechanicherogun");
#endif
		}
	}

	return CBasePlayerWeapon::ItemPostFrame();
}


void CKnifeMechanicHand::Reload()
{
	if (m_iClip < 100)
		return;

	m_iClip = 0;

#ifndef CLIENT_DLL
	strcpy(m_pPlayer->m_szCustomSequence, "M-shoot-knife-upper");
	m_pPlayer->ResetSequenceInfo();
	m_pPlayer->m_flCustomSequenceUntilTime = gpGlobals->time + 0.48s;
#endif

	SendWeaponAnim(ANIM_SKILL, UseDecrement() != FALSE);
	pev->nextthink = gpGlobals->time + 1.17s;
	SetThink(&CKnifeMechanicHand::SkillAttack);

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 2.2s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.2s;
	ResetRunIdle();
}

void CKnifeMechanicHand::SkillAttack()
{
	EMIT_SOUND_DYN(this->edict(), CHAN_WEAPON, "CK_WPN/WEAPON/Mechanic/Arcane_Fire.WAV", 0.8, ATTN_NORM, 0, PITCH_NORM);

	m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;

	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecDir = m_pPlayer->FireBullets3(vecSrc - gpGlobals->v_up * 2.0, gpGlobals->v_forward, 0.0, 8192.0, 0, BULLET_PLAYER_556MM, 0, 0.0, m_pPlayer->pev, false, m_pPlayer->random_seed);

#ifndef CLIENT_DLL
	Vector vecProjDir = gpGlobals->v_forward + gpGlobals->v_right * vecDir.x + gpGlobals->v_up * vecDir.y;

	CMechanicHandGrenade* pEnt = static_cast<CMechanicHandGrenade*>(CBaseEntity::Create("mechanichand_grenade", vecSrc, m_pPlayer->pev->v_angle, ENT(m_pPlayer->pev)));
	if (pEnt)
	{
		pEnt->Init(vecProjDir * 1000, GetRadiusDamage(), m_pPlayer->m_iTeam);
	}
#endif
}

void CKnifeMechanicHand::PrimaryAttack(void)
{
#ifndef CLIENT_DLL
	strcpy(m_pPlayer->m_szCustomSequence, "M-shoot-knife-upper");
	m_pPlayer->ResetSequenceInfo();
	m_pPlayer->m_flCustomSequenceUntilTime = gpGlobals->time + 0.48s;
#endif
	m_iSwing = 0;
	SendWeaponAnim(pev->iuser1++ ? ANIM_SLASH1 : ANIM_SLASH2, UseDecrement() != FALSE);
	if (pev->iuser1)
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "CK_WPN/WEAPON/Mechanic/Attack_11.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
	else
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "CK_WPN/WEAPON/Mechanic/Attack_22.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
	pev->iuser1 %= 2;
	SetThink(&CKnifeMechanicHand::DelayPrimaryAttack);
	pev->nextthink = gpGlobals->time + 0.17s;
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.8s;
	ResetRunIdle();
}

void CKnifeMechanicHand::SecondaryAttack(void)
{
#ifndef CLIENT_DLL
	strcpy(m_pPlayer->m_szCustomSequence, "M-bigshot_Knife");
	m_pPlayer->pev->frame = 0;
	m_pPlayer->ResetSequenceInfo();
	m_pPlayer->m_flCustomSequenceUntilTime = gpGlobals->time + 1.7s;
#endif
	SendWeaponAnim(RANDOM_LONG(0, 1)? ANIM_STAB1:ANIM_STAB2, UseDecrement() != FALSE);
	
	SetThink(&CKnifeMechanicHand::DelaySecondaryAttack);
	pev->nextthink = gpGlobals->time + 0.87s;

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.4s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;
	m_iSwing = 0;
	ResetRunIdle();
}

void CKnifeMechanicHand::DelayPrimaryAttack()
{
#ifndef CLIENT_DLL
	switch (KnifeAttack(GetPrimaryAttackDamage(), 85, 90, DMG_NEVERGIB | DMG_BULLET, this->pev, m_pPlayer))
	{
	case HIT_PLAYER:
	{
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "CK_WPN/WEAPON/Mechanic/Mechanic_Hit.WAV", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_BODY, "CK_WPN/WEAPON/DualKukri/UI_SPECIALKILL2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		break;
	}
	case HIT_WALL:
	{
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "CK_WPN/WEAPON/Mechanic/Mechanic_Swing.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		break;
	}
	}
#endif
}

void CKnifeMechanicHand::DelaySecondaryAttack()
{
	EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "CK_WPN/WEAPON/Mechanic/Attack_33.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
#ifndef CLIENT_DLL
	switch (KnifeAttack(GetSecondaryAttackDamage(), 135, 120, DMG_NEVERGIB | DMG_BULLET, this->pev, m_pPlayer))
	{
	case HIT_PLAYER:
	{
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "CK_WPN/WEAPON/Mechanic/Mechanic_Hit.WAV", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_BODY, "CK_WPN/WEAPON/DualKukri/UI_SPECIALKILL2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		break;
	}
	case HIT_WALL:
	{
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "CK_WPN/WEAPON/Mechanic/Mechanic_Stab.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		break;
	}
	}
#endif

	SetThink(nullptr);
}

#ifndef CLIENT_DLL	
hit_result_t CKnifeMechanicHand::KnifeAttack(float flDamage, float flRadius, float flAngleDegrees, int bitsDamageType, entvars_t* pevInflictor, CBasePlayer* pAttackPlayer)
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

		tr.iHitgroup = HITGROUP_HEAD;
		pEntity->TraceAttack(pAttackPlayer->pev, flDamage * flRatio, (tr.vecEndPos - vecSrc).Normalize(), &tr, bitsDamageType);

		ApplyMultiDamage(pevInflictor, pAttackPlayer->pev);

		result = HIT_PLAYER;
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

void CKnifeMechanicHand::WeaponIdle(void)
{
	ResetEmptySound();
	if (m_flTimeWeaponIdle < UTIL_WeaponTimeBase())
	{
		if (m_pPlayer->pev->button & (IN_FORWARD | IN_MOVELEFT | IN_MOVERIGHT | IN_BACK))
		{
			if (m_flNextRunIdle <= UTIL_WeaponTimeBase())
			{
				SendWeaponAnim(ANIM_RUN, UseDecrement() != FALSE);

				m_flNextRunIdle = UTIL_WeaponTimeBase() + 0.8s;
			}

			m_flNextIdle = UTIL_WeaponTimeBase();
		}
		else
		{
			if (m_flNextIdle <= UTIL_WeaponTimeBase())
			{
				SendWeaponAnim(ANIM_IDLE1, UseDecrement() != FALSE);

				m_flNextIdle = UTIL_WeaponTimeBase() + 20s;
			}

			m_flNextRunIdle = UTIL_WeaponTimeBase();
		}

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.1s;
	}

}

}
