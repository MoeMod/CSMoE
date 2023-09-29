#pragma once

#ifndef CLIENT_DLL

#include "effects.h"
#include "customentity.h"

#endif
#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

class COZWpnSet1 : public CBasePlayerWeapon
{
	enum ozwpnset1_e
    {
        ANIM_IDLE1,
		ANIM_SHOOT1,
		ANIM_SHOOT2,
		ANIM_RELOAD,
		ANIM_DRAW,
		ANIM_DRAW2,
		ANIM_SHOOT_EMPTY,
		ANIM_SHOOT_CHARGE,

    };
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override;
	int iItemSlot() override { return PRIMARY_WEAPON_SLOT; }
	void ItemPostFrame() override;
	void PrimaryAttack() override;
	void Reload() override;
	void WeaponIdle() override;
	BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
	KnockbackData GetKnockBackData() override;
	float GetArmorRatioModifier() override { return 1.5; }
	const char *GetCSModelName() override { return "models/w_ozwpnset1.mdl"; }
#ifndef CLIENT_DLL
		WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return ::sv::GetBuyAmmoConfig(WEAPON_M249); }
#endif
	WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 6, 3, 140 }; }
public:
	void OzWpnSet1Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim);
	float GetDamage() const;
	int GetReloadClip() const
	{
		int iBonus = 0;

		CBasePlayerWeapon* pKnife = (CBasePlayerWeapon*)m_pPlayer->m_rgpPlayerItems[KNIFE_SLOT];

		if (pKnife && !Q_strcmp(STRING(pKnife->pev->classname), "knife_ozwpnset3"))
		{
			iBonus++;
		}

		CBasePlayerWeapon* pWeapon = (CBasePlayerWeapon*)m_pPlayer->m_rgpPlayerItems[PISTOL_SLOT];

		if (pWeapon && pWeapon->m_iId == WEAPON_GLOCK18)
		{
			iBonus++;
		}

		return 20 + 10 * iBonus;
	}

	int GetMaxClip()
	{
		int iBonus = 0;

		CBasePlayerWeapon* pKnife = (CBasePlayerWeapon*)m_pPlayer->m_rgpPlayerItems[KNIFE_SLOT];

		if (pKnife && !Q_strcmp(STRING(pKnife->pev->classname), "knife_ozwpnset3"))
		{
			iBonus ++;
		}

		CBasePlayerWeapon* pWeapon = (CBasePlayerWeapon*)m_pPlayer->m_rgpPlayerItems[PISTOL_SLOT];

		if (pWeapon && pWeapon->m_iId == WEAPON_GLOCK18)
		{
			iBonus++;
		}

		return 100 + iBonus * 50;
	}

	unsigned short m_usFire;
	int m_iShell;

	time_point_t m_tNextPlayEvent;	//phs[4]
	time_point_t m_flNextReload;

	

};

class CIgniteMG : public COZWpnSet1
{
	enum ignitemg_e
	{
		ANIM_IDLE1,
		ANIM_SHOOT1,
		ANIM_SHOOT2,
		ANIM_RELOAD,
		ANIM_DRAW,
		ANIM_DRAW2,
		ANIM_SHOOT_EMPTY,
		ANIM_SHOOT_CHARGE,
		ANIM_SHOOT2_START,
		ANIM_SHOOT2_LOOP,
		ANIM_SHOOT2_END,
	};
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo* p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override;
	int iItemSlot() override { return PRIMARY_WEAPON_SLOT; }
	void ItemPostFrame() override;
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	void Reload() override;
	void WeaponIdle() override;
	float GetDamage() const;
	void Holster(int skiplocal) override;
	int ExtractAmmo(CBasePlayerWeapon* pWeapon) override;
	BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
	KnockbackData GetKnockBackData() override;
	float GetArmorRatioModifier() override { return 1.5; }
	const char* GetCSModelName() override { return "models/w_ignitemg.mdl"; }
#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return ::sv::GetBuyAmmoConfig(WEAPON_M249); }
#endif
	WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 6, 3, 140 }; }
public:
	void IgniteMGFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim);

	int GetReloadClip() const
	{
		return 30;
	}

	int GetMaxClip()
	{
		return 150;
	}

	float GetDamageB() const;

	void ChargeAmmo();
	void Attack();
	void ResetChargeTimer();

	void CreateEffect();
	void ClearEffect();
	void DestroyEffect();
	void SecondaryAttack_FindTargets();
	bool SecondaryAttack_CheckTargetAvailable(CBaseEntity* a2, Vector vecAngleDirection);
	void SecondaryAttack_InstantDamage();
	duration_t GetChargeTime();
#ifndef CLIENT_DLL
	std::array<CBeam*, 3> m_pBeamList;
	std::vector<EHANDLE> m_pEnemyList;
#endif

	int m_iBeamIndex;
	int m_iHitIndex;
	int m_iChargedAmmo;
	int m_iDefaultAmmo2;
	time_point_t m_flAttackLoop;
	time_point_t m_flTimeNextCharge;
};

}
