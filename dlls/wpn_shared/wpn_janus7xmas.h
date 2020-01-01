#ifndef WPN_MG3_H
#define WPN_MG3_H
#ifdef _WIN32
#pragma once
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

//mg3
#define MG3_MAX_SPEED			235
#define MG3_DAMAGE			35
#define MG3_RANGE_MODIFER		0.79
#define MG3_RELOAD_TIME		4.8
#define SIGNAL_SHOTS_COUNT 100
#define SIGNAL_SHOTS_COUNT_ZB 110
#define SIGNAL_SHOTS_COUNT_ZS 120
#define WEAPON_CLIP 200
#define	WEAPON_TIME_RELOAD 4.73s

class CJanus7xmas : public CBasePlayerWeapon
{
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override { return MG3_MAX_SPEED; }
	int iItemSlot() override { return PRIMARY_WEAPON_SLOT; }
	void ItemPostFrame() override;
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
	KnockbackData GetKnockBackData() override { return { 350.0f, 250.0f, 300.0f, 100.0f, 0.6f }; }
		const char *GetCSModelName() override { return "models/w_janus7xmas.mdl"; }

public:
	void Janus7xmasFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim);
	void Janus7Lighting();
	BOOL IsSignal();
	BOOL IsWallBetweenPoints(Vector vecStart, Vector vecEnd);
	BOOL IsTargetAvailable(CBasePlayer *m_pPlayer, CBaseEntity *pEntity, Vector vecSrc);
	float GetDamage() const;
	float GetDamageB() const;
	int m_iShotsCount;
	int m_iShell;
	int m_iSprSmokePuff;
	int m_iSprBeam;
	int m_iJanus7State;
	int iShellOn;
	int iTarget;

private:
	unsigned short m_usFireJanus7xmas;
};

}

#endif