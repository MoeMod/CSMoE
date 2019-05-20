
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "weapons/WeaponTemplate.hpp"
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

class CBeam;

#ifndef CLIENT_DLL
class CGungnirProjectile : public CBaseEntity
{
public:
	void Spawn() override
	{
		Precache();

		m_fSequenceLoops = 0;
		//ph26 = 0;
		SetThink(&CGungnirProjectile::OnThink);
		SetTouch(&CGungnirProjectile::OnTouch);
		SET_MODEL(this->edict(), "sprites/ef_gungnir_missile.spr");
		pev->rendermode = kRenderTransAdd; // 5;
		pev->renderfx = kRenderFxNone;
		pev->renderamt = 255.0;

		//ph32 = ?
		pev->framerate = 10;
		pev->scale = 0.2;
		pev->solid = SOLID_BBOX; // 2
		pev->movetype = MOVETYPE_FLYMISSILE; // 9
		pev->nextthink = gpGlobals->time + 0.0099999998;
		m_flAnimEndTime = gpGlobals->time + 2.0; // ph27?
		m_flMaxFrames = 300.0;
		/*
		v9 = 0x40800000;
		v10 = 0x40800000;
		v11 = 0x40800000;
		v6 = 80000000800000008000000080000000h ^ 0x40800000;
		v7 = 80000000800000008000000080000000h ^ 0x40800000;
		v8 = 80000000800000008000000080000000h ^ 0x40800000;
		return UTIL_SetSize((int)v4, (int)&v6, (int)&v9);
		*/
		UTIL_SetSize(pev, { -4, -4, -4 }, { 4, 4, 4 });
	}

	void Precache() override
	{
		PRECACHE_MODEL("sprites/ef_gungnir_missile.spr");
		PRECACHE_MODEL("sprites/ef_gungnir_bexplo.spr");
	}

	KnockbackData GetKnockBackData()
	{
		return { 0.0, 0.0, 0.0, 0.0, 1.0 };
	}

	void EXPORT OnTouch(CBaseEntity *pOther)
	{
		if (pev->owner == pOther->edict())
			return;

		CBaseEntity *pAttacker = CBaseEntity::Instance(pev->owner);
		CBasePlayer *pAttackePlayer = nullptr;
		if (pAttacker && pAttacker->IsPlayer())
			pAttackePlayer = static_cast<CBasePlayer *>(pAttacker);

		if (pAttackePlayer &&
			pOther->pev->takedamage != DAMAGE_NO &&
			pOther->IsAlive() &&
			pOther->pev->team == m_iTeam
			)
		{
			Vector vecDirection = (pOther->pev->origin - pev->origin).Normalize();

			TraceResult tr;
			UTIL_TraceLine(pev->origin, pOther->pev->origin, missile, ENT(pAttackePlayer->pev), &tr);
			tr.iHitgroup = HITGROUP_CHEST; // ...

			ClearMultiDamage();
			pOther->TraceAttack(pAttackePlayer->pev, m_flTouchDamage, vecDirection, &tr, DMG_BULLET);
			ApplyMultiDamage(pAttackePlayer->pev, pAttackePlayer->pev);
		}

		RadiusDamage();
	}

	void EXPORT OnThink()
	{
		if (gpGlobals->time <= m_flAnimEndTime)
		{
			this->pev->frame = (float)(this->pev->framerate * gpGlobals->frametime) + this->pev->frame;
			if (pev->frame > m_flMaxFrames)
			{
				pev->frame = fmod(pev->frame, m_flMaxFrames);
			}
			pev->nextthink = gpGlobals->time + 0.0099999998;
		}
		else
		{
			Remove();
		}
	}

	void RadiusDamage()
	{
		const float flRadius = m_flExplodeRadius;
		const float flDamage = m_flExplodeDamage;
		const Vector vecSrc = pev->origin;
		entvars_t * const pevAttacker = VARS(pev->owner);
		entvars_t * const pevInflictor = this->pev;
		int bitsDamageType = DMG_BULLET;

		TraceResult tr;
		const float falloff = flRadius ? flDamage / flRadius : 1;
		const int bInWater = (UTIL_PointContents(vecSrc) == CONTENTS_WATER);

		CBaseEntity *pEntity = NULL;
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
					}

					/*CBasePlayer *pVictim = dynamic_cast<CBasePlayer *>(pEntity);
					if (pVictim->m_bIsZombie) // Zombie Knockback...
					{
						ApplyKnockbackData(pVictim, vecSpot - vecSrc, GetKnockBackData());
					}*/
				}
			}
		}

		MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
		WRITE_BYTE(TE_EXPLOSION);
		WRITE_COORD(pev->origin.x);
		WRITE_COORD(pev->origin.y);
		WRITE_COORD(pev->origin.z);
		WRITE_SHORT(MODEL_INDEX("sprites/ef_gungnir_bexplo.spr"));
		WRITE_BYTE(3);
		WRITE_BYTE(15);
		WRITE_BYTE(TE_EXPLFLAG_NODLIGHTS | TE_EXPLFLAG_NOPARTICLES | TE_EXPLFLAG_NOSOUND);
		MESSAGE_END();

		return Remove();
	}

	void Init(Vector vecVelocity, float flTouchDamage, float flExplodeDamage, float flExplodeRadius, TeamName iTeam)
	{
		std::tie(m_flTouchDamage, m_flExplodeDamage, m_flExplodeRadius, m_iTeam) = std::make_tuple(flTouchDamage, flExplodeDamage, flExplodeRadius, iTeam);
		pev->velocity = std::move(vecVelocity);
	}

	int m_fSequenceLoops;
	float m_flAnimEndTime;
	float m_flMaxFrames;
	float m_flTouchDamage;
	float m_flExplodeDamage;
	float m_flExplodeRadius;
	TeamName m_iTeam;

protected:
	void Remove()
	{
		SetThink(nullptr);
		SetTouch(nullptr);
		pev->effects |= EF_NODRAW; // 0x80u
		return UTIL_Remove(this);
	}
};
LINK_ENTITY_TO_CLASS(gungnir_projectile, CGungnirProjectile)

class CGungnirSpear : public CBaseAnimating
{
public:
	void Spawn() override
	{
		Precache();
		ph4 = 0;
		ph5 = 0;
		ph6 = 0;

		m_fSequenceLoops = 0;
		//ph26 = 0;
		SetTouch(&CGungnirSpear::OnTouch);
		SetThink(&CGungnirSpear::FlyThink);
		SET_MODEL(this->edict(), "sprites/gungnir_missile.spr");

		//ph32 = ?
		pev->solid = SOLID_CUSTOM; // 5
		pev->movetype = MOVETYPE_FLY; // 5
		pev->nextthink = gpGlobals->time + 0.0099999998;
		ph7 = gpGlobals->time + 1.0;
		ph8 = 300.0;
		this_1_has_disconnected = 0;
		UTIL_SetSize(pev, { -3, -3, -3 }, { 3, 3, 3 });

	}

	void Precache() override
	{
		PRECACHE_MODEL("models/gungnir_missile.mdl");
		PRECACHE_MODEL("sprites/ef_gungnir_chargeexplo.spr");
		PRECACHE_MODEL("sprites/ef_gungnir_lightline1.spr");
		PRECACHE_MODEL("sprites/ef_gungnir_lightline2.spr");
	}

	KnockbackData GetKnockBackData()
	{
		return { 1100.0f, 500.0f, 700.0f, 400.0f, 0.89999998f };
	}

	void EXPORT FlyThink()
	{
		if (gpGlobals->time < ph7)
		{
			this->pev->nextthink = gpGlobals->time + 0.0099999998;


		}
		else
		{
			TouchWall();
		}
	}

	void EXPORT OnTouch(CBaseEntity *pOther)
	{
		if (gpGlobals->time <= this_1_m_iSwing)
		{
			if (pOther && pOther->IsBSPModel())
			{
				TouchWall();
			}
			else if (pOther->pev->pContainingEntity != this->pev->owner)
			{
				TouchEntity(pOther);
			}
		}
	}

	void TouchWall()
	{

	}

	void TouchEntity(CBaseEntity *pOther)
	{

	}


	int ph4;
	int ph5;
	int ph6;
	float ph7;
	float ph8; // m_pfnThink?
	float this_1_m_iSwing;
	short this_1_has_disconnected;

protected:
	void Remove()
	{
		SetThink(nullptr);
		SetTouch(nullptr);
		pev->effects |= EF_NODRAW; // 0x80u
		return UTIL_Remove(this);
	}
};
LINK_ENTITY_TO_CLASS(gungnir_spear, CGungnirSpear)

#endif

class CGungnir : public LinkWeaponTemplate< CGungnir,
	TGeneralData,
	BuildTGetItemInfoFromCSW<WEAPON_AK47>::template type,
	TPrecacheEvent,
	TReloadDefault,
	TWeaponIdleDefault,
	TGetDamageDefault
>
{
public:

	enum
	{
		ANIM_IDLE1,
		ANIM_RELOAD,
		ANIM_DRAW,
		ANIM_SHOOT_START,
		ANIM_SHOOT_LOOP,
		ANIM_SHOOT_END,
		ANIM_SHOOT_B,
		ANIM_SHOOT_B_CHARGE,
		ANIM_CHARGE_SHOOT,
		ANIM_CHARGE_LOOP
	};

	static constexpr auto DefaultReloadTime = 1.9;
	static constexpr const char *V_Model = "models/v_gungnir.mdl";
	static constexpr const char *P_Model = "models/p_gungnira.mdl";
	static constexpr const char *W_Model = "models/w_gungnir.mdl";
	static constexpr const char *EventFile = "events/gungnir.sc";
	static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
	static constexpr const char *ClassName = "weapon_gungnir";
	static constexpr const char *AnimExtension = "m249";
	struct ItemInfoData_t : Base::ItemInfoData_t
	{
		static constexpr const char *szAmmo1 = "GungnirAmmo";
		static constexpr int iMaxAmmo1 = 100;
	};
	static constexpr int MaxClip = 50;
	static constexpr float WeaponIdleTime = 10;

	KnockbackData KnockBack = { 250.0f, 100.0f, 150.0f, 100.0f, 0.2f };
	static constexpr const char *Beam_SPR = "sprites/ef_gungnir_xbeam.spr"; //

public:
	void Precache() override;
	BOOL Deploy() override;
	void Spawn() override;
	void Holster(int skiplocal) override;
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	bool HasSecondaryAttack() override { return true; }
	void ItemPostFrame() override;

#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_gungnir" , 200 }; }
#endif

public:
	void CreateEffect();
	void DestroyEffect();
	void PrimaryAttack_FindTargets();
	bool PrimaryAttack_CheckTargetAvailable(CBaseEntity *a2, Vector vecDirection);
	void PrimaryAttack_InstantDamage();
	void ClearEffect();
	void ShootProjectile();
	void ShootSpear();

	double GetDamage_PrimaryAttack_Instant() const
	{
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZBS)
			return 1092.0;
		if (g_pModRunning->DamageTrack() == DT_ZB)
			return 304.0;
#endif
		return 20.0;
	}
	float GetDamage_ProjectileA() const
	{
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZBS)
			return 5100.0f;
		if (g_pModRunning->DamageTrack() == DT_ZB)
			return 1100.0f;
#endif
		return 70.0f;
	}
	double GetDamage_ProjectileB() const
	{
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZBS)
			return 1000.0;
		if (g_pModRunning->DamageTrack() == DT_ZB)
			return 900.0;
#endif
		return 50.0;
	}
	double GetDamage_SpearA() const
	{
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZBS)
			return 27500.0;
		if (g_pModRunning->DamageTrack() == DT_ZB)
			return 6900.0;
#endif
		return 120.0;
	}
	double GetDamage_SpearB() const
	{
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZBS)
			return 2500.0;
		if (g_pModRunning->DamageTrack() == DT_ZB)
			return 1500.0;
#endif
		return 80.0;
	}
	double GetDamage_SpearC() const
	{
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZBS)
			return 4100.0;
		if (g_pModRunning->DamageTrack() == DT_ZB)
			return 2000.0;
#endif
		return 50.0;
	}

public:
	float phs2;	// secondary attack start time
	float phs3; // primary attack start time
	float phs4; // spear attack end time... cannot switch weapon?
	std::array<CBeam *, 3> phs5_6_7; // EHANDLE ?
	// unsigned short phs8; // m_usFireGungnir
	std::vector<EHANDLE> phs9_10_11;
	float phs12;
};
LINK_ENTITY_TO_CLASS(weapon_gungnir, CGungnir)

void CGungnir::Precache()
{
	PRECACHE_MODEL(const_cast<char *>(Beam_SPR));
	PRECACHE_MODEL("models/p_gungnirB.mdl");
	return Base::Precache();

}

BOOL CGungnir::Deploy()
{
	phs2 = -1;
	phs3 = -1;
	phs4 = -1;
	phs12 = -1; // 0xBF800000

	CreateEffect();
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0f;

	return DefaultDeploy(V_Model, P_Model, ANIM_DRAW, AnimExtension, UseDecrement() != FALSE);
}

void CGungnir::Spawn()
{
	Base::Spawn();

	const double flAngle = 60.0 * 0.0174532925199433;

	phs2 = -1;
	phs3 = -1;
	phs4 = -1;
	phs12 = -1;
}

void CGungnir::Holster(int skiplocal)
{
	phs2 = -1;
	phs3 = -1;
	phs4 = -1;
	phs12 = -1;

	// clear target list ?
	ClearEffect();
	DestroyEffect();
	return Base::Holster();
}

void CGungnir::PrimaryAttack()
{
	++m_iShotsFired;
	m_bDelayFire = 1;
	if (m_iClip <= 0)
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.2;
		}
		return; // sth ignored
	}
	--m_iClip;
	if (phs2 > 0.0f)
		phs2 = -1.0f; // 0xBF800000
	if (phs3 == -1.0f)
	{
		SendWeaponAnim(ANIM_SHOOT_START, UseDecrement() != FALSE); // 3
		phs3 = gpGlobals->time + 0.23;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 99999.0;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 99999.0;
	}
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1); // 5
#endif
	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME; // 600
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH; // 512
	m_pPlayer->pev->effects |= EF_MUZZLEFLASH; // 2u

	bool v6 = gpGlobals->time > phs12 + 1.0f;
	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif
	PLAYBACK_EVENT_FULL(1, m_pPlayer->edict(), m_usFire, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0, 0, v6, 0, FALSE, FALSE);

	if (v6)
		phs12 = v6 = gpGlobals->time;
	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.12f;

#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.9f;

	PrimaryAttack_FindTargets();
	PrimaryAttack_InstantDamage();
}

void CGungnir::ShootProjectile()
{
	if (m_iClip <= 0)
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.2;
		}
		return; // sth ignored
	}
	m_iClip = std::max(m_iClip - 5, 0);

#ifndef CLIENT_DLL
	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
	Vector vecSrc = m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 10;
	CGungnirProjectile *pEnt = static_cast<CGungnirProjectile *>(CBaseEntity::Create("gungnir_projectile", vecSrc, m_pPlayer->pev->v_angle, ENT(m_pPlayer->pev)));
	if (pEnt)
	{
		pEnt->Init(gpGlobals->v_forward * 1500, GetDamage_ProjectileA(), GetDamage_ProjectileB(), 110, m_pPlayer->m_iTeam);
	}
#endif

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.58;

	PLAYBACK_EVENT_FULL(1, m_pPlayer->edict(), m_usFire, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0, 0, 3, 0, FALSE, FALSE);

#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
}

void CGungnir::ShootSpear()
{
#ifndef CLIENT_DLL
	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
	Vector vecSrcA = m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 10 + gpGlobals->v_right * 5;
	Vector vecSrcB = m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 10 + -gpGlobals->v_right * (-5);
	CBaseEntity *pEnt = CBaseEntity::Create("gungnir_spear", vecSrcA, m_pPlayer->pev->v_angle, ENT(m_pPlayer->pev));
	if (pEnt)
	{
		const auto team = m_pPlayer->m_iTeam;
		Vector vecVelocity = gpGlobals->v_forward * 2000;
		const float dmg1 = GetDamage_SpearA();
		const float dmg2 = GetDamage_SpearB();
		const float dmg3 = GetDamage_SpearC();
		const float exp_radius = 110;
		// set ent...
	}
#endif

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 3.12;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.13;
	m_iClip = std::max(m_iClip - 5, 0);

	PLAYBACK_EVENT_FULL(1, m_pPlayer->edict(), m_usFire, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0, 0, 4, 0, FALSE, FALSE);

#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
}

void CGungnir::SecondaryAttack()
{
	if (phs3 <= 0.0 && m_iClip > 0)
	{
		if (phs2 == -1)
			phs2 = gpGlobals->time;

		m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 9999.0f;
	}
}

void CGungnir::ItemPostFrame()
{
	if (phs4 > 0.0f && gpGlobals->time > phs4)
	{
		m_pPlayer->pev->weaponmodel = MAKE_STRING(P_Model);
		phs4 = -1;
		return;
	}

	if (phs2 <= 0.0f)
	{
		if (phs3 > 0.0f)
		{
			if (this->m_pPlayer->pev->button & IN_ATTACK && this->m_iClip > 0)
			{
				if (gpGlobals->time > phs3)
				{
					this->SendWeaponAnim(ANIM_SHOOT_LOOP, UseDecrement() != FALSE); // 4
					phs3 = gpGlobals->time + 0.5f;
					return CBasePlayerWeapon::ItemPostFrame();
				}
			}
			else
			{
				this->SendWeaponAnim(ANIM_SHOOT_END, UseDecrement() != FALSE); // 5
				phs3 = -1;
				m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.4f;
				ClearEffect();
				PLAYBACK_EVENT_FULL(1, m_pPlayer->edict(), m_usFire, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0, 0, 2, 0, FALSE, FALSE);
				phs12 = -1;
			}
		}
	}
	else
	{
		if (this->m_pPlayer->pev->button & IN_ATTACK2) // 0x800
		{
			const int v16 = m_pPlayer->pev->weaponanim;
			if (gpGlobals->time > phs2 + 0.64999998)
			{
				if (gpGlobals->time - phs2 >= 0.64999998 + 2.03)
				{
					if (v16 != ANIM_CHARGE_LOOP) // 9
					{
						SendWeaponAnim(ANIM_CHARGE_LOOP, UseDecrement() != FALSE);
						return CBasePlayerWeapon::ItemPostFrame();
					}
				}
			}
			else if (v16 != ANIM_SHOOT_B_CHARGE) // 7
			{
				SendWeaponAnim(ANIM_SHOOT_B_CHARGE, UseDecrement() != FALSE);
				Q_strcpy(m_pPlayer->m_szAnimExtention, "gungnir");
				m_pPlayer->pev->weaponmodel = MAKE_STRING("models/p_gungnirB.mdl");
				return CBasePlayerWeapon::ItemPostFrame();
			}
		}
		else
		{
			if (gpGlobals->time - phs2 < 0.64999998)
			{
#ifndef CLIENT_DLL
				m_pPlayer->SetAnimation(PLAYER_ATTACK1); //5
#endif
				SendWeaponAnim(ANIM_SHOOT_B, UseDecrement() != FALSE); // 6
				ShootProjectile();
				phs2 = -1;
				return CBasePlayerWeapon::ItemPostFrame();
			}
			else if (gpGlobals->time - phs2 >= 0.64999998 + 2.03)
			{
#ifndef CLIENT_DLL
				m_pPlayer->SetAnimation(PLAYER_ATTACK1); //5
#endif
				SendWeaponAnim(ANIM_CHARGE_SHOOT, UseDecrement() != FALSE); // 8
				ShootSpear();
				phs2 = -1;
				Q_strcpy(m_pPlayer->m_szAnimExtention, "m249");
				m_pPlayer->pev->weaponmodel = MAKE_STRING("models/null.mdl");
				phs4 = gpGlobals->time + fminf(3.1199999, 0.2);
				return CBasePlayerWeapon::ItemPostFrame();
			}
			else if (m_pPlayer->pev->weaponanim != ANIM_SHOOT_B_CHARGE) // 7
			{
				SendWeaponAnim(ANIM_SHOOT_B_CHARGE, UseDecrement() != FALSE); // 7
				return CBasePlayerWeapon::ItemPostFrame();
			}
		}
	}
	return CBasePlayerWeapon::ItemPostFrame();
}

void CGungnir::CreateEffect()
{
#ifndef CLIENT_DLL
	for (size_t i = 0; i < 3; ++i)
	{
		CBeam *pBeam = CBeam::BeamCreate(Beam_SPR, 10);
		pBeam->SetColor(255, 255, 255);
		pBeam->SetScrollRate(15);
		pBeam->SetBrightness(0);
		pev->effects |= EF_NODRAW;

		phs5_6_7[i] = pBeam;
	}
#endif
}

void CGungnir::DestroyEffect()
{
#ifndef CLIENT_DLL
	for (CBeam *p : phs5_6_7)
	{
		if (p)
			p->SUB_Remove();
	}
#endif
}

void CGungnir::PrimaryAttack_FindTargets()
{
	const float flRadius = 320;

	phs9_10_11.clear();
#ifndef CLIENT_DLL
	CBaseEntity *pEntity = NULL;
	while ((pEntity = UTIL_FindEntityInSphere(pEntity, m_pPlayer->pev->origin, flRadius)) != nullptr)
	{
		if (PrimaryAttack_CheckTargetAvailable(pEntity, m_pPlayer->pev->v_angle))
		{
			EHANDLE eh;
			eh.Set(pEntity->edict());
			phs9_10_11.push_back(eh);
		}
	}
#endif
}

bool CGungnir::PrimaryAttack_CheckTargetAvailable(CBaseEntity * a2, Vector vecAngleDirection)
{
	const float flRadius = 320;

	if (!a2->IsAlive())
		return false;

#ifndef CLIENT_DLL
	if (g_pGameRules->PlayerRelationship(m_pPlayer, a2) == GR_TEAMMATE)
		return false;
#endif

	if (a2->pev == m_pPlayer->pev)
		return false;

	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecDelta = (a2->Center() - vecSrc).Normalize();

	UTIL_MakeVectors(vecAngleDirection);

	if (DotProduct(gpGlobals->v_forward, vecDelta) < 0.5)
		return false;

	return true;
}

void CGungnir::PrimaryAttack_InstantDamage()
{
	if (phs9_10_11.empty())
	{
		ClearEffect();
		if (!phs5_6_7.empty())
		{
			// ???
		}
		return;
	}

	//std::random_shuffle(phs9_10_11.begin(), phs9_10_11.end());
	size_t v8 = 0;
	for (CBaseEntity *pEntity : phs9_10_11)
	{
		if (v8 >= 3)
			break;
		if (!pEntity)
			continue;
		Vector vecDirection = (pEntity->pev->origin - m_pPlayer->pev->origin).Normalize();

		TraceResult tr;
		UTIL_TraceLine(m_pPlayer->pev->origin, pEntity->pev->origin, missile, ENT(m_pPlayer->pev), &tr);

		ClearMultiDamage();
		pEntity->TraceAttack(m_pPlayer->pev, GetDamage_PrimaryAttack_Instant(), vecDirection, &tr, DMG_BULLET);
		ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);


		if (v8 < 3)
		{
#ifndef CLIENT_DLL
			CBeam *pBeam = phs5_6_7[v8];
			if (pBeam)
			{
				//pBeam->EntsInit(ENTINDEX(m_pPlayer->edict()), ENTINDEX(pEntity->edict()));
				pBeam->SetType(BEAM_ENTS);
				pBeam->SetStartEntity(ENTINDEX(m_pPlayer->edict()));
				pBeam->SetEndEntity(ENTINDEX(pEntity->edict()));
				pBeam->SetStartAttachment(1);
				pBeam->SetEndAttachment(0);
				pBeam->RelinkBeam();
				pBeam->SetBrightness(230);
				pBeam->pev->effects &= ~EF_NODRAW;

			}

			MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pEntity->pev->origin);
			WRITE_BYTE(TE_EXPLOSION);
			WRITE_COORD(pEntity->pev->origin.x);
			WRITE_COORD(pEntity->pev->origin.y);
			WRITE_COORD(pEntity->pev->origin.z);
			WRITE_SHORT(MODEL_INDEX("sprites/ef_gungnir_aexplo.spr"));
			WRITE_BYTE(3);
			WRITE_BYTE(15);
			WRITE_BYTE(TE_EXPLFLAG_NODLIGHTS | TE_EXPLFLAG_NOPARTICLES | TE_EXPLFLAG_NOSOUND);
			MESSAGE_END();
#endif
			++v8;
		}
	}
}

void CGungnir::ClearEffect()
{
#ifndef CLIENT_DLL
	for (CBeam *pBeam : phs5_6_7)
	{
		if (pBeam)
		{
			pBeam->SetBrightness(0);
			pev->effects |= EF_NODRAW; // 0x80
		}
	}
#endif
	phs9_10_11.clear();
}

}

