#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

//AUG
#define REVIVEGUN_MAXCLIP	 60
#define AUG_MAX_SPEED			240
#define AUG_DAMAGE			32
#define AUG_RANGE_MODIFER		0.96
#define SECNDARYATTACK_RADIUS		900
#define MAXTARGET		12

class CBeam;

class CReviveGun : public CBasePlayerWeapon
{
	enum revivegun_e
    {
		REVIVEGUN_IDLE,
		REVIVEGUN_RELOAD,
		REVIVEGUN_DRAW,
		REVIVEGUN_SHOOTA,
		REVIVEGUN_SHOOTB_START,
		REVIVEGUN_SHOOTB,
		REVIVEGUN_REVIVE_IDLE,
		REVIVEGUN_REVIVE_RELOAD,
		REVIVEGUN_REVIVE_DRAW,
		REVIVEGUN_REVIVE_SHOOTA,
		REVIVEGUN_REVIVE_SHOOTB_START,
		REVIVEGUN_REVIVE_SHOOTB,
		REVIVEGUN_REVIVE
    };
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override { return AUG_MAX_SPEED; }
	int iItemSlot() override { return PRIMARY_WEAPON_SLOT; }
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	void Holster(int skiplocal) override;
	void Reload() override;
	void ItemPostFrame() override;
	void WeaponIdle() override;
	int ExtractAmmo(CBasePlayerWeapon* pWeapon) override;
	void SecondaryAttack_FindTargets();
	void SecondaryAttack_InstantDamage();
	bool SecondaryAttack_CheckTargetAvailable(CBaseEntity* a2, Vector vecAngleDirection);
	bool IsSignal();
	void ClearEffect();
	void CreateEffect();
	void DestroyEffect();

	BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
	KnockbackData GetKnockBackData() override { return { 550.0f,250.0f,300.0f,300.0f,0.5f }; }

public:
	void ReviveGunFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim);
	const char *GetCSModelName() override { return "models/w_revivegun.mdl"; }
#ifndef CLIENT_DLL
	Vector FireBullets(Vector vecSrc, Vector vecDirShooting, float vecSpread, float flDistance, int iPenetration, int iBulletType, int iDamage, float flRangeModifier, entvars_t* pevAttacker, bool bPistol, int shared_rand);
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_556natostar" , 200 }; }
#endif

	float GetDamage() const;
	float GetSecondaryAttackDamage() const;

	std::vector<EHANDLE> m_pEnemyList;
	std::array<CBeam*, MAXTARGET> m_pBeamList;

private:
	time_point_t m_tBeamThink;
	time_point_t m_tNextPlayEvent;	//phs[4]
	int m_iChargeShotsFired;
	time_point_t m_flTimeHostler;
	time_point_t m_flTimeNextCharge;
	int m_iThinkType;
	int m_iDefaultAmmo2;
	unsigned short m_usFireReviveGun;
	time_point_t m_tNextThink;
	

};

}
