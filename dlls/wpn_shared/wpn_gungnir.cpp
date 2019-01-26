
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "weapons/WeaponTemplate.hpp"

#include <vector>

#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif

class CGungnir: public LinkWeaponTemplate< CGungnir,
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
	static constexpr const char *P_Model = "models/p_gungnir.mdl";
	static constexpr const char *W_Model = "models/w_gungnir.mdl";
	static constexpr const char *EventFile = "events/gungnir.sc";
	static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
	static constexpr const char *ClassName = "weapon_gungnir";
	static constexpr const char *AnimExtension = "m249";
	struct ItemInfoData_t : Base::ItemInfoData_t
	{
		static constexpr int iFlags = ITEM_FLAG_EXHAUSTIBLE;
		static constexpr const char *szAmmo1 = "GungnirAmmo";
		static constexpr int iMaxAmmo1 = 200;
	};
	static constexpr int MaxClip = 50;
	static constexpr float WeaponIdleTime = 10;

public:
	BOOL Deploy() override;
	void Spawn() override;
	void Holster(int skiplocal) override;
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	bool HasSecondaryAttack() override { return true; }
	void ItemPostFrame() override;

public:
	void PrimaryAttack_FindTargets();
	bool PrimaryAttack_CheckTargetAvailable(CBaseEntity *a2, Vector vecDirection);
	void PrimaryAttack_InstantDamage();
	void sub_10334A80();
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
	std::vector<CBaseEntity *> phs5_6_7;
	// unsigned short phs8; // m_usFireGungnir
	std::vector<CBaseEntity *> phs9_10_11;
	float phs12;
};
LINK_ENTITY_TO_CLASS(weapon_gungnir, CGungnir)

BOOL CGungnir::Deploy()
{
	phs2 = -1;
	phs3 = -1;
	phs4 = -1;
	phs12 = -1; // 0xBF800000

	// CreateEffect

	// 0.75, 1.0
	BOOL result = DefaultDeploy(V_Model, P_Model, ANIM_DRAW, AnimExtension, UseDecrement() != FALSE);

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0f;

	return result;
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
	CBaseEntity *pEnt = CBaseEntity::Create("gungnir_projectile", vecSrc, m_pPlayer->pev->v_angle, ENT(m_pPlayer->pev));
	if (pEnt)
	{
		const auto team = m_pPlayer->m_iTeam;
		Vector vecVelocity = gpGlobals->v_forward * 1500;
		const float touch_dmg = GetDamage_ProjectileA();
		const float exp_dmg = GetDamage_ProjectileB();
		const float exp_radius = 110;
		// set ent...
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
				sub_10334A80();
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
				if (gpGlobals->time - phs2 >= 0.0f)
				{
					if (v16 != ANIM_CHARGE_LOOP) // 9
					{
						SendWeaponAnim(ANIM_CHARGE_LOOP, UseDecrement() != FALSE);
						return CBasePlayerWeapon::ItemPostFrame();
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
			else if (gpGlobals->time - phs2 >= 0.0f)
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
			phs9_10_11.push_back(pEntity);
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
		sub_10334A80();
		if (!phs5_6_7.empty())
		{
			// ???
		}
		return;
	}

	size_t iLoopNum = phs9_10_11.size() / 4;
	for (size_t v8 = 0; v8 < iLoopNum; ++v8)
	{
		CBaseEntity *pEntity = phs9_10_11[4 * v8];
		Vector vecDirection = (pEntity->pev->origin - m_pPlayer->pev->origin).Normalize();

		TraceResult tr;
		UTIL_TraceLine(m_pPlayer->pev->origin, pEntity->pev->origin, missile, ENT(m_pPlayer->pev), &tr);

		ClearMultiDamage();
		pEntity->TraceAttack(m_pPlayer->pev, GetDamage_PrimaryAttack_Instant(), vecDirection, &tr, DMG_BULLET);
		ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);

		
		if (v8 < 3)
		{
			//CBaseEntity *v36 = phs5_6_7[4 * v8];

			//phs5_6_7.push_back(pEntity);
			// ???
		}
	}
}

void CGungnir::sub_10334A80()
{
	auto v1 = phs5_6_7.begin();
	size_t v2 = (phs5_6_7.size() + 3) / 2;
	
	for (size_t v3 = 0; v3 != v2; ++v3)
	{
		if (phs5_6_7[v3])
			; // ???
		++v1;
	}

	phs9_10_11.clear();
}

