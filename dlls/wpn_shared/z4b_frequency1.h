#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

//Deagle
#define FREQUENCY1_MAX_SPEED	250
#define FREQUENCY1_DAMAGE		54
#define FREQUENCY1_RANGE_MODIFER	0.81
#define FREQUENCY1_RELOAD_TIME	2.2s
#define FREQUENCY1_MAX_CLIP	6
#define FREQUENCY1_PENETRATION	9

class CFrequency1 : public CBasePlayerWeapon
{
	enum frequency1_e
    {
		ANIM_IDLE = 0,
		ANIM_SHOOT1,
		ANIM_SHOOT2,
		ANIM_SHOOT3,
		ANIM_RELOAD,
		ANIM_DRAW,
		ANIM_SHOOT_SP1,
		ANIM_SHOOT_SP2,
		ANIM_HIGHNOON_START,
		ANIM_HIGHNOON_LOOP,
		ANIM_HIGHNOON_END,
		ANIM_HIGHNOON_STARTSHOOT,
		ANIM_HIGHNOON_SHOOT1,
		ANIM_HIGHNOON_SHOOT2,
		ANIM_HIGHNOON_SHOOT3,
		ANIM_FIGHT,
		ANIM_FLASHBANG,
		ANIM_DASH,
    };
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override;
	int iItemSlot() override { return PISTOL_SLOT; }
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	bool HasSecondaryAttack() override { return true; }
	void Reload() override;
	void WeaponIdle() override;
	void ItemPostFrame() override;
	void Holster(int skiplocal) override;
	BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
	BOOL IsPistol() override { return TRUE; }
	const char* GetCSModelName() override { return "models/z4b/pw_mccree.mdl"; }
	int ExtractAmmo(CBasePlayerWeapon* pWeapon) override;
	KnockbackData GetKnockBackData() override { return { 400.0f, 300.0f, 300.0f, 200.0f, 0.7f }; }

public:
	void Frequency1Fire(float flSpread, duration_t flCycleTime, BOOL fUseSemi);
	void Frequency1FireSP(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim);
	
	float GetDamage1() const;
	float GetDamage2() const;
	float GetDamage3() const;
	bool CanUseSkill();
	int m_iUsingSkill;
	int m_iHolsterAmmo;
	int m_iIndex;
	time_point_t m_tAimTime;
	time_point_t m_flTimeHostler;
	time_point_t m_flTimeNextCharge;
	int m_iShell;
	int m_iDefaultAmmo2;
	int m_iSprBeam;
	int m_iIsTarget[33];
	
#ifndef CLIENT_DLL
	void Frequency1HighNoonFire(int iIndex);
	void CalcIsTarget();
	bool is_target_avaliable(int iIndex);
	Vector CalcPositon();
	void Effect_Beam(Vector vecStart, Vector vecEnd, int iLife, int iBrightness, int r, int g, int b, int iWidth, int iScollSpeed);
#endif
private:
	unsigned short m_usFireFrequency1;
};

}
