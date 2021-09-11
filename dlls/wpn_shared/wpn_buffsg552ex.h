#ifndef WPN_SG552_H
#define WPN_SG552_H
#ifdef _WIN32
#pragma once
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

//buffsg552ex
#define SPECIALSCOPE		0
#define BUFFSG552EX_MAX_SPEED			235
#define BUFFSG552EX_MAX_SPEED_ZOOM		200
#define BUFFSG552EX_DAMAGE			33
#define BUFFSG552EX_RANGE_MODIFER		0.955
#define BUFFSG552EX_RELOAD_TIME		2.03S
#define BUFFSG552EX_INSPECT_TIME		4.48S
#define SIGNAL_SHOTS_COUNT 40
#define SIGNAL_SHOTS_COUNT_ZB 40
#define SIGNAL_SHOTS_COUNT_ZS 40

class CBuffsg552ex : public CBasePlayerWeapon
{
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override;
	int iItemSlot() override { return PRIMARY_WEAPON_SLOT; }
	void PrimaryAttack() override;
	BOOL IsSignal();
	void SecondaryAttack() override;
	void Reload() override;
	void WeaponIdle() override;
	const char* GetCSModelName() override { return "models/w_buffsg552ex.mdl"; }
	BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
	KnockbackData GetKnockBackData() override { return { 350.0f, 250.0f, 300.0f, 100.0f, 0.6f }; }

public:
	void Buffsg552exFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim);
	void Buffsg552exFireB(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim);
	void RadiusDamage(Vector vecAiming, float flDamage);
	Vector Get_ShootPosition(CBaseEntity* pevAttacker, Vector vecSrc, Vector vecDir);
	float GetDamage();
	float SlashDamage();

	int m_iShell;
	int iShellOn;
	int m_iShotsCount;

private:
	unsigned short m_usFireBuffsg552ex;
};

}

#endif