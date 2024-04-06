#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "wpn_divinetitanex.h"
#include "weapons/KnifeAttack.h"
#include "monsters.h"
#include "decals.h"
#include "gamerules.h"
#include "cbase/cbase_hash.h"

#ifndef CLIENT_DLL
#include "effects.h"
#include "customentity.h"
#include "player/player_mod_strategy.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

LINK_ENTITY_TO_CLASS(weapon_divinetitanex, CDivineTitanEx)

static constexpr const auto& ChargingTime = 0.15s;
static constexpr int InitialGauge = 50;
static constexpr const auto& ColorChangeTime = 1.5s;
static constexpr const auto& ChangeTime_D2A = 0.2s;
static constexpr const auto& ChangeTime_D2B = 0.2s;
static constexpr const auto& ChangeTime_D2C = 0.2s;
static constexpr const auto& ChangeTime_A2D = 0.25s;
static constexpr const auto& ChangeTime_B2D = 0.25s;
static constexpr const auto& ChangeTime_A2B = 0.2s;
static constexpr const auto& ChangeTime_B2A = 0.2s;
static constexpr const auto& ChangeTime_Throw = 0.2s;
static constexpr const auto& ChangeTime_ThrowDelay = 0.25s;

static constexpr const int Alert_Alpha = 100;
static constexpr const duration_t A_Cycle = 0.12s;
static constexpr const duration_t A_GaugeDecrTime = 0.2s;
static constexpr const duration_t A_GaugeDecrTimeScenario = 0.4s;

static constexpr float A_1stAttackDist = 8.3;
static constexpr float A_2ndAttackDist = 6.7;
static constexpr int A_AttackCount = 30;
static constexpr float A_DamageMul = 0.5;

static constexpr float A_Damage = 5.0;
static constexpr float A_DamageZombie = 180.0;
static constexpr float A_DamageScenario = 400.0;
static constexpr float A_LenG = 100.0;
static constexpr float A_LenGZombie = 150.0;
static constexpr float A_LenGScenario = 100.0;
static constexpr float A_LenJ = 100.0;
static constexpr float A_LenJZombie = 100.0;
static constexpr float A_LenJScenario = 100.0;
static constexpr float A_LenFJ = 100.0;
static constexpr float A_LenFJZombie = 120.0;
static constexpr float A_LenFJScenario = 100.0;
static constexpr float A_LenC = 100.0;
static constexpr float A_LenCZombie = 100.0;
static constexpr float A_LenCScenario = 100.0;
static constexpr float A_VelMod = 1.0;
static constexpr float A_VelModZombie = 0.3;
static constexpr float A_VelModScenario = 0.5;

static constexpr float A_LargeFlinch = 0.0;
static constexpr float A_LargeFlinchZombie = 1.0;
static constexpr float A_LargeFlinchScenario = 0.0;

static constexpr const duration_t A_FixedMoveDuration = 1.0s;
static constexpr float A_FixedMoveSpeed = 100.0;


static constexpr const duration_t B_Cycle = 0.7s;
static constexpr const duration_t B_GaugeDecrTime = 0.2s;
static constexpr const duration_t B_GaugeDecrTimeScenario = 0.4s;

static constexpr float B_MaxDist = 35.0;
static constexpr float B_MissileSpeed = 1500.0;
static constexpr float B_MissileSize = 12.0;
static constexpr float B_Damage = 12.0;
static constexpr float B_DamageZombie = 1050.0;
static constexpr float B_DamageScenario = 3050.0;
static constexpr float B_DamageRadius = 2.5;

static constexpr float B_LenG = 100.0;
static constexpr float B_LenGZombie = 250.0;
static constexpr float B_LenGScenario = 100.0;
static constexpr float B_LenJ = 100.0;
static constexpr float B_LenJZombie = 100.0;
static constexpr float B_LenJScenario = 100.0;
static constexpr float B_LenFJ = 100.0;
static constexpr float B_LenFJZombie = 100.0;
static constexpr float B_LenFJScenario = 100.0;
static constexpr float B_LenC = 100.0;
static constexpr float B_LenCZombie = 100.0;
static constexpr float B_LenCScenario = 100.0;
static constexpr float B_VelMod = 1.0;
static constexpr float B_VelModZombie = 0.4;
static constexpr float B_VelModScenario = 0.8;

static constexpr float B_LargeFlinch = 1.0;
static constexpr float B_LargeFlinchZombie = 1.0;
static constexpr float B_LargeFlinchScenario = 1.0;

static constexpr const duration_t B_FixedMoveDuration = 1.0s;
static constexpr float B_FixedMoveSpeed = 100.0;


static constexpr float C_ThrowVelocity = 1200.0;
static constexpr float C_Radius = 6.2;
static constexpr float C_PullPower = 500.0;
static constexpr const duration_t C_FixedSpeedTime = 2.5s;
static constexpr const duration_t C_PullDamageInterval = 0.3s;

static constexpr float C_PullDamage = 10.0;
static constexpr float C_PullDamageZombie = 450.0;
static constexpr float C_PullDamageScenario = 500.0;

static constexpr float C_SlamDamage = 100.0;
static constexpr float C_SlamDamageZombie = 31600.0;
static constexpr float C_SlamDamageScenario = 45000.0;

static constexpr duration_t C_SlamTime = 1.5s;
static constexpr int C_MinGaugeForSpecialCharging = 50;
static constexpr int C_MinGaugeForSpecialChargingScenario = 25;

void CDivineTitanEx::Spawn(void)
{
	m_iGrenadeID = GRENADE_DIVINETITANEX;
	Precache();
	m_iId = WEAPON_HEGRENADE;
	SET_MODEL(ENT(pev), "models/w_divinetitanex.mdl");

	m_iDefaultAmmo = 30;

	FallInit();
}

void CDivineTitanEx::Precache(void)
{
	PRECACHE_MODEL("sprites/muzzleflash105.spr");

	PRECACHE_MODEL("models/v_divinetitanex.mdl");
	PRECACHE_MODEL("models/p_divinetitanex.mdl");
	PRECACHE_MODEL("models/w_divinetitanex.mdl");

}

int CDivineTitanEx::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "divinetitanex";
	p->iMaxAmmo1 = 100;
	p->pszAmmo2 = "divinetitanexgauge";
	p->iMaxAmmo2 = 1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 3;
	p->iPosition = 1;
	p->iId = m_iId = WEAPON_HEGRENADE;
	p->iWeight = HEGRENADE_WEIGHT;
	p->iFlags = 0;

	return 1;
}

BOOL CDivineTitanEx::Deploy(void)
{
	m_fMaxSpeed = 260;
#ifndef CLIENT_DLL
	SetThink(nullptr);
	
#endif
	if (DefaultDeploy("models/v_divinetitanex.mdl", "models/p_divinetitanex.mdl", ANIM_DEFAULTTOA, "dragontail", UseDecrement() != FALSE, 0.1s))
	{
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.75s;
		return TRUE;
	}
	return FALSE;
}

BOOL CDivineTitanEx::CanHolster(void)
{
	return CBasePlayerWeapon::CanHolster();
}

void CDivineTitanEx::Holster(int skiplocal)
{
	
	return CBasePlayerWeapon::Holster(skiplocal);
}

void CDivineTitanEx::ItemPostFrame()
{

	return CBasePlayerWeapon::ItemPostFrame();
}


void CDivineTitanEx::PrimaryAttack(void)
{

}

void CDivineTitanEx::SecondaryAttack(void)
{


}

void CDivineTitanEx::WeaponIdle(void)
{
	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 8.4s;
	SendWeaponAnim(ANIM_IDLE1, UseDecrement() != FALSE);

}

#ifndef CLIENT_DLL
class CBeam;

LINK_ENTITY_TO_CLASS(divinetitanex_drone, CDivineTitanExDrone);

void CDivineTitanExDrone::Precache(void)
{
	PRECACHE_MODEL("models/w_divinetitanex.mdl");

	
}

void CDivineTitanExDrone::Spawn()
{
	Precache();
	pev->movetype = MOVETYPE_NOCLIP;
	pev->solid = SOLID_NOT;

	if (pev->classname)
	{
		RemoveEntityHashValue(pev, STRING(pev->classname), CLASSNAME);
	}

	MAKE_STRING_CLASS("grenade", pev);
	AddEntityHashValue(pev, STRING(pev->classname), CLASSNAME);

	SET_MODEL(ENT(pev), "models/w_divinetitanex.mdl");
	UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));

	pBeam = CBeam::BeamCreate("sprites/ef_droneattacklightning.spr", 160);
	pBeam->SetColor(255, 255, 255);
	pBeam->SetScrollRate(30);
	pBeam->SetBrightness(0);
	pBeam->SetNoise(30);
	pBeam->pev->effects |= EF_NODRAW;
}

void CDivineTitanExDrone::ClearBeam()
{
	pBeam->SetBrightness(0);
	pBeam->pev->effects |= EF_NODRAW;
}


void CDivineTitanExDrone::OnThink()
{
	if (!m_pOwner)
	{
		UTIL_Remove(pBeam);
		UTIL_Remove(this);
		return;
	}

	if (m_pOwner->m_bIsZombie || !m_pOwner->IsAlive())
	{
		UTIL_Remove(pBeam);
		UTIL_Remove(this);
		return;
	}

	if(m_pOwner->m_iGrenadeID != GRENADE_DIVINETITANEX)
	{
		UTIL_Remove(pBeam);
		UTIL_Remove(this);
		return;
	}

	StudioFrameAdvance();
	pev->nextthink = gpGlobals->time + 0.01s;


}

LINK_ENTITY_TO_CLASS(divinetitanex_grenade, CDivineTitanExGrenade);

LINK_ENTITY_TO_CLASS(divinetitanex_missile, CDivineTitanExMissile);

#endif
}
