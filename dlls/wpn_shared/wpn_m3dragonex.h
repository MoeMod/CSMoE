#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

//m3dragonex
#define M3DRAGONEX_MAX_SPEED		240
#define M3DRAGONEX_CONE_VECTOR		Vector(0.0675, 0.0675, 0.0)	// special shotgun spreads
#define M3DRAGONEX_MAX_CLIP	15

class CM3DragonEX : public CBasePlayerWeapon
{
	enum m3dragonex_e
    {
        M3DRAGONEX_IDLE,
        M3DRAGONEX_FIRE1,
        M3DRAGONEX_FIRE2,
		M3DRAGONEX_INSERT,
		M3DRAGONEX_AFTER_RELOAD,
		M3DRAGONEX_BEFORE_RELOAD,
        M3DRAGONEX_DRAW,
        M3DRAGONEX_SECONDARY_READY,
		M3DRAGONEX_SECONDARY_SHOOT,
		M3DRAGONEX_SECONDARY_INSERT,
		M3DRAGONEX_SECONDARY_AFTER_RELOAD,
		M3DRAGONEX_SECONDARY_BEFORE_RELOAD,
		M3DRAGONEX_SECONDARY_DRAW,
		M3DRAGONEX_SHOOT_SPECIAL,
		M3DRAGONEX_CHARGE_START,
		M3DRAGONEX_CHARGE_IDLE,
		M3DRAGONEX_CHARGE_ATTACK,
		M3DRAGONEX_CHARGE_STARTB,
		M3DRAGONEX_CHARGE_IDLEB,
		M3DRAGONEX_CHARGE_ATTACKB,

    };
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override { return M3DRAGONEX_MAX_SPEED; }
	int iItemSlot() override { return PRIMARY_WEAPON_SLOT; }
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	void ItemPostFrame() override;
	bool HasSecondaryAttack() override { return true; }
	int ExtractAmmo(CBasePlayerWeapon* pWeapon) override;
	void Reload() override;
	void WeaponIdle() override;
	void Holster(int skiplocal) override;
	const char* GetCSModelName() override { return "models/w_m3dragonex.mdl"; }
	BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
	void StopIdleSound();
	int GetDamage();
	float GetDamageB();
	float GetDamageC();
	KnockbackData GetKnockBackData() override { return { 3000.0f, 480.0f, 900.0f, 1600.0f, 0.3f }; }

#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_m3dragonex" , 60 }; }
#endif
	float GetArmorRatioModifier() override { return 1.1; }
	WeaponCrosshairData GetCrosshairData() override { return { 0, 8, 6, 0 }; }

	void OnAttack2Released();
	void M3DragonEXShootCannon();
	void SecondaryAttack_SignExplode();
public:
	int m_iShell;
	duration_t m_flPumpTime;
	time_point_t m_flTimeCheckCharge;
	int m_iState;

#ifndef CLIENT_DLL
	Vector KnifeAttack2(Vector vecSrc, Vector vecDir, float flDamage, float flRadius, float flAngleDegrees, int bitsDamageType, entvars_t* pevInflictor, entvars_t* pevAttacker, bool &bHit);
	std::vector<EHANDLE> m_pEnemylist;
	std::vector<int> m_iTargetID;
	void SecondaryAttack_FindTargets();
	bool SecondaryAttack_CheckTargetAvailable(CBaseEntity* a2);
	void CheckEnemylist();
	void ClearSign(CBaseEntity* pEntity, bool bExplode);
#endif
private:
	int m_iDefaultAmmo2;
	time_point_t m_flNextPlaySound;
	unsigned short m_usFire;

	int m_iSignStart;
	int m_iSignLoop;
	int m_iSignEnd;
	int m_iSignExp[3];

};

}
