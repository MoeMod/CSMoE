#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

//xm1014
#define XM1014_MAX_SPEED	240
#define XM1014_CONE_VECTOR	Vector(0.0725, 0.0725, 0.0)	// special shotgun spreads

class CBalrog11WC : public CBasePlayerWeapon
{
	enum xm1014_e
    {
		BALROG11_IDLE,
		BALROG11_SHOOT1,
		BALROG11_SHOOT2,
		BALROG11_SHOOTBCS,
		BALROG11_INSERT,
		BALROG11_AFTER_RELOAD,
		BALROG11_START_RELOAD,
		BALROG11_DRAW
    };
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override { return XM1014_MAX_SPEED; }
	int iItemSlot() override { return PRIMARY_WEAPON_SLOT; }
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	bool HasSecondaryAttack(void) override { return true; }
	void Reload() override;
	void WeaponIdle() override;
	int ExtractAmmo(CBasePlayerWeapon* pWeapon) override;
#ifndef CLIENT_DLL
	void BuckBcsFire(float fYaw, CUtlVector<CBaseEntity*>* pList, CUtlVector<CBaseEntity*>* pGroupList);
#endif
	BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
	KnockbackData GetKnockBackData() override { return { 700.0f, 450.0f, 600.0f, 450.0f, 0.4f }; }
	const char* GetCSModelName() override { return "models/w_balrog11wc.mdl"; }

public:
	duration_t m_flPumpTime;
	int m_iDefaultAmmo2;
	int m_iChargeShotsFired;
	float GetDamage() const;
	float GetDamageBcs() const;

private:
	unsigned short m_usFireBalrog11;
};

}
