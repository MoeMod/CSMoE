#ifndef WPN_AUG_H
#define WPN_AUG_H
#ifdef _WIN32
#pragma once
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

//AUG
#define AUG_MAX_SPEED			240
#define AUG_DAMAGE			32
#define AUG_RANGE_MODIFER		0.96
#define AUG_RELOAD_TIME			3.3

class CStarchaserar : public CBasePlayerWeapon
{
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override { return AUG_MAX_SPEED; }
	int iItemSlot() override { return PRIMARY_WEAPON_SLOT; }
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	void Reload() override;
	void WeaponIdle() override;
	BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
	KnockbackData GetKnockBackData() override { return { 350.0f,250.0f,300.0f,100.0f,0.6f }; }

public:
	void StarchaserarFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim);
	void RadiusDamage(Vector vecAiming, float flDamage);
	const char *GetCSModelName() override { return "models/w_starchaserar.mdl"; }
	Vector Get_ShootPosition(CBaseEntity *pevAttacker, Vector vecSrc);
	float GetDamage() const;
	int m_iShell;
	int iShellOn;
	
private:
	unsigned short m_usFireStarchaserar;
	duration_t tNextAttack;
	time_point_t tWorldTime;
	duration_t tDelta;
};

}

#endif